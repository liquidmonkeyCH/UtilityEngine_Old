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
namespace _impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
class _data_factory
{
protected:
	using size_type = typename std::conditional < N == 0, size_t,
		typename std::conditional < N < UCHAR_MAX, unsigned char,
		typename std::conditional<N < USHRT_MAX, unsigned short,
		typename std::conditional<N < ULONG_MAX, unsigned long,
		unsigned long long>::type>::type>::type>::type;

	static_assert(N < ULLONG_MAX - 1, "size out of range!");
	static constexpr size_type _zero = size_type(-1);
	static constexpr size_type _used = _zero - 1;
public:
	class iterator
	{
	public:
		iterator(_data_factory* data, size_type pos) :m_parent(data), m_pos(pos) {}
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
		_data_factory*	m_parent;
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
protected:
	T*			m_data;
	size_type*	m_offset;
	size_type	m_next, m_left, m_size;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N = 0>
class data_factory : public _impl::_data_factory<T,N>
{
private:
	using base = _impl::_data_factory<T,N>;
	using size_type = typename base::size_type;

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
class data_factory<T, 0> : public _impl::_data_factory<T,0>
{
private:
	using base = _impl::_data_factory<T,0>;
	using size_type = typename base::size_type;
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
}/*
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace alloc_mode
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//default: new & delete
	template<class T, size_t N, size_t H>
	class normal
	{
		using chunk = memory_pool<T, N>;
	public:
		void init(size_t size) {}
		memory_pool<T, N>* malloc(size_t size);
		void free(memory_pool<T, N>* p);
	};
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// cache some chunk in free operation for malloc operation
	template<class T, size_t N, size_t H>
	class cache
	{
		using chunk = memory_pool<T, N>;
	public:
		cache(void) { m_head = m_cache - 1; m_tail = m_cache + H - 1; }

		void init(size_t size) {}
		memory_pool<T, N>* malloc(size_t size);
		void free(memory_pool<T, N>* p);
	private:
		std::mutex m_mutex;
		chunk* m_cache[H];
		chunk** m_head;
		chunk** m_tail;
	};

	template<class T, size_t N>
	class cache<T, N, 0>
	{
		using chunk = memory_pool<T, N>;
	public:
		cache(void) :_size(0) {}

		void init(size_t size) { _size = size; }
		memory_pool<T, N>* malloc(size_t size);
		void free(memory_pool<T, N>* p);
	private:
		std::mutex m_mutex;
		std::queue<chunk*> m_cache;
		std::atomic<size_t> _size;
	};
	////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace alloc_mode
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
class _chunk_release_model
{
protected:
	typedef memory_pool<T, N> chunk;
	typedef std::vector<chunk*> chunks;

	chunks	_chunks;
	size_t	_alloc_chunk;
	size_t	_dealloc_chunk;
	size_t	_size;

	_chunk_release_model(void);
	~_chunk_release_model(void);

	size_t _forward(size_t n, size_t i, size_t total) { n = n + i; return n >= total ? n - total : n; }
	size_t _backward(size_t n, size_t i, size_t total) { return n >= i ? n - i : total + n - i; }

	std::ptrdiff_t _npos(T* p, size_t& n);
	static alloc* chunk_alloc(void) { static alloc _alloc; return &_alloc; }
	size_t _chunk_size(size_t n) { return size_t(_chunks[n]->_left); }
public:
	void init(size_t size);
	// set cache size for H == 0
	static void alloc_cache(size_t size) { chunk_alloc()->init(size); }

	void clear(void);
	T* malloc(void);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace release_mode
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// hold all chunks
	template<class T, size_t N, class alloc>
	class hold : public _chunk_release_model<T, N, alloc>
	{
	private:
		typedef _chunk_release_model<T, N, alloc> base;
	public:
		bool free(T* p);
	};
	// hold only one empty chunk
	template<class T, size_t N, class alloc>
	class auto1chunk : public _chunk_release_model<T, N, alloc>
	{
	private:
		typedef _chunk_release_model<T, N, alloc> base;
	public:
		void clear(void);
		bool free(T* p);
	};
	////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace release_mode
////////////////////////////////////////////////////////////////////////////////////////////////////
// T:data type 
// N:data num in one chunk  
// R:chunk release policy
// alloc:chunk alloc&free policy 
// H:chunk cache size 
template<class T, size_t N = 0, template<class, size_t, class> class R = release_mode::auto1chunk,
	template<class, size_t, size_t> class alloc = alloc_mode::normal, size_t H = 0>
class memory_pool_ex : public R<T, N, alloc<T, N, H>>
{
public:
	memory_pool_ex(void) = default;
	~memory_pool_ex(void) = default;

	memory_pool_ex(const memory_pool_ex&) = delete;
	memory_pool_ex& operator=(const memory_pool_ex&) = delete;

	memory_pool_ex(const memory_pool_ex&&) = delete;
	memory_pool_ex& operator=(const memory_pool_ex&&) = delete;
};
}//namespace mem*/
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mem_data_factory.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////	
}//namespace Utility
#endif //__MEM_DATA_FACTORY_HPP__