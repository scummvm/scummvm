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

#include "twine/input.h"
#include "backends/keymapper/keymapper.h"
#include "common/events.h"
#include "common/system.h"
#include "twine/scene/actor.h"
#include "twine/twine.h"

namespace TwinE {

const char *mainKeyMapId = "mainKeyMap";
const char *uiKeyMapId = "uiKeyMap";
const char *cutsceneKeyMapId = "cutsceneKeyMap";
const char *holomapKeyMapId = "holomapKeyMap";

ScopedKeyMap::ScopedKeyMap(TwinEEngine *engine, const char *id) : _engine(engine) {
	_changed = _engine->_input->enableAdditionalKeyMap(id, true);
	_keymap = id;
}

ScopedKeyMap::~ScopedKeyMap() {
	if (_changed) {
		_engine->_input->enableAdditionalKeyMap(_keymap.c_str(), false);
	}
}

Input::Input(TwinEEngine *engine) : _engine(engine) {
	resetLastHoveredMousePosition();
}

bool Input::isActionActive(TwinEActionType actionType, bool onlyFirstTime) const {
	if (onlyFirstTime) {
		return _actionStates[actionType] == 1;
	}
	return _actionStates[actionType] > 0;
}

bool Input::toggleActionIfActive(TwinEActionType actionType) {
	if (_actionStates[actionType] > 0) {
		_actionStates[actionType] = 0;
		return true;
	}
	return false;
}

void Input::resetActionStates() {
	for (int i = 0; i < TwinEActionType::Max; ++i) {
		_actionStates[i] = false;
	}
}

bool Input::toggleAbortAction() {
	bool abortState = false;
	abortState |= toggleActionIfActive(TwinEActionType::CutsceneAbort);
	abortState |= toggleActionIfActive(TwinEActionType::UIAbort);
	abortState |= toggleActionIfActive(TwinEActionType::Escape);
	abortState |= toggleActionIfActive(TwinEActionType::HolomapAbort);
	return abortState;
}

bool Input::isQuickBehaviourActionActive() const {
	return isActionActive(TwinEActionType::QuickBehaviourNormal) || isActionActive(TwinEActionType::QuickBehaviourAthletic) || isActionActive(TwinEActionType::QuickBehaviourAggressive) || isActionActive(TwinEActionType::QuickBehaviourDiscreet);
}

bool Input::isMoveOrTurnActionActive() const {
	return isActionActive(TwinEActionType::TurnLeft) || isActionActive(TwinEActionType::TurnRight) || isActionActive(TwinEActionType::MoveBackward) || isActionActive(TwinEActionType::MoveForward);
}

bool Input::isHeroActionActive() const {
	return isActionActive(TwinEActionType::ExecuteBehaviourAction) || isActionActive(TwinEActionType::SpecialAction);
}

bool Input::resetHeroActions() {
	return toggleActionIfActive(TwinEActionType::ExecuteBehaviourAction) || toggleActionIfActive(TwinEActionType::SpecialAction);
}

bool Input::enableAdditionalKeyMap(const char *id, bool enable) {
	Common::Keymapper *keymapper = g_system->getEventManager()->getKeymapper();
	Common::Keymap *keymap = keymapper->getKeymap(id);
	if (keymap == nullptr) {
		return false;
	}
	const bool changed = keymap->isEnabled() != enable;
	keymap->setEnabled(enable);
	return changed;
}

void Input::enableKeyMap(const char *id) {
	if (_currentKeyMap == id) {
		return;
	}

	Common::Keymapper *keymapper = g_system->getEventManager()->getKeymapper();
	const Common::KeymapArray &keymaps = keymapper->getKeymaps();
	for (Common::Keymap *keymap : keymaps) {
		const Common::String &keymapId = keymap->getId();
		if (keymapId == mainKeyMapId || keymapId == uiKeyMapId || keymapId == cutsceneKeyMapId || keymapId == holomapKeyMapId) {
			keymap->setEnabled(keymapId == id);
		}
	}
	_currentKeyMap = id;
	debugC(1, TwinE::kDebugInput, "enable keymap %s", id);
}

void Input::processCustomEngineEventStart(const Common::Event &event) {
	_actionStates[event.customType] = 1 + event.kbdRepeat;
	debugC(2, TwinE::kDebugInput, "twine custom event type start: %i", event.customType);
}

void Input::processCustomEngineEventEnd(const Common::Event &event) {
	_actionStates[event.customType] = 0;
	debugC(2, TwinE::kDebugInput, "twine custom event type end: %i", event.customType);
}

void Input::readKeys() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			processCustomEngineEventEnd(event);
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			processCustomEngineEventStart(event);
			break;
		default:
			break;
		}
	}
}

Common::Point Input::getMousePositions() const {
	return g_system->getEventManager()->getMousePos();
}

bool Input::isMouseHovering(const Common::Rect &rect, bool onlyIfMoved) {
	if (!_engine->_cfgfile.Mouse) {
		return false;
	}
	const Common::Point &point = getMousePositions();
	if (onlyIfMoved && _lastMousePos == point) {
		return false;
	}
	if (rect.contains(point)) {
		_lastMousePos = point;
		return true;
	}
	return false;
}

void Input::resetLastHoveredMousePosition() {
	_lastMousePos = Common::Point(-1, -1);
}

} // namespace TwinE
