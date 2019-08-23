/**
* @file net_requester.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_REQUESTER_HPP__
#define __NET_REQUESTER_HPP__

#include <future>
#include "net_session.hpp"
#include "msg_controler.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class requester_iface : public framework
{
public:
	enum class state{ none, starting, connecting, connected, timeout, stopping };

	requester_iface(void) :m_io_service(nullptr) {}
	virtual ~requester_iface(void) = default;

	requester_iface(const requester_iface&) = delete;
	requester_iface& operator=(const requester_iface&) = delete;
protected:
	io_service_iface*		m_io_service;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
class requester : public requester_iface
{
public:
	requester(void) = default;
	virtual ~requester(void) = default;

	requester(const requester&) = delete;
	requester& operator=(const requester&) = delete;

	using sokcet_mode = typename session_t::socket_mode;
	using dispatcher_t = dispatcher;
	using message_t = typename session_t::message_t;
public:
	void init(io_service_iface* io_service, dispatcher_t* _dispatcher);
	state start(const char* host, std::uint32_t port, std::uint32_t timeout_msecs = 0);
	void stop(void);
	void join(void);

	void send(const char* msg, unsigned long len) { m_session.send(msg,len); }
	bool is_connected(void){ return m_session.is_connected(); }
protected:
	void post_request(session_iface* session, mem::message* msg, void* ptr);
	bool connect(const char* host, std::uint32_t port, std::uint32_t timeout_msecs);

	void on_close_session(session_iface* session);
protected:
	virtual void on_start(void){}
	virtual void on_stop(void){}
	virtual void on_disconnect(void){}
	virtual bool on_connect_retry(const char* host, std::uint32_t port, std::uint32_t timeout_msecs){ return false; }
protected:
	session_t				m_session;
	//! for session
	unsigned long			m_recv_buffer_size;
	unsigned long			m_send_buffer_size;
	//! for msg handle
	msg::controler_wrap< message_t, handler_manager, dispatcher> m_controler;

	std::atomic_int			m_state;
	std::promise<bool>		m_can_stop;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "net_requester.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_REQUESTER_HPP__