/**
* @file db_mysql.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __DB_MYSQL_HPP__
#define __DB_MYSQL_HPP__

#include "mysql++.h"
#include "logger.hpp"
#include <mutex>
#include <string>
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
	enum class dsn_t;
	enum class errors {
		unknown_dsn,
		duplicate_dsn,
		bad_params,
		connect_fail,
	};
private:
	mysql(void) = default;
	~mysql(void) = default;

	mysql(const mysql&) = delete;
	mysql& operator=(const mysql&) = delete;
	mysql(const mysql&&) = delete;
	mysql& operator=(const mysql&&) = delete;

	template<dsn_t dsn>
	class connection_pool : public mysqlpp::ConnectionPool
	{
	public:
		connection_pool(const char* dbname, const char* user, const char* password,
			const char* server, unsigned int port, const char* charset, unsigned int idel)
			: m_db(dbname ? dbname : "")
			, m_server(server ? server : "")
			, m_user(user ? user : "")
			, m_password(password ? password : "")
			, m_port(port)
			, m_charset(charset ? charset : "")
			, m_idel(idel) {}

		~connection_pool() { clear(); }
	protected:
		mysqlpp::Connection* create()
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

		void destroy(mysqlpp::Connection* connection)
		{
			m_pool.free(connection);
		}

		unsigned int max_idle_time()
		{
			return m_idel;
		}
	private:
		std::string m_charset;
		std::string m_db;
		std::string m_server;
		std::string m_user;
		std::string m_password;
		unsigned int m_port;
		unsigned int m_idel;

		mem::memory_pool_ex<mysqlpp::Connection, 5, mem::release_mode::auto1chunk, mem::alloc_mode::cache, 5> m_pool;
	};

	template<dsn_t dsn>
	struct refence
	{
		refence(void) :m_pool(nullptr) {}
		~refence(void) { delete m_pool; m_pool = nullptr; }

		connection_pool<dsn>* m_pool;
		std::mutex m_mutex;
	};

	template<dsn_t dsn>
	static refence<dsn>* get_ref(void)
	{
		static refence<dsn> ref;
		return &ref;
	}

	template<dsn_t dsn>
	static connection_pool<dsn>* get_connection_pool(void)
	{
		refence<dsn>* ref = get_ref<dsn>();
		if (!ref->m_pool)
			Clog::throw_error(errors::unknown_dsn, "dsn(%d) not reg!", dsn);

		return ref->m_pool;
	}

	template<dsn_t dsn>
	struct release_connection
	{
		release_connection(connection_pool<dsn>* pool) :m_pool(pool) {}
		~release_connection() {}

		void operator()(const mysqlpp::Connection* p)
		{
			if(p) m_pool->release(p);
			mysqlpp::Connection::thread_end();
		}
		connection_pool<dsn>* m_pool;
	};
public:
	using QueryResult = mysqlpp::StoreQueryResult;
	using ExecuteResult = mysqlpp::SimpleResult;

	template<dsn_t dsn>
	static void init(const char* dbname, const char* user, const char* password, const char* server,
		unsigned int port = 3306, const char* charset = "gbk",  unsigned int idel = 3 * 60)
	{
		if (!dbname || !server || !user || !password)
			Clog::throw_error(errors::bad_params, "init dsn(%d) params error!",dsn);

		refence<dsn>* ref = mysql::get_ref<dsn>();
		std::lock_guard<std::mutex> lock(ref->m_mutex);
		if(ref->m_pool)
			Clog::throw_error(errors::duplicate_dsn, "init dsn(%d) duplicate!", dsn);

		ref->m_pool = new connection_pool<dsn>(dbname, user, password, server, port, charset, idel);

		try 
		{
			auto connection = get_connection<dsn>();
		}
		catch (std::exception& e)
		{
			delete ref->m_pool;
			ref->m_pool = nullptr;
			Clog::throw_error(errors::connect_fail,"dsn(%d) %s" ,dsn,e.what());
		}
	}

	template<dsn_t dsn>
	static std::unique_ptr<mysqlpp::Connection, release_connection<dsn>> get_connection(void)
	{
		connection_pool<dsn>* pool = get_connection_pool<dsn>();
		return std::unique_ptr<mysqlpp::Connection, release_connection<dsn>>(pool->grab(), release_connection<dsn>(pool));
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace db
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Utility
////////////////////////////////////////////////////////////////////////////////////////////////////
#define DATABASE_MYSQLPP_BEGIN(DSN)																	\
{																									\
	mysqlpp::Connection::thread_start();															\
	try																								\
	{																								\
		auto utility_db_mysql_connection = Utility::db::mysql::get_connection<DSN>();				\
		mysqlpp::Query utility_db_mysql_query = utility_db_mysql_connection->query();

#define DATABASE_MYSQLPP_END()																		\
	}																								\
	catch(std::exception& e)																		\
	{																								\
		Utility::Clog::error(e.what());																\
	}																								\
}

#define DATABASE_MYSQLPP_EXECUTE(res,sql)															\
	utility_db_mysql_query << sql;																	\
	res = utility_db_mysql_query.execute();

#define DATABASE_MYSQLPP_QUERY_BEGIN(res,sql)														\
	utility_db_mysql_query << sql;																	\
	res = utility_db_mysql_query.store();

#define DATABASE_MYSQLPP_QUERY_NEXT(res)															\
	res = utility_db_mysql_query.store_next();

#define DATABASE_MYSQLPP_QUERY_END()																\
	while(utility_db_mysql_query.more_results())													\
		utility_db_mysql_query.store_next();

#define DATABASE_MYSQLPP_QUERY(res,sql)																\
DATABASE_MYSQLPP_QUERY_BEGIN(res,sql)																\
DATABASE_MYSQLPP_QUERY_END()

#define DATABASE_MYSQLPP_TRANSACTION_BEGIN()														\
{																									\
	mysqlpp::Transaction utility_db_mysql_transaction(*utility_db_mysql_connection);

#define DATABASE_MYSQLPP_TRANSACTION_COMMIT()														\
	utility_db_mysql_transaction.commit();

#define DATABASE_MYSQLPP_TRANSACTION_END()															\
}
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __DB_MYSQL_HPP__