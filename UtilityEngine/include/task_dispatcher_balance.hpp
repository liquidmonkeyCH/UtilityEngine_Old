/**
* @file task_dispatcher_balance.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_DISPATCHER_BALANCE_HPP__
#define __TASK_DISPATCHER_BALANCE_HPP__

#include "task_dispatcher.hpp"
#include "com_thread_pool.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher_balance : public dispatcher_iface
{
public:
	dispatcher_balance(void);
	~dispatcher_balance(void);

	dispatcher_balance(const dispatcher_balance&) = delete;
	dispatcher_balance& operator=(const dispatcher_balance&) = delete;
public:
	void start(std::uint32_t nworker);
	void stop(void);
	void dispatch(task_info&& _task);
private:
	com::task_thread<task_info>* get_worker(std::uint32_t& n);
private:
	com::task_thread<task_info>* m_workers;
	std::uint32_t m_worker_size;
	bool m_running;
	std::mutex m_mutex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_DISPATCHER_BALANCE_HPP__