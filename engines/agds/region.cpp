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

#include "agds/region.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "common/algorithm.h"
#include "common/endian.h"

namespace AGDS {

Region::Region(const Common::String &resourceName, Common::SeekableReadStream * stream) {
	static const int kRegionHeaderSize = 0x26;
	static const int kRegionHeaderWidthOffset = 0x20;
	static const int kRegionHeaderHeightOffset = 0x22;
	static const int kRegionHeaderFlagsOffset = 0x24;

	int size = stream->size();
	debug("region size %d, data %d", size, size - kRegionHeaderSize);
	byte header[kRegionHeaderSize];
	if (stream->read(header, kRegionHeaderSize) != kRegionHeaderSize)
		error("invalid region %s", resourceName.c_str());
	byte * nameEnd = Common::find(header, header + 0x20, 0);
	name		= Common::String(reinterpret_cast<char *>(header), nameEnd - header);
	centerX		= READ_UINT16(header + kRegionHeaderWidthOffset);
	centerY		= READ_UINT16(header + kRegionHeaderHeightOffset);
	flags		= READ_UINT16(header + kRegionHeaderFlagsOffset);
	debug("region %s at (%d,%d) %04x", name.c_str(), centerX, centerY, flags);
	if (size > kRegionHeaderSize) {
		uint16 ext	= stream->readUint16LE();
		//debug("extended entries %u", ext);
		while(ext--) {
			int16 a = stream->readSint16LE();
			int16 b = stream->readSint16LE();
			/* int16 c = */ stream->readUint16LE();
//			if (c != -12851) //0xcdcd
//				debug("extended entry: %d %d %d", a, b, c);
//			else
//				debug("extended entry: %d %d", a, b);
			points.push_back(Common::Point(a, b));
		}
		if (stream->pos() != size)
			warning("region data left: %u", size - stream->pos());
	}
}

bool Region::pointIn(Common::Point point) const {
	return false;
}

}
