/**
* @file msg_handler_manager_map.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_HANDLER_MANAGER_MAP_HPP__
#define __MSG_HANDLER_MANAGER_MAP_HPP__

#include <unordered_map>
#include "msg_defines.hpp"
#include "mem_message.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
class handler_manager_map
{
public:
	using map_t = std::unordered_map <key_t, handler_t>;
public:
	handler_manager_map(void) = default;
	virtual ~handler_manager_map(void) = default;

	handler_manager_map(const handler_manager_map&) = delete;
	handler_manager_map& operator=(const handler_manager_map&) = delete;
public:
	handler_t get_handle(key_t key);
	handler_t get_handle(mem::message* msg);
	bool attach(key_t key, handler_t handle);
	bool detach(key_t key);
private:
	map_t m_map;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
handler_t handler_manager_map<key_t>::get_handle(key_t key)
{
	typename map_t::iterator it = m_map.find(key);
	if (it != m_map.end())
		return it->second;

	return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
handler_t handler_manager_map<key_t>::get_handle(mem::message* msg)
{
	key_t key;
	unsigned long size = sizeof(key_t);
	const char* p;
	unsigned long len = size;
	std::size_t pos = 0;
	do
	{
		p = msg->next(len);
		if (!p) return nullptr;
		memcpy(&key + pos, p, len);
		pos += len;
		if (pos >= size) break;
		len = size - pos;
	} while (true);

	typename map_t::iterator it = m_map.find(key);
	if (it != m_map.end())
		return it->second;

	return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
bool handler_manager_map<key_t>::attach(key_t key, handler_t handle)
{
	typename map_t::iterator it = m_map.find(key);
	if (it != m_map.end())
		return false;

	m_map[key] = handle;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class key_t>
bool handler_manager_map<key_t>::detach(key_t key)
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