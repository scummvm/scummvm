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
#include "kyra/screen_v2.h"
#include "kyra/wsamovie.h"

#include "common/func.h"

namespace Kyra {

void KyraEngine_v2::enterNewScene(uint16 newScene, int facing, int unk1, int unk2, int unk3) {
	// XXX
	_screen->hideMouse();
	
	if (!unk3) {
		//updateSpecialItems();
		//displayInvWsaLastFrame();
	}
	
	if (unk1) {
		int x = _mainCharacter.x1;
		int y = _mainCharacter.y1;

		switch (facing) {
		case 0:
			y -= 6;
			break;

		case 2:
			x = 335;
			break;

		case 4:
			y = 147;
			break;

		case 6:
			x = -16;
			break;
		
		default:
			break;
		}
		
		moveCharacter(facing, x, y);
	}
	
	//XXX sound
	
	_unkFlag1 = false;
	
	if (!unk3) {
		_scriptInterpreter->initScript(&_sceneScriptState, &_sceneScriptData);
		_scriptInterpreter->startScript(&_sceneScriptState, 5);
		while (_scriptInterpreter->validScript(&_sceneScriptState))
			_scriptInterpreter->runScript(&_sceneScriptState);
	}
	
	Common::for_each(_wsaSlots, _wsaSlots+ARRAYSIZE(_wsaSlots), Common::mem_fun(&WSAMovieV2::close));
	_specialExitCount = 0;
	memset(_specialExitTable, -1, sizeof(_specialExitTable));
	
	_mainCharacter.sceneId = newScene;
	_sceneList[newScene].flags &= ~1;
	loadScenePal();
	unloadScene();
	loadSceneMsc();
	
	SceneDesc &scene = _sceneList[newScene];
	_sceneExit1 = scene.exit1;
	_sceneExit2 = scene.exit2;
	_sceneExit3 = scene.exit3;
	_sceneExit4 = scene.exit4;
	
	//XXX sound
	
	startSceneScript(unk3);
	
	if (_overwriteSceneFacing) {
		facing = _mainCharacter.facing;
		_overwriteSceneFacing = false;
	}
	
	enterNewSceneUnk1(facing, unk2, unk3);
	
	setTimer1DelaySecs(-1);
	_sceneScriptState.regs[3] = 1;
	enterNewSceneUnk2(unk3);
	_screen->showMouse();
	_unk5 = 0;
	//setNextIdleAnimTimer();
}

void KyraEngine_v2::enterNewSceneUnk1(int facing, int unk1, int unk2) {
	int x = 0, y = 0;
	int x2 = 0, y2 = 0;
	bool needProc = true;
	
	if (_mainCharX == -1 && _mainCharY == -1) {
		switch (facing+1) {
		case 1: case 2: case 8:
			x2 = _sceneEnterX3;
			y2 = _sceneEnterY3;
			break;
		
		case 3:
			x2 = _sceneEnterX4;
			y2 = _sceneEnterY4;
			break;
		
		case 4: case 5: case 6:
			x2 = _sceneEnterX1;
			y2 = _sceneEnterY1;
			break;
		
		case 7:
			x2 = _sceneEnterX2;
			y2 = _sceneEnterY2;
			break;
		
		default:
			x2 = y2 = -1;
			break;
		}
		
		if (x2 >= 316)
			x2 = 312;
		if (y2 >= 141)
			y2 = 139;
		if (x2 <= 4)
			x2 = 8;
	}
	
	if (_mainCharX >= 0) {
		x = x2 = _mainCharX;
		needProc = false;
	}
	
	if (_mainCharY >= 0) {
		y = y2 = _mainCharY;
		needProc = false;
	}
	
	_mainCharX = _mainCharY = -1;
	
	if (unk1 && needProc) {
		x = x2;
		y = y2;
		
		switch (facing) {
		case 0:
			y2 = 147;
			break;
		
		case 2:
			x2 = -16;
			break;
		
		case 4:
			y2 = y - 4;
			break;
		
		case 6:
			x2 = 335;
			break;
			
		default:
			break;
		}
	}
	
	x2 &= ~3;
	x &= ~3;
	y2 &= ~1;
	y &= ~1;

	_mainCharacter.facing = facing;
	_mainCharacter.x1 = _mainCharacter.x2 = x2;
	_mainCharacter.y1 = _mainCharacter.y2 = y2;
	initSceneAnims(unk2);
	
	if (!unk2) {
		//XXX sound
	}
	
	if (unk1 && !unk2 && _mainCharacter.animFrame != 32)
		moveCharacter(facing, x, y);
}

void KyraEngine_v2::enterNewSceneUnk2(int unk1) {
	_unk3 = -1;
	
	if (_mainCharX == -1 && _mainCharY == -1 && _mainCharacter.sceneId != 61 &&
		!queryGameFlag(0x1F1) && !queryGameFlag(0x192) && !queryGameFlag(0x193) &&
		_mainCharacter.sceneId != 70 && !queryGameFlag(0x159) && _mainCharacter.sceneId != 37) {
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
		refreshAnimObjectsIfNeed();
	}
	
	if (!unk1) {
		runSceneScript4(0);
		//XXX sub_27158
	}
	
	_unk4 = 0;
	_unk3 = -1;
}

int KyraEngine_v2::trySceneChange(int *moveTable, int unk1, int updateChar) {
	bool running = true;
	bool unkFlag = false;
	int8 updateType = -1;
	int changedScene = 0;
	const int *moveTableStart = moveTable;
	_unk4 = 0;
	while (running) {
		if (*moveTable >= 0 && *moveTable <= 7) {
			_mainCharacter.facing = getOppositeFacingDirection(*moveTable);
			unkFlag = true;
		} else {
			if (*moveTable == 8) {
				running = false;
			} else {
				++moveTable;
				unkFlag = false;
			}
		}
		
		if (checkSceneChange()) {
			running = false;
			changedScene = 1;
		}
		
		if (unk1) {
			//XXX
		}
		
		if (!unkFlag || !running)
			continue;

		int ret = 0;
		if (moveTable == moveTableStart || moveTable[1] == 8)
			ret = updateCharPos(0);
		else
			ret = updateCharPos(moveTable);

		if (ret)
			++moveTable;

		++updateType;
		if (!updateType) {
			update();
		} else if (updateType == 1) {
			refreshAnimObjectsIfNeed();
			updateType = -1;
		}
	}
	
	if (updateChar)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	
	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
	
	if (!changedScene && !_unk4) {
		//XXX
	}
	return changedScene;
}

int KyraEngine_v2::checkSceneChange() {
	SceneDesc &curScene = _sceneList[_mainCharacter.sceneId];
	int charX = _mainCharacter.x1, charY = _mainCharacter.y1;
	int facing = 0;
	int process = 0;
	
	if (_screen->getLayer(charX, charY) == 1 && _unk3 == -6) {
		facing = 0;
		process = 1;
	} else if (charX >= 316 && _unk3 == -5) {
		facing = 2;
		process = 1;
	} else if (charY >= 142 && _unk3 == -4) {
		facing = 4;
		process = 1;
	} else if (charX <= 4 && _unk3 == -3) {
		facing = 6;
		process = 1;
	}
	
	if (!process)
		return 0;
	
	uint16 newScene = 0xFFFF;
	switch (facing) {
	case 0:
		newScene = curScene.exit1;
		break;
	
	case 2:
		newScene = curScene.exit2;
		break;
	
	case 4:
		newScene = curScene.exit3;
		break;
	
	case 6:
		newScene = curScene.exit4;
		break;
	
	default:
		newScene = _mainCharacter.sceneId;
		break;
	}
	
	if (newScene == 0xFFFF)
		return 0;
	
	enterNewScene(newScene, facing, 1, 1, 0);
	return 1;
}

void KyraEngine_v2::unloadScene() {
	_scriptInterpreter->unloadScript(&_sceneScriptData);
	freeSceneShapePtrs();
	freeSceneAnims();
}

void KyraEngine_v2::loadScenePal() {
	uint16 sceneId = _mainCharacter.sceneId;
	memcpy(_screen->getPalette(1), _screen->getPalette(0), 768);
	
	char filename[14];
	strcpy(filename, _sceneList[sceneId].filename);
	strcat(filename, ".COL");
	_screen->loadBitmap(filename, 3, 3, 0);
	memcpy(_screen->getPalette(1), _screen->getCPagePtr(3), 384);
	memset(_screen->getPalette(1), 0, 3);
	memcpy(_scenePal, _screen->getCPagePtr(3)+336, 432);
}

void KyraEngine_v2::loadSceneMsc() {
	uint16 sceneId = _mainCharacter.sceneId;
	char filename[14];
	strcpy(filename, _sceneList[sceneId].filename);
	strcat(filename, ".MSC");
	_screen->loadBitmap(filename, 3, 5, 0);
}

void KyraEngine_v2::startSceneScript(int unk1) {
	uint16 sceneId = _mainCharacter.sceneId;
	char filename[14];
	
	strcpy(filename, _sceneList[sceneId].filename);
	if (sceneId == 68 && (queryGameFlag(0x1BC) || queryGameFlag(0x1DC)))
		strcpy(filename, "DOORX");	
	strcat(filename, ".CPS");

	_screen->loadBitmap(filename, 3, 3, 0);
	resetScaleTable();
	_useCharPal = false;
	memset(_charPalTable, 0, sizeof(_charPalTable));
	//XXX _unkTable33
	memset(_specialSceneScriptState, 0, sizeof(_specialSceneScriptState));

	_sceneEnterX1 = 160;
	_sceneEnterY1 = 0;
	_sceneEnterX2 = 296;
	_sceneEnterY2 = 72;
	_sceneEnterX3 = 160;
	_sceneEnterY3 = 128;
	_sceneEnterX4 = 24;
	_sceneEnterY4 = 72;
	
	_sceneCommentString = "Undefined scene comment string!";
	_scriptInterpreter->initScript(&_sceneScriptState, &_sceneScriptData);
	
	strcpy(filename, _sceneList[sceneId].filename);
	strcat(filename, ".");
	strcat(filename, _scriptLangExt[_lang]);
	
	assert(_res->getFileSize(filename));
	_scriptInterpreter->loadScript(filename, &_sceneScriptData, &_opcodes);
	runSceneScript7();
		
	_scriptInterpreter->startScript(&_sceneScriptState, 0);
	_sceneScriptState.regs[0] = sceneId;
	_sceneScriptState.regs[5] = unk1;
	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);

