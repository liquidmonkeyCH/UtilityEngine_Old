// MIT License
//
// Copyright (c) 2016-2017 Simon Ninon <simon.ninon@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
* @file logger.cpp
*
* @author Hourui (liquidmonkey)
*/

#include <iostream>
#include "logger.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#define DECLARE_COLOR(color,flag)							\
	inline std::ostream& color(std::ostream &s)				\
	{														\
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);	\
		SetConsoleTextAttribute(hStdout,					\
		flag | FOREGROUND_INTENSITY);						\
		return s;											\
	}

DECLARE_COLOR(green, FOREGROUND_GREEN | FOREGROUND_INTENSITY)
DECLARE_COLOR(blue, FOREGROUND_BLUE| FOREGROUND_GREEN | FOREGROUND_INTENSITY)
DECLARE_COLOR(yellow, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)
DECLARE_COLOR(red, FOREGROUND_RED | FOREGROUND_INTENSITY)
DECLARE_COLOR(normal, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#undef DECLARE_COLOR
#else
static const char green[] = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
static const char red[] = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
static const char yellow[] = { 0x1b, '[', '1', ';', '3', '3', 'm', 0 };
static const char blue[] = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
static const char normal[] = { 0x1b, '[', '0', ';', '3', '9', 'm', 0 };
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
logger::logger(log_level level) :m_level(level){}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
logger::debug(const char* msg) 
{
	if (m_level < log_level::debug) return;
	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << "[" << green << "DEBUG" << normal << "] " << msg << std::endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
logger::info(const char* msg)
{
	if (m_level < log_level::info) return;
	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << "[" << blue << "INFO" << normal << "] " << msg << std::endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
logger::warn(const char* msg)
{
	if (m_level < log_level::warn) return;
	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << "[" << yellow << "WARN" << normal << "] " << msg << std::endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
logger::error(const char* msg)
{
	if (m_level < log_level::error) return;
	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << "[" << red << "ERROR" << normal << "] " << msg << std::endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility