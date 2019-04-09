/**
* @file task_object.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_OBJECT_HPP__
#define __TASK_OBJECT_HPP__

#include <atomic>
#include <cstdint>
#include <mutex>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class object_iface
{
public:
	object_iface(void);
	~object_iface(void) = default;

	object_iface(const object_iface&) = delete;
	object_iface& operator=(const object_iface&) = delete;

	std::uint32_t compkey(void);
	virtual void handle_error(std::uint32_t compkey) = 0;
	virtual void do_close(void* ptr) = 0;
	
	// for pool mode
	bool add_ref(void);
	void sub_ref(void);
	void wait_ref(void);
	void reset_ref(void);

	// for balance mode
	void set_worker_index(std::uint32_t worker);
	std::uint32_t get_worker_index(void);
protected:
	std::atomic<std::uint32_t> m_compkey;
	std::atomic<std::uint32_t> m_worker_index;
	std::mutex m_ref_mutex;
	std::atomic_int m_ref;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_OBJECT_HPP__