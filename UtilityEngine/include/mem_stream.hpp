/**
* @file mem_stream.hpp
*
* cons
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
		err_unpcak = -2,	// 解包错误数据
		eof = -1,			// 解包遇到文件尾
		good = 0,			// 正常
		over_flow,			// 压包溢出
		err_pack,			// 压包错误数据
	};
public:
	_stream_impl(buffer_iface* buffer) :m_buffer(buffer), m_state(state_t::good){}
	~_stream_impl(void) = default;

	state_t state(void) { return m_state; }
	bool good(void) { return m_state == state_t::good; }
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
class iostream : public istream, public ostream {};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
istream& istream::operator<<(T& kData)
{
	std::uint32_t size = sizeof(T);

	if (m_buffer->writable_size() < size)
	{
		m_state = state_t::over_flow;
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
		m_state = state_t::eof;
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
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_BUFFER_HPP__
