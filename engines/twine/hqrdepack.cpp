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

#include "twine/hqrdepack.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace TwinE {

HQRDepack::HQRDepack(TwinEEngine *engine) : _engine(engine) {}

void HQRDepack::hqrDecompressEntry(uint8 *dst, uint8 *src, int32 decompsize, int32 mode) {
	do {
		uint8 b = *(src++);
		for (int32 d = 0; d < 8; d++) {
			int32 length;
			if (!(b & (1 << d))) {
				uint16 offset = *(uint16 *)(src);
				src += 2;
				length = (offset & 0x0F) + (mode + 1);
				uint8 *ptr = dst - (offset >> 4) - 1;
				for (int32 i = 0; i < length; i++)
					*(dst++) = *(ptr++);
			} else {
				length = 1;
				*(dst++) = *(src++);
			}
			decompsize -= length;
			if (decompsize <= 0)
				return;
		}
	} while (decompsize);
}

void HQRDepack::hqrDecompressLZEntry(uint8 *dst, uint8 *src, int32 decompsize, int32 mode) {
	while (decompsize > 0) {
		uint8 type = *(src++);
		for (uint8 bits = 1; bits != 0; bits <<= 1) {
			int32 length;
			if (!(type & bits)) {
				uint16 offset = *(uint16 *)(src);
				src += 2;
				length = (offset & 0x0F) + (mode + 1);
				uint8 *ptr = dst - (offset >> 4) - 1;
				if (offset == 0) {
					memset(dst, *ptr, length);
				} else {
					if ((ptr + length) >= dst) {
						uint8 *tmp = dst;
						for (int32 n = 0; n < length; n++)
							*tmp++ = *ptr++;
					} else {
						memcpy(dst, ptr, length);
					}
				}
				dst += length;
			} else {
				length = 1;
				*(dst++) = *(src++);
			}
			decompsize -= length;
			if (decompsize <= 0) {
				return;
			}
		}
	}
}

int32 HQRDepack::hqrGetEntry(uint8 *ptr, const char *filename, int32 index) {
	if (!ptr) {
		return 0;
	}
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		error("HQR: %s can't be found!", filename);
	}

	uint32 headerSize = file.readUint32LE();

	if ((uint32)index >= headerSize / 4) {
		warning("HQR WARNING: Invalid entry index!!");
		return 0;
	}

	file.seek(index * 4);
	uint32 offsetToData = file.readUint32LE();

	file.seek(offsetToData);
	uint32 realSize = file.readUint32LE();
	uint32 compSize = file.readUint32LE();
	uint16 mode = file.readUint16LE();

	// uncompressed
	if (mode == 0) {
		file.read(ptr, realSize);
	}
	// compressed: modes (1 & 2)
	else if (mode == 1 || mode == 2) {
		uint8 *compDataPtr = 0;
		compDataPtr = (uint8 *)malloc(compSize);
		file.read(compDataPtr, compSize);
		hqrDecompressEntry(ptr, compDataPtr, realSize, mode);
		free(compDataPtr);
	}

	return realSize;
}

int HQRDepack::hqrEntrySize(const char *filename, int32 index) {
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		error("HQR: %s can't be found!", filename);
	}

	uint32 headerSize;
	file.read(&headerSize, 4);

	if ((uint32)index >= headerSize / 4) {
		warning("HQR WARNING: Invalid entry index!!");
		return 0;
	}

	file.seek(index * 4);
	uint32 offsetToData;
	file.read(&offsetToData, 4);

	file.seek(offsetToData);
	uint32 realSize;
	file.read(&realSize, 4);

	return realSize;
}

int HQRDepack::hqrNumEntries(const char *filename) {
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		error("HQR: %s can't be found!", filename);
	}

	uint32 headerSize;
	file.read(&headerSize, 4);
	return (int)headerSize / 4;
}

int32 HQRDepack::hqrGetallocEntry(uint8 **ptr, const char *filename, int32 index) {
	const int32 size = hqrEntrySize(filename, index);
	*ptr = (uint8 *)malloc(size * sizeof(uint8));
	if (!*ptr) {
		warning("HQR WARNING: unable to allocate entry memory!!");
		return 0;
	}
	hqrGetEntry(*ptr, filename, index);

	return size;
}

int32 HQRDepack::hqrGetVoxEntry(uint8 *ptr, const char *filename, int32 index, int32 hiddenIndex) {
	if (!ptr) {
		return 0;
	}
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		error("HQR: %s can't be found!", filename);
	}

	uint32 headerSize;
	file.read(&headerSize, 4);

	if ((uint32)index >= headerSize / 4) {
		warning("HQR WARNING: Invalid entry index!!");
		return 0;
	}

	file.seek(index * 4);
	uint32 offsetToData;
	file.read(&offsetToData, 4);

	file.seek(offsetToData);
	uint32 realSize;
	file.read(&realSize, 4);
	uint32 compSize;
	file.read(&compSize, 4);
	uint16 mode;
	file.read(&mode, 2);

	// exist hidden entries
	for (int32 i = 0; i < hiddenIndex; i++) {
		file.seek(offsetToData + compSize + 10);   // hidden entry
		offsetToData = offsetToData + compSize + 10; // current hidden offset

		file.read(&realSize, 4);
		file.read(&compSize, 4);
		file.read(&mode, 2);
	}

	// uncompressed
	if (mode == 0) {
		file.read(ptr, realSize);
	}
	// compressed: modes (1 & 2)
	else if (mode == 1 || mode == 2) {
		uint8 *compDataPtr = 0;
		compDataPtr = (uint8 *)malloc(compSize);
		file.read(compDataPtr, compSize);
		hqrDecompressEntry(ptr, compDataPtr, realSize, mode);
		free(compDataPtr);
	}

	return realSize;
}

int HQRDepack::hqrVoxEntrySize(const char *filename, int32 index, int32 hiddenIndex) {
	if (!filename) {
		return 0;
	}

	Common::File file;
	if (!file.open(filename)) {
		error("HQR: %s can't be found!", filename);
	}

	uint32 headerSize;
	file.read(&headerSize, 4);

	if ((uint32)index >= headerSize / 4) {
		warning("HQR WARNING: Invalid entry index!!");
		return 0;
	}

	file.seek(index * 4);
	uint32 offsetToData;
	file.read(&offsetToData, 4);

	file.seek(offsetToData);
	uint32 realSize;
	file.read(&realSize, 4);
	uint32 compSize;
	file.read(&compSize, 4);

	// exist hidden entries
	for (int32 i = 0; i < hiddenIndex; i++) {
		file.seek(offsetToData + compSize + 10);   // hidden entry
		offsetToData = offsetToData + compSize + 10; // current hidden offset

		file.read(&realSize, 4);
		file.read(&compSize, 4);
	}

	return realSize;
}

int32 HQRDepack::hqrGetallocVoxEntry(uint8 **ptr, const char *filename, int32 index, int32 hiddenIndex) {
	const int32 size = hqrVoxEntrySize(filename, index, hiddenIndex);

	*ptr = (uint8 *)malloc(size * sizeof(uint8));
	if (!*ptr) {
		warning("HQR WARNING: unable to allocate entry memory!!\n");
		return 0;
	}
	hqrGetVoxEntry(*ptr, filename, index, hiddenIndex);

	return size;
}

} // namespace TwinE
