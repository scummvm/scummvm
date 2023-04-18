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

#ifndef NANCY_ACTION_ACTIONRECORD_H
#define NANCY_ACTION_ACTIONRECORD_H

#include "engines/nancy/time.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/renderobject.h"

namespace Common {
class SeekableReadStream;
}

namespace Nancy {

class NancyEngine;
class NancyConsole;
struct NancyInput;

namespace Action {

enum struct DependencyType : byte {
	kNone							= 0,
	kInventory						= 1,
	kEvent							= 2,
	kLogic							= 3,
	kElapsedGameTime				= 4,
	kElapsedSceneTime				= 5,
	kElapsedPlayerTime				= 6,
	kSamsSight						= 7,	// Not implemented
	kSamsSound						= 8,	// Not implemented
	kSceneCount						= 9,
	kElapsedPlayerDay				= 10,
	kCursorType						= 11,
	kPlayerTOD						= 12,
	kTimerLessThanDependencyTime	= 13,
	kTimerGreaterThanDependencyTime	= 14,
	kDifficultyLevel				= 15,
	kClosedCaptioning				= 16,
	kSound							= 17,	// Not implemented
	kOpenParentheses				= 18,	// Not implemented
	kCloseParentheses				= 19	// Not implemented
};

// Describes a condition that needs to be fulfilled before the
// action record can be executed
struct DependencyRecord {
	DependencyType type;	// 0x00
	byte label;				// 0x01
	byte condition;			// 0x02
	bool orFlag;			// 0x03
	int16 hours;			// 0x04
	int16 minutes;			// 0x06
	int16 seconds;			// 0x08
	int16 milliseconds;		// 0x0A

	bool satisfied;
	Time timeData;
};

// Describes a single action that will be performed on every update.
// Supports conditional execution (via dependencies) and can have
// clickable hotspots on screen.
// Does _not_ support drawing to screen, records that need this functionality
// will have to subclass RenderActionRecord.
class ActionRecord {
	friend class ActionManager;
	friend class Nancy::NancyConsole;

public:
	enum ExecutionState { kBegin, kRun, kActionTrigger };
	enum ExecutionType { kOneShot = 1, kRepeating = 2 };
	ActionRecord() :
		_type(0),
		_execType(kOneShot),
		_isActive(false),
		_isDone(false),
		_hasHotspot(false),
		_state(ExecutionState::kBegin),
		_days(-1),
		_itemRequired(-1) {}
	virtual ~ActionRecord() {}

	virtual void readData(Common::SeekableReadStream &stream) = 0;
	virtual void execute() {}
	virtual void onPause(bool pause) {}

	virtual CursorManager::CursorType getHoverCursor() const { return CursorManager::kHotspot; }
	virtual void handleInput(NancyInput &input) {}

protected:
	void finishExecution();

	// Used for debugging
	virtual Common::String getRecordTypeName() const = 0;

public:
	Common::String _description;					// 0x00
	byte _type;										// 0x30
	ExecutionType _execType;						// 0x31
	// 0x32 data
	Common::Array<DependencyRecord> _dependencies;	// 0x36
	// 0x3A numDependencies
	bool _isActive;									// 0x3B
	// 0x3C satisfiedDependencies[]
	// 0x48 timers[]
	// 0x78 orFlags[]
	bool _isDone;									// 0x84
	bool _hasHotspot;								// 0x85
	Common::Rect _hotspot;							// 0x89
	ExecutionState _state;							// 0x91
	int16 _days;									// 0x95
	int8 _itemRequired;								// 0x97
};

// Base class for visual ActionRecords
class RenderActionRecord : public ActionRecord, public RenderObject {
public:
	RenderActionRecord(uint zOrder) : RenderObject(zOrder) {}
	virtual ~RenderActionRecord() {}

	// This makes sure the AR is re-added to the render system
	// when returning from a different state (e.g. the Help screen)
	void onPause(bool pause) override { if (!pause) registerGraphics(); }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ACTIONRECORD_H
