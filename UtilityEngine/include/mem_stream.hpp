/**
* @file mem_stream.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_STREAM_HPP__
#define __MEM_STREAM_HPP__

#include "mem_stream_node.hpp"
#include "mem_buffer.hpp"
#include <iostream>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class _stream_impl
{
public:
	enum class state_t
	{
		SST_ERROR_UNPACK = -2,	// 解包错误数据
		SST_EOF = -1,			// 解包遇到文件尾
		SST_OK = 0,				// 正常
		SST_OVERFLOW,			// 压包溢出
		SST_ERROR_PACK,			// 压包错误数据
	};
public:
	_stream_impl(buffer_iface* buffer) :m_buffer(buffer), m_state(state_t::SST_OK){}
	~_stream_impl(void) = default;

	state_t state(void) { return m_state; }
protected:
	buffer_iface* m_buffer;
	state_t	m_state;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class istream : virtual public _stream_impl
{
public:
	template<class T>
	istream& operator << (T& data);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class ostream : virtual public _stream_impl
{
public:
	template<class T>
	ostream& operator >> (T& data);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class stream_view : virtual public _stream_impl
{
public:
	template<class T>
	stream_view& operator >> (T& data);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
istream& istream::operator<<(T& kData)
{
	std::uint32_t size = sizeof(T);

	if (m_buffer->writable_size() < size)
	{
		m_state = state_t::SST_OVERFLOW;
		return *this;
	}

	std::uint32_t len,pos = 0;
	const char* p;

	do {
		len = size - pos;
		p = m_buffer->write(len);
		memcpy(p, &kData + pos, len);
		m_buffer->commit_write(len);
		pos += len;
	} while (pos < size);
	
	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
ostream& ostream::operator>>(T& kData)
{
	std::uint32_t size = sizeof(T);

	if (m_buffer->readable_size() < size)
	{
		m_state = state_t::SST_EOF;
		return *this;
	}

	std::uint32_t len,pos = 0;
	const char* p;

	do {
		len = size - pos;
		p = m_buffer->read(len);
		memcpy(&kData + pos, p, len);
		m_buffer->commit_read(len);
		pos += len;
	} while (pos < size);

	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
stream_view& stream_view::operator>>(T& kData)
{
	std::uint32_t size = sizeof(T);
	std::uint32_t len = size,pos = 0;
	const char* p = m_buffer->next(len);

	if (!p)
	{
		m_state = state_t::SST_EOF;
		return *this;
	}

	do {
		memcpy(&kData+pos, p, len);
		pos += len;
		if (pos >= size) break;
		len = size - pos;
		p = m_buffer->next(len);
	} while (true);

	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_BUFFER_HPP__
