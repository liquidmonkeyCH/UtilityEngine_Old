/**
* @file net_io_service_iocp.hpp
*
* @author Hourui (liquidmonkey)
*/

#ifdef _WIN32
#ifndef __NET_IO_SERVICE_IOCP_HPP__
#define __NET_IO_SERVICE_IOCP_HPP__

#include <thread>
#include <list>
#include "net_io_service.hpp"

#include <MSWSock.h>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class io_service_iocp : public io_service_iface
{
public:
	io_service_iocp(void);
	~io_service_iocp(void);

	io_service_iocp(const io_service_iocp&) = delete;
	io_service_iocp& operator=(const io_service_iocp&) = delete;
public:
	void start(std::uint32_t nthread = 0);
	void stop(void);
private:
	//! after start!
	//! after server socket create!
	void track_server(responder_iface*);
	//! after start!
	//! after session connected!
	void track_session(session_iface*);
private:
	void post_recv_event(per_io_data* data);
	void post_send_event(per_io_data* data);
	void post_accept_event(responder_iface* server, per_io_data* data);
private:
	void create_iocp(void);
	void process_event(void);
	void close(void);
private:
	std::atomic_int m_state;
	std::list<std::thread>		m_threads;

	HANDLE m_hiocp;
	LPFN_ACCEPTEX m_accept;
	LPFN_GETACCEPTEXSOCKADDRS m_accept_addrs;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
using io_service_epoll = io_service_iocp;
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_IO_SERVICE_IOCP_HPP__
#else
#include "net_io_service_epoll.hpp"
#endif // _WIN32