#include "UtilityTest.hpp"
#include "com_service.hpp"
///////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Utility;
template<class T>
using Service = com::wrap::Service<T>;
using ServiceManager = com::ServiceManager;
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ServiceTest : public com::iface::Service
{
	DECLARE_SERVICE_ID(ServiceTest)
public:
	
	void init1() { Clog::info("ServiceTest init!"); }
	int init2(int i) { return i*100; }
	bool init3() { return true; }

	void destory() { Clog::info("ServiceTest destory!"); }
};
ServiceTest::ServiceTest(){}
ServiceTest::~ServiceTest(){}
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Test
{
	void init1() { Clog::info("Test init!"); }
	int init2(int i) { return i * 100; }
	bool init3() { return true; }

	void destory() { Clog::info("Test destory!"); }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
void UtilityTest::_Service()
{
	std::cout << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl << "// Service" << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl;

	try
	{
		ServiceManager::Attach<ServiceTest>(&ServiceTest::init1);
		ServiceTest* pServiceTest = ServiceManager::GetService<ServiceTest>();
		ServiceManager::Detach<ServiceTest>(&ServiceTest::destory);

		pServiceTest = ServiceManager::GetService<ServiceTest>();

		int i = ServiceManager::Attach<ServiceTest>(&ServiceTest::init2, 10);
		Clog::debug("Attach ServiceTest::init2 return %d", i);
		bool res = ServiceManager::Detach<ServiceTest>(&ServiceTest::init3);
		Clog::debug("Detach ServiceTest::init3 return %s", res ? "true" : "false");

		ServiceManager::Attach<Test>(&Test::init1);
		Service<Test>* pService = ServiceManager::GetService<Test>();
		ServiceManager::Detach<Test>(&Test::destory);

		pService = ServiceManager::GetService<Test>();

		i = ServiceManager::Attach<Test>(&Test::init2, 10);
		Clog::debug("Attach Service<Test>::init2 return %d", i);
		res = ServiceManager::Detach<Test>(&Test::init3);
		Clog::debug("Detach Service<Test>::init3 return %s", res ? "true" : "false");
		
		try
		{
			ServiceManager::Attach<Test>(&Test::init1);
			ServiceManager::Attach<Test>(&Test::init3);
		}
		catch (utility_error& e)
		{
			Clog::error("ServiceManager::error::%lld", e.get_error());
		}

		try
		{
			ServiceManager::Detach<Test>(&Test::destory);
			ServiceManager::Detach<Test>(&Test::init3);
		}
		catch (utility_error& e)
		{
			Clog::error("ServiceManager::error::%lld", e.get_error());
		}

		
	}
	catch (utility_error& e)
	{
		Clog::error("ServiceManager::error::%lld", e.get_error());
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////