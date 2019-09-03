#include "UtilityTest.hpp"
#include "com_service.hpp"

#include "db_mysqlpp.hpp"

#include <memory>

using namespace Utility;

enum class db::mysql::dsn_t
{
	GAME_DB,
};

using DSN = db::mysql::dsn_t;

void UtilityTest::_Mysql()
{
	std::cout << std::endl 
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl << "// db_mysqlpp" << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl;
	try 
	{
		db::mysql::init<DSN::GAME_DB>("CrossGate_Game", "root", "3596034", "192.168.56.102", 3306, "gbk", 180);
	}
	catch (utility_error& e)
	{

	}
	
	DATABASE_MYSQLPP_BEGIN(DSN::GAME_DB)
		db::mysql::QueryResult res;
		int Count = 0;
		DATABASE_MYSQLPP_QUERY(res,"SELECT COUNT(*) FROM DS_USER_INFO;");
		if(!res.empty())
			Count = res[0][0];
				
			Clog::debug("%d", Count);
			
	DATABASE_MYSQLPP_END()
}