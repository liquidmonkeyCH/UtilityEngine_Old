#include "UtilityTest.hpp"
#include "com_service.hpp"

template<const char* T>
struct test
{
	using value = T;
	int a;
};


void UtilityTest::_Mysql()
{
	test<"haha"> a;

}

/*#include "db_mysql.hpp"

using namespace Utility;
template<class T>
using Service = com::wrap::Service<T>;
using ServiceManager = com::ServiceManager;

void DbInit(Service<db::mysql>* pService)
{
	pService->init_dsn("GAME","CrossGate_Game","10.0.0.35","root","3596034");
}

void UtilityTest::_Mysql()
{
	std::cout << std::endl 
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl << "// db_mysqlpp" << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl;

	ServiceManager::Attach<db::mysql>(DbInit);
	ServiceManager::Attach<db::mysql>(&db::mysql::init_dsn, "GAME", "CrossGate_Game", "10.0.0.35", "root", "3596034", "gbk", 3306, 180);
		
	DATABASE_MYSQLPP_BEGIN(GAME,ServiceManager::GetService<db::mysql>())
		db::mysql::QueryResult res;
		int Count = 0;
		DATABASE_MYSQLPP_QUERY(res,"SELECT COUNT(*) FROM DS_USER_INFO;");
		if(!res.empty())
			Count = res[0][0];
				
			Clog::debug("%d", Count);
			
	DATABASE_MYSQLPP_END()
		
	ServiceManager::Detach<db::mysql>();
}*/