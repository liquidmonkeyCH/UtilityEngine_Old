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
, m_next(nullptr)
, m_writer(nullptr)
, m_reader(nullptr)
, m_out(nullptr)
, m_readable(0)
, m_lastread(0)
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
	m_next = m_head;

	m_writer = m_head->m_buffer;
	m_reader = m_head->m_buffer;
	m_out = m_reader;

	m_pos = 0;
	m_limit = 0;
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
	unsigned long len = m_head->m_buffer + MAX_PACKET_LEN - m_reader;
	std::lock_guard<std::mutex> lock(m_mutex);
	len = len > m_readable ? m_readable : len;
	size = size > len ? len : size;
	m_lastread = size;
	return m_reader;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
stream_buffer::commit_read(unsigned long size)
{
	assert(size <= m_readable);
	std::lock_guard<std::mutex> lock(m_mutex);
	m_readable -= size;
	unsigned long len = m_head->m_buffer + MAX_PACKET_LEN - m_reader;
	stream_node* tmp;
	do
	{
		if (size >= len)
		{
			size -= len;
			tmp = m_head;
			m_head = m_head->m_next;
			m_reader = m_head->m_buffer;
			m_pool.free(tmp);
			len = m_head == m_tail ? m_writer - m_reader : MAX_PACKET_LEN;
		}
		else
		{
			m_reader += size;
			size = 0;
		}
	} while (size != 0);
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
void
stream_buffer::reset(void)
{
	m_pos = 0;
	m_next = m_head;
	m_out = m_reader;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
stream_buffer::next(unsigned long& size)
{
	unsigned long limit = m_limit > 0 ? m_limit : readable_size(0);

	if (limit <= m_pos)
	{
		size = 0;
		return nullptr;
	}

	unsigned long left = limit - m_pos;
	if (size > left)
	{
		size = left;
		return nullptr;
	}
	
	const char* p = m_out;

	limit = m_next->m_buffer + MAX_PACKET_LEN - m_out;
	limit = limit > left ? left : limit;
	size = (size == 0 || size > limit) ? limit : size;

	m_pos += size;
	m_out += size;
	if (m_out - m_next->m_buffer == MAX_PACKET_LEN)
	{
		m_next = m_next->m_next;
		m_out = m_next->m_buffer;
	}
	
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility