/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ICB_DINPUTH
#define ICB_DINPUTH

#include "common/keyboard.h"

namespace ICB {

extern bool8 keyboard_buf_scancodes[Common::KEYCODE_LAST];
extern bool8 repeats_scancodes[Common::KEYCODE_LAST];

void Init_direct_input();
void setKeyState(Common::KeyCode key, bool pressed);
bool8 Read_DI_keys(uint32 key);
bool8 Read_DI_once_keys(uint32 key);

bool8 DI_key_waiting();
uint32 Get_DI_key_press();
void Clear_DI_key_buffer();
void SetDefaultKeys();
const char *GetKeyName(uint32 key);

} // End of namespace ICB

#endif
