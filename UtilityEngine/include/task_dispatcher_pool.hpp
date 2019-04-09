/**
* @file task_dispatcher_pool.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_DISPATCHER_POOL_HPP__
#define __TASK_DISPATCHER_POOL_HPP__

#include "task_dispatcher.hpp"
#include "thread_pool.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher_pool : public dispatcher_iface
{
public:
	dispatcher_pool(void);
	~dispatcher_pool(void);

	dispatcher_pool(const dispatcher_pool&) = delete;
	dispatcher_pool& operator=(const dispatcher_pool&) = delete;
public:
	void start(std::uint32_t nworker);
	void stop(void);
	void dispatch(task_info&& _task);
private:
	task_thread_pool<task_info> m_workers;
	bool m_running;
	std::mutex m_mutex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_DISPATCHER_POOL_HPP__