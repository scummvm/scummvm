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

#ifndef NANCY_SCRIPT_SCENECONTEXT_H
#define NANCY_SCRIPT_SCENECONTEXT_H

#include "common/array.h"
#include "common/keyboard.h"
#include "common/str.h"

#include "engines/nancy/script/types.h"

struct lua_State;

namespace Nancy {

struct NancyInput;

namespace Script {

class LuaState;
class Record;
class ConvoRecord;
struct ConvoResponse;
class SummaryRecord;

// One scene of a stream in Nancy19 and up, where it takes the place of the action
// records read from a scene's IFF. A scene is a Lua script, which is run once to
// make its records, and then kept around for the functions the records call.
// Every scene has an interpreter of its own, so the scripts of two scenes share
// nothing but what the engine keeps for them.
class SceneContext {
public:
	SceneContext(const Common::String &streamName, const Common::String &sceneName, uint frame, int32 zOffset);
	~SceneContext();

	// Runs the script of the scene. Returns false when it is missing, or when it
	// doesn't begin with a summary.
	bool load();

	// One frame, which lasted the given number of seconds
	void run(double elapsedTime);
	void handleInput(NancyInput &input);

	// Tells the records that the game is about to be saved
	void onSave();

	// Hands a message to the scene when it is the one it names, and to the
	// records that go by that name otherwise, until one of them takes it.
	// Returns false when the message is for neither.
	bool receiveMessage(const Message &message, bool &wasTaken);

	lua_State *getLua() const;

	// A new record only joins the scene at the start of the next frame. Until then
	// it doesn't run, isn't drawn, and scripts that go over the records of the scene
	// don't come across it.
	void addRecord(Record *record) { _newRecords.push_back(record); }
	bool hasRecords() const { return !_records.empty() || !_newRecords.empty(); }
	const Common::Array<Record *> &getRecords() const { return _records; }

	// The record that describes the scene, which is always the first one
	SummaryRecord *getSummary() const { return _summary; }

	const Common::String &getStreamName() const { return _streamName; }
	const Common::String &getSceneName() const { return _sceneName; }
	uint getFrame() const { return _frame; }
	void setFrame(uint frame) { _frame = frame; }
	int32 getZOffset() const { return _zOffset; }

	// The seconds the scene has been running for
	double getTime() const { return _time; }

	// Enhancement: skips the movies that are playing. They are played through
	// a frame at a time, without their sounds, so that everything the scripts
	// tie to them still happens. Returns false when there is nothing to skip.
	bool skipMovies();
	bool isSkipping() const { return _isSkipping; }

	// The conversation of the scene, once one has begun
	ConvoRecord *getConvo() const { return _convo; }
	void setConvo(ConvoRecord *convo) { _convo = convo; }

	// Keeps a response the scripts made for as long as the scene lasts
	void addConvoResponse(ConvoResponse *response) { _convoResponses.push_back(response); }

	// The panels a scene adds to the UI call into its scripts, so they go when it does
	void setMakesUI() { _makesUI = true; }

	// Runs another script as a part of this scene
	bool include(const Common::String &name);

	// Changes the scene of a stream; no name stands for the one this scene is in
	void changeScene(const Common::String &streamName, const Common::String &sceneName, uint16 frame);

	// Scripts can take over parts of what a scene does by setting functions
	// on the "Scene" object. Takes the function at the top of the stack.
	void setSceneFunction(const char *name);

	// Where the mouse is in the viewport. Returns whether it is inside of it.
	static bool getViewportMouse(const NancyInput &input, ScriptPoint &point);

private:
	bool callSceneFunction(const char *name, int numArgs);
	void pushMessage(const Message &message);
	void runFrame(double elapsedTime);
	void renderAttachment();
	bool hasSkippableRecord() const;
	void pushSceneFunctions();
	void sendMouseEvent(int event, const ScriptPoint &point, bool inViewport);
	void sendKeyEvent(int event, Common::KeyCode key);

	Common::String _streamName;
	Common::String _sceneName;
	bool _isEnding = false;
	uint _frame;
	int32 _zOffset;

	LuaState *_lua;
	Common::Array<Record *> _records;
	Common::Array<Record *> _newRecords;
	Common::Array<Record *> _inputOrder;	// the records, in the order the player's input goes to them
	SummaryRecord *_summary = nullptr;
	ConvoRecord *_convo = nullptr;
	Common::Array<ConvoResponse *> _convoResponses;

	double _time = 0.0;
	bool _hasRunOnce = false;
	bool _makesUI = false;
	bool _isSkipping = false;
	bool _hasChangedScene = false;

	// The keys records were told are down
	uint16 _heldMoveKeys = 0;
	Common::Array<Common::KeyCode> _pressedKeys;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_SCENECONTEXT_H
