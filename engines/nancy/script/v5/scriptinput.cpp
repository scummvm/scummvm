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

#include "common/algorithm.h"
#include "common/system.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/script/v5/scriptinput.h"
#include "engines/nancy/script/v5/widgets.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Script {

static const uint32 kDoubleClickTime = 500;	// milliseconds
static const int kDoubleClickDistance = 4;

struct InputState {
	Common::Point32 mouse;
	bool hasMouse = false;
	Record *hovered = nullptr;
	Record *pressed[2] = { nullptr, nullptr };

	uint32 lastClickTime[2] = { 0, 0 };
	Common::Point32 lastClickPoint[2];
	bool isDoubleClick[2] = { false, false };

	uint16 heldMoveKeys = 0;
	bool isEscHeld = false;
	Common::Array<Common::KeyCode> keysToRelease;
};

static InputState &getState() {
	static InputState s_state;
	return s_state;
}

const Common::Point32 &getScriptMousePoint() {
	return getState().mouse;
}

bool isDoubleClick(int button) {
	return getState().isDoubleClick[button];
}

void forgetInputRecord(Record *record) {
	InputState &state = getState();
	if (state.hovered == record) {
		state.hovered = nullptr;
	}

	for (int i = 0; i < 2; ++i) {
		if (state.pressed[i] == record) {
			state.pressed[i] = nullptr;
		}
	}
}

static bool isFrontmost(const Record *a, const Record *b) {
	return a->getZ() > b->getZ();
}

// The enabled records of every stream that gets input, the frontmost first
static void collectRecords(Common::Array<Record *> &records) {
	records.clear();

	Common::Array<State::SceneStream *> streams;
	NancySceneState.getStreams().getDrawOrder(streams);

	// A stream that keeps the input does so from the streams below it in its
	// own tree; the UI's tree is in front of the viewport's
	bool isTreeCaptured[2] = { false, false };

	for (int i = (int)streams.size() - 1; i >= 0; --i) {
		const Script::StreamNodeState &state = streams[i] ? streams[i]->getNodeState() : *NancySceneState.getStreams().getNodeState(State::SceneStreamManager::getMainStreamName());
		bool &isCaptured = isTreeCaptured[state.isInViewport ? 1 : 0];
		if (isCaptured) {
			continue;
		}

		const SceneContext *context = streams[i] ? streams[i]->getScriptScene() : NancySceneState.getScriptScene();
		if (context) {
			// Of two records at the same z, the one made later is in front
			Common::Array<Record *> streamRecords;
			const Common::Array<Record *> &contextRecords = context->getRecords();
			for (int j = (int)contextRecords.size() - 1; j >= 0; --j) {
				if (contextRecords[j]->isNodeEnabled()) {
					streamRecords.push_back(contextRecords[j]);
				}
			}

			Common::sort(streamRecords.begin(), streamRecords.end(), isFrontmost);
			for (Record *record : streamRecords) {
				records.push_back(record);
			}
		}

		if (streams[i] && streams[i]->getFlags().capturesInput) {
			isCaptured = true;
		}
	}
}

// Calls a function an Override has for the input, and returns whether it took it
static bool callInputFunction(Record *record, const char *name, int numArgs = 0) {
	lua_State *L = record->getContext().getLua();
	if (!record->callFunction(name, numArgs, 1)) {
		return false;
	}

	const bool isTaken = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);
	return isTaken;
}

static bool isOverride(Record *record) {
	return record->getType() == kRecordOverride;
}

static void updateHover(const Common::Array<Record *> &records) {
	InputState &state = getState();

	Record *hovered = nullptr;
	Record *pressed = state.pressed[0] ? state.pressed[0] : state.pressed[1];
	if (pressed) {
		// A hotspot that is held down is the only one the mouse can be over
		if (state.hasMouse && pressed->hitTest(state.mouse)) {
			hovered = pressed;
		}
	} else if (state.hasMouse) {
		for (Record *record : records) {
			if (record->isMouseRegion() && record->hitTest(state.mouse)) {
				hovered = record;
				break;
			}
		}
	}

	if (hovered == state.hovered) {
		return;
	}

	Record *left = state.hovered;
	state.hovered = hovered;
	if (left) {
		left->setMouseOver(false);
	}

	if (hovered) {
		hovered->setMouseOver(true);
	}
}

static void updateCursor(const Common::Array<Record *> &records) {
	InputState &state = getState();
	CursorManager &cursor = *g_nancy->_cursor;

	// A hotspot that is held down picks the cursor wherever the mouse is
	Record *pressed = state.pressed[0] ? state.pressed[0] : state.pressed[1];
	uint pressedType = 0;
	bool isPressedHot = false;
	if (pressed && pressed->getRegionCursor(pressedType, isPressedHot)) {
		cursor.setCursorType((CursorManager::CursorType)pressedType, true, isPressedHot);
		return;
	}

	for (Record *record : records) {
		uint cursorType = 0;
		bool isHot = false;

		if (record == state.hovered) {
			if (record->getRegionCursor(cursorType, isHot)) {
				cursor.setCursorType((CursorManager::CursorType)cursorType, true, isHot);
				return;
			}

			break;
		}

		if (isOverride(record)) {
			lua_State *L = record->getContext().getLua();
			if (!record->callFunction("ChangeCursor", 0, 1)) {
				continue;
			}

			bool hasCursor = false;
			if (lua_type(L, -1) == LUA_TSTRING) {
				const int type = cursor.getCursorTypeByName(lua_tostring(L, -1));
				hasCursor = type >= 0;
				cursorType = MAX(type, 0);
			} else {
				hasCursor = testCursor(L, -1, cursorType, isHot);
			}

			lua_pop(L, 1);
			if (hasCursor) {
				cursor.setCursorType((CursorManager::CursorType)cursorType, true, isHot);
				return;
			}

			continue;
		}

		SummaryRecord *summary = toSummaryRecord(record);
		if (summary && state.hasMouse && summary->sendChangeCursor(state.mouse, true, cursorType, isHot)) {
			cursor.setCursorType((CursorManager::CursorType)cursorType, true, isHot);
			return;
		}
	}

	cursor.setCursorType(CursorManager::kNormal);
}

static void pressButton(const Common::Array<Record *> &records, int button) {
	InputState &state = getState();

	// Two clicks close together in time and place are a double click
	const uint32 now = g_system->getMillis();
	state.isDoubleClick[button] = now - state.lastClickTime[button] < kDoubleClickTime &&
		ABS(state.mouse.x - state.lastClickPoint[button].x) <= kDoubleClickDistance &&
		ABS(state.mouse.y - state.lastClickPoint[button].y) <= kDoubleClickDistance;
	state.lastClickTime[button] = state.isDoubleClick[button] ? 0 : now;
	state.lastClickPoint[button] = state.mouse;

	for (Record *record : records) {
		if (record == state.hovered) {
			if (record->pressRegion(button)) {
				state.pressed[button] = record;
				return;
			}

			continue;
		}

		if (isOverride(record)) {
			if (callInputFunction(record, button == 0 ? "OnLeftDown" : "OnRightDown")) {
				return;
			}

			continue;
		}

		SummaryRecord *summary = toSummaryRecord(record);
		if (summary && summary->sendMouseEvent(button == 0 ? kEventLeftDown : kEventRightDown, state.mouse, true)) {
			return;
		}
	}
}

static void releaseButton(const Common::Array<Record *> &records, int button) {
	InputState &state = getState();

	Record *pressed = state.pressed[button];
	if (pressed) {
		state.pressed[button] = nullptr;
		pressed->releaseRegion(button);
		return;
	}

	for (Record *record : records) {
		if (isOverride(record)) {
			if (callInputFunction(record, button == 0 ? "OnLeftUp" : "OnRightUp")) {
				return;
			}

			continue;
		}

		SummaryRecord *summary = toSummaryRecord(record);
		if (summary) {
			summary->sendMouseEvent(button == 0 ? kEventLeftUp : kEventRightUp, state.mouse, true);
			if (button == 0 && summary->sendMouseEvent(kEventLeftClick, state.mouse, true)) {
				return;
			}
		}
	}
}

static void scrollWheel(const Common::Array<Record *> &records, int notches) {
	InputState &state = getState();

	for (Record *record : records) {
		if (record->scrollRegion(state.mouse, notches)) {
			return;
		}

		if (isOverride(record)) {
			lua_pushinteger(record->getContext().getLua(), notches);
			if (callInputFunction(record, "OnMouseScroll", 1)) {
				return;
			}
		}
	}
}

static bool isKeyFrontmost(const Record *a, const Record *b) {
	return a->getKeyZ() > b->getKeyZ();
}

static bool isMoveKey(Common::KeyCode key) {
	return key == Common::KEYCODE_LEFT || key == Common::KEYCODE_RIGHT ||
		key == Common::KEYCODE_UP || key == Common::KEYCODE_DOWN;
}

// A key goes to the records by their keyZ; ones at the same keyZ in the order
// the mouse goes by
static void sendKey(const Common::Array<Record *> &records, Common::KeyCode key, bool isDown, uint16 ascii) {
	Common::Array<Record *> keyOrder = records;
	Common::sort(keyOrder.begin(), keyOrder.end(), isKeyFrontmost);

	for (Record *record : keyOrder) {
		if (record->capturesKeys() || (isDown && record->handleKey(key, ascii))) {
			return;
		}

		if (isOverride(record)) {
			pushKey(record->getContext().getLua(), key);
			if (callInputFunction(record, isDown ? "OnKeyDown" : "OnKeyUp", 1)) {
				return;
			}

			// A typed letter comes as text as well
			if (isDown && ascii >= 32 && ascii < 127) {
				const char text[2] = { (char)ascii, '\0' };
				lua_pushstring(record->getContext().getLua(), text);
				if (callInputFunction(record, "OnChar", 1)) {
					return;
				}
			}

			// The scripts can keep the arrow keys from turning the view
			if (isDown && isMoveKey(key) && callInputFunction(record, "OnKeyMove")) {
				return;
			}

			continue;
		}

		SummaryRecord *summary = toSummaryRecord(record);
		if (summary && isDown && summary->turnByKey(key)) {
			return;
		}
	}
}

struct MoveKey {
	uint16 input;
	Common::KeyCode key;
};

// The arrow keys are bound to moving around, and Esc to the menu
static const MoveKey kMoveKeys[] = {
	{ NancyInput::kMoveLeft, Common::KEYCODE_LEFT },
	{ NancyInput::kMoveRight, Common::KEYCODE_RIGHT },
	{ NancyInput::kMoveUp, Common::KEYCODE_UP },
	{ NancyInput::kMoveDown, Common::KEYCODE_DOWN }
};

void handleNancy26Input(NancyInput &input) {
	InputState &state = getState();

	const Common::Point32 oldMouse = state.mouse;
	state.hasMouse = input.mousePos.x >= 0;
	if (state.hasMouse) {
		state.mouse = Common::Point32(input.mousePos);
	}

	state.isDoubleClick[0] = state.isDoubleClick[1] = false;

	Common::Array<Record *> records;
	collectRecords(records);

	updateHover(records);

	// Overrides follow the mouse with OnMouseMove and get OnNewFrame every frame
	const bool hasMoved = state.mouse.x != oldMouse.x || state.mouse.y != oldMouse.y;
	for (Record *record : records) {
		if (isOverride(record)) {
			if (hasMoved) {
				record->callFunction("OnMouseMove");
			}

			record->callFunction("OnNewFrame");
		} else if (hasMoved && record->getType() == kRecordSummary) {
			record->sendMouseMove(state.mouse, true);
		}
	}

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		pressButton(records, 0);
	}

	if (input.input & NancyInput::kRightMouseButtonDown) {
		pressButton(records, 1);
	}

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		releaseButton(records, 0);
	}

	if (input.input & NancyInput::kRightMouseButtonUp) {
		releaseButton(records, 1);
	}

	if (input.input & NancyInput::kMouseWheel) {
		scrollWheel(records, (input.input & NancyInput::kMouseWheelUp) ? 1 : -1);
	}

	// What the clicks did can move the mouse onto something else
	collectRecords(records);
	updateHover(records);
	updateCursor(records);

	// The keys that went down last frame have no release of their own
	const Common::Array<Common::KeyCode> keysToRelease = state.keysToRelease;
	state.keysToRelease.clear();
	for (Common::KeyCode key : keysToRelease) {
		sendKey(records, key, false, 0);
	}

	for (const MoveKey &moveKey : kMoveKeys) {
		const bool isHeld = (input.input & moveKey.input) != 0;
		const bool wasHeld = (state.heldMoveKeys & moveKey.input) != 0;
		if (isHeld != wasHeld) {
			sendKey(records, moveKey.key, isHeld, 0);
		}
	}

	state.heldMoveKeys = input.input;

	const bool isEscHeld = (input.input & NancyInput::kOpenMainMenu) != 0;
	if (isEscHeld != state.isEscHeld) {
		sendKey(records, Common::KEYCODE_ESCAPE, isEscHeld, 0);
		state.isEscHeld = isEscHeld;
	}

	for (const Common::KeyState &key : input.otherKbdInput) {
		sendKey(records, key.keycode, true, key.ascii);
		state.keysToRelease.push_back(key.keycode);
	}
}

} // End of namespace Script

namespace State {

// Nancy26's records take care of all the input themselves. Esc opens the menu
// through them, unless it skips the movies that play or what the characters
// say; that press is then taken away from everything.
void Scene::handleNancy26Input(NancyInput &input) {
	const bool escPressed = (input.input & NancyInput::kOpenMainMenu) != 0;
	if (escPressed && !_escHeld && _scriptScene) {
		bool hasSkipped = _scriptScene->skipMovies();
		hasSkipped |= _streams.skipMovies();
		if (!hasSkipped) {
			hasSkipped = _scriptScene->skipSpeech();
			hasSkipped |= _streams.skipSpeech();
		}

		if (hasSkipped) {
			g_nancy->_input->eatMainMenuKey();
			input.eatMainMenuKey();
		}
	}

	_escHeld = (input.input & NancyInput::kOpenMainMenu) != 0;

	Script::handleNancy26Input(input);
}

} // End of namespace State
} // End of namespace Nancy
