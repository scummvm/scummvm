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
	virtual uint16 get() = 0;
	virtual uint16 set(uint16 val) = 0;
	virtual uint16 getNum() const { return _num; }
};


class Globals {
public:
	Globals();
	virtual ~Globals();

	uint16 getGlobal(uint16 num);
	uint16 setGlobal(uint16 num, uint16 val);

protected:
	Common::Array<Global *> _globals;
};

// TODO: Work out what this table is even for..
class DragonDataTable {
public:
	DragonDataTable();
	uint16 getValueFromTable();

	uint16 _row;
	uint16 _col;
	uint16 _divBy4;
	uint16 _output;

private:
	int getOffsetForVal(uint16 val);
};

class DragonGameTime {
public:
	DragonGameTime();
	void addGameTime(int mins);

	Global *getMinsGlobal(uint16 num);
	Global *getHoursGlobal(uint16 num);
	Global *getDaysGlobal(uint16 num);

	uint16 _gameMinsAdded;

private:
	int _gameTimeDays;
	int _gameTimeHours;
	int _gameTimeMins;
};

class DragonGlobals : public Globals {
public:
	DragonGlobals();

private:
	uint16 _gameCounterTicksUp;
	uint16 _gameCounterTicksDown;
	uint16 _lastOpcode1SceneChageNum;
	uint16 _sceneOp12SceneNum;
	uint16 _currentSelectedItem;
	uint16 _gameMinsToAdd_1;
	uint16 _gameMinsToAdd_2;
	uint16 _gameMinsToAdd_3;
	uint16 _gameMinsToAdd_4;
	uint16 _gameMinsToAdd_5;
	uint16 _gameGlobal0x57;
	uint16 _gameDays2;
	uint16 _sceneOpcode15Flag;
	uint16 _sceneOpcode15Val;
	uint16 _sceneOpcode100Var;
	uint16 _arcadeModeFlag_3cdc;
	uint16 _opcode106EndMinutes;
	DragonDataTable _table;
	DragonGameTime _time;
	// uint16 _detailSliderSetting; // kept in the engine
};

} // end namespace Dgds

#endif // DGDS_GLOBALS_H
