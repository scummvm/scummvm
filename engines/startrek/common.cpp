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
 */

#include "common/rect.h"
#include "common/serializer.h"

#include "startrek/common.h"

namespace StarTrek {

Common::Rect getRectEncompassing(Common::Rect r1, Common::Rect r2) {
	uint16 l = min(r1.left, r2.left);
	uint16 t = min(r1.top, r2.top);
	uint16 r = max(r1.right, r2.right);
	uint16 b = max(r1.bottom, r2.bottom);

	return Common::Rect(l,t,r,b);
}

void serializeRect(Common::Rect rect, Common::Serializer &ser) {
	ser.syncAsSint16LE(rect.left);
	ser.syncAsSint16LE(rect.top);
	ser.syncAsSint16LE(rect.right);
	ser.syncAsSint16LE(rect.bottom);
}

}
