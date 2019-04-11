/**
* @file net_session.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_SESSION_HPP__
#define __NET_SESSION_HPP__

#include <mutex>
#include "net_socket.hpp"
#include "task_object.hpp"
#include "net_framework.hpp"
#include "net_io_service.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class session_iface : public task::object_iface
{
public:
	enum class reason{ 
		cs_service_stop,
		cs_connect_timeout,
		cs_connect_peer_close,
		cs_send_buffer_overflow,
		cs_read_buffer_overflow,
		cs_handle_error
	};
	enum class state{ none,connected,closing };
	friend class io_service_iface;
	template<class session_t,class control_t> friend class server_wrap;
	template<class session_t, class control_t> friend class client_wrap;
public:
	session_iface(void);
	virtual ~session_iface(void) = default;
public:
	fd_t get_fd(void);
	socket_iface* get_socket(void);
	void close(reason);
	bool is_connected(void);
protected:
	void set_connected(framework* parent,fd_t fd, sockaddr_storage* addr);
	void handle_error(std::uint32_t compkey);
	//! for wrap
	virtual bool process_read(unsigned long size) = 0;
	virtual bool process_send(unsigned long size) = 0;
protected:
	io_service_iface* m_io_service;
	framework* m_parent;
	std::atomic_int m_state;

	socket_iface* m_socket;

	per_io_data m_read_data;
	per_io_data m_send_data;

	std::recursive_mutex m_close_mutex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
class session_wrap : public session_iface
{
public:
	using socket_mode = socket_wrap<st> ;
	template<class session_t, class control_t> friend class server_wrap;
	template<class session_t, class control_t> friend class client_wrap;
public:
	session_wrap(void);
	virtual ~session_wrap(void);
protected:
	void init_buffer(unsigned long read_buffer_size, unsigned long send_buffer_size);

	void clear(void);
	void do_close(void*);
	virtual void on_close(reason){}
	virtual void on_connect(void){}

	bool process_read(unsigned long size);
	bool process_send(unsigned long size);
public:
	void send(const char* packet, unsigned long size);
protected:
	socket_wrap<st> m_socket_impl;
	buffer_t m_read_buffer;
	buffer_t m_send_buffer;
	std::mutex m_send_mutex;
};
#include "net_session.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_SESSION_HPP__