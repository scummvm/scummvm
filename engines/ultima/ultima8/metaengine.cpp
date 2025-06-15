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

#include "ultima/ultima8/metaengine.h"
#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/filesys/savegame.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

namespace Ultima {
namespace Ultima8 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_pressMethod;
	const char *_releaseMethod;
	const char *_input1;
	const char *_input2;
};

static const KeybindingRecord COMMON_KEYS[] = {
	{ ACTION_QUICKSAVE, "QUICKSAVE", "Quick Save", nullptr, nullptr, "F1", nullptr },
	{ ACTION_SAVE, "SAVE", "Save Game", nullptr, nullptr, "F5", nullptr },
	{ ACTION_LOAD, "LOAD", "Load Game", nullptr, nullptr, "F7", nullptr },
	{ ACTION_COMBAT, "COMBAT", "Combat", nullptr, nullptr, "c", "JOY_X" },
	{ ACTION_MENU, "MENU", "Game Menu", nullptr, nullptr, "ESCAPE", "JOY_Y" },
	{ ACTION_TURN_LEFT, "TURN_LEFT", "Turn Left", nullptr, nullptr, "LEFT", "KP4" },
	{ ACTION_TURN_RIGHT, "TURN_RIGHT", "Turn Right", nullptr, nullptr, "RIGHT", "KP6" },
	{ ACTION_MOVE_FORWARD, "MOVE_FORWARD", "Move Forward", nullptr, nullptr, "UP", "KP8" },
	{ ACTION_MOVE_BACK, "MOVE_BACK", "Move Back", nullptr, nullptr, "DOWN", "KP2" },
	{ ACTION_MOVE_UP, "MOVE_UP", "Move Up", nullptr, nullptr, nullptr, "JOY_UP" },
	{ ACTION_MOVE_DOWN, "MOVE_DOWN", "Move Down", nullptr, nullptr, nullptr, "JOY_DOWN" },
	{ ACTION_MOVE_LEFT, "MOVE_LEFT", "Move Left", nullptr, nullptr, nullptr, "JOY_LEFT" },
	{ ACTION_MOVE_RIGHT, "MOVE_RIGHT", "Move Right", nullptr, nullptr, nullptr, "JOY_RIGHT" },
	{ ACTION_MOVE_RUN, "MOVE_RUN", "Run", nullptr, nullptr, "LSHIFT", "JOY_RIGHT_TRIGGER" },
	{ ACTION_HIGHLIGHT_ITEMS, "HIGHLIGHT_ITEMS", "Highlight Items", nullptr, nullptr, "h", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord U8_KEYS[] = {
	{ ACTION_BEDROLL, "BEDROLL", "Bedroll", nullptr, nullptr, "b", nullptr },
	{ ACTION_BACKPACK, "BACKPACK", "Use Backpack", nullptr, nullptr, "i", "JOY_LEFT_SHOULDER" },
	{ ACTION_KEYRING, "KEYRING", "Keyring", nullptr, nullptr, "k", "JOY_RIGHT_STICK" },
	{ ACTION_MINIMAP, "MINIMAP", "Toggle Minimap", nullptr, nullptr, "m", "JOY_BACK" },
	{ ACTION_RECALL, "RECALL", "Use Recall", nullptr, nullptr, "r", nullptr },
	{ ACTION_INVENTORY, "INVENTORY", "Inventory", nullptr, nullptr, "z", "JOY_LEFT_STICK" },
	{ ACTION_CLOSE_GUMPS, "CLOSE_GUMPS", "Close Gumps", nullptr, nullptr, "BACKSPACE", nullptr },
	{ ACTION_JUMP, "JUMP", "Jump (fake both-button-click)", nullptr, nullptr, "SPACE", nullptr },
	{ ACTION_MOVE_STEP, "MOVE_STEP", "Step", nullptr, nullptr, "LCTRL", "JOY_LEFT_TRIGGER" },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CRUSADER_KEYS[] = {
	{ ACTION_NEXT_WEAPON, "NEXT_WEAPON", "Next Weapon", nullptr, nullptr, "w", "KP_MULTIPLY" },
	{ ACTION_NEXT_INVENTORY, "NEXT_INVENTORY", "Next Inventory Item", nullptr, nullptr, "i", "KP_MINUS" },
	{ ACTION_USE_INVENTORY, "USE_INVENTORY", "Use Inventroy Item", nullptr, nullptr, "u", "KP_PERIOD" },
	{ ACTION_USE_MEDIKIT, "USE_MEDIKIT", "Use Medical Kit", nullptr, nullptr, "m", nullptr },
	{ ACTION_USE_ENERGYCUBE, "USE_ENERGYCUBE", "Use Energy Cube", nullptr, nullptr, "e", nullptr },
	{ ACTION_DETONATE_BOMB, "DETONATE_BOMB", "Detonate Bomb", nullptr, nullptr, "b", nullptr },
	// TODO: The same key should be "use datalink" in no regret.
	{ ACTION_DROP_WEAPON, "DROP_WEAPON", "Drop Weapon", nullptr, nullptr, "C+d", nullptr },
	{ ACTION_SELECT_ITEMS, "SELECT_ITEM", "Select Item", nullptr, nullptr, "s", "KP_PLUS" },
	{ ACTION_USE_SELECTION, "USE_SELECTION", "Use Selection", nullptr, nullptr, "RETURN", "KP_ENTER" },
	{ ACTION_GRAB_ITEMS, "GRAB_ITEM", "Grab Items", nullptr, nullptr, "g", nullptr },
	{ ACTION_ATTACK, "ATTACK", "Attack", nullptr, nullptr, "SPACE", "KP0" },
	{ ACTION_CAMERA_AVATAR, "CAMERA_AVATAR", "Focus Camera on Silencer", nullptr, nullptr, "z", nullptr },
	{ ACTION_JUMP, "JUMP", "Jump / Roll / Crouch", nullptr, nullptr, "LCTRL", nullptr },
	{ ACTION_SHORT_JUMP, "SHORT_JUMP", "Short Jump", nullptr, nullptr, "TAB", "j" },
	{ ACTION_STEP_LEFT, "STEP_LEFT", "Sidestep Left", nullptr, nullptr, "DELETE", "KP7" },
	{ ACTION_STEP_RIGHT, "STEP_RIGHT", "Sidestep Rgiht", nullptr, nullptr, "PAGEDOWN", "KP9" },
	{ ACTION_STEP_FORWARD, "STEP_FORWARD", "Step Forward", nullptr, nullptr, "HOME", "KP_DIVIDE" },
	{ ACTION_STEP_BACK, "STEP_BACK", "Step Back", nullptr, nullptr, "END", "KP2" },
	{ ACTION_ROLL_LEFT, "ROLL_LEFT", "Roll Left", nullptr, nullptr, "INSERT", "KP1" },
	{ ACTION_ROLL_RIGHT, "ROLL_RIGHT", "Roll Right", nullptr, nullptr, "PAGEUP", "KP3" },
	{ ACTION_TOGGLE_CROUCH, "TOGGLE_CROUCH", "Toggle Crouch", nullptr, nullptr, "KP5", nullptr },
	{ ACTION_MOVE_STEP, "MOVE_STEP", "Side Step / Advance / Retreat", nullptr, nullptr, "LALT", "JOY_RIGHT_SHOULDER" },
	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ ACTION_CLIPPING, "CLIPPING", "Toggle Clipping", nullptr, nullptr, "M+INSERT", nullptr },
	{ ACTION_QUICK_MOVE_ASCEND, "ASCEND", "Ascend", nullptr, nullptr, "M+HOME", nullptr },
	{ ACTION_QUICK_MOVE_DESCEND, "DESCEND", "Descend", nullptr, nullptr, "M+END", nullptr },
	{ ACTION_QUICK_MOVE_UP, "MOVE_UP", "Move Up", nullptr, nullptr, "M+UP", nullptr },
	{ ACTION_QUICK_MOVE_DOWN, "MOVE_DOWN", "Move Down", nullptr, nullptr, "M+DOWN", nullptr },
	{ ACTION_QUICK_MOVE_LEFT, "MOVE_LEFT", "Move Left", nullptr, nullptr, "M+LEFT", nullptr },
	{ ACTION_QUICK_MOVE_RIGHT, "MOVE_RIGHT", "Move Right", nullptr, nullptr, "M+RIGHT", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

#ifndef RELEASE_BUILD
static const KeybindingRecord DEBUG_KEYS[] = {
	{ ACTION_TOGGLE_TOUCHING, "TOUCHING", "Show Touching Items", nullptr, nullptr, "C+h", nullptr },
	{ ACTION_TOGGLE_PAINT, "TOGGLE_PAINT", "Show Editor Items", nullptr, nullptr, "F9", nullptr },
	{ ACTION_DEC_SORT_ORDER, "DEC_SORT_ORDER", "Decrement Map Sort Order", nullptr, nullptr, "LEFTBRACKET", nullptr },
	{ ACTION_INC_SORT_ORDER, "INC_SORT_ORDER", "Increment Map Sort Order", nullptr, nullptr, "RIGHTBRACKET", nullptr },
	{ ACTION_FRAME_BY_FRAME, "FRAME_BY_FRAME", "Toggle Frame By Frame", nullptr, nullptr, "F12", nullptr },
	{ ACTION_ADVANCE_FRAME, "ADVANCE_FRAME", "Advance Frame", nullptr, nullptr, "f", nullptr },
	{ ACTION_TOGGLE_STASIS, "TOGGLE_STASIS", "Toggle Avatar In Stasis", nullptr, nullptr, "F10", nullptr },
	{ ACTION_SHAPE_VIEWER, "SHAPE_VIEWER", "Show Shape Viewer", nullptr, nullptr, "F11", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};
#endif

Common::KeymapArray MetaEngine::initKeymaps(const Common::String &gameId) {
	Common::KeymapArray keymapArray;

	// Core keymaps
	Common::U32String desc = (gameId == "ultima8" ? _("Ultima VIII") : _("Crusader"));

	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, gameId, desc);
	keymapArray.push_back(keyMap);

	Common::Action *act;

	act = new Common::Action(Common::kStandardActionLeftClick, _("Interact via Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionRightClick, _("Interact via Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keyMap->addAction(act);

	for (const KeybindingRecord *r = COMMON_KEYS; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}

	// Game specific keymaps
	const KeybindingRecord *game_keys = (gameId.equals("ultima8") ? U8_KEYS : CRUSADER_KEYS);
	for (const KeybindingRecord *r = game_keys; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}


	// Cheat keymaps
	keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_cheats", _("Ultima VIII Cheats"));
	keymapArray.push_back(keyMap);

	for (const KeybindingRecord *r = CHEAT_KEYS; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}

#ifndef RELEASE_BUILD
	// Debug keymaps
	keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_debug", _("Ultima VIII Debug"));
	keymapArray.push_back(keyMap);

	for (const KeybindingRecord *r = DEBUG_KEYS; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}
#endif

	return keymapArray;
}

Common::String MetaEngine::getMethod(KeybindingAction keyAction, bool isPress) {
#ifdef RELEASE_BUILD
	const KeybindingRecord *KEY_ARRAYS[] = { COMMON_KEYS, U8_KEYS, CRUSADER_KEYS, CHEAT_KEYS, nullptr };
#else
	const KeybindingRecord *KEY_ARRAYS[] = { COMMON_KEYS, U8_KEYS, CRUSADER_KEYS, CHEAT_KEYS, DEBUG_KEYS, nullptr };
#endif

	for (const KeybindingRecord **arr = KEY_ARRAYS; *arr; ++arr) {
		for (const KeybindingRecord *r = *arr; r->_id; ++r) {
			if (r->_action == keyAction) {
				return Common::String(isPress ? r->_pressMethod : r->_releaseMethod);
			}
		}
	}

	return Common::String();
}

bool MetaEngine::querySaveMetaInfos(const Common::String &filename, SaveStateDescriptor& desc) {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(filename));

	if (f) {
		SavegameReader sg(f.get(), true);
		desc.setDescription(sg.getDescription());
		return sg.isValid();
	}

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
