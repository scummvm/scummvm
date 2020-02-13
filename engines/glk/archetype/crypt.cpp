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

#include "glk/archetype/crypt.h"
#include "glk/archetype/archetype.h"

namespace Glk {
namespace Archetype {

byte CryptMask;
EncryptionType Encryption;
static uint RandSeed;
const int RANDOM_KEY = 33797;

void crypt_init() {
	Encryption = NONE;
	CryptMask = 0x55;
	RandSeed = 0;
}

static void setDecryptionSeed(uint new_seed) {
	RandSeed = new_seed;
}

static void cycleRandomSeed() {
	uint16 c = RandSeed & 0xffff;
	uint val = (uint)c * RANDOM_KEY;
	c <<= 3;
	c = (c & 0xff) | ((((c >> 8) + c) & 0xff) << 8);
	val += (uint)c << 16;

	uint16 b = RandSeed >> 16;
	val += (uint)b << 16;
	b <<= 2;
	val += (uint)b << 16;
	val += (uint)(b & 0xff) << 24;
	b <<= 5;
	val += (uint)(b & 0xff) << 24;

	RandSeed = val + 1;
}

static uint getDeterministicRandomNumber(uint limit) {
	cycleRandomSeed();
	return (limit == 0) ? 0 : (RandSeed >> 16) % limit;
}

void cryptinit(EncryptionType crypt_kind, uint seed) {
	CryptMask = seed & 0xff;
	Encryption = crypt_kind;

	if (Encryption == COMPLEX)
		setDecryptionSeed(seed);
}

void cryptstr(char *buffer, size_t length) {
	byte nextMask;
	char *p = buffer;

	switch (Encryption) {
	case SIMPLE:
		for (size_t i = 0; i < length; ++i, ++p)
			*p ^= CryptMask;
		break;

	case PURPLE:
		for (size_t i = 0; i < length; ++i, ++p) {
			*p ^= CryptMask;
			CryptMask += *p & 7;
		}
		break;

	case UNPURPLE:
		for (size_t i = 0; i < length; ++i, ++p) {
			nextMask = CryptMask + (*p & 7);
			*p ^= CryptMask;
			CryptMask = nextMask;
		}
		break;

	case COMPLEX:
		for (size_t i = 0; i < length; ++i, ++p) {
			*p ^= CryptMask;
			CryptMask = (byte)getDeterministicRandomNumber(0x100);
		}
		break;

	default:
		break;
	}
}

} // End of namespace Archetype
} // End of namespace Glk
