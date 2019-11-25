#include "UtilityTest.hpp"
#include "mem_data_factory.hpp"

using namespace Utility;
void UtilityTest::_DataFactory()
{
	{
		std::cout << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl << "// mem::data_factory" << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl;

		mem::data_factory<int, 100> _data_factory1;
		mem::data_factory<int> _data_factory2;
		_data_factory2.init(10);

		int* a = _data_factory1.malloc();
		int* b = _data_factory2.malloc();

		bool r1 = _data_factory1.free(b);
		bool r2 = _data_factory1.free(a);
		bool r3 = _data_factory1.free(a);
		bool r4 = _data_factory1.free(b);

	}
}