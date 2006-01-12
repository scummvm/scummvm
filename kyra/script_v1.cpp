/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
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

int KyraEngine::cmd_magicInMouseItem(ScriptState *script) {
	debug(3, "cmd_magicInMouseItem(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	magicInMouseItem(stackPos(0), stackPos(1), -1);
	return 0;
}

int KyraEngine::cmd_characterSays(ScriptState *script) {
	// Japanese version?
	/*const char *str1 = "âuâëâôââôüAé?é¢ùêé¢é+é®üH";
	const char *str2 = "âuâëâôâ\\âôüAé?é¢ùêé¢é+é®üH";

	if (strcmp(stackPosString(0), str1) == 0)
		characterSays((char *)str2, stackPos(1), stackPos(2));
	else*/

	if (_features & GF_TALKIE) {
		debug(3, "cmd_characterSays(0x%X) (%d, '%s', %d, %d)", script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3));
		while (snd_voicePlaying() && !_fastMode) {
			delay(10);
		}
		snd_playVoiceFile(stackPos(0));
		characterSays(stackPosString(1), stackPos(2), stackPos(3));
	} else {
		debug(3, "cmd_characterSays(0x%X) ('%s', %d, %d)", script, stackPosString(0), stackPos(1), stackPos(2));
		characterSays(stackPosString(0), stackPos(1), stackPos(2));
	}

	return 0;
}

int KyraEngine::cmd_pauseTicks(ScriptState *script) {
	debug(3, "cmd_pauseTicks(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	if (stackPos(1)) {
		warning("STUB: special cmd_pauseTicks");
		// delete this after correct implementing
		delayWithTicks(stackPos(0));
	} else {
		delayWithTicks(stackPos(0));
	}
	return 0;
}

