/**
* @file mem_stream_buffer.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_STREAM_BUFFER_HPP__
#define __MEM_STREAM_BUFFER_HPP__

#include "mem_buffer.hpp"
#include "mem_data_factory.hpp"
#include "mem_stream_node.hpp"
#include <mutex>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class stream_buffer : public buffer_iface
{
public:
	stream_buffer(void);
	virtual ~stream_buffer(void);

	//! Set size and initialize 
	void init(unsigned long size);
	//! Reset initialize 
	void clear(void);

	//! Returns: total writable size
	unsigned long writable_size(void);
	//! Reg write operation
	//! Param:(in-out)size [0~MAX_PACKET_LEN]
	char* write(unsigned long& size);
	//! Commit write operation
	//! Returns: |true [m_lastread = 0]|false [m_lastread != 0]|
	//!	Next:	 |Need notify readable |No need notify readable|
	bool commit_write(unsigned long size);

	//! Returns: total readable size, 0 when readable size less-than exp.
	//! Change member [m_lastread] to 0 when readable size less-than exp.
	//! Next: readable size less-than exp [m_lastread = 0]. Need wait for readable notify.
	unsigned long readable_size(unsigned long exp = 0);
	//! Reg read operation
	//! Param:(in-out)size [0~MAX_PACKET_LEN]
	//! Always change member [m_lastread]
	const char* read(unsigned long& size);
	//! Commit read operation
	//! Release space for write operation
	void commit_read(unsigned long size);

	//! message iface
	void reset(void) override;
	const char* next(unsigned long& size);
public:
	using factory_t = mem::data_factory_ex<stream_node, 0, mem::factory_cache_type::DYNAMIC>;
private:
	stream_node*	m_head;
	stream_node*	m_tail;
	stream_node*	m_next;
	factory_t		m_factory;

	char*	m_reader;
	char*	m_writer;
	char*	m_out;
	size_t	m_readable;

	unsigned long m_lastread;
#ifndef NDEBUG
	unsigned long m_last_malloc;
#endif

	std::mutex m_mutex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_STREAM_BUFFER_HPP__