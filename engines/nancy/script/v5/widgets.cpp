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

#include "common/events.h"
#include "common/system.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/mathtypes.h"
#include "engines/nancy/script/ref.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/script/v5/scriptinput.h"
#include "engines/nancy/script/v5/widgets.h"

namespace Nancy {
namespace Script {

// Nancy26 names its cursors with strings ("MenuHot")
static bool readCursorValue(lua_State *L, int index, uint &cursorType, bool &isHot) {
	if (lua_type(L, index) == LUA_TSTRING) {
		const int type = g_nancy->_cursor->getCursorTypeByName(lua_tostring(L, index));
		if (type < 0) {
			return false;
		}

		cursorType = type;
		isHot = false;
		return true;
	}

	return testCursor(L, index, cursorType, isHot);
}

static int setTableField(lua_State *L) {
	lua_settable(L, 1);
	return 0;
}

// Sets a field of another record the way a script would, with the value at the
// top of the stack, which gets popped
static void setRecordField(Record *record, const char *key) {
	lua_State *L = record->getContext().getLua();
	lua_pushcfunction(L, setTableField);
	lua_insert(L, -2);
	record->push();
	lua_insert(L, -2);
	lua_pushstring(L, key);
	lua_insert(L, -2);
	LuaState::call(L, 3, 0, "Error setting a field of an AR");
}

// HotspotRecord

void HotspotRecord::readCursorField(int valueIndex, Cursor &cursor) {
	cursor.isSet = readCursorValue(getLua(), valueIndex, cursor.type, cursor.isHot);
}

void HotspotRecord::pushCursorField(const Cursor &cursor) {
	if (cursor.isSet) {
		pushCursor(getLua(), cursor.type, cursor.isHot);
	} else {
		lua_pushnil(getLua());
	}
}

void HotspotRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "onScreen")) {
		_onScreen = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "scene", _sceneName);
	readField(L, tableIndex, "frame", _frame);
	readField(L, tableIndex, "stream", _streamName);
	readField(L, tableIndex, "watchLeft", _watchesLeft);
	readField(L, tableIndex, "watchRight", _watchesRight);

	static const char *const kCursorNames[] = { "cursor", "startCursor", "downCursor", "dragOnCursor", "dragOffCursor" };
	Cursor *const cursors[] = { &_cursor, &_startCursor, &_downCursor, &_dragOnCursor, &_dragOffCursor };
	for (uint i = 0; i < ARRAYSIZE(kCursorNames); ++i) {
		if (pushField(L, tableIndex, kCursorNames[i])) {
			readCursorField(-1, *cursors[i]);
			lua_pop(L, 1);
		}
	}

	Common::String refName;
	if (readField(L, tableIndex, "ref", refName) && !refName.empty()) {
		_ref = getContext().loadRef(refName);
	}

	if (pushField(L, tableIndex, "refColor")) {
		_refColor = checkColor(L, -1);
		_hasRefColor = true;
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "source")) {
		_source = checkRect(L, -1);
		lua_pop(L, 1);
	}
}

bool HotspotRecord::hitTest(const Common::Point32 &point) const {
	double x = point.x;
	double y = point.y;
	getTransform().inverse().transformPoint(x, y);

	int refX = (int)floor(x);
	int refY = (int)floor(y);
	if (!_ref) {
		return _onScreen.contains(refX, refY);
	}

	if (!_source.isEmpty() || !_onScreen.isEmpty()) {
		if (!_onScreen.contains(refX, refY)) {
			return false;
		}

		refX += _source.left - _onScreen.left;
		refY += _source.top - _onScreen.top;
	}

	uint32 pixel;
	if (!_ref->getPixel(refX, refY, pixel)) {
		return false;
	}

	return _hasRefColor ? pixel == _refColor : (pixel >> 24) != 0;
}

