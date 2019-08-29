/**
* @file com_typename.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_TYPENAME_HPP__
#define __COM_TYPENAME_HPP__

#include <typeinfo>

#if defined(__GNUC__)
#include <cxxabi.h> 
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_TYPENAME_HPP__
