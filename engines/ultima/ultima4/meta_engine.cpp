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

#include "ultima/ultima4/meta_engine.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/ultima4.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"

namespace Ultima {
namespace Ultima4 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_method;
	const char *_key;
	const char *_joy;
};

static const KeybindingRecord KEYS[] = {
	{ KEYBIND_UP, "UP", "Up", "move up", "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", "Down", "move down", "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", "Left", "move left", "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", "Right", "move right", "RIGHT", nullptr },
	{ KEYBIND_ATTACK, "ATTACK", "Attack", "attack", "a", nullptr },
	{ KEYBIND_BOARD, "BOARD", "Board", "board", "b", nullptr },
	{ KEYBIND_CAST, "CAST", "Cast", "cast", "c", nullptr },
	{ KEYBIND_ENTER, "ENTER", "Enter", "enter", "e", nullptr },
	{ KEYBIND_FIRE, "FIRE", "Fire", "fire", "f", nullptr },
	{ KEYBIND_GET, "GET", "Get Chest", "get", "g", nullptr },
	{ KEYBIND_HOLE_UP, "HOLE-UP", "Hole Up", "hole", "h", nullptr },
	{ KEYBIND_JIMMY, "JIMMY", "Jimmy", "jimmy", "j", nullptr },
	{ KEYBIND_IGNITE, "IGNITE", "Ignite", "ignite", "i", nullptr },
	{ KEYBIND_LOCATE, "LOCATE", "Locate Position", "locate", "l", nullptr },

	{ KEYBIND_PASS, "PASS", "Pass", "pass", "SPACE", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};


Common::KeymapArray MetaEngine::initKeymaps() {
	Common::KeymapArray keymapArray;

	// Core keymaps
	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima4", _("Ultima IV"));
	keymapArray.push_back(keyMap);

	Common::Action *act;

	act = new Common::Action("LCLK", _("Interact via Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keyMap->addAction(act);

	act = new Common::Action("RCLK", _("Interact via Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keyMap->addAction(act);

	for (const KeybindingRecord *r = KEYS; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		act->addDefaultInputMapping(r->_key);
		if (r->_joy)
			act->addDefaultInputMapping(r->_joy);
		keyMap->addAction(act);
	}

	return keymapArray;
}

void MetaEngine::setKeybindingsActive(bool isActive) {
	g_engine->getEventManager()->getKeymapper()->setEnabled(isActive);
}


void MetaEngine::executeAction(KeybindingAction keyAction) {
	Common::String methodName = getMethod(keyAction);
	if (!methodName.empty())
		g_debugger->executeCommand(methodName);
}

Common::String MetaEngine::getMethod(KeybindingAction keyAction) {
	const KeybindingRecord *KEY_ARRAYS[] = { KEYS, CHEAT_KEYS, nullptr };

	for (const KeybindingRecord **arr = KEY_ARRAYS; *arr; ++arr) {
		for (const KeybindingRecord *r = *arr; r->_id; ++r) {
			if (r->_action == keyAction)
				return r->_method;
		}
	}

	return Common::String();
}

} // End of namespace Ultima8
} // End of namespace Ultima
