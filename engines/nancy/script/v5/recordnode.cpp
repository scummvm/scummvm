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

// The parts of Record that make every record of Nancy26 a node of a tree

#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/transformer.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/script/v5/cliptransformer.h"
#include "engines/nancy/script/v5/messaging.h"
#include "engines/nancy/script/v5/recordnode.h"
#include "engines/nancy/script/v5/scriptinput.h"

namespace Nancy {
namespace Script {

void Record::createNodeState() {
	_nodeState = new RecordNodeState();

	// Every record goes by a name of its own
	static uint s_numRecords = 0;
	_nodeState->uniqueAlias = Common::String::format("AR%u", ++s_numRecords);
}

void Record::destroyNodeState() {
	if (!_nodeState) {
		return;
	}

	deregisterAliases(this);
	forgetInputRecord(this);
	delete _nodeState;
	_nodeState = nullptr;
}

bool Record::isNodeEnabled() const {
	if (!_nodeState->localEnabled || !_context.isStreamEnabled()) {
		return false;
	}

	return !_parentNode || _parentNode->isEnabled();
}

bool Record::isNodeVisible() const {
	if (!_nodeState->localVisible || !_context.isStreamVisible()) {
		return false;
	}

	return !_parentNode || _parentNode->isVisible();
}

bool Record::isNodePaused() const {
	if (_nodeState->localPaused || _context.isStreamPaused()) {
		return true;
	}

	return _parentNode && _parentNode->isPaused();
}

double Record::getAlpha() const {
	return _parentNode ? _nodeState->localAlpha * _parentNode->getAlpha() : _nodeState->localAlpha;
}

void Record::setLocalEnabled(bool isEnabled) {
	_nodeState->localEnabled = isEnabled;
}

void Record::setLocalVisible(bool isVisible) {
	_nodeState->localVisible = isVisible;
}

void Record::setLocalPaused(bool isPaused) {
	_nodeState->localPaused = isPaused;
}

void Record::setActiveFlags(bool isActive) {
	_nodeState->localEnabled = isActive;
	_nodeState->localVisible = isActive;
	_nodeState->localPaused = !isActive;
}

double Record::getKeyZ() const {
	return _nodeState->keyZ;
}

const Common::String &Record::getUniqueAlias() const {
	return _nodeState->uniqueAlias;
}

const Common::String &Record::getName() const {
	return _nodeState->localName.empty() ? _nodeState->uniqueAlias : _nodeState->localName;
}

void Record::setDrawRank(int rank) {
	if (rank != _nodeState->drawRank) {
		_nodeState->drawRank = rank;
		onZChanged();
	}
}

// What the records draw with. The records of the other games are shown while
// they are active, whole and as they are.

bool Record::isShown() const {
	return _nodeState ? isNodeVisible() : isActive();
}

// The owner of a node, which in Nancy26 is a Transformer or a widget made of one
static const ClipTransformerRecord *toClipTransformer(const Record *owner) {
	if (!owner) {
		return nullptr;
	}

	switch (owner->getType()) {
	case kRecordTransformer:
	case kRecordButton:
	case kRecordCheckBox:
	case kRecordRadioButton:
	case kRecordSlider:
		return static_cast<const ClipTransformerRecord *>(owner);
	default:
		return nullptr;
	}
}

Common::Rect Record::getScreenClip() const {
	Common::Rect clip;
	if (!_nodeState) {
		return clip;
	}

	bool hasClip = false;
	for (const GraphNode *node = _parentNode; node; node = node->getParent()) {
		const ClipTransformerRecord *transformer = toClipTransformer(node->getOwner());
		const ScriptRect *nodeClip = transformer ? transformer->getClip() : nullptr;
		if (!nodeClip) {
			continue;
		}

		// The rect is in the coordinates of the Transformer, which moves and
		// resizes it onto the screen. A flip swaps its corners.
		const ScriptRect moved = transformRect(*nodeClip, node->getTransform());
		const Common::Rect area(MIN(moved.left, moved.right), MIN(moved.top, moved.bottom), MAX(moved.left, moved.right), MAX(moved.top, moved.bottom));

		if (hasClip) {
			clip.clip(area);
		} else {
			clip = area;
			hasClip = true;
		}

		// Nothing is left to draw in, which an empty rect can't say
		if (clip.isEmpty()) {
			return Common::Rect(-1, -1, 0, 0);
		}
	}

	return clip;
}

uint32 Record::applyAlpha(uint32 modColor) const {
	if (!_nodeState) {
		return modColor;
	}

	const double alpha = getAlpha();
	if (alpha >= 1.0) {
		return modColor;
	}

	const uint32 a = (uint32)CLIP<double>((modColor >> 24) * alpha + 0.5, 0.0, 255.0);
	return (a << 24) | (modColor & 0xFFFFFF);
}

uint32 Record::takeModColor(uint32 modColor) {
	if (_nodeState) {
		_nodeState->drawnAlpha = getAlpha();
	}

	return applyAlpha(modColor);
}

bool Record::hasAlphaChanged() const {
	return _nodeState && getAlpha() != _nodeState->drawnAlpha;
}

// Nancy26's records take their own transform and alpha from their stages
void Record::onStagesChanged() {
	if (!_nodeState || !_stages) {
		return;
	}

	_transform = _stages->getTransform();
	if (_stages->hasAlpha()) {
		_nodeState->localAlpha = _stages->getAlpha();
	}

	onLocalTransformChanged();
}

void Record::onLocalTransformChanged() {
	if (hasTransform()) {
		onTransformChanged();
	}
}

static bool readOptionalBool(lua_State *L, int tableIndex, const char *name, bool &value) {
	lua_getfield(L, tableIndex, name);
	const bool isBool = lua_isboolean(L, -1);
	if (isBool) {
		value = lua_toboolean(L, -1) != 0;
	}

	lua_pop(L, 1);
	return isBool;
}

static bool readOptionalNumber(lua_State *L, int tableIndex, const char *name, double &value) {
	lua_getfield(L, tableIndex, name);
	const bool isNumber = lua_type(L, -1) == LUA_TNUMBER;
	if (isNumber) {
		value = lua_tonumber(L, -1);
	}

	lua_pop(L, 1);
	return isNumber;
}

void Record::readNodeFields(int tableIndex) {
	lua_State *L = getLua();

	// The exe passes over values of another type, which scripts use for
	// their own purposes, e.g. a function as localVisible in RandomMovie_SC
	readOptionalBool(L, tableIndex, "localEnabled", _nodeState->localEnabled);
	readOptionalBool(L, tableIndex, "localVisible", _nodeState->localVisible);
	readOptionalBool(L, tableIndex, "localPaused", _nodeState->localPaused);
	readOptionalNumber(L, tableIndex, "localAlpha", _nodeState->localAlpha);
	readOptionalNumber(L, tableIndex, "keyZ", _nodeState->keyZ);

	lua_getfield(L, tableIndex, "localName");
	if (lua_type(L, -1) == LUA_TSTRING) {
		_nodeState->localName = lua_tostring(L, -1);
	}

	lua_pop(L, 1);

	bool isSineLerp = true;
	if (readOptionalBool(L, tableIndex, "sinLerp", isSineLerp)) {
		getStages().setSineLerp(isSineLerp);
	}

	if (pushField(L, tableIndex, "localTransform")) {
		_transform = checkMatrix(L, -1);
		lua_pop(L, 1);
	}
}

// An "active" function sets all three flags every frame. The record gets
// activated when it becomes enabled, visible and unpaused, and runs while it
// isn't paused; a record that is done is paused until it gets reset.
void Record::updateNode(double sceneTime, double elapsedTime) {
	// A record that is done stays so until it gets reset, even when its "active"
	// function says otherwise, e.g. RandomMovie_SC's pause timer
	if (_isDone) {
		return;
	}

	lua_State *L = getLua();

	if (_hasActiveFunction) {
		pushStoredValue("active");
		push();
		if (LuaState::call(L, 1, 1, "Error in AR's active function")) {
			setActiveFlags(lua_toboolean(L, -1) != 0);
			lua_pop(L, 1);
		}
	}

	const bool isActiveNow = isNodeEnabled() && isNodeVisible() && !isNodePaused();
	if (isActiveNow != _wasActive) {
		_wasActive = isActiveNow;

		// The records made of script functions call them by themselves
		if (isActiveNow) {
			onActivation();
			if (!isScripted()) {
				callFunction("OnActivation");
			}
		} else {
			onDeactivation();
			if (!isScripted()) {
				callFunction("OnDeactivation");
			}
		}
	}

	if (isNodePaused() || isPaused()) {
		return;
	}

	_hasStarted = true;

	if (!_hasRunOnce) {
		_hasRunOnce = true;
		runOnce();
		callFunction("RunOnce");
	}

	if (_stages) {
		_stages->advance(elapsedTime);
	}

	if (isScripted()) {
		lua_pushnumber(L, sceneTime);
		lua_pushnumber(L, elapsedTime);
		callFunction("Run", 2);
	} else {
		run(sceneTime, elapsedTime);
	}
}

bool Record::resetInactive() {
	if (!prepareReset()) {
		return false;
	}

	_hasRunOnce = false;
	_isDone = false;
	_hasStarted = false;
	_hasActiveFunction = false;
	setActiveFlags(false);
	callFunction("OnReset");
	return true;
}

int Record::receiveMessage(lua_State *from, const Common::String &sender, const Common::String &action, int firstArg, int numArgs) {
	// Every record answers these by itself first
	Common::String upperAction = action;
	upperAction.toUppercase();
	if (upperAction == "ENABLE") {
		_nodeState->localEnabled = true;
	} else if (upperAction == "DISABLE") {
		_nodeState->localEnabled = false;
	} else if (upperAction == "SHOW") {
		_nodeState->localVisible = true;
	} else if (upperAction == "HIDE") {
		_nodeState->localVisible = false;
	} else if (upperAction == "PLAY") {
		_nodeState->localPaused = false;
	} else if (upperAction == "PAUSE") {
		_nodeState->localPaused = true;
	}

	lua_State *L = getLua();
	firstArg = absoluteIndex(from, firstArg);

	pushStoredValue("Receive");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}

