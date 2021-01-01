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

#include "twine/parser/sprite.h"
#include "common/stream.h"
#include "twine/shared.h"

namespace TwinE {

bool SpriteBoundingBoxData::loadFromStream(Common::SeekableReadStream &stream) {
	const int32 size = stream.size();
	const int32 amount = size / 16;
	for (int32 i = 0; i < amount; ++i) {
		SpriteDim spriteDim;
		spriteDim.x = stream.readSint16LE();
		spriteDim.y = stream.readSint16LE();
		BoundingBox boundingBox;
		boundingBox.mins.x = stream.readSint16LE();
		boundingBox.maxs.x = stream.readSint16LE();
		boundingBox.mins.y = stream.readSint16LE();
		boundingBox.maxs.y = stream.readSint16LE();
		boundingBox.mins.z = stream.readSint16LE();
		boundingBox.maxs.z = stream.readSint16LE();
		_boundingBoxes.push_back(boundingBox);
		_dimensions.push_back(spriteDim);
	}
	return !stream.err();
}

bool SpriteData::loadFromStream(Common::SeekableReadStream &stream) {
	stream.skip(8);
	int width = stream.readByte();
	int height = stream.readByte();
	_offsetX = stream.readByte();
	_offsetY = stream.readByte();
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	_surface.create(width, height, format);
	const int maxY = _offsetY + height;
	for (int y = _offsetY; y < maxY; ++y) {
		const uint8 numRuns = stream.readByte();
		int x = _offsetX;
		for (uint8 run = 0; run < numRuns; ++run) {
			const uint8 runSpec = stream.readByte();
			const uint8 runLength = bits(runSpec, 0, 6) + 1;
			const uint8 type = bits(runSpec, 6, 2);
			if (type == 2) {
				uint8 *start = (uint8 *)_surface.getBasePtr(x, y);
				uint8 *end = (uint8 *)_surface.getBasePtr(x + runLength, y);
				Common::fill(start, end, stream.readByte());
			} else if (type == 1 || type == 3) {
				uint8 *start = (uint8 *)_surface.getBasePtr(x, y);
				for (uint8 i = 0; i < runLength; ++i) {
					*start++ = stream.readByte();
				}
			}
			x += runLength;
		}
	}
	return !stream.err();
}

} // namespace TwinE
