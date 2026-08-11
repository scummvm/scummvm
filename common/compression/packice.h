/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @file
 * Pack-Ice decompressor.
 */

#ifndef COMMON_PACKICE_H
#define COMMON_PACKICE_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Common {

enum PackIceVersion {
	kPackIceVersion110 = 0,
	kPackIceVersion200 = 1,
	kPackIceVersion231 = 2
};

struct PackIceHeader {
	uint32 packedSize;
	uint32 rawSize;
	PackIceVersion version;
};

bool detectPackIceHeader(const byte *data, uint32 size, bool exactSizeKnown);
bool parsePackIceHeader(const byte *data, uint32 size, bool exactSizeKnown, PackIceHeader &header);
const char *getPackIceName(PackIceVersion version);

bool decompressPackIce(const byte *data, uint32 size, Common::Array<byte> &out, bool exactSizeKnown = true);
bool decompressPackIceStream(const byte *data, uint32 size, uint32 streamStart, uint32 streamEnd,
		uint32 rawSize, Common::Array<byte> &out, bool useBytes);
bool convertPackIcePictureData(Common::Array<byte> &data, uint32 pictureSize = 32000);

} // End of namespace Common

#endif
