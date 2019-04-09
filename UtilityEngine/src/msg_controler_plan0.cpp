/**
* @file msg_controler_plan0.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "msg_controler_plan0.hpp"
#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace plan0
{
////////////////////////////////////////////////////////////////////////////////////////////////////
controler::controler(void)
	: m_dispatcher(nullptr)
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
controler::init(dispatch_t* dispatcher)
{
	if (m_dispatcher)
		Clog::error_throw(errors::logic, "controler initialized!");

	m_dispatcher = dispatcher;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
controler::post_request(task::object_iface* obj, std::uint32_t compkey, mem::buffer_iface* buffer, void* ptr)
{
	unsigned long len = 0;
	const char* p = nullptr;

	if (buffer)
	{
		p = m_pares(buffer, len);
		if (!p)
		{
			if (len > MAX_PACKET_LEN)
				obj->handle_error(compkey);

			return;
		}
	}
	
	m_dispatcher->dispatch({ this, compkey, buffer, len, obj, p, ptr });
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
controler::handle_wrap(std::uint32_t compkey, mem::buffer_iface* buffer, unsigned long len,
					   task::object_iface* obj, const char* msg, void* ptr)
{
	if (!buffer)
	{
		obj->do_close(ptr);
		return;
	}

	if (obj->compkey() != compkey)
		return;

	handler_t handle = get_handle();
	if (!handle || handle(obj, msg, ptr) != 0)
	{
		obj->handle_error(compkey);
		return;
	}

	buffer->commit_read(len);
	post_request(obj, compkey, buffer, ptr);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace planA
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 