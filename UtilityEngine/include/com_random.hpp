/**
* @file com_random.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_RANDOM_HPP__
#define __COM_RANDOM_HPP__

#include <time.h>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
inline int rand(time_t seed = 0)
{
	static time_t m_nSeed = seed ? seed : time(nullptr);

	m_nSeed = m_nSeed * 214013L + 2531011L;

	return static_cast<int>((m_nSeed >> 16) & 0x7FFFFFFF);
}
////////////////////////////////////////////////////////////////////////////////
template<size_t N>
class random
{
public:
	static int next(void)
	{
		time_t* seed = _seed();
		*seed = *seed * 214013L + 2531011L;
		return static_cast<int>(((*seed) >> 16) & 0x7FFFFFFF);
	}

	static time_t get_seed(void) { return *_seed(); }
	static void set_seed(time_t seed) { *_seed() = seed; }
private:
	static time_t* _seed(void)
	{
		static time_t m_seed = time(nullptr);
		return &m_seed;
	}
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_RANDOM_HPP__
