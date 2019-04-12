/**
* @file msg_pares.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_PARES_HPP__
#define __MSG_PARES_HPP__

#include "msg_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
inline const char* pares_len(mem::buffer_iface* buffer, unsigned long& size)
{
	size = sizeof(std::uint32_t);
	if (!buffer->readable_size(size))
		return nullptr;

	const char* p = buffer->read(size);
	size = *((std::uint32_t*)(p));
	if (size > MAX_PACKET_LEN)
		return nullptr;

	if (!buffer->readable_size(size))
		return nullptr;

	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline const char* pares_zero(mem::buffer_iface* buffer, unsigned long& size)
{
	const char* p = nullptr;
	std::size_t len = 0;
	do{
		size = 0;
		p = buffer->read(size);
		if (size == 0) 
			return nullptr;

		len = strlen(p);
		if (len < size)
		{
			size = len + 1;
			return p;
		}

		if (++size > MAX_PACKET_LEN) 
			return nullptr;

	} while (buffer->readable_size(size));

	return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_PARES_HPP__