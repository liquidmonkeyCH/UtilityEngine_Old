/**
* @file net_io_service_iocp.cpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef _WIN32
#include "net_io_service_epoll.hpp"
#include "net_session.hpp"
#include "net_server.hpp"
#include <sys/sysinfo.h>

#define EPOLL_LOG
#ifdef EPOLL_LOG
#define EPOLL_DEBUG(fmt,...) NET_DEBUG(fmt,##__VA_ARGS__)
#else
#define EPOLL_DEBUG(fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
using close_state = session_iface::reason;
////////////////////////////////////////////////////////////////////////////////////////////////////
io_service_epoll::io_service_epoll(void)
: m_state({ static_cast<int>(state::none) })
, m_epoll(-1)
{
	EPOLL_DEBUG("create io_service_epoll");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
io_service_epoll::~io_service_epoll(void)
{
	EPOLL_DEBUG("destroy io_service_epoll");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::close(void)
{
	m_threads.clear();

	if (closesocket(m_epoll) != 0)
	{
		Clog::error_throw(errors::system, "close epoll error!(%d)",errno);
	}

	EPOLL_DEBUG("destroy epoll handle!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::create_epoll(void)
{
	if (m_epoll >= 0)
	{
		Clog::error_throw(errors::logic, "epoll exists!");
	}
	
	m_epoll = epoll_create(MAXEVENTS);

	if (m_epoll < 0)
	{
		Clog::error_throw(errors::system, "create epoll failure!(%d)",errno);
	}

	EPOLL_DEBUG("create epoll handle!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::stop(void)
{
	int st = static_cast<int>(state::running);
	if (!m_state.compare_exchange_strong(st, static_cast<int>(state::stopping)))
	{
		if (st == static_cast<int>(state::starting))
		{
			Clog::error_throw(errors::logic, "stop io_service_epoll on starting!");
		}
		EPOLL_DEBUG("already stopping!");
		return;
	}

	EPOLL_DEBUG("epoll stopping!");

	for (std::thread& th : m_threads)
		th.join();

	close();

	m_state = static_cast<int>(state::none);

	EPOLL_DEBUG("epoll stopped!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::start(std::uint32_t nthread)
{
	int st = static_cast<int>(state::none);
	if (!m_state.compare_exchange_strong(st, static_cast<int>(state::starting)))
		return;

	EPOLL_DEBUG("epoll starting!");

	create_epoll();

	if (nthread == 0)
		nthread = get_nprocs();

	m_mulit_threads = nthread > 1;
		
	m_events_pool.init(nthread);

	for (std::uint32_t i = 0; i < nthread; ++i)
	{
		epoll_event* m_events  = m_events_pool.malloc()->m_data;
		m_threads.push_back(std::thread(std::bind(&io_service_epoll::process_event, this, m_events)));
	}

	m_state = static_cast<int>(state::running);
	EPOLL_DEBUG("epoll running! threads=%u", nthread);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
io_service_epoll::epoll_control(int op,fd_t fd,epoll_event* ev)
{
	while(epoll_ctl(m_epoll,op,fd,ev) < 0)
	{
		if(errno == EINTR)
			continue;
		
		return false;
	}
	
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::track_server(server_iface* server)
{
	if (m_state != static_cast<int>(state::running))
		return;

	EPOLL_DEBUG("epoll track_server!");
	
	socket_iface* socket = server->get_socket();
	socket->set_blocking(false);
	fd_t fd = socket->get_fd();

	accept_data* data = get_accept_data(server);
	data->m_ol.m_inuse = false;
	data->m_ol.m_need_send = false;
	data->m_op = io_op::accept;
	data->m_owner = server;
	data->m_fd = INVALID_SOCKET;
	data->m_buffer.buf = data->m_buff;
	data->m_buffer.len = sizeof(data->m_buff);
	
	struct epoll_event _ev{EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT,{static_cast<void*>(data)}};

	if(!epoll_control(EPOLL_CTL_ADD,fd,&_ev))
	{
		server->stop();
		Clog::error_throw(errors::system, "server:epoll_ctl(EPOLL_CTL_ADD) failure!(%d)",errno);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::untrack_server(server_iface* server)
{
	struct epoll_event _ev{0,{0}};
	if(!epoll_control(EPOLL_CTL_DEL,server->get_fd(),&_ev))
	{
		Clog::error_throw(errors::system, "server:epoll_ctl(EPOLL_CTL_DEL) failure!(%d)",errno);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::track_session(session_iface* session)
{
	if (m_state != static_cast<int>(state::running))
		return;

	//EPOLL_DEBUG("epoll track_session!");

	fd_t fd = session->get_fd();
	bind(session);
	
	per_io_data* data = get_send_data(session);
	data->m_ol.m_inuse = false;
	data->m_ol.m_need_send = false;
	data->m_op = io_op::other;
	data->m_owner = session;
	data->m_fd = fd;

	data = get_recv_data(session);
	data->m_ol.m_inuse = false;
	data->m_ol.m_need_send = false;
	data->m_op = io_op::other;
	data->m_owner = session;
	data->m_fd = fd;
	
	struct epoll_event _ev{EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT,{static_cast<void*>(data)}};

	if(!epoll_control(EPOLL_CTL_ADD,fd,&_ev))
	{
		session->close(close_state::cs_service_stop);
		Clog::error_throw(errors::system, "session:epoll_ctl(EPOLL_CTL_ADD) failure!(%d)",errno);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::untrack_session(session_iface* session)
{
	struct epoll_event _ev{0,{0}};
	if(!epoll_control(EPOLL_CTL_DEL,session->get_fd(),&_ev))
	{
		Clog::error_throw(errors::system, "session:epoll_ctl(EPOLL_CTL_DEL) failure!(%d)",errno);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::process_event(epoll_event* m_events)
{
	per_io_data* data,*sdata;
	session_iface* session = nullptr;
	server_iface* server = nullptr;
	socket_iface* socket = nullptr;
	fd_t fd;

	epoll_event *end,*it;
	int size,len;

	struct sockaddr_storage addr;
	socklen_t addrlen;
	bool b_exp,b_empty;
	
	struct epoll_event _ev;
	
	while (m_state != static_cast<int>(state::stopping))
	{
		size = epoll_wait(m_epoll, m_events, MAXEVENTS, 500);
		if(size == 0)
			continue;
		
		if (size < 0)
		{
			if(errno == EINTR)
				continue;
				
			Clog::error_throw(errors::system, "epoll_wait wait error!(%d)",errno);
		}
		
		end = m_events + size;
		for(it = m_events;it != end; ++it)
		{
			data = static_cast<per_io_data*>(it->data.ptr);
			if(data->m_op == io_op::accept)
			{
				server = static_cast<server_iface*>(data->m_owner);
				socket = server->get_socket();
				fd = socket->get_fd();
				if(fd == INVALID_SOCKET)
					continue;

				do{
					addrlen = sizeof(sockaddr_storage);
					data->m_fd = accept(fd,(sockaddr*)&addr,&addrlen);
					if(data->m_fd == INVALID_SOCKET)
					{
						if(errno == EINTR)
							continue;

						if(errno == EAGAIN || errno == EWOULDBLOCK || !server->is_running())
							break;

						Clog::error_throw(errors::system, "epoll accept error!(%d)",errno);
					}

					process_accept(server,data,&addr,&session);
					if(session) track_session(session);
				}while(true);

				_ev.events = EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT;
				_ev.data.ptr = static_cast<void*>(data);
				
				if(!epoll_control(EPOLL_CTL_MOD,fd,&_ev))
				{
					if(!server->is_running())
						continue;

					server->stop();
					Clog::error_throw(errors::system, "server:epoll_ctl(EPOLL_CTL_MOD) failure!(%d)",errno);
				}	
				continue;
			}//ACCEPT

			if(it->events & EPOLLRDHUP)
			{
				EPOLL_DEBUG("connection peer closed!");
				static_cast<session_iface*>(data->m_owner)->close(close_state::cs_connect_peer_close);
				continue;
			}

			if(it->events & EPOLLIN && it->events & EPOLLOUT)
			{
				EPOLL_DEBUG("epoll in & out");
			}

			while(it->events & EPOLLIN)
			{
				if(m_mulit_threads)
				{
					b_exp = false;
					if(!data->m_ol.m_inuse.compare_exchange_strong(b_exp,true))
					{
						EPOLL_DEBUG("duplicate read");
						break;
					}	
				}
				session = static_cast<session_iface*>(data->m_owner);
				socket = session->get_socket();
				do{
					len = socket->read(data->m_buffer.buf, data->m_buffer.len);
					if(len < 0)
					{
						if(errno == EINTR)
							continue;

						if(errno == EAGAIN)
							break;

						if (errno == ECONNABORTED || errno == ECONNRESET)
						{
							EPOLL_DEBUG("send error: connection peer closed!");
							session->close(close_state::cs_connect_peer_close);
							break;
						}

						Clog::error_throw(errors::system, "epoll read error!(%d)",errno);
					}

					if(len == 0)
					{
						session->close(close_state::cs_connect_peer_close);
						break;
					}

					b_empty = len < data->m_buffer.len;
					b_exp = process_recv(session,len);
					if(b_empty || !b_exp) 
						break;
				}while(true);

				if(m_mulit_threads) 
					data->m_ol.m_inuse = false;

				if(b_exp)
				{
					_ev.events = EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT;
					if(data->m_ol.m_need_send) _ev.events |= EPOLLOUT;
					_ev.data.ptr = static_cast<void*>(data);
					
					if(!epoll_control(EPOLL_CTL_MOD,data->m_fd,&_ev))
					{
						if (errno == EBADF || errno == ENOENT)
							continue;

						session->close(close_state::cs_service_stop);
						Clog::error_throw(errors::system, "recv:epoll_ctl(EPOLL_CTL_MOD) failure!(%d)",errno);
					}
				}
				break;
			}//EPOLLIN

			if(it->events & EPOLLOUT)
			{
				session = static_cast<session_iface*>(data->m_owner);
				sdata = get_send_data(session);
				if(m_mulit_threads)
				{
					b_exp = false;
					if(!sdata->m_ol.m_inuse.compare_exchange_strong(b_exp,true))
					{
						EPOLL_DEBUG("duplicate send");
						continue;
					}	
				}
				
				b_exp = process_send(sdata,data);
				
				if(m_mulit_threads) 
					sdata->m_ol.m_inuse = false;

				if(b_exp)
				{
					data->m_ol.m_need_send = true;
					_ev.events = EPOLLOUT|EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT;
					_ev.data.ptr = static_cast<void*>(data);
					
					if(!epoll_control(EPOLL_CTL_MOD,data->m_fd,&_ev))
					{
						if (errno == EBADF || errno == ENOENT)
							continue;

						session->close(close_state::cs_service_stop);
						Clog::error_throw(errors::system, "send:epoll_ctl(EPOLL_CTL_MOD) failure!(%d)",errno);
					}
				}
			}//EPOLLOUT
		}//FOR
	}//WHILE

	EPOLL_DEBUG("exit request!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
io_service_epoll::process_send(per_io_data* data,per_io_data* _data)
{
	session_iface* session = static_cast<session_iface*>(data->m_owner);
	socket_iface* socket = session->get_socket();
	int len = 0;
	
	if(_data)
	{
		_data->m_ol.m_need_send = false;
		struct epoll_event _ev{EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT,{static_cast<void*>(_data)}};
		if(!epoll_control(EPOLL_CTL_MOD,_data->m_fd,&_ev))
		{
			if (errno == EBADF || errno == ENOENT)
				return false;

			session->close(close_state::cs_service_stop);
			Clog::error_throw(errors::system, "send: epoll_ctl(EPOLL_CTL_MOD) failure!(%d)", errno);
		}
	}
	
	do
	{
		len = write(data->m_fd,data->m_buffer.buf, data->m_buffer.len);
		if (len <= 0)
		{
			if (errno == EINTR)
				continue;

			if (errno == EAGAIN)
				return true;

			if (errno == EBADF)
				return false;

			if (errno == ECONNABORTED || errno == ECONNRESET)
			{
				EPOLL_DEBUG("send error: connection peer closed!");
				session->close(close_state::cs_connect_peer_close);
				return false;
			}

			Clog::error_throw(errors::system, "Send error! erron=%d", errno);
		}

		if (!io_service_iface::process_send(session, len))
			return false;
			
	}while (true);
	
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::post_send_event(per_io_data* data)
{
	if (m_state != static_cast<int>(state::running))
		return;

	if(!process_send(data))
		return;
		
	EPOLL_DEBUG("send: epoll_ctl(EPOLL_CTL_MOD) EPOLLOUT|EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT;");
				
	session_iface* session = static_cast<session_iface*>(data->m_owner);	
	per_io_data* _data = get_recv_data(session);
	_data->m_ol.m_need_send = true;
	struct epoll_event _ev{EPOLLOUT|EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT,{static_cast<void*>(_data)}};
	
	if(!epoll_control(EPOLL_CTL_MOD,_data->m_fd,&_ev))
	{
		if (errno == EBADF || errno == ENOENT)
			return;

		session->close(close_state::cs_service_stop);
		Clog::error_throw(errors::system, "send: epoll_ctl(EPOLL_CTL_MOD) failure!(%d)", errno);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //_WIN32