/**
* @file net_io_service.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_IO_SERVICE_HPP__
#define __NET_IO_SERVICE_HPP__

#include "net_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class session_iface;
class server_iface;
////////////////////////////////////////////////////////////////////////////////////////////////////
class io_service_iface
{
public:
	io_service_iface(void) = default;
	virtual ~io_service_iface(void) = default;

	io_service_iface(const io_service_iface&) = delete;
	io_service_iface& operator=(const io_service_iface&) = delete;

	template<class session_t,class control_t> friend class server_wrap;
	template<socket_type st,class buffer_t> friend class session_wrap;
public:
	virtual void start(std::uint32_t nthread) = 0;

	//! after start!
	//! after server socket create!
	virtual void track_server(server_iface*) = 0;

	//! after start!
	//! after session connected!
	virtual void track_session(session_iface*) = 0;

	virtual void stop(void) = 0;
protected:
	virtual void post_read_event(per_io_data* data) = 0;
	virtual void post_send_event(per_io_data* data) = 0;
	virtual void post_accept_event(server_iface* server, per_io_data* data) = 0;

	// session_iface
	void bind(session_iface* session);
	per_io_data* get_read_data(session_iface* session);
	per_io_data* get_send_data(session_iface* session);
	int process_read(session_iface* session, size_t size);
	int process_send(session_iface* session, size_t size);
	
	// server_iface
	void process_accept(server_iface* server, per_io_data* data, sockaddr_storage* addr, session_iface** session);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_IO_SERVICE_HPP__