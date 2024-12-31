/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DGDS_GLOBALS_H
#define DGDS_GLOBALS_H

#include "common/types.h"
#include "common/array.h"

#include "dgds/dgds.h"

namespace Dgds {

class Clock;

/**
 * The original game uses a struct here with 3 entries:
 * 1. global variable num (uint16)
 * 2. pointer to the variable (uint16)
 * 3. pointer - get+set function ptr **or** 1 == R/W **or** 0 == RO (32 bits)
 * We make that a bit nicer using c++.
 *
 * The original also creates a lookup table to do O(1) lookups of each global,
 * but there are only ~20 of them so we just iterate through each time.
  */
class Global {
public:
	Global(uint16 num) : _num(num) {}
	virtual ~Global() {}
	virtual int16 get() = 0;
	virtual int16 set(int16 val) = 0;
	virtual uint16 getNum() const { return _num; }
	virtual void setRaw(int16 val) = 0; /// only for use in loading state.
private:
	uint16 _num;
};

/*
 This is a bit of a misnomer - the global is readonly during play,
 but it can be set by load/save or restart operations
*/
template<typename T> class ReadOnlyGlobal : public Global {
public:
	ReadOnlyGlobal(uint16 num, T *val) : Global(num), _val(val) {}
	int16 get() override { return *_val; }
	int16 set(int16 val) override { return *_val; }
	void setRaw(int16 val) override { *_val = val; }
private:
	T *_val;
};

template<typename T> class ReadWriteGlobal : public Global {
public:
	ReadWriteGlobal(uint16 num, T *val) : Global(num), _val(val) {}
	int16 get() override { return *_val; }
	int16 set(int16 val) override { *_val = val; return *_val; }
	void setRaw(int16 val) override { *_val = val; }
private:
	T *_val;
};


class Globals {
public:
	Globals(Clock &clock);
	virtual ~Globals();

	int16 getGlobal(uint16 num);
	int16 setGlobal(uint16 num, int16 val);

	virtual Common::Error syncState(Common::Serializer &s); // note: children should call parent first
	Common::Array<Global *> &getAllGlobals() { return _globals; }

	int16 getGameMinsToAddOnLClick() const { return _gameMinsToAddOnLClick; }
	int16 getGameMinsToAddOnStartDrag() const { return _gameMinsToAddOnStartDrag; }
	int16 getGameMinsToAddOnRClick() const { return _gameMinsToAddOnRClick; }
	int16 getGameMinsToAddOnDragFinished() const { return _gameMinsToAddOnDragFinished; }
	int16 getGameMinsToAddOnObjInteraction() const { return _gameMinsToAddOnObjInteraction; }
	int16 getGameIsInteractiveGlobal() { return _gameIsInteractiveGlobal; }

	void setLastSceneNum(int16 num) { _lastOpcode1SceneChangeNum = num; }
	int16 getLastSceneNum() const { return _lastOpcode1SceneChangeNum; }

protected:

	// these ones seem to be common between games
	int16 _lastOpcode1SceneChangeNum;
	int16 _sceneOp12SceneNum;
	int16 _currentSelectedItem;
	int16 _gameMinsToAddOnLClick;
	int16 _gameMinsToAddOnStartDrag;
	int16 _gameMinsToAddOnRClick;
	int16 _gameMinsToAddOnDragFinished;
	int16 _gameMinsToAddOnObjInteraction;
	int16 _gameIsInteractiveGlobal; // used to decide if the game can start a "meanwhile" sequence
	int16 _sceneOpcode15FromScene;
	int16 _sceneOpcode15ToScene;

	Common::Array<Global *> _globals;
};

// TODO: Work out what this table is even for..
class DragonDataTable {
public:
	DragonDataTable();
	uint16 getValueFromTable();

	int16 _row;
	int16 _col;
	int16 _divBy4;
	int16 _output;

private:
	int getOffsetForVal(uint16 val) const;
};


class DragonGlobals : public Globals {
public:
	DragonGlobals(Clock &clock);

	int16 getArcadeState() const { return _arcadeState; }
	void setArcadeState(int16 state) { _arcadeState = state; }

private:
	// Dragon-specific globals
	int16 _sceneOpcode100Var;
	int16 _arcadeState;
	int16 _opcode106EndMinutes;
	DragonDataTable _table;

	Common::Error syncState(Common::Serializer &s) override;
};

class HocGlobals : public Globals {
public:
	HocGlobals(Clock &clock);

	int16 getSheckels() const { return _sheckels; }

	int16 getShellBet() const { return _shellBet; }
	void setShellBet(int16 bet) { _shellBet = bet; }

	int16 getShellPea() const { return _shellPea; }
	void setShellPea(int16 pea) { _shellPea = pea; }

	int16 getNativeGameState() const { return _nativeGameState; }
	void setNativeGameState(int16 state) { _nativeGameState = state; }

	int16 getTrainState() const { return _trainState; }
	void setTrainState(int16 state) { _trainState = state; }

	int16 getIntroState() const { return _introState; }
	void setIntroState(int16 state) { _introState = state; }

private:
	// HoC-specific globals
	int16 _introState;
	int16 _startScene;
	int16 _trainState;
	int16 _shellPea;
	int16 _shellBet;
	int16 _sheckels;
	int16 _unk45;
	int16 _unk46;
	int16 _unk47;
	int16 _tankState;
	int16 _nativeGameState; // state for the shell game, tank game, etc.
	int16 _tankFinished;
	int16 _currentCharacter;
	int16 _currentCharacter2;
	int16 _unkDlgDlgNum;
	int16 _unkDlgFileNum;
	int16 _unk55;
	int16 _difficultyLevel;

	Common::Error syncState(Common::Serializer &s) override;
};

class WillyGlobals : public Globals {
public:
	WillyGlobals(Clock &clock);

	void setPalFade(int16 val) { _palFade = val; }
	int16 getPalFade() const { return _palFade; }

	void setDroppedItemNum(int16 val) { _droppedItemNum = val; }
	bool isHideMouseCursor() const { return _hideMouseCursor != 0; }
	bool isDrawTimeSkipButtons() const { return _invDrawTimeSkipButtons != 0; }

private:
	// Willy-specific globals
	int16 _trouble;
	int16 _unk3;
	int16 _invDrawTimeSkipButtons;
	int16 _hideMouseCursor;
	int16 _unk74;
	int16 _unk75;
	int16 _palFade;
	int16 _droppedItemNum;
	int16 _characterStance;
	int16 _characterPos;
	int16 _unk81;
	int16 _unk82;

	Common::Error syncState(Common::Serializer &s) override;
};

} // end namespace Dgds

#endif // DGDS_GLOBALS_H
