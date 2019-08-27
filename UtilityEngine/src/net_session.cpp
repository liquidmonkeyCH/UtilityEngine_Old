/**
* @file net_session.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "net_session.hpp"
#include "net_io_service.hpp"
#include "net_framework.hpp"

//#define SESSION_LOG
#ifdef SESSION_LOG
#define SESSION_DEBUG(fmt,...) NET_DEBUG(fmt,##__VA_ARGS__)
#else
#define SESSION_DEBUG(fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
session_iface::session_iface(void)
: m_io_service(nullptr)
, m_parent(nullptr)
, m_socket(nullptr)
, m_state({ static_cast<int>(state::none) })
{
	memset(&m_recv_data, 0, sizeof(per_io_data));
	memset(&m_send_data, 0, sizeof(per_io_data));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
fd_t
session_iface::get_fd(void)
{
	return m_socket->get_fd();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
socket_iface*
session_iface::get_socket(void)
{
	return m_socket;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
session_iface::close(reason st)
{
	int exp = static_cast<int>(state::connected);
	std::lock_guard<std::recursive_mutex> lock(m_close_mutex);
	if (!m_state.compare_exchange_strong(exp, static_cast<int>(state::closing)))
		return;

	m_io_service->untrack_session(this);
	m_socket->close();
	m_state = static_cast<int>(state::none);

	++m_compkey;
	m_parent->post_request(this,nullptr,&st);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void 
session_iface::handle_error(std::uint32_t compkey)
{
	if (m_compkey == compkey)
		close(reason::cs_handle_error);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
session_iface::is_connected(void)
{
	return m_state == static_cast<int>(state::connected);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
session_iface::set_connected(framework* parent, fd_t fd, sockaddr_storage* addr)
{
	m_state = static_cast<int>(state::connected);
	m_parent = parent;

	if (fd == INVALID_SOCKET)
		return;

	m_socket->set_fd(fd, addr);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 