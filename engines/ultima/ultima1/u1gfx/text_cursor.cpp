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

#include "ultima/ultima1/u1gfx/text_cursor.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

#define CURSOR_ANIM_FRAME_TIME 100

void U1TextCursor::draw() {
	uint32 time = getTime();
	if (!_visible || (time - _lastFrameFrame) < CURSOR_ANIM_FRAME_TIME)
		return;

	_lastFrameFrame = time;
	// TODO: Draw u1 cursor
}

uint32 U1TextCursor::getTime() {
	return g_system->getMillis();
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
