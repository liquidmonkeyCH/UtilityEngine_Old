#include "UtilityTest.hpp"
#include "com_singleton.hpp"

using namespace Utility;

namespace Singleton_Test
{
	struct Test
	{
		Test()
		{
			Clog::debug("Singleton<Test> create!");
		}

		void init()
		{
			Clog::debug("Singleton<Test> call init");
		}
	};

	struct SingletonTest : public com::iface::Singleton<SingletonTest>
	{
		SingletonTest()
		{
			Clog::debug("SingletonTest create!");
		}

		void init()
		{
			Clog::debug("SingletonTest call init");
		}
	};
}
using namespace Singleton_Test;
template<class T>
using Singleton = com::wrap::Singleton<T>;

void UtilityTest::_Singleton()
{
	std::cout << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl << "// Singleton" << std::endl
		<< "/////////////////////////////////////////////////////////////////////////"
		<< std::endl;

	Singleton<Test>* p = Singleton<Test>::GetInstance();
	p->init();

	SingletonTest* pSingleton = SingletonTest::GetInstance();
	pSingleton->init();
}