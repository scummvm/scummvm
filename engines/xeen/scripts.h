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

#ifndef XEEN_SCRIPTS_H
#define XEEN_SCRIPTS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/serializer.h"
#include "xeen/files.h"

namespace Xeen {

enum Opcode {
	OP_None			= 0x00,
	OP_Display0x01	= 0x01,
	OP_DoorTextSml	= 0x02,
	OP_DoorTextLrg	= 0x03,
	OP_SignText		= 0x04,
	OP_NPC			= 0x05,
	OP_PlayFX		= 0x06,
	OP_TeleportAndExit = 0x07,
	OP_If_1			= 0x08,
	OP_If_2			= 0x09,
	OP_If3			= 0x0A,
	OP_MoveObj		= 0x0B,
	OP_TakeOrGive	= 0x0C,
	OP_NoAction		= 0x0D,
	OP_Remove		= 0x0E,
	OP_SetChar		= 0x0F,
	OP_Spawn		= 0x10,
	OP_DoTownEvent	= 0x11,
	OP_Exit			= 0x12,
	OP_AfterMap		= 0x13,
	OP_GiveExtended = 0x14,
	OP_ConfirmWord	= 0x15,
	OP_Damage		= 0x16,
	OP_JumpRnd		= 0x17,
	OP_AfterEvent	= 0x18,
	OP_CallEvent	= 0x19,
	OP_Return		= 0x1A,
	OP_SetVar		= 0x1B,
	OP_TakeOrGive_2 = 0x1C,
	OP_TakeOrGive_3 = 0x1D,
	OP_CutsceneEndClouds = 0x1E,
	OP_TeleportAndContinue = 0x1F,
	OP_WhoWill		= 0x20,
	OP_RndDamage	= 0x21,
	OP_MoveWallObj	= 0x22,
	OP_AlterCellFlag= 0x23,
	OP_AlterHed		= 0x24,
	OP_DisplayStat	= 0x25,
	OP_TakeOrGive_4	= 0x26,
	OP_SeatTextSml	= 0x27,
	OP_PlayEventVoc = 0x28,
	OP_DisplayBottom = 0x29,
	OP_IfMapFlag	= 0x2A,
	OP_SelRndChar	= 0x2B,
	OP_GiveEnchanted= 0x2C,
	OP_ItemType		= 0x2D,
	OP_MakeNothingHere = 0x2E,
	OP_NoAction_2	= 0x2F,
	OP_ChooseNumeric= 0x30,
	OP_DisplayBottomTwoLines = 0x31,
	OP_DisplayLarge	= 0x32,
	OP_ExchObj		= 0x33,
	OP_FallToMap	= 0x34,
	OP_DisplayMain	= 0x35,
	OP_Goto			= 0x36,
	OP_ConfirmWord_2= 0x37,
	OP_GotoRandom	= 0x38,
	OP_CutsceneEndDarkside = 0x39,
	OP_CutsceneEdWorld = 0x3A,
	OP_FlipWorld	= 0x3B,
	OP_PlayCD		= 0x3C
};

class XeenEngine;

class MazeEvent {
public:
	Common::Point _position;
	int _direction;
	int _line;
	Opcode _opcode;
	Common::Array<byte> _parameters;
public:
	MazeEvent();

	void synchronize(Common::Serializer &s);
};

class MazeEvents : public Common::Array<MazeEvent> {
public:
	Common::Array<byte> _text;
public:
	void synchronize(XeenSerializer &s);
};

class Scripts {
private:
	XeenEngine *_vm;
public:
	Scripts(XeenEngine *vm);

	void checkEvents();

	void giveTreasure();

	void openGrate(int v1, int v2);
};

} // End of namespace Xeen

#endif /* XEEN_SCRIPTS_H */
