/**
* @file msg_comfirmer_rotative_buffer.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_COMFIRMER_ROTATIVE_BUFFER_HPP__
#define __MSG_COMFIRMER_ROTATIVE_BUFFER_HPP__

#include "msg_confirmer.hpp"
#include "mem_rotative_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/*template<unsigned long MAX_MSG_LEN>
class comfirmer_len<mem::rotative_buffer, MAX_MSG_LEN>
{
public:
	static_assert(MAX_MSG_LEN > 0 && MAX_MSG_LEN <= MAX_PACKET_LEN, "MAX_MSG_LEN out of bound!");

	bool is_done(mem::rotative_buffer* buffer,unsigned long& size)
	{
		size = sizeof(std::uint32_t);
		if (!buffer->readable_size(size))	// 消息长度不足
		{
			size = 0;
			return false;
		}
			
		const char* p = buffer->read(size);
		size = *((std::uint32_t*)(p));
		// out of bounds
		if (size > MAX_MSG_LEN)				// 消息长度有误
			return false;

		//  last_read == 0
		if (!buffer->readable_size(size))	// 消息长度不足
		{
			size = 0;
			return false;
		}

		buffer->set_read_limit(size);		// 消息完整
		return true;
	}
};*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<int MAX_MSG_LEN>
class comfirmer_delimiter<mem::rotative_buffer, MAX_MSG_LEN>
{
public:
	static_assert(MAX_MSG_LEN > 0 & MAX_MSG_LEN <= MAX_PACKET_LEN, "MAX_MSG_LEN out of bound!");
	comfirmer_delimiter(const char* delimiter) :m_delimiter(delimiter), m_hit(0){}
	comfirmer_delimiter(void) = delete;
	bool is_done(mem::rotative_buffer* buffer, unsigned long& size)
	{
		size_t len = m_delimiter.size();			// 消息结束符长度
		size = len - m_hit;							// 需要继续确认的长度
		const char* p = buffer->next(size)
		if (!p)										// 消息长度不足
		{
			size = 0;
			return false;
		}

		size = buffer->get_pos();
		if (MAX_MSG_LEN && size > MAX_MSG_LEN)		// 消息已超出最大长度
			return false;

		// 确认剩余结束符
		for (unsigned long i = m_hit,j=0; i < len,j < size; ++i,++j)
		{
			if (m_delimiter[i] == p[j])
			{
				++m_hit;
				continue;
			}
		}

		if (m_hit == len)					// 所有结束符均匹配
		{
			buffer->set_read_limit(size);
			buffer->reset();
			return true;
		}
		
		// 消息结束符未正确匹配
		size = 0;
		return false;
	}
private:
	std::string m_delimiter;
	unsigned long m_hit;
};
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_COMFIRMER_ROTATIVE_BUFFER_HPP__