void HotspotRecord::setMouseOver(bool isOver) {
	if (isOver == _isMouseOver) {
		return;
	}

	_isMouseOver = isOver;
	callFunction(isOver ? "OnMouseOver" : "OnMouseLeave");
	if (_widget) {
		_widget->onHotspotOver(isOver);
	}
}

bool HotspotRecord::pressRegion(int button) {
	if (!(button == 0 ? _watchesLeft : _watchesRight)) {
		return false;
	}

	_pressedButton = button;
	callFunction(button == 0 ? "OnLeftDown" : "OnRightDown");
	if (_widget) {
		_widget->onHotspotDown(button);
	}

	return true;
}

// A button that comes up over the hotspot clicks it; anywhere else, the
// hotspot only lets go of the mouse
void HotspotRecord::releaseRegion(int button) {
	if (_pressedButton != button) {
		return;
	}

	_pressedButton = -1;

	if (!_isMouseOver) {
		callFunction("OnLoseMouseFocus");
		if (_widget) {
			_widget->onHotspotUp(button, false);
		}

		return;
	}

	callFunction(button == 0 ? "OnLeftUp" : "OnRightUp");
	if (_widget) {
		_widget->onHotspotUp(button, true);
	} else {
		fire();
	}
}

void HotspotRecord::fire() {
	if (!_sceneName.empty()) {
		getContext().changeScene(_streamName, _sceneName, (uint16)_frame);
	}

	finish();
}

bool HotspotRecord::getRegionCursor(uint &cursorType, bool &isHot) const {
	const Cursor *cursor = &_cursor;
	if (_pressedButton >= 0) {
		cursor = _isMouseOver ? &_downCursor : &_dragOffCursor;
	} else if (g_nancy->_cursor->isHoldingItem() && _startCursor.isSet) {
		cursor = &_startCursor;
	}

	if (!cursor->isSet) {
		cursor = &_cursor;
	}

	if (!cursor->isSet) {
		return false;
	}

	cursorType = cursor->type;
	isHot = cursor->isHot;
	return true;
}

bool HotspotRecord::prepareReset() {
	_pressedButton = -1;
	return true;
}

bool HotspotRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _onScreen);
	} else if (key == "scene") {
		lua_pushstring(L, _sceneName.c_str());
	} else if (key == "stream") {
		lua_pushstring(L, _streamName.c_str());
	} else if (key == "frame") {
		lua_pushinteger(L, _frame);
	} else if (key == "watchLeft") {
		lua_pushboolean(L, _watchesLeft);
	} else if (key == "watchRight") {
		lua_pushboolean(L, _watchesRight);
	} else if (key == "down") {
		lua_pushboolean(L, isDown());
	} else if (key == "mouseOver") {
		lua_pushboolean(L, _isMouseOver);
	} else if (key == "cursor") {
		pushCursorField(_cursor);
	} else if (key == "startCursor") {
		pushCursorField(_startCursor);
	} else if (key == "downCursor") {
		pushCursorField(_downCursor);
	} else if (key == "dragOnCursor") {
		pushCursorField(_dragOnCursor);
	} else if (key == "dragOffCursor") {
		pushCursorField(_dragOffCursor);
	} else if (key == "refColor" && _hasRefColor) {
		pushColor(L, _refColor);
	} else if (key == "source") {
		pushRect(L, _source);
	} else {
		return false;
	}

	return true;
}

bool HotspotRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_onScreen = checkRect(L, 3);
	} else if (key == "scene") {
		_sceneName = luaL_checkstring(L, 3);
	} else if (key == "stream") {
		_streamName = luaL_checkstring(L, 3);
	} else if (key == "frame") {
		_frame = luaL_checkint(L, 3);
	} else if (key == "watchLeft") {
		_watchesLeft = lua_toboolean(L, 3) != 0;
	} else if (key == "watchRight") {
		_watchesRight = lua_toboolean(L, 3) != 0;
	} else if (key == "cursor") {
		readCursorField(3, _cursor);
	} else if (key == "startCursor") {
		readCursorField(3, _startCursor);
	} else if (key == "downCursor") {
		readCursorField(3, _downCursor);
	} else if (key == "dragOnCursor") {
		readCursorField(3, _dragOnCursor);
	} else if (key == "dragOffCursor") {
		readCursorField(3, _dragOffCursor);
	} else if (key == "ref") {
		_ref = lua_isnil(L, 3) ? nullptr : getContext().loadRef(luaL_checkstring(L, 3));
	} else if (key == "refColor") {
		_hasRefColor = !lua_isnil(L, 3);
		if (_hasRefColor) {
			_refColor = checkColor(L, 3);
		}
	} else if (key == "source") {
		_source = checkRect(L, 3);
	} else {
		return false;
	}

	return true;
}

// SinkRecord

void SinkRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	// The whole viewport, unless the table says otherwise
	const VIEW *viewport = GetEngineData(VIEW);
	if (viewport) {
		_onScreen = ScriptRect(0, 0, viewport->screenPosition.width(), viewport->screenPosition.height());
	}

	if (pushField(L, tableIndex, "onScreen")) {
		_onScreen = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "handleKeys", _handlesKeys);
	readField(L, tableIndex, "handleMouse", _handlesMouse);

	if (pushField(L, tableIndex, "cursor")) {
		_hasCursor = readCursorValue(L, -1, _cursorType, _cursorIsHot);
		lua_pop(L, 1);
	}
}

bool SinkRecord::hitTest(const Common::Point32 &point) const {
	double x = point.x;
	double y = point.y;
	getTransform().inverse().transformPoint(x, y);
	return _onScreen.contains((int)floor(x), (int)floor(y));
}

bool SinkRecord::getRegionCursor(uint &cursorType, bool &isHot) const {
	if (!_hasCursor) {
		return false;
	}

	cursorType = _cursorType;
	isHot = _cursorIsHot;
	return true;
}

bool SinkRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _onScreen);
	} else if (key == "handleKeys") {
		lua_pushboolean(L, _handlesKeys);
	} else if (key == "handleMouse") {
		lua_pushboolean(L, _handlesMouse);
	} else if (key == "cursor") {
		if (_hasCursor) {
			pushCursor(L, _cursorType, _cursorIsHot);
		} else {
			lua_pushnil(L);
		}
	} else {
		return false;
	}

	return true;
}

bool SinkRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_onScreen = checkRect(L, 3);
	} else if (key == "handleKeys") {
		_handlesKeys = lua_toboolean(L, 3) != 0;
	} else if (key == "handleMouse") {
		_handlesMouse = lua_toboolean(L, 3) != 0;
	} else if (key == "cursor") {
		_hasCursor = readCursorValue(L, 3, _cursorType, _cursorIsHot);
	} else {
		return false;
	}

	return true;
}

// WidgetRecord

WidgetRecord::WidgetRecord(SceneContext &context, RecordType type) :
		ClipTransformerRecord(context, getRecordTypeName(type)),
		_type(type) {}

// A record the table holds, which then hangs under the widget
Record *WidgetRecord::readRecordField(int tableIndex, const char *name) {
	lua_State *L = getLua();

	if (!pushField(L, tableIndex, name)) {
		return nullptr;
	}

	Record *record = toRecord(L, -1);
	lua_pop(L, 1);

	if (record) {
		record->setParentNode(getNode());
	}

	return record;
}

void WidgetRecord::readTable(int tableIndex) {
	ClipTransformerRecord::readTable(tableIndex);
	lua_State *L = getLua();

	_hotspot = dynamic_cast<HotspotRecord *>(readRecordField(tableIndex, "hs"));
	if (_hotspot) {
		_hotspot->setWidget(this);
	}

	_baseOverlay = readRecordField(tableIndex, "baseOvl");
	_upOverlay = readRecordField(tableIndex, "upOvl");
	_downOverlay = readRecordField(tableIndex, "downOvl");
	_overOverlay = readRecordField(tableIndex, "overOvl");
	_selectedOverlay = readRecordField(tableIndex, "selectedOvl");
	_text = readRecordField(tableIndex, "text");

	if (pushField(L, tableIndex, "baseColor")) {
		_baseColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "disabledColor")) {
		_disabledColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "isSelected", _isSelected);
	readField(L, tableIndex, "groupName", _groupName);

	bool isInitiallySelected = false;
	if (readField(L, tableIndex, "initiallySelected", isInitiallySelected) && isInitiallySelected) {
		setSelected(true, false);
	}

	_wasEnabled = !isNodeEnabled();
	updateState();
}

