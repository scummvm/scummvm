/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"

#include "common/system.h"

namespace ICB {

const char *hashTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";

uint32 EngineHashString(const char *fn) {
	if (fn == NULL)
		return 0;
	char *f;
	char c;
	uint32 n;
	n = 0;
	f = const_cast<char *>(fn);
	while ((c = *f) != 0) {
		n = (n << 7) + (n << 1) + n + c; // n=128n+2n+n+c -> n=131n+c
		                                 // n=131*n+c;                  // n=131n+c
		f++;
	}
	return n;
}

// Take a filename, path whatever,
// convert to hash value, then convert that hash value to
// 7 character filename
uint32 EngineHashFile(const char *fn, char *output) {
	uint32 hash = EngineHashString(fn);
	output[0] = hashTable[(hash >> 27) & 0x1F];
	output[1] = hashTable[(hash >> 22) & 0x1F];
	output[2] = hashTable[(hash >> 17) & 0x1F];
	output[3] = hashTable[(hash >> 12) & 0x1F];
	output[4] = hashTable[(hash >> 7) & 0x1F];
	output[5] = hashTable[(hash >> 2) & 0x1F];
	output[6] = hashTable[hash & 0x3];
	output[7] = '\0';
	return hash;
}

// Take a hash
// convert that hash value to
// 7 character filename
uint32 EngineHashToFile(uint32 hash, char *output) {
	output[0] = hashTable[(hash >> 27) & 0x1F];
	output[1] = hashTable[(hash >> 22) & 0x1F];
	output[2] = hashTable[(hash >> 17) & 0x1F];
	output[3] = hashTable[(hash >> 12) & 0x1F];
	output[4] = hashTable[(hash >> 7) & 0x1F];
	output[5] = hashTable[(hash >> 2) & 0x1F];
	output[6] = hashTable[hash & 0x3];
	output[7] = '\0';
	return hash;
}

} // End of namespace ICB
