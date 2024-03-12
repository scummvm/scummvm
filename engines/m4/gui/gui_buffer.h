
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_GUI_GUI_BUFFER_H
#define M4_GUI_GUI_BUFFER_H

#include "m4/m4_types.h"
#include "m4/graphics/gr_buff.h"
#include "m4/gui/gui_univ.h"

namespace M4 {

/**
 * Initialize any code associated with managing buffers in the GUI
 */
bool gui_buffer_system_init();

/**
 * Shutdown any code associated with buffers management
 */
void gui_buffer_system_shutdown();

/**
 * Register a Buffer with the view manager by creating a view mananger screen
 * @param x1		Where the screen should initially be placed, coords relative
					to the top left hand monitor corner.
 * @param y1		The screens initial "y" coord
 * @param scrnFlags	Flags defining the screens: layer, transparency,
					moveability, etc.
 * @param evtHandler	A pointer to the procedure to be executed when
					the view manager registers a keyboard or mouse event
 * @returns			The success of the call
 * @remarks			The user is responsible for keeping the Buffer *.
					Any changes to the contents will be made by the user.
 */
bool gui_buffer_register(int32 x1, int32 y1, Buffer *myBuf, uint32 scrnFlags, EventHandler evtHandler);
void gui_buffer_deregister(void *myBuf);

bool gui_GrBuff_register(int32 x1, int32 y1, GrBuff *myBuf, uint32 scrnFlags, EventHandler evtHandler);
void gui_buffer_activate(Buffer *myBuf);
bool gui_buffer_add_key(Buffer *myBuf, long myKey, HotkeyCB cb);

/**
 * Change which procedure will handle the events sent to the screen, which was
 * created to managed the Buffer specified.
 * @param myBuf			The Buffer specified.
 * @param evtHandler	The new procedure to handle keyboard and mouse events.
 */
bool gui_buffer_set_event_handler(void *myBuf, EventHandler evtHandler);

} // End of namespace M4

#endif