void WidgetRecord::showOverlay(Record *overlay, bool isShown, bool isEnabled) {
	if (!overlay) {
		return;
	}

	overlay->setLocalVisible(isShown);

	if (isEnabled != _wasEnabled) {
		pushColor(overlay->getContext().getLua(), isEnabled ? _baseColor : _disabledColor);
		setRecordField(overlay, "modColor");
	}
}

void WidgetRecord::updateState() {
	const bool isEnabled = isNodeEnabled();
	const bool isOver = _hotspot && _hotspot->isMouseOver() && isEnabled;
	const bool isDown = _hotspot && _hotspot->isDown() && isEnabled;

	showOverlay(_baseOverlay, true, isEnabled);
	showOverlay(_selectedOverlay, _isSelected, isEnabled);
	showOverlay(_downOverlay, isDown && isOver, isEnabled);
	showOverlay(_overOverlay, isOver && !isDown, isEnabled);
	showOverlay(_upOverlay, !(isOver && !isDown) && !(isDown && isOver) && !(_isSelected && _selectedOverlay), isEnabled);

	if (_text && isEnabled != _wasEnabled) {
		pushColor(getLua(), isEnabled ? _baseColor : _disabledColor);
		setRecordField(_text, "modColor");
	}

	_wasEnabled = isEnabled;
}

void WidgetRecord::updateGraphics() {
	// The overlays show what the widget is up to even while it is paused
	updateState();
}

void WidgetRecord::onHotspotOver(bool isOver) {
	callFunction(isOver ? "OnMouseOver" : "OnMouseLeave");
	updateState();
}

void WidgetRecord::onHotspotDown(int button) {
	if (button != 0) {
		return;
	}

	switch (_type) {
	case kRecordCheckBox:
		setSelected(!_isSelected, true);
		break;
	case kRecordRadioButton:
		setSelected(true, true);
		break;
	default:
		break;
	}

	callFunction("OnDown");
	updateState();
}

void WidgetRecord::onHotspotUp(int button, bool isOver) {
	if (button != 0) {
		return;
	}

	callFunction(isOver ? "OnUp" : "OnLoseMouseFocus");
	updateState();
}

// A radio button that gets selected deselects the others of its group
void WidgetRecord::setSelected(bool isSelected, bool callsScripts) {
	if (isSelected == _isSelected) {
		return;
	}

	_isSelected = isSelected;

	if (isSelected && _type == kRecordRadioButton) {
		for (Record *record : getContext().getRecords()) {
			if (record == this || record->getType() != kRecordRadioButton) {
				continue;
			}

			WidgetRecord *other = static_cast<WidgetRecord *>(record);
			if (other->_groupName.equalsIgnoreCase(_groupName)) {
				other->setSelected(false, callsScripts);
			}
		}
	}

	if (callsScripts) {
		callFunction(isSelected ? "OnSelect" : "OnDeselect");
	}

	updateState();
}

bool WidgetRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	Record *record = nullptr;
	if (key == "hs") {
		record = _hotspot;
	} else if (key == "baseOvl") {
		record = _baseOverlay;
	} else if (key == "upOvl") {
		record = _upOverlay;
	} else if (key == "downOvl") {
		record = _downOverlay;
	} else if (key == "overOvl") {
		record = _overOverlay;
	} else if (key == "selectedOvl") {
		record = _selectedOverlay;
	} else if (key == "text") {
		record = _text;
	} else if (key == "baseColor") {
		pushColor(L, _baseColor);
		return true;
	} else if (key == "disabledColor") {
		pushColor(L, _disabledColor);
		return true;
	} else if (key == "isSelected") {
		lua_pushboolean(L, _isSelected);
		return true;
	} else if (key == "groupName") {
		lua_pushstring(L, _groupName.c_str());
		return true;
	} else {
		return ClipTransformerRecord::getField(key);
	}

	if (record) {
		record->push();
	} else {
		lua_pushnil(L);
	}

	return true;
}

bool WidgetRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "baseColor") {
		_baseColor = checkColor(L, 3);
		_wasEnabled = !isNodeEnabled();
	} else if (key == "disabledColor") {
		_disabledColor = checkColor(L, 3);
		_wasEnabled = !isNodeEnabled();
	} else if (key == "isSelected") {
		setSelected(lua_toboolean(L, 3) != 0, false);
	} else {
		return ClipTransformerRecord::setField(key);
	}

	updateState();
	return true;
}

// SliderRecord

void SliderRecord::readTable(int tableIndex) {
	WidgetRecord::readTable(tableIndex);
	lua_State *L = getLua();

	_knobOverlay = readRecordField(tableIndex, "knobOvl");

	if (pushField(L, tableIndex, "knobArea")) {
		_knobArea = checkRect(L, -1);
		lua_pop(L, 1);
	}

	// Without one, the knob goes along the hotspot
	if (_knobArea.isEmpty() && _hotspot) {
		_knobArea = _hotspot->getOnScreen();
	}

	if (pushField(L, tableIndex, "scrollArea")) {
		_scrollArea = checkRect(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "scrollAmount")) {
		readPoint(L, -1, _scrollAmount);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "knobPos")) {
		readPoint(L, -1, _knobPos);
		lua_pop(L, 1);
	}

	setKnobPos(_knobPos, false);
}

// The knob is as large as the part of the picture it shows
static Common::Point32 getOverlaySize(Record *overlay) {
	Common::Point32 size;
	if (!overlay) {
		return size;
	}

	lua_State *L = overlay->getContext().getLua();
	overlay->push();
	lua_getfield(L, -1, "source");
	const ScriptRect *source = testRect(L, -1);
	if (source) {
		size = Common::Point32(source->width(), source->height());
	}

	lua_pop(L, 2);
	return size;
}

Common::Point32 SliderRecord::getKnobMax() const {
	const Common::Point32 knobSize = getOverlaySize(_knobOverlay);
	return Common::Point32(MAX<int>(0, _knobArea.width() - knobSize.x), MAX<int>(0, _knobArea.height() - knobSize.y));
}

void SliderRecord::setKnobPos(const Common::Point32 &pos, bool callsScripts) {
	const Common::Point32 knobMax = getKnobMax();
	const Common::Point32 newPos(CLIP<int>(pos.x, 0, knobMax.x), CLIP<int>(pos.y, 0, knobMax.y));

	const bool hasMoved = newPos.x != _knobPos.x || newPos.y != _knobPos.y;
	_knobPos = newPos;
	placeKnob();

	if (hasMoved && callsScripts) {
		callFunction("OnKnobMoved");
	}
}

void SliderRecord::placeKnob() {
	if (!_knobOverlay) {
		return;
	}

	const Common::Point32 knobSize = getOverlaySize(_knobOverlay);
	ScriptRect dest(0, 0, knobSize.x, knobSize.y);
	dest.translate(_knobArea.left + _knobPos.x, _knobArea.top + _knobPos.y);

	pushRect(_knobOverlay->getContext().getLua(), dest);
	setRecordField(_knobOverlay, "onScreen");
}

