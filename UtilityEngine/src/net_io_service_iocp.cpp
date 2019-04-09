/**
* @file net_io_service_iocp.cpp
*
* @author Hourui (liquidmonkey)
*/
#ifdef _WIN32
#include "net_io_service_iocp.hpp"
#include "net_session.hpp"
#include "net_server.hpp"
#pragma comment(lib, "Mswsock.lib") 

//#define IOCP_LOG
#ifdef IOCP_LOG
#define IOCP_DEBUG(fmt,...) NET_DEBUG(fmt,__VA_ARGS__)
#else
#define IOCP_DEBUG(fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
using close_state = session_iface::reason;
////////////////////////////////////////////////////////////////////////////////////////////////////
io_service_iocp::io_service_iocp(void)
: m_state({ static_cast<int>(state::none) })
, m_hiocp(NULL)
, m_accept(NULL)
, m_accept_addrs(NULL)
{
	IOCP_DEBUG("create io_service_iocp");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
io_service_iocp::~io_service_iocp(void)
{
	if (m_hiocp) { close(); }
	IOCP_DEBUG("destroy io_service_iocp");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::close(void)
{
	m_threads.clear();

	if (CloseHandle(m_hiocp) == FALSE)
	{
		Clog::error_throw(errors::logic, "iocp close error!");
	}

	m_hiocp = NULL;
	IOCP_DEBUG("destroy iocp handle!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::create_iocp(void)
{
	if (m_hiocp != NULL)
	{
		Clog::error_throw(errors::logic, "iocp handle exists!");
	}

	m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	if (m_hiocp == NULL)
	{
		Clog::error_throw(errors::logic, "create iocp handle failure!");
	}

	IOCP_DEBUG("create iocp handle!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::stop(void)
{
	int state = static_cast<int>(state::running);
	if (!m_state.compare_exchange_strong(state, static_cast<int>(state::stopping)))
	{
		if (state == static_cast<int>(state::starting))
		{
			Clog::error_throw(errors::logic, "stop io_service_iocp on starting!");
		}
		IOCP_DEBUG("already stopping!");
		return;
	}

	IOCP_DEBUG("iocp stopping!");

	for (size_t i = 0; i < m_threads.size(); ++i)
		PostQueuedCompletionStatus(m_hiocp, -1, NULL, NULL);

	for (std::thread& th : m_threads)
		th.join();

	close();

	m_state = static_cast<int>(state::none);

	IOCP_DEBUG("iocp stopped!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::start(std::uint32_t nthread)
{
	int state = static_cast<int>(state::none);
	if (!m_state.compare_exchange_strong(state, static_cast<int>(state::starting)))
		return;

	IOCP_DEBUG("iocp starting!");

	create_iocp();

	if (nthread == 0)
	{
		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);
		nthread = sys_info.dwNumberOfProcessors;
	}

	for (std::uint32_t i = 0; i < nthread; ++i)
		m_threads.push_back(std::thread(std::bind(&io_service_iocp::process_event, this)));

	m_state = static_cast<int>(state::running);
	IOCP_DEBUG("iocp running! threads=%u", nthread);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::track_server(server_iface* server)
{
	if (m_state != static_cast<int>(state::running))
		return;

	IOCP_DEBUG("iocp track_server!");
	fd_t fd = server->get_fd();
	
	DWORD dwbytes = 0;
	GUID guidAcceptEx = WSAID_ACCEPTEX;

	if (0 != WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx, sizeof(guidAcceptEx),
		&m_accept, sizeof(m_accept),
		&dwbytes, NULL, NULL))
	{
		Clog::error_throw(errors::system, "get AcceptEx failure!");
	}
	
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (0 != WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidGetAcceptExSockaddrs,
		sizeof(guidGetAcceptExSockaddrs),
		&m_accept_addrs,
		sizeof(m_accept_addrs),
		&dwbytes, NULL, NULL))
	{
		Clog::error_throw(errors::system, "get AcceptEx failure!");
	}

	if (CreateIoCompletionPort((HANDLE)fd, m_hiocp, fd, 0) != m_hiocp)
	{
		Clog::error_throw(errors::system, "server CreateIoCompletionPort failure!");
	}

	while (accept_data* data = get_accept_data(server))
	{
		data->m_op = io_op::accept;
		data->m_owner = server;
		data->m_buffer.buf = data->m_buff;
		data->m_buffer.len = sizeof(data->m_buff);

		post_accept_event(server, data);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::track_session(session_iface* session)
{
	if (m_state != static_cast<int>(state::running))
		return;

	//IOCP_DEBUG("iocp track_session!");

	fd_t fd = session->get_fd();
	if (CreateIoCompletionPort((HANDLE)fd, m_hiocp, fd, 0) != m_hiocp)
	{
		Clog::error_throw(errors::system, "session CreateIoCompletionPort failure!");
	}

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
io_service_iocp::process_event(void)
{
	DWORD dwTrans = 0;
	fd_t socket;
	per_io_data* data;
	while (m_state != static_cast<int>(state::stopping))
	{
		BOOL bRet = GetQueuedCompletionStatus(m_hiocp, &dwTrans, (LPDWORD)&socket, (LPOVERLAPPED*)&data, WSA_INFINITE);
		if (!bRet)
		{
			DWORD err = GetLastError();
			switch (err)
			{
			case WAIT_TIMEOUT:
				if (data->m_op == io_op::accept)
				{
					IOCP_DEBUG("accept wait timeout!");
					server_iface* server = static_cast<server_iface*>(data->m_owner);
					server->get_socket()->close_fd(data->m_fd);
					post_accept_event(server, data);
					break;
				}
				IOCP_DEBUG("connection wait timeout!");
				static_cast<session_iface*>(data->m_owner)->close(close_state::cs_connect_timeout);
				break;
			case ERROR_NETNAME_DELETED:
				if (data->m_op == io_op::accept)
				{
					IOCP_DEBUG("client close socket befor accept completing!");
					server_iface* server = static_cast<server_iface*>(data->m_owner);
					server->get_socket()->close_fd(data->m_fd);
					post_accept_event(server, data);
					break;
				}
				IOCP_DEBUG("connection peer closed(ERROR_NETNAME_DELETED)!");
				static_cast<session_iface*>(data->m_owner)->close(close_state::cs_connect_peer_close);
				break;
			case ERROR_OPERATION_ABORTED:
				IOCP_DEBUG("ERROR_OPERATION_ABORTED!");
				break;
			default:
				Clog::error_throw(err, "iocp GetQueuedCompletionStatus failure!:%d",err);
				break;
			}
			continue;
		}

		if (data == NULL && socket == NULL && dwTrans == -1)
		{
			IOCP_DEBUG("iocp thread exit by exit event!");
			break;
		}
			
		if (dwTrans == 0 && data->m_op != io_op::accept)
		{
			IOCP_DEBUG("connection peer closed!");
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
					IOCP_DEBUG("getpeername error! errno=%d", WSAGetLastError());
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

	IOCP_DEBUG("exit request!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::post_accept_event(server_iface* server, per_io_data* data)
{
	if (!server->is_running())
		return;

	memset(&data->m_ol, 0, sizeof(data->m_ol));
	memset(data->m_buffer.buf, 0, data->m_buffer.len);
	data->m_fd = server->get_socket()->create_fd();
	DWORD byteReceived = 0;
	DWORD addrLen = sizeof(sockaddr_storage) + 16;
	if (m_accept(server->get_fd(), data->m_fd, data->m_buffer.buf, 0,
		addrLen, addrLen, &byteReceived, (LPOVERLAPPED)data) == FALSE)
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err){
			Clog::error_throw(errors::system, "AcceptEx error errno=%d!", err);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::post_read_event(per_io_data* data)
{
	memset(&data->m_ol, 0, sizeof(data->m_ol));
	DWORD dwRecv = 0;
	DWORD dwFlags = 0;
	if (WSARecv(data->m_fd, &data->m_buffer, 1, &dwRecv, &dwFlags, (OVERLAPPED*)data, NULL) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAECONNABORTED || err == WSAECONNRESET)
		{
			IOCP_DEBUG("recv error: connection peer closed!");
			((session_iface*)data->m_owner)->close(close_state::cs_connect_peer_close);
			return;
		}

		if (err != WSA_IO_PENDING)
		{
			Clog::error_throw(errors::system, "WSARecv error! erron=%d", err);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_service_iocp::post_send_event(per_io_data* data)
{
	if (m_state != static_cast<int>(state::running))
		return;

	memset(&data->m_ol, 0, sizeof(data->m_ol));
	DWORD dwTrans = 0;
	DWORD dwFlags = 0;

	if (WSASend(data->m_fd, &data->m_buffer, 1, &dwTrans, dwFlags, (OVERLAPPED*)data, NULL) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAECONNABORTED || err == WSAECONNRESET)
		{
			IOCP_DEBUG("send error: connection peer closed!");
			((session_iface*)data->m_owner)->close(close_state::cs_connect_peer_close);
			return;
		}

		if (err != WSA_IO_PENDING)
		{
			Clog::error_throw(errors::system, "WSASend error! erron=%d", err);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__IO_SERVICE_IOCP_HPP__