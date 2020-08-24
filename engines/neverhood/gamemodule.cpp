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

#include "neverhood/gamemodule.h"

#include "neverhood/graphics.h"
#include "neverhood/menumodule.h"
#include "neverhood/modules/module1000.h"
#include "neverhood/modules/module1100.h"
#include "neverhood/modules/module1200.h"
#include "neverhood/modules/module1300.h"
#include "neverhood/modules/module1400.h"
#include "neverhood/modules/module1500.h"
#include "neverhood/modules/module1600.h"
#include "neverhood/modules/module1700.h"
#include "neverhood/modules/module1800.h"
#include "neverhood/modules/module1900.h"
#include "neverhood/modules/module2000.h"
#include "neverhood/modules/module2100.h"
#include "neverhood/modules/module2200.h"
#include "neverhood/modules/module2300.h"
#include "neverhood/modules/module2400.h"
#include "neverhood/modules/module2500.h"
#include "neverhood/modules/module2600.h"
#include "neverhood/modules/module2700.h"
#include "neverhood/modules/module2800.h"
#include "neverhood/modules/module2900.h"
#include "neverhood/modules/module3000.h"

namespace Neverhood {

static const uint32 kRadioMusicFileHashes[] = {
	0x82B22000,
	0x02B22004,
	0x42B22000,
	0x03322008,
	0x02B22001,
	0x02B22008,
	0x02B22020,
	0x03322001,
	0x03322002,
	0x03322004,
	0x03322040,
	0x02B22002,
	0x02B22010,
	0x03322010,
	0x02B22040,
	0x43322000,
	0x83322000,
	0x03322020
};

enum {
	MENU_MODULE			= 9999
};

GameModule::GameModule(NeverhoodEngine *vm)
	: Module(vm, NULL), _moduleNum(-1), _prevChildObject(NULL), _prevModuleNum(-1),
	_restoreGameRequested(false), _restartGameRequested(false), _canRequestMainMenu(true),
	_mainMenuRequested(false) {

	// Other initializations moved to actual engine class
	_vm->_soundMan->playSoundThree(0x002D0031, 0x08861079);
	SetMessageHandler(&GameModule::handleMessage);
}

GameModule::~GameModule() {
	_vm->_soundMan->deleteSoundGroup(0x002D0031);
	delete _childObject;
	_childObject = NULL;
}

void GameModule::handleMouseMove(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug(2, "GameModule::handleMouseMove(%d, %d)", x, y);
		sendPointMessage(_childObject, NM_MOUSE_MOVE, mousePos);
	}
}

void GameModule::handleMouseDown(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug(2, "GameModule::handleMouseDown(%d, %d)", x, y);
		sendPointMessage(_childObject, NM_MOUSE_CLICK, mousePos);
	}
}

void GameModule::handleMouseUp(int16 x, int16 y) {
	if (_childObject) {
		NPoint mousePos;
		mousePos.x = x;
		mousePos.y = y;
		debug(2, "GameModule::handleMouseUp(%d, %d)", x, y);
		sendPointMessage(_childObject, NM_MOUSE_RELEASE, mousePos);
	}
}

void GameModule::handleWheelUp() {
	if (_childObject) {
		sendMessage(_childObject, NM_MOUSE_WHEELUP, 0);
	}
}

void GameModule::handleWheelDown() {
	if (_childObject) {
		sendMessage(_childObject, NM_MOUSE_WHEELDOWN, 0);
	}
}

void GameModule::handleSpaceKey() {
	if (_childObject) {
		debug(2, "GameModule::handleSpaceKey()");
		sendMessage(_childObject, NM_KEYPRESS_SPACE, 0);
	}
}

void GameModule::handleAsciiKey(char key) {
	if (_childObject) {
		debug(2, "GameModule::handleAsciiKey()");
		sendMessage(_childObject, 0x000A, (uint32)key);
	}
}

void GameModule::handleKeyDown(Common::KeyCode keyCode) {
	if (_childObject) {
		if (keyCode == Common::KEYCODE_ESCAPE)
			handleEscapeKey();
		else if (keyCode == Common::KEYCODE_SPACE)
			handleSpaceKey();
		debug(2, "GameModule::handleKeyDown()");
		sendMessage(_childObject, 0x000B, keyCode);
	}
}

void GameModule::handleEscapeKey() {
	if (_vm->isDemo())
		_vm->quitGame();
	else if (!_prevChildObject && _canRequestMainMenu)
		_mainMenuRequested = true;
	else if (_childObject)
		sendMessage(_childObject, NM_KEYPRESS_ESC, 0);
}

void GameModule::initKeySlotsPuzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x25400B10)) {
		NonRepeatingRandomNumbers keySlots(_vm->_rnd, 16);
		for (uint i = 0; i < 3; i++) {
			setSubVar(VA_GOOD_KEY_SLOT_NUMBERS, i, keySlots.getNumber());
			setSubVar(VA_CURR_KEY_SLOT_NUMBERS, i, keySlots.getNumber());
		}
		setSubVar(VA_IS_PUZZLE_INIT, 0x25400B10, 1);
	}
}

void GameModule::initMemoryPuzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0xC8606803)) {
		NonRepeatingRandomNumbers diceIndices(_vm->_rnd, 3);
		NonRepeatingRandomNumbers availableTiles(_vm->_rnd, 48);
		NonRepeatingRandomNumbers tileSymbols(_vm->_rnd, 10);
		for (uint32 i = 0; i < 3; i++)
			setSubVar(VA_CURR_DICE_NUMBERS, i, 1);
		// Set special symbols
		// Symbol 5 is always one of the three special symbols
		setSubVar(VA_DICE_MEMORY_SYMBOLS, diceIndices.getNumber(), 5);
		tileSymbols.removeNumber(5);
		for (int i = 0; i < 2; i++)
			setSubVar(VA_DICE_MEMORY_SYMBOLS, diceIndices.getNumber(), tileSymbols.getNumber());
		// Insert special symbols tiles
		for (uint32 i = 0; i < 3; ++i) {
			int tileSymbolOccurence = _vm->_rnd->getRandomNumber(4 - 1) * 2 + 2;
			setSubVar(VA_GOOD_DICE_NUMBERS, i, tileSymbolOccurence);
			while (tileSymbolOccurence--)
				setSubVar(VA_TILE_SYMBOLS, availableTiles.getNumber(), getSubVar(VA_DICE_MEMORY_SYMBOLS, i));
		}
		// Fill the remaining tiles
		uint32 tileSymbolIndex = 0;
		while (!availableTiles.empty()) {
			setSubVar(VA_TILE_SYMBOLS, availableTiles.getNumber(), tileSymbols[tileSymbolIndex]);
			setSubVar(VA_TILE_SYMBOLS, availableTiles.getNumber(), tileSymbols[tileSymbolIndex]);
			tileSymbolIndex++;
			if (tileSymbolIndex >= tileSymbols.size())
				tileSymbolIndex = 0;
		}
		setSubVar(VA_IS_PUZZLE_INIT, 0xC8606803, 1);
	}
}

void GameModule::initWaterPipesPuzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x40520234)) {
		setSubVar(VA_GOOD_WATER_PIPES_LEVEL, 0, 3);
		setSubVar(VA_GOOD_WATER_PIPES_LEVEL, 1, 1);
		setSubVar(VA_GOOD_WATER_PIPES_LEVEL, 2, 2);
		setSubVar(VA_GOOD_WATER_PIPES_LEVEL, 3, 0);
		setSubVar(VA_GOOD_WATER_PIPES_LEVEL, 4, 4);
		setSubVar(VA_IS_PUZZLE_INIT, 0x40520234, 1);
	}
}

void GameModule::initRadioPuzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x08C80800)) {
		int currMusicIndex = _vm->_rnd->getRandomNumber(5 - 1) + 3;
		setGlobalVar(V_GOOD_RADIO_MUSIC_INDEX, 5 * currMusicIndex);
		setGlobalVar(V_GOOD_RADIO_MUSIC_NAME, kRadioMusicFileHashes[currMusicIndex]);
		setGlobalVar(V_RADIO_ROOM_LEFT_DOOR, 1);
		setGlobalVar(V_RADIO_ROOM_RIGHT_DOOR, 0);
		setSubVar(VA_IS_PUZZLE_INIT, 0x08C80800, 1);
	}
}

void GameModule::initTestTubes1Puzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x20479010)) {
		for (uint i = 0; i < 3; i++)
			setSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, i, _vm->_rnd->getRandomNumber(3 - 1) + 1);
		setSubVar(VA_IS_PUZZLE_INIT, 0x20479010, 1);
	}
}

void GameModule::initTestTubes2Puzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x66059818)) {
		for (uint i = 0; i < 3; i++)
			setSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, i, _vm->_rnd->getRandomNumber(6 - 1) + 1);
		setSubVar(VA_IS_PUZZLE_INIT, 0x66059818, 1);
	}
}

void GameModule::initCannonSymbolsPuzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x8C9819C2)) {
		for (int i = 0; i < 3; i++) {
			setSubVar(VA_GOOD_CANNON_SYMBOLS_1, i, _vm->_rnd->getRandomNumber(12 - 1));
			setSubVar(VA_GOOD_CANNON_SYMBOLS_2, i, _vm->_rnd->getRandomNumber(12 - 1));
		}
		setSubVar(VA_IS_PUZZLE_INIT, 0x8C9819C2, 1);
	}
}

void GameModule::initCodeSymbolsPuzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x0CD09B50)) {
		for (int i = 0; i < 12; ++i)
			setSubVar(VA_CODE_SYMBOLS, i, i);
		for (int i = 0; i < 12; ++i) {
			uint32 index1 = _vm->_rnd->getRandomNumber(12 - 1);
			uint32 index2 = _vm->_rnd->getRandomNumber(12 - 1);
			uint32 temp = getSubVar(VA_CODE_SYMBOLS, index1);
			setSubVar(VA_CODE_SYMBOLS, index1, getSubVar(VA_CODE_SYMBOLS, index2));
			setSubVar(VA_CODE_SYMBOLS, index2, temp);
		}
		setGlobalVar(V_NOISY_SYMBOL_INDEX, _vm->_rnd->getRandomNumber(11 - 1) + 1);
		setSubVar(VA_IS_PUZZLE_INIT, 0x0CD09B50, 1);
	}
}

void GameModule::initCubeSymbolsPuzzle() {
	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x60400854)) {
		NonRepeatingRandomNumbers cubeSymbols(_vm->_rnd, 9);
		for (uint32 cubePosition = 0; cubePosition < 9; ++cubePosition)
			setSubVar(VA_CUBE_POSITIONS, cubePosition, (uint32)(cubeSymbols.getNumber() - 1));
		setSubVar(VA_IS_PUZZLE_INIT, 0x60400854, 1);
	}
}

void GameModule::initCrystalColorsPuzzle() {
	if (!getGlobalVar(V_CRYSTAL_COLORS_INIT)) {
		TextResource textResource(_vm);
		const char *textStart, *textEnd;
		textResource.load(0x46691611);
		textStart = textResource.getString(0, textEnd);
		for (uint index = 0; index < 5; index++) {
			char colorLetter = (byte)textStart[index];
			byte correctColorNum = 0, misalignedColorNum;
			switch (colorLetter) {
			case 'B':
				correctColorNum = 4;
				break;
			case 'G':
				correctColorNum = 3;
				break;
			case 'O':
				correctColorNum = 1;
				break;
			case 'R':
				correctColorNum = 0;
				break;
			case 'V':
				correctColorNum = 5;
				break;
			case 'Y':
				correctColorNum = 2;
				break;
			default:
				break;
			}
			do {
				misalignedColorNum = _vm->_rnd->getRandomNumber(6 - 1);
			} while (misalignedColorNum == correctColorNum);
			setSubVar(VA_GOOD_CRYSTAL_COLORS, index, correctColorNum);
			setSubVar(VA_CURR_CRYSTAL_COLORS, index, misalignedColorNum);
		}
		setGlobalVar(V_CRYSTAL_COLORS_INIT, 1);
	}
}

uint32 GameModule::getCurrRadioMusicFileHash() {
	uint musicIndex = getGlobalVar(V_CURR_RADIO_MUSIC_INDEX);
	return (musicIndex % 5 != 0) ? 0 : kRadioMusicFileHashes[CLIP<uint>(musicIndex / 5, 0, 17)];
}


uint32 GameModule::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0800:
		_canRequestMainMenu = true;
		break;
	case 0x1009:
		_moduleResult = param.asInteger();
		_done = true;
		break;
	default:
		break;
	}
	return messageResult;
}

void GameModule::startup() {
#if 1
	// Logos and intro video // Real game start
	createModule(1500, 0);
#else
	// DEBUG>>>
	/*
	setGlobalVar(V_SEEN_MUSIC_BOX, 1);
	setGlobalVar(V_CREATURE_EXPLODED, 0);
	setGlobalVar(V_MATCH_STATUS, 0);
	setGlobalVar(V_PROJECTOR_LOCATION, 2);
	*/
	//setGlobalVar(V_ENTRANCE_OPEN, 0);
	//setGlobalVar(V_DOOR_SPIKES_OPEN, 1);
	//setGlobalVar(V_CREATURE_ANGRY, 1);
	setGlobalVar(V_RADIO_ENABLED, 1);
	//setGlobalVar(V_TNT_DUMMY_BUILT, 1);
	setGlobalVar(V_FLYTRAP_RING_DOOR, 1);
	setGlobalVar(V_TV_JOKE_TOLD, 1);
	/*
	// Give all disks
	for (int i = 0; i < 20; i++)
		setSubVar(VA_IS_TAPE_INSERTED, i, 1);
	*/
	setSubVar(VA_IS_KEY_INSERTED, 0, 1);
	setSubVar(VA_IS_KEY_INSERTED, 1, 1);
	setSubVar(VA_IS_KEY_INSERTED, 2, 1);
	for (uint32 index = 0; index < 9; index++)
		setSubVar(VA_CUBE_POSITIONS, index, 7 - index);
	setGlobalVar(V_WALL_BROKEN, 0);
	setGlobalVar(V_WORLDS_JOINED, 1);
	setGlobalVar(V_RADIO_MOVE_DISH_VIDEO, 0);
	// Enable all locations
	for (int i = 0; i < 6; i++)
		setSubVar(V_TELEPORTER_DEST_AVAILABLE, i, 1);
	//setGlobalVar(V_PROJECTOR_LOCATION, 4);
	setGlobalVar(V_KEYDOOR_UNLOCKED, 1);
	setGlobalVar(V_LIGHTS_ON, 1);
	setGlobalVar(V_WATER_RUNNING, 1);
	setGlobalVar(V_HAS_TEST_TUBE, 1);
	setSubVar(VA_CURR_WATER_PIPES_LEVEL, 0, 3);
	setSubVar(VA_CURR_WATER_PIPES_LEVEL, 1, 1);
	setSubVar(VA_CURR_WATER_PIPES_LEVEL, 2, 2);
	setSubVar(VA_CURR_WATER_PIPES_LEVEL, 3, 0);
	setSubVar(VA_CURR_WATER_PIPES_LEVEL, 4, 4);
	setGlobalVar(V_KLAYMEN_SMALL, 1);
	setGlobalVar(V_SHRINK_LIGHTS_ON, 0);
	// <<<DEBUG

#if 1
	_vm->gameState().which = 0;
	_vm->gameState().sceneNum = 0;
	createModule(2400, 0);
#endif

#endif
}

void GameModule::requestRestoreGame() {
	_restoreGameRequested = true;
}

void GameModule::requestRestartGame(bool requestMainMenu) {
	_restartGameRequested = true;
	_mainMenuRequested = requestMainMenu;
}

void GameModule::redrawPrevChildObject() {
	if (_prevChildObject) {
		_prevChildObject->draw();
		_vm->_screen->update();
	}
}

void GameModule::checkRequests() {
	if (_restartGameRequested) {
		_restartGameRequested = false;
		_vm->_gameVars->clear();
		requestRestoreGame();
	}
	if (_restoreGameRequested) {
		_restoreGameRequested = false;
		_vm->_audioResourceMan->stopAllMusic();
		_vm->_audioResourceMan->stopAllSounds();
		_vm->_soundMan->stopAllMusic();
		_vm->_soundMan->stopAllSounds();
		// Reinsert turning sound because SoundMan::stopAllSounds() removes it
		_vm->_soundMan->playSoundThree(0x002D0031, 0x08861079);
		delete _childObject;
		delete _prevChildObject;
		_childObject = NULL;
		_prevChildObject = NULL;
		_prevModuleNum = 0;
		createModuleByHash(getGlobalVar(V_MODULE_NAME));
	}
	if (_mainMenuRequested)
		openMainMenu();
}

void GameModule::createModule(int moduleNum, int which) {
	debug(1, "GameModule::createModule(%d, %d)", moduleNum, which);
	_moduleNum = moduleNum;

	delete _childObject;

	switch (_moduleNum) {
	case 1000:
		setGlobalVar(V_MODULE_NAME, 0x03294419);
		_childObject = new Module1000(_vm, this, which);
		break;
	case 1100:
		setGlobalVar(V_MODULE_NAME, 0x0002C818);
		_childObject = new Module1100(_vm, this, which);
		break;
	case 1200:
		setGlobalVar(V_MODULE_NAME, 0x00478311);
		_childObject = new Module1200(_vm, this, which);
		break;
	case 1300:
		setGlobalVar(V_MODULE_NAME, 0x0061C090);
		_childObject = new Module1300(_vm, this, which);
		break;
	case 1400:
		setGlobalVar(V_MODULE_NAME, 0x00AD0012);
		_childObject = new Module1400(_vm, this, which);
		break;
	case 1500:
		_canRequestMainMenu = false;
		setGlobalVar(V_MODULE_NAME, 0x00F10114);
		_childObject = new Module1500(_vm, this, which);
		break;
	case 1600:
		setGlobalVar(V_MODULE_NAME, 0x01A008D8);
		_childObject = new Module1600(_vm, this, which);
		break;
	case 1700:
		setGlobalVar(V_MODULE_NAME, 0x04212331);
		_childObject = new Module1700(_vm, this, which);
		break;
	case 1800:
		setGlobalVar(V_MODULE_NAME, 0x04A14718);
		_childObject = new Module1800(_vm, this, which);
		break;
	case 1900:
		setGlobalVar(V_MODULE_NAME, 0x04E1C09C);
		_childObject = new Module1900(_vm, this, which);
		break;
	case 2000:
		setGlobalVar(V_MODULE_NAME, 0x08250000);
		_childObject = new Module2000(_vm, this, which);
		break;
	case 2100:
		setGlobalVar(V_MODULE_NAME, 0x10A10C14);
		_childObject = new Module2100(_vm, this, which);
		break;
	case 2200:
		setGlobalVar(V_MODULE_NAME, 0x11391412);
		_childObject = new Module2200(_vm, this, which);
		break;
	case 2300:
		setGlobalVar(V_MODULE_NAME, 0x1A214010);
		_childObject = new Module2300(_vm, this, which);
		break;
	case 2400:
		setGlobalVar(V_MODULE_NAME, 0x202D1010);
		_childObject = new Module2400(_vm, this, which);
		break;
	case 2500:
		setGlobalVar(V_MODULE_NAME, 0x29220120);
		_childObject = new Module2500(_vm, this, which);
		break;
	case 2600:
		setGlobalVar(V_MODULE_NAME, 0x40271018);
		_childObject = new Module2600(_vm, this, which);
		break;
	case 2700:
		setGlobalVar(V_MODULE_NAME, 0x42212411);
		_childObject = new Module2700(_vm, this, which);
		break;
	case 2800:
		setGlobalVar(V_MODULE_NAME, 0x64210814);
		_childObject = new Module2800(_vm, this, which);
		break;
	case 2900:
		setGlobalVar(V_MODULE_NAME, 0x81100020);
		if (which >= 0)
			setGlobalVar(V_TELEPORTER_CURR_LOCATION, which);
		_childObject = new Module2900(_vm, this, which);
		break;
	case 3000:
		setGlobalVar(V_MODULE_NAME, 0x81293110);
		_childObject = new Module3000(_vm, this, which);
		break;
	case 9999:
		createDemoScene();
		break;
	default:
		error("GameModule::createModule() Could not create module %d", moduleNum);
	}
	SetUpdateHandler(&GameModule::updateModule);
	_childObject->handleUpdate();
}

void GameModule::createModuleByHash(uint32 nameHash) {
	debug(1, "GameModule::createModuleByHash(%08X)", nameHash);
	switch (nameHash) {
	case 0x03294419:
		createModule(1000, -1);
		break;
	case 0x0002C818:
		createModule(1100, -1);
		break;
	case 0x00478311:
		createModule(1200, -1);
		break;
	case 0x0061C090:
		createModule(1300, -1);
		break;
	case 0x00AD0012:
		createModule(1400, -1);
		break;
	case 0x00F10114:
		createModule(1500, -1);
		break;
	case 0x01A008D8:
		createModule(1600, -1);
		break;
	case 0x04212331:
		createModule(1700, -1);
		break;
	case 0x04A14718:
		createModule(1800, -1);
		break;
	case 0x04E1C09C:
		createModule(1900, -1);
		break;
	case 0x08250000:
		createModule(2000, -1);
		break;
	case 0x10A10C14:
		createModule(2100, -1);
		break;
	case 0x11391412:
		createModule(2200, -1);
		break;
	case 0x1A214010:
		createModule(2300, -1);
		break;
	case 0x202D1010:
		createModule(2400, -1);
		break;
	case 0x29220120:
		createModule(2500, -1);
		break;
	case 0x40271018:
		createModule(2600, -1);
		break;
	case 0x42212411:
		createModule(2700, -1);
		break;
	case 0x64210814:
		createModule(2800, -1);
		break;
	case 0x81100020:
		createModule(2900, -1);
		break;
	case 0x81293110:
		createModule(3000, -1);
		break;
	default:
		createModule(1000, 0);
		break;
	}
}

void GameModule::updateModule() {
	if (!updateChild()) {
		switch (_moduleNum) {
		case 1000:
			createModule(2300, 0);
			break;
		case 1200:
			if (_moduleResult == 1)
				createModule(2600, 0);
			else
				createModule(2300, 2);
			break;
		case 1100:
			if (_moduleResult == 0)
				createModule(2900, 2);
			else {
				setGlobalVar(V_ENTRANCE_OPEN, 1);
				createModule(1300, 0);
			}
			break;
		case 1300:
			if (_moduleResult == 1) {
				// The game was successfully finished
				requestRestartGame(true);
			} else
				createModule(2900, 0);
			break;
		case 1400:
			createModule(1600, 1);
			break;
		case 1500:
			createModule(1000, 0);
			break;
		case 1600:
			if (_moduleResult == 1)
				createModule(1400, 0);
			else if (_moduleResult == 2)
				createModule(1700, 0);
			else
				createModule(2100, 0);
			break;
		case 1700:
			if (_moduleResult == 1)
				createModule(2900, 3);
			else
				createModule(1600, 2);
			break;
		case 1800:
			if (_moduleResult == 1) {
				// Game over, Klaymen jumped into the hole
				requestRestartGame(true);
			} else if (_moduleResult == 2)
				createModule(2700, 0);
			else if (_moduleResult == 3)
				createModule(3000, 3);
			else
				createModule(2800, 0);
			break;
		case 1900:
			createModule(3000, 1);
			break;
		case 2000:
			createModule(2900, 4);
			break;
		case 2100:
			if (_moduleResult == 1)
				createModule(2900, 1);
			else
				createModule(1600, 0);
			break;
		case 2200:
			createModule(2300, 1);
			break;
		case 2300:
		debug(1, "module 23000 _moduleResult : %d", _moduleResult);
			if (_moduleResult == 2)
				createModule(1200, 0);
			else if (_moduleResult == 0)
				createModule(1000, 1);
			else if (_vm->isDemo())
				createModule(9999, -1);
			else if (_moduleResult == 1)
				createModule(2200, 0);
			else if (_moduleResult == 3)
				createModule(2400, 0);
			else if (_moduleResult == 4)
				createModule(3000, 0);
			break;
		case 2400:
			createModule(2300, 3);
			break;
		case 2500:
			createModule(2600, 1);
			break;
		case 2600:
			if (_vm->isDemo() && !_vm->isBigDemo())
				createModule(9999, -1);
			else if (_moduleResult == 1)
				createModule(2500, 0);
			else
				createModule(1200, 1);
			break;
		case 2700:
			createModule(1800, 2);
			break;
		case 2800:
			if (_moduleResult == 1)
				createModule(2900, 5);
			else
				createModule(1800, 0);
			break;
		case 2900:
			if (_moduleResult != (uint32)-1) {
				switch (_moduleResult) {
				case 0:
					createModule(1300, 5);
					break;
				case 1:
					createModule(2100, 1);
					break;
				case 2:
					createModule(1100, 1);
					break;
				case 3:
					setSubVar(V_TELEPORTER_DEST_AVAILABLE, 2, 1);
					createModule(1700, 1);
					break;
				case 4:
					createModule(2000, 0);
					break;
				case 5:
				default:
					createModule(2800, 1);
					break;
				}
			} else {
				switch (getGlobalVar(V_TELEPORTER_CURR_LOCATION)) {
				case 0:
					createModule(1300, 6);
					break;
				case 1:
					createModule(2100, 2);
					break;
				case 2:
					createModule(1100, 2);
					break;
				case 3:
					createModule(1700, 2);
					break;
				case 4:
					createModule(2000, 1);
					break;
				case 5:
				default:
					createModule(2800, 2);
					break;
				}
			}
			setGlobalVar(V_TELEPORTER_CURR_LOCATION, 0);
			break;
		case 3000:
			// NOTE _moduleResult 2 never used
			// NOTE Check if _moduleResult 4 is used
			if (_moduleResult == 1)
				createModule(1900, 0);
			else if (_moduleResult == 3)
				createModule(1800, 3);
			else if (_moduleResult == 4)
				createModule(3000, 0);
			else
				createModule(2300, 4);
			break;
		case 9999:
			createModuleByHash(getGlobalVar(V_MODULE_NAME));
			break;
		default:
			break;
		}
	}
}

void GameModule::openMainMenu() {
	if (_childObject) {
		sendMessage(_childObject, NM_MOUSE_HIDE, 0);
		_childObject->draw();
	} else {
		// If there's no module, create one so there's something to return to
		createModule(1000, 0);
	}
	_vm->_screen->saveParams();
	_vm->_screen->update();
	_mainMenuRequested = false;
	createMenuModule();
}

void GameModule::createMenuModule() {
	if (!_prevChildObject) {
		_prevChildObject = _childObject;
		_prevModuleNum = _moduleNum;
		_childObject = new MenuModule(_vm, this, 0);
		_childObject->handleUpdate();
		SetUpdateHandler(&GameModule::updateMenuModule);
	}
}

void GameModule::updateMenuModule() {
	if (!updateChild()) {
		_vm->_screen->restoreParams();
		_childObject = _prevChildObject;
		sendMessage(_childObject, NM_MOUSE_SHOW, 0);
		_prevChildObject = NULL;
		_moduleNum = _prevModuleNum;
		SetUpdateHandler(&GameModule::updateModule);
	}
}

NonRepeatingRandomNumbers::NonRepeatingRandomNumbers(Common::RandomSource *rnd, int count)
	: _rnd(rnd) {
	for (int i = 0; i < count; i++)
		push_back(i);
}

int NonRepeatingRandomNumbers::getNumber() {
	int number;
	if (!empty()) {
		uint index = _rnd->getRandomNumber(size() - 1);
		number = (*this)[index];
		remove_at(index);
	} else
		number = 0;
	return number;
}

void NonRepeatingRandomNumbers::removeNumber(int number) {
	for (uint i = 0; i < size(); ++i)
		if ((*this)[i] == number) {
			remove_at(i);
			break;
		}
}

} // End of namespace Neverhood
