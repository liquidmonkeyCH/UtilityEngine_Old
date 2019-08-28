#include <stdio.h>
#include <iostream>

#include "UtilityTest.hpp"
#include "logger.hpp"
using namespace Utility;

int main(int argc, char** argv)
{
	logger klogger(logger::log_level::debug);
	Clog::active_logger(&klogger);

	UtilityTest::_Service();
	UtilityTest::_Singleton();

	std::cout << static_cast<unsigned short>((1437439665242112 & 0x0000ffff00000000ULL) >> 32) << std::endl;
	Clog::debug("test done!");
#ifdef _WIN32
	system("pause");
#endif
	return 0;
}

