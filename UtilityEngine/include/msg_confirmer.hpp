/**
* @file msg_comfirmer.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_COMFIRMER_HPP__
#define __MSG_COMFIRMER_HPP__

#include "msg_defines.hpp"
#include "mem_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type,unsigned long MAX_MSG_LEN>
class comfirmer_len
{
public:
	comfirmer_len(void)
	{
		if(std::is_same<buffer_type, mem::rotative_buffer>::value)
			static_assert(MAX_MSG_LEN > 0 && MAX_MSG_LEN <= MAX_PACKET_LEN, "MAX_MSG_LEN out of bound!");
	
	}
	bool is_done(buffer_type* buffer, unsigned long& size)
	{
		size = sizeof(std::uint32_t);
		if (!buffer->readable_size(size))	// 消息长度不足
		{
			size = 0;
			return false;
		}

		unsigned long len = size,left = size;
		const char* p = buffer->next(len);
		memcpy(&size, p, len);
		left -= len;
		if (left > 0)
		{
			const char* p = buffer->next(left);
			memcpy(&size + len, p, left);
		}

		// out of bounds
		if (size == 0 || size > MAX_MSG_LEN) // 消息长度有误
		{
			size = -1;
			return false;
		}
			
		buffer->reset();
		//  last_read == 0
		if (!buffer->readable_size(size))	// 消息长度不足
		{
			size = 0;
			return false;
		}

		buffer->set_read_limit(size);		// 消息完整
		return true;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type, unsigned long MAX_MSG_LEN>
class comfirmer_delimiter
{
public:
	comfirmer_delimiter(const char* delimiter) { static_assert(false, "unrealized class!"); }
	bool is_done(buffer_type* buffer, unsigned long& size) { size = -1;  return false; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/*template<class buffer_type, class separator, int MAX_MSG_LEN>
inline const char*
comfirmer_wrap<buffer_type, separator, MAX_MSG_LEN>::prepare(mem::buffer_iface* buffer, unsigned long& size)
{
	size = sizeof(std::uint32_t);
	if (!buffer->readable_size(size))
		return nullptr;

	const char* p = buffer->read(size);
	size = *((std::uint32_t*)(p));
	// out of bounds
	if (size > MAX_PACKET_LEN)
		return nullptr;

	//  last_read == 0
	if (!buffer->readable_size(size))
		return nullptr;

	buffer->set_read_limit(size);

	return p;

	const char* p = nullptr;
	std::size_t len = 0;
	unsigned long last_readable = buffer->readable_size(0);
	do
	{
		// no new data
		if (last_readable == 0)
			return nullptr;

		size = last_readable;
		p = buffer->read(size);
		len = strlen(p);
		if (len < size)
		{
			size = len + 1;
			buffer->set_read_limit(size);
			return p;
		}
		// out of bounds
		if (++size > MAX_PACKET_LEN)
			return nullptr;

		last_readable = buffer->readable_size(++last_readable);
	} while (true);

	return nullptr;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_PARES_HPP__