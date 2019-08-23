/**
* @file base_defines.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __BASE_DEFINES_HPP__
#define __BASE_DEFINES_HPP__

//#define NDEBUG

#include <assert.h>
#include <string.h>
#include <time.h>

#define MAX_PACKET_LEN 4096

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class errors{
	none,
	system,
	logic
};
////////////////////////////////////////////////////////////////////////////////////////////////////
struct strless
{
	bool operator()(const char* const& L, const char* const& R) const
	{
		return strcmp(L, R) < 0;
	}
};

struct wstrless
{
	bool operator()(const wchar_t* const& L, const wchar_t* const& R) const
	{
		return wcscmp(L, R) < 0;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
inline int rand(void)
{
	static time_t m_nSeed = time(NULL);

	m_nSeed = m_nSeed * 214013L + 2531011L;

	return static_cast<int>((m_nSeed >> 16) & 0x7FFFFFFF);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)
#include <cxxabi.h> 
#endif
template<class T>
static const char* type_name()
{
	static const char* _id = nullptr;
	if (_id != nullptr) return _id;
#if defined(__GNUC__)
	_id = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#else
	const char* p = typeid(T).name();
	if (p[0] == 'c') _id = p + 6;
	else if (p[0] == 's') _id = p + 7;
	else _id = p;
#endif
	return _id;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif //__BASE_DEFINES_HPP__