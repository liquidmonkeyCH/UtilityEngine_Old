/**
* @file net_io_service_epoll.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef _WIN32
#ifndef __NET_IO_SERVICE_EPOLL_HPP__
#define __NET_IO_SERVICE_EPOLL_HPP__

#include <thread>
#include <list>
#include "net_io_service.hpp"
#include <sys/epoll.h> 

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class io_service_epoll : public io_service_iface
{
public:
	io_service_epoll(void);
	~io_service_epoll(void);

	io_service_epoll(const io_service_epoll&) = delete;
	io_service_epoll& operator=(const io_service_epoll&) = delete;
public:
	void start(std::uint32_t nthread = 0);
	void stop(void);
private:
	//! after start!
	//! after server socket create!
	void track_server(server_iface*);
	void untrack_server(server_iface*);
	//! after start!
	//! after session connected!
	void track_session(session_iface*);
private:
	void post_read_event(per_io_data* data);
	void post_send_event(per_io_data* data);
	void post_accept_event(server_iface* server, per_io_data* data);
private:
	void create_epoll(void);
	void process_event(void);
	void close(void);
private:
	std::atomic_int m_state;
	std::list<std::thread>		m_threads;
	
	fd_t m_epoll;
	static const int MAXEVENTS = 1024; 
    struct epoll_event events[MAXEVENTS];
};
////////////////////////////////////////////////////////////////////////////////////////////////////
using io_service_iocp = io_service_epoll;
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_IO_SERVICE_EPOLL_HPP__
#endif // _WIN32