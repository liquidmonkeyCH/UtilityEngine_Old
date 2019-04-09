/**
* @file net_server.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t,class control_t>
void server_wrap<session_t,control_t>::init(size_t max_session, io_service_iface* io_service, dispatch_t* dispatcher)
{
	if (m_io_service)
		Clog::error_throw(errors::logic, "server initialized!");

	m_pool.init(max_session);
	m_accept_data.init(10);

	m_read_buffer_size = MAX_PACKET_LEN * 10;
	m_send_buffer_size = MAX_PACKET_LEN * 10;

	m_io_service = io_service;
	m_controler.init(dispatcher);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void server_wrap<session_t, control_t>::start(const char* host, std::uint32_t port)
{
	bool b_running = false;
	if (!m_running.compare_exchange_strong(b_running, true))
		Clog::error_throw(errors::logic, "server already running!");

	framework::net_init();
	on_start();

	m_socket->bind(host, port);
	m_socket->listen();

	m_running = true;
	
	m_io_service->track_server(this);
	
	for (size_t i = 0; i < m_accept_data.size(); ++i)
	{
		accept_data* _data = m_accept_data.malloc();
		if (!_data) break;	// accept_data empty!

		_data->m_op = io_op::accept;
		_data->m_owner = this;
		_data->m_buffer.buf = _data->m_buff;
		_data->m_buffer.len = sizeof(_data->m_buff);

		m_io_service->post_accept_event(this, _data);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void server_wrap<session_t, control_t>::stop(void)
{
	m_running = false;
	m_socket->close();

	m_accept_data.clear();

	do{
		accept_data* data = m_accept_data.malloc();
		if (!data) break;

		if (data->m_fd != INVALID_SOCKET)
			m_socket->close_fd(data->m_fd);

	} while (true);

	for (mem::container<session_t>::iterator it = m_pool.used_begin(); it != m_pool.used_end(); ++it)
	{
		it->close(session_iface::reason::cs_service_stop);
	}

	m_pool.clear();

	on_stop();
	framework::net_free();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void server_wrap<session_t, control_t>::process_accept(per_io_data* data, sockaddr_storage* addr, session_iface** se)
{
	std::lock_guard<std::mutex> lock(m_accept_mutex);
	session_t* session = m_pool.malloc();
	if (!session)
	{
		m_socket->close_fd(data->m_fd);
	}
	else
	{
		session->set_connected(this,data->m_fd, addr);
		session->init_buffer(m_read_buffer_size, m_send_buffer_size);
		session->m_socket->set_blocking(false);
	}
	*se = session;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void server_wrap<session_t, control_t>::post_request(session_iface* session, mem::buffer_iface* buffer, void* ptr)
{
	m_controler.post_request(session, session->compkey(), buffer, ptr);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void server_wrap<session_t, control_t>::on_close_session(session_iface* session)
{
	if (!m_running)
		return;

	std::lock_guard<std::mutex> lock(m_accept_mutex);
	m_pool.free(dynamic_cast<session_t*>(session));
}
////////////////////////////////////////////////////////////////////////////////////////////////////