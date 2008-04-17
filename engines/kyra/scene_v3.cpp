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
#include "kyra/screen_v3.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"

namespace Kyra {

void KyraEngine_v3::enterNewScene(uint16 sceneId, int facing, int unk1, int unk2, int unk3) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::enterNewScene('%d, %d, %d, %d, %d)", sceneId, facing, unk1, unk2, unk3);
	++_enterNewSceneLock;
	_screen->hideMouse();

	showMessage(0, 0xF0, 0xF0);
	if (_inventoryState) {
		//XXX hideInventory();
		musicUpdate(0);
	}

	musicUpdate(0);
	//XXX
	musicUpdate(0);

	if (!unk3) {
		//XXX
		musicUpdate(0);
	}

	if (unk1) {
		int x = _mainCharacter.x1;
		int y = _mainCharacter.y1;

		switch (facing) {
		case 0:
			y -= 6;
			break;

		case 2:
			x = 343;
			break;

		case 4:
			y = 191;
			break;

		case 6:
			x = -24;
			break;
		}

		musicUpdate(0);
		moveCharacter(facing, x, y);
	}

	musicUpdate(0);
	uint32 waitUntilTimer = 0;
	bool newSoundFile = false;
	if (_curMusicTrack != _sceneList[sceneId].sound) {
		fadeOutMusic(60);
		waitUntilTimer = _system->getMillis() + 60 * _tickLength;
		newSoundFile = true;
	}

	//XXX

	if (!unk3) {
		_scriptInterpreter->initScript(&_sceneScriptState, &_sceneScriptData);
		_scriptInterpreter->startScript(&_sceneScriptState, 5);
		while (_scriptInterpreter->validScript(&_sceneScriptState)) {
			_scriptInterpreter->runScript(&_sceneScriptState);
			musicUpdate(0);
		}
	}

	musicUpdate(0);

	for (int i = 0; i < 10; ++i)
		_wsaSlots[i]->close();

	musicUpdate(0);

	_specialExitCount = 0;
	Common::set_to(_specialExitTable, _specialExitTable+ARRAYSIZE(_specialExitTable), 0xFFFF);

	musicUpdate(0);

	_mainCharacter.sceneId = sceneId;
	_sceneList[sceneId].flags &= ~1;
	musicUpdate(0);
	unloadScene();
	musicUpdate(0);
	//XXX resetMaskPage();

	for (int i = 0; i < 4; ++i) {
		if (i == _musicSoundChannel || i == _fadeOutMusicChannel)
			_soundDigital->stopSound(i);
	}
	_fadeOutMusicChannel = -1;
	musicUpdate(0);
	loadScenePal();
	musicUpdate(0);

	if (queryGameFlag(0x1D9)) {
		//XXX VQA code here
	}

	musicUpdate(0);
	loadSceneMsc();
	musicUpdate(0);
	_sceneExit1 = _sceneList[sceneId].exit1;
	_sceneExit2 = _sceneList[sceneId].exit2;
	_sceneExit3 = _sceneList[sceneId].exit3;
	_sceneExit4 = _sceneList[sceneId].exit4;

	while (_system->getMillis() > waitUntilTimer)
		_system->delayMillis(10);

	musicUpdate(0);
	initSceneScript(unk3);
	musicUpdate(0);

	if (_overwriteSceneFacing) {
		facing = _mainCharacter.facing;
		_overwriteSceneFacing = false;
	}

	enterNewSceneUnk1(facing, unk2, unk3);
	musicUpdate(0);
	//XXX setCommandLineRestoreTimer(-1);
	_sceneScriptState.regs[3] = 1;
	enterNewSceneUnk2(unk3);
	if (queryGameFlag(0)) {
		_runFlag = false;
	} else {
		if (!--_enterNewSceneLock)
			_unkInputFlag = false;
		//XXX
		if (_itemInHand <= 0) {
			_itemInHand = -1;
			_handItemSet = -1;
			_screen->setMouseCursor(0, 0, _gameShapes[0]);
		}

		Common::Point pos = getMousePos();
		if (pos.y > 187)
			setMousePos(pos.x, 179);
	}
	_screen->showMouse();
}

void KyraEngine_v3::enterNewSceneUnk1(int facing, int unk1, int unk2) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::enterNewSceneUnk1(%d, %d, %d)", facing, unk1, unk2);
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
		if (y2 >= 185)
			y2 = 183;
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
			y2 = 191;
			break;

