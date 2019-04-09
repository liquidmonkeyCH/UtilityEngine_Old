/**
* @file base_defines.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __BASE_DEFINES_HPP__
#define __BASE_DEFINES_HPP__

#include <assert.h>
#include <string.h>
//#define _NDEBUG

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
}



#endif //__BASE_DEFINES_HPP__