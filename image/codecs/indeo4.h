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

#include "common/scummsys.h"
#include "image/codecs/indeo/get_bits.h"

/* Intel Indeo 4 decompressor, derived from ffmpeg.
 *
 * Original copyright note:
 * Intel Indeo 4 (IV31, IV32, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef IMAGE_CODECS_INDEO4_H
#define IMAGE_CODECS_INDEO4_H

#include "image/codecs/indeo/get_bits.h"
#include "image/codecs/indeo/indeo.h"
#include "graphics/managed_surface.h"

namespace Image {

using namespace Indeo;

/**
 * Intel Indeo 4 decoder.
 *
 * Used by AVI.
 *
 * Used in video:
 *  - AVIDecoder
 */
class Indeo4Decoder : public IndeoDecoderBase {
public:
	Indeo4Decoder(uint16 width, uint16 height);
	virtual ~Indeo4Decoder() {}

	virtual const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);

	static bool isIndeo4(Common::SeekableReadStream &stream);
protected:
	/**
	 * Decode the Indeo 4 picture header.
	 * @returns		0 = Ok, negative number = error
	 */
	virtual int decodePictureHeader();
private:
	int scaleTileSize(int def_size, int size_factor);

	/**
	 *  Decode subdivision of a plane.
	 *  This is a simplified version that checks for two supported subdivisions:
	 *  - 1 wavelet band  per plane, size factor 1:1, code pattern: 3
	 *  - 4 wavelet bands per plane, size factor 1:4, code pattern: 2,3,3,3,3
	 *  Anything else is either unsupported or corrupt.
	 *
	 *  @param[in,out] gb    the GetBit context
	 *  @return        number of wavelet bands or 0 on error
	 */
	int decodePlaneSubdivision();

private:
	/**
	 * Standard picture dimensions
	 */
	static const uint _ivi4_common_pic_sizes[14];
};

} // End of namespace Image

#endif
