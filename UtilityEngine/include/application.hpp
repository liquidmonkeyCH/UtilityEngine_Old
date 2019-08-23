/**
* @file application.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include <mutex>
#include <condition_variable>
#include <string>
#include <map>
#include <vector>

#include "base_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace main
{
////////////////////////////////////////////////////////////////////////////////
class application
{
private:
	class controler
	{
	public:
		controler(void) = default;
		~controler(void) = default;
		void run(void);
		void stop(void);
	private:
		std::mutex m_mutex;
		std::condition_variable	m_cv;
	};
public:
	application(void) = default;
	~application(void) = default;
	using param_list = std::vector<std::string>;
	using param_map = std::map<const char*,int,strless>;
	friend void on_signal(int n);
public:
	bool Start(int param_num, char* params[]);
	void Run(void);
protected:
	size_t get_param_num(void);
	const char* get_param(size_t n);
	int get_param_pos(const char* param);
protected:
	virtual bool OnStart(void) = 0;
	virtual void OnStop(void) = 0;
private:
	void daemon(void);
	void setsignal(void);
	static controler* get_controler(void);
private:
	param_list				m_param_list;
	param_map				m_param_map;
};
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility
#endif //__APPLICATION_HPP__

