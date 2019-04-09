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

//#define IOCP_LOG
#ifdef EPOLL_LOG
#define EPOLL_DEBUG(fmt,...) NET_DEBUG(fmt,__VA_ARGS__)
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
		Clog::error_throw(errors::system, "epoll close error!");
	}

	EPOLL_DEBUG("destroy epoll handle!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::create_epoll(void)
{
	if (m_epoll >= 0)
	{
		Clog::error_throw(errors::logic, "epoll handle exists!");
	}
	
	m_epoll = epoll_create(256);


	if (m_epoll < 0)
	{
		Clog::error_throw(errors::system, "create epoll handle failure!(%d)",errno);
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

	/*for (size_t i = 0; i < m_threads.size(); ++i)
		PostQueuedCompletionStatus(m_hepoll, -1, NULL, NULL);*/

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

	for (std::uint32_t i = 0; i < nthread; ++i)
		m_threads.push_back(std::thread(std::bind(&io_service_epoll::process_event, this)));

	m_state = static_cast<int>(state::running);
	EPOLL_DEBUG("epoll running! threads=%u", nthread);
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
	data->m_op = io_op::accept;
	data->m_owner = server;
	data->m_buffer.buf = data->m_buff;
	data->m_buffer.buf = data->m_buff;
	data->m_buffer.len = sizeof(data->m_buff);
	data->m_ol = fd;
	
	 struct epoll_event _ev;
    _ev.events = EPOLLIN|EPOLLET;
    _ev.data.ptr = (void*)data;
	
	if(epoll_ctl(m_epoll,EPOLL_CTL_ADD,fd,&_ev) != 0)
	{
		closesocket(fd);
		closesocket(m_epoll);
		Clog::error_throw(errors::system, "server epoll_ctl failure!");
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

	per_io_data* _data = get_send_data(session);
	_data->m_fd = fd;
	_data->m_owner = session;
	_data->m_op = io_op::send;
	
	_data = get_read_data(session);
	_data->m_fd = fd;
	_data->m_owner = session;
	_data->m_op = io_op::read;

	post_read_event(_data);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::process_event(void)
{
	/*DWORD dwTrans = 0;
	fd_t socket;
	per_io_data* data;
	while (m_state != static_cast<int>(state::stopping))
	{
		BOOL bRet = GetQueuedCompletionStatus(m_hepoll, &dwTrans, (LPDWORD)&socket, (LPOVERLAPPED*)&data, WSA_INFINITE);
		if (!bRet)
		{
			DWORD err = GetLastError();
			switch (err)
			{
			case WAIT_TIMEOUT:
				if (data->m_op == io_op::accept)
				{
					EPOLL_DEBUG("accept wait timeout!");
					server_iface* server = static_cast<server_iface*>(data->m_owner);
					server->get_socket()->close_fd(data->m_fd);
					post_accept_event(server, data);
					break;
				}
				EPOLL_DEBUG("connection wait timeout!");
				static_cast<session_iface*>(data->m_owner)->close(close_state::cs_connect_timeout);
				break;
			case ERROR_NETNAME_DELETED:
				if (data->m_op == io_op::accept)
				{
					EPOLL_DEBUG("client close socket befor accept completing!");
					server_iface* server = static_cast<server_iface*>(data->m_owner);
					server->get_socket()->close_fd(data->m_fd);
					post_accept_event(server, data);
					break;
				}
				EPOLL_DEBUG("connection peer closed(ERROR_NETNAME_DELETED)!");
				static_cast<session_iface*>(data->m_owner)->close(close_state::cs_connect_peer_close);
				break;
			case ERROR_OPERATION_ABORTED:
				EPOLL_DEBUG("ERROR_OPERATION_ABORTED!");
				break;
			default:
				Clog::error_throw(err, "epoll GetQueuedCompletionStatus failure!:%d",err);
				break;
			}
			continue;
		}

		if (data == NULL && socket == NULL && dwTrans == -1)
		{
			EPOLL_DEBUG("epoll thread exit by exit event!");
			break;
		}
			
		if (dwTrans == 0 && data->m_op != io_op::accept)
		{
			EPOLL_DEBUG("connection peer closed!");
			static_cast<session_iface*>(data->m_owner)->close(close_state::cs_connect_peer_close);
			continue;
		}

		session_iface* session = nullptr;
		switch (data->m_op)
		{
		case io_op::read:
			{
				session = static_cast<session_iface*>(data->m_owner);
				if(process_read(session,dwTrans))
					post_read_event(data);
			}
			break;
		case io_op::send:
			{
				session = static_cast<session_iface*>(data->m_owner);
				if (process_send(session,dwTrans))
					post_send_event(data);
			}
			break;
		case io_op::accept:
			{
				server_iface* server = static_cast<server_iface*>(data->m_owner);
				setsockopt(data->m_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(socket), sizeof(socket));

				struct sockaddr_storage* addrClient = NULL, *addrLocal = NULL;
				int client_len = sizeof(sockaddr_storage), local_len = sizeof(sockaddr_storage), addrlen = sizeof(sockaddr_storage) + 16;
#if (1)
				m_accept_addrs(data->m_buffer.buf, 0, addrlen, addrlen,
					(LPSOCKADDR*)&addrLocal, &local_len, (LPSOCKADDR*)&addrClient, &client_len);			
#else
				struct sockaddr_storage addr;
				if (SOCKET_ERROR == getpeername(data->m_fd, (sockaddr*)&addr, &client_len))
				{
					EPOLL_DEBUG("getpeername error! errno=%d", WSAGetLastError());
				}

				addrClient = &addr;
#endif
				process_accept(server, data, addrClient, &session);
				post_accept_event(server, data);
				if (session) track_session(session);
			}
			break;
		}// switch
		
	}

	EPOLL_DEBUG("exit request!");*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::post_accept_event(server_iface* server, per_io_data* data)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::post_read_event(per_io_data* data)
{
	 struct epoll_event _ev;
    _ev.events = EPOLLIN|EPOLLET;
    _ev.data.ptr = (void*)data;
	
	if(epoll_ctl(m_epoll,EPOLL_CTL_ADD,data->m_fd,&_ev) != 0)
	{
		((session_iface*)data->m_owner)->close(close_state::cs_service_stop);
		Clog::error_throw(errors::system, "read epoll_ctl failure!(%d)",errno);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_epoll::post_send_event(per_io_data* data)
{
	if (m_state != static_cast<int>(state::running))
		return;

	 struct epoll_event _ev;
    _ev.events = EPOLLOUT|EPOLLET;
    _ev.data.ptr = (void*)data;
	
	if(epoll_ctl(m_epoll,EPOLL_CTL_ADD,data->m_fd,&_ev) != 0)
	{
		((session_iface*)data->m_owner)->close(close_state::cs_service_stop);
		Clog::error_throw(errors::system, "send epoll_ctl failure!(%d)",errno);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //_WIN32