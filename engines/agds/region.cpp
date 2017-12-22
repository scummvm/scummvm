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
	center.x	= READ_UINT16(header + kRegionHeaderWidthOffset);
	center.y	= READ_UINT16(header + kRegionHeaderHeightOffset);
	flags		= READ_UINT16(header + kRegionHeaderFlagsOffset);
	debug("region %s at (%d,%d) %04x", name.c_str(), center.x, center.y, flags);
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

//FIXME: copied from wintermute/base_region.cpp

typedef struct {
	double x, y;
} dPoint;

bool Region::pointIn(Common::Point point) const {
	uint32 size = points.size();
	if (size < 3) {
		return false;
	}

	int counter = 0;
	double xinters;
	dPoint p, p1, p2;

	p.x = (double)point.x;
	p.y = (double)point.y;

	p1.x = (double)points[0].x;
	p1.y = (double)points[0].y;

	for (uint32 i = 1; i <= size; i++) {
		p2.x = (double)points[i % size].x;
		p2.y = (double)points[i % size].y;

		if (p.y > MIN(p1.y, p2.y)) {
			if (p.y <= MAX(p1.y, p2.y)) {
				if (p.x <= MAX(p1.x, p2.x)) {
					if (p1.y != p2.y) {
						xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
						if (p1.x == p2.x || p.x <= xinters) {
							counter++;
						}
					}
				}
			}
		}
		p1 = p2;
	}

	if (counter % 2 == 0) {
		return false;
	} else {
		return true;
	}
}

}
