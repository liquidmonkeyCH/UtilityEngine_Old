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
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
using fd_t = SOCKET;
enum class socket_type { tcp, };

#ifndef _WIN32
using OVERLAPPED = void*;
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