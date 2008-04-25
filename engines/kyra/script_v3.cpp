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

int KyraEngine_v3::o3_getMalcolmShapes(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getMaloclmShapes(%p) ()", (const void *)script);
	return _malcolmShapes;
}

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

int KyraEngine_v3::o3_defineObject(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_defineObject(%p) (%d, '%s', %d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	TalkObject &obj = _talkObjectList[stackPos(0)];
	strcpy(obj.filename, stackPosString(1));
	obj.sceneAnim = stackPos(2);
	obj.sceneScript = stackPos(3);
	obj.x = stackPos(4);
	obj.y = stackPos(5);
	obj.color = stackPos(6);
	obj.sceneId = stackPos(7);
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

int KyraEngine_v3::o3_getCharacterX(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getCharacterX(%p) ()", (const void *)script);
	return _mainCharacter.x1;
}

int KyraEngine_v3::o3_getCharacterY(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getCharacterY(%p) ()", (const void *)script);
	return _mainCharacter.y1;
}

int KyraEngine_v3::o3_getCharacterFacing(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getCharacterFacing(%p) ()", (const void *)script);
	return _mainCharacter.facing;
}

int KyraEngine_v3::o3_getCharacterScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getCharacterScene(%p) ()", (const void *)script);
	return _mainCharacter.sceneId;
}

int KyraEngine_v3::o3_getMalcolmsMood(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getMalcolmsMood(%p) ()", (const void *)script);
	return _malcolmsMood;
}

int KyraEngine_v3::o3_trySceneChange(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_trySceneChange(%p) (%d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	_unkHandleSceneChangeFlag = 1;
	int success = inputSceneChange(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_unkHandleSceneChangeFlag = 0;

	if (success) {
		_scriptInterpreter->initScript(script, script->dataPtr);
		_unk4 = 0;
		_unk3 = -1;
		_unk5 = 1;
		return 0;
	} else {
		return (_unk4 != 0) ? 1 : 0;
	}
}

int KyraEngine_v3::o3_moveCharacter(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_moveCharacter(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	moveCharacter(stackPos(0), stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine_v3::o3_setCharacterFacing(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setCharacterFacing(%p) (%d)", (const void *)script, stackPos(0));
	_mainCharacter.facing = stackPos(0);
	return 0;
}

int KyraEngine_v3::o3_showSceneFileMessage(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_showSceneFileMessage(%p) (%d)", (const void *)script, stackPos(0));
	showMessage((const char*)getTableEntry(_scenesFile, stackPos(0)), 0xFF, 0xF0);
	return 0;
}

int KyraEngine_v3::o3_showBadConscience(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_showBadConscience(%p) ()", (const void *)script);
	showBadConscience();
	return 0;
}

int KyraEngine_v3::o3_hideBadConscience(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_hideBadConscience(%p) ()", (const void *)script);
	hideBadConscience();
	return 0;
}

int KyraEngine_v3::o3_objectChat(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_objectChat(%p) (%d)", (const void *)script, stackPos(0));
	int id = stackPos(0);
	const char *str = (const char*)getTableEntry(_useActorBuffer ? _actorFile : _sceneStrings, id);
	if (str) {
		objectChat(str, 0, _vocHigh, id);
		playStudioSFX(str);
	}
	return 0;
}

int KyraEngine_v3::o3_checkForItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_checkForItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return findItem(stackPos(0), stackPos(1)) == -1 ? 0 : 1;
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

int KyraEngine_v3::o3_npcChatSequence(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_npcChatSequence(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int id = stackPos(0);
	const char *str = (const char*)getTableEntry(_sceneStrings, id);
	if (str)
		npcChatSequence(str, stackPos(1), _vocHigh, id);
	return 0;
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

int KyraEngine_v3::o3_setHandItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setHandItem(%p) (%d)", (const void *)script, stackPos(0));
	setHandItem(stackPos(0));
	return 0;
}

int KyraEngine_v3::o3_removeHandItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_removeHandItem(%p) ()", (const void *)script);
	removeHandItem();
	return 0;
}

int KyraEngine_v3::o3_handItemSet(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_handItemSet(%p) ()", (const void *)script);
	return _handItemSet;
}

int KyraEngine_v3::o3_hideMouse(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_hideMouse(%p) ()", (const void *)script);
	_screen->hideMouse();
	return 0;
}

int KyraEngine_v3::o3_addSpecialExit(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_addSpecialExit(%p) (%d, %d, %d, %d, %d)", (const void *)script,
		stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	if (_specialExitCount < 5) {
		_specialExitTable[_specialExitCount+0] = stackPos(0);
		_specialExitTable[_specialExitCount+5] = stackPos(1);
		_specialExitTable[_specialExitCount+10] = stackPos(2) + stackPos(0) - 1;
		_specialExitTable[_specialExitCount+15] = stackPos(3) + stackPos(1) - 1;
		_specialExitTable[_specialExitCount+20] = stackPos(4);
		++_specialExitCount;
	}
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

int KyraEngine_v3::o3_badConscienceChat(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_badConscienceChat(%p) (%d)", (const void *)script, stackPos(0));
	int id = stackPos(0);
	const char *str = (const char*)getTableEntry(_useActorBuffer ? _actorFile : _sceneStrings, id);
	badConscienceChat(str, _vocHigh, id);
	return 0;
}

int KyraEngine_v3::o3_wipeDownMouseItem(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o3_wipeDownMouseItem(%p) (-, %d, %d)", (const void *)script, stackPos(1), stackPos(2));
	_screen->hideMouse();
	const int x = stackPos(1) - 12;
	const int y = stackPos(2) - 19;

	if (_itemInHand >= 0) {
		backUpGfxRect32x32(x, y);
		uint8 *shape = getShapePtr(_itemInHand+248);
		for (int curY = y, height = 20; height > 0; height -= 2, curY += 2) {
			restoreGfxRect32x32(x, y);
			_screen->setNewShapeHeight(shape, height);
			uint32 waitTime = _system->getMillis() + _tickLength;
			_screen->drawShape(0, shape, x, curY, 0, 0);
			_screen->updateScreen();
			delayUntil(waitTime);
		}
		restoreGfxRect32x32(x, y);
		_screen->resetShapeHeight(shape);
	}

	_screen->showMouse();
	removeHandItem();

	return 0;
}

int KyraEngine_v3::o3_setMalcolmsMood(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setMalcolmsMood(%p) (%d)", (const void *)script, stackPos(0));
	return (_malcolmsMood = stackPos(0));
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

int KyraEngine_v3::o3_updateScore(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_updateScore(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return updateScore(stackPos(0), stackPos(1)) ? 1 : 0;
}

int KyraEngine_v3::o3_setSceneFilename(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setSceneFilename(%p) (%d, '%s')", (const void *)script, stackPos(0), stackPosString(1));
	strcpy(_sceneList[stackPos(0)].filename1, stackPosString(1));
	_sceneList[stackPos(0)].filename1[9] = 0;
	return 0;
}

int KyraEngine_v3::o3_removeItemsFromScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_removeItemsFromScene(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const uint16 itemId = stackPos(0);
	const uint16 sceneId = stackPos(1);
	const bool allItems = (stackPos(2) != 0);

	int retValue = 0;

	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].sceneId == sceneId && _itemList[i].id == itemId) {
			resetItem(i);
			retValue = 1;
			if (!allItems)
				return 1;
		}
	}

	return retValue;
}

int KyraEngine_v3::o3_drawSceneShapeOnPage(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_drawSceneShapeOnPage(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const int shape = stackPos(0);
	
	int x = _sceneShapeDescs[shape].drawX;
	int y = _sceneShapeDescs[shape].drawY;
	_screen->drawShape(stackPos(2), _sceneShapes[shape], x, y, 2, (stackPos(1) != 0) ? 1 : 0);
	return 0;
}

int KyraEngine_v3::o3_checkInRect(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_checkInRect(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	const int x1 = stackPos(0);
	const int y1 = stackPos(1);
	const int x2 = stackPos(2);
	const int y2 = stackPos(3);
	int x = stackPos(4), y = stackPos(5);
	if (_itemInHand >= 0) {
		const int8 *desc = &_itemBuffer2[_itemInHand*2];
		x -= 12;
		x += desc[0];
		y -= 19;
		y += desc[1];
	}

	if (x >= x1 && x <= x2 && y >= y1 && y <= y2) {
		//XXX
		return 1;
	} else {
		//XXX
		return 0;
	}
}

int KyraEngine_v3::o3_updateConversations(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_updateConversations(%p) (%d)", (const void *)script, stackPos(0));
	int dlgIndex = stackPos(0);
	switch (_curChapter-2) {
	case 0:
		dlgIndex -= 34;
		break;
	
	case 1:
		dlgIndex -= 54;
		break;

	case 2:
		dlgIndex -= 55;
		break;

	case 3:
		dlgIndex -= 70;
		break;

	default:
		break;
	}

	int convs[4];
	Common::set_to(convs, convs+4, -1);

	if (_curChapter == 1) {
		switch (_mainCharacter.dlgIndex) {
		case 0:
			convs[0] = 6;
			convs[1] = 12;
			break;

		case 2:
			convs[0] = 8;
			convs[1] = 14;
			break;

		case 3:
			convs[0] = 9;
			convs[1] = 15;
			break;

		case 4:
			convs[0] = 10;
			convs[1] = 16;
			break;

		case 5:
			convs[0] = 11;
			convs[1] = 17;
			break;

		case 6:
			convs[0] = 0;
			convs[1] = 12;
			break;

		case 8:
			convs[0] = 2;
			convs[1] = 14;
			break;

		case 9:
			convs[0] = 3;
			convs[1] = 15;
			break;

		case 10:
			convs[0] = 4;
			convs[1] = 16;
			break;

		case 11:
			convs[0] = 5;
			convs[1] = 17;
			break;

		case 12:
			convs[0] = 0;
			convs[1] = 6;
			break;

		case 14:
			convs[0] = 2;
			convs[1] = 8;
			break;

		case 15:
			convs[0] = 3;
			convs[1] = 9;
			break;

		case 16:
			convs[0] = 4;
			convs[1] = 10;
			break;

		case 17:
			convs[0] = 5;
			convs[1] = 11;
			break;

		default:
			break;
		}
	} else if (_curChapter == 2) {
		switch (_mainCharacter.dlgIndex) {
		case 0:
			convs[0] = 4;
			convs[1] = 8;
			convs[2] = 5;
			convs[3] = 9;
			break;

		case 1:
			convs[0] = 4;
			convs[1] = 8;
			convs[2] = 0;
			convs[3] = 5;
			break;

		case 2:
			convs[0] = 6;
			convs[2] = 11;
			break;

		case 3:
			convs[0] = 7;
			convs[2] = 12;
			break;

		case 4:
			convs[0] = 0;
			convs[1] = 8;
			convs[2] = 1;
			convs[3] = 9;
			break;

		case 5:
			convs[0] = 0;
			convs[1] = 8;
			convs[2] = 4;
			convs[3] = 1;
			break;

		case 6:
			convs[0] = 2;
			convs[1] = 10;
			break;

		case 7:
			convs[0] = 3;
			convs[1] = 11;
			break;

		case 8:
			convs[0] = 0;
			convs[1] = 4;
			convs[2] = 1;
			break;

		case 9:
			convs[0] = 0;
			convs[1] = 4;
			convs[2] = 0;
			convs[4] = 1;
			break;

		case 10:
			convs[0] = 2;
			convs[1] = 6;
			break;

		case 11:
			convs[0] = 3;
			convs[1] = 7;
			break;

		default:
			break;
		}
	} else if (_curChapter == 4) {
		if (_malcolmsMood == 0) {
			convs[0] = _mainCharacter.dlgIndex - 10;
			convs[1] = _mainCharacter.dlgIndex - 5;
		} else if (_malcolmsMood == 1) {
			convs[0] = _mainCharacter.dlgIndex + 5;
			convs[1] = _mainCharacter.dlgIndex + 10;
		} else if (_malcolmsMood == 2) {
			convs[0] = _mainCharacter.dlgIndex - 5;
			convs[1] = _mainCharacter.dlgIndex + 5;
		}
	}

	for (int i = 0; i < 4; ++i) {
		if (convs[i] != -1)
			_conversationState[dlgIndex][convs[i]] = 0;
	}

	return 1;
}

int KyraEngine_v3::o3_setSceneDim(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setSceneDim(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_sceneMinX = stackPos(0);
	_sceneMaxX = stackPos(1);
	return 0;
}

int KyraEngine_v3::o3_update(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_update(%p) (%d)", (const void *)script, stackPos(0));
	for (int times = stackPos(0); times != 0; --times) {
		if (_chatText)
			updateWithText();
		else
			update();
	}
	return 0;
}

int KyraEngine_v3::o3_enterNewScene(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_enterNewScene(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0),
		stackPos(1), stackPos(2), stackPos(3), stackPos(4));

	_screen->hideMouse();
	enterNewScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));

	_unk5 = 1;

	if (_mainCharX == -1 || _mainCharY == -1) {
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
	}
	_screen->showMouse();

	return 0;
}

int KyraEngine_v3::o3_setMalcolmPos(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setMalcolmPos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_mainCharX = stackPos(0);
	_mainCharY = stackPos(1);

	if (_mainCharX == -1 && _mainCharY == -1)
		_mainCharacter.animFrame = 87;
	else
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	return 0;
}

int KyraEngine_v3::o3_stopMusic(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_stopMusic(%p) ()", (const void *)script);
	stopMusicTrack();
	return 0;
}

int KyraEngine_v3::o3_playMusicTrack(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_playMusicTrack(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	playMusicTrack(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v3::o3_playSoundEffect(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_playSoundEffect(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	playSoundEffect(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v3::o3_getScore(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getScore(%p) ()", (const void *)script);
	return _score;
}

int KyraEngine_v3::o3_blockOutRegion(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_blockOutRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	const int x1 = stackPos(0);
	int y1 = stackPos(1);
	const int x2 = stackPos(2);
	int y2 = stackPos(3);

	if (y1 < _maskPageMinY)
		y1 = _maskPageMinY;
	if (y2 > _maskPageMaxY)
		y2 = _maskPageMaxY;

	_screen->blockOutRegion(x1, y1, x2-x1+1, y2-y1+1);
	return 0;
}

int KyraEngine_v3::o3_getRand(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getRand(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < stackPos(1));
	return _rnd.getRandomNumberRng(stackPos(0), stackPos(1));
}

int KyraEngine_v3::o3_waitForConfirmationClick(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o2_waitForConfirmationClick(%p) (%d)", (const void *)script, stackPos(0));
	resetSkipFlag();
	uint32 maxWaitTime = _system->getMillis() + stackPos(0) * _tickLength;

	while (_system->getMillis() < maxWaitTime) {
		int inputFlag = checkInput(0);
		removeInputTop();

		if (inputFlag == 198 || inputFlag == 199) {
			_sceneScriptState.regs[1] = _mouseX;
			_sceneScriptState.regs[2] = _mouseY;
			return 0;
		}

		update();
		_system->delayMillis(10);
	}

	_sceneScriptState.regs[1] = _mouseX;
	_sceneScriptState.regs[2] = _mouseY;
	return 1;
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

int KyraEngine_v3::o3_runTemporaryScript(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_runTemporaryScript(%p) ('%s', %d, %d, %d)", (const void *)script,
			stackPosString(0), stackPos(1), stackPos(2), stackPos(3));
	const int newShapes = stackPos(1);
	const int unloadShapes = stackPos(2);
	const int allowSkip = stackPos(3);
	runTemporaryScript(stackPosString(0), allowSkip, (unloadShapes != 0) ? 1 : 0, newShapes, unloadShapes);
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

int KyraEngine_v3::o3_runActorScript(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_runActorScript(%p) ()", (const void *)script);
	ScriptData data;
	ScriptState state;
	memset(&data, 0, sizeof(data));
	memset(&state, 0, sizeof(state));

	_res->exists("_ACTOR.EMC", true);
	_scriptInterpreter->loadScript("_ACTOR.EMC", &data, &_opcodes);
	_scriptInterpreter->initScript(&state, &data);
	_scriptInterpreter->startScript(&state, 0);

	state.regs[4] = _itemInHand;
	state.regs[0] = _mainCharacter.sceneId;

	int vocHigh = _vocHigh;
	_vocHigh = 200;
	_useActorBuffer = true;

	while (_scriptInterpreter->validScript(&state))
		_scriptInterpreter->runScript(&state);

	_useActorBuffer = false;
	_vocHigh = vocHigh;
	_scriptInterpreter->unloadScript(&data);

	if (queryGameFlag(0x218)) {
		resetGameFlag(0x218);
		enterNewScene(78, -1, 0, 0, 0);
	}

	return 0;
}

int KyraEngine_v3::o3_runDialog(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_runDialog(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	runDialog(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v3::o3_malcolmRandomChat(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_malcolmRandomChat(%p) ()", (const void *)script);
	malcolmRandomChat();
	return 0;
}

int KyraEngine_v3::o3_setDlgIndex(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setDlgIndex(%p) (%d)", (const void *)script, stackPos(0));
	setDlgIndex(stackPos(0));
	return 0;
}

int KyraEngine_v3::o3_getDlgIndex(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getDlgIndex(%p) ()", (const void *)script);
	return _mainCharacter.dlgIndex;
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

int KyraEngine_v3::o3_countItemInstances(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_countItemInstances(%p) (%d)", (const void *)script, stackPos(0));
	int count = 0;
	const int16 item = stackPos(0);

	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == item)
			++count;
	}

	if (_itemInHand == item)
		++count;

	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].id == item)
			++count;
	}

	return count;
}

int KyraEngine_v3::o3_dialogStartScript(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_dialogStartScript(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	dialogStartScript(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v3::o3_dialogEndScript(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_dialogEndScript(%p) (%d)", (const void *)script, stackPos(0));
	dialogEndScript(stackPos(0));
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

int KyraEngine_v3::o3_setupSceneAnimObject(ScriptState *script) {
	debugC(9, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setupSceneAnimObject(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s')", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9),
			stackPos(10), stackPos(11), stackPosString(12));
	musicUpdate(0);
	setupSceneAnimObject(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8),
						stackPos(9), stackPos(10), stackPos(11), stackPosString(12));
	return 0;
}

int KyraEngine_v3::o3_removeSceneAnimObject(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_removeSceneAnimObject(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	removeSceneAnimObject(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v3::o3_setVocHigh(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_setVocHigh(%p) (%d)", (const void *)script, stackPos(0));
	_vocHigh = stackPos(0);
	return 0;
}

int KyraEngine_v3::o3_getVocHigh(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_getVocHigh(%p) ()", (const void *)script);
	return _vocHigh;
}

int KyraEngine_v3::o3_dummy(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3_dummy(%p) ()", (const void *)script);
	return 0;
}

#pragma mark -

int KyraEngine_v3::o3t_defineNewShapes(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3t_defineNewShapes(%p) ('%s', %d, %d, %d, %d, %d)", (const void *)script,
			stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	strcpy(_newShapeFilename, stackPosString(0));
	_newShapeLastEntry = stackPos(1);
	_newShapeWidth = stackPos(2);
	_newShapeHeight = stackPos(3);
	_newShapeXAdd = stackPos(4);
	_newShapeYAdd = stackPos(5);
	return 0;
}

int KyraEngine_v3::o3t_setCurrentFrame(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3t_setCurrentFrame(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	static const uint8 frameTable[] = {
		0x58, 0xD8, 0xD8, 0x98, 0x78, 0x78, 0xB8, 0xB8
	};

	_newShapeAnimFrame = stackPos(0);
	if (_useFrameTable)
		_newShapeAnimFrame += frameTable[_mainCharacter.facing];

	_newShapeDelay = stackPos(1);
	_temporaryScriptExecBit = true;
	return 0;
}

int KyraEngine_v3::o3t_setNewShapeFlag(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3t_setNewShapeFlag(%p) (%d)", (const void *)script, stackPos(0));
	_newShapeFlag = stackPos(0);
	return 0;
}

#pragma mark -

int KyraEngine_v3::o3d_updateAnim(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3d_updateAnim(%p) (%d)", (const void *)script, stackPos(0));
	if (_dialogSceneAnim >= 0)
		updateSceneAnim(_dialogSceneAnim, stackPos(0));
	return 0;
}

int KyraEngine_v3::o3d_delay(ScriptState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v3::o3d_delay(%p) (%d)", (const void *)script, stackPos(0));
	const uint32 endTime = _system->getMillis() + stackPos(0) * _tickLength;
	while (_system->getMillis() < endTime) {
		if (_chatText)
			updateWithText();
		else
			update();
		_system->delayMillis(10);
	}
	return 0;
}

#pragma mark -

typedef Common::Functor1Mem<ScriptState*, int, KyraEngine_v3> OpcodeV3;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV3(this, &KyraEngine_v3::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV3(this, 0))
void KyraEngine_v3::setupOpcodeTable() {
	Common::Array<const Opcode*> *table = 0;

	SetOpcodeTable(_opcodes);
	// 0x00
	Opcode(o3_getMalcolmShapes);
	Opcode(o3_setCharacterPos);
	Opcode(o3_defineObject);
	Opcode(o3_refreshCharacter);
	// 0x04
	Opcode(o3_getCharacterX);
	Opcode(o3_getCharacterY);
	Opcode(o3_getCharacterFacing);
	Opcode(o3_getCharacterScene);
	// 0x08
	Opcode(o3_getMalcolmsMood);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	OpcodeUnImpl();
	// 0x0c
	OpcodeUnImpl();
	Opcode(o3_trySceneChange);
	Opcode(o3_moveCharacter);
	Opcode(o3_setCharacterFacing);
	// 0x10
	OpcodeUnImpl();
	Opcode(o3_showSceneFileMessage);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x14
	OpcodeUnImpl();
	Opcode(o3_showBadConscience);
	Opcode(o3_dummy);
	Opcode(o3_hideBadConscience);
	// 0x18
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0x1c
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_objectChat);
	// 0x20
	Opcode(o3_checkForItem);
	Opcode(o3_dummy);
	OpcodeUnImpl();
	Opcode(o3_defineItem);
	// 0x24
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_npcChatSequence);
	Opcode(o3_queryGameFlag);
	// 0x28
	Opcode(o3_resetGameFlag);
	Opcode(o3_setGameFlag);
	Opcode(o3_setHandItem);
	Opcode(o3_removeHandItem);
	// 0x2c
	Opcode(o3_handItemSet);
	Opcode(o3_hideMouse);
	Opcode(o3_addSpecialExit);
	Opcode(o3_setMousePos);
	// 0x30
	Opcode(o3_showMouse);
	Opcode(o3_badConscienceChat);
	Opcode(o3_wipeDownMouseItem);
	Opcode(o3_dummy);
	// 0x34
	Opcode(o3_setMalcolmsMood);
	Opcode(o3_playSoundEffect);
	Opcode(o3_dummy);
	Opcode(o3_delay);
	// 0x38
	Opcode(o3_updateScore);
	OpcodeUnImpl();
	Opcode(o3_setSceneFilename);
	OpcodeUnImpl();
	// 0x3c
	Opcode(o3_removeItemsFromScene);
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_drawSceneShapeOnPage);
	// 0x40
	Opcode(o3_checkInRect);
	Opcode(o3_updateConversations);
	OpcodeUnImpl();
	Opcode(o3_dummy);
	// 0x44
	Opcode(o3_dummy);
	Opcode(o3_setSceneDim);
	OpcodeUnImpl();
	Opcode(o3_dummy);
	// 0x48
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	OpcodeUnImpl();
	Opcode(o3_update);
	// 0x4c
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0x50
	Opcode(o3_enterNewScene);
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_dummy);
	// 0x54
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_setMalcolmPos);
	Opcode(o3_stopMusic);
	// 0x58
	Opcode(o3_playMusicTrack);
	Opcode(o3_playSoundEffect);
	Opcode(o3_getScore);
	OpcodeUnImpl();
	// 0x5c
	Opcode(o3_blockOutRegion);
	Opcode(o3_dummy);
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0x60
	Opcode(o3_getRand);
	Opcode(o3_dummy);
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0x64
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x68
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_waitForConfirmationClick);
	// 0x6c
	Opcode(o3_dummy);
	Opcode(o3_defineRoomEntrance);
	Opcode(o3_runTemporaryScript);
	Opcode(o3_setSpecialSceneScriptRunTime);
	// 0x70
	Opcode(o3_defineSceneAnim);
	Opcode(o3_dummy);
	Opcode(o3_updateSceneAnim);
	Opcode(o3_dummy);
	// 0x74
	Opcode(o3_runActorScript);
	Opcode(o3_runDialog);
	Opcode(o3_malcolmRandomChat);
	Opcode(o3_setDlgIndex);
	// 0x78
	Opcode(o3_getDlgIndex);
	Opcode(o3_defineScene);
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0x7c
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x80
	Opcode(o3_dummy);
	OpcodeUnImpl();
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x84
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x88
	Opcode(o3_countItemInstances);
	Opcode(o3_dummy);
	Opcode(o3_dialogStartScript);
	Opcode(o3_dummy);
	// 0x8c
	Opcode(o3_dialogEndScript);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_setSpecialSceneScriptState);
	// 0x90
	Opcode(o3_clearSpecialSceneScriptState);
	Opcode(o3_querySpecialSceneScriptState);
	Opcode(o3_dummy);
	Opcode(o3_setHiddenItemsEntry);
	// 0x94
	Opcode(o3_getHiddenItemsEntry);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	OpcodeUnImpl();
	// 0x98
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_setupSceneAnimObject);
	Opcode(o3_removeSceneAnimObject);
	// 0x9c
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0xa0
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0xa4
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o3_setVocHigh);
	// 0xa8
	Opcode(o3_getVocHigh);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0xac
	OpcodeUnImpl();
	Opcode(o3_dummy);
	OpcodeUnImpl();
	Opcode(o3_dummy);
	
	SetOpcodeTable(_opcodesTemporary);
	// 0x00
	Opcode(o3t_defineNewShapes);
	Opcode(o3t_setCurrentFrame);
	Opcode(o3_playSoundEffect);
	Opcode(o3_dummy);
	// 0x0a
	Opcode(o3t_setNewShapeFlag);
	Opcode(o3_getRand);
	Opcode(o3_getMalcolmShapes);
	Opcode(o3_dummy);

	SetOpcodeTable(_opcodesDialog);
	// 0x00
	Opcode(o3d_updateAnim);
	Opcode(o3d_delay);
	Opcode(o3_getRand);
	Opcode(o3_queryGameFlag);
	// 0x04
	Opcode(o3_dummy);
}

} // end of namespace Kyra
