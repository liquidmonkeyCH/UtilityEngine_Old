/**
* @file net_socket.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_SOCKET_HPP__
#define __NET_SOCKET_HPP__

#include "net_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class socket_iface
{
public:
	friend class session_iface;
public:
	socket_iface(void);
	virtual ~socket_iface(void) = default;
public:
	virtual fd_t create_fd(void) = 0;
public:
	bool operator==(const socket_iface& rhs) const;
	bool operator!=(const socket_iface& rhs) const;
public:
	int read(char* buffer, int len);
	int write(const char* buffer, int len);

	bool connect(const char* host, std::uint32_t port, std::uint32_t timeout_msecs = 0);
	void bind(const char* host, std::uint32_t port);
	void listen(int max_conn = SOMAXCONN);
	
	void close(void);
	void close_fd(fd_t& fd);
public:
	const std::string& get_host(void) const;
	std::uint32_t get_port(void) const;
	const fd_t get_fd(void) const;

	//! only for server
	void set_fd(fd_t fd, sockaddr_storage* addr);

	bool is_ipv6(void) const;
	bool set_blocking(bool bflag);
	bool set_send_buffer(std::uint32_t size);
	bool set_read_buffer(std::uint32_t size);
protected:
	void create_socket(void);
	template<class T>
	bool set_opt(int lv,int name, T val){ return setsockopt(m_fd, lv, name, (char*)&val, sizeof(val)) != SOCKET_ERROR; }
protected:
	fd_t m_fd;
	std::string m_host;
	std::uint32_t m_port;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type t = socket_type::tcp>
class socket_wrap : public socket_iface
{
public:
	fd_t create_fd(void);
	bool set_no_delay(bool val);
	bool set_keep_alive(bool on, std::uint32_t idle, std::uint32_t interval, std::uint32_t cnt);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_SOCKET_HPP__