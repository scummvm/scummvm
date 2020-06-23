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

#ifndef GLK_ZCODE_PICS_DECODER_H
#define GLK_ZCODE_PICS_DECODER_H

#include "common/stream.h"
#include "common/array.h"

namespace Glk {
namespace ZCode {

/**
 * Decodes an Infocom encoded picture into a raw pixel stream that the outer
 * Glk engine is capable of then loading into a picture object
 */
class PictureDecoder {
private:
	byte *_tableVal;
	uint16 *_tableRef;
public:
	/**
	 * Constructor
	 */
	PictureDecoder();

	/**
	 * Destructor
	 */
	~PictureDecoder();

	/**
	 * Decode method
	 */
	Common::SeekableReadStream *decode(Common::ReadStream &src, uint flags,
		const Common::Array<byte> &palette, uint display, size_t width, size_t height);
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
