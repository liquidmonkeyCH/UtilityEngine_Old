#include "mem_pool.hpp"
/**
* @file mem_pool.inl
*
* 小型物件内存管理
* @author Hourui (liquidmonkey)
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
//********************************* memory_pool **************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
inline
memory_pool<T, N>::memory_pool(void) :
	_first(0), _left(N)
{
	memset(_offset, 0, sizeof(size_type)*N);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
inline
memory_pool<T, N>::~memory_pool(void)
{
	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
inline
void memory_pool<T, N>::clear(void)
{
	_first = 0;
	_left = N;

	memset(_offset, 0, sizeof(size_type)*N);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
inline
std::ptrdiff_t memory_pool<T, N>::npos(T* p)
{
	if (p < _data || p >= _data + N) return -1;

	return p - _data;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
inline
T* memory_pool<T, N>::malloc(void)
{
	if (!_left)
		return nullptr;

	T* p = _data + _first;
	base::_set(_first, _offset);
	--_left;
	return p;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
inline
bool memory_pool<T, N>::free(T* p)
{
	std::ptrdiff_t _pos = npos(p);
	if (_pos < 0)
		return false;

	base::_reset(_pos, _first, _offset);
	++_left;
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
memory_pool<T, 0>::memory_pool(void) :
	_data(nullptr), _offset(nullptr), _first(0), _left(0), _size(0)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
memory_pool<T, 0>::~memory_pool(void)
{
	delete[] _data;
	delete[] _offset;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
void memory_pool<T, 0>::clear(void)
{
	_first = 0;
	_left = _size;

	memset(_offset, 0, sizeof(size_type)*size_t(_size));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
void memory_pool<T, 0>::init(size_t size)
{
	if (_size != 0) return;
	_data = new T[size];
	_offset = new size_type[size];
	_size = size;
	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
std::ptrdiff_t memory_pool<T, 0>::npos(T* p)
{
	if (!_data || p < _data || p >= _data + _size) return -1;
	return p - _data;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
T* memory_pool<T, 0>::malloc(void)
{
	if (!_left) return nullptr;
	T* p = _data + _first;
	base::_set(_first, _offset);
	--_left;
	return p;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
bool memory_pool<T, 0>::free(T* p)
{
	std::ptrdiff_t _pos = npos(p);
	if (_pos < 0)
		return false;

	base::_reset(_pos, _first, _offset);
	++_left;
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//********************************* memory_pool_ex ***********************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
// alloc_mode
namespace alloc_mode
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, size_t H>
inline 
memory_pool<T, N>* normal<T, N, H>::malloc(size_t size)
{
	chunk* p = new chunk;
	p->init(size);
	return p;
}
template<class T, size_t N, size_t H>
inline 
void normal<T, N, H>::free(memory_pool<T, N>* p)
{
	delete p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, size_t H>
inline 
memory_pool<T, N>* cache<T, N, H>::malloc(size_t size)
{
	chunk* p = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_head >= m_cache)
		{
			p = *m_head--;
			return p;
		}
	}
	p = new chunk;
	p->init(size);
	return p;
}
template<class T, size_t N, size_t H>
inline 
void cache<T, N, H>::free(memory_pool<T, N>* p)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_head < m_tail)
		{
			*++m_head = p;
			return;
		}
	}
	delete p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
inline
memory_pool<T, N>* cache<T, N, 0>::malloc(size_t size)
{
	chunk* p = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_cache.empty())
		{
			p = m_cache.front();
			m_cache.pop();
			return p;
		}
	}
	p = new chunk;
	p->init(size);
	return p;
}

template<class T, size_t N>
inline
void cache<T, N, 0>::free(memory_pool<T, N>* p)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_cache.size() < _size)
		{
			m_cache.push(p);
			return;
		}
	}
	delete p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace alloc_mode
////////////////////////////////////////////////////////////////////////////////////////////////////
// release_mode
template<class T, size_t N, class alloc>
inline 
_chunk_release_model<T, N, alloc>::_chunk_release_model(void)
	:_size(N)
	,_alloc_chunk(0)
	,_dealloc_chunk(0)
{
	if (_size)
		_chunks.push_back(chunk_alloc()->malloc(_size));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
_chunk_release_model<T, N, alloc>::~_chunk_release_model(void)
{
	for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
	{
		(*it)->clear();
		chunk_alloc()->free(*it);
		*it = nullptr;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
void _chunk_release_model<T, N, alloc>::init(size_t size)
{
	if (_size > 0) return;
	_size = size;
	_chunks.push_back(chunk_alloc()->malloc(_size));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
void _chunk_release_model<T, N, alloc>::clear(void)
{
	for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
		(*it)->clear();

	_alloc_chunk = 0;
	_dealloc_chunk = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
std::ptrdiff_t _chunk_release_model<T, N, alloc>::_npos(T* p, size_t& n)
{
	if (_size == 0) return -1;

	n = _dealloc_chunk;
	std::ptrdiff_t pos = _chunks[n]->npos(p);
	if (pos >= 0)
		return pos;

	size_t total = _chunks.size();
	for (size_t i = 1; i <= total / 2; ++i)
	{
		n = _forward(_dealloc_chunk, i, total);
		pos = _chunks[n]->npos(p);
		if (pos >= 0)
			return pos;

		size_t m = n;
		n = _backward(_dealloc_chunk, i, total);
		if (m == n) break;

		pos = _chunks[n]->npos(p);
		if (pos >= 0)
			return pos;
	}

	return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
T* _chunk_release_model<T, N, alloc>::malloc(void)
{
	if (_size == 0) return nullptr;

	if (!_chunks[_alloc_chunk]->_left)
	{
		_alloc_chunk = _chunks.size();
		for (size_t i = 0; i<_alloc_chunk; ++i)
		{
			if (!_chunks[i]->_left) continue;

			_alloc_chunk = i;
			return _chunks[i]->malloc();
		}

		_chunks.push_back(chunk_alloc()->malloc(_size));
	}

	return _chunks[_alloc_chunk]->malloc();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace release_mode
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
bool hold<T, N, alloc>::free(T* p)
{
	size_t n;
	if (base::_npos(p, n)<0)
		return false;

	base::_chunks[n]->free(p);
	base::_dealloc_chunk = n;
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
bool auto1chunk<T, N, alloc>::free(T* p)
{
	size_t n;
	if (base::_npos(p, n)<0)
		return false;

	base::_chunks[n]->free(p);
	base::_dealloc_chunk = n;

	if (base::_chunk_size(n) < _size)
		return true;

	n = base::_chunks.size() - 1;
	if (n == base::_dealloc_chunk)
		return true;

	if (base::_chunk_size(n) >= _size)
	{
		chunk_alloc()->free(base::_chunks[n]);
		base::_chunks.pop_back();
		if (base::_alloc_chunk == n)
			base::_alloc_chunk = 0;
		--n;
	}

	typename base::chunk* _chunk = base::_chunks[n];
	base::_chunks[n] = base::_chunks[base::_dealloc_chunk];
	base::_chunks[base::_dealloc_chunk] = _chunk;

	if (base::_alloc_chunk == n)
		base::_alloc_chunk = base::_dealloc_chunk;

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N, class alloc>
inline
void auto1chunk<T, N, alloc>::clear(void)
{
	typename base::chunk* _chunk = nullptr;
	alloc* _alloc = chunk_alloc();
	while (base::_chunks.size() > 1)
	{
		_chunk = base::_chunks.back();
		base::_chunks.pop_back();
		_chunk->clear();
		_alloc->free(_chunk);
	}
	base::_chunks.back()->clear();
	_alloc_chunk = 0;
	_dealloc_chunk = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace release_mode
 ////////////////////////////////////////////////////////////////////////////////////////////////////
