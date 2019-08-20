/**
* @file mem_pool.hpp
*
* 小型物件内存管理
* @author Hourui (liquidmonkey)
*/

#ifndef __MEMORY_POOL_HPP__
#define __MEMORY_POOL_HPP__

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
////////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t N>
class _chunk_base
{
protected:
#ifndef NDEBUG
	typedef typename std::conditional<N == 0, unsigned long long,
			typename std::conditional<N < SCHAR_MAX, unsigned char,
			typename std::conditional<N < SHRT_MAX, unsigned short,
			typename std::conditional<N < LONG_MAX, unsigned long,
			unsigned long long>::type>::type>::type>::type size_type;

	static_assert(N <= LLONG_MAX, "size out of range!");
	const static size_type _used = static_cast<size_type>(1) << (std::numeric_limits<size_type>::digits - 1);
	const static size_type _zero = ~_used;

	static void _set(size_type& _first, size_type _offset[])
	{
		assert(N == 0 || _first < N);
		assert(!(_offset[_first] & _used));
		size_type tmp = _first;
		_first = _offset[_first] ? (_offset[_first] == _zero ? 0 : _offset[_first]) : _first + 1;
		assert(N == 0 || _first <= N);
		_offset[tmp] = _used;
	}

	static void _reset(std::ptrdiff_t _pos, size_type& _first, size_type _offset[])
	{
		assert(_offset[_pos] & _used);

		_offset[_pos] = _first ? _first : _zero;
		_first = static_cast<size_type>(_pos);

		assert(N == 0 || _first < N);
	}
#else
	typedef typename std::conditional<N == 0, unsigned long long,
		typename std::conditional<N < UCHAR_MAX, unsigned char,
		typename std::conditional<N < USHRT_MAX, unsigned short,
		typename std::conditional<N < ULONG_MAX, unsigned long,
		unsigned long long>::type>::type>::type>::type size_type;

	static_assert(N <= ULLONG_MAX, "size out of range!");
	const static size_type _zero = std::numeric_limits<size_type>::max();

	static void _set(size_type& _first, size_type _offset[])
	{
		_first = _offset[_first] ? (_offset[_first] == _zero ? 0 : _offset[_first]) : (_first + 1);
	}

	static void _reset(std::ptrdiff_t _pos, size_type& _first, size_type _offset[])
	{
		_offset[_pos] = _first ? _first : _zero;
		_first = static_cast<size_type>(_pos);
	}
#endif
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N = 0>
class memory_pool : public _chunk_base<N>
{
private:
	typedef _chunk_base<N> base;
	typedef typename base::size_type size_type;
	template<class,size_t,class> friend class _chunk_release_model;

	T				_data[N];
	size_type		_offset[N];
	size_type		_first, _left;
public:
	memory_pool(void);
	~memory_pool(void);

	memory_pool(const memory_pool&) = delete;
	memory_pool& operator=(const memory_pool&) = delete;

	memory_pool(const memory_pool&&) = delete;
	memory_pool& operator=(const memory_pool&&) = delete;

	// do nothing
	void init(size_t size) {}

	T*	malloc(void);
	bool free(T* p);

	void clear(void);
protected:
	std::ptrdiff_t npos(T* p);
};

template<class T>
class memory_pool<T, 0> : public _chunk_base<0>
{
private:
	typedef _chunk_base<0> base;
	typedef typename base::size_type size_type;
	template<class, size_t, class> friend class _chunk_release_model;

	T*				_data;
	size_type*		_offset;
	size_type		_first, _size, _left;
public:
	memory_pool(void);
	~memory_pool(void);

	memory_pool(const memory_pool&) = delete;
	memory_pool<T, 0>& operator=(const memory_pool&) = delete;

	memory_pool(const memory_pool&&) = delete;
	memory_pool<T, 0>& operator=(const memory_pool&&) = delete;

	void init(size_t size);

	T*	malloc(void);
	bool free(T* p);

	void clear(void);
protected:
	std::ptrdiff_t npos(T* p);
};
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
	chunk*	m_cache[H];
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
class memory_pool_ex : public R<T, N, alloc<T,N,H>>
{
public:
	memory_pool_ex(void) = default;
	~memory_pool_ex(void) = default;

	memory_pool_ex(const memory_pool_ex&) = delete;
	memory_pool_ex& operator=(const memory_pool_ex&) = delete;

	memory_pool_ex(const memory_pool_ex&&) = delete;
	memory_pool_ex& operator=(const memory_pool_ex&&) = delete;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mem_pool.inl"
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////	
}//namespace Utility
#endif //__MEMORY_POOL_HPP__