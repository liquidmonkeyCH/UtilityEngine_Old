#include "msg_confirmer_rotative_buffer.hpp"
#include "mem_stream_buffer.hpp"

#include <vector>


namespace Utility
{
	template<int a>
	class test;

	template<>
	class test<1>
	{
	public:
		int a;
	};


	namespace msg
	{
		void test()
		{
			comfirmer_len<mem::rotative_buffer, 4096> aasdasdasd;
			mem::rotative_buffer m_buffer;
			unsigned long size = 1000;
			aasdasdasd.is_done(&m_buffer, size);

			comfirmer_len<mem::stream_buffer, 4096> confirmer1;
			//comfirmer_delimiter<mem::rotative_buffer, 4096> confirmer;
		}
	}
}
