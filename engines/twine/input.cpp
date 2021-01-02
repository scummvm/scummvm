/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/input.h"
#include "backends/keymapper/keymapper.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "twine/scene/actor.h"
#include "twine/twine.h"

namespace TwinE {

const char *mainKeyMapId = "mainKeyMap";
const char *uiKeyMapId = "uiKeyMap";
const char *cutsceneKeyMapId = "cutsceneKeyMap";
const char *holomapKeyMapId = "holomapKeyMap";

ScopedKeyMap::ScopedKeyMap(TwinEEngine* engine, const char *id) : _engine(engine) {
	_changed = _engine->_input->enableAdditionalKeyMap(id, true);
	_keymap = id;
}

ScopedKeyMap::~ScopedKeyMap() {
	if (_changed) {
		_engine->_input->enableAdditionalKeyMap(_keymap.c_str(), false);
	}
}

Input::Input(TwinEEngine *engine) : _engine(engine) {}

bool Input::isActionActive(TwinEActionType actionType, bool onlyFirstTime) const {
	if (onlyFirstTime) {
		return actionStates[actionType] == 1;
	}
	return actionStates[actionType] > 0;
}

bool Input::toggleActionIfActive(TwinEActionType actionType) {
	if (actionStates[actionType] > 0) {
		actionStates[actionType] = 0;
		return true;
	}
	return false;
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
		const Common::String& keymapId = keymap->getId();
		if (keymapId == mainKeyMapId || keymapId == uiKeyMapId || keymapId == cutsceneKeyMapId) {
			keymap->setEnabled(keymapId == id);
		}
	}
	_currentKeyMap = id;
	debug("enable keymap %s", id);
}

void Input::processCustomEngineEventStart(const Common::Event &event) {
	if (!_engine->cfgfile.Debug) {
		switch (event.customType) {
		case TwinEActionType::DebugGridCameraPressUp:
		case TwinEActionType::DebugGridCameraPressDown:
		case TwinEActionType::DebugGridCameraPressLeft:
		case TwinEActionType::DebugGridCameraPressRight:
		case TwinEActionType::DebugMenu:
		case TwinEActionType::DebugMenuActivate:
		case TwinEActionType::NextRoom:
		case TwinEActionType::PreviousRoom:
		case TwinEActionType::ApplyCellingGrid:
		case TwinEActionType::IncreaseCellingGridIndex:
		case TwinEActionType::DecreaseCellingGridIndex:
			break;
		default:
			actionStates[event.customType] = 1 + event.kbdRepeat;
			break;
		}
	} else {
		actionStates[event.customType] = 1 + event.kbdRepeat;
	}
}

void Input::processCustomEngineEventEnd(const Common::Event &event) {
	actionStates[event.customType] = 0;
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

bool Input::isMouseHovering(const Common::Rect &rect) const {
	if (!_engine->cfgfile.Mouse) {
		return false;
	}
	const Common::Point &point = getMousePositions();
	return rect.contains(point);
}

} // namespace TwinE