		case 2:
			x2 = -24;
			break;

		case 4:
			y2 = y - 4;
			break;

		case 6:
			x2 = 343;
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

	if (_mainCharacter.sceneId == 9 && !_soundDigital->isPlaying(_musicSoundChannel))
		playMusicTrack(_sceneList[_mainCharacter.sceneId].sound, 0);
	if (!unk2)
		playMusicTrack(_sceneList[_mainCharacter.sceneId].sound, 0);

	if (unk1 && !unk2 && _mainCharacter.animFrame != 87)
		moveCharacter(facing, x, y);
}

void KyraEngine_v3::enterNewSceneUnk2(int unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::enterNewSceneUnk2(%d)", unk1);
	_unk3 = -1;
	if (_mainCharX == -1 && _mainCharY == -1 && !unk1) {
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
		refreshAnimObjectsIfNeed();
	}

	if (!unk1) {
		runSceneScript4(0);
		//malcolmSceneStartupChat();
	}

	_unk4 = 0;
	_unk3 = -1;
}

void KyraEngine_v3::unloadScene() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::unloadScene()");
	delete [] _sceneStrings;
	_sceneStrings = 0;
	musicUpdate(0);
	_scriptInterpreter->unloadScript(&_sceneScriptData);
	musicUpdate(0);
	freeSceneShapes();
	musicUpdate(0);
	freeSceneAnims();
	musicUpdate(0);
}

void KyraEngine_v3::freeSceneShapes() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::freeSceneShapes()");
	for (uint i = 0; i < ARRAYSIZE(_sceneShapes); ++i) {
		delete [] _sceneShapes[i];
		_sceneShapes[i] = 0;
	}
}

void KyraEngine_v3::freeSceneAnims() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::freeSceneAnims()");
	for (int i = 0; i < 16; ++i) {
		_sceneAnims[i].flags = 0;
		_sceneAnimMovie[i]->close();
	}
}

void KyraEngine_v3::loadScenePal() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadScenePal()");
	char filename[16];
	memcpy(_screen->getPalette(2), _screen->getPalette(0), 768);
	strcpy(filename, _sceneList[_mainCharacter.sceneId].filename1);
	strcat(filename, ".COL");

	_screen->loadBitmap(filename, 3, 3, 0);
	memcpy(_screen->getPalette(2), _screen->getCPagePtr(3), 432);
	memset(_screen->getPalette(2), 0, 3);

	for (int i = 144; i <= 167; ++i) {
		uint8 *palette = _screen->getPalette(2) + i * 3;
		palette[0] = palette[2] = 63;
		palette[1] = 0;
	}

	_screen->generateOverlay(_screen->getPalette(2), _paletteOverlay, 0xF0, 0x19);

	uint8 *palette = _screen->getPalette(2) + 432;
	const uint8 *costPal = _costPalBuffer + _malcolmShapes * 72;
	memcpy(palette, costPal, 24*3);
}

void KyraEngine_v3::loadSceneMsc() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadSceneMsc()");
	char filename[16];
	strcpy(filename, _sceneList[_mainCharacter.sceneId].filename1);
	strcat(filename, ".MSC");

	Common::SeekableReadStream *stream = _res->getFileStream(filename);
	assert(stream);
	int16 minY = 0, height = 0;
	minY = stream->readSint16LE();
	height = stream->readSint16LE();
	delete stream;
	stream = 0;
	musicUpdate(0);
	_maskPageMinY = minY;
	_maskPageMaxY = minY + height - 1;

	_screen->setShapePages(5, 3, _maskPageMinY, _maskPageMaxY);

	musicUpdate(0);
	_screen->loadBitmap(filename, 5, 5, 0, true);

	// HACK
	uint8 data[320*200];
	_screen->copyRegionToBuffer(5, 0, 0, 320, 200, data);
	_screen->fillRect(0, 0, 319, _maskPageMinY - 1, 0xFF, 5);
	_screen->copyBlockToPage(5, 0, _maskPageMinY, 320, height, data);
	_screen->fillRect(0, _maskPageMaxY + 1, 319, 199, 0xFF, 5);

	musicUpdate(0);
}

