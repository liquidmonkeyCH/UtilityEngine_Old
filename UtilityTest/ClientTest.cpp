
#include "logger.hpp"

#include <iostream>
#include <sstream>
#include <random>
#include <memory>

#include "net_io_service_iocp.hpp"
#include "msg_controler_plan0.hpp"
#include "mem_rotative_buffer.hpp"
#include "net_client.hpp"

using namespace Utility;

class GameSession : public net::session_wrap < net::socket_type::tcp, mem::rotative_buffer >
{
public:
	void on_connect(void)
	{
		m_socket->set_recv_buffer(8);
	}
};


int handler(task::object_iface* obj, const char* msg, void* ptr)
{
	Clog::info("recv msg: %s", msg);
	std::string str(msg);
	if (str.size() < MAX_PACKET_LEN/2 -10)
		str += msg;

	GameSession* session = dynamic_cast<GameSession*>(obj);
	session->send(str.c_str(), str.size() + 1);
	return 0;
}

class NetClient : public net::client_wrap <GameSession, msg::plan0::controler>
{
public:
	void on_start(void)
	{
		m_recv_buffer_size = 2 * MAX_PACKET_LEN;
		m_send_buffer_size = 2 * MAX_PACKET_LEN;
		m_controler.attach(handler);
	}
};

int main(int argc, char* argv[])
{
	net::framework::net_init();

	logger klogger(logger::log_level::debug);
	Clog::active_logger(&klogger);

	net::io_service_iocp io_service;
	io_service.start();

	NetClient::dispatch_t dispatcher;
	dispatcher.start(2);

	int nCount;
	
	std::cin >> nCount;

	std::list<std::shared_ptr<NetClient>> m_list;

	for (int i = 0; i < nCount; ++i)
	{
		std::shared_ptr<NetClient> client(new NetClient);
		client->init(&io_service, &dispatcher);
		if (client->start("127.0.0.1", 55552,1000) == net::client_iface::state::timeout)
		{
			Clog::debug("connect timeout!");
			continue;
		}

		m_list.push_back(client);
	}
	
	std::string ip;
	std::stringstream str;
	do
	{
	
		std::cin >> ip;
		if (ip.compare("quit") == 0) break;
		int n = 0;

		for (std::list<std::shared_ptr<NetClient>>::iterator it = m_list.begin(); it != m_list.end();)
		{
			if (!(*it)->is_connected())
			{
				m_list.erase(it++);
				continue;
			}
				
			str << ip << " client:" << n++;
			(*it)->send(str.str().c_str(), str.str().length()+1);
			str.str("");
			++it;
		}
		Clog::debug("left client: %u", m_list.size());
	} while (true);


	for (std::shared_ptr<NetClient> client : m_list)
	{
		client->stop();
	}

	for (std::shared_ptr<NetClient> client : m_list)
	{
		client->join();
	}

	m_list.clear();

	io_service.stop();
	dispatcher.stop();

	net::framework::net_free();

	system("pause");

	return 0;
}

