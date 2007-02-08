/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/endian.h"
#include "kyra/kyra.h"
#include "kyra/script.h"
#include "kyra/screen.h"
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"
#include "kyra/animator.h"
#include "kyra/text.h"
#include "common/system.h"

namespace Kyra {
#define stackPos(x) script->stack[script->sp+x]
#define stackPosString(x) (char*)&script->dataPtr->text[READ_BE_UINT16(&((uint16 *)script->dataPtr->text)[stackPos(x)])]

int KyraEngine::o1_magicInMouseItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_magicInMouseItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	magicInMouseItem(stackPos(0), stackPos(1), -1);
	return 0;
}

int KyraEngine::o1_characterSays(ScriptState *script) {
	_skipFlag = false;
	if (_flags.isTalkie) {
		debugC(3, kDebugLevelScriptFuncs, "o1_characterSays(%p) (%d, '%s', %d, %d)", (const void *)script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3));
		characterSays(stackPos(0), stackPosString(1), stackPos(2), stackPos(3));
	} else {
		debugC(3, kDebugLevelScriptFuncs, "o1_characterSays(%p) ('%s', %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
		const char *string = stackPosString(0);

		if (_flags.platform == Common::kPlatformFMTowns && _flags.lang == Common::JA_JPN) {
			static const uint8 townsString1[] = {
				0x83, 0x75, 0x83, 0x89, 0x83, 0x93, 0x83, 0x83, 0x93, 0x81,
				0x41, 0x82, 0xDC, 0x82, 0xBD, 0x97, 0x88, 0x82, 0xBD, 0x82,
				0xCC, 0x82, 0xA9, 0x81, 0x48, 0x00, 0x00, 0x00
			};
			static const uint8 townsString2[] = {
				0x83, 0x75, 0x83, 0x89, 0x83, 0x93, 0x83, 0x5C, 0x83, 0x93,
				0x81, 0x41, 0x82, 0xDC, 0x82, 0xBD, 0x97, 0x88, 0x82, 0xBD,
				0x82, 0xCC, 0x82, 0xA9, 0x81, 0x48, 0x00, 0x00
			};

			if (strncmp((const char *)townsString1, string, sizeof(townsString1)) == 0)
				string = (const char *)townsString2;
		}

		characterSays(-1, string, stackPos(1), stackPos(2));
	}

	return 0;
}

int KyraEngine::o1_pauseTicks(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_pauseTicks(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(1)) {
		warning("STUB: special o1_pauseTicks");
		// delete this after correct implementing
		delayWithTicks(stackPos(0));
	} else {
		delayWithTicks(stackPos(0));
	}
	return 0;
}

int KyraEngine::o1_drawSceneAnimShape(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_drawSceneAnimShape(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->drawShape(stackPos(4), _sprites->_sceneShapes[stackPos(0)], stackPos(1), stackPos(2), 0, (stackPos(3) != 0) ? 1 : 0);
	return 0;
}

int KyraEngine::o1_queryGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_queryGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return queryGameFlag(stackPos(0));
}

int KyraEngine::o1_setGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return setGameFlag(stackPos(0));
}

int KyraEngine::o1_resetGameFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_resetGameFlag(%p) (0x%X)", (const void *)script, stackPos(0));
	return resetGameFlag(stackPos(0));
}

int KyraEngine::o1_runNPCScript(ScriptState *script) {
	warning("STUB: o1_runNPCScript");
	return 0;
}

int KyraEngine::o1_setSpecialExitList(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setSpecialExitList(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	
	for (int i = 0; i < 10; ++i) {
		_exitList[i] = stackPos(i);
	}
	_exitListPtr = _exitList;
	
	return 0;
}

int KyraEngine::o1_blockInWalkableRegion(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_blockInWalkableRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_screen->blockInRegion(stackPos(0), stackPos(1), stackPos(2)-stackPos(0)+1, stackPos(3)-stackPos(1)+1);
	return 0;
}

int KyraEngine::o1_blockOutWalkableRegion(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_blockOutWalkableRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_screen->blockOutRegion(stackPos(0), stackPos(1), stackPos(2)-stackPos(0)+1, stackPos(3)-stackPos(1)+1);
	return 0;
}

int KyraEngine::o1_walkPlayerToPoint(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_walkPlayerToPoint(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	int normalTimers = stackPos(2);
	if (!normalTimers) {
		disableTimer(19);
		disableTimer(14);
		disableTimer(18);
	}

	int reinitScript = handleSceneChange(stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	if (!normalTimers) {
		enableTimer(19);
		enableTimer(14);
		enableTimer(18);
	}

	if (reinitScript) {
		_scriptInterpreter->initScript(script, script->dataPtr);
	}

	if (_sceneChangeState) {
		_sceneChangeState = 0;
		return 1;
	}
	return 0;
}

int KyraEngine::o1_dropItemInScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_dropItemInScene(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int item = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	
	byte freeItem = findFreeItemInScene(_currentCharacter->sceneId);
	if (freeItem != 0xFF) {
		int sceneId = _currentCharacter->sceneId;
		Room *room = &_roomTable[sceneId];
		room->itemsXPos[freeItem] = xpos;
		room->itemsYPos[freeItem] = ypos;
		room->itemsTable[freeItem] = item;
		
		_animator->animAddGameItem(freeItem, sceneId);
		_animator->updateAllObjectShapes();
	} else {
		if (item == 43) {
			placeItemInGenericMapScene(item, 0);
		} else {
			placeItemInGenericMapScene(item, 1);
		}
	}
	return 0;
}

int KyraEngine::o1_drawAnimShapeIntoScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_drawAnimShapeIntoScene(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_screen->hideMouse();
	_animator->restoreAllObjectBackgrounds();
	int shape = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int flags = (stackPos(3) != 0) ? 1 : 0;
	_screen->drawShape(2, _sprites->_sceneShapes[shape], xpos, ypos, 0, flags);
	_screen->drawShape(0, _sprites->_sceneShapes[shape], xpos, ypos, 0, flags);
	_animator->flagAllObjectsForBkgdChange();
	_animator->preserveAnyChangedBackgrounds();
	_animator->flagAllObjectsForRefresh();
	_animator->updateAllObjectShapes();
	_screen->showMouse();
	return 0;
}

int KyraEngine::o1_createMouseItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_createMouseItem(%p) (%d)", (const void *)script, stackPos(0));
	createMouseItem(stackPos(0));
	return 0;
}

int KyraEngine::o1_savePageToDisk(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_savePageToDisk(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	_screen->savePageToDisk(stackPosString(0), stackPos(1));
	return 0;
}

int KyraEngine::o1_sceneAnimOn(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_sceneAnimOn(%p) (%d)", (const void *)script, stackPos(0));
	_sprites->_anims[stackPos(0)].play = true;
	return 0;
}

int KyraEngine::o1_sceneAnimOff(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_sceneAnimOff(%p) (%d)", (const void *)script, stackPos(0));
	_sprites->_anims[stackPos(0)].play = false;
	return 0;
}

int KyraEngine::o1_getElapsedSeconds(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getElapsedSeconds(%p) ()", (const void *)script);
	return _system->getMillis() / 1000;
}

int KyraEngine::o1_mouseIsPointer(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_mouseIsPointer(%p) ()", (const void *)script);
	if (_itemInHand == -1) {
		return 1;
	}
	return 0;
}

int KyraEngine::o1_destroyMouseItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_destroyMouseItem(%p) ()", (const void *)script);
	destroyMouseItem();
	return 0;
}

int KyraEngine::o1_runSceneAnimUntilDone(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_runSceneAnimUntilDone(%p) (%d)", (const void *)script, stackPos(0));
	_screen->hideMouse();
	_animator->restoreAllObjectBackgrounds();
	_sprites->_anims[stackPos(0)].play = true;
	_animator->sprites()[stackPos(0)].active = 1;
	_animator->flagAllObjectsForBkgdChange();
	_animator->preserveAnyChangedBackgrounds();
	while (_sprites->_anims[stackPos(0)].play) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		delay(10);
	}
	_animator->restoreAllObjectBackgrounds();
	_screen->showMouse();
	return 0;
}

int KyraEngine::o1_fadeSpecialPalette(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_fadeSpecialPalette(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_screen->fadeSpecialPalette(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int KyraEngine::o1_playAdlibSound(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_playAdlibSound(%p) (%d)", (const void *)script, stackPos(0));
	snd_playSoundEffect(stackPos(0));
	return 0;
}

int KyraEngine::o1_playAdlibScore(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_playAdlibScore(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	snd_playWanderScoreViaMap(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::o1_phaseInSameScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_phaseInSameScene(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	transcendScenes(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::o1_setScenePhasingFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setScenePhasingFlag(%p) ()", (const void *)script);
	_scenePhasingFlag = 1;
	return 1;
}

int KyraEngine::o1_resetScenePhasingFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_resetScenePhasingFlag(%p) ()", (const void *)script);
	_scenePhasingFlag = 0;
	return 0;
}

int KyraEngine::o1_queryScenePhasingFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_queryScenePhasingFlag(%p) ()", (const void *)script);
	return _scenePhasingFlag;
}

int KyraEngine::o1_sceneToDirection(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_sceneToDirection(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < _roomTableSize);
	Room *curRoom = &_roomTable[stackPos(0)];
	uint16 returnValue = 0xFFFF;
	switch (stackPos(1)) {
	case 0:
		returnValue = curRoom->northExit;
		break;

	case 2:
		returnValue = curRoom->eastExit;
		break;

	case 4:
		returnValue = curRoom->southExit;
		break;

	case 6:
		returnValue = curRoom->westExit;
		break;

	default:
		break;
	}
	if (returnValue == 0xFFFF)
		return -1;
	return returnValue;
}

int KyraEngine::o1_setBirthstoneGem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setBirthstoneGem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int index = stackPos(0);
	if (index < 4 && index >= 0) {
		_birthstoneGemTable[index] = stackPos(1);
		return 1;
	}
	return 0;
}

int KyraEngine::o1_placeItemInGenericMapScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_placeItemInGenericMapScene(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	placeItemInGenericMapScene(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::o1_setBrandonStatusBit(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setBrandonStatusBit(%p) (%d)", (const void *)script, stackPos(0));
	_brandonStatusBit |= stackPos(0);
	return 0;
}

int KyraEngine::o1_pauseSeconds(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_pauseSeconds(%p) (%d)", (const void *)script, stackPos(0));
	if (stackPos(0) > 0 && !_skipFlag)
		delay(stackPos(0)*1000, true);
	_skipFlag = false;
	return 0;
}

int KyraEngine::o1_getCharactersLocation(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getCharactersLocation(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].sceneId;
}

int KyraEngine::o1_runNPCSubscript(ScriptState *script) {
	warning("STUB: o1_runNPCSubscript");
	return 0;
}

int KyraEngine::o1_magicOutMouseItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_magicOutMouseItem(%p) (%d)", (const void *)script, stackPos(0));
	magicOutMouseItem(stackPos(0), -1);
	return 0;
}

int KyraEngine::o1_internalAnimOn(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_internalAnimOn(%p) (%d)", (const void *)script, stackPos(0));
	_animator->sprites()[stackPos(0)].active = 1;
	return 0;
}

int KyraEngine::o1_forceBrandonToNormal(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_forceBrandonToNormal(%p) ()", (const void *)script);
	checkAmuletAnimFlags();
	return 0;
}

int KyraEngine::o1_poisonDeathNow(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_poisonDeathNow(%p) ()", (const void *)script);
	seq_poisonDeathNow(1);
	return 0;
}

int KyraEngine::o1_setScaleMode(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setScaleMode(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int len = stackPos(0);
	int setValue1 = stackPos(1);
	int start2 = stackPos(2);
	int setValue2 = stackPos(3);
	for (int i = 0; i < len; ++i) {
		_scaleTable[i] = setValue1;
	}
	int temp = setValue2 - setValue1;
	int temp2 = start2 - len;
	for (int i = len, offset = 0; i < start2; ++i, ++offset) {
		_scaleTable[i] = (offset * temp) / temp2 + setValue1;
	}
	for (int i = start2; i < 145; ++i) {
		_scaleTable[i] = setValue2;
	}
	_scaleMode = 1;
	return _scaleMode;
}

int KyraEngine::o1_openWSAFile(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_openWSAFile(%p) ('%s', %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3));
	
	char *filename = stackPosString(0);
	int wsaIndex = stackPos(1);
	
	_movieObjects[wsaIndex]->open(filename, (stackPos(3) != 0) ? 1 : 0, 0);
	assert(_movieObjects[wsaIndex]->opened());
	
	return 0;
}

int KyraEngine::o1_closeWSAFile(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_closeWSAFile(%p) (%d)", (const void *)script, stackPos(0));
	
	int wsaIndex = stackPos(0);
	if (_movieObjects[wsaIndex]) {
		_movieObjects[wsaIndex]->close();
	}
	
	return 0;
}

int KyraEngine::o1_runWSAFromBeginningToEnd(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_runWSAFromBeginningToEnd(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	
	_screen->hideMouse();
	
	bool running = true;
	
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int waitTime = stackPos(2);
	int wsaIndex = stackPos(3);
	int worldUpdate = stackPos(4);
	int wsaFrame = 0;
	
	_movieObjects[wsaIndex]->setX(xpos);
	_movieObjects[wsaIndex]->setY(ypos);
	_movieObjects[wsaIndex]->setDrawPage(0);
	while (running) {
		_movieObjects[wsaIndex]->displayFrame(wsaFrame++);
		_animator->_updateScreen = true;
		if (wsaFrame >= _movieObjects[wsaIndex]->frames())
			running = false;
		
		uint32 continueTime = waitTime * _tickLength + _system->getMillis();
		while (_system->getMillis() < continueTime) {
			if (worldUpdate) {
				_sprites->updateSceneAnims();
				_animator->updateAllObjectShapes();
			} else {
				_screen->updateScreen();
			}
			if (continueTime - _system->getMillis() >= 10)
				delay(10);
		}
	}
	
	_screen->showMouse();
	
	return 0;
}

int KyraEngine::o1_displayWSAFrame(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_displayWSAFrame(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int frame = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int waitTime = stackPos(3);
	int wsaIndex = stackPos(4);
	_screen->hideMouse();
	_movieObjects[wsaIndex]->setX(xpos);
	_movieObjects[wsaIndex]->setY(ypos);
	_movieObjects[wsaIndex]->setDrawPage(0);
	_movieObjects[wsaIndex]->displayFrame(frame);
	_animator->_updateScreen = true;
	uint32 continueTime = waitTime * _tickLength + _system->getMillis();
	while (_system->getMillis() < continueTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		if (_skipFlag)
			break;

		if (continueTime - _system->getMillis() >= 10)
			delay(10);
	}
	_screen->showMouse();
	return 0;
}

int KyraEngine::o1_enterNewScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_enterNewScene(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	enterNewScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return 0;
}

int KyraEngine::o1_setSpecialEnterXAndY(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setSpecialEnterXAndY(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_brandonPosX = stackPos(0);
	_brandonPosY = stackPos(1);
	if (_brandonPosX + 1 == 0 && _brandonPosY + 1 == 0)
		_currentCharacter->currentAnimFrame = 88;
	return 0;
}

int KyraEngine::o1_runWSAFrames(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_runWSAFrames(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int delayTime = stackPos(2);
	int startFrame = stackPos(3);
	int endFrame = stackPos(4);
	int wsaIndex = stackPos(5);
	_screen->hideMouse();
	_movieObjects[wsaIndex]->setX(xpos);
	_movieObjects[wsaIndex]->setY(ypos);
	_movieObjects[wsaIndex]->setDrawPage(0);
	for (; startFrame <= endFrame; ++startFrame) {
		uint32 nextRun = _system->getMillis() + delayTime * _tickLength;
		_movieObjects[wsaIndex]->displayFrame(startFrame);
		_animator->_updateScreen = true;
		while (_system->getMillis() < nextRun) {
			_sprites->updateSceneAnims();
			_animator->updateAllObjectShapes();
			if (nextRun - _system->getMillis() >= 10)
				delay(10);
		}
	}
	_screen->showMouse();
	return 0;
}

int KyraEngine::o1_popBrandonIntoScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_popBrandonIntoScene(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int changeScaleMode = stackPos(3);
	int xpos = (int16)(stackPos(0) & 0xFFFC);
	int ypos = (int16)(stackPos(1) & 0xFFFE);
	int facing = stackPos(2);
	_currentCharacter->x1 = _currentCharacter->x2 = xpos;
	_currentCharacter->y1 = _currentCharacter->y2 = ypos;
	_currentCharacter->facing = facing;
	_currentCharacter->currentAnimFrame = 7;
	int xOffset = _defaultShapeTable[0].xOffset;
	int yOffset = _defaultShapeTable[0].yOffset;
	int width = _defaultShapeTable[0].w << 3;
	int height = _defaultShapeTable[0].h;
	AnimObject *curAnim = _animator->actors();
	
	if (changeScaleMode) {
		curAnim->x1 = _currentCharacter->x1;
		curAnim->y1 = _currentCharacter->y1;
		_animator->_brandonScaleY = _scaleTable[_currentCharacter->y1];
		_animator->_brandonScaleX = _animator->_brandonScaleY;
		
		int animWidth = _animator->fetchAnimWidth(curAnim->sceneAnimPtr, _animator->_brandonScaleX) >> 1;
		int animHeight = _animator->fetchAnimHeight(curAnim->sceneAnimPtr, _animator->_brandonScaleY);
		
		animWidth = (xOffset * animWidth) /  width;
		animHeight = (yOffset * animHeight) / height;
		
		curAnim->x2 = curAnim->x1 += animWidth;
		curAnim->y2 = curAnim->y1 += animHeight;
	} else {
		curAnim->x2 = curAnim->x1 = _currentCharacter->x1 + xOffset;
		curAnim->y2 = curAnim->y1 = _currentCharacter->y1 + yOffset;
	}
	
	int scaleModeBackup = _scaleMode;
	if (changeScaleMode) {
		_scaleMode = 1;
	}
	
	_animator->animRefreshNPC(0);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);
	
	_scaleMode = scaleModeBackup;

	return 0;
}

int KyraEngine::o1_restoreAllObjectBackgrounds(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_restoreAllObjectBackgrounds(%p) (%d)", (const void *)script, stackPos(0));
	int disable = stackPos(0);
	int activeBackup = 0;
	if (disable) {
		activeBackup = _animator->actors()->active;
		_animator->actors()->active = 0;
	}
	_animator->restoreAllObjectBackgrounds();
	if (disable)
		_animator->actors()->active = activeBackup;
	return 0;
}

int KyraEngine::o1_setCustomPaletteRange(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setCustomPaletteRange(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	memcpy(_screen->getPalette(1) + stackPos(1)*3, _specialPalettes[stackPos(0)], stackPos(2)*3);
	return 0;
}

int KyraEngine::o1_loadPageFromDisk(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_loadPageFromDisk(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	_screen->loadPageFromDisk(stackPosString(0), stackPos(1));
	_animator->_updateScreen = true;
	return 0;
}

int KyraEngine::o1_customPrintTalkString(ScriptState *script) {
	if (_flags.isTalkie) {
		debugC(3, kDebugLevelScriptFuncs, "o1_customPrintTalkString(%p) (%d, '%s', %d, %d, %d)", (const void *)script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4) & 0xFF);

		if (speechEnabled()) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(stackPos(0));
		}
		_skipFlag = false;
		if (textEnabled())
			_text->printTalkTextMessage(stackPosString(1), stackPos(2), stackPos(3), stackPos(4) & 0xFF, 0, 2);
	} else {
		debugC(3, kDebugLevelScriptFuncs, "o1_customPrintTalkString(%p) ('%s', %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3) & 0xFF);
		_skipFlag = false;
		_text->printTalkTextMessage(stackPosString(0), stackPos(1), stackPos(2), stackPos(3) & 0xFF, 0, 2);
	}
	_screen->updateScreen();
	return 0;
}

int KyraEngine::o1_restoreCustomPrintBackground(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_restoreCustomPrintBackground(%p) ()", (const void *)script);
	snd_voiceWaitForFinish();
	snd_stopVoice();
	_text->restoreTalkTextMessageBkgd(2, 0);
	return 0;
}

int KyraEngine::o1_hideMouse(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_hideMouse(%p) ()", (const void *)script);
	_screen->hideMouse();
	return 0;
}

int KyraEngine::o1_showMouse(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_showMouse(%p) ()", (const void *)script);
	_screen->showMouse();
	return 0;
}

int KyraEngine::o1_getCharacterX(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getCharacterX(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].x1;
}

int KyraEngine::o1_getCharacterY(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getCharacterY(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].y1;
}

int KyraEngine::o1_changeCharactersFacing(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_changeCharactersFacing(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int character = stackPos(0);
	int facing = stackPos(1);
	int newAnimFrame = stackPos(2);
	
	_animator->restoreAllObjectBackgrounds();
	if (newAnimFrame != -1) {
		_characterList[character].currentAnimFrame = newAnimFrame;
	}
	_characterList[character].facing = facing;
	_animator->animRefreshNPC(character);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);
	
	return 0;
}

int KyraEngine::o1_copyWSARegion(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_copyWSARegion(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int width = stackPos(2);
	int height = stackPos(3);
	int srcPage = stackPos(4);
	int dstPage = stackPos(5);
	_screen->copyRegion(xpos, ypos, xpos, ypos, width, height, srcPage, dstPage);
	_animator->_updateScreen = true;
	return 0;
}

int KyraEngine::o1_printText(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_printText(%p) ('%s', %d, %d, 0x%X, 0x%X)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->printText(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->updateScreen();
	return 0;
}

int KyraEngine::o1_random(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_random(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < stackPos(1));
	return _rnd.getRandomNumberRng(stackPos(0), stackPos(1));
}

int KyraEngine::o1_loadSoundFile(ScriptState *script) {
	warning("STUB: o1_loadSoundFile");
	return 0;
}

int KyraEngine::o1_displayWSAFrameOnHidPage(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_displayWSAFrameOnHidPage(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int frame = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int waitTime = stackPos(3);
	int wsaIndex = stackPos(4);
	
	_screen->hideMouse();
	uint32 continueTime = waitTime * _tickLength + _system->getMillis();
	_movieObjects[wsaIndex]->setX(xpos);
	_movieObjects[wsaIndex]->setY(ypos);
	_movieObjects[wsaIndex]->setDrawPage(2);
	_movieObjects[wsaIndex]->displayFrame(frame);
	_animator->_updateScreen = true;
	while (_system->getMillis() < continueTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		if (_skipFlag)
			break;

		if (continueTime - _system->getMillis() >= 10)
			delay(10);
	}
	_screen->showMouse();
	
	return 0;
}

int KyraEngine::o1_displayWSASequentialFrames(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_displayWSASequentialFrames(%p) (%d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6));
	int startFrame = stackPos(0);
	int endFrame = stackPos(1);
	int xpos = stackPos(2);
	int ypos = stackPos(3);
	int waitTime = stackPos(4);
	int wsaIndex = stackPos(5);
	int maxTime = stackPos(6);
	if (maxTime - 1 <= 0)
		maxTime = 1;

	_movieObjects[wsaIndex]->setX(xpos);
	_movieObjects[wsaIndex]->setY(ypos);
	_movieObjects[wsaIndex]->setDrawPage(0);

	// Workaround for bug #1498221 "KYRA1: Glitches when meeting Zanthia"
	// the original didn'to do a forced screen update after displaying a wsa frame
	// while we have to do it, which make brandon disappear for a short moment,
	// which shouldn't happen. So we're not updating the screen for this special
	// case too.
	if (startFrame == 18 && endFrame == 18 && _currentRoom == 45) {
		_movieObjects[wsaIndex]->displayFrame(18);
		delay(waitTime * _tickLength);
		return 0;
	}

	int curTime = 0;
	_screen->hideMouse();
	while (curTime < maxTime) {
		if (endFrame >= startFrame) {
			int frame = startFrame;
			while (endFrame >= frame) {
				uint32 continueTime = waitTime * _tickLength + _system->getMillis();
				_movieObjects[wsaIndex]->displayFrame(frame);
				_animator->_updateScreen = true;
				while (_system->getMillis() < continueTime) {
					_sprites->updateSceneAnims();
					_animator->updateAllObjectShapes();
					if (_skipFlag)
						break;

					if (continueTime - _system->getMillis() >= 10)
						delay(10);
				}
				++frame;
			}
		} else {
			int frame = startFrame;
			while (endFrame <= frame) {
				uint32 continueTime = waitTime * _tickLength + _system->getMillis();
				_movieObjects[wsaIndex]->displayFrame(frame);
				_animator->_updateScreen = true;
				while (_system->getMillis() < continueTime) {
					_sprites->updateSceneAnims();
					_animator->updateAllObjectShapes();
					if (_skipFlag)
						break;

					if (continueTime - _system->getMillis() >= 10)
						delay(10);
				}
				--frame;
			}
		}
		if (_skipFlag)
			break;
		else
			++curTime;
	}
	_screen->showMouse();
	
	return 0;
}

int KyraEngine::o1_drawCharacterStanding(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_drawCharacterStanding(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int character = stackPos(0);
	int animFrame = stackPos(1);
	int newFacing = stackPos(2);
	int updateShapes = stackPos(3);
	_characterList[character].currentAnimFrame = animFrame;
	if (newFacing != -1) {
		_characterList[character].facing = newFacing;
	}
	_animator->animRefreshNPC(character);
	if (updateShapes) {
		_animator->updateAllObjectShapes();
	}
	return 0;
}

int KyraEngine::o1_internalAnimOff(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_internalAnimOff(%p) (%d)", (const void *)script, stackPos(0));
	_animator->sprites()[stackPos(0)].active = 0;
	return 0;
}

int KyraEngine::o1_changeCharactersXAndY(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_changeCharactersXAndY(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	Character *ch = &_characterList[stackPos(0)];
	int16 x = stackPos(1);
	int16 y = stackPos(2);
	if (x != -1 && y != -1) {
		x &= 0xFFFC;
		y &= 0xFFFE;
	}
	_animator->restoreAllObjectBackgrounds();
	ch->x1 = ch->x2 = x;
	ch->y1 = ch->y2 = y;
	_animator->preserveAllBackgrounds();
	return 0;
}

int KyraEngine::o1_clearSceneAnimatorBeacon(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_clearSceneAnimatorBeacon(%p) ()", (const void *)script);
	_sprites->_sceneAnimatorBeaconFlag = 0;
	return 0;
}

int KyraEngine::o1_querySceneAnimatorBeacon(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_querySceneAnimatorBeacon(%p) ()", (const void *)script);
	return _sprites->_sceneAnimatorBeaconFlag;
}

int KyraEngine::o1_refreshSceneAnimator(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_refreshSceneAnimator(%p) ()", (const void *)script);
	_sprites->updateSceneAnims();
	_animator->updateAllObjectShapes();
	return 0;
}

int KyraEngine::o1_placeItemInOffScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_placeItemInOffScene(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int item = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int sceneId = stackPos(3);
	
	byte freeItem = findFreeItemInScene(sceneId);
	if (freeItem != 0xFF) {
		assert(sceneId < _roomTableSize);
		Room *room = &_roomTable[sceneId];
		
		room->itemsTable[freeItem] = item;
		room->itemsXPos[freeItem] = xpos;
		room->itemsYPos[freeItem] = ypos;
	}
	return 0;
}

int KyraEngine::o1_wipeDownMouseItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_wipeDownMouseItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	_screen->hideMouse();
	wipeDownMouseItem(stackPos(1), stackPos(2));
	destroyMouseItem();
	_screen->showMouse();
	return 0;
}

int KyraEngine::o1_placeCharacterInOtherScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_placeCharacterInOtherScene(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int id = stackPos(0);
	int sceneId = stackPos(1);
	int xpos = (int16)(stackPos(2) & 0xFFFC);
	int ypos = (int16)(stackPos(3) & 0xFFFE);
	int facing = stackPos(4);
	int animFrame = stackPos(5);
	
	_characterList[id].sceneId = sceneId;
	_characterList[id].x1 = _characterList[id].x2 = xpos;
	_characterList[id].y1 = _characterList[id].y2 = ypos;
	_characterList[id].facing = facing;
	_characterList[id].currentAnimFrame = animFrame;
	return 0;
}

int KyraEngine::o1_getKey(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getKey(%p) ()", (const void *)script);
	waitForEvent();
	return 0;
}

int KyraEngine::o1_specificItemInInventory(ScriptState *script) {
	warning("STUB: o1_specificItemInInventory");
	return 0;
}

int KyraEngine::o1_popMobileNPCIntoScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_popMobileNPCIntoScene(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), (int16)(stackPos(4) & 0xFFFC), (int8)(stackPos(5) & 0xFE));
	int character = stackPos(0);
	int sceneId = stackPos(1);
	int animFrame = stackPos(2);
	int facing = stackPos(3);
	int16 xpos = (int16)(stackPos(4) & 0xFFFC);
	int8 ypos = (int16)(stackPos(5) & 0xFFFE);
	Character *curChar = &_characterList[character];
	
	curChar->sceneId = sceneId;
	curChar->currentAnimFrame = animFrame;
	curChar->facing = facing;
	curChar->x1 = curChar->x2 = xpos;
	curChar->y1 = curChar->y2 = ypos;
	
	_animator->animAddNPC(character);
	_animator->updateAllObjectShapes();
	return 0;
}

int KyraEngine::o1_mobileCharacterInScene(ScriptState *script) {
	warning("STUB: o1_mobileCharacterInScene");
	return 0;
}

int KyraEngine::o1_hideMobileCharacter(ScriptState *script) {
	warning("STUB: o1_hideMobileCharacter");
	return 0;
}

int KyraEngine::o1_unhideMobileCharacter(ScriptState *script) {
	warning("STUB: o1_unhideMobileCharacter");
	return 0;
}

int KyraEngine::o1_setCharactersLocation(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setCharactersLocation(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	Character *ch = &_characterList[stackPos(0)];
	AnimObject *animObj = &_animator->actors()[stackPos(0)];
	int newScene = stackPos(1);
	if (_currentCharacter->sceneId == ch->sceneId) {
		if (_currentCharacter->sceneId != newScene)
			animObj->active = 0;
	} else if (_currentCharacter->sceneId == newScene) {
		if (_currentCharacter->sceneId != ch->sceneId)
			animObj->active = 1;
	}
	
	ch->sceneId = stackPos(1);
	return 0;
}

int KyraEngine::o1_walkCharacterToPoint(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_walkCharacterToPoint(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int character = stackPos(0);
	int toX = stackPos(1);
	int toY = stackPos(2);
	_pathfinderFlag2 = 1;
	uint32 nextFrame;
	int findWayReturn = findWay(_characterList[character].x1, _characterList[character].y1, toX, toY, _movFacingTable, 150);
	_pathfinderFlag2 = 0;
	if (_lastFindWayRet < findWayReturn) {
		_lastFindWayRet = findWayReturn;
	}
	if (findWayReturn == 0x7D00 || findWayReturn == 0) {
		return 0;
	}
	int *curPos = _movFacingTable;
	bool running = true;
	while (running) {
		bool forceContinue = false;
		switch (*curPos) {
		case 0:
			_characterList[character].facing = 2;
			break;

		case 1:
			_characterList[character].facing = 1;
			break;

		case 2:
			_characterList[character].facing = 0;
			break;

		case 3:
			_characterList[character].facing = 7;
			break;

		case 4:
			_characterList[character].facing = 6;
			break;

		case 5:
			_characterList[character].facing = 5;
			break;

		case 6:
			_characterList[character].facing = 4;
			break;

		case 7:
			_characterList[character].facing = 3;
			break;

		case 8:
			running = 0;
			break;

		default:
			++curPos;
			forceContinue = true;
			break;
		}
		
		if (forceContinue || !running) {
			continue;
		}
		
		setCharacterPosition(character, 0);
		++curPos;

		nextFrame = getTimerDelay(5 + character) * _tickLength + _system->getMillis();
		while (_system->getMillis() < nextFrame) {
			_sprites->updateSceneAnims();
			updateMousePointer();
			updateGameTimers();
			_animator->updateAllObjectShapes();
			updateTextFade();
			if ((nextFrame - _system->getMillis()) >= 10)
				delay(10);
		}
	}
	return 0;
}

int KyraEngine::o1_specialEventDisplayBrynnsNote(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_specialEventDisplayBrynnsNote(%p) ()", (const void *)script);
	_screen->hideMouse();
	_screen->savePageToDisk("HIDPAGE.TMP", 2);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);
	if (_flags.isTalkie) {
		if (_flags.lang == Common::EN_ANY) {
			_screen->loadBitmap("NOTEENG.CPS", 3, 3, 0);
		} else if (_flags.lang == Common::FR_FRA) {
			_screen->loadBitmap("NOTEFRE.CPS", 3, 3, 0);
		} else if (_flags.lang == Common::DE_DEU) {
			_screen->loadBitmap("NOTEGER.CPS", 3, 3, 0);
		}
	} else {
		_screen->loadBitmap("NOTE.CPS", 3, 3, 0);
	}
	_screen->copyRegion(63, 8, 63, 8, 194, 128, 2, 0);
	_screen->updateScreen();
	_screen->showMouse();
	_screen->setFont(Screen::FID_6_FNT);
	return 0;
}

int KyraEngine::o1_specialEventRemoveBrynnsNote(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_specialEventRemoveBrynnsNote(%p) ()", (const void *)script);
	_screen->hideMouse();
	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->loadPageFromDisk("HIDPAGE.TMP", 2);
	_screen->updateScreen();
	_screen->showMouse();
	_screen->setFont(Screen::FID_8_FNT);
	return 0;
}

int KyraEngine::o1_setLogicPage(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setLogicPage(%p) (%d)", (const void *)script, stackPos(0));
	_screen->_curPage = stackPos(0);
	return stackPos(0);
}

int KyraEngine::o1_fatPrint(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_fatPrint(%p) ('%s', %d, %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));

	// Workround for bug #1582672 ("KYRA1: Text crippled and drawn wrong")
	// I'm not sure how the original handels this, since it seems to call
	// printText also, maybe it fails somewhere inside...
	// TODO: fix the reason for this workaround ;-)
	if (_currentRoom == 117)
		return 0;
	_text->printText(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	return 0;
}

int KyraEngine::o1_preserveAllObjectBackgrounds(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_preserveAllObjectBackgrounds(%p) ()", (const void *)script);
	_animator->preserveAllBackgrounds();
	return 0;
}

int KyraEngine::o1_updateSceneAnimations(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_updateSceneAnimations(%p) (%d)", (const void *)script, stackPos(0));
	int times = stackPos(0);
	while (times--) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
	}
	return 0;
}

int KyraEngine::o1_sceneAnimationActive(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_sceneAnimationActive(%p) (%d)", (const void *)script, stackPos(0));
	return _sprites->_anims[stackPos(0)].play;
}

int KyraEngine::o1_setCharactersMovementDelay(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setCharactersMovementDelay(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setTimerDelay(stackPos(0)+5, stackPos(1));
	return 0;
}

int KyraEngine::o1_getCharactersFacing(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getCharactersFacing(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].facing;
}

int KyraEngine::o1_bkgdScrollSceneAndMasksRight(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_bkgdScrollSceneAndMasksRight(%p) (%d)", (const void *)script, stackPos(0));
	_screen->copyBackgroundBlock(stackPos(0), 2, 0);
	_screen->copyBackgroundBlock2(stackPos(0));
	// update the whole screen
	_screen->copyRegion(7, 7, 7, 7, 305, 129, 3, 0);
	_screen->updateScreen();
	return 0;
}

int KyraEngine::o1_dispelMagicAnimation(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_dispelMagicAnimation(%p) ()", (const void *)script);
	seq_dispelMagicAnimation();
	return 0;
}

int KyraEngine::o1_findBrightestFireberry(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_findBrightestFireberry(%p) ()", (const void *)script);
	if (_currentCharacter->sceneId >= 187 && _currentCharacter->sceneId <= 198) {
		return 29;
	}
	if (_currentCharacter->sceneId == 133 || _currentCharacter->sceneId == 137 ||
		_currentCharacter->sceneId == 165 || _currentCharacter->sceneId == 173) {
		return 29;
	}
	if (_itemInHand == 28)
		return 28;
	int brightestFireberry = 107;
	if (_itemInHand >= 29 && _itemInHand <= 33)
		brightestFireberry = _itemInHand;
	for (int i = 0; i < 10; ++i) {
		uint8 item = _currentCharacter->inventoryItems[i];
		if (item == 0xFF)
			continue;
		if (item == 28)
			return 28;
		if (item >= 29 && item <= 33) {
			if (item < brightestFireberry)
				brightestFireberry = item;
		}
	}
	assert(_currentCharacter->sceneId < _roomTableSize);
	Room *curRoom = &_roomTable[_currentCharacter->sceneId];
	for (int i = 0; i < 12; ++i) {
		uint8 item = curRoom->itemsTable[i];
		if (item == 0xFF)
			continue;
		if (item == 28)
			return 28;
		if (item >= 29 && item <= 33) {
			if (item < brightestFireberry)
				brightestFireberry = item;
		}
	}
	if (brightestFireberry == 107)
		return -1;
	return brightestFireberry;
}

int KyraEngine::o1_setFireberryGlowPalette(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setFireberryGlowPalette(%p) (%d)", (const void *)script, stackPos(0));
	int palIndex = 0;
	switch (stackPos(0)) {
	case 0x1E:
		palIndex = 9;
		break;

	case 0x1F:
		palIndex = 10;
		break;

	case 0x20:
		palIndex = 11;
		break;

	case 0x21:
	case -1:
		palIndex = 12;
		break;

	default:
		palIndex = 8;
		break;
	}
	if (_brandonStatusBit & 2) {
		if (_currentCharacter->sceneId != 133 && _currentCharacter->sceneId != 137 &&
			_currentCharacter->sceneId != 165 && _currentCharacter->sceneId != 173 &&
			(_currentCharacter->sceneId < 187 || _currentCharacter->sceneId > 198)) {
			palIndex = 14;
		}
	}
	const uint8 *palette = _specialPalettes[palIndex];
	memcpy(_screen->getPalette(1) + 684, palette, 44);
	return 0;
}

int KyraEngine::o1_setDeathHandlerFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setDeathHandlerFlag(%p) (%d)", (const void *)script, stackPos(0));
	_deathHandler = stackPos(0);
	return 0;
}

int KyraEngine::o1_drinkPotionAnimation(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_drinkPotionAnimation(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	seq_playDrinkPotionAnim(stackPos(0), stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine::o1_makeAmuletAppear(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_makeAmuletAppear(%p) ()", (const void *)script);
	WSAMovieV1 amulet(this);
	amulet.open("AMULET.WSA", 1, 0);
	amulet.setX(224);
	amulet.setY(152);
	amulet.setDrawPage(0);
	if (amulet.opened()) {
		assert(_amuleteAnim);
		_screen->hideMouse();
		snd_playSoundEffect(0x70);
		uint32 nextTime = 0;
		for (int i = 0; _amuleteAnim[i] != 0xFF; ++i) {
			nextTime = _system->getMillis() + 5 * _tickLength;
			
			uint8 code = _amuleteAnim[i];
			if (code == 3 || code == 7) {
				snd_playSoundEffect(0x71);
			}
			
			if (code == 5) {
				snd_playSoundEffect(0x72);
			}
			
			if (code == 14) {
				snd_playSoundEffect(0x73);
			}
			
			
			amulet.displayFrame(code);
			_animator->_updateScreen = true;
			
			while (_system->getMillis() < nextTime) {
				_sprites->updateSceneAnims();
				_animator->updateAllObjectShapes();
				if (nextTime - _system->getMillis() >= 10)
					delay(10);
			}
		}
		_screen->showMouse();
	}
	setGameFlag(0x2D);
	return 0;
}

int KyraEngine::o1_drawItemShapeIntoScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_drawItemShapeIntoScene(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int item = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int flags = stackPos(3);
	int onlyHidPage = stackPos(4);
	if (flags)
		flags = 1;
	if (onlyHidPage) {
		_screen->drawShape(2, _shapes[220+item], x, y, 0, flags);
	} else {
		_screen->hideMouse();
		_animator->restoreAllObjectBackgrounds();
		_screen->drawShape(2, _shapes[220+item], x, y, 0, flags);
		_screen->drawShape(0, _shapes[220+item], x, y, 0, flags);
		_animator->flagAllObjectsForBkgdChange();
		_animator->preserveAnyChangedBackgrounds();
		_animator->flagAllObjectsForRefresh();
		_animator->updateAllObjectShapes();
		_screen->showMouse();
	}
	return 0;
}

int KyraEngine::o1_setCharactersCurrentFrame(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setCharactersCurrentFrame(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_characterList[stackPos(0)].currentAnimFrame = stackPos(1);
	return 0;
}

int KyraEngine::o1_waitForConfirmationMouseClick(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_waitForConfirmationMouseClick(%p) ()", (const void *)script);
	// if (mouseEnabled) {
	while (!_mousePressFlag) {
		updateMousePointer();
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		delay(10);
	}
	
	while (_mousePressFlag) {
		updateMousePointer();
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		delay(10);
	}
	// }
	processButtonList(_buttonList);
	_skipFlag = false;
	script->variables[1] = _mouseX;
	script->variables[2] = _mouseY;
	return 0;
}

int KyraEngine::o1_pageFlip(ScriptState *script) {
	warning("STUB: o1_pageFlip");
	return 0;
}

int KyraEngine::o1_setSceneFile(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setSceneFile(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setSceneFile(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::o1_getItemInMarbleVase(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getItemInMarbleVase(%p) ()", (const void *)script);
	return _marbleVaseItem;
}

int KyraEngine::o1_setItemInMarbleVase(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setItemInMarbleVase(%p) (%d)", (const void *)script, stackPos(0));
	_marbleVaseItem = stackPos(0);
	return 0;
}

int KyraEngine::o1_addItemToInventory(ScriptState *script) {
	warning("STUB: o1_addItemToInventory");
	return 0;
}

int KyraEngine::o1_intPrint(ScriptState *script) {
	warning("STUB: o1_intPrint");
	return 0;
}

int KyraEngine::o1_shakeScreen(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_shakeScreen(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int waitTicks = stackPos(1);
	int times = stackPos(0);

	for (int i = 0; i < times; ++i) {
		_screen->shakeScreen(1);
		delay(waitTicks * _tickLength);
	}

	return 0;
}

int KyraEngine::o1_createAmuletJewel(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_createAmuletJewel(%p) (%d)", (const void *)script, stackPos(0));
	seq_createAmuletJewel(stackPos(0), 0, 0, 0);
	return 0;
}

int KyraEngine::o1_setSceneAnimCurrXY(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setSceneAnimCurrXY(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	_sprites->_anims[stackPos(0)].x = stackPos(1);
	_sprites->_anims[stackPos(0)].y = stackPos(2);
	return 0;
}

int KyraEngine::o1_poisonBrandonAndRemaps(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_poisonBrandonAndRemaps(%p) ()", (const void *)script);
	setBrandonPoisonFlags(1);
	return 0;
}

int KyraEngine::o1_fillFlaskWithWater(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_fillFlaskWithWater(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	seq_fillFlaskWithWater(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::o1_getCharactersMovementDelay(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getCharactersMovementDelay(%p) (%d)", (const void *)script, stackPos(0));
	return getTimerDelay(stackPos(0)+5);
}

int KyraEngine::o1_getBirthstoneGem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getBirthstoneGem(%p) (%d)", (const void *)script, stackPos(0));
	if (stackPos(0) < 4) {
		return _birthstoneGemTable[stackPos(0)];
	}
	return 0;
}

int KyraEngine::o1_queryBrandonStatusBit(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_queryBrandonStatusBit(%p) (%d)", (const void *)script, stackPos(0));
	if (_brandonStatusBit & stackPos(0)) {
		return 1;
	}
	return 0;
}

int KyraEngine::o1_playFluteAnimation(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_playFluteAnimation(%p) ()", (const void *)script);
	seq_playFluteAnimation();
	return 0;
}

int KyraEngine::o1_playWinterScrollSequence(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_playWinterScrollSequence(%p) (%d)", (const void *)script, stackPos(0));
	if (!stackPos(0)) {
		seq_winterScroll2();
	} else {
		seq_winterScroll1();
	}
	return 0;
}

int KyraEngine::o1_getIdolGem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getIdolGem(%p) (%d)", (const void *)script, stackPos(0));
	return _idolGemsTable[stackPos(0)];;
}

int KyraEngine::o1_setIdolGem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setIdolGem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_idolGemsTable[stackPos(0)] = stackPos(1);
	return 0;
}

int KyraEngine::o1_totalItemsInScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_totalItemsInScene(%p) (%d)", (const void *)script, stackPos(0));
	return countItemsInScene(stackPos(0));
}

int KyraEngine::o1_restoreBrandonsMovementDelay(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_restoreBrandonsMovementDelay(%p) ()", (const void *)script);
	setWalkspeed(_configWalkspeed);	
	return 0;
}

int KyraEngine::o1_setMousePos(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setMousePos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_system->warpMouse(stackPos(0), stackPos(1));
	_mouseX = stackPos(0);
	_mouseY = stackPos(1);
	return 0;
}

int KyraEngine::o1_getMouseState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getMouseState(%p) ()", (const void *)script);
	return _mouseState;
}

int KyraEngine::o1_setEntranceMouseCursorTrack(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setEntranceMouseCursorTrack(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_entranceMouseCursorTracks[0] = stackPos(0);
	_entranceMouseCursorTracks[1] = stackPos(1);
	_entranceMouseCursorTracks[2] = stackPos(0) + stackPos(2) - 1;
	_entranceMouseCursorTracks[3] = stackPos(1) + stackPos(3) - 1;
	_entranceMouseCursorTracks[4] = stackPos(4);
	return 0;
}

int KyraEngine::o1_itemAppearsOnGround(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_itemAppearsOnGround(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	processItemDrop(_currentCharacter->sceneId, stackPos(0), stackPos(1), stackPos(2), 2, 0);
	return 0;
}

int KyraEngine::o1_setNoDrawShapesFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setNoDrawShapesFlag(%p) (%d)", (const void *)script, stackPos(0));
	_animator->_noDrawShapesFlag = stackPos(0);
	return 0;
}

int KyraEngine::o1_fadeEntirePalette(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_fadeEntirePalette(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int cmd = stackPos(0);
	uint8 *fadePal = 0;
	if (cmd == 0) {
		fadePal = _screen->getPalette(2);
		uint8 *screenPal = _screen->getPalette(0);
		uint8 *backUpPal = _screen->getPalette(3);
		
		memcpy(backUpPal, screenPal, sizeof(uint8)*768);
		memset(fadePal, 0, sizeof(uint8)*768);
	} else if (cmd == 1) {
		//fadePal = _screen->getPalette(3);
		warning("unimplemented o1_fadeEntirePalette function");
		return 0;
	} else if (cmd == 2) {
		memset(_screen->getPalette(2), 0, 768);
		memcpy(_screen->getPalette(0), _screen->getPalette(1), 768);
		fadePal = _screen->getPalette(0);
	}
	
	_screen->fadePalette(fadePal, stackPos(1));
	return 0;
}

int KyraEngine::o1_itemOnGroundHere(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_itemOnGroundHere(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < _roomTableSize);
	Room *curRoom = &_roomTable[stackPos(0)];
	for (int i = 0; i < 12; ++i) {
		if (curRoom->itemsTable[i] == stackPos(1))
			return 1;
	}
	return 0;
}

int KyraEngine::o1_queryCauldronState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_queryCauldronState(%p) ()", (const void *)script);
	return _cauldronState;
}

int KyraEngine::o1_setCauldronState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setCauldronState(%p) (%d)", (const void *)script, stackPos(0));
	_cauldronState = stackPos(0);
	return _cauldronState;
}

int KyraEngine::o1_queryCrystalState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_queryCrystalState(%p) (%d)", (const void *)script, stackPos(0));
	if (!stackPos(0)) {
		return _crystalState[0];
	} else if (stackPos(0) == 1) {
		return _crystalState[1];
	}
	return -1;
}

int KyraEngine::o1_setCrystalState(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setCrystalState(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (!stackPos(0)) {
		_crystalState[0] = stackPos(1);
	} else if (stackPos(0) == 1) {
		_crystalState[1] = stackPos(1);
	}
	return stackPos(1);
}

int KyraEngine::o1_setPaletteRange(ScriptState *script) {
	warning("STUB: o1_setPaletteRange");
	return 0;
}

int KyraEngine::o1_shrinkBrandonDown(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_shrinkBrandonDown(%p) (%d)", (const void *)script, stackPos(0));
	int delayTime = stackPos(0);
	checkAmuletAnimFlags();
	int scaleValue = _scaleTable[_currentCharacter->y1];
	int scale = 0;
	if (_scaleMode) {
		scale = scaleValue;
	} else {
		scale = 256;
	}
	int scaleModeBackUp = _scaleMode;
	_scaleMode = 1;
	int scaleEnd = scale >> 1;
	for (; scaleEnd <= scale; --scale) {
		_scaleTable[_currentCharacter->y1] = scale;
		_animator->animRefreshNPC(0);
		delayWithTicks(1);
	}
	delayWithTicks(delayTime); // XXX
	_scaleTable[_currentCharacter->y1] = scaleValue;
	_scaleMode = scaleModeBackUp;
	return 0;
}

int KyraEngine::o1_growBrandonUp(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_growBrandonUp(%p) ()", (const void *)script);
	int scaleValue = _scaleTable[_currentCharacter->y1];
	int scale = 0;
	if (_scaleMode) {
		scale = scaleValue;
	} else {
		scale = 256;
	}
	int scaleModeBackUp = _scaleMode;
	_scaleMode = 1;
	for (int curScale = scale >> 1; curScale <= scale; ++curScale) {
		_scaleTable[_currentCharacter->y1] = curScale;
		_animator->animRefreshNPC(0);
		delayWithTicks(1);
	}
	_scaleTable[_currentCharacter->y1] = scaleValue;
	_scaleMode = scaleModeBackUp;
	return 0;
}

int KyraEngine::o1_setBrandonScaleXAndY(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setBrandonScaleXAndY(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_animator->_brandonScaleX = stackPos(0);
	_animator->_brandonScaleY = stackPos(1);
	return 0;
}

int KyraEngine::o1_resetScaleMode(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_resetScaleMode(%p) ()", (const void *)script);
	_scaleMode = 0;
	return 0;
}

int KyraEngine::o1_getScaleDepthTableValue(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getScaleDepthTableValue(%p) (%d)", (const void *)script, stackPos(0));
	assert(stackPos(0) < ARRAYSIZE(_scaleTable));
	return _scaleTable[stackPos(0)];
}

int KyraEngine::o1_setScaleDepthTableValue(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setScaleDepthTableValue(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < ARRAYSIZE(_scaleTable));
	_scaleTable[stackPos(0)] = stackPos(1);
	return stackPos(1);
}

int KyraEngine::o1_message(ScriptState *script) {
	if (_flags.isTalkie) {
		debugC(3, kDebugLevelScriptFuncs, "o1_message(%p) (%d, '%s', %d)", (const void *)script, stackPos(0), stackPosString(1), stackPos(2));
		drawSentenceCommand(stackPosString(1), stackPos(2));
	} else {
		debugC(3, kDebugLevelScriptFuncs, "o1_message(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
		drawSentenceCommand(stackPosString(0), stackPos(1));
	}

	return 0;
}

int KyraEngine::o1_checkClickOnNPC(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_checkClickOnNPC(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return checkForNPCScriptRun(stackPos(0), stackPos(1));
}

int KyraEngine::o1_getFoyerItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_getFoyerItem(%p) (%d)", (const void *)script, stackPos(0));
	assert(stackPos(0) < ARRAYSIZE(_foyerItemTable));
	return _foyerItemTable[stackPos(0)];
}

int KyraEngine::o1_setFoyerItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setFoyerItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < ARRAYSIZE(_foyerItemTable));
	_foyerItemTable[stackPos(0)] = stackPos(1);
	return stackPos(1);
}

int KyraEngine::o1_setNoItemDropRegion(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setNoItemDropRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	addToNoDropRects(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int KyraEngine::o1_walkMalcolmOn(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_walkMalcolmOn(%p) ()", (const void *)script);
	if (!_malcolmFlag)
		_malcolmFlag = 1;
	return 0;
}

int KyraEngine::o1_passiveProtection(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_passiveProtection(%p) ()", (const void *)script);
	return 1;
}

int KyraEngine::o1_setPlayingLoop(ScriptState *script) {
	warning("STUB: o1_setPlayingLoop");
	return 0;
}

int KyraEngine::o1_brandonToStoneSequence(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_brandonToStoneSequence(%p) ()", (const void *)script);
	seq_brandonToStone();
	return 0;
}

int KyraEngine::o1_brandonHealingSequence(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_brandonHealingSequence(%p) ()", (const void *)script);
	seq_brandonHealing();
	return 0;
}

int KyraEngine::o1_protectCommandLine(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_protectCommandLine(%p) (%d)", (const void *)script, stackPos(0));
	return stackPos(0);
}

int KyraEngine::o1_pauseMusicSeconds(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_pauseMusicSeconds(%p) ()", (const void *)script);
	// if music disabled
	//     return
	o1_pauseSeconds(script);
	return 0;
}

int KyraEngine::o1_resetMaskRegion(ScriptState *script) {
	warning("STUB: o1_resetMaskRegion");
	return 0;
}

int KyraEngine::o1_setPaletteChangeFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_setPaletteChangeFlag(%p) (%d)", (const void *)script, stackPos(0));
	_paletteChanged = stackPos(0);
	return _paletteChanged;
}

int KyraEngine::o1_fillRect(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_fillRect(%p) (%d, %d, %d, %d, %d, 0x%X)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int videoPageBackup = _screen->_curPage;
	_screen->_curPage = stackPos(0);
	_screen->fillRect(stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	_screen->_curPage = videoPageBackup;
	return 0;
}

int KyraEngine::o1_vocUnload(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_vocUnload(%p) ()", (const void *)script);
	// this should unload all voc files (not needed)
	return 0;
}

int KyraEngine::o1_vocLoad(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_vocLoad(%p) (%d)", (const void *)script, stackPos(0));
	// this should load the specified voc file (not needed)
	return 0;
}

int KyraEngine::o1_dummy(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "o1_dummy(%p) ()", (const void *)script);
	return 0;
}

} // end of namespace Kyra
