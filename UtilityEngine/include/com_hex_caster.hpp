/**
* @file com_hex_caster.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_HEX_CASTER_HPP__
#define __COM_HEX_CASTER_HPP__

#include <string>
#include <cstdint>

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace _impl
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<bool upper>
struct bin_to_hex_define
{
	static constexpr char HEX[] = { 
		'0','1','2','3','4','5','6','7',
		'8','9','a','b','c','d','e','f' };
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<>
struct bin_to_hex_define<true>
{
	static constexpr char HEX[] = {
		'0','1','2','3','4','5','6','7',
		'8','9','A','B','C','D','E','F' };
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<bool upper>
inline
const char* bin_to_hex(char* out, const std::uint8_t* data, size_t size)
{
	if (!size)
	{
		out[0] = 0x0;
		return out;
	}
	std::int64_t i = -1;
	for (const std::uint8_t* end = data + size; data != end; ++data)
	{
		out[++i] = bin_to_hex_define<upper>::HEX[*data >> 4];
		out[++i] = bin_to_hex_define<upper>::HEX[*data & 0x0F];
	}
	out[++i] = 0x0;

	return out;
}

#define UTILITY_COM_HEX_TRANS(hex) ((hex & 0x40) ? (0x09 + (hex & 0x0F)) : (hex & 0x0F))
inline
void hex_to_bin(char* data, const char* hex, size_t len)
{
	for (size_t i = 0; i < len; i+=2,++data)
	{
		*data = UTILITY_COM_HEX_TRANS(hex[i]) << 4 | UTILITY_COM_HEX_TRANS(hex[i + 1]);
	}
}
#undef UTILITY_COM_HEX_TRANS
///////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t size,bool hex_header>
class bin_to_hex_buffer
{
public:
	static constexpr size_t m_len = size * 2;
	bin_to_hex_buffer(void){ m_data[0] = 0x0; m_head = m_data; }
protected:
	char m_data[m_len +1];
	char* m_head;
};

template<size_t size>
class bin_to_hex_buffer<size,true>
{
public:
	static constexpr size_t m_len = size * 2 + 2;
	bin_to_hex_buffer(void){ memcpy(m_data, "0x\0", 3);  m_head = m_data+2; }
protected:
	char m_data[m_len + 1];
	char* m_head;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, bool hex_header = false, bool upper = true>
struct hex_caster : public _impl::bin_to_hex_buffer<sizeof(T), hex_header>
{
	// bin -> hex
	hex_caster(const T& data) 
	{ _impl::bin_to_hex<upper>(this->m_head, (const std::uint8_t*)&data, sizeof(T)); }
	// hex -> bin
	hex_caster(T& data,const char* hex,size_t len) 
	{ 
		len = len > this->m_len ? this->m_len : len;
		memcpy(this->m_data, hex, len);
		this->m_data[len] = 0x0;
		memset(&data, 0, sizeof(T));

		_impl::hex_to_bin((char*)&data, this->m_head, hex_header ? len-2 : len);
	}
	const char* str(void) { return this->m_data; }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<bool hex_header = false, bool upper = true, class T>
hex_caster<T, hex_header, upper> 
bin2hex(const T& data) { return hex_caster<T, hex_header, upper>(data); }
///////////////////////////////////////////////////////////////////////////////////////////////////
template<bool hex_header = false, class T>
void 
hex2bin(T& data,const char* hex,size_t len) { hex_caster<T, hex_header>(data,hex,len); }
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com 
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_HEX_CASTER_HPP__

