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

Actor::Actor() : head_index(0) {}

//idle animation lists at dseg: 0x6540

Common::Rect Actor::render(Graphics::Surface *surface, const Common::Point &position, uint8 orientation, int delta_frame, bool render_head, uint zoom) {
	const uint8 frames_left_right[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	const uint8 frames_up[] = {18, 19, 20, 21, 22, 23, 24, 25, };
	const uint8 frames_down[] = {10, 11, 12, 13, 14, 15, 16, 17, };

	const uint8 frames_head_left_right[] = { 
		0x27, 0x1a, 0x1b, 
		0x27, 0x1c, 0x1d, 
		0x27, 0x1a, 
		0x27, 0x1e, 0x1f, 
		0x27, 0x1a, 0x1b, 
		0x27, 0x1c, 
		0x27, 0x1e, 
		0x27, 0x1a, 
	};
	
	const uint8 frames_head_up[] = {
		0x29, 0x25, 0x29, 0x29, 
		0x26, 0x29, 0x26, 0x29, 
		0x29, 0x25, 0x29, 0x25, 
		0x29, 0x29, 0x29, 0x25, 
		0x25, 0x29, 0x29, 0x26
	};
	const uint8 frames_head_down[] = {
		0x20, 0x21, 0x22, 0x23, 
		0x28, 0x24, 0x28, 0x28, 
		0x24, 0x28, 0x20, 0x21, 
		0x20, 0x23, 0x28, 0x20, 
		0x28, 0x28, 0x20, 0x28
	};

	Surface *s = NULL, *head = NULL;

	if (delta_frame == 0) {
		index = 0; //static animation
	}
	int dx, dy;
	switch (orientation) {
	case Object::kActorLeft:
	case Object::kActorRight:
		if (render_head) {
			if (head_index >= sizeof(frames_head_left_right))
				head_index = 0;
			head = frames + frames_head_left_right[head_index];
			++head_index;
		}

		if (index >= sizeof(frames_left_right))
			index = 1;
		s = frames + frames_left_right[index];
		dx = 11;
		dy = 62;
		break;
	case Object::kActorUp:
		if (render_head) {
			if (head_index >= sizeof(frames_head_up))
				head_index = 0;
			head = frames + frames_head_up[head_index];
			++head_index;
		}

		if (index >= sizeof(frames_up))
			index = 1;
		s = frames + frames_up[index];
		dx = 11;
		dy = 62;
		break;
	case Object::kActorDown:
		if (render_head) {
			if (head_index >= sizeof(frames_head_down))
				head_index = 0;
			head = frames + frames_head_down[head_index];
			++head_index;
		}

		if (index >= sizeof(frames_down))
			index = 1;
		s = frames + frames_down[index];
		dx = 11;
		dy = 62;
		break;
	default:
		return Common::Rect();
	}
	index += delta_frame;
	if (s == NULL) {
		warning("no surface, skipping");
		return Common::Rect();
	}

	Common::Rect dirty;
	Common::Rect clip(0, 0, s->w, s->h);
	if (head != NULL) 
		clip.top = head->h;

	int xp = position.x - dx, yp = position.y - dy;
	if (xp < 0)
		xp = 0;
	if (xp + s->w > 320)
		xp = 320 - s->w;

	if (yp < 0)
		yp = 0;
	if (yp + clip.top + clip.height() > 200)
		yp = 200 - clip.top - clip.height();
	
	dirty = s->render(surface, xp, yp + clip.top, orientation == Object::kActorLeft, clip, zoom);

	if (head != NULL)
		dirty.extend(head->render(surface, xp, yp, orientation == Object::kActorLeft, Common::Rect(), zoom));

	return dirty;
}

} // End of namespace TeenAgent