	const int base = lua_gettop(L) - 1;
	push();
	lua_pushstring(L, sender.c_str());
	lua_pushstring(L, action.c_str());
	for (int i = 0; i < numArgs; ++i) {
		if (L == from) {
			lua_pushvalue(L, firstArg + i);
		} else {
			copyValue(from, firstArg + i, L);
		}
	}

	if (!LuaState::call(L, numArgs + 3, LUA_MULTRET, "Error running override 'Receive' of AR")) {
		return 0;
	}

	const int numResults = lua_gettop(L) - base;
	if (L != from) {
		for (int i = 0; i < numResults; ++i) {
			copyValue(L, base + 1 + i, from);
		}

		lua_settop(L, base);
	}

	return numResults;
}

// ar:ResetInactive()
static int recordResetInactive(lua_State *L) {
	if (!checkRecord(L, 1)->resetInactive()) {
		return luaL_error(L, "Cannot reset AR");
	}

	return 0;
}

int Record::getNodeScriptField(const Common::String &key) {
	lua_State *L = getLua();

	// The kinds that can be done say so themselves
	if (key == "done" && getField(key)) {
		return 1;
	}

	if (getNodeField(key) || getField(key)) {
		return 1;
	}

	pushStoredValue(key.c_str());
	if (!lua_isnil(L, -1)) {
		return 1;
	}

	lua_pop(L, 1);
	return indexFunctions(L);
}

// The fields and functions every record has
bool Record::getNodeField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "active") {
		lua_pushboolean(L, isActive());
	} else if (key == "done") {
		lua_pushboolean(L, _isDone);
	} else if (key == "z") {
		lua_pushnumber(L, _z);
	} else if (key == "keyZ") {
		lua_pushnumber(L, _nodeState->keyZ);
	} else if (key == "alpha") {
		lua_pushnumber(L, getAlpha());
	} else if (key == "localAlpha") {
		lua_pushnumber(L, _nodeState->localAlpha);
	} else if (key == "enabled") {
		lua_pushboolean(L, isNodeEnabled());
	} else if (key == "visible") {
		lua_pushboolean(L, isNodeVisible());
	} else if (key == "paused") {
		lua_pushboolean(L, isNodePaused());
	} else if (key == "localEnabled") {
		lua_pushboolean(L, _nodeState->localEnabled);
	} else if (key == "localVisible") {
		lua_pushboolean(L, _nodeState->localVisible);
	} else if (key == "localPaused") {
		lua_pushboolean(L, _nodeState->localPaused);
	} else if (key == "transform") {
		pushMatrix(L, getTransform());
	} else if (key == "invTransform") {
		pushMatrix(L, getTransform().inverse());
	} else if (key == "localTransform") {
		pushMatrix(L, getLocalTransform());
	} else if (key == "localInvTransform") {
		pushMatrix(L, getLocalTransform().inverse());
	} else if (key == "blendedTransform") {
		pushMatrix(L, _stages ? _stages->getBlendedTransform() : getLocalTransform());
	} else if (key == "name") {
		lua_pushstring(L, getName().c_str());
	} else if (key == "uniqueAlias") {
		lua_pushstring(L, _nodeState->uniqueAlias.c_str());
	} else if (key == "Send" || key == "TrySend") {
		lua_pushcfunction(L, recordSend);
	} else if (key == "SendAsAlias" || key == "TrySendAsAlias") {
		lua_pushcfunction(L, recordSendAsAlias);
	} else if (key == "RegisterAlias") {
		lua_pushcfunction(L, recordRegisterAlias);
	} else if (key == "DeregisterAlias") {
		lua_pushcfunction(L, recordDeregisterAlias);
	} else if (key == "ResetInactive") {
		lua_pushcfunction(L, recordResetInactive);
	} else if (BlendStages::pushFunction(L, key)) {
		return true;
	} else if (_stages && _stages->getField(L, key)) {
		return true;
	} else {
		return false;
	}

	return true;
}

