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
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v3.h"
#include "kyra/script.h"
#include "kyra/screen_v3.h"
#include "kyra/wsamovie.h"

#include "common/endian.h"

namespace Kyra {

int KyraEngine_v3::o3_setCharacterPos(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setCharacterPos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int x = stackPos(0);
	int y = stackPos(1);

	if (x != -1 && y != -1) {
		x &= ~3;
		y &= ~1;
	}

	_mainCharacter.x1 = _mainCharacter.x2 = x;
	_mainCharacter.y1 = _mainCharacter.y2 = y;

	return 0;
}

int KyraEngine_v3::o3_refreshCharacter(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_refreshCharacter(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const int frame = stackPos(0);
	const int facing = stackPos(1);
	const bool updateNeed = stackPos(2) != 0;

	if (facing >= 0)
		_mainCharacter.facing = facing;

	if (frame >= 0 && frame != 87)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	else
		_mainCharacter.animFrame = 87;

	updateCharacterAnim(0);

	if (updateNeed)
		refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_v3::o3_showSceneFileMessage(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_showSceneFileMessage(%p) (%d)", (const void *)script, stackPos(0));
	showMessage((const char*)getTableEntry(_scenesFile, stackPos(0)), 0xFF, 0xF0);
	return 0;
}

int KyraEngine_v3::o3_defineItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_defineItem(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int freeItem = findFreeItem();
	if (freeItem != -1) {
		_itemList[freeItem].id = stackPos(0);
		_itemList[freeItem].x = stackPos(1);
		_itemList[freeItem].y = stackPos(2);
		_itemList[freeItem].sceneId = stackPos(3);
	}
	return freeItem;
}

int KyraEngine_v3::o3_queryGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_queryGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	return queryGameFlag(stackPos(0));
}

int KyraEngine_v3::o3_resetGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_resetGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	resetGameFlag(stackPos(0));
	return 0;
}

int KyraEngine_v3::o3_setGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	setGameFlag(stackPos(0));
	return 1;
}

int KyraEngine_v3::o3_hideMouse(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_hideMouse(%p) ()", (const void *)script);
	_screen->hideMouse();
	return 0;
}

int KyraEngine_v3::o3_setMousePos(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setMousePos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setMousePos(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v3::o3_showMouse(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_showMouse(%p) ()", (const void *)script);
	_screen->showMouse();
	return 0;
}

int KyraEngine_v3::o3_delay(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_delay(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const uint32 delayTime = stackPos(0) * _tickLength;
	const int delayFunc = stackPos(1);

	if (delayFunc)
		warning("STUB o3_delay func 1");

	delay(delayTime, true);
	return 0;
}

int KyraEngine_v3::o3_setSceneFilename(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setSceneFilename(%p) (%d, '%s')", (const void *)script, stackPos(0), stackPosString(1));
	strcpy(_sceneList[stackPos(0)].filename1, stackPosString(1));
	_sceneList[stackPos(0)].filename1[9] = 0;
	return 0;
}

int KyraEngine_v3::o3_getRand(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getRand(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < stackPos(1));
	return _rnd.getRandomNumberRng(stackPos(0), stackPos(1));
}

int KyraEngine_v3::o3_defineRoomEntrance(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_defineRoomEntrance(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	switch (stackPos(0)) {
	case 0:
		_sceneEnterX1 = stackPos(1);
		_sceneEnterY1 = stackPos(2);
		break;

	case 1:
		_sceneEnterX2 = stackPos(1);
		_sceneEnterY2 = stackPos(2);
		break;

	case 2:
		_sceneEnterX3 = stackPos(1);
		_sceneEnterY3 = stackPos(2);
		break;

	case 3:
		_sceneEnterX4 = stackPos(1);
		_sceneEnterY4 = stackPos(2);
		break;

	default:
		break;
	}
	return 0;
}

int KyraEngine_v3::o3_setSpecialSceneScriptRunTime(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setSpecialSceneScriptRunTime(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) >= 0 && stackPos(0) < 10);
	_sceneSpecialScriptsTimer[stackPos(0)] = _system->getMillis() + stackPos(1) * _tickLength;
	return 0;
}

int KyraEngine_v3::o3_defineSceneAnim(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_defineSceneAnim(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s')",
		(const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7),
		stackPos(8), stackPos(9), stackPos(10), stackPos(11), stackPosString(12));
	const int animId = stackPos(0);
	SceneAnim &anim = _sceneAnims[animId];

	musicUpdate(0);

	uint16 flags = anim.flags = stackPos(1);
	int x = anim.x = stackPos(2);
	int y = anim.y = stackPos(3);
	int x2 = anim.x2 = stackPos(4);
	int y2 = anim.y2 = stackPos(5);
	int w = anim.width = stackPos(6);
	int h = anim.height = stackPos(7);
	anim.unk10 = stackPos(8);
	anim.specialSize = stackPos(9);
	anim.unk14 = stackPos(10);
	anim.shapeIndex = stackPos(11);
	const char *filename = stackPosString(12);

	if (filename)
		strcpy(anim.filename, filename);

	if (flags & 8) {
		_sceneAnimMovie[animId]->open(filename, 1, 0);
		musicUpdate(0);
		if (_sceneAnimMovie[animId]->opened()) {
			anim.wsaFlag = 1;
			if (x2 == -1)
				x2 = _sceneAnimMovie[animId]->xAdd();
			if (y2 == -1)
				y2 = _sceneAnimMovie[animId]->yAdd();
			if (w == -1)
				w = _sceneAnimMovie[animId]->width();
			if (h == -1)
				h = _sceneAnimMovie[animId]->height();
			if (x == -1)
				x = (w >> 1) + x2;
			if (y == -1)
				y = y2 + h - 1;

			anim.x = x;
			anim.y = y;
			anim.x2 = x2;
			anim.y2 = y2;
			anim.width = w;
			anim.height = h;
		}
	}

	musicUpdate(0);

	return 9;
}

int KyraEngine_v3::o3_updateSceneAnim(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_updateSceneAnim(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	updateSceneAnim(stackPos(0), stackPos(1));
	_specialSceneScriptRunFlag = false;
	return 0;
}

int KyraEngine_v3::o3_defineScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_defineScene(%p) (%d, '%s', %d, %d, %d, %d, %d, %d)",
		(const void *)script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	const int scene = stackPos(0);
	strcpy(_sceneList[scene].filename1, stackPosString(1));
	_sceneList[scene].filename1[9] = 0;
	strcpy(_sceneList[scene].filename2, stackPosString(1));
	_sceneList[scene].filename2[9] = 0;

	_sceneList[scene].exit1 = stackPos(2);
	_sceneList[scene].exit2 = stackPos(3);
	_sceneList[scene].exit3 = stackPos(4);
	_sceneList[scene].exit4 = stackPos(5);
	_sceneList[scene].flags = stackPos(6);
	_sceneList[scene].sound = stackPos(7);

	if (_mainCharacter.sceneId == scene) {
		_sceneExit1 = _sceneList[scene].exit1;
		_sceneExit2 = _sceneList[scene].exit2;
		_sceneExit3 = _sceneList[scene].exit3;
		_sceneExit4 = _sceneList[scene].exit4;
	}

	return 0;
}

int KyraEngine_v3::o3_setSpecialSceneScriptState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 1;
	return 1;
}

int KyraEngine_v3::o3_clearSpecialSceneScriptState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_clearSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 0;
	return 0;
}

int KyraEngine_v3::o3_querySpecialSceneScriptState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_querySpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	return _specialSceneScriptState[stackPos(0)];
}