	memcpy(_gamePlayBuffer, _screen->getCPagePtr(3), 46080);
	
	for (int i = 0; i < 10; ++i) {
		_scriptInterpreter->initScript(&_sceneSpecialScripts[i], &_sceneScriptData);
		_scriptInterpreter->startScript(&_sceneSpecialScripts[i], i+8);
		_sceneSpecialScriptsTimer[i] = 0;
	}
	
	_sceneEnterX1 &= ~3;
	_sceneEnterX2 &= ~3;
	_sceneEnterX3 &= ~3;
	_sceneEnterX4 &= ~3;
	_sceneEnterY1 &= ~1;
	_sceneEnterY2 &= ~1;
	_sceneEnterY3 &= ~1;
	_sceneEnterY4 &= ~1;
}

void KyraEngine_v2::runSceneScript2() {
	_scriptInterpreter->initScript(&_sceneScriptState, &_sceneScriptData);
	_sceneScriptState.regs[4] = _itemInHand;
	_scriptInterpreter->startScript(&_sceneScriptState, 2);
	
	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);
}

void KyraEngine_v2::runSceneScript4(int unk1) {
	_sceneScriptState.regs[4] = _itemInHand;
	_sceneScriptState.regs[5] = unk1;

	_scriptInterpreter->startScript(&_sceneScriptState, 4);
	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);
}

