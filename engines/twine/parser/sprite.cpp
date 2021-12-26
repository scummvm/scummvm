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

#include "twine/parser/sprite.h"
#include "common/stream.h"
#include "twine/shared.h"

namespace TwinE {

bool SpriteBoundingBoxData::loadFromStream(Common::SeekableReadStream &stream, bool lba1) {
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

void SpriteData::reset() {
	for (int i = 0; i < _sprites; ++i) {
		_surfaces[i].free();
	}
	_sprites = 0;
}

bool SpriteData::loadFromStream(Common::SeekableReadStream &stream, bool lba1) {
	reset();
	if (_bricks) {
		// brick sprites don't have the offsets
		return loadSprite(stream, 0);
	}
	const uint32 offset1 = stream.readUint32LE();
	const uint32 offset2 = stream.readUint32LE();
	const uint32 offsetData = stream.pos();
	if (!loadSprite(stream, offset1)) {
		return false;
	}
	// for most sprites the second offset is just the end of the stream - but
	// some sprites (like shadow in lba1) have a second sprite following the
	// first one.
	if (offset2 + offsetData >= stream.size()) {
		return true;
	}
	return loadSprite(stream, offset2);
}

bool SpriteData::loadSprite(Common::SeekableReadStream &stream, uint32 offset) {
	stream.seek(offset);
	int width = stream.readByte();
	int height = stream.readByte();
	_offsetX[_sprites] = stream.readByte();
	_offsetY[_sprites] = stream.readByte();
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	_surfaces[_sprites].create(width, height, format);
	const uint8 *last = (const uint8 *)_surfaces[_sprites].getBasePtr(width, height - 1);
	for (int y = 0; y < height; ++y) {
		const uint8 numRuns = stream.readByte();
		int x = 0;
		for (uint8 run = 0; run < numRuns; ++run) {
			const uint8 runSpec = stream.readByte();
			const uint8 runLength = bits(runSpec, 0, 6) + 1;
			const uint8 type = bits(runSpec, 6, 2);
			if (type == 1) {
				uint8 *start = (uint8 *)_surfaces[_sprites].getBasePtr(x, y);
				for (uint8 i = 0; i < runLength; ++i) {
					if (start > last) {
						return false;
					}
					*start++ = stream.readByte();
				}
			} else if (type != 0) {
				uint8 *start = (uint8 *)_surfaces[_sprites].getBasePtr(x, y);
				uint8 *end = (uint8 *)_surfaces[_sprites].getBasePtr(x + runLength, y);
				if (end > last) {
					return false;
				}
				Common::fill(start, end, stream.readByte());
			}
			x += runLength;
		}
	}
	if (stream.err()) {
		return false;
	}
	++_sprites;
	return true;
}

} // namespace TwinE
