/**
* @file net_session.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
session_wrap<st, pares_message_wrap>::session_wrap(void)
{
	m_socket = &m_socket_impl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
session_wrap<st, pares_message_wrap>::~session_wrap(void)
{
	m_socket_impl.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::init_buffer(unsigned long recv_buffer_size, unsigned long send_buffer_size)
{
	m_recv_buffer.init(recv_buffer_size);
	m_send_buffer.init(send_buffer_size);

	m_recv_data.m_buffer.len = MAX_PACKET_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::clear(void)
{
	m_recv_buffer.clear();
	m_send_buffer.clear();

	m_recv_data.m_buffer.len = MAX_PACKET_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::do_close(void* ptr)
{
	on_close(*(reason*)(ptr));
	m_parent->on_close_session(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::send(const char* packet, unsigned long size)
{
	if (m_state != static_cast<int>(state::connected))
		return;

	std::lock_guard<std::mutex> lock(m_send_mutex);

	unsigned long len = m_send_buffer.writable_size();

	if (len < size)
	{
		// buffer overflow 
		close(reason::cs_send_buffer_overflow);
		return;
	}

	char* p = nullptr;
	unsigned long left = size;
	bool b_send = false;
	do{
		len = left;
		p = m_send_buffer.write(len);
		memcpy(p, packet, len);
		b_send = m_send_buffer.commit_write(len);
		packet += len;
		left -= len;
	} while (left != 0);

	if (b_send)
	{
		m_send_data.m_buffer.len = MAX_PACKET_LEN;
		m_send_data.m_buffer.buf = const_cast<char*>(m_send_buffer.read(m_send_data.m_buffer.len));

		if (m_state == static_cast<int>(state::connected))
			m_io_service->post_send_event(&m_send_data);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::process_send(unsigned long size)
{
	if (m_state != static_cast<int>(state::connected))
		return false;

	m_send_buffer.commit_read(size);
	size = MAX_PACKET_LEN;
	const char* p = m_send_buffer.read(size);

	if (size == 0 || m_state != static_cast<int>(state::connected))
		return false;

	m_send_data.m_buffer.buf = const_cast<char*>(p);
	m_send_data.m_buffer.len = size;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::process_recv(unsigned long size)
{
	if (m_state != static_cast<int>(state::connected))
		return false;

	if (m_recv_buffer.commit_write(size))
	{
		std::lock_guard<std::recursive_mutex> lock(m_close_mutex);
		if (m_state != static_cast<int>(state::connected))
			return false;
		m_parent->post_request(this, &m_recv_buffer, nullptr);
	}

	m_recv_data.m_buffer.len = MAX_PACKET_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);

	if (m_recv_data.m_buffer.len == 0)
	{
		close(reason::cs_recv_buffer_overflow);
		return false;
	}

	return (m_state == static_cast<int>(state::connected));
}
////////////////////////////////////////////////////////////////////////////////////////////////////