void KyraEngine_v2::runSceneScript7() {
	int oldPage = _screen->_curPage;
	_screen->_curPage = 2;
	
	_scriptInterpreter->startScript(&_sceneScriptState, 7);
	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);

	_screen->_curPage = oldPage;
}

void KyraEngine_v2::initSceneAnims(int unk1) {
	for (int i = 0; i < ARRAYSIZE(_animObjects); ++i)
		_animObjects[i].enabled = 0;
	
	bool animInit = false;
	
	AnimObj *animState = &_animObjects[0];
	
	if (_mainCharacter.animFrame != 32)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	
	animState->enabled = 1;
	animState->xPos1 = _mainCharacter.x1;
	animState->yPos1 = _mainCharacter.y1;
	animState->shapePtr = _defaultShapeTable[_mainCharacter.animFrame];
	animState->shapeIndex1 = animState->shapeIndex2 = _mainCharacter.animFrame;
	
	int frame = _mainCharacter.animFrame - 9;
	int shapeX = _shapeDescTable[frame].xAdd;
	int shapeY = _shapeDescTable[frame].yAdd;
	
	animState->xPos2 = _mainCharacter.x1;
	animState->yPos2 = _mainCharacter.y1;
	
	_charScaleX = _charScaleY = getScale(_mainCharacter.x1, _mainCharacter.y1);
	
	int shapeXScaled = (shapeX * _charScaleX) >> 8;
	int shapeYScaled = (shapeY * _charScaleY) >> 8;
	
	animState->xPos2 += shapeXScaled;
	animState->yPos2 += shapeYScaled;
	animState->xPos3 = animState->xPos2;
	animState->yPos3 = animState->yPos2;
	animState->needRefresh = 1;
	animState->unk8 = 1;

	_animList = 0;
	
	AnimObj *charAnimState = animState;
	
	for (int i = 0; i < 10; ++i) {
		animState = &_animObjects[i+1];
		animState->enabled = 0;
		animState->needRefresh = 0;
		animState->unk8 = 0;
		
		if (_sceneAnims[i].flags & 1) {
			animState->enabled = 1;
			animState->needRefresh = 1;
			animState->unk8 = 1;
		}
		
		animState->animFlags = _sceneAnims[i].flags & 8;
		
		if (_sceneAnims[i].flags & 2)
			animState->flags = 0x800;
		else
			animState->flags = 0;
		
		if (_sceneAnims[i].flags & 4)
			animState->flags |= 1;
		
		animState->xPos1 = _sceneAnims[i].x;
		animState->yPos1 = _sceneAnims[i].y;
		
		if (_sceneAnims[i].flags & 0x20)
			animState->shapePtr = _sceneShapeTable[_sceneAnims[i].shapeIndex];
		else
			animState->shapePtr = 0;
		
		if (_sceneAnims[i].flags & 0x40) {
			animState->shapeIndex3 = _sceneAnims[i].shapeIndex;
			animState->animNum = i;
		} else {
			animState->shapeIndex3 = 0xFFFF;
			animState->animNum = 0xFFFF;
		}
		
		animState->shapeIndex2 = 0xFFFF;
		
		animState->xPos3 = animState->xPos2 = _sceneAnims[i].x2;
		animState->yPos3 = animState->yPos2 = _sceneAnims[i].y2;
		animState->width = _sceneAnims[i].width;
		animState->height = _sceneAnims[i].height;
		animState->width2 = animState->height2 = _sceneAnims[i].specialSize;
		
		if (_sceneAnims[i].flags & 1) {
			if (animInit) {
				_animList = addToAnimListSorted(_animList, animState);
			} else {
				_animList = initAnimList(_animList, animState);
				animInit = true;
			}
		}
	}
	
	if (animInit) {
		_animList = addToAnimListSorted(_animList, charAnimState);
	} else {
		_animList = initAnimList(_animList, charAnimState);
		animInit = true;
	}
	
	for (int i = 0; i < 30; ++i) {
		animState = &_animObjects[i+11];
		
		uint16 shapeIndex = _itemList[i].id;
		if (shapeIndex == 0xFFFF || _itemList[i].sceneId != _mainCharacter.sceneId) {
			animState->enabled = 0;
			animState->needRefresh = 0;
			animState->unk8 = 0;
		} else {
			animState->xPos1 = _itemList[i].x;
			animState->yPos1 = _itemList[i].y;
			animState->shapePtr = _defaultShapeTable[64+shapeIndex];
			animState->shapeIndex1 = animState->shapeIndex2 = shapeIndex+64;
			
			animState->xPos2 = _itemList[i].x;
			animState->yPos2 = _itemList[i].y;
			int objectScale = getScale(animState->xPos2, animState->yPos2);
			
			const uint8 *shape = getShapePtr(animState->shapeIndex1);
			animState->xPos2 -= (_screen->getShapeScaledWidth(shape, objectScale) >> 1);
			animState->yPos2 -= (_screen->getShapeScaledHeight(shape, objectScale) >> 1);
			animState->xPos3 = animState->xPos2;
			animState->yPos3 = animState->yPos2;
			
			animState->enabled = 1;
			animState->needRefresh = 1;
			animState->unk8 = 1;
			
			if (animInit) {
				_animList = addToAnimListSorted(_animList, animState);
			} else {
				_animList = initAnimList(_animList, animState);
				animInit = true;
			}
		}
	}
	
	_animObjects[0].unk8 = 1;
	_animObjects[0].needRefresh = 1;
	
	for (int i = 1; i < 41; ++i) {
		if (_animObjects[i].enabled) {
			_animObjects[i].needRefresh = 1;
			_animObjects[i].unk8 = 1;
		}
	}
	
	restorePage3();
	drawAnimObjects();
	_screen->hideMouse();
	initSceneScreen(unk1);
	_screen->showMouse();
	refreshAnimObjects(0);
}

