/**
* @file net_io_service.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "net_io_service.hpp"
#include "net_session.hpp"
#include "net_responder.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// session_iface
void io_service_iface::bind(session_iface* session){ session->m_io_service = this; }
per_io_data* io_service_iface::get_recv_data(session_iface* session){ return &session->m_recv_data; }
per_io_data* io_service_iface::get_send_data(session_iface* session){ return &session->m_send_data; }
bool io_service_iface::process_recv(session_iface* session, unsigned long size){ return session->process_recv(size); }
bool io_service_iface::process_send(session_iface* session, unsigned long size){ return session->process_send(size); }
// responder_iface
void io_service_iface::process_accept(responder_iface* server, per_io_data* data, 
	sockaddr_storage* addr, session_iface** session){ server->process_accept(data, addr, session); }
accept_data* io_service_iface::get_accept_data(responder_iface* server){ return server->get_accept_data(); }
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 