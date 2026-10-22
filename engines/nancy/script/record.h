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

#ifndef NANCY_SCRIPT_RECORD_H
#define NANCY_SCRIPT_RECORD_H

#include "common/keyboard.h"
#include "common/str.h"

#include "engines/nancy/script/mathtypes.h"

struct lua_State;

namespace Nancy {

struct NancyInput;

namespace Script {

class GraphNode;
class SceneContext;

// What the player does that records get told about
enum RecordEvent {
	kEventLeftClick		= 0,
	kEventRightClick	= 1,
	kEventLeftDown		= 2,
	kEventLeftUp		= 3,
	kEventRightDown		= 4,
	kEventRightUp		= 5,
	kEventKeyDown		= 6,
	kEventKey			= 7,
	kEventKeyUp			= 8,

	kNumRecordEvents	= 9
};

// One of the things a scene of Nancy19 and up is made of, the way action records
// were before. A scene's script makes its records by handing tables to the "AR"
// object:
//
//   door_HS = AR:NavLogic({ onScreen = Rect:New(...), scene = 2010,
//                           active = function() return Flags.Got_Key end })
//
// There are only a handful of kinds, since what used to take a record type of its
// own is now written in the scripts themselves. A record is active while its
// "active" value, or the function in its place, says so; an active record runs
// every frame until it calls itself done. Scripts read each other's "active" and
// "done" to chain records together.
//
// Besides the fields its kind knows, a record keeps whatever else a script
// stores in it. That includes the functions the engine calls at certain points
// (RunOnce, OnDone, OnReset...), which every kind of record can have.
class Record {
public:
	Record(SceneContext &context, const char *typeName);
	virtual ~Record();

	const char *getTypeName() const { return _typeName; }
	SceneContext &getContext() const { return _context; }

	// Takes over the fields of the table that made the record
	void create(int tableIndex);

	// Pushes an object for the scripts that stands for this record
	void push();

	// Nancy20: a record made with "scriptOwns" lives only as long as scripts can
	// still reach it. Once the last object standing for it has been collected, it
	// stops and is taken out of the scene.
	void onObjectCollected();
	bool isDropped() const { return _isDropped; }

	// One frame. Times are in seconds.
	void update(double sceneTime, double elapsedTime);

	// The record has done its job, and stops running
	void finish();

	// Makes a record that ran start over. Only some kinds can.
	bool reset();

	// Draws an active record right away, where its transform puts it on the
	// screen. Scripts do this for what hangs off the cursor.
	void render();

	// A record another one keeps for later stays out of the way until it is
	// released, and then runs right away
	void hold();
	void release();

	// The game is about to be saved, which is when the records that keep
	// something across scenes store it
	virtual void onSave() {}

	// The scene of the record is being taken down
	virtual void onDie() {}

	// Whether messages sent to the given name are for this record
	virtual bool hasMessagingName(const Common::String &name) { return false; }

	bool isActive() const { return _isActive && !_isDone; }
	bool isDone() const { return _isDone; }
	bool hasStarted() const { return _hasStarted; }
	// Counted from the node the record is under, when it is under one
	double getZ() const;

	// Puts the record no closer to the player than the given z
	void limitZ(double z);

	// Reads or writes one of the fields of the record on behalf of a script.
	// The key is at stack index 2, and a new value at 3.
	int getScriptField();
	int setScriptField();

	// Calls a function a script stored in the record, with the record itself in
	// front of the arguments on the stack. Returns false when there is no such
	// function, or when it failed; no results are left then.
	bool callFunction(const char *name, int numArgs = 0, int numResults = 0);

	// What the player does goes to every record that is active, as a call to
	// the function a script gave the record for it, or to what the kind of
	// record does by itself. Positions are in the coordinates of the viewport.
	// A click, a key and the cursor are up to the first record that takes them.
	void sendMouseMove(const ScriptPoint &point, bool inViewport);
	bool sendChangeCursor(const ScriptPoint &point, bool inViewport, uint &cursorType, bool &isHot);
	bool sendMouseEvent(RecordEvent event, const ScriptPoint &point, bool inViewport);
	bool sendKeyEvent(RecordEvent event, Common::KeyCode key);
	void sendMouseScroll(const ScriptPoint &point, bool inViewport, int notches);

