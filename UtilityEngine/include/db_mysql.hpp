/**
* @file db_mysql.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __DB_MYSQL_HPP__
#define __DB_MYSQL_HPP__

#include "mysql++.h"
#include "logger.hpp"
#include "com_less.hpp"
#include "mem_pool.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace db
{ 
////////////////////////////////////////////////////////////////////////////////////////////////////
class mysql
{
public:
	enum class errors {
		unknow_dsn,
		duplicate_dsn,
		bad_param,
	};

	class connection_pool : public mysqlpp::ConnectionPool
	{
	public:
		connection_pool(const char* db, const char* user, const char* password,
			const char* server, unsigned int port, const char* charset, unsigned int idel);

		~connection_pool();
	protected:
		mysqlpp::Connection* create();
		void destroy(mysqlpp::Connection* connection);
		unsigned int max_idle_time();
	private:
		std::string m_charset;
		std::string m_db;
		std::string m_server;
		std::string m_user;
		std::string m_password;
		unsigned int m_port;
		unsigned int m_idel;

		mem::memory_pool_ex<mysqlpp::Connection, 5,mem::release_mode::auto1chunk,mem::alloc_mode::cache,5> m_pool;
	};

	class connection_guard
	{
	public:
		connection_guard(connection_pool* pool, mysqlpp::Connection* connection) :m_pool(pool), m_connnection(connection) {}
		~connection_guard(void) { if (m_connnection) m_pool->release(m_connnection); mysqlpp::Connection::thread_end();}

		connection_guard(const connection_guard&&) = delete;
		connection_guard& operator=(const connection_guard&&) = delete;
		connection_guard(const connection_guard&) = delete;
		connection_guard& operator=(const connection_guard&) = delete;

		mysqlpp::Connection* operator->() { return m_connnection; }
	private:
		connection_pool* m_pool;
		mysqlpp::Connection* m_connnection;
	};

	using QueryResult = mysqlpp::StoreQueryResult;
	using ExecuteResult = mysqlpp::SimpleResult;
public:
	mysql(void);
	~mysql(void);

	void init_dsn(const char* dsn, const char* db, const char* server, const char* user, const char* password,
		const char* charset = "gbk", unsigned int port = 3306, unsigned int idel = 3 * 60);

	connection_pool* pool(const char* dsn);
private:
	using POOL_MAP = std::map<const char*, connection_pool*, com::strless>;
	using KEY_LIST = std::list<std::string>;

	POOL_MAP m_pool;
	KEY_LIST m_key_list;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace db
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Utility
////////////////////////////////////////////////////////////////////////////////////////////////////
#define DATABASE_MYSQLPP_BEGIN(DSN,MYSQL_PTR)														\
{																									\
	mysqlpp::Connection::thread_start();															\
	try																								\
	{																								\
		Utility::db::mysql::connection_pool* utility_db_mysql_connection_pool = MYSQL_PTR->pool(#DSN);														\
		Utility::db::mysql::connection_guard mysql_connection_guard(utility_db_mysql_connection_pool, utility_db_mysql_connection_pool->grab());			\
		mysqlpp::Query utility_db_mysql_query = mysql_connection_guard->query();

#define DATABASE_MYSQLPP_END()																		\
	}																								\
	catch(std::exception& e)																		\
	{																								\
		Utility::Clog::error(e.what());																\
	}																								\
}

#define DATABASE_MYSQLPP_EXECUTE(res,sql)															\
		utility_db_mysql_query << sql;																\
		res = utility_db_mysql_query.execute();

#define DATABASE_MYSQLPP_QUERY_BEGIN(res,sql)														\
		utility_db_mysql_query << sql;																\
		res = utility_db_mysql_query.store();

#define DATABASE_MYSQLPP_QUERY_NEXT(res)															\
		res = utility_db_mysql_query.store_next();

#define DATABASE_MYSQLPP_QUERY_END()																\
			while(utility_db_mysql_query.more_results())											\
				utility_db_mysql_query.store_next();

#define DATABASE_MYSQLPP_QUERY(res,sql)																\
		DATABASE_MYSQLPP_QUERY_BEGIN(res,sql)														\
		DATABASE_MYSQLPP_QUERY_END()

#define DATABASE_MYSQLPP_TRANSACTION_BEGIN()														\
		{																							\
			mysqlpp::Transaction utility_db_mysql_transaction(*mysql_connection_guard);

#define DATABASE_MYSQLPP_TRANSACTION_COMMIT()														\
			utility_db_mysql_transaction.commit();

#define DATABASE_MYSQLPP_TRANSACTION_END()															\
		}
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __DB_MYSQL_HPP__