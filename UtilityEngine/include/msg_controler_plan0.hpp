/**
* @file msg_controler_plan0.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_CONTROLER_PLAN0_HPP__
#define __MSG_CONTROLER_PLAN0_HPP__

#include "msg_controler.hpp"
#include "msg_handler_manager_deque.hpp"
#include "task_dispatcher_balance.hpp"
#include "msg_pares.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace plan0
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class controler : public controler_iface, public msg_handler_manager_deque
{
public:
	controler(void);
	~controler(void) = default;

	controler(const controler&) = delete;
	controler& operator=(const controler&) = delete;

	using dispatch_t = task::dispatcher_balance;
public:
	void init(dispatch_t* dispatcher);

	void post_request(task::object_iface* obj, std::uint32_t compkey, mem::message* buffer, void* ptr);
private:
	void handle_wrap(task::object_iface* obj,  std::uint32_t compkey, mem::message* buffer, void* ptr);

	std::uint32_t& key_pares_t(const char* msg);
	pares_t m_pares = msg::pares_zero;
private:
	dispatch_t* m_dispatcher;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace plan0
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CONTROLER_PLAN0_HPP__