/*
	$ P2MP3  (C) 2005-2012 mfboy
	$ code.c, 0.1.1005
*/

#include "base.h"
#include "code.h"


#define F(x, y, z)	(((x) & (y)) | ((~x) & (z)))
#define G(x, y, z)	(((x) & (z)) | ((y) & (~z)))
#define H(x, y, z)	((x) ^ (y) ^ (z))
#define I(x, y, z)	((y) ^ ((x) | (~(z))))
#define RL(x, n)	(((x) << (n)) | ((x) >> (32 - (n))))

#define FF(a, b, c, d, x, s, ac) { a += F(b, c, d) + (x) + (ac); a = RL(a, s); a += b; }
#define GG(a, b, c, d, x, s, ac) { a += G(b, c, d) + (x) + (ac); a = RL(a, s); a += b; }
#define HH(a, b, c, d, x, s, ac) { a += H(b, c, d) + (x) + (ac); a = RL(a, s); a += b; }
#define II(a, b, c, d, x, s, ac) { a += I(b, c, d) + (x) + (ac); a = RL(a, s); a += b; }                                            


typedef struct {
	seek_t count[2];
	seek_t state[4];
	var_t buffer[64];
} md5_t;


static var_t md5_padding[] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


static void md5_init(md5_t *context);
static void md5_update(md5_t *context, var_t *input, seek_t length);
static void md5_final(md5_t *context, var_t digest[16]);
static void md5_encode(var_t *output, seek_t *input, seek_t length);
static void md5_decode(seek_t *output, var_t *input, seek_t length);
static void md5_transform(seek_t state[4], var_t block[64]);


char *rc4(char *source, const seek_t length, char *key) {
	seek_t i, j, k, w;
	var_t *sbox, tmp;
	char *skey, *target;

	m_new(sbox, var_t, 256);
	m_new(skey, char, 256);
	m_new(target, char, length);

	for(i = 0, j = (seek_t) strlen(key); i < 256; i++) {
		sbox[i] = (var_t) i;
		skey[i] = key[i % j];
	}

	for(i = j = 0; i < 256; i++) {
		j = (j + sbox[i] + skey[i]) % 256;

		tmp = sbox[i];

		sbox[i] = sbox[j];
		sbox[j] = sbox[i];
	}

	/* RC4 ±àÂë */
	for(i = j = k = 0; k < length; k++) {
		i = (i + 1) % 256;
		j = (j + sbox[i]) % 256;

		tmp = sbox[i];

		sbox[i] = sbox[j];
		sbox[j] = sbox[i];

		w = (sbox[i] + sbox[j]) % 256;

		target[k] = source[k] ^ sbox[w];
	}

	m_del(sbox);
	m_del(skey);

	return target;
}

crc32_t crc32(char *source, seek_t length) {
	static unsigned int *table;

	seek_t i, j, k;
	crc32_t crc = ~0;

	/* Éú³É¹þÏ£±í */
	if(table == NULL) {
		m_new(table, unsigned int, 256);

		for(i = 0; i < 256; i++) {
			for(k = i, j = 0; j < 8; j++) {
				if(k & 0x01) {
					k = 0xEDB88320L ^ (k >> 1);
				} else {
					k >>= 1;
				}
			}

			table[i] = k;
		}
	}

	while(length--) {
		crc = table[(crc ^ *source++) & 0xFF] ^ (crc >> 8);
	}

	return ~crc;
}

char *md5(char *source) {
	int i;
	char *result;

	var_t decrypt[16];
	md5_t md5;

	md5_init(&md5);
	md5_update(&md5, (unsigned char *) source, (seek_t) strlen(source));
	md5_final(&md5, decrypt);

	m_del(source);
	ms_new(result, 32);

	for(i = 0; i < 16; i++) {
		sprintf(result + (i * 2), "%02x", decrypt[i]);
	}

	return result;
}


static void md5_init(md5_t *context) {
	context->count[0] = 0;
	context->count[1] = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
}

static void md5_update(md5_t *context, var_t *input, seek_t length) {
	seek_t i = 0;

	seek_t index	= 0;
	seek_t partlen	= 0;

	index	= (context->count[0] >> 3) & 0x3F;
	partlen	= 64 - index;

	context->count[0] += length << 3;

	if(context->count[0] < (length << 3)) {
		context->count[1]++;
	}

	context->count[1] += length >> 29;

	if(length >= partlen) {
		m_copy(&context->buffer[index], input, partlen);

		md5_transform(context->state, context->buffer);

		for(i = partlen; i + 64 <= length; i += 64) {
			md5_transform(context->state, &input[i]);
		}

		index = 0;
	} else {
		i = 0;
	}

	m_copy(&context->buffer[index], &input[i], length - i);
}

static void md5_final(md5_t *context, var_t digest[16]) {
	seek_t index	= 0;
	seek_t padlen	= 0;

	var_t bits[8];

	index	= (context->count[0] >> 3) & 0x3F;
	padlen	= (index < 56) ? (56 - index) : (120 - index);

	md5_encode(bits, context->count, 8);
	md5_update(context, md5_padding, padlen);
	md5_update(context, bits, 8);
	md5_encode(digest, context->state, 16);
}

