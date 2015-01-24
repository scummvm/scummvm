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

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"

#include "common/endian.h"
#include "common/rect.h"

namespace Mortevielle {

MouseHandler::MouseHandler(MortevielleEngine *vm) {
	_vm = vm;
}

/**
 * Initialize the mouse
 * @remarks	Originally called 'init_mouse'
 */
void MouseHandler::initMouse() {
	_pos = Common::Point(0, 0);
	_vm->setMouseClick(false);
}

/**
 * Backs up the area behind where the mouse cursor is to be drawn
 * @remarks	Originally called 'hide_mouse'
 */
void MouseHandler::hideMouse() {
	// No implementation needed in ScummVM
}

/**
 * Draws the mouse cursor
 * @remarks	Originally called 'show_mouse'
 */
void MouseHandler::showMouse() {
	// ScummVM implementation uses CursorMan for drawing the cursor
}

/**
 * Set mouse position
 * @remarks	Originally called 'pos_mouse'
 */
void MouseHandler::setMousePosition(Common::Point newPos) {
	if (newPos.x > 314 * kResolutionScaler)
		newPos.x = 314 * kResolutionScaler;
	else if (newPos.x < 0)
		newPos.x = 0;
	if (newPos.y > 199)
		newPos.y = 199;
	else if (newPos.y < 0)
		newPos.y = 0;
	if (newPos == _pos)
		return;

	// Set the new position
	_vm->setMousePos(newPos);
}

/**
 * Get mouse poisition
 * @remarks	Originally called 'read_pos_mouse'
 */
void MouseHandler::getMousePosition(int &x, int &y, bool &click) {
	x = _vm->getMousePos().x;
	y = _vm->getMousePos().y;
	click = _vm->getMouseClick();
}

/**
 * Move mouse
 * @remarks	Originally called 'mov_mouse'
 */
void MouseHandler::moveMouse(bool &funct, char &key) {
	int cx, cy;
	bool click;

	// Set defaults and check pending events
	funct = false;
	key = '\377';
	bool p_key = _vm->keyPressed();

	// If mouse button clicked, return it
	if (_vm->getMouseClick())
		return;

	// Handle any pending keypresses
	while (p_key) {
		if (_vm->shouldQuit())
			return;

		char in1 = _vm->getChar();
		getMousePosition(cx, cy, click);
		switch (toupper(in1)) {
		case '4':
			cx -= 8;
			break;
		case '2':
			cy += 8;
			break;
		case '6':
			cx += 8;
			break;
		case '8':
			cy -= 8;
			break;
		case '7':
			cy = 1;
			cx = 1;
			break;
		case '1':
			cx = 1;
			cy = 190;
			break;
		case '9':
			cx = 315 * kResolutionScaler;
			cy = 1;
			break;
		case '3':
			cy = 190;
			cx = 315 * kResolutionScaler;
			break;
		case '5':
			cy = 100;
			cx = 155 * kResolutionScaler;
			break;
		case ' ':
		case '\15':
			_vm->setMouseClick(true);
			return;
			break;
		case '\33':
			p_key = _vm->keyPressed();

			if (p_key) {
				char in2 = _vm->getChar();

				if ((in2 >= ';') && (in2 <= 'D')) {
					funct = true;
					key = in2;
					return;
				} else {
					switch (in2) {
					case 'K':
						--cx;
						break;
					case 'P':
						++cy;
						break;
					case 'M':
						cx += 2;
						break;
					case 'H':
						--cy;
						break;
					case 'G':
						--cx;
						--cy;
						break;
					case 'I':
						++cx;
						--cy;
						break;
					case 'O':
						--cx;
						++cy;
						break;
					case 'Q':
						++cx;
						++cy;
						break;
					default:
						break;
					}    // case
				}
			}
			break;
		case 'I':
			cx = kResolutionScaler * 32;
			cy = 8;
			break;
		case 'D':
			cx = 80 * kResolutionScaler;
			cy = 8;
			break;
		case 'A':
			cx = 126 * kResolutionScaler;
			cy = 8;
			break;
		case 'S':
			cx = 174 * kResolutionScaler;
			cy = 8;
			break;
		case 'P':
			cx = 222 * kResolutionScaler;
			cy = 8;
			break;
		case 'F':
			cx = kResolutionScaler * 270;
			cy = 8;
			break;
		case '\23':
			_vm->_soundOff = !_vm->_soundOff;
			return;
			break;
		case '\24':           // ^T => mode tandy
			funct = true;
			key = '\11';
			break;
		case '\10':           // ^H => mode Hercule
			funct = true;
			key = '\7';
			break;
		case '\1':
		case '\3':
		case '\5':
			funct = true;
			key = in1;
			break;
		default:
			break;
		}

		setMousePosition(Common::Point(cx, cy));
		p_key = _vm->keyPressed();
	}
}

/**
 * Mouse function : Is mouse in a given rect?
 * @remarks	Originally called 'dans_rect'
 */
bool MouseHandler::isMouseIn(Common::Rect r) {
	int x, y;
	bool click;

	getMousePosition(x, y, click);
	if ((x > r.left) && (x < r.right) && (y > r.top) && (y < r.bottom))
		return true;

	return false;
}

} // End of namespace Mortevielle
