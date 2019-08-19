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
	enum class state{
		none = 0,
		starting,
		running,
		stopping,
	};
public:
	io_service_iface(void) = default;
	virtual ~io_service_iface(void) = default;

	io_service_iface(const io_service_iface&) = delete;
	io_service_iface& operator=(const io_service_iface&) = delete;

	template<class session_t, class handler_manager, class dispatcher> friend class server_wrap;
	template<class session_t, class handler_manager, class dispatcher> friend class client_wrap;
	template<socket_type st,class message_wrap> friend class session_wrap;
	friend class session_iface;
public:
	virtual void start(std::uint32_t nthread) = 0;
	virtual void stop(void) = 0;
protected:
	//! after start!
	//! after server socket create!
	virtual void track_server(server_iface*) = 0;
	virtual void untrack_server(server_iface*){};
	//! after start!
	//! after session connected!
	virtual void track_session(session_iface*) = 0;
	virtual void untrack_session(session_iface*){}
protected:
	virtual void post_send_event(per_io_data* data) = 0;

	// session_iface
	void bind(session_iface* session);
	per_io_data* get_recv_data(session_iface* session);
	per_io_data* get_send_data(session_iface* session);
	bool process_recv(session_iface* session, unsigned long size);
	bool process_send(session_iface* session, unsigned long size);
	
	// server_iface
	void process_accept(server_iface* server, per_io_data* data, sockaddr_storage* addr, session_iface** session);
	accept_data* get_accept_data(server_iface* server);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_IO_SERVICE_HPP__