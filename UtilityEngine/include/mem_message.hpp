/**
* @file mem_message.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_MESSAGE_HPP__
#define __MEM_MESSAGE_HPP__

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class message
{
public:
	message(void) :m_limit(0), m_pos(0) {}
	~message(void) = default;
  
	virtual const char* next(unsigned long &size) = 0;
	virtual void reset(void) { m_pos = 0; }

	unsigned long get_pos(void) { return m_pos; }
	void set_read_limit(unsigned long limit) { m_limit = limit; }
	unsigned long get_read_limit(void) { return m_limit; }
protected:
	unsigned long m_limit;
	unsigned long m_pos;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_BUFFER_HPP__