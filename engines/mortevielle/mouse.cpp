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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/endian.h"
#include "common/rect.h"
#include "mortevielle/mouse.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/**
 * Initialize the mouse
 * @remarks	Originally called 'init_mouse'
 */
void MouseHandler::initMouse() {
	_counter = 0;
	_pos = Common::Point(0, 0);

	g_vm->setMouseClick(false);
}

/**
 * Hide the mouse
 * @remarks	Originally called 'hide_mouse'
 */
void MouseHandler::hideMouse() {
	--_counter;
	if (_counter == 0) {
		int j = 0;
		switch (g_vm->_currGraphicalDevice) {
		case MODE_CGA: {
			int k = 0;
			j = ((uint)_pos.y >> 1) * 80 + ((uint)_pos.x >> 2);
			do {
				WRITE_LE_UINT16(&g_vm->_mem[0xb000 * 16 + j], s_s[0][k]);
				WRITE_LE_UINT16(&g_vm->_mem[0xb800 * 16 + j + 2], s_s[1][k]);
				WRITE_LE_UINT16(&g_vm->_mem[0xba00 * 16 + j], s_s[2][k]);
				WRITE_LE_UINT16(&g_vm->_mem[0xba00 * 16 + j + 2], s_s[3][k]);
				j += 80;
				++k;
			} while (k < 5);
			}
			break;
		case MODE_AMSTRAD1512: {
			bool imp = odd(_pos.y);
			for (int i = 0; i <= 3; ++i) {
				int k = 0;
				j = 0;
				do {
					if (imp) {
						WRITE_LE_UINT16(&g_vm->_mem[0xb800 * 16 + j], s_s[i][k]);
						j += 80 - 0x2000;
					} else {
						WRITE_LE_UINT16(&g_vm->_mem[0xb800 * 16 + j], s_s[i][k]);
						j += 0x2000;
					}
					imp = !imp;
					++k;
				} while (k < 8);
			}
			break;
			}
		case MODE_EGA: {
			int i = 0;
			do {
				int k = 0;
				j = 0;
				do {
					// Useless ?
					// ps = mem[0xa000 * 16 + j];
					g_vm->_mem[0xa000 * 16 + j] = lo(s_s[i][k]);

					// Useless ??
					// ps = mem[0xa000 * 16 + j + 1];
					g_vm->_mem[0xa000 * 16 + j + 1] = hi(s_s[i][k]);
					j += 80;
					++k;
				} while (k < 8);
				++i;
			} while (i != 4);
			}
			break;
		case MODE_HERCULES:
			j = ((uint)_pos.y >> 1) * 80 + ((uint)_pos.x >> 3);
			for (int i = 0; i <= 5; ++i) {
				for (int k = 0; k <= 3; ++k)
					WRITE_LE_UINT16(&g_vm->_mem[0xb000 * 16 + k * 0x200 + j], s_s[i][k]);
				j += 80;
			}
			break;
		case MODE_TANDY: {
			j = ((uint)_pos.y >> 2) * 160 + ((uint)_pos.x >> 1);
			int k = 0;
			do {
				for (int i = 0; i <= 3; ++i) {
					WRITE_LE_UINT16(&g_vm->_mem[0xb800 * 16 + 0x200 * i + j], s_s[k][i + (k << 2)]);
					WRITE_LE_UINT16(&g_vm->_mem[0xb800 * 16 + 0x200 * i + j + 2], s_s[k + 3][i + (k << 2)]);
				}
				j += 160;
				++k;
			} while (k != 3);
			}
			break;
		default:
			break;
		}     // case Gd
	}
}

/**
 * Show mouse
 * @remarks	Originally called 'show_mouse'
 */
