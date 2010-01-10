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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "wage/wage.h"

#include "common/stream.h"

namespace Wage {

Common::String readPascalString(Common::SeekableReadStream &in) {
	Common::String s;
	char *buf;
	int len;
	int i;

	len = in.readSByte();
	if (len < 0)
		len += 256;

	buf = (char *)malloc(len + 1);
	for (i = 0; i < len; i++) {
		buf[i] = in.readByte();
		if (buf[i] == 0x0d)
			buf[i] = '\n';
	}

	buf[i] = 0;

	s = buf;
	free(buf);

	return s;
}

Common::Rect *readRect(Common::SeekableReadStream &in) {
	int x1, y1, x2, y2;

	y1 = in.readUint16LE();
	x1 = in.readUint16LE();
	y2 = in.readUint16LE() + 4;
	x2 = in.readUint16LE() + 4;

	return new Common::Rect(x1, y1, x2, y2);
}

} // End of namespace Wage
