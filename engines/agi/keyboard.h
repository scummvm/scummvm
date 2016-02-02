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

#ifndef AGI_KEYBOARD_H
#define AGI_KEYBOARD_H

namespace Agi {

#define keyEnqueue(k) \
	do { \
		_keyQueue[_keyQueueEnd++] = (k); \
		_keyQueueEnd %= KEY_QUEUE_SIZE; \
	} while (0)
#define keyDequeue(k) \
	do { \
		(k) = _keyQueue[_keyQueueStart++]; \
		_keyQueueStart %= KEY_QUEUE_SIZE; \
	} while (0)

// Class to turn on synthetic events temporarily. Usually until the end of the
// current function.

class AllowSyntheticEvents {
private:
	AgiEngine *_vm;
public:
	AllowSyntheticEvents(AgiEngine *vm) : _vm(vm) {
		_vm->allowSynthetic(true);
	}

	~AllowSyntheticEvents() {
		_vm->allowSynthetic(false);
	}
};

#define AGI_KEY_BACKSPACE  0x08
#define AGI_KEY_ESCAPE     0x1B
#define AGI_KEY_ENTER      0x0D
#define AGI_KEY_UP         0x4800
#define AGI_KEY_DOWN       0x5000
#define AGI_KEY_LEFT       0x4B00
#define AGI_KEY_STATIONARY 0x4C00
#define AGI_KEY_RIGHT      0x4D00

#define AGI_KEY_DOWN_LEFT  0x4F00
#define AGI_KEY_DOWN_RIGHT 0x5100
#define AGI_KEY_UP_LEFT    0x4700
#define AGI_KEY_UP_RIGHT   0x4900

#define AGI_KEY_F1  0x3B00
#define AGI_KEY_F2  0x3C00
#define AGI_KEY_F3  0x3D00
#define AGI_KEY_F4  0x3E00
#define AGI_KEY_F5  0x3F00
#define AGI_KEY_F6  0x4000
#define AGI_KEY_F7  0x4100
#define AGI_KEY_F8  0x4200
#define AGI_KEY_F9  0x4300
#define AGI_KEY_F10 0x4400
#define AGI_KEY_F11 0xd900  // F11
#define AGI_KEY_F12 0xda00  // F12

#define AGI_KEY_PAGE_UP     0x4900  // Page Up (fixed by Ziv Barber)
#define AGI_KEY_PAGE_DOWN   0x5100  // Page Down
#define AGI_KEY_HOME        0x4700  // Home
#define AGI_KEY_END         0x4f00  // End *

#define AGI_MOUSE_BUTTON_LEFT  0xF101  // Left mouse button
#define AGI_MOUSE_BUTTON_RIGHT 0xF202  // Right mouse button
#define AGI_MOUSE_WHEEL_UP     0xF203  // Mouse wheel up
#define AGI_MOUSE_WHEEL_DOWN   0xF204  // Mouse wheel down

// special menu triggers
// Attention: at least Mixed Up Mother Goose on Apple IIgs actually hooks ESC for menu only
// Which is why we have to check, if the corresponding trigger is hooked before changing it
// And otherwise simply use the regular ESC.
#define AGI_MENU_TRIGGER_PC       0x001B // will trigger menu for PC
#define AGI_MENU_TRIGGER_APPLE2GS 0x0301 // will trigger menu for AppleIIgs + Amiga
#define AGI_MENU_TRIGGER_ATARIST  0x0101 // will trigger menu for Atari ST

extern const uint8 scancodeTable[];

} // End of namespace Agi

#endif /* AGI_KEYBOARD_H */
