/**
* @file net_framework.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_FRAME_WORK_HPP__
#define __NET_FRAME_WORK_HPP__

#include "net_defines.hpp"
#include "mem_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class framework
{
public:
	friend class session_iface;
	template<socket_type st, class buffer_t> friend class session_wrap;
public:
	framework(void);
	virtual ~framework(void);

public:
	static void net_init(void);
	static void net_free(void);
protected:
	virtual void on_close_session(session_iface* session) = 0;
	virtual void post_request(session_iface* session, mem::buffer_iface* buffer, void* ptr) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_FRAME_WORK_HPP__