void KyraEngine_v3::initSceneScript(int unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::initSceneScript(%d)", unk1);
	const SceneDesc &scene = _sceneList[_mainCharacter.sceneId];
	musicUpdate(0);

	char filename[16];
	strcpy(filename, scene.filename1);
	strcat(filename, ".DAT");

	Common::SeekableReadStream *stream = _res->getFileStream(filename);
	assert(stream);
	stream->seek(2, SEEK_CUR);

	byte scaleTable[15];
	stream->read(scaleTable, 15);
	stream->read(_sceneDatPalette, 45);
	stream->read(_sceneDatLayerTable, 15);
	int16 shapesCount = stream->readSint16LE();

	for (int i = 0; i < 15; ++i)
		_scaleTable[i] = (uint16(scaleTable[i]) << 8) / 100;

	if (shapesCount > 0) {
		strcpy(filename, scene.filename1);
		strcat(filename, "9.CPS");
		musicUpdate(0);
		_screen->loadBitmap(filename, 3, 3, 0);
		int pageBackUp = _screen->_curPage;
		_screen->_curPage = 2;
		for (int i = 0; i < shapesCount; ++i) {
			int16 x = stream->readSint16LE();
			int16 y = stream->readSint16LE();
			int16 w = stream->readSint16LE();
			int16 h = stream->readSint16LE();
			_sceneShapes[i] = _screen->encodeShape(x, y, w, h, 0);
			assert(_sceneShapes[i]);
			musicUpdate(0);
		}
		_screen->_curPage = pageBackUp;
		musicUpdate(0);
	}
	musicUpdate(0);

	strcpy(filename, scene.filename1);
	strcat(filename, ".CPS");
	_screen->loadBitmap(filename, 3, 3, 0);
	musicUpdate(0);

	Common::set_to(_specialSceneScriptState, _specialSceneScriptState+ARRAYSIZE(_specialSceneScriptState), false);
	_sceneEnterX1 = 160;
	_sceneEnterY1 = 0;
	_sceneEnterX2 = 296;
	_sceneEnterY2 = 93;
	_sceneEnterX3 = 160;
	_sceneEnterY3 = 171;
	_sceneEnterX4 = 24;
	_sceneEnterY4 = 93;
	_sceneMinY = 0;
	_sceneMaxY = 319;

	_scriptInterpreter->initScript(&_sceneScriptState, &_sceneScriptData);
	strcpy(filename, scene.filename2);
	strcat(filename, ".EMC");
	musicUpdate(0);
	_scriptInterpreter->loadScript(filename, &_sceneScriptData, &_opcodes);

	strcpy(filename, scene.filename1);
	strcat(filename, ".");
	loadLanguageFile(filename, _sceneStrings);
	musicUpdate(0);

	runSceneScript8();
	_scriptInterpreter->startScript(&_sceneScriptState, 0);
	_sceneScriptState.regs[0] = _mainCharacter.sceneId;
	_sceneScriptState.regs[5] = unk1;
	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);

	_screen->copyRegionToBuffer(3, 0, 0, 320, 200, _gamePlayBuffer);
	musicUpdate(0);

	for (int i = 0; i < 10; ++i) {
		_scriptInterpreter->initScript(&_sceneSpecialScripts[i], &_sceneScriptData);
		_scriptInterpreter->startScript(&_sceneSpecialScripts[i], i+9);
		musicUpdate(0);
		_sceneSpecialScriptsTimer[i] = 0;
	}

	_sceneEnterX1 &= ~3;
	_sceneEnterY1 &= ~1;
	_sceneEnterX2 &= ~3;
	_sceneEnterY2 &= ~1;
	_sceneEnterX3 &= ~3;
	_sceneEnterY3 &= ~1;
	_sceneEnterX4 &= ~3;
	_sceneEnterY4 &= ~1;
	musicUpdate(0);
}

void KyraEngine_v3::initSceneAnims(int unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::initSceneAnims(%d)", unk1);
	for (int i = 0; i < 67; ++i)
		_animObjects[i].enabled = false;

	AnimObj *obj = &_animObjects[0];

	if (_mainCharacter.animFrame != 87 && !unk1)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	obj->enabled = true;
	obj->xPos1 = _mainCharacter.x1;
	obj->yPos1 = _mainCharacter.y1;
	obj->shapePtr = getShapePtr(_mainCharacter.animFrame);
	obj->shapeIndex2 = obj->shapeIndex = _mainCharacter.animFrame;
	obj->xPos2 = _mainCharacter.x1;
	obj->yPos2 = _mainCharacter.y1;
	_charScale = getScale(_mainCharacter.x1, _mainCharacter.y1);
	obj->xPos3 = obj->xPos2 += (_malcolmShapeXOffset * _charScale) >> 8;
	obj->yPos3 = obj->yPos2 += (_malcolmShapeYOffset * _charScale) >> 8;
	_mainCharacter.x3 = _mainCharacter.x1 - (_charScale >> 4) - 1;
	_mainCharacter.y3 = _mainCharacter.y1 - (_charScale >> 6) - 1;
	obj->needRefresh = true;
	_animList = 0;

	for (int i = 0; i < 16; ++i) {
		const SceneAnim &anim = _sceneAnims[i];
		obj = &_animObjects[i+1];
		obj->enabled = false;
		obj->needRefresh = false;

		if (anim.flags & 1) {
			obj->enabled = true;
			obj->needRefresh = true;
		}

		obj->unk8 = (anim.flags & 0x20) ? 1 : 0;
		obj->flags = (anim.flags & 0x10) ? 0x800 : 0;
		if (anim.flags & 2)
			obj->flags |= 1;

		obj->xPos1 = anim.x;
		obj->yPos1 = anim.y;

		if ((anim.flags & 4) && anim.shapeIndex != 0xFFFF)
			obj->shapePtr = _sceneShapes[anim.shapeIndex];
		else
			obj->shapePtr = 0;

		if (anim.flags & 8) {
			obj->shapeIndex3 = anim.shapeIndex;
			obj->animNum = i;
		} else {
			obj->shapeIndex3 = 0xFFFF;
			obj->animNum = 0xFFFF;
		}

		obj->xPos3 = obj->xPos2 = anim.x2;
		obj->yPos3 = obj->yPos3 = anim.y2;
		obj->width = anim.width;
		obj->height = anim.height;
		obj->width2 = obj->height2 = anim.specialSize;

		if (anim.flags & 1) {
			if (_animList)
				_animList = addToAnimListSorted(_animList, obj);
			else
				_animList = initAnimList(_animList, obj);
		}
	}

	if (_animList)
		_animList = addToAnimListSorted(_animList, &_animObjects[0]);
	else
		_animList = initAnimList(_animList, &_animObjects[0]);

	for (int i = 0; i < 50; ++i) {
		obj = &_animObjects[i+17];
		const Item &item = _itemList[i];
		if (item.id != 0xFFFF && item.sceneId == _mainCharacter.sceneId) {
			obj->xPos1 = item.x;
			obj->yPos1 = item.y;
			animSetupPaletteEntry(obj);
			obj->shapePtr = 0;
			obj->shapeIndex = obj->shapeIndex2 = item.id + 248;
			obj->xPos2 = item.x;
			obj->yPos2 = item.y;

			int scale = getScale(obj->xPos1, obj->yPos1);
			const uint8 *shape = getShapePtr(obj->shapeIndex);
			obj->xPos3 = obj->xPos2 -= (_screen->getShapeScaledWidth(shape, scale) >> 1);
			obj->yPos3 = obj->yPos2 -= _screen->getShapeScaledHeight(shape, scale) - 1;
			obj->enabled = true;
			obj->needRefresh = true;

			if (_animList)
				_animList = addToAnimListSorted(_animList, obj);
			else
				_animList = initAnimList(_animList, obj);
		} else {
			obj->enabled = false;
			obj->needRefresh = false;
		}
	}

	for (int i = 0; i < 67; ++i)
		_animObjects[i].needRefresh = _animObjects[i].enabled;

	restorePage3();
	drawAnimObjects();
	_screen->hideMouse();
	initSceneScreen(unk1);
	_screen->showMouse();
	refreshAnimObjects(0);
}

void KyraEngine_v3::initSceneScreen(int unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::initSceneScreen(%d)", unk1);
	_screen->copyBlockToPage(2, 0, 188, 320, 12, _interfaceCommandLine);

	if (_unkSceneScreenFlag1) {
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
		return;
	}

	if (_noScriptEnter) {
		memset(_screen->getPalette(0), 0, 432);
		if (!_wsaPlayingVQA)
			_screen->setScreenPalette(_screen->getPalette(0));
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

	if (_noScriptEnter) {
		if (!_wsaPlayingVQA)
			_screen->setScreenPalette(_screen->getPalette(2));
		memcpy(_screen->getPalette(0), _screen->getPalette(2), 432);
		if (_wsaPlayingVQA) {
			_screen->fadeFromBlack(0x3C);
			_wsaPlayingVQA = false;
		}
	}

	updateCharPal(0);

	//XXX when loading from main menu
}

void KyraEngine_v3::updateSpecialSceneScripts() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateSpecialSceneScripts()");
	uint32 nextTime = _system->getMillis() + _tickLength;
	const int startScript = _lastProcessedSceneScript;

	while (_system->getMillis() <= nextTime) {
		if (_sceneSpecialScriptsTimer[_lastProcessedSceneScript] <= _system->getMillis() &&
			!_specialSceneScriptState[_lastProcessedSceneScript]) {
			_specialSceneScriptRunFlag = true;

			while (_specialSceneScriptRunFlag && _sceneSpecialScriptsTimer[_lastProcessedSceneScript] <= _system->getMillis()) {
				if (!_scriptInterpreter->runScript(&_sceneSpecialScripts[_lastProcessedSceneScript]))
					_specialSceneScriptRunFlag = false;
			}
		}

		if (!_scriptInterpreter->validScript(&_sceneSpecialScripts[_lastProcessedSceneScript])) {
			_scriptInterpreter->startScript(&_sceneSpecialScripts[_lastProcessedSceneScript], 9+_lastProcessedSceneScript);
			_specialSceneScriptRunFlag = false;
		}

		++_lastProcessedSceneScript;
		if (_lastProcessedSceneScript >= 10)
			_lastProcessedSceneScript = 0;

		if (_lastProcessedSceneScript == startScript)
			return;
	}
}

void KyraEngine_v3::runSceneScript4(int unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::runSceneScript4(%d)", unk1);
	_sceneScriptState.regs[4] = _itemInHand;
	_sceneScriptState.regs[5] = unk1;
	_sceneScriptState.regs[3] = 0;
	_noStartupChat = false;

	_scriptInterpreter->startScript(&_sceneScriptState, 4);
	while (_scriptInterpreter->validScript(&_sceneScriptState))
		_scriptInterpreter->runScript(&_sceneScriptState);

	if (_sceneScriptState.regs[3])
		_noStartupChat = true;
}

void KyraEngine_v3::runSceneScript8() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::runSceneScript8()");
	_scriptInterpreter->startScript(&_sceneScriptState, 8);
	while (_scriptInterpreter->validScript(&_sceneScriptState)) {
		musicUpdate(0);
		_scriptInterpreter->runScript(&_sceneScriptState);
	}
}

} // end of namespace Kyra
