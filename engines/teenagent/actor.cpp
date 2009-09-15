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
 * $URL$
 * $Id$
 */

#include "teenagent/actor.h"
#include "teenagent/objects.h"

namespace TeenAgent {

Common::Rect Actor::render(Graphics::Surface *surface, const Common::Point &position, uint8 orientation, int delta_frame) {
	const uint8 frames_left_right[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	const uint8 frames_up[] = {18, 19, 20, 21, 22, 23, 24, 25, };
	const uint8 frames_down[] = {10, 11, 12, 13, 14, 15, 16, 17, };

	Surface *s = NULL;

	if (delta_frame == 0) {
		index = 0; //static animation
	}
	int dx, dy;
	switch (orientation) {
	case Object::kActorLeft:
	case Object::kActorRight:
		if (index >= sizeof(frames_left_right))
			index = 1;
		s = frames + frames_left_right[index];
		dx = 11;
		dy = 62;
		break;
	case Object::kActorUp:
		if (index >= sizeof(frames_up))
			index = 1;
		s = frames + frames_up[index];
		dx = 29;
		dy = 67;
		break;
	case Object::kActorDown:
		if (index >= sizeof(frames_down))
			index = 1;
		s = frames + frames_down[index];
		dx = 29;
		dy = 67;
		break;
	default:
		return Common::Rect();
	}
	index += delta_frame;

	int xp = position.x - dx, yp = position.y - dy;
	if (xp < 0)
		xp = 0;
	if (xp + s->w > 320)
		xp = 320 - s->w;

	if (yp < 0)
		yp = 0;
	if (yp + s->h > 200)
		yp = 200 - s->h;
	
	return s != NULL? s->render(surface, xp, yp, orientation == Object::kActorLeft): Common::Rect();
}

} // End of namespace TeenAgent
