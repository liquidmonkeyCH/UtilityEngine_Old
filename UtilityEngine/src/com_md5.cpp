/**
* @file com_md5.cpp
*
* @author Hourui (liquidmonkey)
**/

#include "com_md5.hpp"

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
///////////////////////////////////////////////////////////////////////////////////////////////////
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21
// F, G, H and I are basic md5 functions.
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
/* TRANSFORM transformations for rounds 1, 2, 3, and 4. N in F G H I.
Rotation is separate from addition to prevent recomputation. */
#define TRANSFORM(N, a, b, c, d, x, s, ac){ \
(a) += N ((b), (c), (d)) + (x) + ac; \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Construct
md5::md5(void) { reset(); }
md5::md5(const void* input, size_t length) { reset(); update(input, length); }
///////////////////////////////////////////////////////////////////////////////////////////////////
// Reset the calculate state 
void md5::reset() {
	_finished = false;
	// reset number of bits.
	_count[0] = _count[1] = 0;
	// Load magic initialization constants.
	_state[0] = 0x67452301;
	_state[1] = 0xefcdab89;
	_state[2] = 0x98badcfe;
	_state[3] = 0x10325476;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/* md5 block update operation. Continues an md5 message-digest
operation, processing another message block, and updating the
context.
*/
void md5::update(const void* input, size_t length) {
	unsigned long i, index, partLen;
	_finished = false;
	/* Compute number of uchar8s mod 64 */
	index = (unsigned long)((_count[0] >> 3) & 0x3f);
	/* update number of bits */
	if ((_count[0] += ((unsigned long)length << 3)) < ((unsigned long)length << 3)) {
		++_count[1];
	}
	_count[1] += ((unsigned long)length >> 29);
	partLen = 64 - index;
	/* transform as many times as possible. */
	if (length >= partLen) {
		memcpy(&_buffer[index], input, partLen);
		transform(_buffer);
		for (i = partLen; i + 63 < length; i += 64) {
			transform((const unsigned char*)input + i);
		}
		index = 0;
	}
	else {
		i = 0;
	}
	/* Buffer remaining input */
	memcpy(&_buffer[index], (const unsigned char*)input + i, length - i);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Return the message-digest
const unsigned char* md5::digest() {
	if (!_finished) {
		_finished = true;
		final();
	}
	return _digest;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/* md5 finalization. Ends an md5 message-_digest operation, writing the
the message _digest and zeroizing the context.
*/
void md5::final() {
	unsigned char bits[8];
	unsigned long oldState[4];
	unsigned long oldCount[2];
	unsigned long index, padLen;
	/* Save current state and count. */
	memcpy(oldState, _state, 16);
	memcpy(oldCount, _count, 8);
	/* Save number of bits */
	encode(_count, bits, 8);
	/* Pad out to 56 mod 64. */
	index = (unsigned long)((_count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	update(PADDING, padLen);
	/* Append length (before padding) */
	update(bits, 8);
	/* Store state in digest */
	encode(_state, _digest, 16);
	/* Restore current state and count. */
	memcpy(_state, oldState, 16);
	memcpy(_count, oldCount, 8);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/* md5 basic transformation. Transforms _state based on block. */
void md5::transform(const unsigned char block[64]) {
	unsigned long a = _state[0], b = _state[1], c = _state[2], d = _state[3], x[16];
	decode(block, x, 64);
	/* Round 1 */
	TRANSFORM(F, a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
	TRANSFORM(F, d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
	TRANSFORM(F, c, d, a, b, x[2], S13, 0x242070db); /* 3 */
	TRANSFORM(F, b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
	TRANSFORM(F, a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
	TRANSFORM(F, d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
	TRANSFORM(F, c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
	TRANSFORM(F, b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
	TRANSFORM(F, a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
	TRANSFORM(F, d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
	TRANSFORM(F, c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	TRANSFORM(F, b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	TRANSFORM(F, a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	TRANSFORM(F, d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	TRANSFORM(F, c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	TRANSFORM(F, b, c, d, a, x[15], S14, 0x49b40821); /* 16 */
	/* Round 2 */
	TRANSFORM(G, a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
	TRANSFORM(G, d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
	TRANSFORM(G, c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	TRANSFORM(G, b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
	TRANSFORM(G, a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
	TRANSFORM(G, d, a, b, c, x[10], S22, 0x2441453); /* 22 */
	TRANSFORM(G, c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	TRANSFORM(G, b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
	TRANSFORM(G, a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
	TRANSFORM(G, d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	TRANSFORM(G, c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
	TRANSFORM(G, b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
	TRANSFORM(G, a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	TRANSFORM(G, d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
	TRANSFORM(G, c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
	TRANSFORM(G, b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */
	/* Round 3 */
	TRANSFORM(H, a, b, c, d, x[5], S31, 0xfffa3942); /* 33 */
	TRANSFORM(H, d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
	TRANSFORM(H, c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	TRANSFORM(H, b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	TRANSFORM(H, a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
	TRANSFORM(H, d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
	TRANSFORM(H, c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
	TRANSFORM(H, b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	TRANSFORM(H, a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	TRANSFORM(H, d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
	TRANSFORM(H, c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
	TRANSFORM(H, b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
	TRANSFORM(H, a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
	TRANSFORM(H, d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	TRANSFORM(H, c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	TRANSFORM(H, b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */
	/* Round 4 */
	TRANSFORM(I, a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
	TRANSFORM(I, d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
	TRANSFORM(I, c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	TRANSFORM(I, b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
	TRANSFORM(I, a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	TRANSFORM(I, d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
	TRANSFORM(I, c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	TRANSFORM(I, b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
	TRANSFORM(I, a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
	TRANSFORM(I, d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	TRANSFORM(I, c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
	TRANSFORM(I, b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	TRANSFORM(I, a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
	TRANSFORM(I, d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	TRANSFORM(I, c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
	TRANSFORM(I, b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */
	_state[0] += a;
	_state[1] += b;
	_state[2] += c;
	_state[3] += d;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/* Encodes input (ulong) into output (uchar8). Assumes length is
a multiple of 4.
*/
void md5::encode(const unsigned long* input, unsigned char* output, size_t length) {
	for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j + 1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/* Decodes input (uchar8) into output (ulong). Assumes length is
a multiple of 4.
*/
void md5::decode(const unsigned char* input, unsigned long* output, size_t length) {
	for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
		output[i] = ((unsigned long)input[j]) | (((unsigned long)input[j + 1]) << 8) |
			(((unsigned long)input[j + 2]) << 16) | (((unsigned long)input[j + 3]) << 24);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const char*
md5::c_str(void)
{ return _finished ? _result : _impl::bin_to_hex<false>(_result, digest(), 16); }
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility