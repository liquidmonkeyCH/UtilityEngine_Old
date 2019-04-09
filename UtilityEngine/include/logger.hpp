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
* @file logger.hpp
*
* @author Hourui (liquidmonkey)
*/

#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <atomic>
#include <stdarg.h>
#include <cstdint>
#include <mutex>
#include <stdexcept>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class utility_error : public std::runtime_error
{
public:
	utility_error(const std::int64_t error_no, const char* what = nullptr);
	~utility_error(void) = default;

	utility_error(const utility_error&) = default;
	utility_error& operator=(const utility_error&) = default;
public:
	const std::int64_t get_error(void) const;
private:
	std::int64_t m_error;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class logger_iface 
{
public:
	logger_iface(void) = default;
	virtual ~logger_iface(void) = default;
	logger_iface(const logger_iface&) = delete;
	logger_iface& operator=(const logger_iface&) = delete;
public:
	virtual void debug(const char*) = 0;
	virtual void info(const char*) = 0;
	virtual void warn(const char*) = 0;
	virtual void error(const char*) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class logger : public logger_iface 
{
public:
	enum class log_level {
		error = 0,
		warn = 1,
		info = 2,
		debug = 3
	};
public:
	logger(log_level level = log_level::info);
	~logger(void) = default;
	logger(const logger&) = delete;
	logger& operator=(const logger&) = delete;
public:
	void debug(const char*);
	void info(const char*);
	void warn(const char*);
	void error(const char*);
private:
	log_level m_level;
	std::mutex m_mutex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#define LOG_FORMAT(fmt)							\
	char str[MAX_LOG_LEN + 2];					\
	va_list arglist;							\
	va_start(arglist, fmt);						\
	vsnprintf(str, MAX_LOG_LEN, fmt, arglist);	\
	str[MAX_LOG_LEN] = 0x0;						\
	va_end(arglist);
#define LOG_OUTPUT(type,fmt)					\
	logger_iface* _logger = *refence();			\
	if(!_logger){ return; }						\
	LOG_FORMAT(fmt)								\
	_logger->type(str);			
#define LOG_THROW(e,type,fmt)					\
	LOG_FORMAT(fmt)								\
	logger_iface* _logger = *refence();			\
	if(_logger){ _logger->type(str); }
#define LOG_STHROW(e,type,fmt)					\
	LOG_THROW(e,type,fmt)						\
	throw utility_error(static_cast<std::int64_t>(e),str);
#define LOG_NTHROW(e,type,fmt)					\
	LOG_THROW(e,type,fmt)						\
	throw utility_error(static_cast<std::int64_t>(e));	
////////////////////////////////////////////////////////////////////////////////////////////////////
class Clog
{
private:
	static const int MAX_LOG_LEN = 1024;
public:
	Clog(void) = delete;
	Clog(const Clog&) = delete;
	Clog& operator=(const Clog&) = delete;
	Clog(const Clog&&) = delete;
	Clog& operator=(const Clog&&) = delete;
public:	
	static void active_logger(logger_iface* _logger)			{ *refence() = _logger; }
	static void debug(const char* fmt, ...)						{ LOG_OUTPUT(debug, fmt) }
	static void info(const char* fmt, ...)						{ LOG_OUTPUT(info, fmt) }
	static void warn(const char* fmt, ...)						{ LOG_OUTPUT(warn, fmt)	}
	static void error(const char* fmt, ...)						{ LOG_OUTPUT(error, fmt) }
	template<class T>
	static void warn_throw(T e_no, const char* fmt, ...)		{ LOG_STHROW(e_no, warn, fmt) }
	template<class T>
	static void error_throw(T e_no, const char* fmt, ...)		{ LOG_STHROW(e_no, error, fmt) }
	template<class T>
	static void warn_throw_no(T e_no, const char* fmt, ...)		{ LOG_NTHROW(e_no, warn, fmt) }
	template<class T>
	static void error_throw_no(T e_no, const char* fmt, ...)	{ LOG_NTHROW(e_no, error, fmt) }
private:
	static logger_iface** refence()
	{
		static logger_iface* m_logger = nullptr;
		return &m_logger;
	}
};
#undef LOG_OUTPUT
#undef LOG_FORMAT
#undef LOG_THROW
#undef LOG_STHROW
#undef LOG_NTHROW
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__LOGGER_HPP__