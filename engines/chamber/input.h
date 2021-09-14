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

#ifndef CHAMBER_INPUT_H
#define CHAMBER_INPUT_H

namespace Chamber {

extern unsigned char buttons;
extern unsigned char right_button;

extern unsigned char have_mouse;

extern volatile unsigned char key_direction;
extern volatile unsigned char key_code;
extern unsigned char key_held;

unsigned char ReadKeyboardChar(void);
void ClearKeyboard(void);

unsigned char PollMouse(void);
unsigned char PollKeyboard(void);
void SetInputButtons(unsigned char keys);

void PollInput(void);
void ProcessInput(void);

void InitInput(void);
void UninitInput(void);

} // End of namespace Chamber

#endif