static void md5_encode(var_t *output, seek_t *input, seek_t length) {
	seek_t i = 0, j = 0;

    while(j < length) {
		output[j] = input[i] & 0xFF;
		output[j + 1] = (input[i] >> 8) & 0xFF;
		output[j + 2] = (input[i] >> 16) & 0xFF;
		output[j + 3] = (input[i] >> 24) & 0xFF;

		i++;
		j += 4;
	}
}

static void md5_decode(seek_t *output, var_t *input, seek_t length) {
	seek_t i = 0, j = 0;

	while(j < length) {
		output[i] = (input[j]) | (input[j + 1] << 8) | (input[j + 2] << 16) | (input[j + 3] << 24);

		i++;
		j += 4;
	}
}

static void md5_transform(seek_t state[4], var_t block[64]) {
	seek_t a = state[0];
	seek_t b = state[1];
	seek_t c = state[2];
	seek_t d = state[3];
	seek_t x[64];

	md5_decode(x, block, 64);

	FF(a, b, c, d, x[ 0],  7, 0xd76aa478);
	FF(d, a, b, c, x[ 1], 12, 0xe8c7b756);
	FF(c, d, a, b, x[ 2], 17, 0x242070db);
	FF(b, c, d, a, x[ 3], 22, 0xc1bdceee);
	FF(a, b, c, d, x[ 4],  7, 0xf57c0faf);
	FF(d, a, b, c, x[ 5], 12, 0x4787c62a);
	FF(c, d, a, b, x[ 6], 17, 0xa8304613);
	FF(b, c, d, a, x[ 7], 22, 0xfd469501);
	FF(a, b, c, d, x[ 8],  7, 0x698098d8);
	FF(d, a, b, c, x[ 9], 12, 0x8b44f7af);
	FF(c, d, a, b, x[10], 17, 0xffff5bb1);
	FF(b, c, d, a, x[11], 22, 0x895cd7be);
	FF(a, b, c, d, x[12],  7, 0x6b901122);
	FF(d, a, b, c, x[13], 12, 0xfd987193);
	FF(c, d, a, b, x[14], 17, 0xa679438e);
	FF(b, c, d, a, x[15], 22, 0x49b40821);

	GG(a, b, c, d, x[ 1],  5, 0xf61e2562);
	GG(d, a, b, c, x[ 6],  9, 0xc040b340);
	GG(c, d, a, b, x[11], 14, 0x265e5a51);
	GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa);
	GG(a, b, c, d, x[ 5],  5, 0xd62f105d);
	GG(d, a, b, c, x[10],  9,  0x2441453);
	GG(c, d, a, b, x[15], 14, 0xd8a1e681);
	GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8);
	GG(a, b, c, d, x[ 9],  5, 0x21e1cde6);
	GG(d, a, b, c, x[14],  9, 0xc33707d6);
	GG(c, d, a, b, x[ 3], 14, 0xf4d50d87);
	GG(b, c, d, a, x[ 8], 20, 0x455a14ed);
	GG(a, b, c, d, x[13],  5, 0xa9e3e905);
	GG(d, a, b, c, x[ 2],  9, 0xfcefa3f8);
	GG(c, d, a, b, x[ 7], 14, 0x676f02d9);
	GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);
 
	HH(a, b, c, d, x[ 5],  4, 0xfffa3942);
	HH(d, a, b, c, x[ 8], 11, 0x8771f681);
	HH(c, d, a, b, x[11], 16, 0x6d9d6122);
	HH(b, c, d, a, x[14], 23, 0xfde5380c);
	HH(a, b, c, d, x[ 1],  4, 0xa4beea44);
	HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9);
	HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60);
	HH(b, c, d, a, x[10], 23, 0xbebfbc70);
	HH(a, b, c, d, x[13],  4, 0x289b7ec6);
	HH(d, a, b, c, x[ 0], 11, 0xeaa127fa);
	HH(c, d, a, b, x[ 3], 16, 0xd4ef3085);
	HH(b, c, d, a, x[ 6], 23,  0x4881d05);
	HH(a, b, c, d, x[ 9],  4, 0xd9d4d039);
	HH(d, a, b, c, x[12], 11, 0xe6db99e5);
	HH(c, d, a, b, x[15], 16, 0x1fa27cf8);
	HH(b, c, d, a, x[ 2], 23, 0xc4ac5665);
 
	II(a, b, c, d, x[ 0],  6, 0xf4292244);
	II(d, a, b, c, x[ 7], 10, 0x432aff97);
	II(c, d, a, b, x[14], 15, 0xab9423a7);
	II(b, c, d, a, x[ 5], 21, 0xfc93a039);
	II(a, b, c, d, x[12],  6, 0x655b59c3);
	II(d, a, b, c, x[ 3], 10, 0x8f0ccc92);
	II(c, d, a, b, x[10], 15, 0xffeff47d);
	II(b, c, d, a, x[ 1], 21, 0x85845dd1);
	II(a, b, c, d, x[ 8],  6, 0x6fa87e4f);
	II(d, a, b, c, x[15], 10, 0xfe2ce6e0);
	II(c, d, a, b, x[ 6], 15, 0xa3014314);
	II(b, c, d, a, x[13], 21, 0x4e0811a1);
	II(a, b, c, d, x[ 4],  6, 0xf7537e82);
	II(d, a, b, c, x[11], 10, 0xbd3af235);
	II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb);
	II(b, c, d, a, x[ 9], 21, 0xeb86d391);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}
