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
	uint16 _num;
	virtual int16 get() = 0;
	virtual int16 set(int16 val) = 0;
	virtual uint16 getNum() const { return _num; }
};

template<typename T> class ReadOnlyGlobal : public Global {
public:
	ReadOnlyGlobal(uint16 num, const T *val) : Global(num), _val(val) {}
	int16 get() override { return *_val; }
	int16 set(int16 val) override { return *_val; }
private:
	const T *_val;
};

template<typename T> class ReadWriteGlobal : public Global {
public:
	ReadWriteGlobal(uint16 num, T *val) : Global(num), _val(val) {}
	int16 get() override { return *_val; }
	int16 set(int16 val) override { *_val = val; return *_val; }
private:
	T *_val;
};


class Globals {
public:
	Globals();
	virtual ~Globals();

	int16 getGlobal(uint16 num);
	int16 setGlobal(uint16 num, int16 val);

	virtual Common::Error syncState(Common::Serializer &s) { return Common::kNoError; };
protected:
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
	int getOffsetForVal(uint16 val);
};


class DragonGlobals : public Globals {
public:
	DragonGlobals(Clock &clock);

	int16 getGameMinsToAddOnLClick() const { return _gameMinsToAddOnLClick; }
	int16 getGameMinsToAddOnStartDrag() const { return _gameMinsToAddOnStartDrag; }
	int16 getGameMinsToAddOnRClick() const { return _gameMinsToAddOnRClick; }
	int16 getGameMinsToAddOnDragFinished() const { return _gameMinsToAddOnDragFinished; }
	int16 getGameMinsToAddOnObjInteraction() const { return _gameMinsToAddOnObjInteraction; }

private:
	int16 _lastOpcode1SceneChageNum;
	int16 _sceneOp12SceneNum;
	int16 _currentSelectedItem;
	int16 _gameMinsToAddOnLClick;
	int16 _gameMinsToAddOnStartDrag;
	int16 _gameMinsToAddOnRClick;
	int16 _gameMinsToAddOnDragFinished;
	int16 _gameMinsToAddOnObjInteraction;
	int16 _gameGlobal0x57;
	int16 _sceneOpcode15FromScene;
	int16 _sceneOpcode15ToScene;
	int16 _sceneOpcode100Var;
	int16 _arcadeModeFlag_3cdc;
	int16 _opcode106EndMinutes;
	DragonDataTable _table;
	// Clock _clock; // kept in the engine
	// uint16 _detailSliderSetting; // kept in the engine

	Common::Error syncState(Common::Serializer &s) override;
};

} // end namespace Dgds

#endif // DGDS_GLOBALS_H
