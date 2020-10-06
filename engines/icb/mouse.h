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

#ifndef ICB_MOUSE_H
#define ICB_MOUSE_H

#include "engines/icb/p4_generic.h"

namespace ICB {

// Mouse defines
#define RDMOUSE_NOFLASH 0x00
#define RDMOUSE_FLASH 0x01

#define RENDERWIDE 640
#define RENDERDEEP 480

// Mouse button defines
#define RD_LEFTBUTTONDOWN 0x01
#define RD_LEFTBUTTONUP 0x02
#define RD_RIGHTBUTTONDOWN 0x04
#define RD_RIGHTBUTTONUP 0x08
#define RD_LEFTBUTTONDBLCLK 0x10
#define RD_RIGHTBUTTONDBLCLK 0x20

typedef struct { uint16 buttons; } _mouseEvent;

extern _mouseEvent *lastMouseEvent; // keep track of last mouse event for debugging

extern int32 mousex; // Mouse x coordinate
extern int32 mousey; // Mouse y coordinate

extern void LogMouseEvent(uint16 buttons); // Adds a mouse event to the log
extern int32 DrawMouse();                  // Renders the mouse onto the back buffer.
extern _mouseEvent *MouseEvent();

} // End of namespace ICB

#endif
