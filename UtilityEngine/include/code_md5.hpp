/**
* @file code_md5.hpp
*
*/

#ifndef CODE_MD5_HPP
#define CODE_MD5_HPP

#include <fstream>
#include <string.h>
#include <string>

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////	
namespace code
{
///////////////////////////////////////////////////////////////////////////////////////////////////
class md5
{
public:
	const static int BUFFER_SIZE = 1024;
public:
	md5();
	md5(const void* input, size_t length);
	md5(const std::string& str);
	md5(std::ifstream& in);

	md5(const md5&) = delete;
	md5& operator=(const md5&) = delete;

	void update(const void* input, size_t length);
	void update(const std::string& str);
	void update(std::ifstream& in);
	const unsigned char* digest();
	std::string toString();
	void reset();
private:
	void update(const unsigned char* input, size_t length);
	void final();
	void transform(const unsigned char block[64]);
	void encode(const unsigned long* input, unsigned char* output, size_t length);
	void decode(const unsigned char* input, unsigned long* output, size_t length);
	std::string uchar8sToHexString(const unsigned char* input, size_t length);
private:
	unsigned long _state[4]; /* state (ABCD) */
	unsigned long _count[2]; /* number of bits, modulo 2^64 (low-order word first) */
	unsigned char _buffer[64]; /* input buffer */
	unsigned char _digest[16]; /* message digest */
	bool _finished; /* calculate finished ? */
	static const unsigned char PADDING[64]; /* padding for calculate */
	static const char HEX[16];
};
///////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace code
///////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility
#endif /*CODE_MD5_HPP*/
