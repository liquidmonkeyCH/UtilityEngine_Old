#include "UtilityTest.hpp"
#include "com_hex_caster.hpp"
#include "com_md5.hpp"

using namespace Utility;
void UtilityTest::_Other()
{
	{
		std::cout << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl << "// com::hex_caster" << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl;

		Clog::info("data(char[16]): 0,1,2,3,4,5,36,7,8,9,215,11,12,13,14,255, ");
		unsigned char data[16] = { 0,1,2,3,4,5,36,7,8,9,215,11,12,13,14,255 };
		auto mm = com::bin2hex<true>(data);
		Clog::debug("bin2hex:%s",mm.str());

		unsigned char res[16];
		com::hex2bin<true>(res, mm.str(), mm.m_len);
		Clog::debug("hex2bin(char[16]):");
		for(int i=0;i<16; ++i)
			std::cout << (unsigned int)res[i] << ",";

		std::cout << std::endl;
	}
	{
		std::cout << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl << "// com::md5" << std::endl
			<< "/////////////////////////////////////////////////////////////////////////"
			<< std::endl;
		
		Clog::info("data(char[16]): 0,1,2,3,4,5,36,7,8,9,215,11,12,13,14,255, ");
		unsigned char data[16] = { 0,1,2,3,4,5,36,7,8,9,215,11,12,13,14,255 };
		com::md5 md5_data(data, 16);
		Clog::debug("md5:%s",md5_data.c_str());

		Clog::info("data1(char[8]): 0,1,2,3,4,5,36,7, ");
		Clog::info("data2(char[8]): 8,9,215,11,12,13,14,255, ");
		unsigned char data1[8] = { 0,1,2,3,4,5,36,7 };
		unsigned char data2[8] = { 8,9,215,11,12,13,14,255 };
		md5_data.reset();
		md5_data.update(data1, 8);
		Clog::debug("data1->md5:%s" , md5_data.c_str() );
		md5_data.update(data2, 8);
		Clog::debug("data1+data2->md5:%s", md5_data.c_str());
	}
}