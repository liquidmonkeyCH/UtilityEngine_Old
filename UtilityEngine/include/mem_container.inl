/**
* @file mem_container.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
container<T>::container(void)
: m_data(nullptr)
, m_offset(nullptr)
, m_next(0)
, m_left(0)
, m_size(0)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
container<T>::~container(void)
{
	delete[] m_data;
	delete[] m_offset;

	m_data = nullptr;
	m_offset = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void 
container<T>::clear(void)
{
	m_next = 0;
	m_left = m_size;
	memset(m_offset, 0, sizeof(size_t) * m_size);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void 
container<T>::init(size_t size)
{
	assert(size != 0);
	assert(size <= size_t(-1) - 2);
	if (m_data != nullptr)
	{
		if (size == m_size)
		{
			clear();
			return;
		}

		delete[] m_data;
		delete[] m_offset;
	}

	m_data = new T[size];
	m_offset = new size_t[size];
	m_left = size;
	m_size = size;
	_zero = size + 1;
	_used = size + 2;

	memset(m_offset, 0, sizeof(size_t) * m_size);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline std::ptrdiff_t 
container<T>::npos(T* p)
{
	if (!m_data || p < m_data || p >= m_data + m_size) return -1;

	return p - m_data;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline T* 
container<T>::malloc(void)
{
	if (!m_left) return nullptr;

	--m_left;
	size_t next = m_next;
	m_next = m_offset[m_next] ? (m_offset[m_next] == _zero ? 0 : m_offset[m_next]) : (m_next + 1);
	
	m_offset[next] = _used;
	return m_data + next;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline bool 
container<T>::free(T* p)
{
	std::ptrdiff_t _pos = npos(p);
	if(_pos < 0) return false;

	assert(m_offset[_pos] == _used);
	++m_left;

	m_offset[_pos] = m_next ? m_next : _zero;
	m_next = static_cast<size_t>(_pos);

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline typename container<T>::iterator 
container<T>::used_begin(void)
{
	if (m_offset[0] == _used) return{ this, 0 };
	if (m_offset[0] == 0) return{ this, m_size };

	iterator it(this, 0);
	return ++it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline typename container<T>::iterator 
container<T>::used_end(void)
{
	return{ this, m_size };
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline size_t 
container<T>::head_zero(void)
{
	size_t head = 0;
	size_t tail = m_size;
	size_t n;
	do{
		n = (head + tail) / 2;
		if (m_offset[n] == 0)
		{
			if (n == head)	return n;
			tail = n;
		}
		else
		{
			if (n == head){	return n + 1; }
			head = n;
		}
	} while (true);

	return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline typename container<T>::iterator&
container<T>::iterator::operator++(void)
{
	while (m_pos < m_parent->m_size && m_parent->m_offset[++m_pos] != m_parent->_used){
		if (m_parent->m_offset[m_pos] == 0){
			m_pos = m_parent->m_size;
			break;
		}
	}

	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline typename container<T>::iterator
container<T>::iterator::operator++(int)
{
	iterator it = *this;
	operator++();
	return it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline typename container<T>::iterator&
container<T>::iterator::operator--(void)
{
	if (m_parent->m_offset[0] == 0) return *this;
	if (m_pos == m_parent->m_size) m_pos = m_parent->head_zero();

	while (m_pos > 0 && m_parent->m_offset[--m_pos] != m_parent->_used){}
	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline typename container<T>::iterator
container<T>::iterator::operator--(int)
{
	iterator it = *this;
	operator--();
	return it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////