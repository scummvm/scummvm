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

#include "dgds/globals.h"
#include "dgds/dgds.h"

namespace Dgds {

template<typename T> class ReadOnlyGlobal : public Global {
public:
	ReadOnlyGlobal(uint16 num, const T *val) : Global(num), _val(val) {}
	uint16 get() override { return *_val; }
	uint16 set(uint16 val) override { return *_val; }
private:
	const T *_val;
};

template<typename T> class ReadWriteGlobal : public Global {
public:
	ReadWriteGlobal(uint16 num, T *val) : Global(num), _val(val) {}
	uint16 get() override { return *_val; }
	uint16 set(uint16 val) override { *_val = val; return *_val; }
private:
	T *_val;
};

typedef ReadOnlyGlobal<uint16> ROU16Global;
typedef ReadWriteGlobal<uint16> RWU16Global;

////////////////////////////////

Globals::Globals() {
}

Globals::~Globals() {
	for (auto &g : _globals) {
		delete g;
	}
}

uint16 Globals::getGlobal(uint16 num) {
	for (auto &global : _globals) {
		if (global->_num == num) {
			return global->get();
		}
	}
	return 0;
}

uint16 Globals::setGlobal(uint16 num, uint16 val) {
	for (auto &global : _globals) {
		if (global->_num == num) {
			return global->set(val);
		}
	}
	return 0;
}

////////////////////////////////

class DetailLevelROGlobal : public Global {
public:
	DetailLevelROGlobal(uint16 num) : Global(num) {}
	uint16 get() override { return static_cast<DgdsEngine *>(g_engine)->getDetailLevel(); }
	uint16 set(uint16 val) override { return static_cast<DgdsEngine *>(g_engine)->getDetailLevel(); }
};

////////////////////////////////

static byte dragonDataTableOffsets[] = {0x9, 0x29, 0xF, 0x15, 0x5C, 0x19, 0x28, 0x1F};

static byte dragonDataTable[] = {
	 0x4,  0x8, 0x16,  0xE,  0x8,  0xE, 0x17, 0x1C,
	 0x8,  0x2, 0x18,  0x8, 0x10, 0x10, 0x17, 0x15,
	0x16, 0x18,  0x3, 0x20, 0x1E, 0x24, 0x32, 0x30,
	 0xE,  0x8, 0x20,  0x3, 0x13,  0xE, 0x14, 0x12,
	 0x8, 0x10, 0x1E, 0x13,  0x3,  0x8, 0x12, 0x22,
	 0xE, 0x10, 0x24,  0xE,  0x8,  0x3,  0xB, 0x20,
	0x17, 0x17, 0x32, 0x14, 0x12,  0xB,  0x4, 0x24,
	0x1C, 0x15, 0x30, 0x12, 0x22, 0x20, 0x24,  0x4
};

DragonDataTable::DragonDataTable() : _row(0), _col(0), _divBy4(0), _output(0) {}

int DragonDataTable::getOffsetForVal(uint16 val) {
	for (int i = 0; i < ARRAYSIZE(dragonDataTableOffsets); i++) {
		if (dragonDataTableOffsets[i] == val)
			return i;
	}
	return 0;
}

uint16 DragonDataTable::getValueFromTable() {
	int row = getOffsetForVal(_row);
	int col = getOffsetForVal(_col);
	_output = dragonDataTable[row * 8 + col];
	if (_divBy4)
		_output /= 4;
	if (_output == 0)
		_output = 1;
	return _output;
}

class DragonDataTableGlobal : public Global {
public:
	DragonDataTableGlobal(uint16 num, DragonDataTable &table) : Global(num), _table(table) {}
	uint16 get() override { return _table.getValueFromTable(); }
	uint16 set(uint16 val) override { return _table.getValueFromTable(); }
private:
	DragonDataTable &_table;
};

////////////////////////////////

class DragonTimeGlobal : public ReadWriteGlobal<int> {
public:
	DragonTimeGlobal(uint16 num, int *val, DragonGameTime &time) : ReadWriteGlobal<int>(num, val), _time(time) {}
	uint16 set(uint16 val) override {
		if (val != ReadWriteGlobal::get()) {
			ReadWriteGlobal::set(val);
			_time.addGameTime(0);
		}
		return val;
	}

private:
	DragonGameTime &_time;
};

DragonGameTime::DragonGameTime() : _gameTimeDays(0), _gameTimeMins(0), _gameTimeHours(0), _gameMinsAdded(0) {}

Global *DragonGameTime::getDaysGlobal(uint16 num) {
	return new DragonTimeGlobal(num, &_gameTimeDays, *this);
}

Global *DragonGameTime::getHoursGlobal(uint16 num) {
	return new DragonTimeGlobal(num, &_gameTimeHours, *this);
}

Global *DragonGameTime::getMinsGlobal(uint16 num) {
	return new DragonTimeGlobal(num, &_gameTimeMins, *this);
}

void DragonGameTime::addGameTime(int mins) {
	_gameMinsAdded += mins;
	int nhours = (_gameTimeMins + mins) / 60;
	_gameTimeMins = (_gameTimeMins + mins) % 60;
	if (_gameTimeMins < 0) {
		_gameTimeMins += 0x3c;
		nhours--;
	}
	int ndays = (_gameTimeHours + nhours) / 24;
	_gameTimeHours = (_gameTimeHours + nhours) % 24;
	if (_gameTimeHours < 0) {
		_gameTimeHours += 24;
		_gameTimeDays -= 1;
	}
	_gameTimeDays += ndays;
	// TODO: if any change was made to days/hours/mins..
	//if (plusmins + nhours + ndays != 0)
	//  UINT_39e5_0ffa = 0;
}

////////////////////////////////

DragonGlobals::DragonGlobals() : Globals(),
_gameCounterTicksUp(0), _gameCounterTicksDown(0), _lastOpcode1SceneChageNum(0),
_sceneOp12SceneNum(0), _currentSelectedItem(0), _gameMinsToAdd_1(0), _gameMinsToAdd_2(0),
_gameMinsToAdd_3(0), _gameMinsToAdd_4(0), _gameMinsToAdd_5(0), _gameGlobal0x57(0), _gameDays2(0),
_sceneOpcode15Flag(0), _sceneOpcode15Val(0), _sceneOpcode100Var(0), _arcadeModeFlag_3cdc(0),
_opcode106EndMinutes(0) {
	_globals.push_back(new ROU16Global(1, &_time._gameMinsAdded));
	_globals.push_back(new ROU16Global(0x64, &_gameCounterTicksUp));
	_globals.push_back(new ROU16Global(0x62, &_lastOpcode1SceneChageNum));
	_globals.push_back(new RWU16Global(0x61, &_sceneOp12SceneNum));
	_globals.push_back(new RWU16Global(0x60, &_currentSelectedItem));
	_globals.push_back(_time.getDaysGlobal(0x5F));
	_globals.push_back(_time.getHoursGlobal(0x5E));
	_globals.push_back(_time.getMinsGlobal(0x5D));
	_globals.push_back(new RWU16Global(0x5C, &_gameMinsToAdd_1));
	_globals.push_back(new RWU16Global(0x5B, &_gameMinsToAdd_2));
	_globals.push_back(new RWU16Global(0x5A, &_gameMinsToAdd_3));
	_globals.push_back(new RWU16Global(0x59, &_gameMinsToAdd_4));
	_globals.push_back(new RWU16Global(0x58, &_gameMinsToAdd_5));
	_globals.push_back(new RWU16Global(0x57, &_gameGlobal0x57)); // TODO: Function to get/set 1f1a:4ec1
	_globals.push_back(new RWU16Global(0x56, &_gameDays2));
	_globals.push_back(new RWU16Global(0x55, &_sceneOpcode15Flag));
	_globals.push_back(new RWU16Global(0x54, &_sceneOpcode15Val));
	_globals.push_back(new RWU16Global(0x20, &_sceneOpcode100Var));
	_globals.push_back(new RWU16Global(0x21, &_arcadeModeFlag_3cdc));
	_globals.push_back(new RWU16Global(0x22, &_opcode106EndMinutes));
	_globals.push_back(new RWU16Global(0x23, &_table._row));
	_globals.push_back(new RWU16Global(0x24, &_table._col));
	_globals.push_back(new RWU16Global(0x25, &_table._divBy4));
	_globals.push_back(new DragonDataTableGlobal(0x26, _table));
	_globals.push_back(new DetailLevelROGlobal(0x27));
}


} // end namespace Dgds
