/**
* @file ThreadPool.hpp
*
* 线程池@
* @author Hourui (liquidmonkey)
*/
#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <assert.h>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
class task_wrap : public T
{
public:
	using func_t = std::function < void() >;
	task_wrap(void) :m_valid(false){}
	task_wrap(const func_t&& fun) :m_simple_call(std::move(fun)), m_valid(true){}
	task_wrap(T&& _task) :T(std::move(_task)), m_valid(true){}

	void operator()()
	{
		if (m_simple_call)
		{
			m_simple_call();
			return;
		}
		T::run();
	}

	operator bool()
	{
		return m_valid;
	}
private:
	func_t m_simple_call;
	bool   m_valid;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
class thread_pool_wrap
{
protected:
	using func_t = std::function < void() >;
	using task_t = typename std::conditional<std::is_same<T, func_t>::value, T, task_wrap<T>>::type;
	enum class state :unsigned char{ none, stop, start };
protected:
	std::thread* pool = nullptr;
	std::queue<task_t> tasks;
	std::mutex mtx;
	std::condition_variable cv;
	size_t m_size = 0;
	std::atomic_size_t m_working = { 0 };
	std::atomic_size_t m_ntasks = { 0 };
	state m_state = state::none;
public:
	thread_pool_wrap(void){}
	~thread_pool_wrap(void){}
public:
	bool schedule(task_t&& task)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (m_state != state::start)
			return false;

		tasks.emplace(std::move(task));
		lock.unlock();

		++m_ntasks;
		cv.notify_one();
		return true;
	}

	template<class F, class... Args>
	// 提交任务 返回std::future<ResultType> .valid()判断是否提交成功 .get()阻塞等待结果返回.
	auto schedule_future(F&& f, Args&&... args)->std::future<decltype(std::bind(std::forward<F>(f), std::forward<Args>(args)...)())>
	{
		using ResType = decltype(std::bind(std::forward<F>(f), std::forward<Args>(args)...)());
		std::future<ResType> future;

		auto task = std::make_shared<std::packaged_task<ResType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		if (schedule([task](){ (*task)(); }))
			future = task->get_future();

		return future;
	}

	template<class F, class... Args>
	// 提交任务 返回是否提交成功
	bool schedule_normal(F&& f, Args&&... args)
	{
		return schedule(func_t(std::bind(std::forward<F>(f), std::forward<Args>(args)...)));
	}

	// 关闭任务提交 等待剩余任务完成 (调用时请确保pool不会析构)
	bool safe_stop(void)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (m_state != state::start) return false;
		m_state = state::stop;
		if (tasks.empty())
		{
			lock.unlock();
			_destory();
			return true;
		}

		auto task = std::make_shared<std::packaged_task<bool()>>([](){ return true; });
		tasks.emplace([task](){ (*task)(); });

		lock.unlock();

		++m_ntasks;
		cv.notify_one();

		task->get_future().get();
		_destory();

		return true;
	}
	// 返回当前线程池线程总数
	size_t size(void) { return m_size; }
	size_t work_size(void) { return m_working; }
	size_t task_size(void) { return m_ntasks; }
private:
	task_t get_task(size_t i)
	{
		std::unique_lock<std::mutex> lock(mtx);
		while (tasks.empty() && m_state != state::none)
			cv.wait(lock);

		++m_working;
		if (m_state == state::none)
			return task_t();

		task_t task(std::move(tasks.front()));
		tasks.pop();

		lock.unlock();

		--m_ntasks;
		return task;
	}

	void _run(size_t i)
	{
		++m_working;
		while (true)
		{
			--m_working;
			if (task_t task = get_task(i))
				task();
			else
				break;
		}
	}
protected:
	void _init(void)
	{
		for (size_t i = 0; i < m_size; ++i)
			pool[i] = std::thread(&thread_pool_wrap::_run, this, i);

		std::lock_guard<std::mutex> lock(mtx);
		m_state = state::start;
	}

	void _destory(void)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (m_state == state::none)
			return;

		m_state = state::none;
		lock.unlock();

		cv.notify_all();

		for (size_t i = 0; i < m_size; ++i)
			pool[i].join();

	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t N, class T = std::function<void()>>
class thread_pool : public thread_pool_wrap < T >
{
private:
	std::thread _pool[N];
	using base = thread_pool_wrap < T >;
public:
	~thread_pool(void){ this->_destory(); this->pool = nullptr; }
	thread_pool(void)
	{
		this->pool = _pool;
		this->m_size = N;
		this->m_state = base::state::stop;
		this->_init();
	}

	thread_pool(const thread_pool&) = delete;
	thread_pool& operator=(const thread_pool&) = delete;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
class thread_pool<0, T> : public thread_pool_wrap < T >
{
public:
	using base = thread_pool_wrap < T >;
	~thread_pool(void){ this->_destory(); delete[] this->pool; this->pool = nullptr; }
	thread_pool(void){}
	thread_pool(size_t size)
	{
		assert(size != 0);
		this->m_state = base::state::stop;
		this->m_size = size;
		this->pool = new std::thread[size];
		this->_init();
	}

	thread_pool(const thread_pool&) = delete;
	thread_pool<0>& operator=(const thread_pool&) = delete;

	void init(size_t size)
	{
		assert(size != 0);
		std::unique_lock<std::mutex> lock(this->mtx);
		if (this->m_state != base::state::none) return;
		this->m_state = base::state::stop;
		lock.unlock();
		this->m_size = size;
		this->pool = new std::thread[size];
		this->_init();
	}
};
using threadpool = thread_pool < 0 >;
using thread = thread_pool < 1 >;
template<class T> class task_thread_pool : public thread_pool < 0, T >{};
template<class T> class task_thread : public thread_pool < 1, T >{};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif //__THREAD_POOL_HPP__ 