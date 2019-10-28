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

void crypt_init() {
	Encryption = NONE;
	CryptMask = 0x55;
}

void cryptinit(EncryptionType crypt_kind, uint seed) {
	CryptMask = seed & 0xff;
	Encryption = crypt_kind;

	if (Encryption == COMPLEX)
		g_vm->setRandomNumberSeed(seed);
}

void cryptstr(Common::String &s) {
	byte nextMask;

	switch (Encryption) {
	case SIMPLE:
		for (uint i = 0; i < s.size(); ++i)
			s.setChar(s[i] ^ CryptMask, i);
		break;

	case PURPLE:
		for (uint i = 0; i < s.size(); ++i) {
			s.setChar(s[i] ^ CryptMask, i);
			CryptMask += s[i] & 7;
		}
		break;

	case UNPURPLE:
		for (uint i = 0; i < s.size(); ++i) {
			nextMask = CryptMask + (s[i] & 7);
			s.setChar(s[i] ^ CryptMask, i);
			CryptMask = nextMask;
		}
		break;

	case COMPLEX:
		for (uint i = 0; i < s.size(); ++i) {
			s.setChar(s[i] ^ CryptMask, i);
			CryptMask = g_vm->getRandomNumber(0x100);
		}
		break;

	default:
		break;
	}
}

} // End of namespace Archetype
} // End of namespace Glk
