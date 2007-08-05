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

#include "kyra/kyra_v2.h"
#include "kyra/wsamovie.h"

#include "common/endian.h"

namespace Kyra {

int KyraEngine_v2::o2_setCharacterFacingRefresh(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_setCharacterFacingRefresh(%p) (-, %d, %d)", (const void *)script, stackPos(1), stackPos(2));
	int animFrame = stackPos(2);
	if (animFrame >= 0)
		_mainCharacter.animFrame = animFrame;
	_mainCharacter.facing = stackPos(1);
	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_v2::o2_defineObject(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_defineObject(%p) (%d, '%s', %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	Object *object = &_objectList[stackPos(0)];
	strcpy(object->filename, stackPosString(1));
	object->scriptId = stackPos(2);
	object->x = stackPos(3);
	object->y = stackPos(4);
	object->unk12 = stackPos(5);
	return 0;
}

int KyraEngine_v2::o2_refreshCharacter(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_refreshCharacter(%p) (-, %d, %d, %d)", (const void *)script, stackPos(1), stackPos(2), stackPos(3));
	int unk = stackPos(1);
	int facing = stackPos(2);
	int refresh = stackPos(3);
	if (facing >= 0)
		_mainCharacter.facing = facing;
	if (unk >= 0 && unk != 32)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);
	if (refresh)
		refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_v2::o2_getCharacterX(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_getCharacterX(%p) ()", (const void *)script);
	return _mainCharacter.x1;
}

int KyraEngine_v2::o2_getCharacterY(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_getCharacterY(%p) ()", (const void *)script);
	return _mainCharacter.y1;
}

int KyraEngine_v2::o2_getCharacterFacing(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_getCharacterFacing(%p) ()", (const void *)script);
	return _mainCharacter.facing;
}

int KyraEngine_v2::o2_setSceneComment(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_setSceneComment(%p) ('%s')", (const void *)script, stackPosString(0));
	_sceneCommentString = stackPosString(0);
	return 0;
}

int KyraEngine_v2::o2_showChapterMessage(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_showChapterMessage(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	showChapterMessage(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v2::o2_wsaClose(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_wsaClose(%p) (%d)", (const void *)script, stackPos(0));
	assert(stackPos(0) >= 0 && stackPos(0) < ARRAYSIZE(_wsaSlots));
	_wsaSlots[stackPos(0)]->close();
	return 0;
}

int KyraEngine_v2::o2_displayWsaFrame(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_displayWsaFrame(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8));
	int frame = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int waitTime = stackPos(3);
	int slot = stackPos(4);
	int copyParam = stackPos(5);
	int doUpdate = stackPos(6);
	int dstPage = stackPos(7);
	int backUp = stackPos(8);
	
	_screen->hideMouse();
	uint32 endTime = _system->getMillis() + waitTime * _tickLength;
	_wsaSlots[slot]->setX(x);
	_wsaSlots[slot]->setY(y);
	_wsaSlots[slot]->setDrawPage(dstPage);
	_wsaSlots[slot]->displayFrame(frame, copyParam | 0xC000);
	_screen->updateScreen();
	
	if (backUp)
		memcpy(_gamePlayBuffer, _screen->getCPagePtr(3), 46080);

	while (_system->getMillis() < endTime) {
		if (doUpdate)
			update();
		
		if (endTime - _system->getMillis() >= 10)
			delay(10);
	}
	_screen->showMouse();	
	return 0;
}

int KyraEngine_v2::o2_displayWsaSequentialFrames(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_displayWsaSequentialFrames(%p) (%d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	int startFrame = stackPos(0);
	int endFrame = stackPos(1);
	int x = stackPos(2);
	int y = stackPos(3);
	int waitTime = stackPos(4);
	int slot = stackPos(5);
	int maxTimes = stackPos(6);
	int copyFlags = stackPos(7);
	
	if (maxTimes > 1)
		maxTimes = 1;
	
	_wsaSlots[slot]->setX(x);
	_wsaSlots[slot]->setY(y);
	_wsaSlots[slot]->setDrawPage(0);
	
	_screen->hideMouse();
	int curTime = 0;
	while (curTime < maxTimes) {
		if (startFrame < endFrame) {
			for (int i = startFrame; i <= endFrame; ++i) {
				uint32 endTime = _system->getMillis() + waitTime * _tickLength;
				_wsaSlots[slot]->displayFrame(i, 0xC000 | copyFlags);
				_screen->updateScreen();
				
				do {
					update();

					if (endTime - _system->getMillis() >= 10)
						delay(10);
				} while (_system->getMillis() < endTime);
			}
		} else {
			for (int i = startFrame; i >= endFrame; --i) {
				uint32 endTime = _system->getMillis() + waitTime * _tickLength;
				_wsaSlots[slot]->displayFrame(i, 0xC000 | copyFlags);
				_screen->updateScreen();

				do {
					update();

					if (endTime - _system->getMillis() >= 10)
						delay(10);
				} while (_system->getMillis() < endTime);
			}
		}

		++curTime;
	}
	_screen->showMouse();
	return 0;
}

int KyraEngine_v2::o2_wsaOpen(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_wsaOpen(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	assert(stackPos(1) >= 0 && stackPos(1) < ARRAYSIZE(_wsaSlots));
	_wsaSlots[stackPos(1)]->open(stackPosString(0), 1, 0);
	return 0;
}

int KyraEngine_v2::o2_defineItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_defineItem(%p) (%d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int freeItem = findFreeItem();
	
	if (freeItem >= 0) {
		_itemList[freeItem].id = stackPos(0);
		_itemList[freeItem].x = stackPos(1);
		_itemList[freeItem].y = stackPos(2);
		_itemList[freeItem].sceneId = stackPos(3);
	}

	return freeItem;
}

int KyraEngine_v2::o2_queryGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_queryGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	return queryGameFlag(stackPos(0));
}

int KyraEngine_v2::o2_resetGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_resetGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	return resetGameFlag(stackPos(0));
}

int KyraEngine_v2::o2_setGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_setGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	return setGameFlag(stackPos(0));
}

int KyraEngine_v2::o2_hideMouse(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_hideMouse(%p) ()", (const void *)script);
	_screen->hideMouse();
	return 0;
}

int KyraEngine_v2::o2_addSpecialExit(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_addSpecialExit(%p) (%d, %d, %d, %d, %d)", (const void *)script,
		stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	if (_specialExitCount < 5) {
		_specialExitTable[_specialExitCount+0] = stackPos(0);
		_specialExitTable[_specialExitCount+5] = stackPos(1);
		_specialExitTable[_specialExitCount+10] = stackPos(2);
		_specialExitTable[_specialExitCount+15] = stackPos(3);
		_specialExitTable[_specialExitCount+20] = stackPos(4);
		++_specialExitCount;
	}
	return 0;
}

int KyraEngine_v2::o2_showMouse(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_showMouse(%p) ()", (const void *)script);
	_screen->showMouse();
	return 0;
}

int KyraEngine_v2::o2_setScaleTableItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_setScaleTableItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setScaleTableItem(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v2::o2_setDrawLayerTableItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_setDrawLayerTableItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setDrawLayerTableEntry(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v2::o2_drawSceneShapeOnPage(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_drawSceneShapeOnPage(%p) (%d, %d, %d, %d, %d)", (const void *)script,
		stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int shape = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int flag = stackPos(3);
	int drawPage = stackPos(4);
	_screen->drawShape(drawPage, _sceneShapeTable[shape], x, y, 2, flag ? 1 : 0);
	return 0;
}

int KyraEngine_v2::o2_restoreBackBuffer(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_restoreBackBuffer(%p) (%d)", (const void *)script, stackPos(0));
	int disable = stackPos(0);
	int oldState = 0;
	if (disable) {
		oldState = _animObjects[0].enabled;
		_animObjects[0].enabled = 0;
	}
	restorePage3();
	if (disable)
		_animObjects[0].enabled = oldState;
	return 0;
}

int KyraEngine_v2::o2_getRand(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_getRand(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < stackPos(1));
	return _rnd.getRandomNumberRng(stackPos(0), stackPos(1));
}

int KyraEngine_v2::o2_encodeShape(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_encodeShape(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1),
		stackPos(2), stackPos(3), stackPos(4));
	_sceneShapeTable[stackPos(0)] = _screen->encodeShape(stackPos(1), stackPos(2), stackPos(3), stackPos(4), 2);
	return 0;
}

int KyraEngine_v2::o2_defineRoomEntrance(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_defineRoomEntrance(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
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

int KyraEngine_v2::o2_setSpecialSceneScriptRunTime(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_setSpecialSceneScriptRunTime(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) >= 0 && stackPos(0) < 10);
	_sceneSpecialScriptsTimer[stackPos(0)] = _system->getMillis() + stackPos(1) * _tickLength;
	return 0;
}

int KyraEngine_v2::o2_defineSceneAnim(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_defineSceneAnim(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s')", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8),
			stackPos(9), stackPos(10), stackPos(11), stackPosString(12));
	int animId = stackPos(0);
	SceneAnim &anim = _sceneAnims[animId];
	anim.flags = stackPos(1);
	anim.x = stackPos(2);
	anim.y = stackPos(3);
	anim.x2 = stackPos(4);
	anim.y2 = stackPos(5);
	anim.width = stackPos(6);
	anim.height = stackPos(7);
	anim.unkE = stackPos(8);
	anim.specialSize = stackPos(9);
	anim.unk12 = stackPos(10);
	anim.shapeIndex = stackPos(11);
	if (stackPosString(12) != 0)
		strcpy(anim.filename, stackPosString(12));
	
	if (anim.flags & 0x40) {
		if (!_sceneAnimMovie[animId]->open(anim.filename, 1, 0))
			error("couldn't load '%s'", anim.filename);
	
		if (_sceneAnimMovie[animId]->xAdd() || _sceneAnimMovie[animId]->yAdd())
			anim.wsaFlag = 1;
		else
			anim.wsaFlag = 0;
	}
	
	return 0;
}

int KyraEngine_v2::o2_updateSceneAnim(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_updateSceneAnim(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	updateSceneAnim(stackPos(0), stackPos(1));
	_specialSceneScriptRunFlag = false;
	return 0;
}

int KyraEngine_v2::o2_defineRoom(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_defineRoom(%p) (%d, '%s', %d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	SceneDesc *scene = &_sceneList[stackPos(0)];
	strcpy(scene->filename, stackPosString(1));
	scene->exit1 = stackPos(2);
	scene->exit2 = stackPos(3);
	scene->exit3 = stackPos(4);
	scene->exit4 = stackPos(5);
	scene->flags = stackPos(6);
	scene->sound = stackPos(7);
	
	if (_mainCharacter.sceneId == stackPos(0)) {
		_sceneExit1 = scene->exit1;
		_sceneExit2 = scene->exit2;
		_sceneExit3 = scene->exit3;
		_sceneExit4 = scene->exit4;
	}

	return 0;
}

int KyraEngine_v2::o2_setSpecialSceneScriptState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_setSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 1;
	return 1;
}

int KyraEngine_v2::o2_clearSpecialSceneScriptState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_clearSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 0;
	return 0;
}

int KyraEngine_v2::o2_querySpecialSceneScriptState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_querySpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	return _specialSceneScriptState[stackPos(0)];
}

int KyraEngine_v2::o2_dummy(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o2_dummy(%p) ()", (const void *)script);
	return 0;
}

} // end of namespace Kyra
