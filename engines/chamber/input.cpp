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

#include "common/events.h"
#include "common/system.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/dialog.h"
#include "chamber/input.h"
#include "chamber/cursor.h"
#include "chamber/print.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"
#include "chamber/timer.h"
#include "chamber/ifgm.h"

namespace Chamber {

byte have_mouse = 0;
byte have_joystick = 0;
byte key_held;
volatile byte key_direction;
volatile byte key_code;

volatile byte keyboard_scan;
volatile byte keyboard_specials;
volatile byte keyboard_arrows;
volatile byte keyboard_buttons;

byte buttons_repeat = 0;
byte buttons;
byte right_button;
byte key_direction_old;
byte accell_countdown;
uint16 accelleration = 1;
byte mouseButtons = 0;

void PollDiscrete(void);

byte ChamberEngine::readKeyboardChar() {
	Common::Event event;

	while (true) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				return event.kbd.ascii;

			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
				_shouldQuit = true;
				return 0;

			default:
				break;
			}

			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}
}

void ClearKeyboard(void) {
}

void SetInputButtons(byte keys) {
	if (keys & 2)
		right_button = ~0;
	if (keys & 1)
		right_button = 0;
	buttons = keys;
	buttons_repeat = keys;
}

byte PollMouse(void) {
	PollInput();

	return buttons;
}

byte PollKeyboard(void) {
	byte direction = key_direction;
	if (direction && direction == key_direction_old) {
		if (++accell_countdown == 10) {
			accelleration++;
			accell_countdown = 0;
		}
	} else {
		accelleration = 1;
		accell_countdown = 0;
	}
	key_direction_old = direction;

	if (direction & 0x0F) {
		if (direction == 1) {
			cursor_x += accelleration;
			if (cursor_x >= 304) /*TODO: >*/
				cursor_x = 304;
		} else {
			cursor_x -= accelleration;
			if ((int16)cursor_x < 0)
				cursor_x = 0;
		}
	}

	if (direction & 0xF0) {
		if (direction == 0x10) {
			cursor_y += accelleration;
			if (cursor_y >= 184) /*TODO: >*/
				cursor_y = 184;
		} else {
			cursor_y -= accelleration;
			if ((int8)cursor_y < 0)
				cursor_y = 0;
		}
	}

	return key_code;
}

/*
Show game exit confirmation dialog and get user's input
*/
int16 AskQuitGame(void) {
	/*EU version comes without requited text string*/
	if (g_vm->getLanguage() != Common::EN_USA)
		return 0;

	int16 quit = -1;

	byte *msg = SeekToString(desci_data, 411);	/*DO YOU WANT TO QUIT ? (Y OR N).*/
	char_draw_max_width = 32;
	draw_x = 1;
	draw_y = 188;
	cga_DrawTextBox(msg, frontbuffer);

	Common::Event event;

	while (quit == -1) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_y)
					quit = 1;
				else if (event.kbd.keycode == Common::KEYCODE_n)
					quit = 0;
				break;

			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
				quit = 1;
				break;
			default:
				break;
			}
		}
	}
	cga_CopyScreenBlock(backbuffer, char_draw_max_width + 2, char_draw_coords_y - draw_y + 8, frontbuffer, cga_CalcXY_p(draw_x, draw_y));

	return quit;
}

void PollInputButtonsOnly() {
	PollInput();
}

void PollInput(void) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_SPACE)
				mouseButtons |= 1;
			else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				if (g_vm->getLanguage() == Common::EN_USA) {
					if (AskQuitGame() != 0)
						g_vm->_shouldQuit = true;
				}
			}
			break;

		case Common::EVENT_KEYUP:
			if (event.kbd.keycode == Common::KEYCODE_SPACE)
				mouseButtons &= ~1;
			break;

		case Common::EVENT_RETURN_TO_LAUNCHER:
		case Common::EVENT_QUIT:
			g_vm->_shouldQuit = true;
			break;

		case Common::EVENT_MOUSEMOVE:
			cursor_x = event.mouse.x;
			cursor_y = event.mouse.y;
			break;

		case Common::EVENT_LBUTTONDOWN:
			mouseButtons |= 1;
			break;

		case Common::EVENT_LBUTTONUP:
			mouseButtons &= ~1;
			break;

		case Common::EVENT_RBUTTONDOWN:
			mouseButtons |= 2;
			break;

		case Common::EVENT_RBUTTONUP:
			mouseButtons &= ~2;
			break;

		default:
			break;
		}
	}

	SetInputButtons(mouseButtons);
}

void ProcessInput(void) {
	PollInput();
	UpdateCursor();
	DrawCursor(frontbuffer);
}

void KeyboardIsr() {
	warning("STUB: KeyboardIsr()");
#if 0
	byte scan, strobe;
	scan = inportb(0x60);
	/*consume scan from kbd. controller*/
	strobe = inportb(0x61);
	outportb(0x61, strobe | 0x80);
	outportb(0x61, strobe);

	if (scan == 1) {        /*esc*/
		esc_pressed = ~0;
	} else {
		if (scan & 0x80) {      /*key release?*/
			key_code = 0;
			key_direction = 0;
		} else {
			switch (scan) {
			case 0x39:  /*space*/
				key_code = scan;
				key_direction = 0;
				break;
			case 0x48:  /*up*/
				key_code = 0;
				key_direction = 0xF0;
				break;
			case 0x50:  /*down*/
				key_code = 0;
				key_direction = 0x10;
				break;
			case 0x4B:  /*left*/
				key_code = 0;
				key_direction = 0x0F;
				break;
			case 0x4D:  /*right*/
				key_code = 0;
				key_direction = 0x01;
				break;
			}
		}
	}
	outportb(0x20, 0x20);
#endif
}

void InitInput(void) {
	have_mouse = 1;
}

void UninitInput(void) {
}

} // End of namespace Chamber
