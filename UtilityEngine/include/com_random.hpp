/**
* @file com_random.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_RANDOM_HPP__
#define __COM_RANDOM_HPP__

#include <time.h>
#include <atomic>

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
	random(time_t seed = 0) { set_seed(seed ? seed : time(nullptr)); }
	int next(void)
	{
		m_seed = m_seed * 214013L + 2531011L;
		return static_cast<int>(((m_seed) >> 16) & 0x7FFFFFFF);
	}

	void set_seed(time_t seed) { m_seed = seed; }
private:
	std::atomic<time_t> m_seed;
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_RANDOM_HPP__
