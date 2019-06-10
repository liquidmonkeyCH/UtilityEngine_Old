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
memory_pool<T, N>::memory_pool(void) :
	_first(0), _left(N)
{
	memset(_offset, 0, sizeof(size_type)*N);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
memory_pool<T, N>::~memory_pool(void)
{
	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
void memory_pool<T, N>::clear(void)
{
	_first = 0;
	_left = N;

	memset(_offset, 0, sizeof(size_type)*N);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
std::ptrdiff_t memory_pool<T, N>::npos(T* p)
{
	if (p < _data || p >= _data + N) return -1;

	return p - _data;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
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

	memset(_offset, 0, sizeof(size_type)*_size);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
void memory_pool<T, 0>::init(size_t size)
{
	_data = new T[size];
	_offset = new size_type[size];
	_left = size;
	_size = size;
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
//********************************* memory_poolEx ************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
_chunk_release_model<T, N>::_chunk_release_model(void) :
	_alloc_chunk(0),
	_dealloc_chunk(0)
{
	_chunks.push_back(new chunk);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
_chunk_release_model<T, N>::~_chunk_release_model(void)
{
	for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
	{
		delete *it;
		*it = nullptr;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
void _chunk_release_model<T, N>::clear(void)
{
	for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
		it->clear();

	_alloc_chunk = 0;
	_dealloc_chunk = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
std::ptrdiff_t _chunk_release_model<T, N>::_npos(T* p, size_t& n)
{
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
template<class T, size_t N>
T* _chunk_release_model<T, N>::malloc(void)
{
	if (!_chunks[_alloc_chunk]->_left)
	{
		_alloc_chunk = _chunks.size();
		for (size_t i = 0; i<_alloc_chunk; ++i)
		{
			if (!_chunks[i]->_left) continue;

			_alloc_chunk = i;
			return _chunks[i]->malloc();
		}

		_chunks.push_back(new chunk);
	}

	return _chunks[_alloc_chunk]->malloc();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace release_mode
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
void hold<T, N>::free(T* p)
{
	size_t n;
	if (base::_npos(p, n)<0)
		return;

	base::_chunks[n]->free(p);
	base::_dealloc_chunk = n;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, size_t N>
void auto1<T, N>::free(T* p)
{
	size_t n;
	if (base::_npos(p, n)<0)
		return;

	base::_chunks[n]->free(p);
	base::_dealloc_chunk = n;

	if (base::_chunk_size(n) < N)
		return;

	n = base::_chunks.size() - 1;
	if (n == base::_dealloc_chunk)
		return;

	if (base::_chunk_size(n) >= N)
	{
		delete base::_chunks[n];
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
}
///////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace release_mode
 ////////////////////////////////////////////////////////////////////////////////////////////////////
