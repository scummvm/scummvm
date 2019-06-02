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

#ifndef STARK_FORMATS_DDS_H
#define STARK_FORMATS_DDS_H

#include "common/array.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Stark {
namespace Formats {

// Based on xoreos' DDS code

/**
 * DDS texture
 *
 * Only a very small subset of DDS features are supported. Especially,
 * compressed formats are not supported. This class is meant to
 * load a single DDS file per instance.
 */
class DDS {
public:
	~DDS();

	typedef Common::Array<Graphics::Surface> MipMaps;

	/** Load a DDS texture from a stream */
	bool load(Common::SeekableReadStream &dds, const Common::String &name);

	/**
	 * Retrieve the mip map levels for a loaded texture
	 *
	 * The first mipmap is the full size image. Each further
	 * mipmap divides by two the with and the height of the
	 * previous one.
	 */
	const MipMaps &getMipMaps() const;

private:
	/** The specific pixel format of the included image data. */
	struct DDSPixelFormat {
		/** The size of the image data in bytes */
		uint32 size;

		/** Features of the image data */
		uint32 flags;

		/** The FourCC to detect the format by */
		uint32 fourCC;

		/** Number of bits per pixel */
		uint32 bitCount;

		/** Bit mask for the red color component */
		uint32 rBitMask;

		/** Bit mask for the green color component */
		uint32 gBitMask;

		/** Bit mask for the blue color component */
		uint32 bBitMask;

		/** Bit mask for the alpha component */
		uint32 aBitMask;
	};

	bool readHeader(Common::SeekableReadStream &dds);
	bool readData(Common::SeekableReadStream &dds);

	bool detectFormat(const DDSPixelFormat &format);

	MipMaps _mipmaps;
	Graphics::PixelFormat _format;
	Common::String _name;
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_FORMATS_DDS_H
