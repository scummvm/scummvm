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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _TOUCHKEYBOARD_H_
#define _TOUCHKEYBOARD_H_

#include "osystem_ds.h"

namespace DS {

static const int NUM_WORDS = 12;
static const int KEYBOARD_DATA_SIZE = 4736 * 2;
static const int KEYBOARD_BOTTOM_Y = 105;

void createKeyEvent(int keyNum, Common::Event& event);

void drawKeyboard(int tileBase, int mapBase, u16* saveSpace);
void restoreVRAM(int tileBase, int mapBase, u16* saveSpace);
void addKeyboardEvents();
bool getKeyboardClosed();
bool isInsideKeyboard(int x, int y);

void addAutoComplete(const char* word);
void clearAutoComplete();
void setCharactersEntered(int count);
void releaseAllKeys();

}

#endif
