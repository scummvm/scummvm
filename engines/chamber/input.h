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

#ifndef CHAMBER_INPUT_H
#define CHAMBER_INPUT_H

namespace Chamber {

extern byte buttons;
extern byte right_button;

extern byte have_mouse;

extern volatile byte key_direction;
extern volatile byte key_code;
extern byte key_held;

byte readKeyboardChar(void);
void clearKeyboard(void);
byte getKeyScan(void);

byte pollMouse(uint16 *curs_x, uint8 *curs_y);
byte pollKeyboard(void);
void setInputButtons(byte keys);

void pollInput(void);
void processInput(void);
void pollInputButtonsOnly(void);
void resetInput(void);

void initInput(void);
void uninitInput(void);

} // End of namespace Chamber

#endif
