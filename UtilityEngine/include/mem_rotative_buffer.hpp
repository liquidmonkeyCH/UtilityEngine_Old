/**
* @file mem_rotative_buffer.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_ROTATIVE_BUFFER_HPP__
#define __MEM_ROTATIVE_BUFFER_HPP__

#include "mem_buffer.hpp"
#include <mutex>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class rotative_buffer : public buffer_iface
{
public:
	rotative_buffer(void);
	~rotative_buffer(void);

	//! Set size and initialize 
	void init(unsigned long size);
	//! Reset initialize 
	void clear(void);

	//! Returns: total writable size
	unsigned long writable_size(void);
	//! Reg write operator
	//! Param:(in-out)size [0~MAX_PACKET_LEN]
	char* write(unsigned long& size);
	//! Commit write operator
	//! Returns: |true [m_lastread = 0]|false [m_lastread != 0]|
	//!	Next:	 |Need notify readable |No need notify readable|
	bool commit_write(unsigned long size);

	//! Returns: total readable size or 0 when readable size less-than exp.
	//! Change member [m_lastread] to 0 when readable size less-than exp.
	//! Next: readable size less-than exp [m_lastread = 0]. Need wait for readable notify.
	unsigned long readable_size(unsigned long exp = 0);
	//! Reg read operator
	//! Param:(in-out)size [0~MAX_PACKET_LEN]
	//! Always change member [m_lastread]
	const char* read(unsigned long& size);
	//! Commit read operator
	//! Release space for write operator
	void commit_read(unsigned long size);
private:
	char*	m_buffer;
	char*	m_reader;
	char*	m_writer;
	char*	m_final;
	unsigned long m_lastcopy;
	unsigned long m_lastread;
	unsigned long m_size;

	std::mutex m_mutex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_ROTATIVE_BUFFER_HPP__