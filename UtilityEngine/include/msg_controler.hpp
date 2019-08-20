/**
* @file msg_controler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_CONTROLER_HPP__
#define __MSG_CONTROLER_HPP__

#include "msg_defines.hpp"
#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task{ class dispatcher_iface; }
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class controler_iface
{
	friend class task::dispatcher_iface;
	virtual void handle_wrap(task::object_iface* obj, std::uint32_t compkey, mem::message* message, void* ptr) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class message_wrap,class handler_manager,class dispatcher>
class controler_wrap : public handler_manager,public controler_iface
{
public:
	friend class task::dispatcher_iface;
	using message_t = message_wrap;
	using dispatcher_t = dispatcher;
public:
	controler_wrap(void) : m_dispatcher(nullptr){}
	~controler_wrap(void) = default;

	controler_wrap(const controler_wrap&) = delete;
	controler_wrap& operator=(const controler_wrap&) = delete;
public:
	void init(dispatcher_t* _dispatcher)
	{
		if (m_dispatcher)
			Clog::error_throw(errors::logic, "controler initialized!");

		m_dispatcher = _dispatcher;
	}

	void post_request(task::object_iface* obj, std::uint32_t compkey, mem::message* message, void* ptr)
	{
		unsigned long len = 0;
		const char* p = nullptr;
		message_t* msg = dynamic_cast<message_t*>(message);

		if (msg)
		{
			if(!msg->comfirm(len))
			{
				if (len)
					obj->handle_error(compkey);

				return;
			}
		}

		m_dispatcher->dispatch({ this, obj, compkey, message, ptr });
	}
private:
	void handle_wrap(task::object_iface* obj, std::uint32_t compkey, mem::message* message, void* ptr)
	{
		message_t* msg = dynamic_cast<message_t*>(message);
		if (!msg)
		{
			obj->do_close(ptr);
			return;
		}

		if (obj->compkey() != compkey)
			return;

		handler_t handle = get_handle(msg);
		if (!handle || handle(obj, message, ptr) != 0)
		{
			obj->handle_error(compkey);
			return;
		}

		msg->commit();
		post_request(obj, compkey, message, ptr);
	}
private:
	dispatcher_t* m_dispatcher;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CONTROLER_HPP__