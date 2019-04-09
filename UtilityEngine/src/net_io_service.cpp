/**
* @file net_io_service.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "net_io_service.hpp"
#include "net_session.hpp"
#include "net_server.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// session_iface
void io_service_iface::bind(session_iface* session){ session->m_io_service = this; }
per_io_data* io_service_iface::get_read_data(session_iface* session){ return &session->m_read_data; }
per_io_data* io_service_iface::get_send_data(session_iface* session){ return &session->m_send_data; }
int io_service_iface::process_read(session_iface* session, size_t size){ return session->process_read(size); }
int io_service_iface::process_send(session_iface* session, size_t size){ return session->process_send(size); }
// server_iface
void io_service_iface::process_accept(server_iface* server, per_io_data* data, 
	sockaddr_storage* addr, session_iface** session){ server->process_accept(data, addr, session); }
accept_data* io_service_iface::get_accept_data(server_iface* server){ return server->get_accept_data(); }
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 