int KyraEngine_v3::o3_setHiddenItemsEntry(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setHiddenItemsEntry(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return (_hiddenItems[stackPos(0)] = (uint16)stackPos(1));
}

int KyraEngine_v3::o3_getHiddenItemsEntry(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getHiddenItemsEntry(%p) (%d)", (const void *)script, stackPos(0));
	return (int16)_hiddenItems[stackPos(0)];
}

int KyraEngine_v3::o3_removeSceneAnimObject(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_removeSceneAnimObject(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	removeSceneAnimObject(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v3::o3_dummy(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_dummy(%p) ()", (const void *)script);
	return 0;
}

typedef Functor1Mem<ScriptState*, int, KyraEngine_v3> OpcodeV3;
#define Opcode(x) OpcodeV3(this, &KyraEngine_v3::x)
#define OpcodeUnImpl() OpcodeV3(this, 0)
void KyraEngine_v3::setupOpcodeTable() {
	static const OpcodeV3 opcodeTable[] = {
		// 0x00
		OpcodeUnImpl(),
		Opcode(o3_setCharacterPos),
		OpcodeUnImpl(),
		Opcode(o3_refreshCharacter),
		// 0x04
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x08
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		// 0x0c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x10
		OpcodeUnImpl(),
		Opcode(o3_showSceneFileMessage),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		// 0x14
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		// 0x18
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x1c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x20
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		Opcode(o3_defineItem),
		// 0x24
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_queryGameFlag),
		// 0x28
		Opcode(o3_resetGameFlag),
		Opcode(o3_setGameFlag),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x2c
		OpcodeUnImpl(),
		Opcode(o3_hideMouse),
		OpcodeUnImpl(),
		Opcode(o3_setMousePos),
		// 0x30
		Opcode(o3_showMouse),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		// 0x34
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		Opcode(o3_delay),
		// 0x38
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_setSceneFilename),
		OpcodeUnImpl(),
		// 0x3c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x40
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		// 0x44
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		// 0x48
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x4c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x50
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		// 0x54
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x58
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x5c
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x60
		Opcode(o3_getRand),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x64
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		// 0x68
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x6c
		Opcode(o3_dummy),
		Opcode(o3_defineRoomEntrance),
		OpcodeUnImpl(),
		Opcode(o3_setSpecialSceneScriptRunTime),
		// 0x70
		Opcode(o3_defineSceneAnim),
		Opcode(o3_dummy),
		Opcode(o3_updateSceneAnim),
		Opcode(o3_dummy),
		// 0x74
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x78
		OpcodeUnImpl(),
		Opcode(o3_defineScene),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x7c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		// 0x80
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		// 0x84
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		// 0x88
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		// 0x8c
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		Opcode(o3_setSpecialSceneScriptState),
		// 0x90
		Opcode(o3_clearSpecialSceneScriptState),
		Opcode(o3_querySpecialSceneScriptState),
		Opcode(o3_dummy),
		Opcode(o3_setHiddenItemsEntry),
		// 0x94
		Opcode(o3_getHiddenItemsEntry),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		// 0x98
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o3_removeSceneAnimObject),
		// 0x9c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0xa0
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		Opcode(o3_dummy),
		// 0xa4
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0xa8
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0xac
		OpcodeUnImpl(),
		Opcode(o3_dummy),
		OpcodeUnImpl(),
		Opcode(o3_dummy),
	};

	for (int i = 0; i < ARRAYSIZE(opcodeTable); ++i)
		_opcodes.push_back(&opcodeTable[i]);
}

} // end of namespace Kyra
