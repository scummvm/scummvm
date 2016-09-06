/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "image/codecs/indeo/get_bits.h"
#include "common/algorithm.h"
#include "common/endian.h"
#include "common/textconsole.h"

/* Intel Indeo 4 bitstream reader
 *
 * Original copyright note:
 * Copyright (c) 2004 Michael Niedermayer
 */

namespace Image {
namespace Indeo {

/* Macro documentation
 * name
 *   arbitrary name which is used as prefix for local variables
 *
 * OPEN_READER(name)
 *   load index into local variable
 *
 * CLOSE_READER(name)
 *   store local index back into class state
 *
 * UPDATE_CACHE(name)
 *   Refill the internal cache from the bitstream.
 *   After this call at least MIN_CACHE_BITS will be available.
 *
 * GET_CACHE(name)
 *   Will output the contents of the internal cache,
 *   next bit is MSB of 32 or 64 bits (FIXME 64 bits).
 *
 * SHOW_UBITS(name, num)
 *   Will return the next num bits.
 *
 * SHOW_SBITS(name, num)
 *   Will return the next num bits and do sign extension.
 *
 * SKIP_BITS(name, num)
 *   Will skip over the next num bits.
 *   Note, this is equivalent to SKIP_CACHE; SKIP_COUNTER.
 *
 * SKIP_CACHE(name, num)
 *   Will remove the next num bits from the cache (note SKIP_COUNTER
 *   MUST be called before UPDATE_CACHE / CLOSE_READER).
 *
 * SKIP_COUNTER(name, num)
 *   Will increment the internal bit counter (see SKIP_CACHE & SKIP_BITS).
 *
 * LAST_SKIP_BITS(name, num)
 *   Like SKIP_BITS, to be used if next call is UPDATE_CACHE or CLOSE_READER.
 *
 * BITS_LEFT(name)
 *   Return the number of bits left
 *
 * For examples see getBits, show_bits, skip_bits, get_vlc.
 */
#define BITSTREAM_READER_LE

#ifdef LONG_BITSTREAM_READER
#   define MIN_CACHE_BITS 32
#else
#   define MIN_CACHE_BITS 25
#endif

#define NEG_USR32(a,s) (((uint32)(a)) >> (32 -(s)))

#define OPEN_READER_NOSIZE(name)           \
    unsigned int name ## _index = _index;  \
    unsigned int name ## _cache

#define OPEN_READER(name) OPEN_READER_NOSIZE(name)
#define BITS_AVAILABLE(name) 1

#define CLOSE_READER(name) _index = name ## _index

# ifdef LONG_BITSTREAM_READER
# define UPDATE_CACHE_LE(name) name ## _cache = \
      AV_RL64(_buffer + (name ## _index >> 3)) >> (name ## _index & 7)

# define UPDATE_CACHE_BE(name) name ## _cache = \
      AV_RB64(_buffer + (name ## _index >> 3)) >> (32 - (name ## _index & 7))
#else
# define UPDATE_CACHE_LE(name) name ## _cache = \
      READ_LE_UINT32(_buffer + (name ## _index >> 3)) >> (name ## _index & 7)

# define UPDATE_CACHE_BE(name) name ## _cache = \
      AV_RB32(_buffer + (name ## _index >> 3)) << (name ## _index & 7)
#endif

#ifdef BITSTREAM_READER_LE
# define UPDATE_CACHE(name) UPDATE_CACHE_LE(name)
# define SKIP_CACHE(name, num) name ## _cache >>= (num)
#else
# define UPDATE_CACHE(name) UPDATE_CACHE_BE(name)
# define SKIP_CACHE(name, num) name ## _cache <<= (num)
#endif

#define SKIP_COUNTER(name, num) name ## _index += (num)

#define BITS_LEFT(name) ((int)(_size_in_bits - name ## _index))

#define SKIP_BITS(name, num)                \
    do {                                        \
        SKIP_CACHE(name, num);              \
        SKIP_COUNTER(name, num);            \
    } while (0)

#define LAST_SKIP_BITS(name, num) SKIP_COUNTER(name, num)

#define SHOW_UBITS_LE(name, num) zeroExtend(name ## _cache, num)
#define SHOW_SBITS_LE(name, num) signExtend(name ## _cache, num)

#define SHOW_UBITS_BE(name, num) NEG_USR32(name ## _cache, num)
#define SHOW_SBITS_BE(name, num) NEG_SSR32(name ## _cache, num)

#ifdef BITSTREAM_READER_LE
#   define SHOW_UBITS(name, num) SHOW_UBITS_LE(name, num)
#   define SHOW_SBITS(name, num) SHOW_SBITS_LE(name, num)
#else
#   define SHOW_UBITS(name, num) SHOW_UBITS_BE(name, num)
#   define SHOW_SBITS(name, num) SHOW_SBITS_BE(name, num)
#endif

#define GET_CACHE(name) ((uint32) name ## _cache)


static int signExtend(int val, uint bits) {
	uint shift = 8 * sizeof(int) - bits;
	union { uint u; int s; } v = { (unsigned)val << shift };
	return v.s >> shift;
}

static uint zeroExtend(uint val, uint bits) {
	return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}

GetBits::GetBits(const byte *buffer, size_t totalBits) {
	assert(buffer && totalBits < (INT_MAX - 7));

	_buffer = buffer;
	_disposeAfterUse = DisposeAfterUse::NO;
	_sizeInBits = totalBits;
	_sizeInBitsPlus8 = totalBits + 8;
	_index = 0;
}

GetBits::GetBits(Common::SeekableReadStream &stream) {
	byte *buffer = new byte[stream.size()];
	stream.read(buffer, stream.size());
	_buffer = buffer;
	_disposeAfterUse = DisposeAfterUse::YES;
	_sizeInBits = stream.size() * 8;
	_sizeInBitsPlus8 = _sizeInBits + 8;
	_index = 0;
}

GetBits::GetBits(const GetBits &src) : _index(src._index), _buffer(src._buffer),
		_sizeInBits(src._sizeInBits), _sizeInBitsPlus8(src._sizeInBitsPlus8),
		_disposeAfterUse(src._disposeAfterUse) {	
	if (_disposeAfterUse == DisposeAfterUse::YES) {
		byte *buffer = new byte[src._sizeInBits / 8];
		Common::copy(src._buffer, src._buffer + (src._sizeInBits / 8), buffer);
		_buffer = buffer;
	}
}

GetBits::~GetBits() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete[] _buffer;
}

int GetBits::getXbits(int n) {
	int sign;
	int cache;
	OPEN_READER(re);
	assert(n > 0 && n <= 25);
	UPDATE_CACHE(re);
	cache = GET_CACHE(re);
	sign = ~cache >> 31;
	LAST_SKIP_BITS(re, n);
	CLOSE_READER(re);
	return (NEG_USR32(sign ^ cache, n) ^ sign) - sign;
}

int GetBits::getSbits(int n) {
	int tmp;
	OPEN_READER(re);
	assert(n > 0 && n <= 25);
	UPDATE_CACHE(re);
	tmp = SHOW_SBITS(re, n);
	LAST_SKIP_BITS(re, n);
	CLOSE_READER(re);
	return tmp;
}

/**
* Read 1-25 bits.
*/
uint GetBits::getBits(int n) {
	int tmp;
	OPEN_READER(re);
	assert(n > 0 && n <= 25);
	UPDATE_CACHE(re);
	tmp = SHOW_UBITS(re, n);
	LAST_SKIP_BITS(re, n);
	CLOSE_READER(re);
	return tmp;
}

int GetBits::getBitsZ(int n) {
	return n ? getBits(n) : 0;
}

uint GetBits::getBitsLE(int n) {
	int tmp;
	OPEN_READER(re);
	assert(n > 0 && n <= 25);
	UPDATE_CACHE_LE(re);
	tmp = SHOW_UBITS_LE(re, n);
	LAST_SKIP_BITS(re, n);
	CLOSE_READER(re);
	return tmp;
}

uint GetBits::showBits(int n) {
	int tmp;
	OPEN_READER_NOSIZE(re);
	assert(n > 0 && n <= 25);
	UPDATE_CACHE(re);
	tmp = SHOW_UBITS(re, n);
	return tmp;
}

void GetBits::skipBits(int n) {
	OPEN_READER(re);
	LAST_SKIP_BITS(re, n);
	CLOSE_READER(re);
}

uint GetBits::getBits1() {
	uint index = _index;
	uint8 result = _buffer[index >> 3];
#ifdef BITSTREAM_READER_LE
	result >>= index & 7;
	result &= 1;
#else
	result <<= index & 7;
	result >>= 8 - 1;
#endif
#if !UNCHECKED_BITSTREAM_READER
	if (_index < _sizeInBitsPlus8)
#endif
		index++;
	_index = index;

	return result;
}

uint GetBits::showBits1() {
	return showBits(1);
}

void GetBits::skipBits1() {
	skipBits(1);
}

/**
* Read 0-32 bits.
*/
uint GetBits::getBitsLong(int n) {
	if (!n) {
		return 0;
	} else if (n <= MIN_CACHE_BITS) {
		return getBits(n);
	} else {
#ifdef BITSTREAM_READER_LE
		unsigned ret = getBits(16);
		return ret | (getBits(n - 16) << 16);
#else
		unsigned ret = getBits(16) << (n - 16);
		return ret | getBits(n - 16);
#endif
	}
}

/**
 * Read 0-64 bits.
 */
uint64 GetBits::getBits64(int n) {
	if (n <= 32) {
		return getBitsLong(n);
	} else {
#ifdef BITSTREAM_READER_LE
		uint64 ret = getBitsLong(32);
		return ret | (uint64)getBitsLong(n - 32) << 32;
#else
		uint64 ret = (uint64)getBitsLong(n - 32) << 32;
		return ret | getBitsLong(32);
#endif
	}
}

int GetBits::getSbitsLong(int n) {
	return signExtend(getBitsLong(n), n);
}

/**
* Show 0-32 bits.
*/
uint GetBits::showBitsLong(int n) {
	if (n <= MIN_CACHE_BITS) {
		return showBits(n);
	} else {
		GetBits gb(*this);
		return gb.getBitsLong(n);
	}
}

int GetBits::checkMarker(void *logctx, const char *msg) {
	int bit = getBits1();
	if (!bit)
		warning("Marker bit missing at %d of %d %s\n",
			getBitsCount() - 1, _sizeInBits, msg);

	return bit;
}

const byte *GetBits::alignGetBits() {
	int n = -(int)getBitsCount() & 7;
	if (n)
		skipBits(n);

	return _buffer + (_index >> 3);
}

/**
 * If the vlc code is invalid and max_depth=1, then no bits will be removed.
 * If the vlc code is invalid and max_depth>1, then the number of bits removed
 * is undefined.
 */
#define GET_VLC(code, name, table, bits, max_depth)         \
    do {                                                        \
        int n, nb_bits;                                         \
        unsigned int index;                                     \
                                                                \
        index = SHOW_UBITS(name, bits);                     \
        code  = table[index][0];                                \
        n     = table[index][1];                                \
                                                                \
        if (max_depth > 1 && n < 0) {                           \
            LAST_SKIP_BITS(name, bits);                     \
            UPDATE_CACHE(name);                             \
                                                                \
            nb_bits = -n;                                       \
                                                                \
            index = SHOW_UBITS(name, nb_bits) + code;       \
            code  = table[index][0];                            \
            n     = table[index][1];                            \
            if (max_depth > 2 && n < 0) {                       \
                LAST_SKIP_BITS(name, nb_bits);              \
                UPDATE_CACHE(name);                         \
                                                                \
                nb_bits = -n;                                   \
                                                                \
                index = SHOW_UBITS(name, nb_bits) + code;   \
                code  = table[index][0];                        \
                n     = table[index][1];                        \
            }                                                   \
        }                                                       \
        SKIP_BITS(name, n);                                 \
    } while (0)

#define GET_RL_VLC(level, run, name, table, bits,  \
                   max_depth, need_update)                      \
    do {                                                        \
        int n, nb_bits;                                         \
        unsigned int index;                                     \
                                                                \
        index = SHOW_UBITS(name, bits);                     \
        level = table[index].level;                             \
        n     = table[index].len;                               \
                                                                \
        if (max_depth > 1 && n < 0) {                           \
            SKIP_BITS(name, bits);                          \
            if (need_update) {                                  \
                UPDATE_CACHE(name);                         \
            }                                                   \
                                                                \
            nb_bits = -n;                                       \
                                                                \
            index = SHOW_UBITS(name, nb_bits) + level;      \
            level = table[index].level;                         \
            n     = table[index].len;                           \
            if (max_depth > 2 && n < 0) {                       \
                LAST_SKIP_BITS(name, nb_bits);              \
                if (need_update) {                              \
                    UPDATE_CACHE(name);                     \
                }                                               \
                nb_bits = -n;                                   \
                                                                \
                index = SHOW_UBITS(name, nb_bits) + level;  \
                level = table[index].level;                     \
                n     = table[index].len;                       \
            }                                                   \
        }                                                       \
        run = table[index].run;                                 \
        SKIP_BITS(name, n);                                 \
    } while (0)

/**
* Parse a vlc code.
* @param bits is the number of bits which will be read at once, must be
*             identical to nb_bits in init_vlc()
* @param max_depth is the number of times bits bits must be read to completely
*                  read the longest vlc code
*                  = (max_vlc_length + bits - 1) / bits
*/
int GetBits::getVLC2(int (*table)[2], int bits, int maxDepth) {
	int code;

	OPEN_READER(re);
	UPDATE_CACHE(re);

	GET_VLC(code, re, table, bits, maxDepth);

	CLOSE_READER(re);

	return code;
}

int GetBits::decode012() {
	int n;
	n = getBits1();
	if (n == 0)
		return 0;
	else
		return getBits1() + 1;
}

int GetBits::decode210() {
	if (getBits1())
		return 0;
	else
		return 2 - getBits1();
}

int GetBits::skip1stop8dataBits() {
	if (getBitsLeft() <= 0)
		return -1;

	while (getBits1()) {
		skipBits(8);
		if (getBitsLeft() <= 0)
			return -1;
	}

	return 0;
}

} // End of namespace Indeo
} // End of namespace Image
