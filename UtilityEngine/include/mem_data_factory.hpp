/**
* @file mem_data_factory.hpp
*
* @author Hourui (liquidmonkey)
*/

#ifndef __MEM_DATA_FACTORY_HPP__
#define __MEM_DATA_FACTORY_HPP__

#include <assert.h>
#include <limits>
#include <climits>
#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include <string.h>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem 
{ 
enum factory_cache_type { NO_CACHE, DYNAMIC }; 
template<class, std::uint64_t, size_t> class data_factory_ex;
} 
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace _impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace factory
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
class data_factory
{
public:
	using size_type = typename std::conditional < N == 0, size_t,
		typename std::conditional < N < UCHAR_MAX, unsigned char,
		typename std::conditional<N < USHRT_MAX, unsigned short,
		typename std::conditional<N < ULONG_MAX, unsigned long,
		unsigned long long>::type>::type>::type>::type;

	static constexpr size_type _zero = size_type(-1);
	static constexpr size_type _used = _zero - 1;
	static_assert(N < _used, "size out of range!");

	template<class, std::uint64_t, size_t> friend class mem::data_factory_ex;
public:
	class iterator
	{
	public:
		iterator(data_factory* data, size_type pos) :m_parent(data), m_pos(pos) {}
	public:
		T* operator->(void) { return m_parent->m_data + m_pos; }
		T* operator*(void) { return m_parent->m_data + m_pos; }
		iterator operator++(int);
		iterator& operator++(void);
		iterator operator--(int);
		iterator& operator--(void);
		bool operator==(const iterator& rhs) { return m_parent == rhs.m_parent && m_pos == rhs.m_pos; }
		bool operator!=(const iterator& rhs) { return !operator==(rhs); }
	private:
		data_factory*	m_parent;
		size_type		m_pos;
	};
	friend class iterator;
public:
	T* malloc(void);
	bool free(T* p);

	iterator used_begin(void);
	iterator used_end(void);

	size_type size(void) { return m_size; }
	size_type left(void) { return m_left; }
	size_type used(void) { return m_size - m_left; }
protected:
	size_type npos(T* p);
	size_type head_zero(void);
	T* head_data(void) { return m_data; }
protected:
	T*			m_data;
	size_type*	m_offset;
	size_type	m_next, m_left, m_size;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory, size_t Cache>
class allocator
{
public:
	using size_type = typename Factory::size_type;

	allocator(void) { m_head = m_cache + Cache; m_tail = m_head; }

	Factory* malloc(size_type size);
	void free(Factory* p);
private:
	std::mutex	m_mutex;
	Factory* m_cache[Cache];
	Factory** m_head;
	Factory** m_tail;
};

template<class Factory>
class allocator<Factory, mem::factory_cache_type::NO_CACHE>
{
public:
	using size_type = typename Factory::size_type;

	Factory* malloc(size_type size);
	void free(Factory* p);
};

template<class Factory>
class allocator<Factory, mem::factory_cache_type::DYNAMIC>
{
public:
	using size_type = typename Factory::size_type;

	void init(size_t size) { m_size = size; }
	Factory* malloc(size_type size);
	void free(Factory* p);
private:
	std::mutex				m_mutex;
	std::queue<Factory*>	m_cache;
	std::atomic<size_t>		m_size;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/*template<class Factory>
struct factory_node : public Factory
{
	factory_node(void) :m_prev(nullptr), m_next(nullptr) {}
	void clear(void) { m_prev = nullptr; m_next = nullptr; Factory::clear(); }
	factory_node* m_prev;
	factory_node* m_next;
};*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory, size_t Cache>
class allocator_wrap
{
protected:
	using allocator_t = allocator<Factory, Cache>;
	static allocator_t* get_allocator(void) { static allocator_t _allocator; return &_allocator; }
};

template<class Factory>
class allocator_wrap<Factory,mem::factory_cache_type::DYNAMIC>
{
protected:
	using allocator_t = allocator<Factory, mem::factory_cache_type::DYNAMIC>;
	static allocator_t* get_allocator(void) { static allocator_t _allocator; return &_allocator; }
public:
	static void init_allocator(size_t size) { get_allocator()->init(size); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace factory
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N = 0>
class data_factory : public _impl::factory::data_factory<T,N>
{
public:
	using base = _impl::factory::data_factory<T,N>;
	using size_type = typename base::size_type;
	using iterator = typename base::iterator;

	T				_data[N];
	size_type		_offset[N];
public:
	data_factory(void);
	~data_factory(void);

	data_factory(const data_factory&) = delete;
	data_factory& operator=(const data_factory&) = delete;

	data_factory(const data_factory&&) = delete;
	data_factory& operator=(const data_factory&&) = delete;

	void init(size_type size) { clear(); }
	void clear(void);
};

template<class T>
class data_factory<T, 0> : public _impl::factory::data_factory<T, 0>
{
public:
	using base = _impl::factory::data_factory<T,0>;
	using size_type = base::size_type;
	using iterator = base::iterator;
public:
	data_factory(void);
	~data_factory(void);

	data_factory(const data_factory&) = delete;
	data_factory<T, 0>& operator=(const data_factory&) = delete;

	data_factory(const data_factory&&) = delete;
	data_factory<T, 0>& operator=(const data_factory&&) = delete;

	void init(size_type size);
	void clear(void);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T,std::uint64_t N,size_t Cache = factory_cache_type::NO_CACHE>
class data_factory_ex : public 
	_impl::factory::allocator_wrap< data_factory<T, N>,Cache>
{
public:
	using base_t = data_factory<T, N>;
	using allocator_t = _impl::factory::allocator<base_t, Cache>;
	using chunks_t = std::vector<base_t*>;
	using size_type = typename base_t::size_type;
	using allocator_wrap_t = _impl::factory::allocator_wrap<base_t, Cache>;
private:
	chunks_t	m_chunks;
	size_t		m_alloc_chunk;
	size_t		m_dealloc_chunk;
	size_type	m_size;
private:
	auto npos(T* p, size_t& n)->size_type;
public:
	data_factory_ex(void);
	~data_factory_ex(void);

	data_factory_ex(const data_factory_ex&) = delete;
	data_factory_ex& operator=(const data_factory_ex&) = delete;

	data_factory_ex(const data_factory_ex&&) = delete;
	data_factory_ex& operator=(const data_factory_ex&&) = delete;

	void init(size_type size);
	void clear(void);

	T* malloc(void);
	bool free(T* p);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // mem
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mem_data_factory.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////	
}//namespace Utility
#endif //__MEM_DATA_FACTORY_HPP__