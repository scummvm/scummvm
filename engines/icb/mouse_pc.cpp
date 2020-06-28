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
#include "engines/icb/global_objects_pc.h" // for surface_manager
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

int32 DrawMouse() {
	uint8 *ad;
	uint32 pitch;

	ad = surface_manager->Lock_surface(working_buffer_id);
	pitch = surface_manager->Get_pitch(working_buffer_id);
	uint32 bytesPerPixel = 4;
	ad += (mousey * pitch) + mousex * bytesPerPixel;

	while (bytesPerPixel--)
		*ad++ = 255;

	surface_manager->Unlock_surface(working_buffer_id);

	return (0);
}

} // End of namespace ICB
