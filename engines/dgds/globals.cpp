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
#include "dgds/scene.h"

namespace Dgds {


typedef ReadOnlyGlobal<int16> ROI16Global;
typedef ReadWriteGlobal<int16> RWI16Global;

////////////////////////////////

// TODO: Move this to Scene??
class GameIsInteractiveGlobal : public Global {
public:
	GameIsInteractiveGlobal(uint16 num, int16 *ptr) : Global(num), _ptr(ptr), _isSetOff(false) {}

	int16 get() override {
		SDSScene *scene = static_cast<DgdsEngine *>(g_engine)->getScene();
		bool nonInteractive = _isSetOff || scene->getDragItem() || scene->hasVisibleOrOpeningDialog();
		*_ptr = !nonInteractive;
		return *_ptr;
	}

	int16 set(int16 val) override {
		_isSetOff = (val == 0);
		return get();
	}

	void setRaw(int16 val) override { }

private:
	int16 *_ptr;
	bool _isSetOff;
};


Globals::Globals(Clock &clock) :
_lastOpcode1SceneChageNum(0), _sceneOp12SceneNum(0), _currentSelectedItem(0),
_gameMinsToAddOnLClick(0), _gameMinsToAddOnStartDrag(0), _gameMinsToAddOnRClick(0), _gameMinsToAddOnDragFinished(0),
_gameMinsToAddOnObjInteraction(0), _gameIsInteractiveGlobal(0), _sceneOpcode15FromScene(0),
_sceneOpcode15ToScene(0) {
	_globals.push_back(clock.getGameMinsAddedGlobal(1));
	_globals.push_back(clock.getGameTicksUpGlobal(0x64));
	_globals.push_back(clock.getGameTicksDownGlobal(0x63));
	_globals.push_back(new ROI16Global(0x62, &_lastOpcode1SceneChageNum));
	_globals.push_back(new RWI16Global(0x61, &_sceneOp12SceneNum));
	_globals.push_back(new RWI16Global(0x60, &_currentSelectedItem));
	_globals.push_back(clock.getDaysGlobal(0x5F));
	_globals.push_back(clock.getHoursGlobal(0x5E));
	_globals.push_back(clock.getMinsGlobal(0x5D));
	_globals.push_back(new RWI16Global(0x5C, &_gameMinsToAddOnLClick));
	_globals.push_back(new RWI16Global(0x5B, &_gameMinsToAddOnStartDrag));
	_globals.push_back(new RWI16Global(0x5A, &_gameMinsToAddOnRClick));
	_globals.push_back(new RWI16Global(0x59, &_gameMinsToAddOnDragFinished));
	_globals.push_back(new RWI16Global(0x58, &_gameMinsToAddOnObjInteraction));
	_globals.push_back(new GameIsInteractiveGlobal(0x57, &_gameIsInteractiveGlobal));
	_globals.push_back(clock.getDays2Global(0x56));
	_globals.push_back(new RWI16Global(0x55, &_sceneOpcode15FromScene));
	_globals.push_back(new RWI16Global(0x54, &_sceneOpcode15ToScene));
}

Globals::~Globals() {
	for (auto &g : _globals) {
		delete g;
	}
}

int16 Globals::getGlobal(uint16 num) {
	for (auto &global : _globals) {
		if (global->_num == num) {
			return global->get();
		}
	}
	return 0;
}

int16 Globals::setGlobal(uint16 num, int16 val) {
	for (auto &global : _globals) {
		if (global->_num == num) {
			return global->set(val);
		}
	}
	return 0;
}

Common::Error Globals::syncState(Common::Serializer &s) {
	s.syncAsSint16LE(_lastOpcode1SceneChageNum);
	s.syncAsSint16LE(_sceneOp12SceneNum);
	s.syncAsSint16LE(_currentSelectedItem);
	s.syncAsSint16LE(_gameMinsToAddOnLClick);
	s.syncAsSint16LE(_gameMinsToAddOnStartDrag);
	s.syncAsSint16LE(_gameMinsToAddOnRClick);
	s.syncAsSint16LE(_gameMinsToAddOnDragFinished);
	s.syncAsSint16LE(_gameMinsToAddOnObjInteraction);
	s.syncAsSint16LE(_gameIsInteractiveGlobal);
	s.syncAsSint16LE(_sceneOpcode15FromScene);
	s.syncAsSint16LE(_sceneOpcode15ToScene);
	return Common::kNoError;
}

////////////////////////////////

class DetailLevelROGlobal : public Global {
public:
	DetailLevelROGlobal(uint16 num) : Global(num) {}
	int16 get() override { return static_cast<DgdsEngine *>(g_engine)->getDetailLevel(); }
	int16 set(int16 val) override { return static_cast<DgdsEngine *>(g_engine)->getDetailLevel(); }
	void setRaw(int16 val) override { }
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
	int16 get() override { return _table.getValueFromTable(); }
	int16 set(int16 val) override { return _table.getValueFromTable(); }
	void setRaw(int16 val) override { }
private:
	DragonDataTable &_table;
};


////////////////////////////////

DragonGlobals::DragonGlobals(Clock &clock) : Globals(clock),
 _sceneOpcode100Var(0), _arcadeModeFlag_3cdc(0), _opcode106EndMinutes(0) {
	_globals.push_back(new RWI16Global(0x20, &_sceneOpcode100Var));
	_globals.push_back(new RWI16Global(0x21, &_arcadeModeFlag_3cdc));
	_globals.push_back(new RWI16Global(0x22, &_opcode106EndMinutes));
	_globals.push_back(new RWI16Global(0x23, &_table._row));
	_globals.push_back(new RWI16Global(0x24, &_table._col));
	_globals.push_back(new RWI16Global(0x25, &_table._divBy4));
	_globals.push_back(new DragonDataTableGlobal(0x26, _table));
	_globals.push_back(new DetailLevelROGlobal(0x27));
}

Common::Error DragonGlobals::syncState(Common::Serializer &s) {
	Globals::syncState(s);
	s.syncAsSint16LE(_sceneOpcode100Var);
	s.syncAsSint16LE(_arcadeModeFlag_3cdc);
	s.syncAsSint16LE(_opcode106EndMinutes);

	s.syncAsSint16LE(_table._row);
	s.syncAsSint16LE(_table._col);
	s.syncAsSint16LE(_table._divBy4);
	s.syncAsSint16LE(_table._output);

	return Common::kNoError;
}

} // end namespace Dgds
