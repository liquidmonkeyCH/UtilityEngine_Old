/**
* @file net_responder.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_RESPONDER_HPP__
#define __NET_RESPONDER_HPP__

#include <future>
#include "mem_container.hpp"
#include "net_session.hpp"
#include "msg_controler.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class responder_iface : public framework
{
public:
	responder_iface(void) :m_socket(nullptr), m_io_service(nullptr), m_running({ false }){}
	virtual ~responder_iface(void) = default;

	responder_iface(const responder_iface&) = delete;
	responder_iface& operator=(const responder_iface&) = delete;

	friend class io_service_iface;
public:
	fd_t get_fd(void){ return m_socket->get_fd(); }
	socket_iface* get_socket(void){ return m_socket; }
	bool is_running(void){ return m_running; }
	virtual void stop(void) = 0;
protected:
	virtual void process_accept(per_io_data*, sockaddr_storage*, session_iface**) = 0;
	virtual accept_data* get_accept_data(void) = 0;
protected:
	socket_iface* m_socket;
	io_service_iface* m_io_service;
	std::atomic_bool m_running;
	std::promise<bool> m_can_stop;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
class responder : public responder_iface
{
public:
	responder(void){ m_socket = &m_socket_impl; }
	virtual ~responder(void) = default;

	responder(const responder&) = delete;
	responder_iface& operator=(const responder&) = delete;

	using sokcet_mode = typename session_t::socket_mode;
	using dispatcher_t = dispatcher;
	using message_t = typename session_t::message_t;
public:
	void init(size_t max_session, io_service_iface* io_service, dispatcher_t* _dispatcher);

	void start(const char* host, std::uint32_t port);
	void stop(void);
private:
	void process_accept(per_io_data*, sockaddr_storage*, session_iface**);
	accept_data* get_accept_data(void);
	void post_request(session_iface* session, mem::message* msg,void* ptr);
	session_t* get_session(void);
	void on_close_session(session_iface* session);
protected:
	virtual void on_start(void){}
	virtual void on_stop(void){}
protected:
	sokcet_mode					m_socket_impl;
	mem::container<accept_data>	m_accept_data;
	mem::container<session_t>	m_session_pool;
	std::mutex					m_session_mutex;
	//! for session
	unsigned long				m_recv_buffer_size = MAX_PACKET_LEN * 10;
	unsigned long				m_send_buffer_size = MAX_PACKET_LEN * 10;
	//! for hanlder
	msg::controler_wrap< message_t, handler_manager, dispatcher> m_controler;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "net_responder.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_RESPONDER_HPP__