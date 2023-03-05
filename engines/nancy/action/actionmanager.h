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

#ifndef NANCY_ACTION_ACTIONMANAGER_H
#define NANCY_ACTION_ACTIONMANAGER_H

#include "common/array.h"

namespace Common {
class Serializer;
class SeekableReadStream;
}

namespace Nancy {

class NancyEngine;
class NancyConsole;
struct NancyInput;

namespace State {
class Scene;
}

namespace Action {

class ActionRecord;

// The class that handles ActionRecords and their execution
class ActionManager {
	friend class Nancy::State::Scene;
	friend class Nancy::NancyConsole;

public:
	static const byte kCursInvHolding			= 0;
	static const byte kCursInvNotHolding		= 1;
	static const byte kCursInvNotHoldingOffset 	= 100;

	ActionManager() {}
	virtual ~ActionManager() {}

	void handleInput(NancyInput &input);

	void processActionRecords();
	bool addNewActionRecord(Common::SeekableReadStream &inputData);
	Common::Array<ActionRecord *> &getActionRecords() { return _records; }
	ActionRecord *getActionRecord(uint id) { if (id < _records.size()) return _records[id]; else return nullptr;}
	void clearActionRecords();

	void onPause(bool pause);

	void synchronize(Common::Serializer &serializer);

protected:
	virtual ActionRecord *createActionRecord(uint16 type);

	Common::Array<ActionRecord *> _records;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ACTIONMANAGER_H
