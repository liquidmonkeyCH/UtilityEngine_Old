/**
* @file mem_data_factory.inl
*
* @author Hourui (liquidmonkey)
*/

namespace _impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename _data_factory<T, N>::size_type
_data_factory<T, N>::npos(T* p)
{
	if (!m_data || p < m_data || p >= m_data + m_size) return _zero;

	return p - m_data;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline T*
_data_factory<T, N>::malloc(void)
{
	if (!m_left) return nullptr;

	--m_left;
	size_type next = m_next;
	m_next = m_offset[m_next] ? (m_offset[m_next] == _zero ? 0 : m_offset[m_next]) : (m_next + 1);

	m_offset[next] = _used;
	return m_data + next;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline bool
_data_factory<T, N>::free(T* p)
{
	size_type _pos = npos(p);
	if (_pos == _zero) return false;

	assert(m_offset[_pos] == _used);
	++m_left;

	m_offset[_pos] = m_next ? m_next : _zero;
	m_next = _pos;

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename _data_factory<T, N>::iterator
_data_factory<T, N>::used_begin(void)
{
	if (m_offset[0] == _used) return{ this, 0 };
	if (m_offset[0] == 0) return{ this, m_size };

	iterator it(this, 0);
	return ++it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename _data_factory<T, N>::iterator
_data_factory<T, N>::used_end(void)
{
	return{ this, m_size };
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline  typename _data_factory<T, N>::size_type
_data_factory<T, N>::head_zero(void)
{
	size_type head = 0;
	size_type tail = m_size;
	size_type n;
	do {
		n = (head + tail) / 2;
		if (m_offset[n] == 0)
		{
			if (n == head)	return n;
			tail = n;
		}
		else
		{
			if (n == head) { return n + 1; }
			head = n;
		}
	} while (true);

	return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename _data_factory<T, N>::iterator&
_data_factory<T, N>::iterator::operator++(void)
{
	while (m_pos < m_parent->m_size && m_parent->m_offset[++m_pos] != m_parent->_used) {
		if (m_parent->m_offset[m_pos] == 0) {
			m_pos = m_parent->m_size;
			break;
		}
	}

	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename _data_factory<T, N>::iterator
_data_factory<T, N>::iterator::operator++(int)
{
	iterator it = *this;
	operator++();
	return it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename _data_factory<T, N>::iterator&
_data_factory<T, N>::iterator::operator--(void)
{
	if (m_parent->m_offset[0] == 0) return *this;
	if (m_pos == m_parent->m_size) m_pos = m_parent->head_zero();

	while (m_pos > 0 && m_parent->m_offset[--m_pos] != m_parent->_used) {}
	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename _data_factory<T, N>::iterator
_data_factory<T, N>::iterator::operator--(int)
{
	iterator it = *this;
	operator--();
	return it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline
data_factory<T, N>::data_factory(void)
{
	this->m_offset = _offset;
	this->m_data = _data;
	this->m_size = N;

	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline
data_factory<T, N>::~data_factory(void)
{
	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline
void data_factory<T, N>::clear(void)
{
	this->m_next = 0;
	this->m_left = N;
	memset(_offset, 0, sizeof(size_type) * N);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
data_factory<T, 0>::data_factory(void)
{
	this->m_data = nullptr;
	this->m_offset = nullptr;
	this->m_next = 0;
	this->m_left = 0;
	this->m_size = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
data_factory<T, 0>::~data_factory(void)
{
	delete[] this->m_data;
	delete[] this->m_offset;

	this->m_data = nullptr;
	this->m_offset = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
void data_factory<T, 0>::clear(void)
{
	this->m_next = 0;
	this->m_left = this->m_size;

	memset(this->m_offset, 0, sizeof(size_type) * size_t(this->m_size));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
void data_factory<T, 0>::init(size_type size)
{
	assert(size != 0);
	assert(size < this->_used);
	if (m_data != nullptr)
	{
		if (size == this->m_size)
		{
			clear();
			return;
		}

		delete[] this->m_data;
		delete[] this->m_offset;
	}

	this->m_data = new T[size];
	this->m_offset = new size_type[size];
	this->m_size = size;
	clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////