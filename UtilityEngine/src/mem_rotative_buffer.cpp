/**
* @file mem_rotative_buffer.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "base_defines.hpp"
#include "mem_rotative_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
rotative_buffer::rotative_buffer(void)
: m_buffer(nullptr)
, m_writer(nullptr)
, m_reader(nullptr)
, m_final(nullptr)
, m_lastread(0)
, m_lastcopy(0)
, m_size(0)
#ifndef NDEBUG
, m_last_malloc(0)
#endif
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
rotative_buffer::~rotative_buffer(void)
{
	delete[] m_buffer;
	m_buffer = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
rotative_buffer::clear(void)
{
	m_lastcopy = 0;
	m_lastread = 0;
	m_writer = m_buffer;
	m_reader = m_buffer;
	m_final = m_buffer + m_size;
#ifndef NDEBUG
	m_last_malloc = 0;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
rotative_buffer::init(unsigned long size)
{
	assert(size >= MAX_PACKET_LEN);
	if (m_buffer != nullptr)
	{
		if (size == m_size)
			return;

		delete[] m_buffer;
	}

	m_buffer = new char[size+MAX_PACKET_LEN+1];
	m_buffer[size + MAX_PACKET_LEN] = 0;
	m_size = size;
	clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long
rotative_buffer::readable_size(unsigned long exp)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	//! ¡ý******¡ý*******************¡ý******¡ý**********¡ý
	//! head reader             writer  final       tail
	m_lastread = (m_writer >= m_reader) ? m_writer - m_reader : m_size - (m_reader - m_writer);
	if (m_lastread < exp) m_lastread = 0;
	return m_lastread;
	//!	|   len	  |                   |    read     |
	//! | copy |                      | left | copy |
	//! ¡ý*********¡ý*******************¡ý******¡ý**********¡ý
	//! head     writer            reader  final       tail
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
rotative_buffer::read(unsigned long& size)
{
	if (size > MAX_PACKET_LEN || size == 0) 
		size = MAX_PACKET_LEN;

	std::lock_guard<std::mutex> lock(m_mutex);
	//! ¡ý******¡ý*******************¡ý******¡ý**********¡ý
	//! head  reader			   writer  final     tail
	if (m_writer >= m_reader){
		m_lastread = m_writer - m_reader;
		(m_lastread > size) ? (m_lastread = size) : (size = m_lastread);
		return m_reader;
	}
	//!	|   len	  |                   |    read     |
	//! | copy |                      | left | copy |
	//! ¡ý*********¡ý*******************¡ý******¡ý**********¡ý
	//! head     writer            reader  final       tail
	size_t left = m_final - m_reader;
	m_lastread = left + m_lastcopy;
	if (size <= m_lastread){
		m_lastread = size;
		return m_reader;
	}
	
	left = (m_writer - m_buffer) + left;
	if (left < size)
	{
		size = m_lastread;
		return m_reader;
	}
		
	//! copy
	left = size - m_lastread;
	memcpy(m_final + m_lastcopy, m_buffer + m_lastcopy, left);
	m_lastcopy += left;
	m_lastread = size;

	return m_reader;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
rotative_buffer::commit_read(unsigned long size)
{
	assert(size <= m_lastread);
	std::lock_guard<std::mutex> lock(m_mutex);
	m_reader += size;
	//! reader touch finial
	//!	                        |   size  |
	//! ¡ý******¡ý*******************¡ý******¡ý**********¡ý
	//! head  writer			  final  reader     tail
	if (m_reader >= m_final) {
		m_reader = m_buffer + (m_reader - m_final);
		m_lastcopy = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long
rotative_buffer::writable_size(void)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	//! ¡ý******¡ý*******************¡ý******¡ý**********¡ý
	//! head  writer            reader  final       tail
	return (m_reader > m_writer) ? m_reader - m_writer - 1 : m_size - (m_writer - m_reader) - 1;
	//!	|   len	  |                   |    read     |
	//! | copy |                      | left | copy |
	//! ¡ý*********¡ý*******************¡ý******¡ý**********¡ý
	//! head    reader             writer  final       tail
}
////////////////////////////////////////////////////////////////////////////////////////////////////
char*
rotative_buffer::write(unsigned long& size)
{
	if (size > MAX_PACKET_LEN || size == 0)
		size = MAX_PACKET_LEN;

	std::lock_guard<std::mutex> lock(m_mutex);
	//!	|   len	  |                   |    size     |
	//! | copy |                      | left | copy |
	//! ¡ý*********¡ý*******************¡ý******¡ý**********¡ý
	//! head    reader             writer  final       tail
	unsigned long left = (m_writer >= m_reader) ? m_size - (m_writer - m_reader) : m_reader - m_writer;
	(--left > size) ? (left = size) : (size = left);
#ifndef NDEBUG
	m_last_malloc = size;
#endif
	return m_writer;
	//!
	//! ¡ý*********¡ý**********¡ý******¡ý*********¡ý
	//! head   writer      reader final      tail
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
rotative_buffer::commit_write(unsigned long size)
{
	std::lock_guard<std::mutex> lock(m_mutex);
#ifndef NDEBUG
	assert(m_last_malloc >= size);
	m_last_malloc = 0;
#endif
	m_writer += size;

	//! writer touch finial
	//!	                          |  size  |
	//! ¡ý*********¡ý*******************¡ý****¡ý********¡ý
	//! head    reader              final writer    tail
	if (m_writer >= m_final) {
		m_lastcopy = m_writer - m_final;
		m_writer = m_buffer + m_lastcopy;

		//! copy
		memcpy(m_buffer, m_final, m_lastcopy);
	}

	return (m_lastread == 0); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const char*
rotative_buffer::next(unsigned long& size)
{
	if (m_limit <= m_pos)
	{
		size = 0;
		return nullptr;
	}
	size = m_limit - m_pos;
	const char* p = read(size);
	m_pos += size;
	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 