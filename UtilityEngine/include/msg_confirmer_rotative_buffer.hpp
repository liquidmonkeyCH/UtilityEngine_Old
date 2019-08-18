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
		if (!buffer->readable_size(size))	// ��Ϣ���Ȳ���
		{
			size = 0;
			return false;
		}
			
		const char* p = buffer->read(size);
		size = *((std::uint32_t*)(p));
		// out of bounds
		if (size > MAX_MSG_LEN)				// ��Ϣ��������
			return false;

		//  last_read == 0
		if (!buffer->readable_size(size))	// ��Ϣ���Ȳ���
		{
			size = 0;
			return false;
		}

		buffer->set_read_limit(size);		// ��Ϣ����
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
		size_t len = m_delimiter.size();			// ��Ϣ����������
		size = len - m_hit;							// ��Ҫ����ȷ�ϵĳ���
		const char* p = buffer->next(size)
		if (!p)										// ��Ϣ���Ȳ���
		{
			size = 0;
			return false;
		}

		size = buffer->get_pos();
		if (MAX_MSG_LEN && size > MAX_MSG_LEN)		// ��Ϣ�ѳ�����󳤶�
			return false;

		// ȷ��ʣ�������
		for (unsigned long i = m_hit,j=0; i < len,j < size; ++i,++j)
		{
			if (m_delimiter[i] == p[j])
			{
				++m_hit;
				continue;
			}
		}

		if (m_hit == len)					// ���н�������ƥ��
		{
			buffer->set_read_limit(size);
			buffer->reset();
			return true;
		}
		
		// ��Ϣ������δ��ȷƥ��
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