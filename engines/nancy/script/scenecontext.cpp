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

#include "common/textconsole.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/script/convo.h"
#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/ref.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/viewport.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace Script {

SceneContext::SceneContext(const Common::String &streamName, const Common::String &sceneName, uint frame, int32 zOffset) :
		_streamName(streamName),
		_sceneName(sceneName),
		_frame(frame),
		_zOffset(zOffset) {
	_lua = new LuaState(this);
}

SceneContext::~SceneContext() {
	// Taking the records down can have the UI send messages
	_isEnding = true;

	UI::NDUI::Manager *ui = _makesUI ? NancySceneState.getNDUI() : nullptr;
	if (ui) {
		ui->clearPanels();
	}

	for (Record *record : _records) {
		delete record;
	}

	for (Record *record : _newRecords) {
		delete record;
	}

	for (ConvoResponse *response : _convoResponses) {
		delete response;
	}

	for (GraphNode *node : _graphNodes) {
		delete node;
	}

	for (auto &ref : _refs) {
		delete ref._value;
	}

	delete _lua;
}

const RefImage *SceneContext::loadRef(const Common::String &name) {
	RefImage *ref = _refs.getValOrDefault(name, nullptr);
	if (!ref) {
		ref = new RefImage();
		if (!g_nancy->_resource->loadImage(Common::Path(name), ref->image)) {
			warning("Could not load Ref %s", name.c_str());
		}

		_refs.setVal(name, ref);
	}

	return ref;
}

lua_State *SceneContext::getLua() const {
	return _lua->get();
}

bool SceneContext::load() {
	// Nancy20 runs a few shared scripts in every scene before its own
	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	if (boot) {
		for (const Common::String &name : boot->includeFileNames) {
			if (!_lua->runScript(name)) {
				return false;
			}
		}
	}

	if (!_lua->runScript(_sceneName)) {
		return false;
	}

	// Nothing can be made before there is a summary. A scene can go on to make
	// more of them, and so can the scripts it includes; the last one that names
	// a background is the one that shows.
	_summary = _newRecords.empty() ? nullptr : dynamic_cast<SummaryRecord *>(_newRecords[0]);
	for (Record *record : _newRecords) {
		SummaryRecord *summary = dynamic_cast<SummaryRecord *>(record);
		if (_summary && summary && !summary->getBackgroundName().empty()) {
			_summary = summary;
		}
	}

	if (!_summary) {
		warning("Scene %s: No summary AR specified", _sceneName.c_str());
		return false;
	}

	if (_makesUI) {
		NancySceneState.onScriptedUILoaded();
	}

	return true;
}

Common::String SceneContext::findGlobalName(const Record *record) const {
	lua_State *L = getLua();
	const int top = lua_gettop(L);
	Common::String name;

	lua_pushnil(L);
	while (name.empty() && lua_next(L, LUA_GLOBALSINDEX)) {
		if (lua_type(L, -2) == LUA_TSTRING) {
			if (toRecord(L, -1) == record) {
				name = lua_tostring(L, -2);
			} else if (lua_istable(L, -1)) {
				lua_pushnil(L);
				while (name.empty() && lua_next(L, -2)) {
					if (toRecord(L, -1) == record) {
						const Common::String key = lua_type(L, -2) == LUA_TSTRING ?
							Common::String::format("\"%s\"", lua_tostring(L, -2)) :
							Common::String::format("%g", lua_tonumber(L, -2));
						name = Common::String::format("%s[%s]", lua_tostring(L, -4), key.c_str());
					}

					lua_pop(L, 1);
				}
			}
		}

		lua_pop(L, 1);
	}

	lua_settop(L, top);
	return name;
}

bool SceneContext::include(const Common::String &name) {
	return _lua->runScript(name);
}

// The functions set on the "Scene" object are kept in a table of the registry
void SceneContext::pushSceneFunctions() {
	lua_State *L = getLua();

	lua_pushlightuserdata(L, this);
	lua_rawget(L, LUA_REGISTRYINDEX);
	if (lua_istable(L, -1)) {
		return;
	}

	lua_pop(L, 1);
	lua_newtable(L);
	lua_pushlightuserdata(L, this);
	lua_pushvalue(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

void SceneContext::setSceneFunction(const char *name) {
	lua_State *L = getLua();

	pushSceneFunctions();
	lua_insert(L, -2);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

bool SceneContext::callSceneFunction(const char *name, int numArgs) {
	lua_State *L = getLua();

	pushSceneFunctions();
	lua_getfield(L, -1, name);
	lua_remove(L, -2);

	if (!lua_isfunction(L, -1)) {
		lua_pop(L, numArgs + 1);
		return false;
	}

	if (numArgs > 0) {
		lua_insert(L, -1 - numArgs);
	}

	const Common::String what = Common::String::format("Error during Scene %s", name);
	LuaState::call(L, numArgs, 0, what.c_str());
	return true;
}

// The frames a skipped movie moves on by every time the scene runs, and how
// long each of them counts for
static const uint kSkipFramesPerRun = 30;
static const double kSkipFrameSeconds = 1.0 / 15.0;

bool SceneContext::hasSkippableRecord() const {
	for (const Record *record : _records) {
		if (record->canBeSkipped()) {
			return true;
		}
	}

	return false;
}

bool SceneContext::skipMovies() {
	if (_isSkipping || !hasSkippableRecord()) {
		return false;
	}

	_isSkipping = true;
	for (Record *record : _records) {
		record->onSkip();
	}

	return true;
}

bool SceneContext::skipSpeech() {
	bool hasSkipped = false;
	for (Record *record : _records) {
		hasSkipped |= record->skipSpeech();
	}

	return hasSkipped;
}

void SceneContext::end() {
	for (Record *record : _records) {
		record->onDie();
	}
}

// A scene that has a function for it runs that in place of its records
void SceneContext::onSave() {
	if (callSceneFunction("OnSave", 0)) {
		return;
	}

	// A record can make others, or take some down, while it stores its state
	const Common::Array<Record *> records = _records;
	for (Record *record : records) {
		record->onSave();
	}
}

void SceneContext::pushMessage(const Message &message) {
	lua_State *L = getLua();

	lua_newtable(L);
	lua_pushstring(L, message.action.c_str());
	lua_setfield(L, -2, "action");
	lua_pushstring(L, message.target.c_str());
	lua_setfield(L, -2, "victim");
	lua_pushstring(L, message.sender.c_str());
	lua_setfield(L, -2, "sender");
	lua_pushstring(L, message.text.c_str());
	lua_setfield(L, -2, "text");
	lua_pushnumber(L, message.value);
	lua_setfield(L, -2, "value");
}

// TODO: The "cause" field of a message
bool SceneContext::receiveMessage(const Message &message, bool &wasTaken) {
	lua_State *L = getLua();
	wasTaken = false;

	if (_isEnding) {
		return false;
	}

	if (message.target.equalsIgnoreCase(_sceneName)) {
		pushSceneFunctions();
		lua_getfield(L, -1, "ReceiveMessage");
		lua_remove(L, -2);

		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 1);
			return true;
		}

		pushMessage(message);
		if (LuaState::call(L, 1, 1, "Error during Scene ReceiveMessage")) {
			wasTaken = lua_toboolean(L, -1) != 0;
			lua_pop(L, 1);
		}

		return true;
	}

	bool hasReceiver = false;
	const Common::Array<Record *> records = _records;
	for (Record *record : records) {
		if (!record->hasMessagingName(message.target)) {
			continue;
		}

		hasReceiver = true;
		pushMessage(message);
		if (record->callFunction("ReceiveMessage", 1, 1)) {
			wasTaken = lua_toboolean(L, -1) != 0;
			lua_pop(L, 1);
		}

		if (wasTaken) {
			break;
		}
	}

	return hasReceiver;
}

void SceneContext::run(double elapsedTime) {
	runFrame(elapsedTime);

	// The skipping is over with the movies, or with the scene
	for (uint i = 0; _isSkipping && i < kSkipFramesPerRun; ++i) {
		if (_hasChangedScene || !hasSkippableRecord()) {
			_isSkipping = false;
			for (Record *record : _records) {
				record->onSkipEnded();
			}
		} else {
			runFrame(kSkipFrameSeconds);
		}
	}

	renderAttachment();

	for (Record *record : _records) {
		record->updateGraphics();
	}
}

// What hangs off the cursor is drawn by a function of the scene, which gets
// handed where the cursor is on the screen
void SceneContext::renderAttachment() {
	const VIEW *viewport = GetEngineData(VIEW);
	assert(viewport);

	ScriptPoint mouse;
	getViewportMouse(g_nancy->_input->getInput(), mouse);

	ScriptMatrix transform;
	transform.m[3][0] = mouse.x + viewport->screenPosition.left;
	transform.m[3][1] = mouse.y + viewport->screenPosition.top;

	pushMatrix(getLua(), transform);
	callSceneFunction("RenderAttachment", 1);
}

void SceneContext::runFrame(double elapsedTime) {
	lua_State *L = getLua();

	_time += elapsedTime;
	_elapsedTime = elapsedTime;

	// The records made since the last frame join in
	_records.push_back(_newRecords);
	_newRecords.clear();

	// Nancy20 collects what the scripts let go of once a frame, and only then.
	// That takes out the records that belonged to the scripts.
	if (g_nancy->getGameType() >= kGameTypeNancy20) {
		lua_gc(L, LUA_GCRESTART, 0);
		lua_gc(L, LUA_GCCOLLECT, 0);
		lua_gc(L, LUA_GCSTOP, 0);
		deleteDroppedRecords();
	}

	// The records run in the order they were made, but what the player does
	// goes to the ones closest to them first, and among the ones just as close
	// to the one made last. A z can change at any time.
	_inputOrder.clear();
	for (int i = (int)_records.size() - 1; i >= 0; --i) {
		_inputOrder.push_back(_records[i]);
	}

	for (uint i = 1; i < _inputOrder.size(); ++i) {
		Record *record = _inputOrder[i];
		uint j = i;
		while (j > 0 && _inputOrder[j - 1]->getZ() < record->getZ()) {
			_inputOrder[j] = _inputOrder[j - 1];
			--j;
		}

		_inputOrder[j] = record;
	}

	if (!_hasRunOnce) {
		_hasRunOnce = true;
		callSceneFunction("RunOnce", 0);
	}

	// A scene that has a function for it runs that in place of its records
	lua_pushnumber(L, _time);
	lua_pushnumber(L, elapsedTime);
	if (!callSceneFunction("Run", 2)) {
		for (Record *record : _records) {
			record->update(_time, elapsedTime);
		}
	}
}

void SceneContext::deleteDroppedRecords() {
	for (uint i = 0; i < _records.size(); ) {
		Record *record = _records[i];
		if (!record->isDropped()) {
			++i;
			continue;
		}

		if (record == _summary) {
			_summary = nullptr;
		}

		if (record == _convo) {
			_convo = nullptr;
		}

		_records.remove_at(i);
		delete record;
	}
}

const ViewTransform *SceneContext::getViewTransform() const {
	return NancySceneState.getStreams().getViewTransform(_streamName);
}

GraphNode *SceneContext::createGraphNode() {
	GraphNode *node = new GraphNode(*this);
	_graphNodes.push_back(node);
	return node;
}

void SceneContext::onGraphNodeChanged(GraphNode *node) {
	for (Record *record : _records) {
		if (record->getParentNode() && record->getParentNode()->isUnder(node)) {
			record->onParentNodeChanged();
		}
	}

	for (Record *record : _newRecords) {
		if (record->getParentNode() && record->getParentNode()->isUnder(node)) {
			record->onParentNodeChanged();
		}
	}
}

bool SceneContext::getViewportMouse(const NancyInput &input, ScriptPoint &point) {
	const UI::Viewport &viewport = NancySceneState.getViewport();
	const Common::Rect screenPosition = viewport.getScreenPosition();

	const Common::Rect local = viewport.convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	point.x = local.left;
	point.y = local.top;

	return screenPosition.contains(input.mousePos);
}

// Everything the player does goes to the records in their order for input,
// which leaves the turning around in the background for when nothing else
// takes a click.
// Only clicks and keys can be taken; the rest goes to every record.
void SceneContext::sendMouseEvent(int event, const ScriptPoint &point, bool inViewport) {
	const bool canBeTaken = event == kEventLeftClick || event == kEventRightClick;

	for (Record *record : _inputOrder) {
		if (record->sendMouseEvent((RecordEvent)event, point, inViewport) && canBeTaken) {
			return;
		}
	}
}

void SceneContext::sendKeyEvent(int event, Common::KeyCode key) {
	for (Record *record : _inputOrder) {
		if (record->sendKeyEvent((RecordEvent)event, key) && event == kEventKey) {
			return;
		}
	}
}

struct MoveKey {
	uint16 input;
	Common::KeyCode key;
};

// The arrow keys are bound to moving around, and arrive as that
static const MoveKey kMoveKeys[] = {
	{ NancyInput::kMoveLeft, Common::KEYCODE_LEFT },
	{ NancyInput::kMoveRight, Common::KEYCODE_RIGHT },
	{ NancyInput::kMoveUp, Common::KEYCODE_UP },
	{ NancyInput::kMoveDown, Common::KEYCODE_DOWN }
};

void SceneContext::handleInput(NancyInput &input) {
	ScriptPoint point;
	const bool inViewport = getViewportMouse(input, point);

	for (Record *record : _inputOrder) {
		record->sendMouseMove(point, inViewport);
	}

	// The UI that took the mouse already picked the cursor
	if (input.mousePos.x >= 0) {
		for (Record *record : _inputOrder) {
			uint cursorType = 0;
			bool isHot = false;
			if (record->sendChangeCursor(point, inViewport, cursorType, isHot)) {
				g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true, isHot);
				break;
			}
		}
	}

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		sendMouseEvent(kEventLeftDown, point, inViewport);
	}

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		sendMouseEvent(kEventLeftUp, point, inViewport);
		sendMouseEvent(kEventLeftClick, point, inViewport);
	}

	if (input.input & NancyInput::kRightMouseButtonDown) {
		sendMouseEvent(kEventRightDown, point, inViewport);
	}

	if (input.input & NancyInput::kRightMouseButtonUp) {
		sendMouseEvent(kEventRightUp, point, inViewport);
		sendMouseEvent(kEventRightClick, point, inViewport);
	}

	// Every record tracks mouse wheel events, one notch at a time; up is 1
	if (input.input & NancyInput::kMouseWheel) {
		const int notches = (input.input & NancyInput::kMouseWheelUp) ? 1 : -1;
		for (Record *record : _inputOrder) {
			record->sendMouseScroll(point, inViewport, notches);
		}
	}

	// The keys that went down last frame have no release of their own
	for (Common::KeyCode key : _pressedKeys) {
		sendKeyEvent(kEventKeyUp, key);
	}

	_pressedKeys.clear();

	for (const MoveKey &moveKey : kMoveKeys) {
		const bool isHeld = (input.input & moveKey.input) != 0;
		const bool wasHeld = (_heldMoveKeys & moveKey.input) != 0;

		if (isHeld && !wasHeld) {
			sendKeyEvent(kEventKeyDown, moveKey.key);
		} else if (!isHeld && wasHeld) {
			sendKeyEvent(kEventKeyUp, moveKey.key);
		}
	}

	_heldMoveKeys = input.input;

	for (const Common::KeyState &key : input.otherKbdInput) {
		sendKeyEvent(kEventKeyDown, key.keycode);
		sendKeyEvent(kEventKey, key.keycode);
		_pressedKeys.push_back(key.keycode);
	}
}

void SceneContext::changeScene(const Common::String &streamName, const Common::String &sceneName, uint16 frame) {
	State::SceneStreamManager &streams = NancySceneState.getStreams();

	_hasChangedScene = true;

	// The scenes of the main stream mostly go by number
	uint16 sceneID;
	if (parseSceneName(sceneName, sceneID)) {
		SceneChangeDescription scene;
		scene.sceneID = sceneID;
		scene.frameID = frame;
		scene.continueSceneSound = kContinueSceneSound;
		streams.changeScene(streamName, scene);
	} else {
		streams.changeScene(streamName, sceneName, frame);
	}
}

} // End of namespace Script
} // End of namespace Nancy
