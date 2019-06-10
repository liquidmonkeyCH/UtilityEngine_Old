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
	const static size_type _zero = _used - 1;

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
template<class T, size_t N>
class _chunk_release_model;
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N = 0>
class memory_pool : public _chunk_base<N>
{
private:
	typedef _chunk_base<N> base;
	typedef typename base::size_type size_type;
	friend class _chunk_release_model<T, N>;

	T				_data[N];
	size_type		_offset[N];
	size_type		_first, _left;

	void _free(T* p);
public:
	memory_pool(void);
	~memory_pool(void);

	memory_pool(const memory_pool&) = delete;
	memory_pool& operator=(const memory_pool&) = delete;

	memory_pool(const memory_pool&&) = delete;
	memory_pool& operator=(const memory_pool&&) = delete;

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
	friend class _chunk_release_model<T, 0>;

	T*				_data;
	size_type*		_offset;
	size_type		_first, _left, _size;
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
template<class T, size_t N>
class _chunk_release_model
{
protected:
	typedef memory_pool<T, N> chunk;
	typedef std::vector<chunk*> chunks;
	typedef typename chunks::iterator Ckiter;
	typedef typename chunks::reverse_iterator RCiter;

	chunks	_chunks;
	size_t	_alloc_chunk;
	size_t	_dealloc_chunk;

	_chunk_release_model(void);
	~_chunk_release_model(void);

	_chunk_release_model(const _chunk_release_model&) = delete;
	_chunk_release_model& operator=(const _chunk_release_model&) = delete;

	_chunk_release_model(const _chunk_release_model&&) = delete;
	_chunk_release_model& operator=(const _chunk_release_model&&) = delete;

	size_t _forward(size_t n, size_t i, size_t total) { n = n + i; return n >= total ? n - total : n; }
	size_t _backward(size_t n, size_t i, size_t total) { return n >= i ? n - i : total + n - i; }

	std::ptrdiff_t _npos(T* p, size_t& n);

	size_t _chunk_size(size_t n) { return _chunks[n]->_left; }
public:
	void clear(void);
	T* malloc(void);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace release_mode
{ 
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
struct hold : public _chunk_release_model<T, N>
{
	typedef _chunk_release_model<T, N> base;
	std::ptrdiff_t npos(T* p) { size_t i; std::ptrdiff_t pos = _npos(p, i); return pos<0 ? -1 : pos + i*N; }
	void free(T* p);
};

template<class T, size_t N>
struct auto1 : public _chunk_release_model<T, N>
{
	typedef _chunk_release_model<T, N> base;
	void free(T* p);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace release_mode
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N = 254, template<class, size_t> class Release = release_mode::auto1>
struct memory_pool_ex : public Release<T, N>
{
	static_assert(N, "N can not be 0!");

	memory_pool_ex(void) {}
	~memory_pool_ex(void) {}

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