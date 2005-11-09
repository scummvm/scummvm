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
#include "common/system.h"

namespace Kyra {

void ScriptHelper::c1_jmpTo() {
	_curScript->ip = _curScript->dataPtr->data + (_parameter << 1);
}

void ScriptHelper::c1_setRetValue() {
	_curScript->retValue = _parameter;
}

void ScriptHelper::c1_pushRetOrPos() {
	switch (_parameter) {
		case 0:
			_curScript->stack[--_curScript->sp] = _curScript->retValue;
		break;
		
		case 1:
			_curScript->stack[--_curScript->sp] = (_curScript->ip - _curScript->dataPtr->data) / 2 + 1;
			_curScript->stack[--_curScript->sp] = _curScript->bp;
			_curScript->bp = _curScript->sp + 2;
		break;
		
		default:
			_continue = false;
			_curScript->ip = 0;
		break;
	}
}

void ScriptHelper::c1_push() {
	_curScript->stack[--_curScript->sp] = _parameter;
}

void ScriptHelper::c1_pushVar() {
	_curScript->stack[--_curScript->sp] = _curScript->variables[_parameter];
}

void ScriptHelper::c1_pushBPNeg() {
	_curScript->stack[--_curScript->sp] = _curScript->stack[(-(int32)(_parameter + 2)) + _curScript->bp];
}

void ScriptHelper::c1_pushBPAdd() {
	_curScript->stack[--_curScript->sp] = _curScript->stack[(_parameter - 1) + _curScript->bp];
}

void ScriptHelper::c1_popRetOrPos() {
	switch (_parameter) {
		case 0:
			_curScript->retValue = _curScript->stack[++_curScript->sp-1];
		break;
		
		case 1:
			if (_curScript->sp >= 60) {
				_continue = false;
				_curScript->ip = 0;
			} else {
				_curScript->bp = _curScript->stack[++_curScript->sp-1];
				_curScript->ip = _curScript->dataPtr->data + (_curScript->stack[++_curScript->sp-1] << 1);
			}
		break;
		
		default:
			_continue = false;
			_curScript->ip = 0;
		break;
	}
}

void ScriptHelper::c1_popVar() {
	_curScript->variables[_parameter] = _curScript->stack[++_curScript->sp-1];
}

void ScriptHelper::c1_popBPNeg() {
	_curScript->stack[(-(int32)(_parameter + 2)) + _curScript->bp] = _curScript->stack[++_curScript->sp-1];
}

void ScriptHelper::c1_popBPAdd() {
	_curScript->stack[(_parameter - 1) + _curScript->bp] = _curScript->stack[++_curScript->sp-1];
}

void ScriptHelper::c1_addSP() {
	_curScript->sp += _parameter;
}

void ScriptHelper::c1_subSP() {
	_curScript->sp -= _parameter;
}

void ScriptHelper::c1_execOpcode() {
	assert((int)_parameter < _curScript->dataPtr->opcodeSize);
	if (_curScript->dataPtr->opcodes[_parameter] == &KyraEngine::cmd_dummy)
		debug("calling unimplemented opcode(0x%.02X)", _parameter);
	_curScript->retValue = (_vm->*_curScript->dataPtr->opcodes[_parameter])(_curScript);
}

void ScriptHelper::c1_ifNotJmp() {
	if (_curScript->stack[++_curScript->sp-1] != 0) {
		_parameter &= 0x7FFF;
		_curScript->ip = _curScript->dataPtr->data + (_parameter << 1);
	}
}

void ScriptHelper::c1_negate() {
	int16 value = _curScript->stack[_curScript->sp];
	switch (_parameter) {
		case 0:
			if (!value) {
				_curScript->stack[_curScript->sp] = 0;
			} else {
				_curScript->stack[_curScript->sp] = 1;
			}
		break;
		
		case 1:
			_curScript->stack[_curScript->sp] = -value;
		break;
		
		case 2:
			_curScript->stack[_curScript->sp] = ~value;
		break;
		
		default:
			_continue = false;
		break;
	}
}

void ScriptHelper::c1_eval() {
	int16 ret = 0;
	bool error = false;
	
	int16 val1 = _curScript->stack[++_curScript->sp-1];
	int16 val2 = _curScript->stack[++_curScript->sp-1];
	
	switch (_parameter) {
		case 0:
			if (!val2 || !val1) {
				ret = 0;
			} else {
				ret = 1;
			}
		break;
		
		case 1:
			if (val2 || val1) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 2:
			if (val1 == val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 3:
			if (val1 != val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 4:
			if (val1 > val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 5:
			if (val1 >= val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 6:
			if (val1 < val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 7:
			if (val1 <= val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 8:
			ret = val1 + val2;
		break;
		
		case 9:
			ret = val2 - val1;
		break;
		
		case 10:
			ret = val1 * val2;
		break;
		
		case 11:
			ret = val2 / val1;
		break;
		
		case 12:
			ret = val2 >> val1;
		break;
		
		case 13:
			ret = val2 << val1;
		break;
		
		case 14:
			ret = val1 & val2;
		break;
		
		case 15:
			ret = val1 | val2;
		break;
		
		case 16:
			ret = val2 % val1;
		break;
		
		case 17:
			ret = val1 ^ val2;
		break;
		
		default:
			warning("Unknown evaluate func: %d", _parameter);
			error = true;
		break;
	}
	
	if (error) {
		_curScript->ip = 0;
		_continue = false;
	} else {
		_curScript->stack[--_curScript->sp] = ret;
	}
}

void ScriptHelper::c1_setRetAndJmp() {
	if (_curScript->sp >= 60) {
		_continue = false;
		_curScript->ip = 0;
	} else {
		_curScript->retValue = _curScript->stack[++_curScript->sp-1];
		uint16 temp = _curScript->stack[++_curScript->sp-1];
		_curScript->stack[60] = 0;
		_curScript->ip = &_curScript->dataPtr->data[temp*2];
	}
}

#pragma mark -
#pragma mark - Opcode implementations
#pragma mark -

#define stackPos(x) script->stack[script->sp+x]

int KyraEngine::cmd_magicInMouseItem(ScriptState *script) {
	warning("STUB: cmd_magicInMouseItem");
	return 0;
}

int KyraEngine::cmd_characterSays(ScriptState *script) {
	warning("STUB: cmd_characterSays");
	return 0;
}

int KyraEngine::cmd_pauseTicks(ScriptState *script) {
	warning("STUB: cmd_pauseTicks");
	return 0;
}

int KyraEngine::cmd_drawSceneAnimShape(ScriptState *script) {
	debug(9, "cmd_drawSceneAnimShape(0x%X)", script);
	_screen->drawShape(stackPos(4), _sprites->getSceneShape(stackPos(0)), stackPos(1), stackPos(2), 0, stackPos(3));
	return 0;
}

int KyraEngine::cmd_queryGameFlag(ScriptState *script) {
	debug(9, "cmd_queryGameFlag(0x%X)", script);
	return queryGameFlag(stackPos(0));
}

int KyraEngine::cmd_setGameFlag(ScriptState *script) {
	debug(9, "cmd_setGameFlag(0x%X)", script);
	return setGameFlag(stackPos(0));
}

int KyraEngine::cmd_resetGameFlag(ScriptState *script) {
	debug(9, "cmd_resetGameFlag(0x%X)", script);
	return resetGameFlag(stackPos(0));
}

int KyraEngine::cmd_runNPCScript(ScriptState *script) {
	warning("STUB: cmd_runNPCScript");
	return 0;
}

int KyraEngine::cmd_setSpecialExitList(ScriptState *script) {
	debug(9, "cmd_setSpecialExitList(0x%X)", script);
	
	for (int i = 0; i < 10; ++i) {
		_exitList[i] = stackPos(i);
	}
	_exitListPtr = _exitList;
	
	return 0;
}

int KyraEngine::cmd_blockInWalkableRegion(ScriptState *script) {
	debug(9, "cmd_blockInWalkableRegion(0x%X)", script);
	blockOutRegion(stackPos(0), stackPos(1), stackPos(2)-stackPos(0)+1, stackPos(3)-stackPos(1)+1);
	return 0;
}

int KyraEngine::cmd_blockOutWalkableRegion(ScriptState *script) {
	debug(9, "cmd_blockOutWalkableRegion(0x%X)", script);
	blockOutRegion(stackPos(0), stackPos(1), stackPos(2)-stackPos(0)+1, stackPos(3)-stackPos(1)+1);
	return 0;
}

int KyraEngine::cmd_walkPlayerToPoint(ScriptState *script) {
	debug(9, "cmd_walkPlayerToPoint(0x%X)", script);
	// if !stackPos(2)
	// XXX
	int reinitScript = handleSceneChange(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	// XXX
	if (reinitScript) {
		_scriptInterpreter->initScript(script, script->dataPtr);
	}
	return 0;
}

int KyraEngine::cmd_dropItemInScene(ScriptState *script) {
	debug(9, "cmd_dropItemInScene(0x%X)", script);
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
		
		warning("PARTIALLY IMPLEMENTED: cmd_dropItemInScene");
		// XXX animAddGameItem
		// XXX updateAllObjectShapes
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
	warning("STUB: cmd_drawAnimShapeIntoScene");
	return 0;
}

int KyraEngine::cmd_createMouseItem(ScriptState *script) {
	warning("STUB: cmd_createMouseItem");
	return 0;
}

int KyraEngine::cmd_savePageToDisk(ScriptState *script) {
	warning("STUB: cmd_savePageToDisk");
	return 0;
}

int KyraEngine::cmd_sceneAnimOn(ScriptState *script) {
	debug(9, "cmd_sceneAnimOn(0x%X)", script);
	_sprites->enableAnim(stackPos(0));
	return 0;
}

int KyraEngine::cmd_sceneAnimOff(ScriptState *script) {
	debug(9, "cmd_sceneAnimOff(0x%X)", script);
	_sprites->disableAnim(stackPos(0));
	return 0;
}

int KyraEngine::cmd_getElapsedSeconds(ScriptState *script) {
	warning("STUB: cmd_getElapsedSeconds");
	return 0;
}

int KyraEngine::cmd_mouseIsPointer(ScriptState *script) {
	warning("STUB: cmd_mouseIsPointer");
	return 0;
}

int KyraEngine::cmd_destroyMouseItem(ScriptState *script) {
	warning("STUB: cmd_destroyMouseItem");
	return 0;
}

int KyraEngine::cmd_runSceneAnimUntilDone(ScriptState *script) {
	warning("STUB: cmd_runSceneAnimUntilDone");
	return 0;
}

int KyraEngine::cmd_fadeSpecialPalette(ScriptState *script) {
	warning("STUB: cmd_fadeSpecialPalette");
	return 0;
}

int KyraEngine::cmd_playAdlibSound(ScriptState *script) {
	warning("STUB: cmd_playAdlibSound");
	return 0;
}

int KyraEngine::cmd_playAdlibScore(ScriptState *script) {
	warning("STUB: cmd_playAdlibScore");
	return 0;
}

int KyraEngine::cmd_phaseInSameScene(ScriptState *script) {
	warning("STUB: cmd_phaseInSameScene");
	return 0;
}

int KyraEngine::cmd_setScenePhasingFlag(ScriptState *script) {
	warning("STUB: cmd_setScenePhasingFlag");
	return 0;
}

int KyraEngine::cmd_resetScenePhasingFlag(ScriptState *script) {
	warning("STUB: cmd_resetScenePhasingFlag");
	return 0;
}

int KyraEngine::cmd_queryScenePhasingFlag(ScriptState *script) {
	warning("STUB: cmd_queryScenePhasingFlag");
	return 0;
}

int KyraEngine::cmd_sceneToDirection(ScriptState *script) {
	warning("STUB: cmd_sceneToDirection");
	return 0;
}

int KyraEngine::cmd_setBirthstoneGem(ScriptState *script) {
	debug(9, "cmd_setBirthstoneGem(0x%X)", script);
	int index = stackPos(0);
	if (index < 4 && index >= 0) {
		_birthstoneGemTable[index] = stackPos(1);
		return 1;
	}
	return 0;
}

int KyraEngine::cmd_placeItemInGenericMapScene(ScriptState *script) {
	debug(9, "cmd_placeItemInGenericMapScene(0x%X)", script);
	placeItemInGenericMapScene(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine::cmd_setBrandonStatusBit(ScriptState *script) {
	warning("STUB: cmd_setBrandonStatusBit");
	return 0;
}

int KyraEngine::cmd_pauseSeconds(ScriptState *script) {
	debug(9, "cmd_pauseSeconds(0x%X)", script);
	_system->delayMillis(stackPos(0)*1000);
	return 0;
}

int KyraEngine::cmd_getCharactersLocation(ScriptState *script) {
	warning("STUB: cmd_getCharactersLocation");
	return 0;
}

int KyraEngine::cmd_runNPCSubscript(ScriptState *script) {
	warning("STUB: cmd_runNPCSubscript");
	return 0;
}

int KyraEngine::cmd_magicOutMouseItem(ScriptState *script) {
	warning("STUB: cmd_magicOutMouseItem");
	return 0;
}

int KyraEngine::cmd_internalAnimOn(ScriptState *script) {
	warning("STUB: cmd_internalAnimOn");
	return 0;
}

int KyraEngine::cmd_forceBrandonToNormal(ScriptState *script) {
	warning("STUB: cmd_forceBrandonToNormal");
	return 0;
}

int KyraEngine::cmd_poisonDeathNow(ScriptState *script) {
	warning("STUB: cmd_poisonDeathNow");
	return 0;
}

int KyraEngine::cmd_setScaleMode(ScriptState *script) {
	warning("STUB: cmd_setScaleMode");
	return 0;
}

int KyraEngine::cmd_openWSAFile(ScriptState *script) {
	debug(9, "cmd_openWSAFile(0x%X)", script);
	
	int wsaIndex = stackPos(0);
	uint16 offset = READ_BE_UINT16(&script->dataPtr->text[wsaIndex]);
	char *filename = (char*)&script->dataPtr->text[offset];
	
	wsaIndex = stackPos(1);
	// stackPos(2) is NOT used whyever
	int offscreenDecode = 0;
	if (!stackPos(3)) {
		offscreenDecode = 1;
	} else {
		offscreenDecode = 0;
	}
	
	_wsaObjects[wsaIndex] = wsa_open(filename, offscreenDecode, 0);
	assert(_wsaObjects[wsaIndex]);
	
	return 0;
}

int KyraEngine::cmd_closeWSAFile(ScriptState *script) {
	debug(9, "cmd_closeWSAFile(0x%X)", script);
	
	int wsaIndex = stackPos(0);
	if (_wsaObjects[wsaIndex]) {
		wsa_close(_wsaObjects[wsaIndex]);
		_wsaObjects[wsaIndex] = 0;
	}
	
	return 0;
}

int KyraEngine::cmd_runWSAFromBeginningToEnd(ScriptState *script) {
	debug(9, "cmd_runWSAFromBeginningToEnd(0x%X)", script);
	
	_screen->hideMouse();
	
	bool running = true;
	
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int waitTime = stackPos(2);
	int wsaIndex = stackPos(3);
	int worldUpdate = stackPos(4);
	int wsaFrame = 0;
	
	while (running) {
		wsa_play(_wsaObjects[wsaIndex], wsaFrame++, xpos, ypos, 0);
		if (wsaFrame >= wsa_getNumFrames(_wsaObjects[wsaIndex]))
			running = false;
			
		waitTicks(waitTime);
		_screen->updateScreen();
		if (worldUpdate) {
			updateAllObjectShapes();
			// XXX
		}
	}
	
	_screen->showMouse();
	
	return 0;
}

int KyraEngine::cmd_displayWSAFrame(ScriptState *script) {
	warning("STUB: cmd_displayWSAFrame");
	return 0;
}

int KyraEngine::cmd_enterNewScene(ScriptState *script) {
	debug(9, "cmd_enterNewScene(0x%X)", script);
	enterNewScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return 0;
}

int KyraEngine::cmd_setSpecialEnterXAndY(ScriptState *script) {
	debug(9, "cmd_setSpecialEnterXAndY(0x%X)", script);
	_brandonPosX = stackPos(0);
	_brandonPosY = stackPos(1);
	if (_brandonPosX + 1 == 0 && _brandonPosY + 1 == 0)
		_currentCharacter->currentAnimFrame = 88;
	return 0;
}

int KyraEngine::cmd_runWSAFrames(ScriptState *script) {
	warning("STUB: cmd_runWSAFrames");
	return 0;
}

int KyraEngine::cmd_popBrandonIntoScene(ScriptState *script) {
	warning("STUB: cmd_popBrandonIntoScene");
	return 0;
}

int KyraEngine::cmd_restoreAllObjectBackgrounds(ScriptState *script) {
	warning("STUB: cmd_restoreAllObjectBackgrounds");
	return 0;
}

int KyraEngine::cmd_setCustomPaletteRange(ScriptState *script) {
	warning("STUB: cmd_setCustomPaletteRange");
	return 0;
}

int KyraEngine::cmd_loadPageFromDisk(ScriptState *script) {
	warning("STUB: cmd_loadPageFromDisk");
	return 0;
}

int KyraEngine::cmd_customPrintTalkString(ScriptState *script) {
	warning("STUB: cmd_customPrintTalkString");
	return 0;
}

int KyraEngine::cmd_restoreCustomPrintBackground(ScriptState *script) {
	warning("STUB: cmd_restoreCustomPrintBackground");
	return 0;
}

int KyraEngine::cmd_hideMouse(ScriptState *script) {
	debug(9, "cmd_hideMouse(0x%X)", script);
	_screen->hideMouse();
	return 0;
}

int KyraEngine::cmd_showMouse(ScriptState *script) {
	debug(9, "cmd_showMouse(0x%X)", script);
	_screen->showMouse();
	return 0;
}

int KyraEngine::cmd_getCharacterX(ScriptState *script) {
	debug(9, "cmd_getCharacterX(0x%X)", script);
	return _characterList[stackPos(0)].x1;
}

int KyraEngine::cmd_getCharacterY(ScriptState *script) {
	debug(9, "cmd_getCharacterY(0x%X)", script);
	return _characterList[stackPos(0)].y1;
}

int KyraEngine::cmd_changeCharactersFacing(ScriptState *script) {
	warning("STUB: cmd_changeCharactersFacing");
	return 0;
}

int KyraEngine::cmd_CopyWSARegion(ScriptState *script) {
	warning("STUB: cmd_CopyWSARegion");
	return 0;
}

int KyraEngine::cmd_printText(ScriptState *script) {
	warning("STUB: cmd_printText");
	return 0;
}

int KyraEngine::cmd_random(ScriptState *script) {
	debug(9, "cmd_random(0x%X)", script);
	assert(stackPos(0) < stackPos(1));
	return _rnd.getRandomNumberRng(stackPos(0), stackPos(1));
}

int KyraEngine::cmd_loadSoundFile(ScriptState *script) {
	warning("STUB: cmd_loadSoundFile");
	return 0;
}

int KyraEngine::cmd_displayWSAFrameOnHidPage(ScriptState *script) {
	warning("STUB: cmd_displayWSAFrameOnHidPage");
	return 0;
}

int KyraEngine::cmd_displayWSASequentialFrames(ScriptState *script) {
	warning("STUB: cmd_displayWSASequentialFrames");
	return 0;
}

int KyraEngine::cmd_drawCharacterStanding(ScriptState *script) {
	debug(9, "cmd_drawCharacterStanding(0x%X)", script);
	// XXX
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
		updateAllObjectShapes();
	}
	return 0;
}

int KyraEngine::cmd_internalAnimOff(ScriptState *script) {
	warning("STUB: cmd_internalAnimOff");
	return 0;
}

int KyraEngine::cmd_changeCharactersXAndY(ScriptState *script) {
	warning("STUB: cmd_changeCharactersXAndY");
	return 0;
}

int KyraEngine::cmd_clearSceneAnimatorBeacon(ScriptState *script) {
	warning("STUB: cmd_clearSceneAnimatorBeacon");
	return 0;
}

int KyraEngine::cmd_querySceneAnimatorBeacon(ScriptState *script) {
	warning("STUB: cmd_querySceneAnimatorBeacon");
	return 0;
}

int KyraEngine::cmd_refreshSceneAnimator(ScriptState *script) {
	warning("STUB: cmd_refreshSceneAnimator");
	return 0;
}

int KyraEngine::cmd_placeItemInOffScene(ScriptState *script) {
	debug(9, "cmd_placeItemInOffScene(0x%X)", script);
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
	warning("STUB: cmd_wipeDownMouseItem");
	return 0;
}

int KyraEngine::cmd_placeCharacterInOtherScene(ScriptState *script) {
	debug(9, "cmd_placeCharacterInOtherScene(0x%X)", script);
	int id = stackPos(0);
	int sceneId = stackPos(1);
	int xpos = stackPos(2) & 0xFFFC;
	int ypos = stackPos(3) & 0xFE;
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
	warning("STUB: cmd_getKey");
	return 0;
}

int KyraEngine::cmd_specificItemInInventory(ScriptState *script) {
	warning("STUB: cmd_specificItemInInventory");
	return 0;
}

int KyraEngine::cmd_popMobileNPCIntoScene(ScriptState *script) {
	warning("STUB: cmd_popMobileNPCIntoScene");
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
	warning("STUB: cmd_setCharactersLocation");
	return 0;
}

int KyraEngine::cmd_walkCharacterToPoint(ScriptState *script) {
	debug(9, "cmd_walkCharacterToPoint(0x%X)", script);
	int character = stackPos(0);
	int toX = stackPos(1);
	int toY = stackPos(2);
	_pathfinderFlag2 = 1;
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
		// XXX
		waitTicks(10);
		// XXX updateAnimFlags();
		// XXX updateMouseCursor();
		// XXX updateGameTimers();
		updateAllObjectShapes();
		// XXX processPalette();
	}
	return 0;
}

int KyraEngine::cmd_specialEventDisplayBrynnsNote(ScriptState *script) {
	warning("STUB: cmd_specialEventDisplayBrynnsNote");
	return 0;
}

int KyraEngine::cmd_specialEventRemoveBrynnsNote(ScriptState *script) {
	warning("STUB: cmd_specialEventRemoveBrynnsNote");
	return 0;
}

int KyraEngine::cmd_setLogicPage(ScriptState *script) {
	warning("STUB: cmd_setLogicPage");
	return 0;
}

int KyraEngine::cmd_fatPrint(ScriptState *script) {
	warning("STUB: cmd_fatPrint");
	return 0;
}

int KyraEngine::cmd_preserveAllObjectBackgrounds(ScriptState *script) {
	warning("STUB: cmd_preserveAllObjectBackgrounds");
	return 0;
}

int KyraEngine::cmd_updateSceneAnimations(ScriptState *script) {
	warning("STUB: cmd_updateSceneAnimations");
	return 0;
}

int KyraEngine::cmd_sceneAnimationActive(ScriptState *script) {
	warning("STUB: cmd_sceneAnimationActive");
	return 0;
}

int KyraEngine::cmd_setCharactersMovementDelay(ScriptState *script) {
	warning("STUB: cmd_setCharactersMovementDelay");
	return 0;
}

int KyraEngine::cmd_getCharactersFacing(ScriptState *script) {
	warning("STUB: cmd_getCharactersFacing");
	return 0;
}

int KyraEngine::cmd_bkgdScrollSceneAndMasksRight(ScriptState *script) {
	warning("STUB: cmd_bkgdScrollSceneAndMasksRight");
	return 0;
}

int KyraEngine::cmd_dispelMagicAnimation(ScriptState *script) {
	warning("STUB: cmd_dispelMagicAnimation");
	return 0;
}

int KyraEngine::cmd_findBrightestFireberry(ScriptState *script) {
	warning("STUB: cmd_findBrightestFireberry");
	return 0;
}

int KyraEngine::cmd_setFireberryGlowPalette(ScriptState *script) {
	warning("STUB: cmd_setFireberryGlowPalette");
	return 0;
}

int KyraEngine::cmd_setDeathHandlerFlag(ScriptState *script) {
	warning("STUB: cmd_setDeathHandlerFlag");
	return 0;
}

int KyraEngine::cmd_drinkPotionAnimation(ScriptState *script) {
	warning("STUB: cmd_drinkPotionAnimation");
	return 0;
}

int KyraEngine::cmd_makeAmuletAppear(ScriptState *script) {
	warning("STUB: cmd_makeAmuletAppear");
	return 0;
}

int KyraEngine::cmd_drawItemShapeIntoScene(ScriptState *script) {
	warning("STUB: cmd_drawItemShapeIntoScene");
	return 0;
}

int KyraEngine::cmd_setCharactersCurrentFrame(ScriptState *script) {
	warning("STUB: cmd_setCharactersCurrentFrame");
	return 0;
}

int KyraEngine::cmd_waitForConfirmationMouseClick(ScriptState *script) {
	warning("STUB: cmd_waitForConfirmationMouseClick");
	return 0;
}

int KyraEngine::cmd_pageFlip(ScriptState *script) {
	warning("STUB: cmd_pageFlip");
	return 0;
}

int KyraEngine::cmd_setSceneFile(ScriptState *script) {
	warning("STUB: cmd_setSceneFile");
	return 0;
}

int KyraEngine::cmd_getItemInMarbleVase(ScriptState *script) {
	warning("STUB: cmd_getItemInMarbleVase");
	return 0;
}

int KyraEngine::cmd_setItemInMarbleVase(ScriptState *script) {
	debug(9, "cmd_setItemInMarbleVase(0x%X)", script);
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
	warning("STUB: cmd_createAmuletJewel");
	return 0;
}

int KyraEngine::cmd_setSceneAnimCurrXY(ScriptState *script) {
	warning("STUB: cmd_setSceneAnimCurrXY");
	return 0;
}

int KyraEngine::cmd_Poison_Brandon_And_Remaps(ScriptState *script) {
	warning("STUB: cmd_Poison_Brandon_And_Remaps");
	return 0;
}

int KyraEngine::cmd_fillFlaskWithWater(ScriptState *script) {
	warning("STUB: cmd_fillFlaskWithWater");
	return 0;
}

int KyraEngine::cmd_getCharactersMovementDelay(ScriptState *script) {
	warning("STUB: cmd_getCharactersMovementDelay");
	return 0;
}

int KyraEngine::cmd_getBirthstoneGem(ScriptState *script) {
	warning("STUB: cmd_getBirthstoneGem");
	return 0;
}

int KyraEngine::cmd_queryBrandonStatusBit(ScriptState *script) {
	warning("STUB: cmd_queryBrandonStatusBit");
	return 0;
}

int KyraEngine::cmd_playFluteAnimation(ScriptState *script) {
	warning("STUB: cmd_playFluteAnimation");
	return 0;
}

int KyraEngine::cmd_playWinterScrollSequence(ScriptState *script) {
	warning("STUB: cmd_playWinterScrollSequence");
	return 0;
}

int KyraEngine::cmd_getIdolGem(ScriptState *script) {
	warning("STUB: cmd_getIdolGem");
	return 0;
}

int KyraEngine::cmd_setIdolGem(ScriptState *script) {
	debug(9, "cmd_setIdolGem(0x%X)", script);
	_idolGemsTable[stackPos(0)] = stackPos(1);
	return 0;
}

int KyraEngine::cmd_totalItemsInScene(ScriptState *script) {
	warning("STUB: cmd_totalItemsInScene");
	return 0;
}

int KyraEngine::cmd_restoreBrandonsMovementDelay(ScriptState *script) {
	warning("STUB: cmd_restoreBrandonsMovementDelay");
	return 0;
}

int KyraEngine::cmd_setMousePos(ScriptState *script) {
	warning("STUB: cmd_setMousePos");
	return 0;
}

int KyraEngine::cmd_getMouseState(ScriptState *script) {
	warning("STUB: cmd_getMouseState");
	return 0;
}

int KyraEngine::cmd_setEntranceMouseCursorTrack(ScriptState *script) {
	debug(9, "cmd_setEntranceMouseCursorTrack(0x%X)", script);
	_entranceMouseCursorTracks[0] = stackPos(0);
	_entranceMouseCursorTracks[1] = stackPos(1);
	_entranceMouseCursorTracks[2] = stackPos(0) + stackPos(2) - 1;
	_entranceMouseCursorTracks[3] = stackPos(1) + stackPos(3) - 1;
	_entranceMouseCursorTracks[4] = stackPos(3);
	return 0;
}

int KyraEngine::cmd_itemAppearsOnGround(ScriptState *script) {
	warning("STUB: cmd_itemAppearsOnGround");
	return 0;
}

int KyraEngine::cmd_setNoDrawShapesFlag(ScriptState *script) {
	warning("STUB: cmd_setNoDrawShapesFlag");
	return 0;
}

int KyraEngine::cmd_fadeEntirePalette(ScriptState *script) {
	warning("STUB: cmd_fadeEntirePalette");
	return 0;
}

int KyraEngine::cmd_itemOnGroundHere(ScriptState *script) {
	warning("STUB: cmd_itemOnGroundHere");
	return 0;
}

int KyraEngine::cmd_queryCauldronState(ScriptState *script) {
	warning("STUB: cmd_queryCauldronState");
	return 0;
}

int KyraEngine::cmd_setCauldronState(ScriptState *script) {
	warning("STUB: cmd_setCauldronState");
	return 0;
}

int KyraEngine::cmd_queryCrystalState(ScriptState *script) {
	warning("STUB: cmd_queryCrystalState");
	return 0;
}

int KyraEngine::cmd_setCrystalState(ScriptState *script) {
	warning("STUB: cmd_setCrystalState");
	return 0;
}

int KyraEngine::cmd_setPaletteRange(ScriptState *script) {
	warning("STUB: cmd_setPaletteRange");
	return 0;
}

int KyraEngine::cmd_shrinkBrandonDown(ScriptState *script) {
	warning("STUB: cmd_shrinkBrandonDown");
	return 0;
}

int KyraEngine::cmd_growBrandonUp(ScriptState *script) {
	warning("STUB: cmd_growBrandonUp");
	return 0;
}

int KyraEngine::cmd_setBrandonScaleXAndY(ScriptState *script) {
	warning("STUB: cmd_setBrandonScaleXAndY");
	return 0;
}

int KyraEngine::cmd_resetScaleMode(ScriptState *script) {
	warning("STUB: cmd_resetScaleMode");
	return 0;
}

int KyraEngine::cmd_getScaleDepthTableValue(ScriptState *script) {
	warning("STUB: cmd_getScaleDepthTableValue");
	return 0;
}

int KyraEngine::cmd_setScaleDepthTableValue(ScriptState *script) {
	warning("STUB: cmd_setScaleDepthTableValue");
	return 0;
}

int KyraEngine::cmd_message(ScriptState *script) {
	warning("STUB: cmd_message");
	return 0;
}

int KyraEngine::cmd_checkClickOnNPC(ScriptState *script) {
	warning("STUB: cmd_checkClickOnNPC");
	return 0;
}

int KyraEngine::cmd_getFoyerItem(ScriptState *script) {
	warning("STUB: cmd_getFoyerItem");
	return 0;
}

int KyraEngine::cmd_setFoyerItem(ScriptState *script) {
	warning("STUB: cmd_setFoyerItem");
	return 0;
}

int KyraEngine::cmd_setNoItemDropRegion(ScriptState *script) {
	debug(9, "cmd_setNoItemDropRegion(0x%X)", script);
	addToNoDropRects(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int KyraEngine::cmd_walkMalcolmOn(ScriptState *script) {
	warning("STUB: cmd_walkMalcolmOn");
	return 0;
}

int KyraEngine::cmd_passiveProtection(ScriptState *script) {
	warning("STUB: cmd_passiveProtection");
	return 0;
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
	warning("STUB: cmd_brandonHealingSequence");
	return 0;
}

int KyraEngine::cmd_protectCommandLine(ScriptState *script) {
	warning("STUB: cmd_protectCommandLine");
	return 0;
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
	warning("STUB: cmd_setPaletteChangeFlag");
	return 0;
}

int KyraEngine::cmd_fillRect(ScriptState *script) {
	warning("STUB: cmd_fillRect");
	return 0;
}

int KyraEngine::cmd_dummy(ScriptState *script) {
	debug(9, "cmd_dummy(0x%X)", script);
	return 0;
}

} // end of namespace Kyra
