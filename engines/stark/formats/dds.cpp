/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/formats/dds.h"

#include "common/textconsole.h"

namespace Stark {
namespace Formats {

// Based on xoreos' DDS code

static const uint32 kDDSID  = MKTAG('D', 'D', 'S', ' ');

static const uint32 kHeaderFlagsHasMipMaps = 0x00020000;

static const uint32 kPixelFlagsHasAlpha  = 0x00000001;
static const uint32 kPixelFlagsHasFourCC = 0x00000004;
static const uint32 kPixelFlagsIsIndexed = 0x00000020;
static const uint32 kPixelFlagsIsRGB     = 0x00000040;

DDS::~DDS() {
	for (uint i = 0; i < _mipmaps.size(); i++) {
		_mipmaps[i].free();
	}
}

bool DDS::load(Common::SeekableReadStream &dds, const Common::String &name) {
	assert(_mipmaps.empty());

	_name = name;

	if (!readHeader(dds)) {
		return false;
	}

	return readData(dds);
}

const DDS::MipMaps &DDS::getMipMaps() const {
	return _mipmaps;
}

bool DDS::readHeader(Common::SeekableReadStream &dds) {
	// We found the FourCC of a standard DDS
	uint32 magic = dds.readUint32BE();
	if (magic != kDDSID) {
		warning("Invalid DDS magic number: %d for %s", magic, _name.c_str());
		return false;
	}

	// All DDS header should be 124 bytes (+ 4 for the FourCC)
	uint32 headerSize = dds.readUint32LE();
	if (headerSize != 124) {
		warning("Invalid DDS header size: %d for %s", headerSize, _name.c_str());
		return false;
	}

	// DDS features
	uint32 flags = dds.readUint32LE();

	// Image dimensions
	uint32 height = dds.readUint32LE();
	uint32 width  = dds.readUint32LE();

	if ((width >= 0x8000) || (height >= 0x8000)) {
		warning("Unsupported DDS image dimensions (%ux%u) for %s", width, height, _name.c_str());
		return false;
	}

	dds.skip(4 + 4); // Pitch + Depth
	//uint32 pitchOrLineSize = dds.readUint32LE();
	//uint32 depth           = dds.readUint32LE();
	uint32 mipMapCount     = dds.readUint32LE();

	// DDS doesn't provide any mip maps, only one full-size image
	if ((flags & kHeaderFlagsHasMipMaps) == 0) {
		mipMapCount = 1;
	}

	dds.skip(44); // Reserved

	// Read the pixel data format
	DDSPixelFormat format;
	format.size     = dds.readUint32LE();
	format.flags    = dds.readUint32LE();
	format.fourCC   = dds.readUint32BE();
	format.bitCount = dds.readUint32LE();
	format.rBitMask = dds.readUint32LE();
	format.gBitMask = dds.readUint32LE();
	format.bBitMask = dds.readUint32LE();
	format.aBitMask = dds.readUint32LE();

	// Detect which specific format it describes
	if (!detectFormat(format)) {
		return false;
	}

	dds.skip(16 + 4); // DDCAPS2 + Reserved

	_mipmaps.resize(mipMapCount);
	for (uint32 i = 0; i < mipMapCount; i++) {
		_mipmaps[i].create(width, height, _format);

		width  >>= 1;
		height >>= 1;
	}

	return true;
}

bool DDS::readData(Common::SeekableReadStream &dds) {
	for (uint i = 0; i < _mipmaps.size(); i++) {
		Graphics::Surface &mipmap = _mipmaps[i];

		uint32 size = mipmap.pitch * mipmap.h;
		uint32 readSize = dds.read(mipmap.getPixels(), size);

		if (readSize != size) {
			warning("Inconsistent read size in DDS file: %d, expected %d for %s level %d",
			        readSize, size, _name.c_str(), i);
			return false;
		}
	}

	return true;
}

bool DDS::detectFormat(const DDSPixelFormat &format) {
	if (format.flags & kPixelFlagsHasFourCC) {
		warning("Unsupported DDS feature: FourCC pixel format %d for %s", format.fourCC, _name.c_str());
		return false;
	}

	if (format.flags & kPixelFlagsIsIndexed) {
		warning("Unsupported DDS feature: Indexed %d-bits pixel format for %s", format.bitCount, _name.c_str());
		return false;
	}

	if (!(format.flags & kPixelFlagsIsRGB)) {
		warning("Only RGB DDS files are supported for %s", _name.c_str());
		return false;
	}

	if (format.bitCount != 24 && format.bitCount != 32) {
		warning("Only 24-bits and 32-bits DDS files are supported for %s", _name.c_str());
		return false;
	}

	if ((format.flags & kPixelFlagsHasAlpha) &&
	           (format.bitCount == 32) &&
	           (format.rBitMask == 0x00FF0000) && (format.gBitMask == 0x0000FF00) &&
	           (format.bBitMask == 0x000000FF) && (format.aBitMask == 0xFF000000)) {
#ifdef SCUMM_BIG_ENDIAN
		_format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 0, 8, 16);
#else
		_format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
#endif
		return true;
	} else if (!(format.flags & kPixelFlagsHasAlpha) &&
	           (format.bitCount == 24) &&
	           (format.rBitMask == 0x00FF0000) && (format.gBitMask == 0x0000FF00) &&
	           (format.bBitMask == 0x000000FF)) {
#ifdef SCUMM_BIG_ENDIAN
		_format = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
#else
		_format = Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
#endif
		return true;
	} else {
		warning("Unsupported pixel format (%X, %X, %d, %X, %X, %X, %X) for %s",
		        format.flags, format.fourCC, format.bitCount,
		        format.rBitMask, format.gBitMask, format.bBitMask, format.aBitMask,
		        _name.c_str());
		return false;
	}
}

} // End of namespace Formats
} // End of namespace Stark
