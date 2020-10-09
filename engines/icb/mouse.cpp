/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/mouse.h"
#include "engines/icb/debug.h"

namespace ICB {

#define MAX_MOUSE_EVENTS 16
#define MOUSEFLASHFRAME 6

typedef struct {
	uint8 runTimeComp;  // type of runtime compression used for the frame data
	uint8 noAnimFrames; // number of frames in the anim
	int8 xHotSpot;
	int8 yHotSpot;
	uint8 mousew;
	uint8 mouseh;
} _mouseAnim;

int32 mousex;
int32 mousey;

static uint8 mouseBacklog = 0;
static uint8 mouseLogPos = 0;
static _mouseEvent mouseLog[MAX_MOUSE_EVENTS];

_mouseEvent *lastMouseEvent; // keep track of last mouse event for debugging

void LogMouseEvent(uint16 buttons) {
	_mouseEvent *me;

	if (mouseBacklog == MAX_MOUSE_EVENTS - 1) { // We need to leave the one which is
		// the current event alone!
		return;
	}

	me = &mouseLog[(mouseBacklog + mouseLogPos) % MAX_MOUSE_EVENTS];
	me->buttons = buttons;
	mouseBacklog += 1;
}

int32 DecompressMouse(uint8 *decomp, uint8 *comp, int32 size) {
	int32 i = 0;

	while (i < size) {
		if (*comp > 183) {
			*decomp++ = *comp++;
			i += 1;
		} else {
			memset(decomp, 0, *comp);
			decomp += *comp;
			i += *comp++;
		}
	}
	return (1);
}

_mouseEvent *MouseEvent() {
	_mouseEvent *me;

	if (mouseBacklog) {
		me = &mouseLog[mouseLogPos];
		if (++mouseLogPos == MAX_MOUSE_EVENTS) {
			mouseLogPos = 0;
		}
		mouseBacklog -= 1;

		lastMouseEvent = me; // keep track of last mouse event for debugging

		return (me);
	}
	return (NULL);
}

} // End of namespace ICB