void KyraEngine_v2::initSceneScreen(int unk1) {
	if (_unkSceneScreenFlag1) {
		_screen->copyRegion(0, 0, 0, 0, 320, 144, 2, 0);
		return;
	}

	if (_noScriptEnter) {
		memset(_screen->getPalette(0), 0, 384);
		_screen->setScreenPalette(_screen->getPalette(0));
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 144, 2, 0);

	if (_noScriptEnter)
		memcpy(_screen->getPalette(0), _screen->getPalette(1), 384);

	updateCharPal(0);

	_scriptInterpreter->startScript(&_sceneScriptState, 3);
	_sceneScriptState.regs[5] = unk1;
	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);
}

void KyraEngine_v2::updateSpecialSceneScripts() {
	uint32 nextTime = _system->getMillis() + _tickLength;
	const int startScript = _lastProcessedSceneScript;

	while (_system->getMillis() <= nextTime) {
		if (_sceneSpecialScriptsTimer[_lastProcessedSceneScript] <= _system->getMillis() && 
			!_specialSceneScriptState[_lastProcessedSceneScript]) {
			_specialSceneScriptRunFlag = true;
			
			while (_specialSceneScriptRunFlag && _sceneSpecialScriptsTimer[_lastProcessedSceneScript] <= _system->getMillis())
				_specialSceneScriptRunFlag = _scriptInterpreter->runScript(&_sceneSpecialScripts[_lastProcessedSceneScript]) != 0;
		}
		
		if (!_scriptInterpreter->validScript(&_sceneSpecialScripts[_lastProcessedSceneScript])) {
			_scriptInterpreter->startScript(&_sceneSpecialScripts[_lastProcessedSceneScript], 8+_lastProcessedSceneScript);
			_specialSceneScriptRunFlag = false;
		}
		
		++_lastProcessedSceneScript;
		if (_lastProcessedSceneScript >= 10)
			_lastProcessedSceneScript = 0;
		
		if (_lastProcessedSceneScript == startScript)
			return;
	}
}

