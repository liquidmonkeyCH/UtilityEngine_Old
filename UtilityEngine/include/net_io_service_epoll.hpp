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
#include <sys/epoll.h> 

#include "net_io_service.hpp"
#include "mem_data_factory.hpp"

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
	void track_server(responder_iface*);
	void untrack_server(responder_iface*);
	//! after start!
	//! after session connected!
	void track_session(session_iface*);
	void untrack_session(session_iface*);
private:
	void post_recv_event(per_io_data* data);
	void post_send_event(per_io_data* data);
	void post_accept_event(responder_iface* server, per_io_data* data);
private:
	bool process_send(per_io_data* data,per_io_data* _data = nullptr);
	bool epoll_control(int op,fd_t fd,epoll_event* ev);
private:
	void create_epoll(void);
	void process_event(epoll_event* m_events);
	void close(void);
private:
	std::atomic_int m_state;
	std::list<std::thread>		m_threads;
	bool m_mulit_threads;
	
	fd_t m_epoll;
	static const int MAXEVENTS = 255; 
	struct event_array{ struct epoll_event m_data[MAXEVENTS]; };
	mem::data_factory<event_array> m_events_pool;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
using io_service_iocp = io_service_epoll;
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_IO_SERVICE_EPOLL_HPP__
#else
#include "net_io_service_iocp.hpp"
#endif // _WIN32