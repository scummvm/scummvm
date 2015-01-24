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
#include "common/stack.h"
#include "common/str-array.h"
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
	Common::StringArray _text;
public:
	void synchronize(XeenSerializer &s);
};

struct StackEntry : public Common::Point {
	int line;

	StackEntry(const Common::Point &pt, int l) : Common::Point(pt), line(l) {}
};

class Scripts {
private:
	XeenEngine *_vm;
	int _charFX[6];
	int _itemType;
	int _treasureItems;
	int _treasureGold;
	int _treasureGems;
	int _lineNum;
	int _charIndex;

	int _v2;
	int _var4F;
	int _nEdamageType;
	Common::String _paramText;
	MazeEvent *_event;
	Common::Point _currentPos;
	Common::Stack<StackEntry> _stack;

	void doOpcode(MazeEvent &event);
	void cmdDisplay1(Common::Array<byte> &params);
	void cmdDoorTextSml(Common::Array<byte> &params);
	void cmdDoorTextLrg(Common::Array<byte> &params);
	void cmdSignText(Common::Array<byte> &params);
	void cmdNPC(Common::Array<byte> &params);
	void cmdPlayFX(Common::Array<byte> &params);
	void cmdTeleport(Common::Array<byte> &params);
	void cmdIf(Common::Array<byte> &params);
	void cmdMoveObj(Common::Array<byte> &params);
	void cmdTakeOrGive(Common::Array<byte> &params);
	void cmdNoAction(Common::Array<byte> &params);
	void cmdRemove(Common::Array<byte> &params);
	void cmdSetChar(Common::Array<byte> &params);
	void cmdSpawn(Common::Array<byte> &params);
	void cmdDoTownEvent(Common::Array<byte> &params);
	void cmdExit(Common::Array<byte> &params);
	void cmdAlterMap(Common::Array<byte> &params);
	void cmdGiveExtended(Common::Array<byte> &params);
	void cmdConfirmWord(Common::Array<byte> &params);
	void cmdDamage(Common::Array<byte> &params);
	void cmdJumpRnd(Common::Array<byte> &params);
	void cmdAlterEvent(Common::Array<byte> &params);
	void cmdCallEvent(Common::Array<byte> &params);
	void cmdReturn(Common::Array<byte> &params);
	void cmdSetVar(Common::Array<byte> &params);
	void cmdCutsceneEndClouds(Common::Array<byte> &params);
	void cmdWhoWill(Common::Array<byte> &params);
	void cmdRndDamage(Common::Array<byte> &params);
	void cmdMoveWallObj(Common::Array<byte> &params);
	void cmdAlterCellFlag(Common::Array<byte> &params);
	void cmdAlterHed(Common::Array<byte> &params);
	void cmdDisplayStat(Common::Array<byte> &params);
	void cmdSeatTextSml(Common::Array<byte> &params);
	void cmdPlayEventVoc(Common::Array<byte> &params);
	void cmdDisplayBottom(Common::Array<byte> &params);
	void cmdIfMapFlag(Common::Array<byte> &params);
	void cmdSelRndChar(Common::Array<byte> &params);
	void cmdGiveEnchanted(Common::Array<byte> &params);
	void cmdItemType(Common::Array<byte> &params);
	void cmdMakeNothingHere(Common::Array<byte> &params);
	void cmdNoAction2(Common::Array<byte> &params);
	void cmdChooseNumeric(Common::Array<byte> &params);
	void cmdDisplayBottomTwoLines(Common::Array<byte> &params);
	void cmdDisplayLarge(Common::Array<byte> &params);
	void cmdExchObj(Common::Array<byte> &params);
	void cmdFallToMap(Common::Array<byte> &params);
	void cmdDisplayMain(Common::Array<byte> &params);
	void cmdGoto(Common::Array<byte> &params);
	void cmdGotoRandom(Common::Array<byte> &params);
	void cmdCutsceneEndDarkside(Common::Array<byte> &params);
	void cmdCutsceneEdWorld(Common::Array<byte> &params);
	void cmdFlipWorld(Common::Array<byte> &params);
	void cmdPlayCD(Common::Array<byte> &params);

	int whoWill(int v1, int v2, int v3);

	void doEndGame();

	void doEndGame2();

	void doWorldEnd();

	void doEnding(const Common::String &endStr, int v2);
public:
	int _animCounter;
	bool _eventSkipped;
	int _whoWill;
public:
	Scripts(XeenEngine *vm);

	void checkEvents();

	void giveTreasure();

	void openGrate(int v1, int v2);
};

} // End of namespace Xeen

#endif /* XEEN_SCRIPTS_H */
