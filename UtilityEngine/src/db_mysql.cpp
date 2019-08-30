/**
* @file db_mysqlpp.cpp
*
* @author Hourui (liquidmonkey)
**/

#include "db_mysql.hpp"

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace db
{
///////////////////////////////////////////////////////////////////////////////////////////////////
mysql::connection_pool::connection_pool(const char* db,
	const char* user,
	const char* password,
	const char* server,
	unsigned int port,
	const char* charset,
	unsigned int idel) :
	m_db(db ? db : ""),
	m_server(server ? server : ""),
	m_user(user ? user : ""),
	m_password(password ? password : ""),
	m_port(port),
	m_charset(charset ? charset : ""),
	m_idel(idel)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
mysql::connection_pool::~connection_pool()
{
	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
mysqlpp::Connection*
mysql::connection_pool::create()
{
	mysqlpp::Connection* connection_ptr = m_pool.malloc();
	connection_ptr->set_option(new mysqlpp::MultiStatementsOption(true));
	if (!m_charset.empty())
		connection_ptr->set_option(new mysqlpp::SetCharsetNameOption(m_charset));
	connection_ptr->connect(m_db.empty() ? 0 : m_db.c_str(),
		m_server.empty() ? 0 : m_server.c_str(),
		m_user.empty() ? 0 : m_user.c_str(),
		m_password.empty() ? 0 : m_password.c_str(),
		m_port);

	return connection_ptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void
mysql::connection_pool::destroy(mysqlpp::Connection* connection_ptr)
{
	m_pool.free(connection_ptr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int
mysql::connection_pool::max_idle_time()
{
	return m_idel;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DB_mysqlpp
///////////////////////////////////////////////////////////////////////////////////////////////////
mysql::mysql(void)
{
	m_pool.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
mysql::~mysql(void)
{
	POOL_MAP::iterator it = m_pool.begin();
	for (; it != m_pool.end(); ++it)
	{
		delete it->second;
		it->second = nullptr;
	}

	m_pool.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
mysql::connection_pool*
mysql::pool(const char* dsn)
{
	POOL_MAP::iterator it = m_pool.find(dsn);

	if (it == m_pool.end())
		Clog::error_throw_no(errors::unknow_dsn,"dsn(%s) not reg!",dsn);

	return it->second;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void
mysql::init_dsn(const char* dsn, const char* db, const char* server, const char* user,
		const char* password, const char* charset, unsigned int port, unsigned int idel)
{
	if (!dsn || !db || !server || !user || !password)
		Clog::error_throw_no(errors::bad_param, "init dsn param error!");

	POOL_MAP::iterator it = m_pool.find(dsn);

	if (it != m_pool.end())
		Clog::error_throw_no(errors::duplicate_dsn, "init dsn(%s) duplicate!",dsn);

	connection_pool* pool = new connection_pool(db, server, user, password, port, charset, idel);

	m_key_list.push_back(dsn);
	m_pool.insert(std::make_pair(m_key_list.back().c_str(), pool));

	connection_guard guard(pool, pool->grab());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace db
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility