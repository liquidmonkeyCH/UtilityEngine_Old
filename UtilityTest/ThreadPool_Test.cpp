#include "UtilityTest.hpp"
#include "thread_pool.hpp"

using namespace Utility;

void thread_echo(int n)
{
	for (int i = 0; i < 10; ++i)
	{
		if (n % 2 == 0)
		{
			std::this_thread::yield();
			Clog::warn("%d,%lld",n,std::this_thread::get_id());
			continue;
		}
		std::this_thread::yield();
		Clog::info("%d,%lld", n, std::this_thread::get_id());
	}
}

void thread_listen(threadpool* pool, int n)
{
	while (true)
	{
		Clog::info("working:%d Tasks:%d", pool->work_size(), pool->task_size());
		
		if (pool->work_size() == n && pool->task_size() == 0)
			return;
			

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void thread_loop()
{
	while (true)
	{
		std::this_thread::yield();
	}
}

class TestThread
{
public:
	void Stop(void)
	{
		m_bRun = false;
		cv.notify_one();
		m_thread.safe_stop();
	}

	void Start(void)
	{
		m_bRun = true;
		m_thread.schedule_normal(&TestThread::run, this);
		//m_Dispatcher.schedule(&TestThread::run2, this);
	}
	
	void run(void)
	{
		Clog::info("I'm 1 running!");
		if (!m_bRun.load())
			return;

		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::this_thread::yield();
		m_thread.schedule_normal(&TestThread::run, this);
	}

	void run2(void)
	{
		Clog::info("I'm 2 running!");

		std::unique_lock<std::mutex> lock(_mtu);
		cv.wait(lock);

		Clog::info("I'm 2 unlock!");
		if (!m_bRun.load())
			return;

		//std::this_thread::sleep_for(std::chrono::seconds(1));
		std::this_thread::yield();
		//m_Dispatcher.schedule(&TestThread::run2, this);
	}
private:
	thread m_thread;
	std::atomic_bool m_bRun;
	std::mutex _mtu;
	std::condition_variable cv;
};

class test
{
public:
	static void run(){ Clog::info("test run"); }

};


void UtilityTest::_ThreadPool()
{
	std::cout << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl << "// ThreadPool " << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl;

	thread_pool<20> pool3;
	thread_pool<2> pool4;
	threadpool pool; pool.init(20);
	threadpool pool2; pool2.init(2);

	task_thread<test> task_pool;

	TestThread th;
	th.Start();

	pool.schedule_normal(thread_listen, &pool, 1);
	pool.schedule_normal(thread_echo,1);
	pool.schedule_normal(thread_echo, 2);
	pool.schedule_normal(thread_echo, 3);
	pool.schedule_normal(thread_echo, 4);
	pool.schedule_normal(thread_echo, 5);
	pool.schedule_future(thread_echo, 1);
	pool.schedule_normal(thread_echo, 6);
	
	pool2.schedule_normal(thread_listen, &pool2, 1);
	pool2.schedule_normal(thread_echo, 1);
	pool2.schedule_normal(thread_echo, 2);
	pool2.schedule_normal(thread_echo, 3);
	pool2.schedule_normal(thread_echo, 4);
	pool2.schedule_normal(thread_echo, 5);

	pool3.schedule_normal(thread_echo, 6);
	pool4.schedule_normal(thread_echo, 7);

	task_pool.schedule(test());
	task_pool.schedule_normal(thread_echo, 100);

	thread_listen(&pool, 0);

	pool.safe_stop();
	pool2.safe_stop();
	pool3.safe_stop();
	pool4.safe_stop();
	task_pool.safe_stop();

	th.Stop();

	Clog::info("hahaha");
}