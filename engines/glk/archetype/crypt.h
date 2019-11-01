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

#ifndef ARCHETYPE_CRYPT
#define ARCHETYPE_CRYPT

#include "glk/archetype/string.h"

namespace Glk {
namespace Archetype {

enum EncryptionType {
	NONE, SIMPLE, PURPLE, UNPURPLE, COMPLEX, DEBUGGING_ON
};

extern byte CryptMask;
extern EncryptionType Encryption;

/**
 * Initializes fields local to the file
 */
extern void crypt_init();

extern void cryptinit(EncryptionType crypt_kind, uint seed);

/**
 * Encrypts or decrypts a string.Since all encryption methods are based on XOR,
 * the same method both encrypts anddecrypts.
 * If <method> is SIMPLE, the CryptMask is simply XORed with each byte in the string.
 * If <method> is PURPLE, the CryptMask is changed each time after using it,
 * by adding to it the lowest three bits of the result of the last encrypted
 * byte.This way the mask changes frequently anddynamically in a way that
 * is difficult to predict.
 * If <method> is UNPURPLE, the same algorithm as PURPLE is used except that
 * the next CryptMask must be determined before altering the byte under consideration.
 * if <method> is COMPLEX, a pseudorandom sequence is used to alter the
 * CryptMask.This can make prediction well-nigh impossible.
 */
extern void cryptstr(char *buffer, size_t length);

} // End of namespace Archetype
} // End of namespace Glk

#endif
