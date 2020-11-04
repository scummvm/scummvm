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

#include "twine/hqr.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace TwinE {

namespace HQR {

#define wrap(cmd)                           \
	if ((cmd) == 0) {                       \
		warning("Failed to execute " #cmd); \
		return 0;                           \
	}

/**
 * Decompress entry based in Yaz0r and Zink decompression code
 * @param dst destination pointer where will be the decompressed entry
 * @param src compressed data pointer
 * @param decompsize real file size after decompression
 * @param mode compression mode used
 */
static void decompressEntry(uint8 *dst, const uint8 *src, int32 decompsize, int32 mode) {
	do {
		uint8 b = *(src++);
		for (int32 d = 0; d < 8; d++) {
			int32 length;
			if (!(b & (1 << d))) {
				const uint16 offset = *(const uint16 *)(src);
				src += 2;
				length = (offset & 0x0F) + (mode + 1);
				const uint8 *ptr = dst - (offset >> 4) - 1;
				for (int32 i = 0; i < length; i++) {
					*(dst++) = *(ptr++);
				}
			} else {
				length = 1;
				*(dst++) = *(src++);
			}
			decompsize -= length;
			if (decompsize <= 0) {
				return;
			}
		}
	} while (decompsize);
}

/**
 * Get a HQR entry pointer
 * @param filename HQR file name
 * @param index entry index to extract
 * @return entry real size
 */
static int voxEntrySize(const char *filename, int32 index, int32 hiddenIndex) {
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		warning("HQR: Could not open %s", filename);
		return 0;
	}

	uint32 headerSize = file.readUint32LE();
	if ((uint32)index >= headerSize / 4) {
		warning("HQR: Invalid entry index");
		return 0;
	}

	wrap(file.seek(index * 4))
	uint32 offsetToData = file.readUint32LE();

	wrap(file.seek(offsetToData))
	uint32 realSize = file.readUint32LE();
	uint32 compSize = file.readUint32LE();

	    // exist hidden entries
	for (int32 i = 0; i < hiddenIndex; i++) {
		wrap(file.seek(offsetToData + compSize + 10)) // hidden entry
		offsetToData = offsetToData + compSize + 10;  // current hidden offset

		realSize = file.readUint32LE();
		compSize = file.readUint32LE();
	}

	return realSize;
}

int32 getEntry(uint8 *ptr, const char *filename, int32 index) {
	if (!ptr) {
		return 0;
	}
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		warning("HQR: Could not open %s", filename);
		return 0;
	}

	uint32 headerSize = file.readUint32LE();

	if ((uint32)index >= headerSize / 4) {
		warning("HQR: Invalid entry index");
		return 0;
	}

	wrap(file.seek(index * 4))
	uint32 offsetToData = file.readUint32LE();

	wrap(file.seek(offsetToData))
	uint32 realSize = file.readUint32LE();
	uint32 compSize = file.readUint32LE();
	uint16 mode = file.readUint16LE();

	// uncompressed
	if (mode == 0) {
		wrap(file.read(ptr, realSize))
	}
	// compressed: modes (1 & 2)
	else if (mode == 1 || mode == 2) {
		uint8 *compDataPtr = (uint8 *)malloc(compSize);
		wrap(file.read(compDataPtr, compSize))
		decompressEntry(ptr, compDataPtr, realSize, mode);
		free(compDataPtr);
	}

	return realSize;
}

int32 entrySize(const char *filename, int32 index) {
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		warning("HQR: Could not open %s", filename);
		return 0;
	}

	uint32 headerSize = file.readUint32LE();
	if ((uint32)index >= headerSize / 4) {
		warning("HQR: Invalid entry index");
		return 0;
	}

	wrap(file.seek(index * 4))
	uint32 offsetToData = file.readUint32LE();

	wrap(file.seek(offsetToData))
	uint32 realSize = file.readUint32LE();

	return realSize;
}

int32 numEntries(const char *filename) {
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		warning("HQR: Could not open %s", filename);
		return 0;
	}

	uint32 headerSize = file.readUint32LE();
	return ((int)headerSize / 4) - 1;
}

int32 getAllocEntry(uint8 **ptr, const char *filename, int32 index) {
	const int32 size = entrySize(filename, index);
	if (size <= 0) {
		warning("HQR: failed to get entry for index %i from file: %s", index, filename);
		return 0;
	}
	*ptr = (uint8 *)malloc(size * sizeof(uint8));
	if (!*ptr) {
		warning("HQR: unable to allocate entry memory");
		return 0;
	}
	const int32 entrySize = getEntry(*ptr, filename, index);
	assert(entrySize == size);
	return entrySize;
}

int32 getVoxEntry(uint8 *ptr, const char *filename, int32 index, int32 hiddenIndex) {
	if (!ptr) {
		return 0;
	}
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		warning("HQR: Could not open %s", filename);
		return 0;
	}

	uint32 headerSize = file.readUint32LE();

	if ((uint32)index >= headerSize / 4) {
		warning("HQR: Invalid entry index");
		return 0;
	}

	wrap(file.seek(index * 4))
	uint32 offsetToData = file.readUint32LE();

	wrap(file.seek(offsetToData))
	uint32 realSize = file.readUint32LE();
	uint32 compSize = file.readUint32LE();
	uint16 mode = file.readSint16LE();

	// exist hidden entries
	for (int32 i = 0; i < hiddenIndex; i++) {
		wrap(file.seek(offsetToData + compSize + 10)) // hidden entry
		offsetToData = offsetToData + compSize + 10;  // current hidden offset

		realSize = file.readUint32LE();
		compSize = file.readUint32LE();
		mode = file.readUint16LE();
	}

	// uncompressed
	if (mode == 0) {
		wrap(file.read(ptr, realSize))
	}
	// compressed: modes (1 & 2)
	else if (mode == 1 || mode == 2) {
		uint8 *compDataPtr = (uint8 *)malloc(compSize);
		wrap(file.read(compDataPtr, compSize))
		decompressEntry(ptr, compDataPtr, realSize, mode);
		free(compDataPtr);
	}

	return realSize;
}

int32 getAllocVoxEntry(uint8 **ptr, const char *filename, int32 index, int32 hiddenIndex) {
	const int32 size = voxEntrySize(filename, index, hiddenIndex);

	*ptr = (uint8 *)malloc(size * sizeof(uint8));
	if (!*ptr) {
		warning("HQR: unable to allocate entry memory");
		return 0;
	}
	const int32 entrySize = getVoxEntry(*ptr, filename, index, hiddenIndex);
	assert(entrySize == size);
	return entrySize;
}

#undef wrap

} // namespace HQR

} // namespace TwinE
