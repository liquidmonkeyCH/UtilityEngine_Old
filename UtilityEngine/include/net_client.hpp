/**
* @file net_client.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_CLIENT_HPP__
#define __NET_CLIENT_HPP__

#include "net_framework.hpp"
#include "net_io_service.hpp"
#include "net_session.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class client_iface : public framework
{
public:
	client_iface(void) = default;
	virtual ~client_iface(void) = default;

	client_iface(const client_iface&) = delete;
	client_iface& operator=(const client_iface&) = delete;
protected:
	io_service_iface*		m_io_service;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t,class control_t>
class client_wrap : public client_iface
{
public:
	client_wrap(void) = default;
	virtual ~client_wrap(void) = default;

	client_wrap(const client_wrap&) = delete;
	client_wrap& operator=(const client_wrap&) = delete;

	using sokcet_mode = typename session_t::socket_mode;
	using dispatch_t = typename control_t::dispatch_t;
public:
	void init(io_service_iface* io_service, dispatch_t* dispatcher);
	bool start(const char* host, std::uint32_t port, std::uint32_t timeout_msecs = 0);
	void stop(void);

	void send(const char* msg, unsigned long len) { m_session.send(msg,len); }
	bool is_connected(void){ return m_session.is_connected(); }
protected:
	void post_request(session_iface* session, mem::buffer_iface* buffer, void* ptr);
	void on_close_session(session_iface* session);
	virtual void on_start(void){}
	virtual void on_stop(void){}
protected:
	session_t				m_session;
	//! for session
	unsigned long			m_read_buffer_size;
	unsigned long			m_send_buffer_size;

	control_t				m_controler;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "net_client.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_CLIENT_HPP__