/**
* @file msg_handler_manager_deque.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_HANDLER_MANAGER_DEQUE_HPP__
#define __MSG_HANDLER_MANAGER_DEQUE_HPP__

#include <deque>
#include "msg_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class msg_handler_manager_deque
{
public:
	using map_t = std::deque<handler_t>;
public:
	msg_handler_manager_deque(void) = default;
	~msg_handler_manager_deque(void) = default;

	msg_handler_manager_deque(const msg_handler_manager_deque&) = delete;
	msg_handler_manager_deque& operator=(const msg_handler_manager_deque&) = delete;
public:
	handler_t get_handle(void);
	bool attach(handler_t handle);
	bool detach(void);
private:
	map_t m_map;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
inline handler_t 
msg_handler_manager_deque::get_handle(void)
{
	if (m_map.empty())
		return nullptr;

	return m_map.front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool 
msg_handler_manager_deque::attach(handler_t handle)
{
	m_map.push_back(handle);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool
msg_handler_manager_deque::detach(void)
{
	m_map.pop_front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_HANDLER_MANAGER_DEQUE_HPP__