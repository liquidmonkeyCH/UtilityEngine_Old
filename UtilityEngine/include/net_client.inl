/**
* @file net_client.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void client_wrap<session_t, control_t>::init(io_service_iface* io_service, dispatch_t* dispatcher)
{
	if (m_io_service)
		Clog::error_throw(errors::logic, "client initialized!");

	m_io_service = io_service;
	m_controler.init(dispatcher);
	m_recv_buffer_size = MAX_PACKET_LEN * 10;
	m_send_buffer_size = MAX_PACKET_LEN * 10;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
bool client_wrap<session_t, control_t>::start(const char* host, std::uint32_t port, std::uint32_t timeout_msecs)
{
	framework::net_init();
	on_start();
	
	try{
		if (m_session.m_socket->connect(host, port, timeout_msecs)){
			m_session.set_connected(this,INVALID_SOCKET, nullptr);
			m_session.on_connect();
			m_session.init_buffer(m_recv_buffer_size, m_send_buffer_size);
			m_session.m_socket->set_blocking(false);
			m_io_service->track_session(&m_session);
			return true;
		}
	}
	catch (utility_error e){
		Clog::error(e.what());
		throw(e);
	}
	
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void client_wrap<session_t, control_t>::stop(void)
{
	m_session.close(session_iface::reason::cs_service_stop);
	on_stop();
	framework::net_free();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void client_wrap<session_t, control_t>::post_request(session_iface* session, mem::buffer_iface* buffer, void* ptr)
{
	m_controler.post_request(session, session->compkey(), buffer, ptr);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class control_t>
void client_wrap<session_t, control_t>::on_close_session(session_iface* session)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////