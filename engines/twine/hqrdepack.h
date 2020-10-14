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

#ifndef TWINE_HQRDEPACK_H
#define TWINE_HQRDEPACK_H

#include "common/scummsys.h"

namespace TwinE {

class TwinEEngine;

class HQRDepack {
private:
	TwinEEngine *_engine;

	/**
	 * Get a HQR entry pointer
	 * @param filename HQR file name
	 * @param index entry index to extract
	 * @return entry real size
	 * */
	int hqrVoxEntrySize(const char *filename, int32 index, int32 hiddenIndex);
	/**
	 * Decompress entry based in the original expand lzss lba code
	 * @param dst destination pointer where will be the decompressed entry
	 * @param src compressed data pointer
	 * @param decompsize real file size after decompression
	 * @param mode compression mode used
	 */
	void hqrDecompressLZEntry(uint8 *dst, uint8 *src, int32 decompsize, int32 mode);
	/**
	 * Decompress entry based in Yaz0r and Zink decompression code
	 * @param dst destination pointer where will be the decompressed entry
	 * @param src compressed data pointer
	 * @param decompsize real file size after decompression
	 * @param mode compression mode used
	 */
	void hqrDecompressEntry(uint8 *dst, uint8 *src, int32 decompsize, int32 mode);

public:
	HQRDepack(TwinEEngine *engine);

	/**
	 * Get a HQR entry pointer
	 * @param ptr pointer to save the entry
	 * @param filename HQR file name
	 * @param index entry index to extract
	 * @return entry real size
	 */
	int32 hqrGetEntry(uint8 *ptr, const char *filename, int32 index);

	/**
	 * Get a HQR entry pointer
	 * @param filename HQR file name
	 * @param index entry index to extract
	 * @return entry real size
	 */
	int32 hqrEntrySize(const char *filename, int32 index);

	/**
	 * Get a HQR total number of entries
	 * @param filename HQR file name
	 * @return total number of entries
	 */
	int32 hqrNumEntries(const char *filename);

	/**
	 * Get a HQR entry pointer with memory allocation
	 * @param ptr pointer to save the entry
	 * @param filename HQR file name
	 * @param index entry index to extract
	 * @return entry real size
	 */
	int32 hqrGetallocEntry(uint8 **ptr, const char *filename, int32 index);

	/**
	 * Get a HQR entry pointer
	 * @param ptr pointer to save the entry
	 * @param filename HQR file name
	 * @param index entry index to extract
	 * @return entry real size
	 */
	int32 hqrGetVoxEntry(uint8 *ptr, const char *filename, int32 index, int32 hiddenIndex);
	/**
	 * Get a HQR entry pointer with memory allocation
	 * @param ptr pointer to save the entry
	 * @param filename HQR file name
	 * @param index entry index to extract
	 * @return entry real size
	 */
	int32 hqrGetallocVoxEntry(uint8 **ptr, const char *filename, int32 index, int32 hiddenIndex);
};

} // namespace TwinE

#endif
