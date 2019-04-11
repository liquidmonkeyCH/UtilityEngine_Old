/**
* @file net_socket.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "net_socket.hpp"

#ifdef _WIN32
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")  
#endif

//#define SOCKET_LOG

#ifdef SOCKET_LOG
#define SOCKET_DEBUG(fmt,...) NET_DEBUG(fmt,##__VA_ARGS__)
#else
#define SOCKET_DEBUG(fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
socket_iface::socket_iface(void)
: m_fd(INVALID_SOCKET)
, m_host("")
, m_port(0)
{
	SOCKET_DEBUG("create socket_iface");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
socket_iface::operator==(const socket_iface& rhs) const
{
	return rhs.m_fd == m_fd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
socket_iface::operator!=(const socket_iface& rhs) const
{
	return !operator==(rhs);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string&
socket_iface::get_host(void) const
{
	return m_host;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
std::uint32_t
socket_iface::get_port(void) const
{
	return m_port;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const fd_t
socket_iface::get_fd(void) const
{
	return m_fd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
socket_iface::set_fd(fd_t fd, sockaddr_storage* ss)
{
	m_fd = fd;

	if (!ss)
		return;

	//! ipv6
	if (ss->ss_family == AF_INET6) {
		struct sockaddr_in6* addr6 = reinterpret_cast<struct sockaddr_in6*>(ss);
		char buf[INET6_ADDRSTRLEN] = {};
		m_host = std::string("[") + ::inet_ntop(ss->ss_family, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN) + "]";
		m_port = ntohs(addr6->sin6_port);
	}
	//! ipv4
	else {
		struct sockaddr_in* addr4 = reinterpret_cast<struct sockaddr_in*>(ss);
		char buf[INET_ADDRSTRLEN] = {};
		m_host = ::inet_ntop(ss->ss_family, &addr4->sin_addr, buf, INET_ADDRSTRLEN);
		m_port = ntohs(addr4->sin_port);
	}

	SOCKET_DEBUG("connected from ip:%s port%d", m_host.c_str(), m_port);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
socket_iface::is_ipv6(void) const
{
	return m_host.find(':') != std::string::npos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
socket_iface::set_blocking(bool bflag)
{
	SOCKET_DEBUG("set socket %s", bflag ? "blocking" : "non-blocking");

	u_long mode = bflag ? 0 : 1;
	return ioctlsocket(m_fd, FIONBIO, &mode) == 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int
socket_iface::read(char* buffer, int len)
{
	if (m_fd == INVALID_SOCKET) {
		Clog::error_throw(errors::logic, "read invalid socket!");
	}

	return recv(m_fd, buffer, len, 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int
socket_iface::write(const char* buffer, int len)
{
	if (m_fd == INVALID_SOCKET) {
		Clog::error_throw(errors::logic, "write invalid socket!");
	}

	return send(m_fd, buffer, len, 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
socket_iface::create_socket(void)
{
	if (m_fd != INVALID_SOCKET) {
		Clog::error_throw(errors::logic, "socket in use!");
	}

	m_fd = create_fd();

	SOCKET_DEBUG("create socket fd=%u", m_fd);

	if (m_fd == INVALID_SOCKET){
		Clog::error_throw(errors::system, "socket() failure!");
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
socket_iface::connect(const char* host, std::uint32_t port, std::uint32_t timeout_msecs)
{
	m_host = host;
	port = port;

	create_socket();

	struct sockaddr_storage ss;
	socklen_t addr_len;

	//! 0-init addr info struct
	memset(&ss, 0, sizeof(ss));

	if (is_ipv6()) {
		SOCKET_DEBUG("connect ipv6! host=%s port=%d", host, port);
		//! init sockaddr_in6 struct
		struct sockaddr_in6* addr = reinterpret_cast<struct sockaddr_in6*>(&ss);
		//! convert addr
		if (::inet_pton(AF_INET6, host, &addr->sin6_addr) < 0) {
			close();
			Clog::error_throw(errors::system, "inet_pton() failure");
		}
		//! remaining fields
		ss.ss_family = AF_INET6;
		addr->sin6_port = htons(port);
		addr_len = sizeof(*addr);
	}
	else {
		SOCKET_DEBUG("connect ipv4! host=%s port=%d", host, port);
		struct addrinfo* result = nullptr;
		struct addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_family = AF_INET;

		//! resolve DNS
		if (getaddrinfo(host, nullptr, &hints, &result) != 0) {
			close();
			Clog::error_throw(errors::system, "getaddrinfo() failure");
		}

		//! init sockaddr_in struct
		struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(&ss);
		//! host
		addr->sin_addr = ((struct sockaddr_in*) (result->ai_addr))->sin_addr;
		//! Remaining fields
		addr->sin_port = htons(port);
		ss.ss_family = AF_INET;
		addr_len = sizeof(*addr);

		freeaddrinfo(result);
	}

	if (timeout_msecs > 0) {
		//! for timeout connection handling:
		//!  1. set socket to non blocking
		//!  2. connect
		//!  3. poll select
		//!  4. check connection status
		if (!set_blocking(false)) {
			close();
			Clog::error_throw(errors::system, "connect() set non-blocking failure");
		}
	}
	else {
		//! For no timeout case, still make sure that the socket is in blocking mode
		//! As reported in #32, this might not be the case on some OS
		if (!set_blocking(true)) {
			close();
			Clog::error_throw(errors::system, "connect() set blocking failure");
		}
	}

	int conn_ret = ::connect(m_fd, reinterpret_cast<const struct sockaddr*>(&ss), addr_len);
	if (conn_ret == 0) { SOCKET_DEBUG("connect success!"); return true; }
	if (conn_ret == SOCKET_ERROR){
		int error_code = WSAGetLastError();
		SOCKET_DEBUG("do connect host=%s port=%d res=%d errno=%d", host, port, conn_ret, error_code);
		if (error_code == WSAETIMEDOUT)
		{
			close();
			Clog::warn("connect() timed out errno=%d", error_code);
			return false;
		}

		if (error_code != WSAEWOULDBLOCK && error_code != WSAEALREADY && error_code != WSAEINPROGRESS && error_code != 0){
			close();
			Clog::error_throw(errors::system, "connect() failure, errno=%d", error_code);
		}
	}

	if (timeout_msecs > 0) {
		SOCKET_DEBUG("test connect timeout in %u msecs", timeout_msecs);
		timeval tv;
		tv.tv_sec = (timeout_msecs / 1000);
		tv.tv_usec = ((timeout_msecs - (tv.tv_sec * 1000)) * 1000);

		fd_set set;
		FD_ZERO(&set);
		FD_SET(m_fd, &set);

		//! 1 means we are connected.
		//! 0 means a timeout.
		do{
			int ret = select(static_cast<int>(m_fd)+1, NULL, &set, NULL, &tv);
			SOCKET_DEBUG("do select result=%d", ret);
			if (ret < 0){
				int error_code = WSAGetLastError();
				if (error_code == WSAEINTR) continue;

				close();
				Clog::error_throw(errors::system, "connect() failure, errno=%d", error_code);
			}

			if (ret == 0) {
				close();
				Clog::warn("connect() timed out");
				return false;
			}

			//! Make sure there are no async connection errors
			int err = 0;
			socklen_t len = sizeof(err);
			if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &len) == -1 || err != 0) {
				close();
				Clog::error_throw(errors::system, "connect() failure");
			}

			//! Set back to blocking mode as the user of this class is expecting
			if (!set_blocking(true)) {
				close();
				Clog::error_throw(errors::system, "connect() set blocking failure");
			}

			break;

		} while (true);
	}

	SOCKET_DEBUG("connect success!");
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
socket_iface::bind(const char* host, std::uint32_t port)
{
	m_host = host;
	m_port = port;
	create_socket();

	struct sockaddr_storage ss;
	socklen_t addr_len;

	//! 0-init addr info struct
	memset(&ss, 0, sizeof(ss));

	if (is_ipv6())
	{
		//! init sockaddr_in6 struct
		struct sockaddr_in6* addr = reinterpret_cast<struct sockaddr_in6*>(&ss);
		//! convert addr
		if (::inet_pton(AF_INET6, m_host.c_str(), &addr->sin6_addr) < 0) {
			Clog::error_throw(errors::system, "inet_pton() failure!");
		}
		//! remaining fields
		addr->sin6_port = htons(m_port);
		ss.ss_family = AF_INET6;
		addr_len = sizeof(*addr);
	}
	else
	{
		struct addrinfo* result = nullptr;

		//! dns resolution
		if (getaddrinfo(m_host.c_str(), nullptr, nullptr, &result) != 0) {
			Clog::error_throw(errors::system, "getaddrinfo() failure!");
		}

		//! init sockaddr_in struct
		struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(&ss);
		//! addr
		addr->sin_addr = ((struct sockaddr_in*) (result->ai_addr))->sin_addr;
		//! remaining fields
		addr->sin_port = htons(m_port);
		ss.ss_family = AF_INET;
		addr_len = sizeof(*addr);

		freeaddrinfo(result);
	}

	if (::bind(m_fd, reinterpret_cast<const struct sockaddr*>(&ss), addr_len) == SOCKET_ERROR) {
		Clog::error_throw(errors::system, "bind() failure!");
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
socket_iface::listen(int max_conn)
{
	if (::listen(m_fd, max_conn) == SOCKET_ERROR){
		Clog::error_throw(errors::system, "listen() failure! errno=%d", WSAGetLastError());
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
socket_iface::close_fd(fd_t& fd)
{
	if(fd == INVALID_SOCKET) { return; }

	SOCKET_DEBUG("close socket");
	shutdown(fd, SD_BOTH);
	closesocket(fd);
	fd = INVALID_SOCKET;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
socket_iface::close(void)
{
	close_fd(m_fd);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<>
fd_t socket_wrap<socket_type::tcp>::create_fd(void)
{
	ADDRESS_FAMILY family = is_ipv6() ? AF_INET6 : AF_INET;
	return socket(family, SOCK_STREAM, 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 