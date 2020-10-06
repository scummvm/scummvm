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

#ifndef ICB_RES_MAN_PC_H
#define ICB_RES_MAN_PC_H

#include "engines/icb/debug.h"
#include "engines/icb/common/px_string.h"

#include "common/stream.h"
#include "common/file.h"

namespace ICB {

typedef struct async_PacketType {
	pxString fn;
	uint8 *p;
	int32 size;
	int32 zipped;
	int32 memListNo;
} async_PacketType;

bool checkFileExists(const char *fullpath);

Common::SeekableReadStream *openDiskFileForBinaryStreamRead(const Common::String &filename);
Common::WriteStream *openDiskWriteStream(const Common::String &filename);
inline int32 SameUrl(const char *urla, const uint32 urla_hash, const uint32 clustera_hash, const char *urlb, const uint32 urlb_hash, const uint32 clusterb_hash) {
	// If urls are provided then the hash values should be identical if the strings are the same, and not if not
	if ((urla) && (urlb)) {
		if (strcmp(urla, urlb) == 0) {
			if (urla_hash != urlb_hash)
				Fatal_error("Hash error in %s @ %d", __FILE__, __LINE__);
		} else {
			if (urla_hash == urlb_hash)
				Fatal_error("Hash error in %s @ %d", __FILE__, __LINE__);
		}
	}

	// For clustering we just check the hashes
	if (clustera_hash != clusterb_hash)
		return 0;
	if (urla_hash != urlb_hash)
		return 0;
	return 1;
}

} // End of namespace ICB

#endif // #ifndef RES_MAN_PC_H
