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

#include "twine/parser/brick.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace TwinE {

bool BrickData::loadFromStream(Common::SeekableReadStream &stream) {
	int width = stream.readByte();
	int height = stream.readByte();
	_offsetX = stream.readByte();
	_offsetY = stream.readByte();
	const int maxY = _offsetY + height;
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	_surface.create(width, height, format);
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