void KyraEngine_v2::freeSceneShapePtrs() {
	for (int i = 0; i < ARRAYSIZE(_sceneShapeTable); ++i)
		delete [] _sceneShapeTable[i];
	memset(_sceneShapeTable, 0, sizeof(_sceneShapeTable));
}

void KyraEngine_v2::freeSceneAnims() {
	Common::for_each(_sceneAnimMovie, _sceneAnimMovie+ARRAYSIZE(_sceneAnimMovie), Common::mem_fun(&WSAMovieV2::close));
}

#pragma mark -
#pragma mark - Pathfinder
#pragma mark -

int KyraEngine_v2::findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::findWay(%d, %d, %d, %d, %p, %d)", x, y, toX, toY, (const void *)moveTable, moveTableSize);
	x &= ~3; toX &= ~3;
	y &= ~1; toY &= ~1;
	int size = KyraEngine::findWay(x, y, toX, toY, moveTable, moveTableSize);
	if (size) {
/*
		int temp = pathfinderUnk1(moveTable);
		temp = pathfinderUnk3(temp, x, y);
		pathfinderUnk5(moveTable, temp, x, y, moveTableSize);
*/
	}
	return getMoveTableSize(moveTable);
}

bool KyraEngine_v2::lineIsPassable(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::lineIsPassable(%d, %d)", x, y);
	static int unkTable[] = { 1, 1, 1, 1, 1, 2, 4, 6, 8 };

	if (_pathfinderFlag & 2) {
		if (x >= 320)
			return false;
	}
	
	if (_pathfinderFlag & 4) {
		if (y >= 144)
			return false;
	}
	
	if (_pathfinderFlag & 8) {
		if (x < 0)
			return false;
	}
	
	if (y > 143)
		return false;

	int unk1 = unkTable[getScale(x, y) >> 5];
	
	if (y < 0)
		y = 0;
	x -= unk1 >> 1;
	if (x < 0)
		x = 0;
	int x2 = x + unk1;
	if (x2 > 320)
		x2 = 320;
	
	for (;x < x2; ++x)
		if (!_screen->getShapeFlag1(x, y))
			return false;

	return true;
}

