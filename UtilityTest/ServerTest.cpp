
#include <deque>
#include <map>
#include <iostream>
#include <chrono>

#include "logger.hpp"
#include "net_io_service_iocp.hpp"
#include "net_responder.hpp"

#include "mem_rotative_buffer.hpp"
#include "mem_stream_buffer.hpp"
#include "msg_message.hpp"

#include "msg_handler_manager_map.hpp"
#include "msg_handler_manager_deque.hpp"

#include "task_dispatcher_balance.hpp"

#include "mem_pool.hpp"

using namespace Utility;
class GameSession : public net::session_wrap < net::socket_type::tcp, msg::pares_zero::message_wrap<mem::rotative_buffer,MAX_PACKET_LEN>>
{
public:
	void on_connect(void)
	{
		m_socket->set_send_buffer(8);
		m_socket->set_recv_buffer(8);
		m_socket_impl.set_no_delay(true);
		m_socket_impl.set_keep_alive(true, 1000, 1000, 15);
	}
};

std::atomic_uint64_t total_msg = 0;
int handler(task::object_iface* obj, mem::message* msg, void* ptr)
{
	++total_msg;
	char buffer[MAX_PACKET_LEN + 1];
	GameSession* session = dynamic_cast<GameSession*>(obj);
	unsigned long limit = msg->get_read_limit();
	unsigned long len = 0,size = 0;
	while (true) {
		const char* p = msg->next(len);
		if (!p) break;
		
		memcpy(buffer + size, p, len);
		size += len;
		len = 0;
	}
	
	//Clog::info("recv msg: %s", buffer);
	session->send(buffer, size);
	
	return 0;
}

class GameServer : public net::responder <GameSession, msg::handler_manager_deque, task::dispatcher_balance>
{
public:
	virtual void on_start(void)
	{
		m_max_session = m_session_pool.size();
		m_connected = 0;
		m_last_connect = 0;
		m_max_per_connect = 0;
		m_run = true;
		m_thread = std::thread(std::bind(&GameServer::run, this));
		m_controler.attach(handler);
	}
	virtual void on_stop(void)
	{
		m_run = false;
		m_thread.join();
	}
private:
	void run(void)
	{
		std::uint64_t total = 0;
		std::uint64_t per_msg = 0;
		std::uint64_t max = 0;
		while (m_run)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			m_connected = m_session_pool.used();
			int per = m_connected - m_last_connect;
			m_max_per_connect = m_max_per_connect < per ? per : m_max_per_connect;
			per_msg = total_msg.load();
			total_msg = 0;
			max = max < per_msg ? per_msg : max;
			total += per_msg;
			Clog::debug("total:%lld ,now:%lld/s, max:%lld/s", total, per_msg, max);
			if (per == 0) continue;
			Clog::debug("last connected:%d ,now connected:%d per:%d max:%d", m_last_connect, m_connected, per, m_max_per_connect);
			m_last_connect = m_connected;
		}
	}
private:
	int m_connected;
	std::thread m_thread;
	std::atomic_bool m_run;
	
	int m_max_per_connect;
	int m_last_connect;
	size_t m_max_session;
};

int main(int argc, char* argv[])
{
	logger klogger(logger::log_level::debug);
	Clog::active_logger(&klogger);

	net::io_service_iocp io_service;
	io_service.start();
	GameServer::dispatcher_t dispatcher;
	dispatcher.start(10);

	mem::stream_buffer::pool_t::alloc_cache(100);

	int nCount;
	std::cin >> nCount;
	try{

	do{
		GameServer Server;

		
			Server.init(nCount, &io_service, &dispatcher);
			Server.start( "127.0.0.1", 55552);
		
	
		std::string str;
		do{
			std::cin >> str;
			if (str.compare("exit") == 0) break;
			if (str.compare("quit") == 0) break;
		} while (true);

		Server.stop();
		if (str.compare("exit") == 0) break;
	} while (true);

	

	io_service.stop();
	dispatcher.stop();

	}
	catch (utility_error e){
		Clog::error("%s:%d", e.what(), e.get_error());
	}

	//net::framework::net_free();

	system("pause");
	return 0;
}

