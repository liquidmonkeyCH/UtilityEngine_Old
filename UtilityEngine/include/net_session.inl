/**
* @file net_session.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
session_wrap<st, buffer_t>::session_wrap(void)
{
	m_socket = &m_socket_impl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
session_wrap<st, buffer_t>::~session_wrap(void)
{
	m_socket_impl.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
void session_wrap<st, buffer_t>::init_buffer(unsigned long read_buffer_size, unsigned long send_buffer_size)
{
	m_read_buffer.init(read_buffer_size);
	m_send_buffer.init(send_buffer_size);

	m_read_data.m_buffer.len = MAX_PACKET_LEN;
	m_read_data.m_buffer.buf = m_read_buffer.write(m_read_data.m_buffer.len);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
void session_wrap<st, buffer_t>::clear(void)
{
	m_read_buffer.clear();
	m_send_buffer.clear();

	m_read_data.m_buffer.len = MAX_PACKET_LEN;
	m_read_data.m_buffer.buf = m_read_buffer.write(m_read_data.m_buffer.len);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
void session_wrap<st, buffer_t>::do_close(void* ptr)
{
	on_close(*(reason*)(ptr));

	clear();
	m_parent->on_close_session(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
void session_wrap<st, buffer_t>::send(const char* packet, unsigned long size)
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

		std::lock_guard<std::recursive_mutex> lock(m_close_mutex);
		if (m_state == static_cast<int>(state::connected))
			m_io_service->post_send_event(&m_send_data);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
bool session_wrap<st, buffer_t>::process_send(unsigned long size)
{
	if (m_state != static_cast<int>(state::connected))
		return false;

	m_send_buffer.commit_read(size);
	m_send_data.m_buffer.len = MAX_PACKET_LEN;
	m_send_data.m_buffer.buf = const_cast<char*>(m_send_buffer.read(m_send_data.m_buffer.len));

	std::lock_guard<std::recursive_mutex> lock(m_close_mutex);
	return (m_send_data.m_buffer.len > 0) && (m_state == static_cast<int>(state::connected));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class buffer_t>
bool session_wrap<st, buffer_t>::process_read(unsigned long size)
{
	if (m_state != static_cast<int>(state::connected))
		return false;

	if (m_read_buffer.commit_write(size))
	{
		std::lock_guard<std::recursive_mutex> lock(m_close_mutex);
		if (m_state != static_cast<int>(state::connected))
			return false;
		m_parent->post_request(this, &m_read_buffer, nullptr);
	}

	m_read_data.m_buffer.len = MAX_PACKET_LEN;
	m_read_data.m_buffer.buf = m_read_buffer.write(m_read_data.m_buffer.len);

	if (m_read_data.m_buffer.len == 0)
	{
		close(reason::cs_read_buffer_overflow);
		return false;
	}

	std::lock_guard<std::recursive_mutex> lock(m_close_mutex);
	return (m_state == static_cast<int>(state::connected));
}
////////////////////////////////////////////////////////////////////////////////////////////////////