bool KyraEngine_v2::directLinePassable(int x, int y, int toX, int toY) {
	while (x != toX && y != toY) {
		int facing = getFacingFromPointToPoint(x, y, toX, toY);
		x += _addXPosTable[facing];
		y += _addYPosTable[facing];
		if (!_screen->getShapeFlag1(x, y))
			return false;
	}
	return true;
}

int KyraEngine_v2::pathfinderUnk1(int *moveTable) {
	bool breakLoop = false;
	int *moveTableCur = moveTable;
	int oldEntry = *moveTableCur, curEntry = *moveTableCur;
	int oldX = 0, newX = 0, oldY = 0, newY = 0;
	int lastEntry = 0;
	lastEntry = pathfinderUnk2(lastEntry, 0, 0);
	
	while (*moveTableCur != 8) {
		oldEntry = curEntry;
		
		while (true) {
			curEntry = *moveTableCur;
			if (curEntry >= 0 && curEntry <= 7)
				break;
			
			if (curEntry == 8) {
				breakLoop = true;
				break;
			} else {
				++moveTableCur;
			}
		}
		
		if (breakLoop)
			break;
		
		oldX = newX;
		oldY = newY;
		
		newX += _addXPosTable[curEntry];
		newY += _addYPosTable[curEntry];
		
		int temp = ABS(curEntry - oldEntry);
		if (temp > 4) {
			temp = 8 - temp;
		}
		
		if (temp > 1 || oldEntry != curEntry)
			lastEntry = pathfinderUnk2(lastEntry, oldX, oldY);
	
		++moveTableCur;
	}
	
	lastEntry = pathfinderUnk2(lastEntry, newX, newY);
	_pathfinderUnkTable1[lastEntry*2+0] = -1;
	_pathfinderUnkTable1[lastEntry*2+1] = -1;
	return lastEntry;
}

int KyraEngine_v2::pathfinderUnk2(int index, int v1, int v2) {
	_pathfinderUnkTable1[index<<1] = v1;
	_pathfinderUnkTable1[(index<<1)+1] = v2;
	++index;
	if (index >= 199)
		--index;
	return index;
}

int KyraEngine_v2::pathfinderUnk3(int tableLen, int x, int y) {
	int x1 = 0, y1 = 0;
	int x2 = 0, y2 = 0;
	int lastEntry = 0;
	int index2 = tableLen-1, index1 = 0;
	while (index2 > index1) {
		x1 = _pathfinderUnkTable1[index1*2+0] + x;
		y1 = _pathfinderUnkTable1[index1*2+1] + y;
		x2 = _pathfinderUnkTable1[index2*2+0] + x;
		y2 = _pathfinderUnkTable1[index2*2+0] + x;
		
		if (directLinePassable(x1, y1, x2, y2)) {
			lastEntry = pathfinderUnk4(lastEntry, index2);
			if (tableLen-1 == index2)
				break;
			index1 = index2;
			index2 = tableLen-1;
		} else if (index1+1 == index2) {
			lastEntry = pathfinderUnk4(lastEntry, index2);
			index1 = index2;
			index2 = tableLen-1;
		} else {
			--index2;
		}
	}
	return lastEntry;
}

int KyraEngine_v2::pathfinderUnk4(int index, int v) {
	_pathfinderUnkTable2[index] = v;
	++index;
	if (index >= 199)
		--index;
	return index;
}

void KyraEngine_v2::pathfinderUnk5(int *moveTable, int tableLen, int x, int y, int moveTableSize) {
	int x1 = 0, y1 = 0;
	int x2 = 0, y2 = 0;
	int index1 = 0, index2 = 0;
	int sizeLeft = moveTableSize;
	for (int i = 0; i < tableLen; ++i) {
		index2 = _pathfinderUnkTable2[i];
		x1 = _pathfinderUnkTable1[index1*2+0] + x;
		y1 = _pathfinderUnkTable1[index1*2+1] + y;
		x2 = _pathfinderUnkTable1[index2*2+0] + x;
		y2 = _pathfinderUnkTable1[index2*2+0] + x;
		
		int wayLen = findWay(x1, y1, x2, y2, moveTable, sizeLeft);
		moveTable += wayLen;
		sizeLeft -= wayLen;	// unlike the original we want to be sure that the size left is correct
		index1 = index2;
	}
}

} // end of namespace Kyra
