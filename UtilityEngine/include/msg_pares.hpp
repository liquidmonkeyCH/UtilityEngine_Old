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
	// out of bounds
	if (size > MAX_PACKET_LEN)
		return nullptr;

	//  last_read == 0
	if (!buffer->readable_size(size))
		return nullptr;

	buffer->set_read_limit(size);

	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline const char* pares_zero(mem::buffer_iface* buffer, unsigned long& size)
{
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
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_PARES_HPP__