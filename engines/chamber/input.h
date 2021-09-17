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

extern byte buttons;
extern byte right_button;

extern byte have_mouse;

extern volatile byte key_direction;
extern volatile byte key_code;
extern byte key_held;

void ClearKeyboard(void);

byte PollMouse(void);
byte PollKeyboard(void);
void SetInputButtons(byte keys);

void PollInput(void);
void ProcessInput(void);

void InitInput(void);
void UninitInput(void);

} // End of namespace Chamber

#endif
