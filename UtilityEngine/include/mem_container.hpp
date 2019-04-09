/**
* @file mem_container.hpp
*
* @author Hourui (liquidmonkey)
*/

#ifndef __MEM_CONTAINER_HPP__
#define __MEM_CONTAINER_HPP__

#include <assert.h>
#include <climits>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
class container
{
public:
	class iterator
	{
	public:
		iterator(container* data, size_t pos) :m_parent(data), m_pos(pos){}
	public:
		T* operator->(void){ return m_parent->m_data + m_pos; }
		T* operator*(void){ return m_parent->m_data + m_pos; }
		iterator operator++(int);
		iterator& operator++(void);
		iterator operator--(int);
		iterator& operator--(void);
		bool operator==(const iterator& rhs){ return m_parent == rhs.m_parent && m_pos == rhs.m_pos; }
		bool operator!=(const iterator& rhs){ return !operator==(rhs); }
	private:
		container*		m_parent;
		size_t			m_pos;
	};
	friend class iterator;
public:
	container(void);
	~container(void);

	container(const container&) = delete;
	container<T>& operator=(const container&) = delete;
public:
	void init(size_t size);
	void clear(void);

	std::ptrdiff_t npos(T* p);

	T*	malloc(void);
	bool free(T* p);

	iterator used_begin(void);
	iterator used_end(void);

	size_t size(void){ return m_size; }
	size_t left(void){ return m_left; }
	size_t used(void){ return m_size - m_left; }
private:
	size_t head_zero(void);
private:
	T*			m_data;
	size_t*		m_offset;
	size_t		m_next, m_left, m_size, _zero, _used;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mem_container.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////	
}//namespace Utility
#endif //__MEM_CONTAINER_HPP__