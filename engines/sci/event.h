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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_ENGINE_EVENT_H
#define SCI_ENGINE_EVENT_H

#include "common/list.h"

namespace Sci {

#define SCI_INPUT_DEFAULT_CLOCKTIME 100000
#define SCI_INPUT_DEFAULT_REDRAWTIME 30000


struct sciEvent {
	short type;
	short data;
	short buckybits;
	short character; /* for keyboard events: 'data' after applying
			 ** the effects of 'buckybits', e.g. if
			 **   type == SCI_EVT_KEYBOARD
			 **   data == 'a'
			 **   buckybits == SCI_EVM_LSHIFT
			 ** then
			 **   character == 'A'
			 ** For 'Alt', characters are interpreted by their
			 ** PC keyboard scancodes.
			 */
};

/*Values for type*/
#define SCI_EVENT_NONE            0
#define SCI_EVENT_MOUSE_PRESS     (1<<0)
#define SCI_EVENT_MOUSE_RELEASE   (1<<1)
#define SCI_EVENT_KEYBOARD        (1<<2)
#define SCI_EVENT_JOYSTICK        (1<<6)
#define SCI_EVENT_SAID            (1<<7)
/*Fake values for other events*/
#define SCI_EVENT_ERROR           (1<<10)
#define SCI_EVENT_QUIT            (1<<11)
#define SCI_EVENT_PEEK            (1<<15)
/* The QUIT event may be used to signal an external 'quit' command being
** issued to the gfx driver.  */
#define SCI_EVENT_ANY             0x7fff

/* Keycodes of special keys: */
#define SCI_KEY_ESC 27
#define SCI_KEY_BACKSPACE 8
#define SCI_KEY_ENTER 13
#define SCI_KEY_TAB '\t'
#define SCI_KEY_SHIFT_TAB (0xf << 8)

#define SCI_KEY_HOME (71 << 8)	// 7
#define SCI_KEY_UP (72 << 8)		// 8
#define SCI_KEY_PGUP (73 << 8)	// 9
//
#define SCI_KEY_LEFT (75 << 8)	// 4 
#define SCI_KEY_CENTER (76 << 8)	// 5
#define SCI_KEY_RIGHT (77 << 8)	// 6
//
#define SCI_KEY_END (79 << 8)		// 1
#define SCI_KEY_DOWN (80 << 8)	// 2
#define SCI_KEY_PGDOWN (81 << 8)	// 3
//
#define SCI_KEY_INSERT (82 << 8)	// 0
#define SCI_KEY_DELETE (83 << 8)	// .

#define SCI_KEY_F1 (59<<8)
#define SCI_KEY_F2 (60<<8)
#define SCI_KEY_F3 (61<<8)
#define SCI_KEY_F4 (62<<8)
#define SCI_KEY_F5 (63<<8)
#define SCI_KEY_F6 (64<<8)
#define SCI_KEY_F7 (65<<8)
#define SCI_KEY_F8 (66<<8)
#define SCI_KEY_F9 (67<<8)
#define SCI_KEY_F10 (68<<8)

/*Values for buckybits */
#define SCI_KEYMOD_RSHIFT          (1<<0)
#define SCI_KEYMOD_LSHIFT          (1<<1)
#define SCI_KEYMOD_CTRL            (1<<2)
#define SCI_KEYMOD_ALT             (1<<3)
#define SCI_KEYMOD_SCRLOCK         (1<<4)
#define SCI_KEYMOD_NUMLOCK         (1<<5)
#define SCI_KEYMOD_CAPSLOCK        (1<<6)
#define SCI_KEYMOD_INSERT          (1<<7)

#define SCI_KEYMOD_NO_FOOLOCK      (~(SCI_KEYMOD_SCRLOCK | SCI_KEYMOD_NUMLOCK | SCI_KEYMOD_CAPSLOCK | SCI_KEYMOD_INSERT))
#define SCI_KEYMOD_ALL             0xFF

class SciEvent {
public:
	SciEvent();
	~SciEvent();

	sciEvent get(unsigned int mask);

private:
	int controlify (int ch);
	int altify (int ch);
	int shiftify (int c);
	int numlockify (int c);
	sciEvent getFromScummVM();

	Common::List<sciEvent> _events;
};

void kernel_sleep(SciEvent *event, uint32 msecs); // is in kernel.cpp

} // End of namespace Sci

#endif
