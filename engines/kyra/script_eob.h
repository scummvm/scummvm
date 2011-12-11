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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifdef ENABLE_EOB

#ifndef KYRA_SCRIPT_EOB_H
#define KYRA_SCRIPT_EOB_H

#include "common/func.h"
#include "common/substream.h"
#include "common/savefile.h"

namespace Kyra {

class LolEobBaseEngine;

class EobInfProcessor {
public:
	EobInfProcessor(EobCoreEngine *engine, Screen_Eob *_screen);
	~EobInfProcessor();

	void loadData(const uint8 *data, uint32 dataSize);
	void run(int func, int sub);

	void setFlag(int flag) { _flagTable[17] |= flag; }
	bool checkFlag(int flag) { return (_flagTable[17] & flag) ? true : false; }

	void loadState(Common::SeekableSubReadStreamEndian &in);
	void saveState(Common::OutSaveFile *out);

private:
	const char *getString(uint16 index);

	int oeob_setWallType(int8 *data);
	int oeob_toggleWallState(int8 *data);
	int oeob_openDoor(int8 *data);
	int oeob_closeDoor(int8 *data);
	int oeob_replaceMonster(int8 *data);
	int oeob_movePartyOrObject(int8 *data);
	int oeob_moveInventoryItemToBlock(int8 *data);
	int oeob_printMessage_v1(int8 *data);
	int oeob_printMessage_v2(int8 *data);
	int oeob_setFlags(int8 *data);
	int oeob_playSoundEffect(int8 *data);
	int oeob_removeFlags(int8 *data);
	int oeob_modifyCharacterHitPoints(int8 *data);
	int oeob_calcAndInflictCharacterDamage(int8 *data);
	int oeob_jump(int8 *data);
	int oeob_end(int8 *data);
	int oeob_popPosAndReturn(int8 *data);
	int oeob_pushPosAndJump(int8 *data);
	int oeob_eval_v1(int8 *data);
	int oeob_eval_v2(int8 *data);
	int oeob_deleteItem(int8 *data);
	int oeob_loadNewLevelOrMonsters(int8 *data);
	int oeob_increasePartyExperience(int8 *data);
	int oeob_createItem_v1(int8 *data);
	int oeob_createItem_v2(int8 *data);
	int oeob_launchObject(int8 *data);
	int oeob_changeDirection(int8 *data);
	int oeob_identifyItems(int8 *data);
	int oeob_sequence(int8 *data);
	int oeob_delay(int8 *data);
	int oeob_drawScene(int8 *data);
	int oeob_dialogue(int8 *data);
	int oeob_specialEvent(int8 *data);

	EobCoreEngine *_vm;
	Screen_Eob *_screen;

	typedef Common::Functor1Mem<int8*, int, EobInfProcessor> InfProc;
	Common::Array<const InfProc*> _opcodes;

	int8 *_scriptData;

	uint8 _abortScript;
	uint16 _abortAfterSubroutine;
	int _dlgResult;
	uint8 _script2;

	uint16 _lastScriptFunc;
	uint16 _lastScriptSub;

	int8 **_scriptPosStack;
	int _scriptPosStackIndex;

	uint32 *_flagTable;

	int16 *_stack;
	int _stackIndex;

	int8 _activeCharacter;

	const int _commandMin;
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB
