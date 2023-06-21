
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
extern bool gui_buffer_system_init();

extern void gui_buffer_system_shutdown();
extern bool gui_buffer_register(int32 x1, int32 y1, Buffer *myBuf, uint32 scrnFlags, EventHandler evtHandler);

extern bool gui_GrBuff_register(int32 x1, int32 y1, GrBuff *myBuf, uint32 scrnFlags, EventHandler evtHandler);

extern void gui_buffer_deregister(Buffer *myBuf);

extern bool gui_GrBuff_register(int32 x1, int32 y1, GrBuff *myBuf, uint32 scrnFlags, EventHandler evtHandler);
extern void gui_buffer_activate(Buffer *myBuf);
extern bool gui_buffer_add_key(Buffer *myBuf, long myKey, HotkeyCB cb);
extern bool gui_buffer_set_event_handler(Buffer *myBuf, EventHandler evtHandler);

} // End of namespace M4

#endif
