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
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "common/system.h"
#include "graphics/cursorman.h"

#include "eventman.h"
#include "dungeonman.h"
#include "movesens.h"
#include "objectman.h"
#include "inventory.h"
#include "menus.h"
#include "timeline.h"
#include "projexpl.h"
#include "text.h"
#include "group.h"


namespace DM {
byte g42_bitmapArrowPointer[576] = { // @ G0042_auc_Graphic562_Bitmap_ArrowPointer
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x7, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x7, 0xF, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x7, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xF, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x6, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x8, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x8, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x3, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xA, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xC, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xE, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xC, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x7, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
byte g43_bitmapHanPointer[576] = { // @ G0043_auc_Graphic562_Bitmap_HandPointer
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x3, 0x5, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xA, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xD, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xE, 0xA, 0x8, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x7, 0xF, 0x8, 0x0, 0x0, 0x0, 0x0, 0xC, 0x7, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0x6, 0x7, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0,
	0x3, 0xF, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0x3, 0xF, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0x1, 0xF, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x1, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0xA, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x5, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x1, 0x2, 0xA, 0x8, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x5, 0x4, 0x0, 0x0, 0x0, 0x0, 0xC, 0x8, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x2, 0x8, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0,
	0x9, 0x8, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x8, 0x8, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0,
	0x2, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x1, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x7, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xF, 0xF, 0x8, 0x0, 0x0, 0x0, 0x0, 0x1, 0xF, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0,
	0xC, 0xF, 0xF, 0xC, 0x0, 0x0, 0x0, 0x0, 0xE, 0xF, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0,
	0x7, 0xF, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0x7, 0xF, 0xF, 0xE, 0x0, 0x0, 0x0, 0x0, 0x3, 0xF, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x1, 0xF, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x7, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xF, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

Box g462_BoxObjectPiles[4] = { // @ G0462_as_Graphic561_Box_ObjectPiles
	/* { X1, X2, Y1, Y2 } */
	Box(24, 111, 148, 168),   /* Front left */
	Box(112, 199, 148, 168),   /* Front right */
	Box(112, 183, 122, 147),   /* Back right */
	Box(40, 111, 122, 147)}; /* Back left */


KeyboardInput g458_primaryKeyboardInput_interface[7] = { // @ G0458_as_Graphic561_PrimaryKeyboardInput_Interface
														 /* { Command, Code } */
	KeyboardInput(k7_CommandToggleInventoryChampion_0, Common::KEYCODE_F1, 0), /* F1 (<CSI>1~) Atari ST: Code = 0x3B00 */
	KeyboardInput(k8_CommandToggleInventoryChampion_1, Common::KEYCODE_F2, 0), /* F2 (<CSI>2~) Atari ST: Code = 0x3C00 */
	KeyboardInput(k9_CommandToggleInventoryChampion_2, Common::KEYCODE_F3, 0), /* F3 (<CSI>3~) Atari ST: Code = 0x3D00 */
	KeyboardInput(k10_CommandToggleInventoryChampion_3, Common::KEYCODE_F4, 0), /* F4 (<CSI>4~) Atari ST: Code = 0x3E00 */
	KeyboardInput(k140_CommandSaveGame, Common::KEYCODE_s, Common::KBD_CTRL), /* CTRL-S       Atari ST: Code = 0x0013 */
	KeyboardInput(k147_CommandFreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc (0x1B)   Atari ST: Code = 0x001B */
	KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)};


KeyboardInput g459_secondaryKeyboardInput_movement[19] = { // @ G0459_as_Graphic561_SecondaryKeyboardInput_Movement
														   /* { Command, Code } */
	KeyboardInput(k1_CommandTurnLeft, Common::KEYCODE_KP4, 0), /* Numeric pad 4 Atari ST: Code = 0x5200 */
	KeyboardInput(k3_CommandMoveForward, Common::KEYCODE_KP5, 0), /* Numeric pad 5 Atari ST: Code = 0x4800 */
	KeyboardInput(k2_CommandTurnRight, Common::KEYCODE_KP6, 0), /* Numeric pad 6 Atari ST: Code = 0x4700 */
	KeyboardInput(k6_CommandMoveLeft, Common::KEYCODE_KP1, 0), /* Numeric pad 1 Atari ST: Code = 0x4B00 */
	KeyboardInput(k5_CommandMoveBackward, Common::KEYCODE_KP2, 0), /* Numeric pad 2 Atari ST: Code = 0x5000 */
	KeyboardInput(k4_CommandMoveRight, Common::KEYCODE_KP3, 0), /* Numeric pad 3 Atari ST: Code = 0x4D00. Remaining entries below not present */
	KeyboardInput(k3_CommandMoveForward, Common::KEYCODE_w, 0), /* Up Arrow (<CSI>A) */ /*Differs for testing convenience*/
	KeyboardInput(k3_CommandMoveForward, Common::KEYCODE_w, Common::KBD_SHIFT), /* Shift Up Arrow (<CSI>T) */ /*Differs for testing convenience*/
	KeyboardInput(k6_CommandMoveLeft, Common::KEYCODE_a, 0), /* Backward Arrow (<CSI>D) */ /*Differs for testing convenience*/
	KeyboardInput(k6_CommandMoveLeft, Common::KEYCODE_a, Common::KBD_SHIFT), /* Shift Forward Arrow (<CSI> A) */ /*Differs for testing convenience*/
	KeyboardInput(k4_CommandMoveRight, Common::KEYCODE_d, 0), /* Forward Arrow (<CSI>C) */ /*Differs for testing convenience*/
	KeyboardInput(k4_CommandMoveRight, Common::KEYCODE_d, Common::KBD_SHIFT), /* Shift Backward Arrow (<CSI> @) */ /*Differs for testing convenience*/
	KeyboardInput(k5_CommandMoveBackward, Common::KEYCODE_s, 0), /* Down arrow (<CSI>B) */ /*Differs for testing convenience*/
	KeyboardInput(k5_CommandMoveBackward, Common::KEYCODE_s, Common::KBD_SHIFT), /* Shift Down arrow (<CSI>S) */ /*Differs for testing convenience*/
	KeyboardInput(k1_CommandTurnLeft, Common::KEYCODE_q, 0), /* Del (0x7F) */ /*Differs for testing convenience*/
	KeyboardInput(k1_CommandTurnLeft, Common::KEYCODE_q, Common::KBD_SHIFT), /* Shift Del (0x7F) */ /*Differs for testing convenience*/
	KeyboardInput(k2_CommandTurnRight, Common::KEYCODE_e, 0), /* Help (<CSI>?~) */ /*Differs for testing convenience*/
	KeyboardInput(k2_CommandTurnRight, Common::KEYCODE_e, Common::KBD_SHIFT), /* Shift Help (<CSI>?~) */ /*Differs for testing convenience*/
	KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)};
KeyboardInput g460_primaryKeyboardInput_partySleeping[3] = { // @ G0460_as_Graphic561_PrimaryKeyboardInput_PartySleeping
															 /* { Command, Code } */
	KeyboardInput(k146_CommandWakeUp, Common::KEYCODE_RETURN, 0), /* Return */
	KeyboardInput(k147_CommandFreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc */
	KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)};
KeyboardInput g461_primaryKeyboardInput_frozenGame[2] = { // @ G0461_as_Graphic561_PrimaryKeyboardInput_FrozenGame
														  /* { Command, Code } */
	KeyboardInput(k148_CommandUnfreezeGame, Common::KEYCODE_ESCAPE, 0), /* Esc */
	KeyboardInput(k0_CommandNone, Common::KEYCODE_INVALID, 0)};


MouseInput g445_PrimaryMouseInput_Entrance[4] = { // @ G0445_as_Graphic561_PrimaryMouseInput_Entrance[4]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k200_CommandEntranceEnterDungeon, 244, 298,  45,  58, k1_LeftMouseButton),
	// Strangerke - C201_COMMAND_ENTRANCE_RESUME isn't present in the demo
	MouseInput(k201_CommandEntranceResume,        244, 298,  76,  93, k1_LeftMouseButton),
	MouseInput(k202_CommandEntranceDrawCredits,  248, 293, 187, 199, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g446_PrimaryMouseInput_RestartGame[2] = { // @ G0446_as_Graphic561_PrimaryMouseInput_RestartGame[2]
													 /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k215_CommandRestartGame, 103, 217, 145, 159, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g447_PrimaryMouseInput_Interface[20] = { // @ G0447_as_Graphic561_PrimaryMouseInput_Interface[20]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k12_CommandClickInChampion_0_StatusBox,       0,  42,   0,  28, k1_LeftMouseButton),
	MouseInput(k13_CommandClickInChampion_1_StatusBox,      69, 111,   0,  28, k1_LeftMouseButton),
	MouseInput(k14_CommandClickInChampion_2_StatusBox,     138, 180,   0,  28, k1_LeftMouseButton),
	MouseInput(k15_CommandClickInChampion_3_StatusBox,     207, 249,   0,  28, k1_LeftMouseButton),
	MouseInput(k125_CommandClickOnChamptionIcon_Top_Left,    274, 299,   0,  13, k1_LeftMouseButton),
	MouseInput(k126_CommandClickOnChamptionIcon_Top_Right,   301, 319,   0,  13, k1_LeftMouseButton),
	MouseInput(k127_CommandClickOnChamptionIcon_Lower_Right, 301, 319,  15,  28, k1_LeftMouseButton),
	MouseInput(k128_CommandClickOnChamptionIcon_Lower_Left,  274, 299,  15,  28, k1_LeftMouseButton),
	MouseInput(k7_CommandToggleInventoryChampion_0,         43,  66,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 =  44. swapped with 4 next entries */
	MouseInput(k8_CommandToggleInventoryChampion_1,        112, 135,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 113. swapped with 4 next entries */
	MouseInput(k9_CommandToggleInventoryChampion_2,        181, 204,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 182. swapped with 4 next entries */
	MouseInput(k10_CommandToggleInventoryChampion_3,        250, 273,   0,  28, k1_LeftMouseButton), /* Atari ST: Only present in CSB 2.x and with Box.X1 = 251. swapped with 4 next entries */
	MouseInput(k7_CommandToggleInventoryChampion_0,          0,  66,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(k8_CommandToggleInventoryChampion_1,         69, 135,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(k9_CommandToggleInventoryChampion_2,        138, 204,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(k10_CommandToggleInventoryChampion_3,        207, 273,   0,  28, k2_RightMouseButton), /* Atari ST: swapped with 4 previous entries */
	MouseInput(k100_CommandClickInSpellArea,                233, 319,  42,  73, k1_LeftMouseButton),
	MouseInput(k111_CommandClickInActionArea,               233, 319,  77, 121, k1_LeftMouseButton),
	MouseInput(k147_CommandFreezeGame,                          0,   1, 198, 199, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g448_SecondaryMouseInput_Movement[9] = { // @ G0448_as_Graphic561_SecondaryMouseInput_Movement[9]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k1_CommandTurnLeft,             234, 261, 125, 145, k1_LeftMouseButton),
	MouseInput(k3_CommandMoveForward,          263, 289, 125, 145, k1_LeftMouseButton),
	MouseInput(k2_CommandTurnRight,            291, 318, 125, 145, k1_LeftMouseButton),
	MouseInput(k6_CommandMoveLeft,             234, 261, 147, 167, k1_LeftMouseButton),
	MouseInput(k5_CommandMoveBackward,         263, 289, 147, 167, k1_LeftMouseButton),
	MouseInput(k4_CommandMoveRight,            291, 318, 147, 167, k1_LeftMouseButton),
	MouseInput(k80_CommandClickInDungeonView,   0, 223,  33, 168, k1_LeftMouseButton),
	MouseInput(k83_CommandToggleInventoryLeader, 0, 319,  33, 199, k2_RightMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g449_SecondaryMouseInput_ChampionInventory[38] = { // @ G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[38]
															  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k11_CommandCloseInventory,                                   0, 319,   0, 199, k2_RightMouseButton),
	MouseInput(k140_CommandSaveGame,                                       174, 182,  36,  44, k1_LeftMouseButton),
	MouseInput(k145_CommandSleep,                                           188, 204,  36,  44, k1_LeftMouseButton),
	MouseInput(k11_CommandCloseInventory,                                 210, 218,  36,  44, k1_LeftMouseButton),
	MouseInput(k28_CommandClickOnSlotBoxInventoryReadyHand ,        6,  21,  86, 101, k1_LeftMouseButton),
	MouseInput(k29_CommandClickOnSlotBoxInventoryActionHand,       62,  77,  86, 101, k1_LeftMouseButton),
	MouseInput(k30_CommandClickOnSlotBoxInventoryHead,              34,  49,  59,  74, k1_LeftMouseButton),
	MouseInput(k31_CommandClickOnSlotBoxInventoryTorso,             34,  49,  79,  94, k1_LeftMouseButton),
	MouseInput(k32_CommandClickOnSlotBoxInventoryLegs,              34,  49,  99, 114, k1_LeftMouseButton),
	MouseInput(k33_CommandClickOnSlotBoxInventoryFeet,              34,  49, 119, 134, k1_LeftMouseButton),
	MouseInput(k34_CommandClickOnSlotBoxInventoryPouch_2,            6,  21, 123, 138, k1_LeftMouseButton),
	MouseInput(k70_CommandClickOnMouth,                                   56,  71,  46,  61, k1_LeftMouseButton),
	MouseInput(k71_CommandClickOnEye,                                     12,  27,  46,  61, k1_LeftMouseButton),
	MouseInput(k35_CommandClickOnSlotBoxInventoryQuiverLine_2_1,    79,  94, 106, 121, k1_LeftMouseButton),
	MouseInput(k36_CommandClickOnSlotBoxInventoryQuiverLine_1_2,    62,  77, 123, 138, k1_LeftMouseButton),
	MouseInput(k37_CommandClickOnSlotBoxInventoryQuiverLine_2_2,    79,  94, 123, 138, k1_LeftMouseButton),
	MouseInput(k38_CommandClickOnSlotBoxInventoryNeck,               6,  21,  66,  81, k1_LeftMouseButton),
	MouseInput(k39_CommandClickOnSlotBoxInventoryPouch_1,            6,  21, 106, 121, k1_LeftMouseButton),
	MouseInput(k40_CommandClickOnSlotBoxInventoryQuiverLine_1_1,    62,  77, 106, 121, k1_LeftMouseButton),
	MouseInput(k41_CommandClickOnSlotBoxInventoryBackpackLine_1_1,  66,  81,  66,  81, k1_LeftMouseButton),
	MouseInput(k42_CommandClickOnSlotBoxInventoryBackpackLine_2_2,  83,  98,  49,  64, k1_LeftMouseButton),
	MouseInput(k43_CommandClickOnSlotBoxInventoryBackpackLine_2_3, 100, 115,  49,  64, k1_LeftMouseButton),
	MouseInput(k44_CommandClickOnSlotBoxInventoryBackpackLine_2_4, 117, 132,  49,  64, k1_LeftMouseButton),
	MouseInput(k45_CommandClickOnSlotBoxInventoryBackpackLine_2_5, 134, 149,  49,  64, k1_LeftMouseButton),
	MouseInput(k46_CommandClickOnSlotBoxInventoryBackpackLine_2_6, 151, 166,  49,  64, k1_LeftMouseButton),
	MouseInput(k47_CommandClickOnSlotBoxInventoryBackpackLine_2_7, 168, 183,  49,  64, k1_LeftMouseButton),
	MouseInput(k48_CommandClickOnSlotBoxInventoryBackpackLine_2_8, 185, 200,  49,  64, k1_LeftMouseButton),
	MouseInput(k49_CommandClickOnSlotBoxInventoryBackpackLine_2_9, 202, 217,  49,  64, k1_LeftMouseButton),
	MouseInput(k50_CommandClickOnSlotBoxInventoryBackpackLine_1_2,  83,  98,  66,  81, k1_LeftMouseButton),
	MouseInput(k51_CommandClickOnSlotBoxInventoryBackpackLine_1_3, 100, 115,  66,  81, k1_LeftMouseButton),
	MouseInput(k52_CommandClickOnSlotBoxInventoryBackpackLine_1_4, 117, 132,  66,  81, k1_LeftMouseButton),
	MouseInput(k53_CommandClickOnSlotBoxInventoryBackpackLine_1_5, 134, 149,  66,  81, k1_LeftMouseButton),
	MouseInput(k54_CommandClickOnSlotBoxInventoryBackpackLine_1_6, 151, 166,  66,  81, k1_LeftMouseButton),
	MouseInput(k55_CommandClickOnSlotBoxInventoryBackpackLine_1_7, 168, 183,  66,  81, k1_LeftMouseButton),
	MouseInput(k56_CommandClickOnSlotBoxInventoryBackpackLine_1_8, 185, 200,  66,  81, k1_LeftMouseButton),
	MouseInput(k57_CommandClickOnSlotBoxInventoryBackpackLine_1_9, 202, 217,  66,  81, k1_LeftMouseButton),
	MouseInput(k81_CommandClickInPanel,                                   96, 223,  83, 167, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g450_PrimaryMouseInput_PartySleeping[3] = { // @ G0450_as_Graphic561_PrimaryMouseInput_PartySleeping[3]
													   /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k146_CommandWakeUp, 0, 223, 33, 168, k1_LeftMouseButton),
	MouseInput(k146_CommandWakeUp, 0, 223, 33, 168, k2_RightMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g451_PrimaryMouseInput_FrozenGame[3] = { // @ G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3]
													/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k148_CommandUnfreezeGame, 0, 319, 0, 199, k1_LeftMouseButton),
	MouseInput(k148_CommandUnfreezeGame, 0, 319, 0, 199, k2_RightMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g452_MouseInput_ActionAreaNames[5] = { // @ G0452_as_Graphic561_MouseInput_ActionAreaNames[5]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k112_CommandClickInActionAreaPass,     285, 318,  77,  83, k1_LeftMouseButton),
	MouseInput(k113_CommandClickInActionAreaAction_0, 234, 318,  86,  96, k1_LeftMouseButton),
	MouseInput(k114_CommandClickInActionAreaAction_1, 234, 318,  98, 108, k1_LeftMouseButton),
	MouseInput(k115_CommandClickInActionAreaAction_2, 234, 318, 110, 120, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g453_MouseInput_ActionAreaIcons[5] = { // @ G0453_as_Graphic561_MouseInput_ActionAreaIcons[5]
												  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k116_CommandClickInActionAreaChampion_0_Action, 233, 252, 86, 120, k1_LeftMouseButton),
	MouseInput(k117_CommandClickInActionAreaChampion_1_Action, 255, 274, 86, 120, k1_LeftMouseButton),
	MouseInput(k118_CommandClickInActionAreaChampion_2_Action, 277, 296, 86, 120, k1_LeftMouseButton),
	MouseInput(k119_CommandClickInActionAreaChampion_3_Action, 299, 318, 86, 120, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g454_MouseInput_SpellArea[9] = { // @ G0454_as_Graphic561_MouseInput_SpellArea[9]
											/* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k101_CommandClickInSpellAreaSymbol_1,      235, 247, 51, 61, k1_LeftMouseButton),
	MouseInput(k102_CommandClickInSpellAreaSymbol_2,      249, 261, 51, 61, k1_LeftMouseButton),
	MouseInput(k103_CommandClickInSpellAreaSymbol_3,      263, 275, 51, 61, k1_LeftMouseButton),
	MouseInput(k104_CommandClickInSpellAreaSymbol_4,      277, 289, 51, 61, k1_LeftMouseButton),
	MouseInput(k105_CommandClickInSpellAreaSymbol_5,      291, 303, 51, 61, k1_LeftMouseButton),
	MouseInput(k106_CommandClickInSpellAreaSymbol_6,      305, 317, 51, 61, k1_LeftMouseButton),
	MouseInput(k108_CommandClickInSpeallAreaCastSpell,    234, 303, 63, 73, k1_LeftMouseButton),
	MouseInput(k107_CommandClickInSpellAreaRecantSymbol, 305, 318, 63, 73, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g455_MouseInput_ChampionNamesHands[13] = { // @ G0455_as_Graphic561_MouseInput_ChampionNamesHands[13]
													  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k16_CommandSetLeaderChampion_0,                                    0,  42,  0,  6, k1_LeftMouseButton),
	MouseInput(k17_CommandSetLeaderChampion_1,                                   69, 111,  0,  6, k1_LeftMouseButton),
	MouseInput(k18_CommandSetLeaderChampion_2,                                  138, 180,  0,  6, k1_LeftMouseButton),
	MouseInput(k19_CommandSetLeaderChampion_3,                                  207, 249,  0,  6, k1_LeftMouseButton),
	MouseInput(k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand,    4,  19, 10, 25, k1_LeftMouseButton),
	MouseInput(k21_CommandClickOnSlotBoxChampion_0_StatusBoxActionHand,  24,  39, 10, 25, k1_LeftMouseButton),
	MouseInput(k22_CommandClickOnSlotBoxChampion_1_StatusBoxReadyHand,   73,  88, 10, 25, k1_LeftMouseButton),
	MouseInput(k23_CommandClickOnSlotBoxChampion_1_StatusBoxActionHand,  93, 108, 10, 25, k1_LeftMouseButton),
	MouseInput(k24_CommandClickOnSlotBoxChampion_2_StatusBoxReadyHand,  142, 157, 10, 25, k1_LeftMouseButton),
	MouseInput(k25_CommandClickOnSlotBoxChampion_2_StatusBoxActionHand, 162, 177, 10, 25, k1_LeftMouseButton),
	MouseInput(k26_CommandClickOnSlotBoxChampion_3_StatusBoxReadyHand,  211, 226, 10, 25, k1_LeftMouseButton),
	MouseInput(k27_CommandClickOnSlotBoxChampion_3_StatusBoxActionHand, 231, 246, 10, 25, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g456_MouseInput_PanelChest[9] = { // @ G0456_as_Graphic561_MouseInput_PanelChest[9]
											 /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k58_CommandClickOnSlotBoxChest_1, 117, 132,  92, 107, k1_LeftMouseButton),
	MouseInput(k59_CommandClickOnSlotBoxChest_2, 106, 121, 109, 124, k1_LeftMouseButton),
	MouseInput(k60_CommandClickOnSlotBoxChest_3, 111, 126, 126, 141, k1_LeftMouseButton),
	MouseInput(k61_CommandClickOnSlotBoxChest_4, 128, 143, 131, 146, k1_LeftMouseButton),
	MouseInput(k62_CommandClickOnSlotBoxChest_5, 145, 160, 134, 149, k1_LeftMouseButton),
	MouseInput(k63_CommandClickOnSlotBoxChest_6, 162, 177, 136, 151, k1_LeftMouseButton),
	MouseInput(k64_CommandClickOnSlotBoxChest_7, 179, 194, 137, 152, k1_LeftMouseButton),
	MouseInput(k65_CommandClickOnSlotBoxChest_8, 196, 211, 138, 153, k1_LeftMouseButton),
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g457_MouseInput_PanelResurrectReincarnateCancel[4] = { // @ G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4]
																  /* { Command, Box.X1, Box.X2, Box.Y1, Box.Y2, Button } */
	MouseInput(k160_CommandClickInPanelResurrect,   108, 158,  90, 138, k1_LeftMouseButton), /* Atari ST: Box = 104, 158,  86, 142 */
	MouseInput(k161_CommandClickInPanelReincarnate, 161, 211,  90, 138, k1_LeftMouseButton), /* Atari ST: Box = 163, 217,  86, 142 */
	MouseInput(k162_CommandClickInPanelCancel,      108, 211, 141, 153, k1_LeftMouseButton), /* Atari ST: Box = 104, 217, 146, 156 */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};


MouseInput g471_PrimaryMouseInput_ViewportDialog1Choice[2] = { // @ G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2]
	MouseInput(k210_CommandClickOnDialogChoice_1, 16, 207, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g472_PrimaryMouseInput_ViewportDialog2Choices[3] = { // @ G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3]
	MouseInput(k210_CommandClickOnDialogChoice_1, 16, 207, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 16, 207, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g473_PrimaryMouseInput_ViewportDialog3Choices[4] = { // @ G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4]
	MouseInput(k210_CommandClickOnDialogChoice_1,  16, 207, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2,  16, 101, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k212_CommandClickOnDialogChoice_3, 123, 207, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g474_PrimaryMouseInput_ViewportDialog4Choices[5] = { // @ G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5]
	MouseInput(k210_CommandClickOnDialogChoice_1,  16, 101, 101, 115, k1_LeftMouseButton), /* Top left button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 123, 207, 101, 115, k1_LeftMouseButton), /* Top right button */
	MouseInput(k212_CommandClickOnDialogChoice_3,  16, 101, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k213_CommandClickOnDialogChoice_4, 123, 207, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g475_PrimaryMouseInput_ScreenDialog1Choice[2] = { // @ G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2]
	MouseInput(k210_CommandClickOnDialogChoice_1, 63, 254, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g476_PrimaryMouseInput_ScreenDialog2Choices[3] = { // @ G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3]
	MouseInput(k210_CommandClickOnDialogChoice_1, 63, 254, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 63, 254, 138, 152, k1_LeftMouseButton), /* Bottom button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g477_PrimaryMouseInput_ScreenDialog3Choices[4] = { // @ G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4]
	MouseInput(k210_CommandClickOnDialogChoice_1,  63, 254, 101, 115, k1_LeftMouseButton), /* Top button */
	MouseInput(k211_CommandClickOnDialogChoice_2,  63, 148, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k212_CommandClickOnDialogChoice_3, 170, 254, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};
MouseInput g478_PrimaryMouseInput_ScreenDialog4Choices[5] = { // @ G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5]
	MouseInput(k210_CommandClickOnDialogChoice_1,  63, 148, 101, 115, k1_LeftMouseButton), /* Top left button */
	MouseInput(k211_CommandClickOnDialogChoice_2, 170, 254, 101, 115, k1_LeftMouseButton), /* Top right button */
	MouseInput(k212_CommandClickOnDialogChoice_3,  63, 148, 138, 152, k1_LeftMouseButton), /* Lower left button */
	MouseInput(k213_CommandClickOnDialogChoice_4, 170, 254, 138, 152, k1_LeftMouseButton), /* Lower right button */
	MouseInput(k0_CommandNone, 0, 0, 0, 0, k0_NoneMouseButton)};

MouseInput* g480_PrimaryMouseInput_DialogSets[2][4] = { // @ G0480_aaps_PrimaryMouseInput_DialogSets
	{g471_PrimaryMouseInput_ViewportDialog1Choice,
	g472_PrimaryMouseInput_ViewportDialog2Choices,
	g473_PrimaryMouseInput_ViewportDialog3Choices,
	g474_PrimaryMouseInput_ViewportDialog4Choices},
	{g475_PrimaryMouseInput_ScreenDialog1Choice,
	g476_PrimaryMouseInput_ScreenDialog2Choices,
	g477_PrimaryMouseInput_ScreenDialog3Choices,
	g478_PrimaryMouseInput_ScreenDialog4Choices},};


EventManager::EventManager(DMEngine *vm) : _vm(vm) {
	_mousePos = Common::Point(0, 0);
	_dummyMapIndex = 0;
	_g436_pendingClickPresent = false;
	_g437_pendingClickPos = Common::Point(0, 0);
	_g615_mousePointerOriginalColorsObject = nullptr;
	_g613_mousePointerOriginalColorsChampionIcon = nullptr;
	_gK190_mousePointerTempBuffer = nullptr;
	_g435_isCommandQueueLocked = true;
	_gK104_mousePointerType = 0;
	_gK105_previousMousePointerType = 0;
	_g441_primaryMouseInput = nullptr;
	_g442_secondaryMouseInput = nullptr;
	_g598_mousePointerBitmapUpdated = true;
	_g326_refreshMousePointerInMainLoop = false;
	_g341_highlightBoxEnabled = false;
	_g599_useChampionIconOrdinalAsMousePointerBitmap = 0;
	_g439_pendingClickButton = k0_NoneMouseButton;
	_g600_useObjectAsMousePointerBitmap = false;
	_g601_useHandAsMousePointerBitmap = false;
	_gK100_preventBuildPointerScreenArea = false;
	_g443_primaryKeyboardInput = nullptr;
	_g444_secondaryKeyboardInput = nullptr;
}

EventManager::~EventManager() {
	delete[] _g615_mousePointerOriginalColorsObject;
	delete[] _gK190_mousePointerTempBuffer;
	delete[] _g613_mousePointerOriginalColorsChampionIcon;
}

void EventManager::initMouse() {
	if (!_g615_mousePointerOriginalColorsObject)
		_g615_mousePointerOriginalColorsObject = new byte[32 * 18];
	if (!_gK190_mousePointerTempBuffer)
		_gK190_mousePointerTempBuffer = new byte[32 * 18];
	if (!_g613_mousePointerOriginalColorsChampionIcon)
		_g613_mousePointerOriginalColorsChampionIcon = new byte[32 * 18];

	_gK104_mousePointerType = k0_pointerArrow;
	_gK105_previousMousePointerType = k1_pointerHand;

	_mousePos = Common::Point(0, 0);
	f73_buildpointerScreenArea(_mousePos.x, _mousePos.y);
	CursorMan.showMouse(false);

	setMousePos(Common::Point(320 / 2, 200 / 2));
}

void EventManager::f67_setMousePointerToNormal(int16 mousePointer) {
	_gK100_preventBuildPointerScreenArea = true;
	_g600_useObjectAsMousePointerBitmap = false;
	_g601_useHandAsMousePointerBitmap = (mousePointer == k1_pointerHand);
	_g598_mousePointerBitmapUpdated = true;
	_gK100_preventBuildPointerScreenArea = false;
	f73_buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::f68_setPointerToObject(byte* bitmap) {
	static byte gK27_palChangesMousepointerOjbectIconShadow[16] = {120, 120, 120, 120, 120, 120, 120, 120,
		120, 120, 120, 120, 0, 120, 120, 120}; // @ K0027_auc_PaletteChanges_MousePointerObjectIconShadow
	static byte g44_palChangesMousePointerIcon[16] = {120, 10, 20, 30, 40, 50, 60, 70, 80, 90,
		100, 110, 0, 130, 140, 150}; // @ G0044_auc_Graphic562_PaletteChanges_MousePointerIcon
	static Box g619_BoxMousePointerObjectShadow(2, 17, 2, 17); // @ G0619_s_Box_MousePointer_ObjectShadow 
	static Box g620_BoxMousePointerObject(0, 15, 0, 15); // @ G0620_s_Box_MousePointer_Object 

	_gK100_preventBuildPointerScreenArea = true;
	_g600_useObjectAsMousePointerBitmap = true;
	_g601_useHandAsMousePointerBitmap = false;
	_g598_mousePointerBitmapUpdated = true;
	_vm->_displayMan->_g578_useByteBoxCoordinates = true;
	byte *L0051_puc_Bitmap = _g615_mousePointerOriginalColorsObject;
	memset(L0051_puc_Bitmap, 0, 32 * 18);

	/*
	_vm->_displayMan->f129_blitToBitmapShrinkWithPalChange(bitmap, _gK190_mousePointerTempBuffer, 16, 16, 16, 16, gK27_palChangesMousepointerOjbectIconShadow);
	_vm->_displayMan->f132_blitToBitmap(_gK190_mousePointerTempBuffer, L0051_puc_Bitmap, g619_BoxMousePointerObjectShadow, 0, 0, 8, 16, kM1_ColorNoTransparency, 16, 18);
	_vm->_displayMan->f129_blitToBitmapShrinkWithPalChange(bitmap, _gK190_mousePointerTempBuffer, 16, 16, 16, 16, g44_palChangesMousePointerIcon);
	_vm->_displayMan->f132_blitToBitmap(_gK190_mousePointerTempBuffer, L0051_puc_Bitmap, g620_BoxMousePointerObject, 0, 0, 8, 16, k0_ColorBlack, 16, 18);
	*/
	warning(false, "TODO - Call f129_blitToBitmapShrinkWithPalChange");
	// dummy code
	_vm->_displayMan->f132_blitToBitmap(bitmap, L0051_puc_Bitmap, g619_BoxMousePointerObjectShadow, 0, 0, 8, 16, kM1_ColorNoTransparency, 16, 18);
	_vm->_displayMan->f132_blitToBitmap(bitmap, L0051_puc_Bitmap, g620_BoxMousePointerObject, 0, 0, 8, 16, k0_ColorBlack, 16, 18);

	_gK100_preventBuildPointerScreenArea = false;
	f73_buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::f71_mouseDropChampionIcon() {
	_gK100_preventBuildPointerScreenArea = true;
	uint16 L0058_ui_ChampionIconIndex = _vm->M1_ordinalToIndex(_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap);
	_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap = _vm->M0_indexToOrdinal(kM1_ChampionNone);
	_vm->_eventMan->_g598_mousePointerBitmapUpdated = true;
	bool L0057_B_UseByteBoxCoordinatesBackup = _vm->_displayMan->_g578_useByteBoxCoordinates;
	_vm->_displayMan->f21_blitToScreen(_g613_mousePointerOriginalColorsChampionIcon, &g54_BoxChampionIcons[L0058_ui_ChampionIconIndex << 2], 16, k12_ColorDarkestGray, 18);
	_vm->_displayMan->_g578_useByteBoxCoordinates = L0057_B_UseByteBoxCoordinatesBackup;
	_gK100_preventBuildPointerScreenArea = false;
}

void EventManager::f73_buildpointerScreenArea(int16 mousePosX, int16 mousePosY) {
	uint16 L1577_i_ChampionIndex;
	int16 L1578_i_XOverChampionStatusBox;

	// if (_gK100_preventBuildPointerScreenArea)
	//	return;

	_gK100_preventBuildPointerScreenArea = true;
	if (_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap) {
		if ((mousePosY > 28) || (mousePosX < 274)) {
			_gK104_mousePointerType = k4_pointerTypeAutoselect;
			f71_mouseDropChampionIcon();
		} else {
			_gK104_mousePointerType = k2_pointerTypeChampionIcon;
		}
	} else {
		if (mousePosY >= 169) {
			_gK104_mousePointerType = k0_pointerTypeArrow;
		} else {
			if (mousePosX >= 274) {
				_gK104_mousePointerType = k0_pointerTypeArrow;
			} else {
				if (mousePosY <= 28) {
					L1577_i_ChampionIndex = mousePosX / 69;
					L1578_i_XOverChampionStatusBox = mousePosX % 69;
					if (L1577_i_ChampionIndex >= _vm->_championMan->_g305_partyChampionCount) {
						_gK104_mousePointerType = k4_pointerTypeAutoselect;
					} else {
						if (L1578_i_XOverChampionStatusBox > 42) {
							_gK104_mousePointerType = k4_pointerTypeAutoselect;
						} else {
							L1577_i_ChampionIndex++;
							if (L1577_i_ChampionIndex == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
								_gK104_mousePointerType = k0_pointerTypeArrow;
							} else {
								if (mousePosY <= 6) {
									_gK104_mousePointerType = k0_pointerTypeArrow;
								} else {
									_gK104_mousePointerType = k4_pointerTypeAutoselect;
								}
							}
						}
					}
				} else {
					if (mousePosX >= 224) {
						_gK104_mousePointerType = k0_pointerTypeArrow;
					} else {
						_gK104_mousePointerType = k4_pointerTypeAutoselect;
					}
				}
			}
		}
	}
	if (_gK104_mousePointerType == k4_pointerTypeAutoselect) {
		_gK104_mousePointerType = (_g600_useObjectAsMousePointerBitmap) ? k1_pointerTypeObjectIcon : (_g601_useHandAsMousePointerBitmap) ? k3_pointerTypeHand : k0_pointerTypeArrow;
	}
	if (_vm->_eventMan->_g598_mousePointerBitmapUpdated || (_gK104_mousePointerType != _gK105_previousMousePointerType)) {
		_vm->_eventMan->_g598_mousePointerBitmapUpdated = false;
		switch (_gK104_mousePointerType) {
		case k0_pointerTypeArrow:
			CursorMan.replaceCursor(g42_bitmapArrowPointer, 32, 18, 0, 0, 0);
			break;
		case k1_pointerTypeObjectIcon:
			CursorMan.replaceCursor(_g615_mousePointerOriginalColorsObject, 32, 18, 0, 0, 0);
			break;
		case k2_pointerTypeChampionIcon:
			CursorMan.replaceCursor(_g613_mousePointerOriginalColorsChampionIcon, 32, 18, 0, 0, 0);
			break;
		case k3_pointerTypeHand:
			CursorMan.replaceCursor(g43_bitmapHanPointer, 32, 18, 0, 0, 0);
			break;
		}
	}
	_gK105_previousMousePointerType = _gK104_mousePointerType;
	_gK100_preventBuildPointerScreenArea = false;
}

void EventManager::f69_setMousePointer() {
	if (_vm->_championMan->_g415_leaderEmptyHanded) {
		f67_setMousePointerToNormal((_vm->_championMan->_g411_leaderIndex == kM1_ChampionNone) ? k0_pointerArrow : k1_pointerHand);
	} else {
		f68_setPointerToObject(_vm->_objectMan->_g412_objectIconForMousePointer);
	}
}

void EventManager::f78_showMouse() {
	CursorMan.showMouse(true);
}

void EventManager::f77_hideMouse() {
	// CursorMan.showMouse(false);
}


void EventManager::setMousePos(Common::Point pos) {
	_vm->_system->warpMouse(pos.x, pos.y);
}


void EventManager::processInput() {
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN: {
			if (event.synthetic)
				break;
			if (_g443_primaryKeyboardInput) {
				KeyboardInput *input = _g443_primaryKeyboardInput;
				while (input->_commandToIssue != k0_CommandNone) {
					if ((input->_key == event.kbd.keycode) && (input->_modifiers == (event.kbd.flags & input->_modifiers))) {
						f360_processPendingClick(); // possible fix to BUG0_73
						_commandQueue.push(Command(Common::Point(-1, -1), input->_commandToIssue));
						break;
					}
					input++;
				}
			}

			if (_g444_secondaryKeyboardInput) {
				KeyboardInput *input = _g444_secondaryKeyboardInput;
				while (input->_commandToIssue != k0_CommandNone) {
					if ((input->_key == event.kbd.keycode) && (input->_modifiers == (event.kbd.flags & input->_modifiers))) {
						f360_processPendingClick(); // possible fix to BUG0_73
						_commandQueue.push(Command(Common::Point(-1, -1), input->_commandToIssue));
						break;
					}
					input++;
				}
			}

		}
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_g436_pendingClickPresent = true;
			_g437_pendingClickPos = _mousePos;
			_g439_pendingClickButton = (event.type == Common::EVENT_LBUTTONDOWN) ? k1_LeftMouseButton : k2_RightMouseButton;
			break;
		default:
			break;
		}
	}
}

void EventManager::f360_processPendingClick() {
	if (_g436_pendingClickPresent) {
		_g436_pendingClickPresent = false;
		f359_processClick(_g437_pendingClickPos, _g439_pendingClickButton);
	}
}

void EventManager::f359_processClick(Common::Point mousePos, MouseButton button) {
	CommandType commandType;

	commandType = f358_getCommandTypeFromMouseInput(_g441_primaryMouseInput, mousePos, button);
	if (commandType == k0_CommandNone)
		commandType = f358_getCommandTypeFromMouseInput(_g442_secondaryMouseInput, mousePos, button);

	if (commandType != k0_CommandNone)
		_commandQueue.push(Command(mousePos, commandType));

	_g435_isCommandQueueLocked = false;
}

CommandType EventManager::f358_getCommandTypeFromMouseInput(MouseInput *input, Common::Point mousePos, MouseButton button) {
	if (!input)
		return k0_CommandNone;
	CommandType commandType = k0_CommandNone;

	while ((commandType = input->_commandTypeToIssue) != k0_CommandNone) {
		if (input->_hitbox.isPointInside(mousePos) && input->_button == button)
			break;
		input++;
	}
	return commandType;
}


void EventManager::f380_processCommandQueue() {
	int16 AL1159_i_ChampionIndex;
	CommandType cmdType;
	int16 L1161_i_CommandX;
	int16 L1162_i_CommandY;
	static KeyboardInput* G0481_ps_PrimaryKeyboardInputBackup;
	static KeyboardInput* G0482_ps_SecondaryKeyboardInputBackup;
	static MouseInput* G0483_ps_PrimaryMouseInputBackup;
	static MouseInput* G0484_ps_SecondaryMouseInputBackup;


	_vm->_eventMan->_g435_isCommandQueueLocked = true;
	if (_commandQueue.empty()) { /* If the command queue is empty */
		_vm->_eventMan->_g435_isCommandQueueLocked = false;
		_vm->_eventMan->f360_processPendingClick();
		return;
	}

	Command cmd = _commandQueue.pop();
	cmdType = cmd._type;
	if ((cmdType >= k3_CommandMoveForward) && (cmdType <= k6_CommandMoveLeft) && (_vm->_g310_disabledMovementTicks || (_vm->_g311_projectileDisableMovementTicks && (_vm->_g312_lastProjectileDisabledMovementDirection == (M21_normalizeModulo4(_vm->_dungeonMan->_g308_partyDir + cmdType - k3_CommandMoveForward)))))) { /* If movement is disabled */
		_vm->_eventMan->_g435_isCommandQueueLocked = false;
		_vm->_eventMan->f360_processPendingClick();
		return;
	}
	L1161_i_CommandX = cmd._pos.x;
	L1162_i_CommandY = cmd._pos.y;
	_vm->_eventMan->_g435_isCommandQueueLocked = false;
	_vm->_eventMan->f360_processPendingClick();
	if ((cmdType == k2_CommandTurnRight) || (cmdType == k1_CommandTurnLeft)) {
		_vm->_eventMan->f365_commandTurnParty(cmdType);
		return;
	}
	if ((cmdType >= k3_CommandMoveForward) && (cmdType <= k6_CommandMoveLeft)) {
		_vm->_eventMan->f366_commandMoveParty(cmdType);
		return;
	}
	if ((cmdType >= k12_CommandClickInChampion_0_StatusBox) && (cmdType <= k15_CommandClickInChampion_3_StatusBox)) {
		if (((AL1159_i_ChampionIndex = cmdType - k12_CommandClickInChampion_0_StatusBox) < _vm->_championMan->_g305_partyChampionCount) && !_vm->_championMan->_g299_candidateChampionOrdinal) {
			f367_commandProcessTypes12to27_clickInChampionStatusBox(AL1159_i_ChampionIndex, L1161_i_CommandX, L1162_i_CommandY);
		}
		return;
	}
	if ((cmdType >= k125_CommandClickOnChamptionIcon_Top_Left) && (cmdType <= k128_CommandClickOnChamptionIcon_Lower_Left)) {
		f70_mouseProcessCommands125To128_clickOnChampionIcon(cmdType - k125_CommandClickOnChamptionIcon_Top_Left);
		return;
	}
	if ((cmdType >= k28_CommandClickOnSlotBoxInventoryReadyHand) && (cmdType < (k65_CommandClickOnSlotBoxChest_8 + 1))) {
		if (_vm->_championMan->_g411_leaderIndex != kM1_ChampionNone) {
			_vm->_championMan->f302_processCommands28to65_clickOnSlotBox(cmdType - k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand);
		}
		return;
	}
	if ((cmdType >= k7_CommandToggleInventoryChampion_0) && (cmdType <= k11_CommandCloseInventory)) {
		if ((((AL1159_i_ChampionIndex = cmdType - k7_CommandToggleInventoryChampion_0) == k4_ChampionCloseInventory) || (AL1159_i_ChampionIndex < _vm->_championMan->_g305_partyChampionCount)) && !_vm->_championMan->_g299_candidateChampionOrdinal) {
			_vm->_inventoryMan->f355_toggleInventory((ChampionIndex)AL1159_i_ChampionIndex);
		}
		return;
	}
	if (cmdType == k83_CommandToggleInventoryLeader) {
		if (_vm->_championMan->_g411_leaderIndex != kM1_ChampionNone) {
			_vm->_inventoryMan->f355_toggleInventory(_vm->_championMan->_g411_leaderIndex);
		}
		return;
	}
	if (cmdType == k100_CommandClickInSpellArea) {
		if ((!_vm->_championMan->_g299_candidateChampionOrdinal) && (_vm->_championMan->_g514_magicCasterChampionIndex != kM1_ChampionNone)) {
			f370_commandProcessType100_clickInSpellArea(L1161_i_CommandX, L1162_i_CommandY);
		}
		return;
	}
	if (cmdType == k111_CommandClickInActionArea) {
		if (!_vm->_championMan->_g299_candidateChampionOrdinal) {
			f371_commandProcessType111To115_ClickInActionArea(L1161_i_CommandX, L1162_i_CommandY);
		}
		return;
	}
	if (cmdType == k70_CommandClickOnMouth) {
		warning(false, "MISSING CODE: F0349_INVENTORY_ProcessCommand70_ClickOnMouth();");
		return;
	}
	if (cmdType == k71_CommandClickOnEye) {
		warning(false, "MISSING CODE: F0352_INVENTORY_ProcessCommand71_ClickOnEye();");
		return;
	}
	if (cmdType == k80_CommandClickInDungeonView) {
		_vm->_eventMan->f377_commandProcessType80ClickInDungeonView(L1161_i_CommandX, L1162_i_CommandY);
		return;
	}
	if (cmdType == k81_CommandClickInPanel) {
		_vm->_eventMan->f378_commandProcess81ClickInPanel(L1161_i_CommandX, L1162_i_CommandY);
		return;
	}

	if (_vm->_g331_pressingEye || _vm->_g333_pressingMouth) {
		return;
	}

	if (cmdType == k145_CommandSleep) {
		if (!_vm->_championMan->_g299_candidateChampionOrdinal) {
			if (_vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
				_vm->_inventoryMan->f355_toggleInventory(k4_ChampionCloseInventory);
			}
			_vm->_menuMan->f456_drawDisabledMenu();
			_vm->_championMan->_g300_partyIsSleeping = true;
			warning(false, "MISSING CODE: F0379_COMMAND_DrawSleepScreen();");
			_vm->_displayMan->f97_drawViewport(k2_viewportAsBeforeSleepOrFreezeGame);
			_vm->_g318_waitForInputMaxVerticalBlankCount = 0;
			_vm->_eventMan->_g441_primaryMouseInput = g450_PrimaryMouseInput_PartySleeping;
			_vm->_eventMan->_g442_secondaryMouseInput = 0;
			_g443_primaryKeyboardInput = g460_primaryKeyboardInput_partySleeping;
			_g444_secondaryKeyboardInput = nullptr;
			f357_discardAllInput();
		}
		return;
	}
	if (cmdType == k146_CommandWakeUp) {
		_vm->_championMan->f314_wakeUp();
		return;
	}
	if (cmdType == k140_CommandSaveGame) {
		if ((_vm->_championMan->_g305_partyChampionCount > 0) && !_vm->_championMan->_g299_candidateChampionOrdinal) {
			warning(false, "MISSING CODE: F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF();");
		}
		return;
	}
	if (cmdType == k147_CommandFreezeGame) {
		_vm->_g301_gameTimeTicking = false;
		_vm->_menuMan->f456_drawDisabledMenu();
		_vm->_displayMan->f134_fillBitmap(_vm->_displayMan->_g296_bitmapViewport, k0_ColorBlack, 224, 136);
		// TODO: localization
		_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, 81, 69, k4_ColorCyan, k0_ColorBlack,
											 "GAME FROZEN", k136_heightViewport);
		_vm->_displayMan->f97_drawViewport(k2_viewportAsBeforeSleepOrFreezeGame);
		G0483_ps_PrimaryMouseInputBackup = _vm->_eventMan->_g441_primaryMouseInput;
		G0484_ps_SecondaryMouseInputBackup = _vm->_eventMan->_g442_secondaryMouseInput;
		G0481_ps_PrimaryKeyboardInputBackup = _g443_primaryKeyboardInput;
		G0482_ps_SecondaryKeyboardInputBackup = _g444_secondaryKeyboardInput;
		_vm->_eventMan->_g441_primaryMouseInput = g451_PrimaryMouseInput_FrozenGame;
		_vm->_eventMan->_g442_secondaryMouseInput = 0;
		_g443_primaryKeyboardInput = g461_primaryKeyboardInput_frozenGame;
		_g444_secondaryKeyboardInput = nullptr;
		f357_discardAllInput();
		return;
	}
	if (cmdType == k148_CommandUnfreezeGame) {
		_vm->_g301_gameTimeTicking = true;
		_vm->_menuMan->f457_drawEnabledMenus();
		_vm->_eventMan->_g441_primaryMouseInput = G0483_ps_PrimaryMouseInputBackup;
		_vm->_eventMan->_g442_secondaryMouseInput = G0484_ps_SecondaryMouseInputBackup;
		_g443_primaryKeyboardInput = G0481_ps_PrimaryKeyboardInputBackup;
		_g444_secondaryKeyboardInput = G0482_ps_SecondaryKeyboardInputBackup;
		f357_discardAllInput();
		return;
	}
	if (cmdType == k200_CommandEntranceEnterDungeon) {
		_vm->_g298_newGame = k1_modeLoadDungeon;
		return;
	}
	if (cmdType == k201_CommandEntranceResume) {
		_vm->_g298_newGame = k0_modeLoadSavedGame;
		return;
	}
	if (cmdType == k202_CommandEntranceDrawCredits) {
		warning(false, "MISSING CODE: F0442_STARTEND_ProcessCommand202_EntranceDrawCredits()");
		return;
	}
	if ((cmdType >= k210_CommandClickOnDialogChoice_1) && (cmdType <= k213_CommandClickOnDialogChoice_4)) {
		warning(false, "MISSING CODE:G0335_ui_SelectedDialogChoice = cmdType - (k210_CommandClickOnDialogChoice_1 - 1);");
		return;
	}
	if (cmdType == k215_CommandRestartGame) {
		_vm->_g523_restartGameRequest = true;
	}
}

void EventManager::f365_commandTurnParty(CommandType cmdType) {
	uint16 L1114_ui_Square;

	_vm->_g321_stopWaitingForPlayerInput = true;
	if (cmdType == k1_CommandTurnLeft) {
		warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
	} else {
	}
	if (Square(L1114_ui_Square = _vm->_dungeonMan->f151_getSquare(_vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY).toByte()).getType() == k3_ElementTypeStairs) {
		f364_commandTakeStairs(getFlag(L1114_ui_Square, k0x0004_StairsUp));
		return;
	}
	_vm->_movsens->f276_sensorProcessThingAdditionOrRemoval(_vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, Thing::_party, true, false);
	_vm->_championMan->f284_setPartyDirection(M21_normalizeModulo4(_vm->_dungeonMan->_g308_partyDir + ((cmdType == k2_CommandTurnRight) ? 1 : 3)));
	_vm->_movsens->f276_sensorProcessThingAdditionOrRemoval(_vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, Thing::_party, true, true);
}

void EventManager::f366_commandMoveParty(CommandType cmdType) {
	static Box g463_BoxMovementArrows[4] = { // @ G0463_as_Graphic561_Box_MovementArrows
		/* { X1, X2, Y1, Y2 } */
		Box(263, 289, 125, 145),   /* Forward */
		Box(291, 318, 147, 167),   /* Right */
		Box(263, 289, 147, 167),   /* Backward */
		Box(234, 261, 147, 167)}; /* Left */

	static int16 g465_movementArrowToStepForwardCount[4] = { // @ G0465_ai_Graphic561_MovementArrowToStepForwardCount
		1,   /* Forward */
		0,   /* Right */
		-1,  /* Backward */
		0}; /* Left */
	static int16 g466_movementArrowToSepRightCount[4] = { // @ G0466_ai_Graphic561_MovementArrowToStepRightCount
		0,    /* Forward */
		1,    /* Right */
		0,    /* Backward */
		-1}; /* Left */

	uint16 L1115_ui_Multiple;
#define AL1115_ui_Square L1115_ui_Multiple
#define AL1115_ui_Ticks  L1115_ui_Multiple
	int16 L1116_i_SquareType;
	int16 L1117_B_MovementBlocked;
	uint16 L1118_ui_Multiple;
#define AL1118_ui_ChampionIndex      L1118_ui_Multiple
#define AL1118_ui_MovementArrowIndex L1118_ui_Multiple
	Champion* L1119_ps_Champion;
	Box* L1120_ps_Box;
	int16 L1121_i_MapX;
	int16 L1122_i_MapY;
	bool L1123_B_StairsSquare;
	int16 L1124_i_FirstDamagedChampionIndex;
	int16 L1125_i_SecondDamagedChampionIndex;


	_vm->_g321_stopWaitingForPlayerInput = true;
	L1119_ps_Champion = _vm->_championMan->_gK71_champions;
	for (AL1118_ui_ChampionIndex = k0_ChampionFirst; AL1118_ui_ChampionIndex < _vm->_championMan->_g305_partyChampionCount; AL1118_ui_ChampionIndex++) {
		_vm->_championMan->f325_decrementStamine(AL1118_ui_ChampionIndex, ((L1119_ps_Champion->_load * 3) / _vm->_championMan->f309_getMaximumLoad(L1119_ps_Champion)) + 1); /* BUG0_50 When a champion is brought back to life at a Vi Altar, his current stamina is lower than what it was before dying. Each time the party moves the current stamina of all champions is decreased, including for dead champions, by an amount that depends on the current load of the champion. For a dead champion the load before he died is used */
		L1119_ps_Champion++;
	}
	AL1118_ui_MovementArrowIndex = cmdType - k3_CommandMoveForward;
	L1120_ps_Box = &g463_BoxMovementArrows[AL1118_ui_MovementArrowIndex];
	warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
	L1123_B_StairsSquare = (Square(AL1115_ui_Square = _vm->_dungeonMan->f151_getSquare(L1121_i_MapX = _vm->_dungeonMan->_g306_partyMapX, L1122_i_MapY = _vm->_dungeonMan->_g307_partyMapY).toByte()).getType() == k3_ElementTypeStairs);
	if (L1123_B_StairsSquare && (AL1118_ui_MovementArrowIndex == 2)) { /* If moving backward while in stairs */
		f364_commandTakeStairs(getFlag(AL1115_ui_Square, k0x0004_StairsUp));
		return;
	}
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(_vm->_dungeonMan->_g308_partyDir, g465_movementArrowToStepForwardCount[AL1118_ui_MovementArrowIndex], g466_movementArrowToSepRightCount[AL1118_ui_MovementArrowIndex], L1121_i_MapX, L1122_i_MapY);
	L1116_i_SquareType = Square(AL1115_ui_Square = _vm->_dungeonMan->f151_getSquare(L1121_i_MapX, L1122_i_MapY).toByte()).getType();
	if (L1116_i_SquareType == k3_ElementTypeStairs) {
		_vm->_movsens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, kM1_MapXNotOnASquare, 0);
		_vm->_dungeonMan->_g306_partyMapX = L1121_i_MapX;
		_vm->_dungeonMan->_g307_partyMapY = L1122_i_MapY;
		f364_commandTakeStairs(getFlag(AL1115_ui_Square, k0x0004_StairsUp));
		return;
	}
	L1117_B_MovementBlocked = false;
	if (L1116_i_SquareType == k0_ElementTypeWall) {
		L1117_B_MovementBlocked = true;
	} else {
		if (L1116_i_SquareType == k4_DoorElemType) {
			L1117_B_MovementBlocked = Square(AL1115_ui_Square).getDoorState();
			L1117_B_MovementBlocked = (L1117_B_MovementBlocked != k0_doorState_OPEN) && (L1117_B_MovementBlocked != k1_doorState_FOURTH) && (L1117_B_MovementBlocked != k5_doorState_DESTROYED);
		} else {
			if (L1116_i_SquareType == k6_ElementTypeFakeWall) {
				L1117_B_MovementBlocked = (!getFlag(AL1115_ui_Square, k0x0004_FakeWallOpen) && !getFlag(AL1115_ui_Square, k0x0001_FakeWallImaginary));
			}
		}
	}
	if (_vm->_championMan->_g305_partyChampionCount == 0) {
	} else {
		if (L1117_B_MovementBlocked) {
			L1117_B_MovementBlocked = _vm->_championMan->f321_addPendingDamageAndWounds_getDamage(L1124_i_FirstDamagedChampionIndex = _vm->_championMan->f286_getTargetChampionIndex(L1121_i_MapX, L1122_i_MapY, M21_normalizeModulo4(AL1118_ui_MovementArrowIndex += (_vm->_dungeonMan->_g308_partyDir + 2))), 1, k0x0008_ChampionWoundTorso | k0x0010_ChampionWoundLegs, k2_attackType_SELF);
			if (L1124_i_FirstDamagedChampionIndex != (L1125_i_SecondDamagedChampionIndex = _vm->_championMan->f286_getTargetChampionIndex(L1121_i_MapX, L1122_i_MapY, returnNextVal(AL1118_ui_MovementArrowIndex)))) {
				L1117_B_MovementBlocked |= _vm->_championMan->f321_addPendingDamageAndWounds_getDamage(L1125_i_SecondDamagedChampionIndex, 1, k0x0008_ChampionWoundTorso | k0x0010_ChampionWoundLegs, k2_attackType_SELF);
			}
			if (L1117_B_MovementBlocked) {
				warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
			}
		} else {
			if (L1117_B_MovementBlocked = (_vm->_groupMan->f175_groupGetThing(L1121_i_MapX, L1122_i_MapY) != Thing::_endOfList)) {
				_vm->_groupMan->f209_processEvents29to41(L1121_i_MapX, L1122_i_MapY, kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent, 0);
			}
		}
	}
	if (L1117_B_MovementBlocked) {
		f357_discardAllInput();
		_vm->_g321_stopWaitingForPlayerInput = false;
		return;
	}
	if (L1123_B_StairsSquare) {
		_vm->_movsens->f267_getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, L1121_i_MapX, L1122_i_MapY);
	} else {
		_vm->_movsens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, L1121_i_MapX, L1122_i_MapY);
	}
	AL1115_ui_Ticks = 1;
	L1119_ps_Champion = _vm->_championMan->_gK71_champions;
	for (AL1118_ui_ChampionIndex = k0_ChampionFirst; AL1118_ui_ChampionIndex < _vm->_championMan->_g305_partyChampionCount; AL1118_ui_ChampionIndex++) {
		if (L1119_ps_Champion->_currHealth) {
			warning(false, "possibly dangerous cast to uint16");
			AL1115_ui_Ticks = MAX(AL1115_ui_Ticks, (uint16)_vm->_championMan->f310_getMovementTicks(L1119_ps_Champion));
		}
		L1119_ps_Champion++;
	}
	_vm->_g310_disabledMovementTicks = AL1115_ui_Ticks;
	_vm->_g311_projectileDisableMovementTicks = 0;
}

bool EventManager::f375_processType80_clickDungeonView_isLeaderHandObjThrown(int16 posX, int16 posY) {
	bool L1149_B_ObjectThrown;


#define k0_sideLeft 0 // @ C0_SIDE_LEFT 
#define k1_sideRight 1 // @ C0_SIDE_LEFT 

	if ((posY < 47) || (posY > 102)) {
		return false;
	}
	if (posX <= 111) {
		if (_vm->_dungeonMan->_g285_squareAheadElement == k17_DoorFrontElemType) {
			if (posX < 64) {
				return false;
			}
		} else {
			if (posX < 32) {
				return false;
			}
		}
		// Strangerke: Only poresent in CSB2.1... But it fixes a bug so we keep it
		L1149_B_ObjectThrown = _vm->_championMan->f329_isLeaderHandObjectThrown(k0_sideLeft);
	} else {
		if (_vm->_dungeonMan->_g285_squareAheadElement == k17_DoorFrontElemType) {
			if (posX > 163) {
				return false;
			}
		} else {
			if (posX > 191) {
				return false;
			}
		}
		L1149_B_ObjectThrown = _vm->_championMan->f329_isLeaderHandObjectThrown(k1_sideRight);
	}
	if (L1149_B_ObjectThrown) {
		_vm->_g321_stopWaitingForPlayerInput = true;
	}
	return L1149_B_ObjectThrown;
}



void EventManager::f368_commandSetLeader(ChampionIndex champIndex) {
	ChampionMan &cm = *_vm->_championMan;
	ChampionIndex leaderIndex;

	if ((cm._g411_leaderIndex == champIndex) || ((champIndex != kM1_ChampionNone) && !cm._gK71_champions[champIndex]._currHealth))
		return;

	if (cm._g411_leaderIndex != kM1_ChampionNone) {
		leaderIndex = cm._g411_leaderIndex;
		cm._gK71_champions[leaderIndex].setAttributeFlag(k0x0200_ChampionAttributeLoad, true);
		cm._gK71_champions[leaderIndex].setAttributeFlag(k0x0080_ChampionAttributeNameTitle, true);
		cm._gK71_champions[leaderIndex]._load -= _vm->_dungeonMan->f140_getObjectWeight(cm._g414_leaderHandObject);
		cm._g411_leaderIndex = kM1_ChampionNone;
		cm.f292_drawChampionState(leaderIndex);
	}
	if (champIndex == kM1_ChampionNone) {
		cm._g411_leaderIndex = kM1_ChampionNone;
		return;
	}
	cm._g411_leaderIndex = champIndex;
	Champion *champion = &cm._gK71_champions[cm._g411_leaderIndex];
	champion->_dir = _vm->_dungeonMan->_g308_partyDir;
	cm._gK71_champions[champIndex]._load += _vm->_dungeonMan->f140_getObjectWeight(cm._g414_leaderHandObject);
	if (_vm->M0_indexToOrdinal(champIndex) != cm._g299_candidateChampionOrdinal) {
		champion->setAttributeFlag(k0x0400_ChampionAttributeIcon, true);
		champion->setAttributeFlag(k0x0080_ChampionAttributeNameTitle, true);
		cm.f292_drawChampionState(champIndex);
	}
}

void EventManager::f372_commandProcessType80ClickInDungeonViewTouchFrontWall() {
	uint16 L1135_ui_MapX;
	uint16 L1136_ui_MapY;


	L1135_ui_MapX = _vm->_dungeonMan->_g306_partyMapX;
	L1136_ui_MapY = _vm->_dungeonMan->_g307_partyMapY;
	L1135_ui_MapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_g308_partyDir], L1136_ui_MapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_g308_partyDir];
	if ((L1135_ui_MapX >= 0) && (L1135_ui_MapX < _vm->_dungeonMan->_g273_currMapWidth) && (L1136_ui_MapY >= 0) && (L1136_ui_MapY < _vm->_dungeonMan->_g274_currMapHeight)) {
		_vm->_g321_stopWaitingForPlayerInput = _vm->_movsens->f275_sensorIsTriggeredByClickOnWall(L1135_ui_MapX, L1136_ui_MapY, returnOppositeDir(_vm->_dungeonMan->_g308_partyDir));
	}
}

void EventManager::f377_commandProcessType80ClickInDungeonView(int16 posX, int16 posY) {
	uint16 L1150_ui_ViewCell;
	Junk* L1151_ps_Junk;
	Thing L1152_T_Thing;
	uint16 L1153_ui_IconIndex;
	uint16 L1154_ui_Weight;
	int16 L1155_i_MapX;
	int16 L1156_i_MapY;

	if (_vm->_dungeonMan->_g285_squareAheadElement == k17_DoorFrontElemType) {
		if (_vm->_championMan->_g411_leaderIndex == kM1_ChampionNone) {
			return;
		}
		L1155_i_MapX = _vm->_dungeonMan->_g306_partyMapX;
		L1156_i_MapY = _vm->_dungeonMan->_g307_partyMapY;
		L1155_i_MapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_g308_partyDir], L1156_i_MapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_g308_partyDir];
		if (_vm->_championMan->_g415_leaderEmptyHanded) {
			L1151_ps_Junk = (Junk*)_vm->_dungeonMan->f157_getSquareFirstThingData(L1155_i_MapX, L1156_i_MapY);
			if ((((Door*)L1151_ps_Junk)->hasButton()) && _vm->_dungeonMan->_g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn].isPointInside(posX, posY - 33)) {
				_vm->_g321_stopWaitingForPlayerInput = true;
				warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				_vm->_movsens->f268_addEvent(k10_TMEventTypeDoor, L1155_i_MapX, L1156_i_MapY, 0, k2_SensorEffToggle, _vm->_g313_gameTime + 1);
				return;
			}
		} else {
			if (f375_processType80_clickDungeonView_isLeaderHandObjThrown(posX, posY)) {
				return;
			}
		}
	}
	if (_vm->_championMan->_g415_leaderEmptyHanded) {
		for (L1150_ui_ViewCell = k0_ViewCellFronLeft; L1150_ui_ViewCell < k5_ViewCellDoorButtonOrWallOrn + 1; L1150_ui_ViewCell++) {
			if (_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[L1150_ui_ViewCell].isPointInside(posX, posY - 33)) {
				if (L1150_ui_ViewCell == k5_ViewCellDoorButtonOrWallOrn) {
					if (!_vm->_dungeonMan->_g286_isFacingAlcove) {
						f372_commandProcessType80ClickInDungeonViewTouchFrontWall();
					}
				} else {
					f373_processType80_clickInDungeonView_grabLeaderHandObject(L1150_ui_ViewCell);
				}
				return;
			}
		}
	} else {
		L1152_T_Thing = _vm->_championMan->_g414_leaderHandObject;
		L1151_ps_Junk = (Junk*)_vm->_dungeonMan->f156_getThingData(L1152_T_Thing);
		if (_vm->_dungeonMan->_g285_squareAheadElement == k0_ElementTypeWall) {
			for (L1150_ui_ViewCell = k0_ViewCellFronLeft; L1150_ui_ViewCell < k1_ViewCellFrontRight + 1; L1150_ui_ViewCell++) {
				if (g462_BoxObjectPiles[L1150_ui_ViewCell].isPointInside(posX, posY)) {
					f374_processType80_clickInDungeonViewDropLeaderHandObject(L1150_ui_ViewCell);
					return;
				}
			}
			if (_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn].isPointInside(posX, posY - 33)) {
				if (_vm->_dungeonMan->_g286_isFacingAlcove) {
					f374_processType80_clickInDungeonViewDropLeaderHandObject(k4_ViewCellAlcove);
				} else {
					if (_vm->_dungeonMan->_g288_isFacingFountain) {
						L1153_ui_IconIndex = _vm->_objectMan->f33_getIconIndex(L1152_T_Thing);
						L1154_ui_Weight = _vm->_dungeonMan->f140_getObjectWeight(L1152_T_Thing);
						if ((L1153_ui_IconIndex >= k8_IconIndiceJunkWater) && (L1153_ui_IconIndex <= k9_IconIndiceJunkWaterSkin)) {
							L1151_ps_Junk->setChargeCount(3); /* Full */
						} else {
							if (L1153_ui_IconIndex == k195_IconIndicePotionEmptyFlask) {
								((Potion*)L1151_ps_Junk)->setType(k15_PotionTypeWaterFlask);
							} else {
								goto T0377019;
							}
						}
						_vm->_championMan->f296_drawChangedObjectIcons();
						_vm->_championMan->_gK71_champions[_vm->_championMan->_g411_leaderIndex]._load += _vm->_dungeonMan->f140_getObjectWeight(L1152_T_Thing) - L1154_ui_Weight;
					}
T0377019:
					f372_commandProcessType80ClickInDungeonViewTouchFrontWall();
				}
			}
		} else {
			if (f375_processType80_clickDungeonView_isLeaderHandObjThrown(posX, posY)) {
				return;
			}
			for (L1150_ui_ViewCell = k0_ViewCellFronLeft; L1150_ui_ViewCell < k3_ViewCellBackLeft + 1; L1150_ui_ViewCell++) {
				if (g462_BoxObjectPiles[L1150_ui_ViewCell].isPointInside(posX, posY)) {
					f374_processType80_clickInDungeonViewDropLeaderHandObject(L1150_ui_ViewCell);
					return;
				}
			}
		}
	}
}

void EventManager::f282_commandProcessCommands160To162ClickInResurrectReincarnatePanel(CommandType commandType) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;

	uint16 championIndex = champMan._g305_partyChampionCount - 1;
	Champion *champ = &champMan._gK71_champions[championIndex];
	if (commandType == k162_CommandClickInPanelCancel) {
		invMan.f355_toggleInventory(k4_ChampionCloseInventory);
		champMan._g299_candidateChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
		if (champMan._g305_partyChampionCount == 1) {
			f368_commandSetLeader(kM1_ChampionNone);
		}
		champMan._g305_partyChampionCount--;
		Box box;
		box._y1 = 0;
		box._y2 = 28;
		box._x1 = championIndex * k69_ChampionStatusBoxSpacing;
		box._x2 = box._x1 + 66;
		dispMan._g578_useByteBoxCoordinates = false;
		dispMan.D24_fillScreenBox(box, k0_ColorBlack);
		dispMan.D24_fillScreenBox(g54_BoxChampionIcons[champMan.M26_championIconIndex(champ->_cell, dunMan._g308_partyDir) * 2], k0_ColorBlack);
		_vm->_menuMan->f457_drawEnabledMenus();
		_vm->_eventMan->f78_showMouse();
		return;
	}

	champMan._g299_candidateChampionOrdinal = _vm->M0_indexToOrdinal(kM1_ChampionNone);
	int16 mapX = dunMan._g306_partyMapX + _vm->_dirIntoStepCountEast[dunMan._g308_partyDir];
	int16 mapY = dunMan._g307_partyMapY + _vm->_dirIntoStepCountNorth[dunMan._g308_partyDir];

	for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++) {
		Thing thing = champ->getSlot((ChampionSlot)slotIndex);
		if (thing != Thing::_none) {
			_vm->_dungeonMan->f164_unlinkThingFromList(thing, Thing(0), mapX, mapY);
		}
	}
	Thing thing = dunMan.f161_getSquareFirstThing(mapX, mapY);
	for (;;) { // infinite
		if (thing.getType() == k3_SensorThingType) {
			((Sensor*)dunMan.f156_getThingData(thing))->setTypeDisabled();
			break;
		}
		thing = dunMan.f159_getNextThing(thing);
	}

	if (commandType == k161_CommandClickInPanelReincarnate) {
		champMan.f281_renameChampion(champ);
		champ->resetSkillsToZero();

		for (uint16 i = 0; i < 12; i++) {
			uint16 statIndex = _vm->_rnd->getRandomNumber(7);
			champ->getStatistic((ChampionStatisticType)statIndex, k1_ChampionStatCurrent)++; // returns reference
			champ->getStatistic((ChampionStatisticType)statIndex, k0_ChampionStatMaximum)++; // returns reference
		}
	}

	if (champMan._g305_partyChampionCount == 1) {
		_vm->_projexpl->_g362_lastPartyMovementTime = _vm->_g313_gameTime;
		f368_commandSetLeader(k0_ChampionFirst);
		_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(k0_ChampionFirst);
	} else {
		_vm->_menuMan->f393_drawSpellAreaControls(champMan._g514_magicCasterChampionIndex);
	}

	_vm->_textMan->f51_messageAreaPrintLineFeed();
	Color champColor = g46_ChampionColor[championIndex];
	_vm->_textMan->f47_messageAreaPrintMessage(champColor, champ->_name);
	_vm->_textMan->f47_messageAreaPrintMessage(champColor, (commandType == k160_CommandClickInPanelResurrect) ? " RESURRECTED." : " REINCARNATED."); // TODO: localization

	invMan.f355_toggleInventory(k4_ChampionCloseInventory);
	_vm->_menuMan->f457_drawEnabledMenus();
	_vm->_eventMan->f67_setMousePointerToNormal((_vm->_championMan->_g411_leaderIndex == kM1_ChampionNone) ? k0_pointerArrow : k1_pointerHand);
}

void EventManager::f378_commandProcess81ClickInPanel(int16 x, int16 y) {
	ChampionMan &champMan = *_vm->_championMan;
	InventoryMan &invMan = *_vm->_inventoryMan;

	CommandType commandType;
	switch (invMan._g424_panelContent) {
	case k4_PanelContentChest:
		if (champMan._g411_leaderIndex == kM1_ChampionNone) // if no leader
			return;
		commandType = f358_getCommandTypeFromMouseInput(g456_MouseInput_PanelChest, Common::Point(x, y), k1_LeftMouseButton);
		if (commandType != k0_CommandNone)
			warning(false, "MISSING CODE: F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox");
		break;
	case k5_PanelContentResurrectReincarnate:
		if (!champMan._g415_leaderEmptyHanded)
			break;
		commandType = f358_getCommandTypeFromMouseInput(g457_MouseInput_PanelResurrectReincarnateCancel, Common::Point(x, y), k1_LeftMouseButton);
		if (commandType != k0_CommandNone)
			f282_commandProcessCommands160To162ClickInResurrectReincarnatePanel(commandType);
		break;
	default:
		break;
	}
}

void EventManager::f373_processType80_clickInDungeonView_grabLeaderHandObject(uint16 viewCell) {
	int16 L1137_i_MapX;
	int16 L1138_i_MapY;
	Thing L1139_T_Thing;


	if (_vm->_championMan->_g411_leaderIndex == kM1_ChampionNone) {
		return;
	}
	L1137_i_MapX = _vm->_dungeonMan->_g306_partyMapX;
	L1138_i_MapY = _vm->_dungeonMan->_g307_partyMapY;
	if (viewCell >= k2_ViewCellBackRight) {
		L1137_i_MapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_g308_partyDir], L1138_i_MapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_g308_partyDir];
		if (((L1139_T_Thing = _vm->_groupMan->f175_groupGetThing(L1137_i_MapX, L1138_i_MapY)) != Thing::_endOfList) &&
			!_vm->_movsens->f264_isLevitating(L1139_T_Thing) &&
			_vm->_groupMan->f176_getCreatureOrdinalInCell((Group*)_vm->_dungeonMan->f156_getThingData(L1139_T_Thing), M21_normalizeModulo4(viewCell + _vm->_dungeonMan->_g308_partyDir))) {
			return; /* It is not possible to grab an object on floor if there is a non levitating creature on its cell */
		}
	}
	L1139_T_Thing = _vm->_dungeonMan->_g292_pileTopObject[viewCell];
	if (_vm->_objectMan->f33_getIconIndex(L1139_T_Thing) != kM1_IconIndiceNone) {
		_vm->_movsens->f267_getMoveResult(L1139_T_Thing, L1137_i_MapX, L1138_i_MapY, kM1_MapXNotOnASquare, 0);
		_vm->_championMan->f297_putObjectInLeaderHand(L1139_T_Thing, true);
	}
	_vm->_g321_stopWaitingForPlayerInput = true;
}

void EventManager::f374_processType80_clickInDungeonViewDropLeaderHandObject(uint16 viewCell) {
	int16 L1140_i_MapX;
	int16 L1141_i_MapY;
	Thing L1142_T_Thing;
	Junk* L1143_ps_Junk;
	int16 L1144_i_IconIndex;
	uint16 L1145_ui_Cell;
	bool L1146_B_DroppingIntoAnAlcove;
	TimelineEvent L1147_s_Event;


	if (_vm->_championMan->_g411_leaderIndex == kM1_ChampionNone) {
		return;
	}
	L1140_i_MapX = _vm->_dungeonMan->_g306_partyMapX;
	L1141_i_MapY = _vm->_dungeonMan->_g307_partyMapY;
	if (L1146_B_DroppingIntoAnAlcove = (viewCell == k4_ViewCellAlcove)) {
		viewCell = k2_ViewCellBackRight;
	}
	if (viewCell > k1_ViewCellFrontRight) {
		L1140_i_MapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_g308_partyDir], L1141_i_MapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_g308_partyDir];
	}
	L1145_ui_Cell = M21_normalizeModulo4(_vm->_dungeonMan->_g308_partyDir + viewCell);
	L1142_T_Thing = _vm->_championMan->f298_getObjectRemovedFromLeaderHand();
	_vm->_movsens->f267_getMoveResult(M15_thingWithNewCell(L1142_T_Thing, L1145_ui_Cell), kM1_MapXNotOnASquare, 0, L1140_i_MapX, L1141_i_MapY);
	if (L1146_B_DroppingIntoAnAlcove && _vm->_dungeonMan->_g287_isFacingViAltar && ((L1144_i_IconIndex = _vm->_objectMan->f33_getIconIndex(L1142_T_Thing)) == k147_IconIndiceJunkChampionBones)) {
		L1143_ps_Junk = (Junk*)_vm->_dungeonMan->f156_getThingData(L1142_T_Thing);
		M33_setMapAndTime(L1147_s_Event._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + 1);
		L1147_s_Event._type = k13_TMEventTypeViAltarRebirth;
		L1147_s_Event._priority = L1143_ps_Junk->getChargeCount();
		L1147_s_Event._B._location._mapX = L1140_i_MapX;
		L1147_s_Event._B._location._mapY = L1141_i_MapY;
		L1147_s_Event._C.A._cell = L1145_ui_Cell;
		L1147_s_Event._C.A._effect = k2_SensorEffToggle;
		_vm->_timeline->f238_addEventGetEventIndex(&L1147_s_Event);
	}
	_vm->_g321_stopWaitingForPlayerInput = true;
}

bool EventManager::f360_hasPendingClick(Common::Point& point, MouseButton button) {
	if (_g439_pendingClickButton && button == _g439_pendingClickButton)
		point = _g437_pendingClickPos;

	return _g436_pendingClickPresent;
}

void EventManager::f379_drawSleepScreen() {
	_vm->_displayMan->f134_fillBitmap(_vm->_displayMan->_g296_bitmapViewport, k0_ColorBlack, 224, 136); // TODO: localization
}

void EventManager::f357_discardAllInput() {
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event))
		;
	_commandQueue.clear();
}

void EventManager::f364_commandTakeStairs(bool stairsGoDown) {
	_vm->_movsens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, kM1_MapXNotOnASquare, 0);
	_vm->_g327_newPartyMapIndex = _vm->_dungeonMan->f154_getLocationAfterLevelChange(_vm->_dungeonMan->_g309_partyMapIndex, stairsGoDown ? -1 : 1, &_vm->_dungeonMan->_g306_partyMapX, &_vm->_dungeonMan->_g307_partyMapY);
	_vm->_dungeonMan->f173_setCurrentMap(_vm->_g327_newPartyMapIndex);
	_vm->_championMan->f284_setPartyDirection(_vm->_dungeonMan->f155_getStairsExitDirection(_vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY));
	_vm->_dungeonMan->f173_setCurrentMap(_vm->_dungeonMan->_g309_partyMapIndex);
}

void EventManager::f367_commandProcessTypes12to27_clickInChampionStatusBox(uint16 champIndex, int16 posX, int16 posY) {
	uint16 L1126_ui_Command;

	if (_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
		_vm->_eventMan->f368_commandSetLeader((ChampionIndex)champIndex);
	} else {
		L1126_ui_Command = _vm->_eventMan->f358_getCommandTypeFromMouseInput(g455_MouseInput_ChampionNamesHands, Common::Point(posX, posY), k1_LeftMouseButton);
		if ((L1126_ui_Command >= k16_CommandSetLeaderChampion_0) && (L1126_ui_Command <= k19_CommandSetLeaderChampion_3)) {
			_vm->_eventMan->f368_commandSetLeader((ChampionIndex)(L1126_ui_Command - k16_CommandSetLeaderChampion_0));
		} else {
			if ((L1126_ui_Command >= k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand) && (L1126_ui_Command <= k27_CommandClickOnSlotBoxChampion_3_StatusBoxActionHand)) {
				_vm->_championMan->f302_processCommands28to65_clickOnSlotBox(L1126_ui_Command - k20_CommandClickOnSlotBoxChampion_0_StatusBoxReadyHand);
			}
		}
	}
}

void EventManager::f70_mouseProcessCommands125To128_clickOnChampionIcon(uint16 champIconIndex) {
	static Box G0621_s_Box_MousePointer_ChampionIconShadow = Box(2, 20, 2, 15);
	static Box G0622_s_Box_MousePointer_ChampionIcon = Box(0, 18, 0, 13);
	static byte G0045_auc_Graphic562_PaletteChanges_MousePointerIconShadow[16] = {0, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 0, 120, 120, 120};

	uint16 L0052_ui_ChampionIconIndex;
	int16 L0053_i_ChampionIndex;
	int16 L0054_i_ChampionIndex;
	Box* L0055_pi_ChampionIconBox;
	byte* L0056_puc_Bitmap;


	_gK100_preventBuildPointerScreenArea = true;
	if (!_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap) {
		if (_vm->_championMan->f285_getIndexInCell(M21_normalizeModulo4(champIconIndex + _vm->_dungeonMan->_g308_partyDir)) == kM1_ChampionNone) {
			_gK100_preventBuildPointerScreenArea = false;
			return;
		}
		_vm->_eventMan->_g598_mousePointerBitmapUpdated = true;
		_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap = true;
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		L0056_puc_Bitmap = _gK190_mousePointerTempBuffer;
		memset(L0056_puc_Bitmap, 0, 32 * 18);
		L0055_pi_ChampionIconBox = &g54_BoxChampionIcons[champIconIndex];

		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g348_bitmapScreen, L0056_puc_Bitmap, G0621_s_Box_MousePointer_ChampionIconShadow, L0055_pi_ChampionIconBox->_x1, L0055_pi_ChampionIconBox->_y1, k160_byteWidthScreen, k16_byteWidth, k0_ColorBlack, 200, 18);

		_vm->_displayMan->f129_blitToBitmapShrinkWithPalChange(L0056_puc_Bitmap, _g613_mousePointerOriginalColorsChampionIcon, 32, 18, 32, 18, G0045_auc_Graphic562_PaletteChanges_MousePointerIconShadow);

		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g348_bitmapScreen, _g613_mousePointerOriginalColorsChampionIcon, G0622_s_Box_MousePointer_ChampionIcon, L0055_pi_ChampionIconBox->_x1, L0055_pi_ChampionIconBox->_y1, k160_byteWidthScreen, k16_byteWidth, k0_ColorBlack, 200, 18);

		_vm->_displayMan->D24_fillScreenBox(*L0055_pi_ChampionIconBox, k0_ColorBlack);
		_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap = _vm->M0_indexToOrdinal(champIconIndex);
	} else {
		_vm->_eventMan->_g598_mousePointerBitmapUpdated = true;
		L0052_ui_ChampionIconIndex = _vm->M1_ordinalToIndex(_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap);
		_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap = _vm->M0_indexToOrdinal(kM1_ChampionNone);
		L0054_i_ChampionIndex = _vm->_championMan->f285_getIndexInCell(M21_normalizeModulo4(L0052_ui_ChampionIconIndex + _vm->_dungeonMan->_g308_partyDir));
		if (L0052_ui_ChampionIconIndex == champIconIndex) {
			setFlag(_vm->_championMan->_gK71_champions[L0054_i_ChampionIndex]._attributes, k0x0400_ChampionAttributeIcon);
			_vm->_championMan->f292_drawChampionState((ChampionIndex)L0054_i_ChampionIndex);
		} else {
			L0053_i_ChampionIndex = _vm->_championMan->f285_getIndexInCell(M21_normalizeModulo4(champIconIndex + _vm->_dungeonMan->_g308_partyDir));
			if (L0053_i_ChampionIndex >= 0) {
				_vm->_championMan->_gK71_champions[L0053_i_ChampionIndex]._cell = (ViewCell)M21_normalizeModulo4(L0052_ui_ChampionIconIndex + _vm->_dungeonMan->_g308_partyDir);
				setFlag(_vm->_championMan->_gK71_champions[L0053_i_ChampionIndex]._attributes, k0x0400_ChampionAttributeIcon);
				_vm->_championMan->f292_drawChampionState((ChampionIndex)L0053_i_ChampionIndex);
			} else {
				_vm->_displayMan->D24_fillScreenBox(g54_BoxChampionIcons[L0052_ui_ChampionIconIndex], k0_ColorBlack);
			}
			_vm->_championMan->_gK71_champions[L0054_i_ChampionIndex]._cell = (ViewCell)M21_normalizeModulo4(champIconIndex + _vm->_dungeonMan->_g308_partyDir);
			setFlag(_vm->_championMan->_gK71_champions[L0054_i_ChampionIndex]._attributes, k0x0400_ChampionAttributeIcon);
			_vm->_championMan->f292_drawChampionState((ChampionIndex)L0054_i_ChampionIndex);
		}
	}
	_gK100_preventBuildPointerScreenArea = false;
	f73_buildpointerScreenArea(_mousePos.x, _mousePos.y);
}

void EventManager::f370_commandProcessType100_clickInSpellArea(uint16 posX, uint16 posY) {
	int16 L1132_i_Command;
	int16 L1133_i_ChampionIndex;


	L1133_i_ChampionIndex = kM1_ChampionNone;
	if (posY <= 48) {
		switch (_vm->_championMan->_g514_magicCasterChampionIndex) {
		case 0:
			if ((posX >= 280) && (posX <= 291)) {
				L1133_i_ChampionIndex = 1;
			} else {
				if ((posX >= 294) && (posX <= 305)) {
					L1133_i_ChampionIndex = 2;
				} else {
					if (posX >= 308) {
						L1133_i_ChampionIndex = 3;
					}
				}
			}
			break;
		case 1:
			if ((posX >= 233) && (posX <= 244)) {
				L1133_i_ChampionIndex = 0;
			} else {
				if ((posX >= 294) && (posX <= 305)) {
					L1133_i_ChampionIndex = 2;
				} else {
					if (posX >= 308) {
						L1133_i_ChampionIndex = 3;
					}
				}
			}
			break;
		case 2:
			if ((posX >= 233) && (posX <= 244)) {
				L1133_i_ChampionIndex = 0;
			} else {
				if ((posX >= 247) && (posX <= 258)) {
					L1133_i_ChampionIndex = 1;
				} else {
					if (posX >= 308) {
						L1133_i_ChampionIndex = 3;
					}
				}
			}
			break;
		case 3:
			if ((posX >= 247) && (posX <= 258)) {
				L1133_i_ChampionIndex = 1;
			} else {
				if ((posX >= 261) && (posX <= 272)) {
					L1133_i_ChampionIndex = 2;
				} else {
					if (posX <= 244) {
						L1133_i_ChampionIndex = 0;
					}
				}
			}
		}
		if ((L1133_i_ChampionIndex != kM1_ChampionNone) && (L1133_i_ChampionIndex < _vm->_championMan->_g305_partyChampionCount)) {
			_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(L1133_i_ChampionIndex);
		}
		return;
	}
	L1132_i_Command = _vm->_eventMan->f358_getCommandTypeFromMouseInput(g454_MouseInput_SpellArea, Common::Point(posX, posY), k1_LeftMouseButton);
	if (L1132_i_Command != k0_CommandNone) {
		f369_commandProcessTypes101To108_clickInSpellSymbolsArea((CommandType)L1132_i_Command);
	}
}

void EventManager::f369_commandProcessTypes101To108_clickInSpellSymbolsArea(CommandType cmdType) {
	static Box G0464_as_Graphic561_Box_SpellSymbolsAndDelete[7] = {
		/* { X1, X2, Y1, Y2 } */
		Box(235, 247, 51, 61),   /* Symbol 1 */
		Box(249, 261, 51, 61),   /* Symbol 2 */
		Box(263, 275, 51, 61),   /* Symbol 3 */
		Box(277, 289, 51, 61),   /* Symbol 4 */
		Box(291, 303, 51, 61),   /* Symbol 5 */
		Box(305, 317, 51, 61),   /* Symbol 6 */
		Box(305, 318, 63, 73)}; /* Delete */

	uint16 L1130_ui_SymbolIndex;
	Box* L1131_ps_Box;


	if (cmdType == k108_CommandClickInSpeallAreaCastSpell) {
		if (_vm->_championMan->_gK71_champions[_vm->_championMan->_g514_magicCasterChampionIndex]._symbols[0] == '\0') {
			return;
		}
		warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
		_vm->_g321_stopWaitingForPlayerInput = _vm->_menuMan->f408_getClickOnSpellCastResult();
		return;
	}
	L1130_ui_SymbolIndex = cmdType - k101_CommandClickInSpellAreaSymbol_1;
	L1131_ps_Box = &G0464_as_Graphic561_Box_SpellSymbolsAndDelete[L1130_ui_SymbolIndex];
	warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
	_vm->f22_delay(1);
	warning(false, "MISSING CODE: F0363_COMMAND_HighlightBoxDisable");
	if (L1130_ui_SymbolIndex < 6) {
		_vm->_menuMan->f399_addChampionSymbol(L1130_ui_SymbolIndex);
	} else {
		_vm->_menuMan->f400_deleteChampionSymbol();
	}
}

void EventManager::f371_commandProcessType111To115_ClickInActionArea(int16 posX, int16 posY) {
	uint16 L1134_ui_Command;


	if (_vm->_championMan->_g506_actingChampionOrdinal) {
		L1134_ui_Command = _vm->_eventMan->f358_getCommandTypeFromMouseInput(g452_MouseInput_ActionAreaNames, Common::Point(posX, posY), k1_LeftMouseButton);
		if (L1134_ui_Command != k0_CommandNone) {
			if (L1134_ui_Command == k112_CommandClickInActionAreaPass) {
				warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
				_vm->_menuMan->f391_didClickTriggerAction(-1);
			} else {
				if ((L1134_ui_Command - k112_CommandClickInActionAreaPass) <= _vm->_menuMan->_g507_actionCount) {
					if (L1134_ui_Command == k113_CommandClickInActionAreaAction_0) {
						warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
					} else {
						if (L1134_ui_Command == k114_CommandClickInActionAreaAction_1) {
							warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
						} else {
							warning(false, "MISSING CODE: F0362_COMMAND_HighlightBoxEnable");
						}
					}
					_vm->_g321_stopWaitingForPlayerInput = _vm->_menuMan->f391_didClickTriggerAction(L1134_ui_Command - k113_CommandClickInActionAreaAction_0);
				}
			}
		}
	} else {
		if (_vm->_menuMan->_g509_actionAreaContainsIcons) {
			L1134_ui_Command = _vm->_eventMan->f358_getCommandTypeFromMouseInput(g453_MouseInput_ActionAreaIcons, Common::Point(posX, posY), k1_LeftMouseButton);
			if (L1134_ui_Command != k0_CommandNone) {
				if ((L1134_ui_Command = L1134_ui_Command - k116_CommandClickInActionAreaChampion_0_Action) < _vm->_championMan->_g305_partyChampionCount) {
					_vm->_menuMan->f389_processCommands116To119_setActingChampion(L1134_ui_Command);
				}
			}
		}
	}
}
} // end of namespace DM
