/**
* @file mem_stream_buffer.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "mem_stream_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
stream_buffer::stream_buffer(void)
	: m_head(nullptr)
	, m_tail(nullptr)
	, m_writer(nullptr)
	, m_reader(nullptr)
#ifndef NDEBUG
	, m_last_malloc(0)
#endif
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
stream_buffer::~stream_buffer(void)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::clear(void)
{
	m_pool.clear();
	
	m_head = m_pool.malloc();
	m_head->m_next = nullptr;
	m_tail = m_head;

	m_writer = m_head->m_buffer;
	m_reader = m_head->m_buffer;

	m_lastread = 0;
	m_readable = 0;
#ifndef NDEBUG
	m_last_malloc = 0;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::init(unsigned long size)
{
	assert(size >= MAX_PACKET_LEN);

	size_t nchunk = size / MAX_PACKET_LEN;
	m_pool.init(size%MAX_PACKET_LEN ? nchunk +1 : nchunk);
	clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long
stream_buffer::readable_size(unsigned long exp)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_lastread = m_readable;
	if (m_lastread < exp) m_lastread = 0;
	return m_lastread;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
stream_buffer::read(unsigned long& size)
{
	size = m_head->m_buffer + MAX_PACKET_LEN - m_reader;
	std::lock_guard<std::mutex> lock(m_mutex);
	size = size > m_readable ? m_readable : size;
	m_lastread = size;
	return m_reader;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::commit_read(unsigned long size)
{
	assert(size <= m_lastread);
	std::lock_guard<std::mutex> lock(m_mutex);
	m_reader += size;
	m_readable -= size;
	if (m_reader >= m_head->m_buffer + MAX_PACKET_LEN) {
		node* tmp = m_head;
		m_head = m_head->m_next;
		m_reader = m_head->m_buffer;
		m_pool.free(tmp);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long
stream_buffer::writable_size(void)
{
	return ULONG_MAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
char*
stream_buffer::write(unsigned long& size)
{
	unsigned long left = m_tail->m_buffer + MAX_PACKET_LEN - m_writer;
	size = size > left ? left : size;
#ifndef NDEBUG
	m_last_malloc = size;
#endif
	return m_writer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
stream_buffer::commit_write(unsigned long size)
{
	std::lock_guard<std::mutex> lock(m_mutex);
#ifndef NDEBUG
	assert(m_last_malloc >= size);
	m_last_malloc = 0;
#endif

	m_writer += size;
	m_readable += size;

	if (m_writer >= m_tail->m_buffer + MAX_PACKET_LEN) {
		m_tail->m_next = m_pool.malloc();
		m_tail = m_tail->m_next;
		m_tail->m_next = nullptr;
		m_writer = m_tail->m_buffer;
	}

	return (m_lastread == 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
stream_buffer::next(unsigned long& size)
{
	size = m_read_limit - m_pos;
	const char* p = read(size);
	m_read_limit -= size;
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility