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

#ifndef NEVERHOOD_GAMEMODULE_H
#define NEVERHOOD_GAMEMODULE_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"

namespace Neverhood {

class GameModule : public Module {
public:
	GameModule(NeverhoodEngine *vm);
	~GameModule() override;
	void startup();
	void requestRestoreGame();
	void requestRestartGame(bool requestMainMenu);
	void redrawPrevChildObject();
	void checkRequests();
	void handleMouseMove(int16 x, int16 y);
	void handleMouseDown(int16 x, int16 y);
	void handleMouseUp(int16 x, int16 y);
	void handleWheelUp();
	void handleWheelDown();
	void handleSpaceKey();
	void handleAsciiKey(char key);
	void handleKeyDown(Common::KeyCode keyCode);
	void handleEscapeKey();
	void initKeySlotsPuzzle();
	void initMemoryPuzzle();
	void initWaterPipesPuzzle();
	void initRadioPuzzle();
	void initTestTubes1Puzzle();
	void initTestTubes2Puzzle();
	void initCannonSymbolsPuzzle();
	void initCodeSymbolsPuzzle();
	void initCubeSymbolsPuzzle();
	void initCrystalColorsPuzzle();
	uint32 getCurrRadioMusicFileHash();
	int getCurrentModuleNum() { return _moduleNum; }
	int getPreviousModuleNum() { return _moduleNum; }

	void createModule(int moduleNum, int which);
protected:
	int _moduleNum;
	Entity *_prevChildObject;
	int _prevModuleNum;
	bool _restoreGameRequested;
	bool _restartGameRequested;
	bool _canRequestMainMenu;
	bool _mainMenuRequested;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createModuleByHash(uint32 nameHash);
	void updateModule();
	void openMainMenu();
	void createMenuModule();
	void updateMenuModule();
};

class NonRepeatingRandomNumbers : public Common::Array<int> {
public:
	NonRepeatingRandomNumbers(Common::RandomSource *rnd, int count);
	int getNumber();
	void removeNumber(int number);
protected:
	Common::RandomSource *_rnd;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE_H */
