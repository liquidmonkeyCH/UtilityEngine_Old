/**
* @file net_responder.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
void responder<session_t, handler_manager, dispatcher>::init(size_t max_session, io_service_iface* io_service, dispatch_t* dispatcher)
{
	if (m_io_service)
		Clog::error_throw(errors::logic, "server initialized!");

	m_session_pool.init(max_session);
	m_accept_data.init(10);

	m_recv_buffer_size = MAX_PACKET_LEN * 10;
	m_send_buffer_size = MAX_PACKET_LEN * 10;

	m_io_service = io_service;
	m_controler.init(dispatcher);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
void responder<session_t, handler_manager, dispatcher>::start(const char* host, std::uint32_t port)
{
	bool exp = false;
	if (!m_running.compare_exchange_strong(exp, true))
		Clog::error_throw(errors::logic, "server already running!");

	framework::net_init();
	on_start();

	m_socket->bind(host, port);
	m_socket->listen();

	m_running = true;
	m_can_stop = std::promise<bool>();
	
	m_io_service->track_server(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
void responder<session_t, handler_manager, dispatcher>::stop(void)
{
	bool exp = true;
	if (!m_running.compare_exchange_strong(exp, false))
		return;

	m_io_service->untrack_server(this);
	m_socket->close();

	for (typename mem::container<accept_data>::iterator it = m_accept_data.used_begin(); it != m_accept_data.used_end(); ++it)
	{
		if (it->m_fd != INVALID_SOCKET)
			m_socket->close_fd(it->m_fd);
	}
	m_accept_data.clear();

	std::future<bool> stop_wait;
	m_session_mutex.lock();

	for (typename mem::container<session_t>::iterator it = m_session_pool.used_begin(); it != m_session_pool.used_end(); ++it)
	{
		if (!stop_wait.valid())
			stop_wait = m_can_stop.get_future();

		it->close(session_iface::reason::cs_service_stop);
	}
	m_session_mutex.unlock();

	// wait all session close!
	if (stop_wait.valid())
		stop_wait.get();

	m_session_pool.clear();

	on_stop();
	framework::net_free();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
accept_data* responder<session_t, handler_manager, dispatcher>::get_accept_data(void)
{
	return m_accept_data.malloc();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
session_t* responder<session_t, handler_manager, dispatcher>::get_session(void)
{
	std::lock_guard<std::mutex> lock(m_session_mutex);
	if (!m_running)
		return nullptr;

	return m_session_pool.malloc();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
void responder<session_t, handler_manager, dispatcher>::process_accept(per_io_data* data, sockaddr_storage* addr, session_iface** se)
{
	session_t* session = get_session();
	if (!session)
	{
		m_socket->close_fd(data->m_fd);
	}
	else
	{
		session->set_connected(this,data->m_fd, addr);
		session->on_connect();
		session->init_buffer(m_recv_buffer_size, m_send_buffer_size);
		session->m_socket->set_blocking(false);
	}
	*se = session;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
void responder<session_t, handler_manager, dispatcher>::post_request(session_iface* session, mem::buffer_iface* buffer, void* ptr)
{
	m_controler.post_request(session, session->compkey(), buffer, ptr);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager, class dispatcher>
void responder<session_t, handler_manager, dispatcher>::on_close_session(session_iface* session)
{
	std::lock_guard<std::mutex> lock(m_session_mutex);
	m_session_pool.free(dynamic_cast<session_t*>(session));

	if (!m_running && m_session_pool.used() == 0)
		m_can_stop.set_value(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////