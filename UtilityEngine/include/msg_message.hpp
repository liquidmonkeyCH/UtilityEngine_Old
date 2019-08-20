/**
* @file msg_comfirmer.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_MESSAGE_HPP__
#define __MSG_MESSAGE_HPP__

#include "msg_defines.hpp"
#include "mem_buffer.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type,unsigned long MAX_MSG_LEN>
class _message_impl : public buffer_type
{
public:
	_message_impl(void) :m_size(0)
	{
		static_assert(MAX_MSG_LEN > 0, "MAX_MSG_LEN out of range!");
		mem::message_assert<buffer_type, MAX_MSG_LEN>::check();
	}
	virtual ~_message_impl(void) = default;

	void commit(void)
	{
		commit_read(m_size);
		m_size = 0;
		reset();
		set_read_limit(0);
	}

	void clear(void)
	{
		m_size = 0;
		reset();
		set_read_limit(0);
		buffer_type::clear();
	}
protected:
	std::uint32_t m_size;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pares_len
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type, unsigned long MAX_MSG_LEN>
class message_wrap : public _message_impl<buffer_type, MAX_MSG_LEN>
{
public:
	using buffer_t = buffer_type;

	message_wrap(void)
	{ 
		static_assert(MAX_MSG_LEN > sizeof(std::uint32_t), "MAX_MSG_LEN out of range!");
	}
	virtual ~message_wrap(void) = default;

	bool comfirm(unsigned long& size)
	{
		if (m_size == 0)
		{
			size = sizeof(std::uint32_t);
			if (!readable_size(size))	// 消息长度不足
			{
				size = 0;
				return false;
			}

			unsigned long len = size, pos = 0;
			const char* p;
			do {
				p = next(len);
				memcpy(&m_size + pos, p, len);
				pos += len;
				if (pos >= size) break;
				len = size - pos;
			} while (true);
			
			if (m_size < sizeof(std::uint32_t) || m_size > MAX_MSG_LEN)
			{
				size = -1;
				return false;
			}
		}

		//  last_read == 0
		if (!readable_size(m_size))	// 消息长度不足
		{
			size = 0;
			return false;
		}

		size = m_size;
		set_read_limit(m_size);		// 消息完整 
		return true;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pares_len
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pares_zero
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type, unsigned long MAX_MSG_LEN>
class message_wrap : public _message_impl<buffer_type, MAX_MSG_LEN>
{
public:
	using buffer_t = buffer_type;

	message_wrap(void) = default;
	virtual ~message_wrap(void) = default;

	bool comfirm(unsigned long& size)
	{
		const char* p = nullptr;
		unsigned long len;
		unsigned long last_readable = readable_size(0);
		do {
			// no new data
			if (last_readable == 0)
			{
				size = 0;
				return false;
			}

			do {
				len = 0;
				p = next(len);
				if (!p) break;
				size = strlen(p);

				if (size < len)
				{
					m_size = m_size + size + 1;
					size = m_size;

					if (size > MAX_MSG_LEN)
						return false;

					set_read_limit(m_size);
					reset();

					return true;
				}

				size = m_size + len;
				m_size = size;

				if (size > MAX_MSG_LEN)
					return false;

			} while (true);
			last_readable = readable_size(++last_readable);
		} while (true);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pares_zero
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pares_delimiter
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type, unsigned long MAX_MSG_LEN,const char* Delimiter>
class message_wrap : public _message_impl<buffer_type, MAX_MSG_LEN>
{
public:
	using buffer_t = buffer_type;
	using base_t = _message_impl<buffer_type, MAX_MSG_LEN>;
	const static std::size_t m_len = strlen(Delimiter);

	message_wrap(void) : m_hit(0)
	{ 
		static_assert(Delimiter && strlen(Delimiter), "Empty Delimiter!");
	}

	virtual ~message_wrap(void) = default;

	void commit(void)
	{
		base_t::commit();
		m_hit = 0;
	}

	void clear(void)
	{
		base_t::clear();
		m_hit = 0;
	}

	bool comfirm(unsigned long& size)
	{ 
		const char* p = nullptr;
		std::size_t len;
		unsigned long last_readable = readable_size(0);
		do{
			// no new data
			if (last_readable == 0)
			{
				size = 0;
				return false;
			}

			do {
				len = 0;
				p = next(len);
				if (!p) break;
				
				for (std::size_t i = 0; i < len; ++i)
				{
					if (p[i] == Delimiter[m_hit])
					{
						if (++m_hit == m_len)
						{
							size = m_size + m_hit;
							if (size > MAX_MSG_LEN)
								return false;

							m_size = size;
							set_read_limit(m_size);
							reset();
							return true;
						}
						continue;
					}

					forward();

					size = m_size + m_hit;
					if (size > MAX_MSG_LEN)
						return false;

					i = 0;
				}

				size = m_size + len;
				m_size = size - m_hit;

				if (size > MAX_MSG_LEN)
					return false;

			} while (true);
			last_readable = readable_size(++last_readable);
		} while (true);
	}
private:
	void forward(void)
	{
		std::size_t last_hit = m_hit;
		std::size_t head = 1;
		m_hit = 0;
		for (std::size_t i = head; i < last_hit;)
		{
			if (Delimiter[m_hit] == Delimiter[i])
			{
				++m_hit;
				++i;
				continue;
			}

			i = ++head;
			m_hit = 0;
		}

		m_size += head;
	}
private:
	std::size_t m_hit;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pares_delimiter
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_MESSAGE_HPP__