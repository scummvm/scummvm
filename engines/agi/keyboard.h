/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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
 * $URL$
 * $Id$
 *
 */

#ifndef __AGI_KEYBOARD_H
#define __AGI_KEYBOARD_H

#include "common/stdafx.h"

#include "agi/agi.h"

namespace Agi {

/* QNX4 has a KEY_DOWN defined which we don't need to care about */
#undef KEY_DOWN

/* Allegro defines these */
#undef KEY_BACKSPACE
#undef KEY_ENTER
#undef KEY_LEFT
#undef KEY_RIGHT
#undef KEY_UP
#undef KEY_PGUP
#undef KEY_PGDN
#undef KEY_HOME
#undef KEY_END

#define KEY_BACKSPACE	0x08
#define	KEY_ESCAPE	0x1B
#define KEY_ENTER	0x0D
#define KEY_UP		0x4800
#define	KEY_DOWN	0x5000
#define KEY_LEFT	0x4B00
#define KEY_STATIONARY	0x4C00
#define KEY_RIGHT	0x4D00

#define KEY_DOWN_LEFT	0x4F00
#define KEY_DOWN_RIGHT	0x5100
#define KEY_UP_LEFT	0x4700
#define KEY_UP_RIGHT	0x4900

#define KEY_STATUSLN	0xd900	/* F11 */
#define KEY_PRIORITY	0xda00	/* F12 */

#define KEY_PGUP	0x4900	/* Page Up (fixed by Ziv Barber) */
#define KEY_PGDN	0x5100	/* Page Down */
#define KEY_HOME	0x4700	/* Home */
#define KEY_END		0x4f00	/* End * */

#ifdef USE_MOUSE
#define BUTTON_LEFT	0xF101	/* Left mouse button */
#define BUTTON_RIGHT	0xF202	/* Right mouse button */
#endif

#define KEY_SCAN(k)	(k >> 8)
#define KEY_ASCII(k)	(k & 0xff)

extern uint8 scancode_table[];

void init_words(void);
void clean_input(void);
int do_poll_keyboard(void);
void clean_keyboard(void);
void handle_keys(int);
void handle_getstring(int);
int handle_controller(int);
void get_string(int, int, int, int);
uint16 agi_get_keypress(void);
int wait_key(void);
int wait_any_key(void);

}                             // End of namespace Agi

#endif				/* __AGI_KEYBOARD_H */