int KyraEngine::cmd_drawSceneAnimShape(ScriptState *script) {
	debug(3, "cmd_drawSceneAnimShape(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_screen->drawShape(stackPos(4), _sprites->_sceneShapes[stackPos(0)], stackPos(1), stackPos(2), 0, stackPos(3));
	return 0;
}

int KyraEngine::cmd_queryGameFlag(ScriptState *script) {
	debug(3, "cmd_queryGameFlag(0x%X) (0x%X)", script, stackPos(0));
	return queryGameFlag(stackPos(0));
}

int KyraEngine::cmd_setGameFlag(ScriptState *script) {
	debug(3, "cmd_setGameFlag(0x%X) (0x%X)", script, stackPos(0));
	return setGameFlag(stackPos(0));
}

int KyraEngine::cmd_resetGameFlag(ScriptState *script) {
	debug(3, "cmd_resetGameFlag(0x%X) (0x%X)", script, stackPos(0));
	return resetGameFlag(stackPos(0));
}

int KyraEngine::cmd_runNPCScript(ScriptState *script) {
	warning("STUB: cmd_runNPCScript");
	return 0;
}

int KyraEngine::cmd_setSpecialExitList(ScriptState *script) {
	debug(3, "cmd_setSpecialExitList(0x%X) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	
	for (int i = 0; i < 10; ++i) {
		_exitList[i] = stackPos(i);
	}
	_exitListPtr = _exitList;
	
	return 0;
}

int KyraEngine::cmd_blockInWalkableRegion(ScriptState *script) {
	debug(3, "cmd_blockInWalkableRegion(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	blockInRegion(stackPos(0), stackPos(1), stackPos(2)-stackPos(0)+1, stackPos(3)-stackPos(1)+1);
	return 0;
}

int KyraEngine::cmd_blockOutWalkableRegion(ScriptState *script) {
	debug(3, "cmd_blockOutWalkableRegion(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	blockOutRegion(stackPos(0), stackPos(1), stackPos(2)-stackPos(0)+1, stackPos(3)-stackPos(1)+1);
	return 0;
}

int KyraEngine::cmd_walkPlayerToPoint(ScriptState *script) {
	debug(3, "cmd_walkPlayerToPoint(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));

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

int KyraEngine::cmd_dropItemInScene(ScriptState *script) {
	debug(3, "cmd_dropItemInScene(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
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

int KyraEngine::cmd_drawAnimShapeIntoScene(ScriptState *script) {
	debug(3, "cmd_drawAnimShapeIntoScene(0x%X) (%d, %d, %d, %d)", stackPos(0), stackPos(1), stackPos(2), stackPos(3));
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

int KyraEngine::cmd_createMouseItem(ScriptState *script) {
	debug(3, "cmd_createMouseItem(0x%X) (%d)", script, stackPos(0));
	createMouseItem(stackPos(0));
	return 0;
}

int KyraEngine::cmd_savePageToDisk(ScriptState *script) {
	warning("STUB: cmd_savePageToDisk");
	return 0;
}

int KyraEngine::cmd_sceneAnimOn(ScriptState *script) {
	debug(3, "cmd_sceneAnimOn(0x%X) (%d)", script, stackPos(0));
	_sprites->_anims[stackPos(0)].play = true;
	return 0;
}

int KyraEngine::cmd_sceneAnimOff(ScriptState *script) {
	debug(3, "cmd_sceneAnimOff(0x%X) (%d)", script, stackPos(0));
	_sprites->_anims[stackPos(0)].play = false;
	return 0;
}

int KyraEngine::cmd_getElapsedSeconds(ScriptState *script) {
	debug(3, "cmd_getElapsedSeconds(0x%X) ()", script);
	return _system->getMillis() / 1000;
}

int KyraEngine::cmd_mouseIsPointer(ScriptState *script) {
	debug(3, "cmd_mouseIsPointer(0x%X) ()", script);
	if (_itemInHand == -1) {
		return 1;
	}
	return 0;
}

int KyraEngine::cmd_destroyMouseItem(ScriptState *script) {
	debug(3, "cmd_destroyMouseItem(0x%X) ()", script);
	destroyMouseItem();
	return 0;
}

int KyraEngine::cmd_runSceneAnimUntilDone(ScriptState *script) {
	debug(3, "cmd_runSceneAnimUntilDone(0x%X) (%d)", script, stackPos(0));
	_sprites->_anims[stackPos(0)].play = true;
	_animator->sprites()[stackPos(0)].active = 1;
	_screen->hideMouse();
	_animator->restoreAllObjectBackgrounds();
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

int KyraEngine::cmd_fadeSpecialPalette(ScriptState *script) {
	debug(3, "cmd_fadeSpecialPalette(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_screen->fadeSpecialPalette(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int KyraEngine::cmd_playAdlibSound(ScriptState *script) {
	debug(3, "cmd_playAdlibSound(0x%X) (%d)", script, stackPos(0));
	snd_playSoundEffect(stackPos(0));
	return 0;
}

int KyraEngine::cmd_playAdlibScore(ScriptState *script) {
	debug(3, "cmd_playAdlibScore(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	snd_playWanderScoreViaMap(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::cmd_phaseInSameScene(ScriptState *script) {
	debug(3, "cmd_phaseInSameScene(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	transcendScenes(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::cmd_setScenePhasingFlag(ScriptState *script) {
	debug(3, "cmd_setScenePhasingFlag(0x%X) ()", script);
	_scenePhasingFlag = 1;
	return 1;
}

int KyraEngine::cmd_resetScenePhasingFlag(ScriptState *script) {
	debug(3, "cmd_resetScenePhasingFlag(0x%X) ()", script);
	_scenePhasingFlag = 0;
	return 0;
}

int KyraEngine::cmd_queryScenePhasingFlag(ScriptState *script) {
	debug(3, "cmd_queryScenePhasingFlag(0x%X) ()", script);
	return _scenePhasingFlag;
}

int KyraEngine::cmd_sceneToDirection(ScriptState *script) {
	debug(3, "cmd_sceneToDirection(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
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

int KyraEngine::cmd_setBirthstoneGem(ScriptState *script) {
	debug(3, "cmd_setBirthstoneGem(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	int index = stackPos(0);
	if (index < 4 && index >= 0) {
		_birthstoneGemTable[index] = stackPos(1);
		return 1;
	}
	return 0;
}

int KyraEngine::cmd_placeItemInGenericMapScene(ScriptState *script) {
	debug(3, "cmd_placeItemInGenericMapScene(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	placeItemInGenericMapScene(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::cmd_setBrandonStatusBit(ScriptState *script) {
	debug(3, "cmd_setBrandonStatusBit(0x%X) (%d)", script, stackPos(0));
	_brandonStatusBit |= stackPos(0);
	return 0;
}

int KyraEngine::cmd_pauseSeconds(ScriptState *script) {
	debug(3, "cmd_pauseSeconds(0x%X) (%d)", script, stackPos(0));
	if (stackPos(0) > 0)
		delay(stackPos(0)*1000, true);
	return 0;
}

int KyraEngine::cmd_getCharactersLocation(ScriptState *script) {
	debug(3, "cmd_getCharactersLocation(0x%X) (%d)", script, stackPos(0));
	return _characterList[stackPos(0)].sceneId;
}

int KyraEngine::cmd_runNPCSubscript(ScriptState *script) {
	warning("STUB: cmd_runNPCSubscript");
	return 0;
}

int KyraEngine::cmd_magicOutMouseItem(ScriptState *script) {
	debug(3, "cmd_magicOutMouseItem(0x%X) (%d)", script, stackPos(0));
	magicOutMouseItem(stackPos(0), -1);
	return 0;
}

int KyraEngine::cmd_internalAnimOn(ScriptState *script) {
	debug(3, "cmd_internalAnimOn(0x%X) (%d)", script, stackPos(0));
	_animator->sprites()[stackPos(0)].active = 1;
	return 0;
}

int KyraEngine::cmd_forceBrandonToNormal(ScriptState *script) {
	debug(3, "cmd_forceBrandonToNormal(0x%X) ()", script);
	checkAmuletAnimFlags();
	return 0;
}

int KyraEngine::cmd_poisonDeathNow(ScriptState *script) {
	debug(3, "cmd_poisonDeathNow(0x%X) ()", script);
	seq_poisonDeathNow(1);
	return 0;
}

int KyraEngine::cmd_setScaleMode(ScriptState *script) {
	debug(3, "cmd_setScaleMode(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
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

int KyraEngine::cmd_openWSAFile(ScriptState *script) {
	debug(3, "cmd_openWSAFile(0x%X) ('%s', %d)", script, stackPosString(0), stackPos(1));
	
	char *filename = stackPosString(0);
	int wsaIndex = stackPos(1);
	
	_movieObjects[wsaIndex]->open(filename, 1, 0);
	assert(_movieObjects[wsaIndex]->opened());
	
	return 0;
}

int KyraEngine::cmd_closeWSAFile(ScriptState *script) {
	debug(3, "cmd_closeWSAFile(0x%X) (%d)", script, stackPos(0));
	
	int wsaIndex = stackPos(0);
	if (_movieObjects[wsaIndex]) {
		_movieObjects[wsaIndex]->close();
	}
	
	return 0;
}

int KyraEngine::cmd_runWSAFromBeginningToEnd(ScriptState *script) {
	debug(3, "cmd_runWSAFromBeginningToEnd(0x%X) (%d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	
	_screen->hideMouse();
	
	bool running = true;
	
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int waitTime = stackPos(2);
	int wsaIndex = stackPos(3);
	int worldUpdate = stackPos(4);
	int wsaFrame = 0;
	
	_movieObjects[wsaIndex]->_x = xpos;
	_movieObjects[wsaIndex]->_y = ypos;
	_movieObjects[wsaIndex]->_drawPage = 0;
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

int KyraEngine::cmd_displayWSAFrame(ScriptState *script) {
	debug(3, "cmd_displayWSAFrame(0x%X) (%d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int frame = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int waitTime = stackPos(3);
	int wsaIndex = stackPos(4);
	_screen->hideMouse();
	_movieObjects[wsaIndex]->_x = xpos;
	_movieObjects[wsaIndex]->_y = ypos;
	_movieObjects[wsaIndex]->_drawPage = 0;
	_movieObjects[wsaIndex]->displayFrame(frame);
	_animator->_updateScreen = true;
	uint32 continueTime = waitTime * _tickLength + _system->getMillis();
	while (_system->getMillis() < continueTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		if (continueTime - _system->getMillis() >= 10)
			delay(10);
	}
	_screen->showMouse();
	return 0;
}

int KyraEngine::cmd_enterNewScene(ScriptState *script) {
	debug(3, "cmd_enterNewScene(0x%X) (%d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	enterNewScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return 0;
}

int KyraEngine::cmd_setSpecialEnterXAndY(ScriptState *script) {
	debug(3, "cmd_setSpecialEnterXAndY(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	_brandonPosX = stackPos(0);
	_brandonPosY = stackPos(1);
	if (_brandonPosX + 1 == 0 && _brandonPosY + 1 == 0)
		_currentCharacter->currentAnimFrame = 88;
	return 0;
}

int KyraEngine::cmd_runWSAFrames(ScriptState *script) {
	debug(3, "cmd_runWSAFrames(0x%X) (%d, %d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int delayTime = stackPos(2);
	int startFrame = stackPos(3);
	int endFrame = stackPos(4);
	int wsaIndex = stackPos(5);
	_screen->hideMouse();
	_movieObjects[wsaIndex]->_x = xpos;
	_movieObjects[wsaIndex]->_y = ypos;
	_movieObjects[wsaIndex]->_drawPage = 0;
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

int KyraEngine::cmd_popBrandonIntoScene(ScriptState *script) {
	debug(3, "cmd_popBrandonIntoScene(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
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
		_brandonScaleY = _scaleTable[_currentCharacter->y1];
		_brandonScaleX = _brandonScaleY;
		
		int animWidth = fetchAnimWidth(curAnim->sceneAnimPtr, _brandonScaleX) >> 1;
		int animHeight = fetchAnimHeight(curAnim->sceneAnimPtr, _brandonScaleY);
		
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
	
	animRefreshNPC(0);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);
	
	_scaleMode = scaleModeBackup;

	return 0;
}

int KyraEngine::cmd_restoreAllObjectBackgrounds(ScriptState *script) {
	debug(3, "cmd_restoreAllObjectBackgrounds(0x%X) ()", script);
	_animator->restoreAllObjectBackgrounds();
	return 0;
}

int KyraEngine::cmd_setCustomPaletteRange(ScriptState *script) {
	debug(3, "cmd_setCustomPaletteRange(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
	uint8 *screenPal = _screen->_currentPalette;
	memcpy(&screenPal[stackPos(1)*3], _specialPalettes[stackPos(0)], stackPos(2)*3);
	_screen->setScreenPalette(screenPal);
	return 0;
}

int KyraEngine::cmd_loadPageFromDisk(ScriptState *script) {
	warning("STUB: cmd_loadPageFromDisk");
	return 0;
}

int KyraEngine::cmd_customPrintTalkString(ScriptState *script) {
	if (_features & GF_TALKIE) {
		debug(3, "cmd_customPrintTalkString(0x%X) (%d, '%s', %d, %d, %d)", script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4) & 0xFF);
		while (snd_voicePlaying() && !_fastMode) {
			delay(10);
		}
		snd_playVoiceFile(stackPos(0));
		_text->printTalkTextMessage(stackPosString(1), stackPos(2), stackPos(3), stackPos(4) & 0xFF, 0, 2);
	} else {
		debug(3, "cmd_customPrintTalkString(0x%X) ('%s', %d, %d, %d)", script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3) & 0xFF);
		_text->printTalkTextMessage(stackPosString(0), stackPos(1), stackPos(2), stackPos(3) & 0xFF, 0, 2);
	}
	_animator->_updateScreen = true;
	return 0;
}

int KyraEngine::cmd_restoreCustomPrintBackground(ScriptState *script) {
	debug(3, "cmd_restoreCustomPrintBackground(0x%X) ()", script);
	_text->restoreTalkTextMessageBkgd(2, 0);
	return 0;
}

int KyraEngine::cmd_hideMouse(ScriptState *script) {
	debug(3, "cmd_hideMouse(0x%X) ()", script);
	_screen->hideMouse();
	return 0;
}

int KyraEngine::cmd_showMouse(ScriptState *script) {
	debug(3, "cmd_showMouse(0x%X) ()", script);
	_screen->showMouse();
	return 0;
}

int KyraEngine::cmd_getCharacterX(ScriptState *script) {
	debug(3, "cmd_getCharacterX(0x%X) (%d)", script, stackPos(0));
	return _characterList[stackPos(0)].x1;
}

int KyraEngine::cmd_getCharacterY(ScriptState *script) {
	debug(3, "cmd_getCharacterY(0x%X) (%d)", script, stackPos(0));
	return _characterList[stackPos(0)].y1;
}

int KyraEngine::cmd_changeCharactersFacing(ScriptState *script) {
	debug(3, "cmd_changeCharactersFacing(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
	int character = stackPos(0);
	int facing = stackPos(1);
	int newAnimFrame = stackPos(2);
	
	_animator->restoreAllObjectBackgrounds();
	if (newAnimFrame != -1) {
		_characterList[character].currentAnimFrame = newAnimFrame;
	}
	_characterList[character].facing = facing;
	animRefreshNPC(character);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);
	
	return 0;
}

int KyraEngine::cmd_copyWSARegion(ScriptState *script) {
	debug(3, "cmd_copyWSARegion(0x%X) (%d, %d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int width = stackPos(2);
	int height = stackPos(3);
	int srcPage = stackPos(4);
	int dstPage = stackPos(5);
	_screen->copyRegion(xpos, ypos, xpos, ypos, width, height, srcPage, dstPage, 0);
	return 0;
}

int KyraEngine::cmd_printText(ScriptState *script) {
	debug(3, "cmd_printText(0x%X) ('%s', %d, %d, 0x%X, 0x%X)", script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->printText(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->updateScreen();
	return 0;
}

int KyraEngine::cmd_random(ScriptState *script) {
	debug(3, "cmd_random(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	assert(stackPos(0) < stackPos(1));
	return _rnd.getRandomNumberRng(stackPos(0), stackPos(1));
}

int KyraEngine::cmd_loadSoundFile(ScriptState *script) {
	warning("STUB: cmd_loadSoundFile");
	return 0;
}

int KyraEngine::cmd_displayWSAFrameOnHidPage(ScriptState *script) {
	debug(3, "cmd_displayWSAFrameOnHidPage(0x%X) (%d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int frame = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int waitTime = stackPos(3);
	int wsaIndex = stackPos(4);
	
	_screen->hideMouse();
	_movieObjects[wsaIndex]->_x = xpos;
	_movieObjects[wsaIndex]->_y = ypos;
	_movieObjects[wsaIndex]->_drawPage = 2;
	_movieObjects[wsaIndex]->displayFrame(frame);
	_animator->_updateScreen = true;
	uint32 continueTime = waitTime * _tickLength + _system->getMillis();
	while (_system->getMillis() < continueTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		if (continueTime - _system->getMillis() >= 10)
			delay(10);
	}
	_screen->showMouse();
	
	return 0;
}

int KyraEngine::cmd_displayWSASequentialFrames(ScriptState *script) {
	debug(3, "cmd_displayWSASequentialFrames(0x%X) (%d, %d, %d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6));
	int startFrame = stackPos(0);
	int endFrame = stackPos(1);
	int xpos = stackPos(2);
	int ypos = stackPos(3);
	int waitTime = stackPos(4);
	int wsaIndex = stackPos(5);
	int maxTime = stackPos(6);
	if (maxTime - 1 <= 0)
		maxTime = 1;

	_movieObjects[wsaIndex]->_x = xpos;
	_movieObjects[wsaIndex]->_y = ypos;
	_movieObjects[wsaIndex]->_drawPage = 0;

	int curTime = 0;
	_screen->hideMouse();
	while (curTime < maxTime) {
		if (endFrame >= startFrame) {
			int frame = startFrame;
			while (endFrame >= frame) {
				_movieObjects[wsaIndex]->displayFrame(frame);
				_animator->_updateScreen = true;
				uint32 continueTime = waitTime * _tickLength + _system->getMillis();
				while (_system->getMillis() < continueTime) {
					_sprites->updateSceneAnims();
					_animator->updateAllObjectShapes();
					if (continueTime - _system->getMillis() >= 10)
						delay(10);
				}
				++frame;
			}
		} else {
			int frame = startFrame;
			while (endFrame <= frame) {
				_movieObjects[wsaIndex]->displayFrame(frame);
				_animator->_updateScreen = true;
				uint32 continueTime = waitTime * _tickLength + _system->getMillis();
				while (_system->getMillis() < continueTime) {
					_sprites->updateSceneAnims();
					_animator->updateAllObjectShapes();
					if (continueTime - _system->getMillis() >= 10)
						delay(10);
				}
				--frame;
			}
		}
		++curTime;
	}
	_screen->showMouse();
	
	return 0;
}

int KyraEngine::cmd_drawCharacterStanding(ScriptState *script) {
	debug(3, "cmd_drawCharacterStanding(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int character = stackPos(0);
	int animFrame = stackPos(1);
	int newFacing = stackPos(2);
	int updateShapes = stackPos(3);
	_characterList[character].currentAnimFrame = animFrame;
	if (newFacing != -1) {
		_characterList[character].facing = newFacing;
	}
	animRefreshNPC(character);
	if (updateShapes) {
		_animator->updateAllObjectShapes();
	}
	return 0;
}

int KyraEngine::cmd_internalAnimOff(ScriptState *script) {
	debug(3, "cmd_internalAnimOff(0x%X) (%d)", script, stackPos(0));
	_animator->sprites()[stackPos(0)].active = 0;
	return 0;
}

int KyraEngine::cmd_changeCharactersXAndY(ScriptState *script) {
	debug(3, "cmd_changeCharactersXAndY(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
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

int KyraEngine::cmd_clearSceneAnimatorBeacon(ScriptState *script) {
	debug(3, "cmd_clearSceneAnimatorBeacon(0x%X) ()", script);
	_sprites->_sceneAnimatorBeaconFlag = 0;
	return 0;
}

int KyraEngine::cmd_querySceneAnimatorBeacon(ScriptState *script) {
	debug(3, "cmd_querySceneAnimatorBeacon(0x%X) ()", script);
	return _sprites->_sceneAnimatorBeaconFlag;
}

int KyraEngine::cmd_refreshSceneAnimator(ScriptState *script) {
	debug(3, "cmd_refreshSceneAnimator(0x%X) ()", script);
	_sprites->updateSceneAnims();
	_animator->updateAllObjectShapes();
	return 0;
}

int KyraEngine::cmd_placeItemInOffScene(ScriptState *script) {
	debug(3, "cmd_placeItemInOffScene(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
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

int KyraEngine::cmd_wipeDownMouseItem(ScriptState *script) {
	debug(3, "cmd_wipeDownMouseItem(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
	_screen->hideMouse();
	wipeDownMouseItem(stackPos(1), stackPos(2));
	destroyMouseItem();
	_screen->showMouse();
	return 0;
}

int KyraEngine::cmd_placeCharacterInOtherScene(ScriptState *script) {
	debug(3, "cmd_placeCharacterInOtherScene(0x%X) (%d, %d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
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

int KyraEngine::cmd_getKey(ScriptState *script) {
	debug(3, "cmd_getKey(0x%X) ()", script);
	waitForEvent();
	return 0;
}

int KyraEngine::cmd_specificItemInInventory(ScriptState *script) {
	warning("STUB: cmd_specificItemInInventory");
	return 0;
}

int KyraEngine::cmd_popMobileNPCIntoScene(ScriptState *script) {
	debug(3, "cmd_popMobileNPCIntoScene(0x%X) (%d, %d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), (int16)(stackPos(4) & 0xFFFC), (int8)(stackPos(5) & 0xFE));
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

int KyraEngine::cmd_mobileCharacterInScene(ScriptState *script) {
	warning("STUB: cmd_mobileCharacterInScene");
	return 0;
}

int KyraEngine::cmd_hideMobileCharacter(ScriptState *script) {
	warning("STUB: cmd_hideMobileCharacter");
	return 0;
}

int KyraEngine::cmd_unhideMobileCharacter(ScriptState *script) {
	warning("STUB: cmd_unhideMobileCharacter");
	return 0;
}

int KyraEngine::cmd_setCharactersLocation(ScriptState *script) {
	debug(3, "cmd_setCharactersLocation(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
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

int KyraEngine::cmd_walkCharacterToPoint(ScriptState *script) {
	debug(3, "cmd_walkCharacterToPoint(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
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

int KyraEngine::cmd_specialEventDisplayBrynnsNote(ScriptState *script) {
	debug(3, "cmd_specialEventDisplayBrynnsNote(0x%X) ()", script);
	_hidPage = (uint8*)malloc(320*200);
	_screenPage = (uint8*)malloc(320*200);
	assert(_hidPage && _screenPage);
	_screen->hideMouse();
	_screen->copyRegionToBuffer(0, 0, 0, 320, 200, _screenPage);
	_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _hidPage);
	if (_features & GF_TALKIE) {
		if (_features & GF_ENGLISH) {
			loadBitmap("NOTEENG.CPS", 3, 3, 0);
		} else if (_features & GF_FRENCH) {
			loadBitmap("NOTEFRE.CPS", 3, 3, 0);
		} else if (_features & GF_GERMAN) {
			loadBitmap("NOTEGER.CPS", 3, 3, 0);
		}
	} else {
		loadBitmap("NOTE.CPS", 3, 3, 0);
	}
	_screen->copyRegion(63, 8, 63, 8, 194, 128, 2, 0);
	_screen->updateScreen();
	_screen->showMouse();
	_screen->setFont(Screen::FID_6_FNT);
	return 0;
}

int KyraEngine::cmd_specialEventRemoveBrynnsNote(ScriptState *script) {
	debug(3, "cmd_specialEventRemoveBrynnsNote(0x%X) ()", script);
	_screen->hideMouse();
	assert(_hidPage && _screenPage);
	_screen->copyBlockToPage(0, 0, 0, 320, 200, _screenPage);
	_screen->copyBlockToPage(2, 0, 0, 320, 200, _hidPage);
	free(_screenPage);
	free(_hidPage);
	_screen->updateScreen();
	_screen->showMouse();
	_screen->setFont(Screen::FID_8_FNT);
	return 0;
}

int KyraEngine::cmd_setLogicPage(ScriptState *script) {
	debug(3, "cmd_setLogicPage(0x%X) (%d)", script, stackPos(0));
	_screen->_curPage = stackPos(0);
	return stackPos(0);
}

int KyraEngine::cmd_fatPrint(ScriptState *script) {
	debug(3, "cmd_fatPrint(0x%X) ('%s', %d, %d, %d, %d, %d)", script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	_text->printText(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	return 0;
}

int KyraEngine::cmd_preserveAllObjectBackgrounds(ScriptState *script) {
	debug(3, "cmd_preserveAllObjectBackgrounds(0x%X) ()", script);
	_animator->preserveAllBackgrounds();
	return 0;
}

int KyraEngine::cmd_updateSceneAnimations(ScriptState *script) {
	debug(3, "cmd_updateSceneAnimations(0x%X) (%d)", script, stackPos(0));
	if (stackPos(0)) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
	}
	return 0;
}

int KyraEngine::cmd_sceneAnimationActive(ScriptState *script) {
	debug(3, "cmd_sceneAnimationActive(0x%X) (%d)", script, stackPos(0));
	return _sprites->_anims[stackPos(0)].play;
}

int KyraEngine::cmd_setCharactersMovementDelay(ScriptState *script) {
	debug(3, "cmd_setCharactersMovementDelay(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	setTimerDelay(stackPos(0)+5, stackPos(1));
	return 0;
}

int KyraEngine::cmd_getCharactersFacing(ScriptState *script) {
	debug(3, "cmd_getCharactersFacing(0x%X) (%d)", script, stackPos(0));
	return _characterList[stackPos(0)].facing;
}

int KyraEngine::cmd_bkgdScrollSceneAndMasksRight(ScriptState *script) {
	debug(3, "cmd_bkgdScrollSceneAndMasksRight(0x%X) (%d)", script, stackPos(0));
	copyBackgroundBlock(stackPos(0), 2, 0);
	copyBackgroundBlock2(stackPos(0));
	// update the whole screen
	_screen->copyRegion(7, 7, 7, 7, 305, 129, 3, 0);
	_screen->updateScreen();
	return 0;
}

int KyraEngine::cmd_dispelMagicAnimation(ScriptState *script) {
	debug(3, "cmd_dispelMagicAnimation(0x%X) ()", script);
	seq_dispelMagicAnimation();
	return 0;
}

int KyraEngine::cmd_findBrightestFireberry(ScriptState *script) {
	debug(3, "cmd_findBrightestFireberry(0x%X) ()", script);
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

int KyraEngine::cmd_setFireberryGlowPalette(ScriptState *script) {
	debug(3, "cmd_setFireberryGlowPalette(0x%X) (%d)", script, stackPos(0));
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
	uint8 *palette = _specialPalettes[palIndex];
	memcpy(&_screen->_currentPalette[684], palette, 44);
	_screen->setScreenPalette(_screen->_currentPalette);
	return 0;
}

int KyraEngine::cmd_setDeathHandlerFlag(ScriptState *script) {
	debug(3, "cmd_drinkPotionAnimation(0x%X) (%d)", script, stackPos(0));
	_deathHandler = stackPos(0);
	return 0;
}

int KyraEngine::cmd_drinkPotionAnimation(ScriptState *script) {
	debug(3, "cmd_drinkPotionAnimation(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
	seq_playDrinkPotionAnim(stackPos(0), stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine::cmd_makeAmuletAppear(ScriptState *script) {
	debug(3, "cmd_makeAmuletAppear(0x%X) ()", script);
	WSAMovieV1 amulet(this);
	amulet.open("AMULET.WSA", 1, 0);
	amulet._drawPage = 0;
	amulet._x = 224;
	amulet._y = 152;
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

int KyraEngine::cmd_drawItemShapeIntoScene(ScriptState *script) {
	debug(3, "cmd_drawItemShapeIntoScene(0x%X) (%d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
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
		_animator->flagAllObjectsForRefresh();
		_animator->updateAllObjectShapes();
		_screen->showMouse();
	}
	return 0;
}

int KyraEngine::cmd_setCharactersCurrentFrame(ScriptState *script) {
	debug(3, "cmd_setCharactersCurrentFrame(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	_characterList[stackPos(0)].currentAnimFrame = stackPos(1);
	return 0;
}

int KyraEngine::cmd_waitForConfirmationMouseClick(ScriptState *script) {
	debug(3, "cmd_waitForConfirmationMouseClick(0x%X) ()", script);
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
	// XXX processButtonList calls
	script->variables[1] = _mouseX;
	script->variables[2] = _mouseY;
	return 0;
}

int KyraEngine::cmd_pageFlip(ScriptState *script) {
	warning("STUB: cmd_pageFlip");
	return 0;
}

int KyraEngine::cmd_setSceneFile(ScriptState *script) {
	debug(3, "cmd_setSceneFile(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	setSceneFile(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::cmd_getItemInMarbleVase(ScriptState *script) {
	debug(3, "cmd_getItemInMarbleVase(0x%X) ()", script);
	return _marbleVaseItem;
}

int KyraEngine::cmd_setItemInMarbleVase(ScriptState *script) {
	debug(3, "cmd_setItemInMarbleVase(0x%X) (%d)", script, stackPos(0));
	_marbleVaseItem = stackPos(0);
	return 0;
}

int KyraEngine::cmd_addItemToInventory(ScriptState *script) {
	warning("STUB: cmd_addItemToInventory");
	return 0;
}

int KyraEngine::cmd_intPrint(ScriptState *script) {
	warning("STUB: cmd_intPrint");
	return 0;
}

int KyraEngine::cmd_shakeScreen(ScriptState *script) {
	warning("STUB: cmd_shakeScreen");
	return 0;
}

int KyraEngine::cmd_createAmuletJewel(ScriptState *script) {
	debug(3, "cmd_createAmuletJewel(0x%X) (%d)", script, stackPos(0));
	seq_createAmuletJewel(stackPos(0), 0, 0, 0);
	return 0;
}

int KyraEngine::cmd_setSceneAnimCurrXY(ScriptState *script) {
	debug(3, "cmd_setSceneAnimCurrXY(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
	_sprites->_anims[stackPos(0)].x = stackPos(1);
	_sprites->_anims[stackPos(0)].y = stackPos(2);
	return 0;
}

int KyraEngine::cmd_poisonBrandonAndRemaps(ScriptState *script) {
	debug(3, "cmd_setSceneAnimCurrXY(0x%X) ()", script);
	setBrandonPoisonFlags(1);
	return 0;
}

int KyraEngine::cmd_fillFlaskWithWater(ScriptState *script) {
	debug(3, "cmd_fillFlaskWithWater(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	seq_fillFlaskWithWater(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::cmd_getCharactersMovementDelay(ScriptState *script) {
	debug(3, "cmd_getCharactersMovementDelay(0x%X) (%d)", script, stackPos(0));
	return getTimerDelay(stackPos(0)+5);
}

int KyraEngine::cmd_getBirthstoneGem(ScriptState *script) {
	debug(3, "cmd_getBirthstoneGem(0x%X) (%d)", script, stackPos(0));
	if (stackPos(0) < 4) {
		return _birthstoneGemTable[stackPos(0)];
	}
	return 0;
}

int KyraEngine::cmd_queryBrandonStatusBit(ScriptState *script) {
	debug(3, "cmd_queryBrandonStatusBit(0x%X) (%d)", script, stackPos(0));
	if (_brandonStatusBit & stackPos(0)) {
		return 1;
	}
	return 0;
}

int KyraEngine::cmd_playFluteAnimation(ScriptState *script) {
	debug(3, "cmd_playFluteAnimation(0x%X) ()", script);
	seq_playFluteAnimation();
	return 0;
}

int KyraEngine::cmd_playWinterScrollSequence(ScriptState *script) {
	debug(3, "cmd_playWinterScrollSequence(0x%X) (%d)", script, stackPos(0));
	if (!stackPos(0)) {
		seq_winterScroll2();
	} else {
		seq_winterScroll1();
	}
	return 0;
}

int KyraEngine::cmd_getIdolGem(ScriptState *script) {
	debug(3, "cmd_getIdolGem(0x%X) (%d)", script, stackPos(0));
	return _idolGemsTable[stackPos(0)];;
}

int KyraEngine::cmd_setIdolGem(ScriptState *script) {
	debug(3, "cmd_setIdolGem(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	_idolGemsTable[stackPos(0)] = stackPos(1);
	return 0;
}

int KyraEngine::cmd_totalItemsInScene(ScriptState *script) {
	debug(3, "cmd_totalItemsInScene(0x%X) (%d)", script, stackPos(0));
	return countItemsInScene(stackPos(0));
}

int KyraEngine::cmd_restoreBrandonsMovementDelay(ScriptState *script) {
	debug(3, "cmd_restoreBrandonsMovemenyDelay(0x%X) ()", script);
	//TODO: Use movement set by menu, instead of 5.
	setTimerDelay(5, 5);
	return 0;
}

int KyraEngine::cmd_setMousePos(ScriptState *script) {
	debug(3, "cmd_setMousePos(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	_system->warpMouse(stackPos(0), stackPos(1));
	_mouseX = stackPos(0);
	_mouseY = stackPos(1);
	return 0;
}

int KyraEngine::cmd_getMouseState(ScriptState *script) {
	debug(3, "cmd_getMouseState(0x%X) ()", script);
	return _mouseState;
}

int KyraEngine::cmd_setEntranceMouseCursorTrack(ScriptState *script) {
	debug(3, "cmd_setEntranceMouseCursorTrack(0x%X) (%d, %d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_entranceMouseCursorTracks[0] = stackPos(0);
	_entranceMouseCursorTracks[1] = stackPos(1);
	_entranceMouseCursorTracks[2] = stackPos(0) + stackPos(2) - 1;
	_entranceMouseCursorTracks[3] = stackPos(1) + stackPos(3) - 1;
	_entranceMouseCursorTracks[4] = stackPos(4);
	return 0;
}

int KyraEngine::cmd_itemAppearsOnGround(ScriptState *script) {
	debug(3, "cmd_itemAppearsOnGround(0x%X) (%d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2));
	processItemDrop(_currentCharacter->sceneId, stackPos(0), stackPos(1), stackPos(2), 2, 0);
	return 0;
}

int KyraEngine::cmd_setNoDrawShapesFlag(ScriptState *script) {
	debug(3, "cmd_setNoDrawShapesFlag(0x%X) (%d)", script, stackPos(0));
	_animator->_noDrawShapesFlag = stackPos(0);
	return 0;
}

int KyraEngine::cmd_fadeEntirePalette(ScriptState *script) {
	warning("STUB: cmd_fadeEntirePalette");
	return 0;
}

int KyraEngine::cmd_itemOnGroundHere(ScriptState *script) {
	debug(3, "cmd_itemOnGroundHere(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	assert(stackPos(0) < _roomTableSize);
	Room *curRoom = &_roomTable[stackPos(0)];
	for (int i = 0; i < 12; ++i) {
		if (curRoom->itemsTable[i] == stackPos(1))
			return 1;
	}
	return 0;
}

int KyraEngine::cmd_queryCauldronState(ScriptState *script) {
	debug(3, "cmd_queryCauldronState(0x%X) ()", script);
	return _cauldronState;
}

int KyraEngine::cmd_setCauldronState(ScriptState *script) {
	debug(3, "cmd_setCauldronState(0x%X) (%d)", script, stackPos(0));
	_cauldronState = stackPos(0);
	return _cauldronState;
}

int KyraEngine::cmd_queryCrystalState(ScriptState *script) {
	debug(3, "cmd_queryCrystalState(0x%X) (%d)", script, stackPos(0));
	if (!stackPos(0)) {
		return _crystalState[0];
	} else if (stackPos(0) == 1) {
		return _crystalState[1];
	}
	return -1;
}

int KyraEngine::cmd_setCrystalState(ScriptState *script) {
	debug(3, "cmd_setCrystalState(0x%X) (%d)", script, stackPos(0), stackPos(1));
	if (!stackPos(0)) {
		_crystalState[0] = stackPos(1);
	} else if (stackPos(0) == 1) {
		_crystalState[1] = stackPos(1);
	}
	return stackPos(1);
}

int KyraEngine::cmd_setPaletteRange(ScriptState *script) {
	warning("STUB: cmd_setPaletteRange");
	return 0;
}

int KyraEngine::cmd_shrinkBrandonDown(ScriptState *script) {
	debug(3, "cmd_shrinkBrandonDown(0x%X) (%d)", script, stackPos(0));
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
		animRefreshNPC(0);
		delayWithTicks(1);
	}
	delayWithTicks(delayTime); // XXX
	_scaleTable[_currentCharacter->y1] = scaleValue;
	_scaleMode = scaleModeBackUp;
	return 0;
}

int KyraEngine::cmd_growBrandonUp(ScriptState *script) {
	debug(3, "cmd_growBrandonUp(0x%X) ()", script);
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
		animRefreshNPC(0);
		delayWithTicks(1);
	}
	_scaleTable[_currentCharacter->y1] = scaleValue;
	_scaleMode = scaleModeBackUp;
	return 0;
}

int KyraEngine::cmd_setBrandonScaleXAndY(ScriptState *script) {
	debug(3, "cmd_setBrandonScaleXAndY(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	_brandonScaleX = stackPos(0);
	_brandonScaleY = stackPos(1);
	return 0;
}

int KyraEngine::cmd_resetScaleMode(ScriptState *script) {
	debug(3, "cmd_resetScaleMode(0x%X) ()", script);
	_scaleMode = 0;
	return 0;
}

int KyraEngine::cmd_getScaleDepthTableValue(ScriptState *script) {
	debug(3, "cmd_getScaleDepthTableValue(0x%X) (%d)", script, stackPos(0));
	assert(stackPos(0) < ARRAYSIZE(_scaleTable));
	return _scaleTable[stackPos(0)];
}

int KyraEngine::cmd_setScaleDepthTableValue(ScriptState *script) {
	debug(3, "cmd_setScaleDepthTableValue(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	assert(stackPos(0) < ARRAYSIZE(_scaleTable));
	_scaleTable[stackPos(0)] = stackPos(1);
	return stackPos(1);
}

int KyraEngine::cmd_message(ScriptState *script) {
	if (_features & GF_TALKIE) {
		debug(3, "cmd_message(0x%X) (%d, '%s', %d)", script, stackPos(0), stackPosString(1), stackPos(2));
		drawSentenceCommand(stackPosString(1), stackPos(2));
	} else {
		debug(3, "cmd_message(0x%X) ('%s', %d)", script, stackPosString(0), stackPos(1));
		drawSentenceCommand(stackPosString(0), stackPos(1));
	}

	return 0;
}

int KyraEngine::cmd_checkClickOnNPC(ScriptState *script) {
	debug(3, "cmd_checkClickOnNPC(0x%X) (%d, %d)", script, stackPos(0), stackPos(1));
	return checkForNPCScriptRun(stackPos(0), stackPos(1));
}

int KyraEngine::cmd_getFoyerItem(ScriptState *script) {
	debug(3, "cmd_getFoyerItem(0x%X) (%d)", stackPos(0));
	assert(stackPos(0) < ARRAYSIZE(_foyerItemTable));
	return _foyerItemTable[stackPos(0)];
}

int KyraEngine::cmd_setFoyerItem(ScriptState *script) {
	debug(3, "cmd_setFoyerItem(0x%X) (%d, %d)", stackPos(0), stackPos(1));
	assert(stackPos(0) < ARRAYSIZE(_foyerItemTable));
	_foyerItemTable[stackPos(0)] = stackPos(1);
	return stackPos(1);
}

int KyraEngine::cmd_setNoItemDropRegion(ScriptState *script) {
	debug(3, "cmd_setNoItemDropRegion(0x%X) (%d, %d, %d, %d)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	addToNoDropRects(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int KyraEngine::cmd_walkMalcolmOn(ScriptState *script) {
	debug(3, "cmd_walkMalcolmOn(0x%X) ()", script);
	if (!_malcolmFlag)
		_malcolmFlag = 1;
	return 0;
}

int KyraEngine::cmd_passiveProtection(ScriptState *script) {
	debug(3, "cmd_passiveProtection(0x%X) ()", script);
	return 1;
}

int KyraEngine::cmd_setPlayingLoop(ScriptState *script) {
	warning("STUB: cmd_setPlayingLoop");
	return 0;
}

int KyraEngine::cmd_brandonToStoneSequence(ScriptState *script) {
	warning("STUB: cmd_brandonToStoneSequence");
	return 0;
}

int KyraEngine::cmd_brandonHealingSequence(ScriptState *script) {
	debug(3, "cmd_brandonHealingSequence(0x%X) ()", script);
	seq_brandonHealing();
	return 0;
}

int KyraEngine::cmd_protectCommandLine(ScriptState *script) {
	debug(3, "cmd_protectCommandLine(0x%X) (%d)", script, stackPos(0));
	return stackPos(0);
}

int KyraEngine::cmd_pauseMusicSeconds(ScriptState *script) {
	warning("STUB: cmd_pauseMusicSeconds");
	return 0;
}

int KyraEngine::cmd_resetMaskRegion(ScriptState *script) {
	warning("STUB: cmd_resetMaskRegion");
	return 0;
}

int KyraEngine::cmd_setPaletteChangeFlag(ScriptState *script) {
	debug(3, "cmd_setPaletteChangeFlag(0x%X) (%d)", script, stackPos(0));
	_paletteChanged = stackPos(0);
	return _paletteChanged;
}

int KyraEngine::cmd_fillRect(ScriptState *script) {
	debug(3, "cmd_fillRect(0x%X) (%d, %d, %d, %d, %d, 0x%X)", script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int videoPageBackup = _screen->_curPage;
	_screen->_curPage = stackPos(0);
	_screen->fillRect(stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	_screen->_curPage = videoPageBackup;
	return 0;
}

int KyraEngine::cmd_vocUnload(ScriptState *script) {
	debug(3, "cmd_vocUnload(0x%X) ()", script);
	// this should unload all voc files (not needed)
	return 0;
}

int KyraEngine::cmd_vocLoad(ScriptState *script) {
	debug(3, "cmd_vocLoad(0x%X) (%d)", script, stackPos(0));
	// this should load the specified voc file (not needed)
	return 0;
}

int KyraEngine::cmd_dummy(ScriptState *script) {
	debug(3, "cmd_dummy(0x%X) ()", script);
	return 0;
}

} // end of namespace Kyra