void MouseHandler::showMouse() {
	int k, l;

	++_counter;
	if (_counter != 1)
		return;
	int j = 0;
	int i = _pos.x & 7;
	switch (g_vm->_currGraphicalDevice) {
	case MODE_CGA:
		k = 0;
		j = ((uint)_pos.y >> 1) * 80 + ((uint)_pos.x >> 2);
		do {
			s_s[0][k] = READ_LE_UINT16(&g_vm->_mem[0xb800 * 16 + j]);
			s_s[1][k] = READ_LE_UINT16(&g_vm->_mem[0xb800 * 16 + j + 2]);
			s_s[2][k] = READ_LE_UINT16(&g_vm->_mem[0xba00 * 16 + j]);
			s_s[3][k] = READ_LE_UINT16(&g_vm->_mem[0xba00 * 16 + j + 2]);
			j += 80;
			++k;
		} while (k < 5);
		break;
	case MODE_AMSTRAD1512: {
		bool imp = odd(_pos.y);
		for (i = 0; i <= 3; ++i) {
			j = 0;
			imp = odd(_pos.y);
			k = 0;
			do {
				if (imp) {
					s_s[i][k] = READ_LE_UINT16(&g_vm->_mem[0xb800 * 16 + j]);
					j += 80 - 0x2000;
				} else {
					s_s[i][k] = READ_LE_UINT16(&g_vm->_mem[0xb800 * 16 + j]);
					j += 0x2000;
				}
				imp = !imp;
				++k;
			} while (k < 8);
		}
		break;
		}
	case MODE_EGA:
		l = 0;
		do {
			k = 0;
			j = 0;
			do {
				s_s[l][k] = g_vm->_mem[0xa000 * 16 + j] + (g_vm->_mem[(0xa000 * 16) + j + 1] << 8);
				j += 80;
				++k;
			} while (k < 8);
			++l;
		} while (l != 4);
		break;
	case MODE_HERCULES:
		j = ((uint)_pos.y >> 1) * 80 + ((uint)_pos.x >> 3);
		for (i = 0; i <= 5; ++i) {
			for (k = 0; k <= 3; ++k)
				s_s[i][k] = READ_LE_UINT16(&g_vm->_mem[0xb000 * 16 + k * 0x200 + j]);
			j += 80;
		}
		break;
	case MODE_TANDY:
		j = ((uint)_pos.y >> 2) * 160 + ((uint)_pos.x >> 1);
		k = 0;
		do {
			for (i = 0; i <= 3; ++i) {
				s_s[k][i + (k << 2)] = READ_LE_UINT16(&g_vm->_mem[0xb800 * 16 + 0x200 * i + j]);
				s_s[k + 3][i + (k << 2)] = READ_LE_UINT16(&g_vm->_mem[0xb800 * 16 + 0x200 * i + j + 2]);
			}
			j += 160;
			++k;
		} while (k != 3);
		break;
	default:
		break;
	}    //  case Gd
}

/**
 * Set mouse position
 * @remarks	Originally called 'pos_mouse'
 */
void MouseHandler::setMousePosition(Common::Point newPos) {
	if (newPos.x > 314 * g_vm->_res)
		newPos.x = 314 * g_vm->_res;
	else if (newPos.x < 0)
		newPos.x = 0;
	if (newPos.y > 199)
		newPos.y = 199;
	else if (newPos.y < 0)
		newPos.y = 0;
	if (newPos == _pos)
		return;

	// Set the new position
	g_vm->setMousePos(newPos);
}

/**
 * Get mouse poisition
 * @remarks	Originally called 'read_pos_mouse'
 */
void MouseHandler::getMousePosition(int &x, int &y, int &c) {
	x = g_vm->getMousePos().x;
	y = g_vm->getMousePos().y;
	c = g_vm->getMouseClick() ? 1 : 0;
}

/**
 * Move mouse
 * @remarks	Originally called 'mov_mouse'
 */
void MouseHandler::moveMouse(bool &funct, char &key) {
	bool p_key;
	char in1, in2;
	int cx, cy, cd;

	// Set defaults and check pending events
	funct = false;
	key = '\377';
	p_key = g_vm->keyPressed();

	// If mouse button clicked, return it
	if (g_vm->getMouseClick())
		return;

	// Handle any pending keypresses
	while (p_key) {
		CHECK_QUIT;

		in1 = g_vm->getChar();
		getMousePosition(cx, cy, cd);
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
			cx = 315 * g_vm->_res;
			cy = 1;
			break;
		case '3':
			cy = 190;
			cx = 315 * g_vm->_res;
			break;
		case '5':
			cy = 100;
			cx = 155 * g_vm->_res;
			break;
		case ' ':
		case '\15':
			g_vm->setMouseClick(true);
			return;
			break;
		case '\33':
			p_key = g_vm->keyPressed();

			if (p_key) {
				in2 = g_vm->getChar();

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
			cx = g_vm->_res * 32;
			cy = 8;
			break;
		case 'D':
			cx = 80 * g_vm->_res;
			cy = 8;
			break;
		case 'A':
			cx = 126 * g_vm->_res;
			cy = 8;
			break;
		case 'S':
			cx = 174 * g_vm->_res;
			cy = 8;
			break;
		case 'P':
			cx = 222 * g_vm->_res;
			cy = 8;
			break;
		case 'F':
			cx = g_vm->_res * 270;
			cy = 8;
			break;
		case '\23':
			g_vm->_soundOff = !g_vm->_soundOff;
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
		p_key = g_vm->keyPressed();
	}
}

/**
 * Mouse function : Is mouse in a given rect?
 * @remarks	Originally called 'dans_rect'
 */
bool MouseHandler::isMouseIn(Common::Rect r) {
	int x, y, c;

	getMousePosition(x, y, c);
	if ((x > r.left) && (x < r.right) && (y > r.top) && (y < r.bottom))
		return true;

	return false;
}

} // End of namespace Mortevielle
