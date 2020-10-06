/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ICB_DINPUTH
#define ICB_DINPUTH

#include "engines/icb/common/px_rccommon.h"

#include "common/keyboard.h"

namespace ICB {

// Global joystick variables
extern int currentJoystick;  // Mode the joystick is in
extern int attachedJoystick; // Mode the joystick is

// Digital defaults (to emulate playstation pad)
#define JOY_UP -127
#define JOY_DOWN 127
#define JOY_LEFT -127
#define JOY_RIGHT 127

// Used to request analogue values
#define X_AXIS 0xFE
#define Y_AXIS 0xFD

// For mapping purposes
extern uint8 up_joy;
extern uint8 down_joy;
extern uint8 left_joy;
extern uint8 right_joy;

// Equivalent values to call read joystick with
extern uint8 sidestep_button;
extern uint8 run_button;
extern uint8 crouch_button;
extern uint8 interact_button;
extern uint8 arm_button;
extern uint8 fire_button;
extern uint8 inventory_button;
extern uint8 remora_button;
extern uint8 pause_button;
extern bool8 keyboard_buf_ascii[256];
extern bool8 repeats_ascii[256];
extern bool8 keyboard_buf_scancodes[512]; // SDL_NUM_SCANCODES
extern bool8 repeats_scancodes[512];      // SDL_NUM_SCANCODES

// Generic joystick types (used to regulate control mode)
#define NO_JOYSTICK 0
#define ANALOGUE_JOYSTICK 1
#define DIGITAL_JOYSTICK 2

void Init_direct_input();
void setKeyState(Common::KeyCode key, bool pressed);
void Poll_direct_input();
bool8 Read_DI_keys(uint32 key);
bool8 Read_DI_once_keys(uint32 key);

bool8 DI_key_waiting();
uint32 Get_DI_key_press();
void Clear_DI_key_buffer();
void SetDefaultKeys();
const char *GetKeyName(uint32 key);
void SetDefaultJoystick();

int32 Read_Joystick(uint8 part);
int32 Read_Joystick_once(uint8 part);
const char *GetJoystickName();
const char *GetButtonName(uint8 part);
const char *GetAxisName(uint8 part);
uint8 GetJoystickButtonPress();
uint8 GetJoystickAxisPress();
void UnsetJoystickConfig();
void UnsetKeys();

} // End of namespace ICB

#endif
