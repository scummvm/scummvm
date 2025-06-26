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
	const char *_input1;
	const char *_input2;
};

static const KeybindingRecord COMMON_KEYS[] = {
	{ ACTION_QUICKSAVE, "QUICKSAVE", _s("Quick save"), "F1", nullptr },
	{ ACTION_SAVE, "SAVE", _s("Save game"), "F5", nullptr },
	{ ACTION_LOAD, "LOAD", _s("Load game"), "F7", nullptr },
	{ ACTION_COMBAT, "COMBAT", _s("Draw weapon / Combat"), "c", "JOY_X" },
	{ ACTION_MENU, "MENU", _s("Game menu"), "ESCAPE", "JOY_Y" },
	{ ACTION_TURN_LEFT, "TURN_LEFT", _s("Turn left"), "LEFT", "KP4" },
	{ ACTION_TURN_RIGHT, "TURN_RIGHT", _s("Turn right"), "RIGHT", "KP6" },
	{ ACTION_MOVE_FORWARD, "MOVE_FORWARD", _s("Move forward"), "UP", "KP8" },
	{ ACTION_MOVE_BACK, "MOVE_BACK", _s("Move back"), "DOWN", "KP2" },
	{ ACTION_MOVE_UP, "MOVE_UP", _s("Move up"), nullptr, "JOY_UP" },
	{ ACTION_MOVE_DOWN, "MOVE_DOWN", _s("Move down"), nullptr, "JOY_DOWN" },
	{ ACTION_MOVE_LEFT, "MOVE_LEFT", _s("Move left"), nullptr, "JOY_LEFT" },
	{ ACTION_MOVE_RIGHT, "MOVE_RIGHT", _s("Move right"), nullptr, "JOY_RIGHT" },
	{ ACTION_MOVE_RUN, "MOVE_RUN", _s("Run"), "LSHIFT", "JOY_RIGHT_TRIGGER" },
	{ ACTION_HIGHLIGHT_ITEMS, "HIGHLIGHT_ITEMS", _s("Highlight items"), "h", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord U8_KEYS[] = {
	{ ACTION_BEDROLL, "BEDROLL", _s("Use bedroll"), "b", nullptr },
	{ ACTION_BACKPACK, "BACKPACK", _s("Open backpack"), "i", "JOY_LEFT_SHOULDER" },
	{ ACTION_KEYRING, "KEYRING", _s("Use keyring"), "k", "JOY_RIGHT_STICK" },
	{ ACTION_MINIMAP, "MINIMAP", _s("Open minimap"), "m", "JOY_BACK" },
	{ ACTION_RECALL, "RECALL", _s("Use recall"), "r", nullptr },
	{ ACTION_INVENTORY, "INVENTORY", _s("Open inventory & statistics"), "z", "JOY_LEFT_STICK" },
	// I18N: gump is Graphical User Menu Pop-up
	{ ACTION_CLOSE_GUMPS, "CLOSE_GUMPS", _s("Close all displays"), "BACKSPACE", nullptr },
	{ ACTION_JUMP, "JUMP", _s("Jump (fake both-button-click)"), "SPACE", nullptr },
	{ ACTION_MOVE_STEP, "MOVE_STEP", _s("Careful step"), "LCTRL", "JOY_LEFT_TRIGGER" },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CRUSADER_KEYS[] = {
	{ ACTION_NEXT_WEAPON, "NEXT_WEAPON", _s("Cycle through weapon inventory"), "w", "KP_MULTIPLY" },
	{ ACTION_NEXT_INVENTORY, "NEXT_INVENTORY", _s("Cycle through item inventory"), "i", "KP_MINUS" },
	{ ACTION_USE_INVENTORY, "USE_INVENTORY", _s("Use inventory item"), "u", "KP_PERIOD" },
	{ ACTION_USE_MEDIKIT, "USE_MEDIKIT", _s("Use medikit"), "m", nullptr },
	{ ACTION_USE_ENERGYCUBE, "USE_ENERGYCUBE", _s("Use energy cube"), "e", nullptr },
	{ ACTION_DETONATE_BOMB, "DETONATE_BOMB", _s("Detonate bomb"), "b", nullptr },
	// TODO: The same key should be "use datalink" in no regret.
	{ ACTION_DROP_WEAPON, "DROP_WEAPON", _s("Drop weapon"), "C+d", nullptr },
	{ ACTION_SELECT_ITEMS, "SELECT_ITEM", _s("Search / Select item"), "s", "KP_PLUS" },
	{ ACTION_USE_SELECTION, "USE_SELECTION", _s("Use selection"), "RETURN", "KP_ENTER" },
	{ ACTION_GRAB_ITEMS, "GRAB_ITEM", _s("Grab items"), "g", nullptr },
	{ ACTION_ATTACK, "ATTACK", _s("Fire weapon"), "SPACE", "KP0" },
	{ ACTION_CAMERA_AVATAR, "CAMERA_AVATAR", _s("Center screen on Silencer"), "z", nullptr },
	{ ACTION_JUMP, "JUMP", _s("Jump / Roll / Crouch"), "LCTRL", nullptr },
	{ ACTION_SHORT_JUMP, "SHORT_JUMP", _s("Short jump"), "TAB", "j" },
	{ ACTION_STEP_LEFT, "STEP_LEFT", _s("Sidestep left"), "DELETE", "KP7" },
	{ ACTION_STEP_RIGHT, "STEP_RIGHT", _s("Sidestep right"), "PAGEDOWN", "KP9" },
	{ ACTION_STEP_FORWARD, "STEP_FORWARD", _s("Step forward"), "HOME", "KP_DIVIDE" },
	{ ACTION_STEP_BACK, "STEP_BACK", _s("Step back"), "END", "KP2" },
	{ ACTION_ROLL_LEFT, "ROLL_LEFT", _s("Roll left"), "INSERT", "KP1" },
	{ ACTION_ROLL_RIGHT, "ROLL_RIGHT", _s("Roll right"), "PAGEUP", "KP3" },
	{ ACTION_TOGGLE_CROUCH, "TOGGLE_CROUCH", _s("Toggle crouch"), "KP5", nullptr },
	{ ACTION_MOVE_STEP, "MOVE_STEP", _s("Side step / Advance / Retreat"), "LALT", "JOY_RIGHT_SHOULDER" },
	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ ACTION_CLIPPING, "CLIPPING", _s("Toggle clipping"), "M+INSERT", nullptr },
	{ ACTION_MOVE_ASCEND, "ASCEND", _s("Ascend"), "M+HOME", nullptr },
	{ ACTION_MOVE_DESCEND, "DESCEND", _s("Descend"), "M+END", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr }
};

#ifndef RELEASE_BUILD
static const KeybindingRecord DEBUG_KEYS[] = {
	{ ACTION_TOGGLE_TOUCHING, "TOUCHING", _s("Show touching items"), "C+h", nullptr },
	{ ACTION_TOGGLE_PAINT, "TOGGLE_PAINT", _s("Show editor items"), "F9", nullptr },
	{ ACTION_DEC_SORT_ORDER, "DEC_SORT_ORDER", _s("Decrement map sort order"), "LEFTBRACKET", nullptr },
	{ ACTION_INC_SORT_ORDER, "INC_SORT_ORDER", _s("Increment map sort order"), "RIGHTBRACKET", nullptr },
	{ ACTION_FRAME_BY_FRAME, "FRAME_BY_FRAME", _s("Toggle frame by frame"), "F12", nullptr },
	{ ACTION_ADVANCE_FRAME, "ADVANCE_FRAME", _s("Advance frame"), "f", nullptr },
	{ ACTION_TOGGLE_STASIS, "TOGGLE_STASIS", _s("Toggle Avatar in stasis"), "F10", nullptr },
	{ ACTION_SHAPE_VIEWER, "SHAPE_VIEWER", _s("Show shape viewer"), "F11", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr }
};
#endif

Common::KeymapArray MetaEngine::initKeymaps(const Common::String &gameId) {
	Common::KeymapArray keymapArray;

	// Core keymaps
	Common::U32String desc = (gameId == "ultima8" ? _("Ultima VIII") : _("Crusader"));

	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, gameId, desc);
	keymapArray.push_back(keyMap);

	Common::Action *act;

	act = new Common::Action(Common::kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionRightClick, _("Right click"));
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
