#include "msg_message.hpp"
#include "mem_stream_buffer.hpp"
#include "mem_rotative_buffer.hpp"

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
			len::message_wrap<mem::rotative_buffer, 4096> aasdasdasd;
			unsigned long size;
			aasdasdasd.comfirm(size);
			aasdasdasd.commit();
			aasdasdasd.get_pos

			
			//comfirmer_delimiter<mem::rotative_buffer, 4096> confirmer;
		}
	}
}