bool Record::setNodeField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "active") {
		if (lua_isfunction(L, 3)) {
			lua_pushvalue(L, 3);
			storeValue("active");
			_hasActiveFunction = true;
		} else {
			setActiveFlags(lua_toboolean(L, 3) != 0);
			_hasActiveFunction = false;
		}
	} else if (key == "z") {
		const double z = luaL_checknumber(L, 3);
		if (z != _z) {
			_z = z;
			onZChanged();
		}
	} else if (key == "keyZ") {
		_nodeState->keyZ = luaL_checknumber(L, 3);
	} else if (key == "localAlpha") {
		_nodeState->localAlpha = luaL_checknumber(L, 3);
	} else if (key == "localEnabled") {
		_nodeState->localEnabled = lua_toboolean(L, 3) != 0;
	} else if (key == "localVisible") {
		_nodeState->localVisible = lua_toboolean(L, 3) != 0;
	} else if (key == "localPaused") {
		_nodeState->localPaused = lua_toboolean(L, 3) != 0;
	} else if (key == "localTransform") {
		if (_stages && !_stages->isEmpty()) {
			luaL_error(L, "Cannot set the localTransform of an AR that has stages");
		}

		const ScriptMatrix &transform = checkMatrix(L, 3);
		if (transform != _transform) {
			_transform = transform;
			onLocalTransformChanged();
		}
	} else if (_stages && _stages->setField(L, key)) {
		return true;
	} else if (key == "enabled" || key == "visible" || key == "paused" || key == "alpha" ||
			key == "transform" || key == "invTransform" || key == "localInvTransform" ||
			key == "blendedTransform" || key == "name" || key == "uniqueAlias" || key == "done") {
		luaL_error(L, "Cannot set field '%s' of an AR", key.c_str());
	} else {
		return false;
	}

	return true;
}

// What the records under a node take on, which are the ones of the Transformer
// that owns it. The owner counts the nodes above it in already.

bool GraphNode::isEnabled() const {
	if (_owner) {
		return _owner->isNodeEnabled();
	}

	return !_parent || _parent->isEnabled();
}

bool GraphNode::isVisible() const {
	if (_owner) {
		return _owner->isNodeVisible();
	}

	return !_parent || _parent->isVisible();
}

bool GraphNode::isPaused() const {
	if (_owner) {
		return _owner->isNodePaused();
	}

	return _parent && _parent->isPaused();
}

double GraphNode::getAlpha() const {
	if (_owner) {
		return _owner->getAlpha();
	}

	return _parent ? _parent->getAlpha() : 1.0;
}

} // End of namespace Script
} // End of namespace Nancy