// A click on the knob drags it from where it was taken; anywhere else on the
// bar, the knob jumps there first
void SliderRecord::onHotspotDown(int button) {
	if (button != 0) {
		return;
	}

	const Common::Point32 mouse = getScriptMousePoint();
	const Common::Point32 knobSize = getOverlaySize(_knobOverlay);
	const ScriptRect knob(_knobArea.left + _knobPos.x, _knobArea.top + _knobPos.y,
		_knobArea.left + _knobPos.x + knobSize.x, _knobArea.top + _knobPos.y + knobSize.y);

	if (knob.contains(mouse.x, mouse.y)) {
		_dragOffset = Common::Point32(mouse.x - knob.left, mouse.y - knob.top);
	} else {
		_dragOffset = Common::Point32(knobSize.x / 2, knobSize.y / 2);
		setKnobPos(Common::Point32(mouse.x - _knobArea.left - _dragOffset.x, mouse.y - _knobArea.top - _dragOffset.y), true);
	}

	_isDragging = true;
	WidgetRecord::onHotspotDown(button);
}

void SliderRecord::onHotspotUp(int button, bool isOver) {
	if (button == 0) {
		_isDragging = false;
	}

	WidgetRecord::onHotspotUp(button, isOver);
}

void SliderRecord::run(double sceneTime, double elapsedTime) {
	ClipTransformerRecord::run(sceneTime, elapsedTime);

	if (_isDragging) {
		const Common::Point32 mouse = getScriptMousePoint();
		setKnobPos(Common::Point32(mouse.x - _knobArea.left - _dragOffset.x, mouse.y - _knobArea.top - _dragOffset.y), true);
	}
}

bool SliderRecord::scrollRegion(const Common::Point32 &point, int notches) {
	if (!isNodeEnabled() || (_scrollAmount.x == 0 && _scrollAmount.y == 0)) {
		return false;
	}

	if (!_scrollArea.contains(point.x, point.y) && !_knobArea.contains(point.x, point.y)) {
		return false;
	}

	setKnobPos(Common::Point32(_knobPos.x - notches * _scrollAmount.x, _knobPos.y - notches * _scrollAmount.y), true);
	return true;
}

void SliderRecord::updateState() {
	WidgetRecord::updateState();

	if (_knobOverlay) {
		_knobOverlay->setLocalVisible(true);
	}
}

bool SliderRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "knobOvl") {
		if (_knobOverlay) {
			_knobOverlay->push();
		} else {
			lua_pushnil(L);
		}
	} else if (key == "knobArea") {
		pushRect(L, _knobArea);
	} else if (key == "scrollArea") {
		pushRect(L, _scrollArea);
	} else if (key == "scrollAmount") {
		pushPoint(L, _scrollAmount);
	} else if (key == "knobPos") {
		pushPoint(L, _knobPos);
	} else if (key == "knobMax") {
		pushPoint(L, getKnobMax());
	} else if (key == "percent") {
		const Common::Point32 knobMax = getKnobMax();
		pushVector(L, ScriptVector(knobMax.x > 0 ? (double)_knobPos.x / knobMax.x : 0.0,
			knobMax.y > 0 ? (double)_knobPos.y / knobMax.y : 0.0, 0.0));
	} else {
		return WidgetRecord::getField(key);
	}

	return true;
}

bool SliderRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "knobPos") {
		Common::Point32 pos;
		readPoint(L, 3, pos);
		setKnobPos(pos, false);
	} else if (key == "scrollArea") {
		_scrollArea = checkRect(L, 3);
	} else if (key == "scrollAmount") {
		readPoint(L, 3, _scrollAmount);
	} else if (key == "knobArea") {
		_knobArea = checkRect(L, 3);
		setKnobPos(_knobPos, false);
	} else {
		return WidgetRecord::setField(key);
	}

	return true;
}

// EditBoxRecord

// The edit box that has the keyboard
static EditBoxRecord *s_focusedEditBox = nullptr;

EditBoxRecord::EditBoxRecord(SceneContext &context) : Record(context, "ND.EditBoxAR") {}

EditBoxRecord::~EditBoxRecord() {
	if (s_focusedEditBox == this) {
		s_focusedEditBox = nullptr;
	}
}

void EditBoxRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "onScreen")) {
		_dest = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "fontIdx", _fontID);
	readField(L, tableIndex, "selectedColor", _selectedColor);
	readField(L, tableIndex, "unselectedColor", _unselectedColor);
	readField(L, tableIndex, "disabledSelectedColor", _disabledSelectedColor);
	readField(L, tableIndex, "disabledUnselectedColor", _disabledUnselectedColor);
	readField(L, tableIndex, "enteredString", _text);

	if (pushField(L, tableIndex, "caretColor")) {
		_caretColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "selectionBgColor")) {
		_selectionColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "cursor")) {
		_hasCursor = readCursorValue(L, -1, _cursorType, _cursorIsHot);
		lua_pop(L, 1);
	}

	// The picture behind the text hangs where the box does
	if (pushField(L, tableIndex, "baseOvl")) {
		Record *overlay = toRecord(L, -1);
		lua_pop(L, 1);
		if (overlay) {
			overlay->setParentNode(getParentNode());
		}
	}

	_graphic.init();
	_graphic.setVisible(false);
	applyDest();
	onZChanged();
	_graphic.registerGraphics();
}

bool EditBoxRecord::hasFocus() const {
	return s_focusedEditBox == this && isNodeEnabled();
}

void EditBoxRecord::applyDest() {
	const int width = MAX<int>(_dest.width(), 1);
	const int height = MAX<int>(_dest.height(), 1);
	if (_graphic._drawSurface.w != width || _graphic._drawSurface.h != height) {
		_graphic._drawSurface.create(width, height, g_nancy->_graphics->getTransparentPixelFormat());
	}

	double x = _dest.left;
	double y = _dest.top;
	getTransform().transformPoint(x, y);
	_graphic.moveTo(Common::Rect((int)x, (int)y, (int)x + width, (int)y + height));
	_isDirty = true;
}

void EditBoxRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext());
}

void EditBoxRecord::registerGraphics() {
	_graphic.registerGraphics();
}

bool EditBoxRecord::hitTest(const Common::Point32 &point) const {
	double x = point.x;
	double y = point.y;
	getTransform().inverse().transformPoint(x, y);
	return _dest.contains((int)floor(x), (int)floor(y));
}

bool EditBoxRecord::pressRegion(int button) {
	if (button != 0) {
		return false;
	}

	s_focusedEditBox = this;
	_isDirty = true;
	callFunction("OnClicked");
	return true;
}

bool EditBoxRecord::getRegionCursor(uint &cursorType, bool &isHot) const {
	if (!_hasCursor) {
		return false;
	}

	cursorType = _cursorType;
	isHot = _cursorIsHot;
	return true;
}

void EditBoxRecord::setString(const Common::String &text, bool callsScripts) {
	if (text == _text) {
		return;
	}

	_text = text;
	_isDirty = true;
	if (callsScripts) {
		callFunction("OnEdit");
	}
}

void EditBoxRecord::selectAll() {
	s_focusedEditBox = this;
	_isAllSelected = true;
	_isDirty = true;
}

void EditBoxRecord::endSelection() {
	_isAllSelected = false;
	_isDirty = true;
}

bool EditBoxRecord::handleKey(Common::KeyCode key, uint16 ascii) {
	if (!hasFocus()) {
		return false;
	}

	switch (key) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		callFunction("OnEditComplete");
		return true;
	case Common::KEYCODE_TAB:
		if (g_system->getEventManager()->getModifierState() & Common::KBD_SHIFT) {
			callFunction("OnShiftTabPressed");
		} else {
			callFunction("OnTabPressed");
		}

		return true;
	case Common::KEYCODE_BACKSPACE:
		if (_isAllSelected) {
			_isAllSelected = false;
			setString("", true);
		} else if (!_text.empty()) {
			Common::String text = _text;
			text.deleteLastChar();
			setString(text, true);
		}

		return true;
	default:
		break;
	}

	if (ascii < 32 || ascii >= 127) {
		return false;
	}

	Common::String text = _isAllSelected ? Common::String() : _text;
	text += (char)ascii;

	// A letter there is no room for stays out
	if (getBitmapTextWidth(text, _fontID) > _dest.width()) {
		callFunction("OnTooWide");
		return true;
	}

	_isAllSelected = false;
	setString(text, true);
	return true;
}

static uint32 toSurfaceColor(const Graphics::ManagedSurface &surface, uint32 argb) {
	return surface.format.ARGBToColor(argb >> 24, (argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);
}

void EditBoxRecord::updateGraphics() {
	_graphic.setClipRect(getScreenClip());
	_graphic.setVisible(isShown(), false);

	const bool isFocused = hasFocus();
	if (isFocused != _hadFocus) {
		_hadFocus = isFocused;
		_isDirty = true;
	}

	if (!_isDirty && !hasAlphaChanged()) {
		return;
	}

	_isDirty = false;

	Graphics::ManagedSurface &surface = _graphic._drawSurface;
	surface.clear(0);

	int colorID = isFocused ? _selectedColor : _unselectedColor;
	if (!isNodeEnabled()) {
		colorID = isFocused ? _disabledSelectedColor : _disabledUnselectedColor;
	}

	_layout.layout(Common::String::format("<f%d><vc>", _fontID) + _text, surface.w, surface.h, colorID);
	const ScriptRect &bounds = _layout.getBounds();

	if (_isAllSelected && !_text.empty()) {
		const Common::Rect area(MAX<int>(bounds.left, 0), 0, MIN<int>(bounds.right, surface.w), surface.h);
		if (!area.isEmpty()) {
			surface.fillRect(area, toSurfaceColor(surface, applyAlpha(_selectionColor)));
		}
	}

	_layout.draw(surface, Common::Point(0, 0), takeModColor(0xFFFFFFFF));

	// The caret goes after the text while the box has the keyboard
	if (isFocused) {
		const int x = CLIP<int>(_text.empty() ? 1 : bounds.right + 1, 0, surface.w - 1);
		surface.vLine(x, 2, surface.h - 3, toSurfaceColor(surface, applyAlpha(_caretColor)));
	}

	_graphic.setNeedsRedraw(true);
}

static int editBoxSelectAll(lua_State *L) {
	EditBoxRecord *editBox = dynamic_cast<EditBoxRecord *>(checkRecord(L, 1));
	if (editBox) {
		editBox->selectAll();
	}

	return 0;
}

static int editBoxEndSelection(lua_State *L) {
	EditBoxRecord *editBox = dynamic_cast<EditBoxRecord *>(checkRecord(L, 1));
	if (editBox) {
		editBox->endSelection();
	}

	return 0;
}

bool EditBoxRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "enteredString") {
		lua_pushstring(L, _text.c_str());
	} else if (key == "fontIdx") {
		lua_pushinteger(L, _fontID);
	} else if (key == "selectedColor") {
		lua_pushinteger(L, _selectedColor);
	} else if (key == "unselectedColor") {
		lua_pushinteger(L, _unselectedColor);
	} else if (key == "disabledSelectedColor") {
		lua_pushinteger(L, _disabledSelectedColor);
	} else if (key == "disabledUnselectedColor") {
		lua_pushinteger(L, _disabledUnselectedColor);
	} else if (key == "caretColor") {
		pushColor(L, _caretColor);
	} else if (key == "selectionBgColor" || key == "disabledSelectionBgColor") {
		pushColor(L, _selectionColor);
	} else if (key == "SelectAll") {
		lua_pushcfunction(L, editBoxSelectAll);
	} else if (key == "EndSelection") {
		lua_pushcfunction(L, editBoxEndSelection);
	} else {
		return false;
	}

	return true;
}

bool EditBoxRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_dest = checkRect(L, 3);
		applyDest();
	} else if (key == "enteredString") {
		setString(luaL_checkstring(L, 3), false);
	} else {
		return false;
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
