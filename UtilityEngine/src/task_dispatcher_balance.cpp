/**
* @file task_dispatcher_balance.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "task_dispatcher_balance.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
dispatcher_balance::dispatcher_balance(void)
: m_workers(nullptr)
, m_worker_size(0)
, m_running(false)
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
dispatcher_balance::~dispatcher_balance(void)
{
	stop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
dispatcher_balance::stop(void)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_running)
		return;

	for (std::uint32_t i = 0; i < m_worker_size; ++i)
		m_workers[i].safe_stop();

	delete[] m_workers;

	m_running = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void 
dispatcher_balance::start(std::uint32_t nworker)
{
	assert(nworker > 0);
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_running)
		return;

	m_running = true;
	m_workers = new com::task_thread<task_info>[nworker];
	m_worker_size = nworker;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void 
dispatcher_balance::dispatch(task_info&& _task)
{
	std::uint32_t n = _task.m_obj->get_worker_index();
	com::task_thread<task_info>* worker = get_worker(n);
	_task.m_obj->set_worker_index(n);
	worker->schedule(std::move(_task));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
com::task_thread<dispatcher_iface::task_info>*
dispatcher_balance::get_worker(std::uint32_t& n)
{
	if (n > m_worker_size)
		n = 0;

	if (n > 0)
		return m_workers + n - 1;

	com::task_thread<task_info>* worker = m_workers;
	size_t ntask = 0;
	size_t min_task = m_workers->task_size();
	n = 1;
	for (std::uint32_t i = 1; i < m_worker_size; ++i)
	{
		ntask = m_workers[i].task_size();
		if (min_task > ntask)
		{
			min_task = ntask;
			worker = m_workers + i;
			n = i+1;
		}
	}
	return worker;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 