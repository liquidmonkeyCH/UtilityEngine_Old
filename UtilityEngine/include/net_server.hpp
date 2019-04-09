/**
* @file net_server.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_SERVER_HPP__
#define __NET_SERVER_HPP__

#include "mem_container.hpp"
#include "net_session.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class server_iface : public framework
{
public:
	server_iface(void) :m_socket(nullptr), m_io_service(nullptr), m_running({ false }){}
	virtual ~server_iface(void) = default;

	server_iface(const server_iface&) = delete;
	server_iface& operator=(const server_iface&) = delete;

	friend class io_service_iface;
public:
	fd_t get_fd(void){ return m_socket->get_fd(); }
	socket_iface* get_socket(void){ return m_socket; }
	bool is_running(void){ return m_running; }
protected:
	virtual void process_accept(per_io_data*, sockaddr_storage*, session_iface**) = 0;
	virtual accept_data* get_accept_data(void) = 0;
protected:
	socket_iface* m_socket;
	io_service_iface* m_io_service;
	std::atomic_bool m_running;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t,class control_t>
class server_wrap : public server_iface
{
public:
	server_wrap(void){ m_socket = &m_socket_impl; }
	virtual ~server_wrap(void) = default;

	server_wrap(const server_wrap&) = delete;
	server_iface& operator=(const server_wrap&) = delete;

	using sokcet_mode = typename session_t::socket_mode;
	using dispatch_t = typename control_t::dispatch_t;
public:
	void init(size_t max_session, io_service_iface* io_service, dispatch_t* dispatcher);

	void start(const char* host, std::uint32_t port);
	void stop(void);
private:
	void process_accept(per_io_data*, sockaddr_storage*, session_iface**);
	accept_data* get_accept_data(void);
	void post_request(session_iface* session, mem::buffer_iface* buffer,void* ptr);
	void on_close_session(session_iface* session);
protected:
	virtual void on_start(void){}
	virtual void on_stop(void){}
protected:
	sokcet_mode					m_socket_impl;
	mem::container<session_t>	m_pool;
	mem::container<accept_data>	m_accept_data;
	std::mutex					m_accept_mutex;
	//! for session
	unsigned long				m_read_buffer_size;
	unsigned long				m_send_buffer_size;

	control_t					m_controler;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "net_server.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_SERVER_HPP__