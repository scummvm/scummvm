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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/input.h"
#include "chamber/cursor.h"
#include "chamber/cga.h"

namespace Chamber {


unsigned char have_mouse;
unsigned char key_held;
volatile unsigned char key_direction;
volatile unsigned char key_code;
volatile unsigned char esc_pressed;
unsigned char buttons_repeat = 0;
unsigned char buttons;
unsigned char right_button;
unsigned char key_direction_old;
unsigned char accell_countdown;
unsigned int accelleration = 1;

void interrupt(*old_keyboard_isr)(void);

unsigned char ReadKeyboardChar(void) {
#ifdef DEBUG
	if (old_keyboard_isr) {
		do {
			PollInput();
		} while (!buttons);
	} else
#endif
		return (unsigned char)getch();
}

void ClearKeyboard(void) {
	while (kbhit()) {
		getch();
	}
}

void interrupt NullIsr(void) {
	/*nothing*/
}

void SetInputButtons(unsigned char keys) {
	if (keys && buttons_repeat) {
		/*ignore buttons repeat*/
		buttons = 0;
		return;
	}
	if (keys & 2)
		right_button = ~0;
	if (keys & 1)
		right_button = 0;
	buttons = keys;
	buttons_repeat = keys;
}

unsigned char PollMouse(void) {
	union REGS reg;
	reg.x.ax = 3;
	int86(0x33, &reg, &reg);
	cursor_x = reg.x.cx;
	cursor_y = reg.h.dl;
	return reg.h.bl;    /*buttons*/
}

unsigned char PollKeyboard(void) {
	unsigned char direction = key_direction;
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
			if ((signed int)cursor_x < 0)
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
			if ((signed char)cursor_y < 0)
				cursor_y = 0;
		}
	}

	return key_code;
}

void PollInput(void) {
	unsigned char keys;
	if (have_mouse)
		keys = PollMouse();
	else
		keys = PollKeyboard();
	SetInputButtons(keys);
}

void ProcessInput(void) {
	PollInput();
	UpdateCursor();
	DrawCursor(frontbuffer);
}

void interrupt KeyboardIsr() {
	unsigned char scan, strobe;
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
}

void InitInput(void) {
	/*disable critical errors handler*/
	setvect(0x24, NullIsr);

	/*is mouse present?*/
	if (getvect(0x33)) {
		union REGS reg;
		reg.x.ax = 0;
		int86(0x33, &reg, &reg);
		if (reg.x.ax == 0xFFFF) {
			/*mouse detected*/

			reg.x.ax = 0xF; /*set cursor speed*/
			reg.x.cx = 16;
			reg.x.dx = 16;
			int86(0x33, &reg, &reg);

			reg.x.ax = 7;   /*set x range*/
			reg.x.cx = 0;
			reg.x.dx = 303;
			int86(0x33, &reg, &reg);

			reg.x.ax = 8;   /*set y range*/
			reg.x.cx = 0;
			reg.x.dx = 183;
			int86(0x33, &reg, &reg);

			reg.x.ax = 4;   /*set coords*/
			cursor_x = reg.x.cx = 10;
			cursor_y = reg.x.dx = 10;
			int86(0x33, &reg, &reg);

			have_mouse = ~0;
		}
	}

	if (have_mouse)
		return;

	/*no mouse*/

	old_keyboard_isr = getvect(9);
	setvect(9, KeyboardIsr);
}

void UninitInput(void) {
	if (have_mouse)
		return;

	setvect(9, old_keyboard_isr);
#ifdef DEBUG
	old_keyboard_isr = 0;
#endif
}

} // End of namespace Chamber
