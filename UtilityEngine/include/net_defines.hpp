/**
* @file net_defines.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_DEFINES_HPP__
#define __NET_DEFINES_HPP__


#include <string>
#include <cstdint>

#include "base_defines.hpp"

#define NET_LOG
#ifdef NET_LOG
	#include "logger.hpp"
	#define NET_DEBUG(fmt,...) Clog::debug(fmt,__VA_ARGS__);
#else
	#define NET_DEBUG(fmt,...)
#endif

#ifdef _WIN32
	#include <WinSock2.h>
#else
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <sys/select.h> 
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class socket_type { tcp, };

#ifdef _WIN32
	using fd_t = SOCKET;
#else
	using fd_t = int;
	using OVERLAPPED = void*;

	struct WSABUF
	{
		unsigned long len;
		char* buf;
	};
	
	int WSAGetLastError(){ return errno; }
	
	#define ioctlsocket ioctl
	#define INVALID_SOCKET (-1)
	#define SOCKET_ERROR (-1)
	
	#define WSAETIMEDOUT ETIMEDOUT
	#define WSAEWOULDBLOCK EWOULDBLOCK 
	#define WSAEALREADY EALREADY
	#define WSAEINPROGRESS EINPROGRESS
#endif

enum class io_op{ read, send, accept };
struct per_io_data
{
	OVERLAPPED m_ol;                 // ÖØµþ½á¹¹
	WSABUF m_buffer;
	io_op m_op;
	void* m_owner;
	fd_t m_fd;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_DEFINES_HPP__