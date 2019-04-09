/**
* @file msg_handler_manager_map.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_HANDLER_MANAGER_MAP_HPP__
#define __MSG_HANDLER_MANAGER_MAP_HPP__

#include <unordered_map>
#include "msg_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
class msg_handler_manager_map
{
public:
	using map_t = std::unordered_map <key_t, handler_t>;
public:
	msg_handler_manager_map(void) = default;
	~msg_handler_manager_map(void) = default;

	msg_handler_manager_map(const msg_handler_manager_map&) = delete;
	msg_handler_manager_map& operator=(const msg_handler_manager_map&) = delete;
public:
	handler_t get_handle(key_t key);
	bool attach(key_t key, handler_t handle);
	bool detach(key_t key);
private:
	map_t m_map;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
handler_t msg_handler_manager_map<key_t>::get_handle(key_t key)
{
	typename map_t::iterator it = m_map.find(key);
	if (it != m_map.end())
		return it->second;

	return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
bool msg_handler_manager_map<key_t>::attach(key_t key, handler_t handle)
{
	typename map_t::iterator it = m_map.find(key);
	if (it != m_map.end())
		return false;

	m_map[key] = handle;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
bool msg_handler_manager_map<key_t>::detach(key_t key)
{
	typename map_t::iterator it = m_map.find(key);
	if (it == m_map.end())
		return false;

	m_map.erase(it);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_HANDLER_MANAGER_MAP_HPP__