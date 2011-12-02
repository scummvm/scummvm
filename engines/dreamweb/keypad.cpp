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
 */

#include "dreamweb/dreamweb.h"

namespace DreamGen {

void DreamGenContext::getUnderMenu() {
	multiGet(segRef(data.word(kBuffers)).ptr(kUndertimedtext, 0), kMenux, kMenuy, 48, 48);
}

void DreamGenContext::putUnderMenu() {
	multiPut(segRef(data.word(kBuffers)).ptr(kUndertimedtext, 0), kMenux, kMenuy, 48, 48);
}

void DreamGenContext::singleKey(uint8 key, uint16 x, uint16 y) {
	if (key == data.byte(kGraphicpress)) {
		key += 11;
		if (data.byte(kPresscount) < 8)
			key -= 11;
	}
	key -= 20;
	showFrame(tempGraphics(), x, y, key, 0);
}

void DreamGenContext::singleKey() {
	singleKey(al, di, bx);
}

} /*namespace dreamgen */

