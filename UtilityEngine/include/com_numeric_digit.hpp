/**
* @file com_numeric_digit.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_NUMERIC_DIGIT_HPP__
#define __COM_NUMERIC_DIGIT_HPP__

#include <cstdint>
#include <limits>

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace _impl
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename num_t, std::uint8_t bit, bool ...args>
class numeric_digit_info;
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename num_t, std::uint8_t bit, bool ...args>
class numeric_digit : private numeric_digit_info<num_t, bit, args...>
{
public:
	template<typename num_t, std::uint8_t bit, bool ...args> friend class numeric_digit_info;
	static constexpr std::uint8_t child_bit = bit / 2;
	using base = numeric_digit_info<num_t, bit, args...>;
	using CHILD_L = numeric_digit<num_t, child_bit, true, args...>;
	using CHILD_H = numeric_digit<num_t, child_bit, false, args...>;
	static constexpr num_t L = ~(base::self << child_bit) & base::self;
	static constexpr num_t H = (base::self >> child_bit & L) << child_bit;
public:
	inline static int max(const num_t& param){ return param & H ? child_bit + CHILD_H::max(param) : CHILD_L::max(param); }
	inline static int min(const num_t& param){	return param & L ? CHILD_L::min(param) : child_bit + CHILD_H::min(param); }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename num_t, bool ...args>
class numeric_digit<num_t, 2, args...> : private numeric_digit_info<num_t, 2, args...>
{
public:
	using base = numeric_digit_info<num_t, 2, args...>;
	static constexpr num_t L = ~(base::self << 1) & base::self;
	static constexpr num_t H = (base::self >> 1 & L) << 1;
public:
	inline static constexpr int max(const num_t& param) { return param & H ? 2 : (param & L ? 1 : 0); }
	inline static constexpr int min(const num_t& param) { return param & L ? 1 : (param & H ? 2 : 0); }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename num_t, std::uint8_t bit, bool low, bool ...args>
struct numeric_digit_info<num_t, bit, low, args...>
{
	static constexpr num_t self = low ? numeric_digit<num_t, bit * 2, args...>::L : numeric_digit<num_t, bit * 2, args...>::H;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename num_t, std::uint8_t bit>
struct numeric_digit_info<num_t, bit>
{
	static constexpr num_t self = std::numeric_limits<num_t>::max();
};
///////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace _impl
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename num_t>
class numeric_digit : public _impl::numeric_digit<typename std::conditional<std::is_integral<num_t>::value, num_t, std::uint64_t>::type, sizeof(num_t) * 8>
{ 
	static_assert(std::is_integral<num_t>::value, "num_t must be integral!");
public:
	static constexpr int max_bit = sizeof(num_t) * 8;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com 
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_NUMERIC_DIGIT_HPP__

#define _numeric_digit_fast_max_2(param,self)(param & self::H ? 2 : (param & self::L) ? 1 : 0)
#define _numeric_digit_fast_max_4(param,self)(param & self::H ? self::child_bit + _numeric_digit_fast_max_2(param,self::CHILD_H) : _numeric_digit_fast_max_2(param,self::CHILD_L))
#define _numeric_digit_fast_max_8(param,self)(param & self::H ? self::child_bit + _numeric_digit_fast_max_4(param,self::CHILD_H) : _numeric_digit_fast_max_4(param,self::CHILD_L))
#define _numeric_digit_fast_max_16(param,self)(param & self::H ? self::child_bit + _numeric_digit_fast_max_8(param,self::CHILD_H) : _numeric_digit_fast_max_8(param,self::CHILD_L))
#define _numeric_digit_fast_max_32(param,self)(param & self::H ? self::child_bit + _numeric_digit_fast_max_16(param,self::CHILD_H) : _numeric_digit_fast_max_16(param,self::CHILD_L))
#define _numeric_digit_fast_max_64(param,self)(param & self::H ? self::child_bit + _numeric_digit_fast_max_32(param,self::CHILD_H) : _numeric_digit_fast_max_32(param,self::CHILD_L))
#define com_numeric_digit_fast_max_8(param) _numeric_digit_fast_max_8(param,com::numeric_digit<decltype(param)>)
#define com_numeric_digit_fast_max_16(param) _numeric_digit_fast_max_16(param,com::numeric_digit<decltype(param)>)
#define com_numeric_digit_fast_max_32(param) _numeric_digit_fast_max_32(param,com::numeric_digit<decltype(param)>)
#define com_numeric_digit_fast_max_64(param) _numeric_digit_fast_max_64(param,com::numeric_digit<decltype(param)>)
#define com_numeric_digit_fast_max(param,len) _numeric_digit_fast_max_##len(param,com::numeric_digit<decltype(param)>)