	// Where the record puts what it shows, for the kinds that show something,
	// after the nodes it is under moved it
	ScriptMatrix getTransform() const;

	// Nancy21: puts the record under a node; null for none
	GraphNode *getParentNode() const { return _parentNode; }
	void setParentNode(GraphNode *node);

	// A node above the record changed its z or its transform
	void onParentNodeChanged();

	// Shows or hides what the record draws, once the records of a frame have run
	virtual void updateGraphics() {}

	// Enhancement: the player can skip what a scene plays. Whether the record is
	// something that can be skipped, and what it does when the skipping begins.
	virtual bool isSkippable() const { return false; }
	virtual void onSkip() {}

	// A record that ended during a skip doesn't keep it going when the scripts
	// start it over, as the random fidget movies do
	bool canBeSkipped() const { return !_hasEndedInSkip && isSkippable(); }
	void onSkipEnded() { _hasEndedInSkip = false; }

	// Enhancement: what a character says can be skipped as well. Returns
	// whether the record was saying something.
	virtual bool skipSpeech() { return false; }

	// For the debugger: what the scripts gave the record, as "name = value"
	void describeFields(Common::Array<Common::String> &lines);

protected:
	lua_State *getLua() const;

	// The fields of the kind of record, from the table that made it
	virtual void readTable(int tableIndex) {}

	// What the record does when no script took its place
	virtual void runOnce() {}
	virtual void run(double sceneTime, double elapsedTime) {}
	virtual void onActivation() {}
	virtual void onDeactivation() {}
	virtual void onZChanged() {}
	virtual void renderNow() {}

	// Whether the record can start over, after getting itself ready for it
	virtual bool prepareReset() { return false; }

	// Whether the functions of the scripts take the place of what the record does
	// by itself: Run, Render, and everything the player does
	virtual bool isScripted() const { return false; }

	virtual void onMouseMove(const ScriptPoint &point, bool inViewport) {}
	virtual bool getCursor(const ScriptPoint &point, bool inViewport, uint &cursorType, bool &isHot) { return false; }
	virtual bool onMouseEvent(RecordEvent event, const ScriptPoint &point, bool inViewport) { return false; }

	// Whether the kind of record has a "transform", and what to do when it changes
	virtual bool hasTransform() const { return false; }
	virtual void onTransformChanged() {}

	// The fields of the kind of record. Reading pushes the value and returns true;
	// writing takes the value at index 3. Both return false for unknown fields.
	virtual bool getField(const Common::String &key) { return false; }
	virtual bool setField(const Common::String &key) { return false; }

	// What scripts stored in the record
	void pushStoredValue(const char *name);
	void storeValue(const char *name);	// pops the value

private:
	void pushStorage();

	SceneContext &_context;
	const char *_typeName;

	double _z = 0.0;
	ScriptMatrix _transform;
	GraphNode *_parentNode = nullptr;
	bool _hasActiveFunction = false;
	bool _hasStarted = false;
	bool _isActive = true;
	bool _wasActive = false;
	bool _isDone = false;
	bool _isFinishing = false;	// while OnDone runs
	bool _hasEndedInSkip = false;
	bool _hasRunOnce = false;

	bool _isScriptOwned = false;
	uint _numObjects = 0;	// the objects standing for the record that Lua still has
	bool _isDropped = false;
};

// Makes the "AR" object, and the types of the records
void registerRecordAPI(lua_State *L);

// The record a script handed in, or a script error
Record *checkRecord(lua_State *L, int index);

// The record a value stands for, or null when it is something else
Record *toRecord(lua_State *L, int index);

// For the debugger: a value of the scripts as text
Common::String describeValue(lua_State *L, int index);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_RECORD_H
