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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_DATARECORDS_H
#define NANCY_ACTION_DATARECORDS_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

class NancyEngine;

namespace Action {

// Changes the selected value inside the TableData. Value can be incremented, decremented, or not changed.
// Also responsible for checking whether all values are correct (as described in the TABL chunk). nancy6 only.
class TableIndexSetValueHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return (CursorManager::CursorType)_cursorType; }

protected:
	Common::String getRecordTypeName() const override { return "TableIndexSetValueHS"; }

	uint16 _tableIndex = 0;
	byte _valueChangeType = kNoChangeTableValue;
	int16 _entryCorrectFlagID = -1;
	int16 _allEntriesCorrectFlagID = -1;

	MultiEventFlagDescription _flags;
	uint16 _cursorType = 1;
	Common::Array<HotspotDescription> _hotspots;
};

// Sets (or adds to) a value inside the TableData struct
class SetValue : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "SetValue"; }

	byte _index = 0;
	bool _shouldSet = false;
	int16 _value = kNoTableValue;
};

class SetValueCombo : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "SetValueCombo"; }

	byte _valueIndex = 0;
	Common::Array<byte> _indices;
	Common::Array<int16> _percentages;
};

class ValueTest : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "ValueTest"; }

	byte _valueIndex = 0;
	byte _testType = 0;
	byte _condition = 0;
	Common::Array<byte> _indicesToTest;

	int16 _flagToSet = kFlagNoLabel;
};

// Sets up to 10 flags at once.
class EventFlags : public ActionRecord {
public:
	EventFlags(bool terse = false) : _isTerse(terse) {}
	virtual ~EventFlags() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	MultiEventFlagDescription _flags;
	bool _isTerse;

protected:
	Common::String getRecordTypeName() const override { return _isTerse ? "EventFlagsTerse" : "EventFlags"; }
};

// Sets up to 10 flags when clicked. Hotspot can move alongside background frame.
class EventFlagsMultiHS : public EventFlags {
public:
	EventFlagsMultiHS(bool isCursor, bool terse = false) : EventFlags(terse), _isCursor(isCursor) {}
	virtual ~EventFlagsMultiHS() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return _hoverCursor; }

	CursorManager::CursorType _hoverCursor = CursorManager::kHotspot;
	Common::Array<HotspotDescription> _hotspots;

	bool _isCursor;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return _isCursor ? (_isTerse ? "EventFlagsHSTerse" : "EventFlagsCursorHS") : "EventFlagsMultiHS"; }
};

// Sets the difficulty level for the current save. Only appears at the start of the game.
// First appears in nancy1. Nancy1 and nancy2 have three difficulty values, while later games
// only have two: 0 and 2.
class DifficultyLevel : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _difficulty = 0;
	FlagDescription _flag;

protected:
	Common::String getRecordTypeName() const override { return "DifficultyLevel"; }
};

class ModifyListEntry : public ActionRecord {
public:
	enum Type { kAdd, kDelete, kMark };

	ModifyListEntry(Type type) : _type(type) {}
	virtual ~ModifyListEntry() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Type _type;

	uint16 _surfaceID = 0;
	Common::String _stringID;
	uint16 _mark = 0;
	uint16 _sceneID = kNoScene;

protected:
	Common::String getRecordTypeName() const override;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DATARECORDS_H
