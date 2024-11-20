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

#include "ultima/nuvie/metaengine.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"
#include "ultima/nuvie/keybinding/keys_enum.h"

namespace Ultima {
namespace Nuvie {

void MetaEngine::listSaves(SaveStateList &saveList) {
	// Check whether there's an entry for the original save slot
	for (const SaveStateDescriptor &state : saveList) {
		if (state.getSaveSlot() == ORIGINAL_SAVE_SLOT)
			return;
	}

	// Add in an entry
	SaveStateDescriptor desc;
	desc.setSaveSlot(ORIGINAL_SAVE_SLOT);
	desc.setDescription(_("Original Save"));
	saveList.push_back(desc);

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
}

struct NuvieActionDescription {
	const char *_id;
	const char *_desc;
	const char *_key1;
	const char *_key2;
};

//
// For associated actions for each of these IDs, see NuvieActions in
// keybinding/keys.cpp
//
static const NuvieActionDescription NuvieActionDescriptions[] = {
	{ "WALK_WEST", "Walk west", "LEFT", "KP4" },
	{ "WALK_EAST", "Walk east", "RIGHT", "KP6" },
	{ "WALK_NORTH", "Walk north", "UP", "KP8" },
	{ "WALK_SOUTH", "Walk south", "DOWN", "KP2" },
	{ "WALK_NORTH_EAST", "Walk north-east", "KP9", nullptr },
	{ "WALK_SOUTH_EAST", "Walk south-east", "KP3", nullptr },
	{ "WALK_NORTH_WEST", "Walk north-west", "KP7", nullptr },
	{ "WALK_SOUTH_WEST", "Walk south-west", "KP1", nullptr },
	{ "LOOK", "Look", "l", nullptr },
	{ "TALK", "Talk", "t", nullptr },
	{ "USE", "Use", "u", nullptr },
	{ "GET", "Get", "g", nullptr },
	{ "MOVE", "Move", "m", nullptr },
	{ "DROP", "Drop", "d", nullptr },
	{ "TOGGLE_COMBAT", "Toggle combat", "b", nullptr },
	{ "ATTACK", "Attack", "a", nullptr },
	{ "MULTI_USE", "Multi-use", "SEMICOLON", nullptr },
	{ "SELECT_COMMAND_BAR", "Select Command Bar", nullptr, nullptr },
	{ "NEW_COMMAND_BAR", "Select New Command Bar", "COMMA", nullptr },
	{ "DOLL_GUMP", "Doll gump", "i", nullptr },
	{ "PREVIOUS_PARTY_MEMBER", "Previous party member", "MINUS", "KP_MINUS" },
	{ "NEXT_PARTY_MEMBER", "Next party member", "PLUS", "KP_PLUS" },
	{ "HOME_KEY", "Home key", "HOME", nullptr },
	{ "END_KEY", "End key", "END", nullptr },
	{ "TOGGLE_VIEW", "Toggle between inventory and actor view", "KP_ASTERISK", "ASTERISK" },
	{ "PARTY_VIEW", "Party view", "F10", "SLASH" },
	{ "PARTY_MODE", "Party mode", "0", nullptr },
	{ "SAVE_MENU", "Save menu", "s", nullptr },
	{ "LOAD_LATEST_SAVE", "Load latest save", "C+l", "C+r" },
	{ "QUICK_SAVE", "Quick save", nullptr, nullptr },
	{ "QUICK_LOAD", "Quick load", nullptr, nullptr },
	{ "QUIT", "Quit", "q", "C+q" },
	//{ "QUIT_NO_DIALOG", "Quit without confirmation", "M+q", nullptr },
	{ "GAME_MENU_DIALOG", "Show game menu; Cancel action if in middle of action", "ESCAPE", nullptr },
	//{ "TOGGLE_FULLSCREEN", "Toggle Fullscreen", "M+RETURN", nullptr },
	{ "TOGGLE_CURSOR", "Toggle Cursor", "TAB", nullptr },
	{ "TOGGLE_COMBAT_STRATEGY", "Toggle combat strategy", "BACKQUOTE", nullptr },
	{ "TOGGLE_FPS_DISPLAY", "Toggle frames per second display", "C+PLUS", nullptr },
	{ "TOGGLE_AUDIO", "Toggle audio", "C+a", nullptr },
	{ "TOGGLE_MUSIC", "Toggle music", "C+m", nullptr },
	{ "TOGGLE_SFX", "Toggle sfx", "C+s", nullptr },
	{ "TOGGLE_ORIGINAL_STYLE_COMMAND_BAR", "Show/hide original style command bar", nullptr, nullptr },
	{ "DO_ACTION", "Do action", "RETURN", "KP_ENTER" },
	{ "CANCEL_ACTION", "Cancel action", "SPACE", nullptr },
	{ "MSG_SCROLL_UP", "Msg scroll up", "PAGEUP", nullptr },
	{ "MSG_SCROLL_DOWN", "Msg scroll down", "PAGEDOWN", nullptr },
	{ "SHOW_KEYS", "Show keys", "h", nullptr },
	{ "DECREASE_DEBUG", "Decrease debug", "C+d", nullptr },
	{ "INCREASE_DEBUG", "Increase debug", "C+i", nullptr },
	{ "CLOSE_GUMPS", "Close gumps", "z", nullptr },
	//{ "USE_ITEM", "Use item", nullptr, nullptr }, // TODO: this takes an item no parameter
	{ "TOGGLE_ALT_CODE_MODE", "Enter ALT code (hold)", "LALT", nullptr },
	{ "ALT_CODE_0", "ALT Code 0", "A+KP0", "A+0" },
	{ "ALT_CODE_1", "ALT Code 1", "A+KP1", "A+1" },
	{ "ALT_CODE_2", "ALT Code 2", "A+KP2", "A+2" },
	{ "ALT_CODE_3", "ALT Code 3", "A+KP3", "A+3" },
	{ "ALT_CODE_4", "ALT Code 4", "A+KP4", "A+4" },
	{ "ALT_CODE_5", "ALT Code 5", "A+KP5", "A+5" },
	{ "ALT_CODE_6", "ALT Code 6", "A+KP6", "A+6" },
	{ "ALT_CODE_7", "ALT Code 7", "A+KP7", "A+7" },
	{ "ALT_CODE_8", "ALT Code 8", "A+KP8", "A+8" },
	{ "ALT_CODE_9", "ALT Code 9", "A+KP9", "A+9" },
};

static const NuvieActionDescription CheatKeyDescriptions[] = {
	{ "ASSET_VIEWER", "Open the asset viewer", nullptr, nullptr },
	{ "SHOW_EGGS", "Show eggs", "e", nullptr },
	{ "TOGGLE_HACKMOVE", "Toggle hack move", "M+h", nullptr },
	{ "TOGGLE_EGG_SPAWN", "Toggle egg spawn", "M+C+e", nullptr },
	{ "TOGGLE_NO_DARKNESS", "Toggle no darkness", "M+i", nullptr },
	{ "TOGGLE_PICKPOCKET_MODE", "Toggle pickpocket mode", "M+C+p", nullptr },
	{ "TOGGLE_GOD_MODE", "Toggle god mode", "M+C+g", nullptr },
	{ "TOGGLE_ETHEREAL", "Toggle ethereal mode", "M+e", nullptr},
	{ "TOGGLE_X_RAY", "Toggle X-ray mode", "x", nullptr },
	{ "HEAL_PARTY", "Heal party", "M+C+h", nullptr },
	{ "TELEPORT_TO_CURSOR", "Teleport to cursor", "C+t", nullptr },
	{ "TOGGLE_CHEATS", "Toggle cheats", "C+c", nullptr },
};

static const NuvieActionDescription PerPartyMemberActionDescriptions[] {
	{ "SOLO_MODE_1", "Solo mode as Avatar", "1", nullptr },
	{ "SOLO_MODE_2", "Solo mode as party member 2", "2", nullptr },
	{ "SOLO_MODE_3", "Solo mode as party member 3", "3", nullptr },
	{ "SOLO_MODE_4", "Solo mode as party member 4", "4", nullptr },
	{ "SOLO_MODE_5", "Solo mode as party member 5", "5", nullptr },
	{ "SOLO_MODE_6", "Solo mode as party member 6", "6", nullptr },
	{ "SOLO_MODE_7", "Solo mode as party member 7", "7", nullptr },
	{ "SOLO_MODE_8", "Solo mode as party member 8", "8", nullptr },
	{ "SOLO_MODE_9", "Solo mode as party member 9", "9", nullptr },
	{ "SHOW_STATS_1", "Show stats for Avatar", "C+1", nullptr },
	{ "SHOW_STATS_2", "Show stats for party member 2", "C+2", nullptr },
	{ "SHOW_STATS_3", "Show stats for party member 3", "C+3", nullptr },
	{ "SHOW_STATS_4", "Show stats for party member 4", "C+4", nullptr },
	{ "SHOW_STATS_5", "Show stats for party member 5", "C+5", nullptr },
	{ "SHOW_STATS_6", "Show stats for party member 6", "C+6", nullptr },
	{ "SHOW_STATS_7", "Show stats for party member 7", "C+7", nullptr },
	{ "SHOW_STATS_8", "Show stats for party member 8", "C+8", nullptr },
	{ "SHOW_STATS_9", "Show stats for party member 9", "C+9", nullptr },
	{ "INVENTORY_1", "Show inventory for Avatar", "F1", nullptr },
	{ "INVENTORY_2", "Show inventory for party member 2", "F2", nullptr },
	{ "INVENTORY_3", "Show inventory for party member 3", "F3", nullptr },
	{ "INVENTORY_4", "Show inventory for party member 4", "F4", nullptr },
	{ "INVENTORY_5", "Show inventory for party member 5", "F5", nullptr },
	{ "INVENTORY_6", "Show inventory for party member 6", "F6", nullptr },
	{ "INVENTORY_7", "Show inventory for party member 7", "F7", nullptr },
	{ "INVENTORY_8", "Show inventory for party member 8", "F8", nullptr },
	{ "INVENTORY_9", "Show inventory for party member 9", "F9", nullptr },
	{ "DOLL_GUMP_1", "Show doll gump for Avatar", "M+F1", nullptr },
	{ "DOLL_GUMP_2", "Show doll gump for party member 2", "M+F2", nullptr },
	{ "DOLL_GUMP_3", "Show doll gump for party member 3", "M+F3", nullptr },
	{ "DOLL_GUMP_4", "Show doll gump for party member 4", "M+F4", nullptr },
	{ "DOLL_GUMP_5", "Show doll gump for party member 5", "M+F5", nullptr },
	{ "DOLL_GUMP_6", "Show doll gump for party member 6", "M+F6", nullptr },
	{ "DOLL_GUMP_7", "Show doll gump for party member 7", "M+F7", nullptr },
	{ "DOLL_GUMP_8", "Show doll gump for party member 8", "M+F8", nullptr },
	{ "DOLL_GUMP_9", "Show doll gump for party member 9", "M+F9", nullptr },
};

static const NuvieActionDescription U6ActionDescriptions[] = {
	{ "CAST", "Cast", "c", nullptr },
	{ "REST", "Rest", "r", nullptr },
	{ "TOGGLE_UNLIMITED_CASTING", "Toggle unlimited casting", "M+w", nullptr },
};

static const NuvieActionDescription SEActionDescriptions[] = {
	{ "REST", "Rest", "r", nullptr },
	{ "TOGGLE_UNLIMITED_CASTING", "Toggle unlimited casting", "M+w", nullptr },
};


static Common::Action *actionDescriptionFromNuvieAction(const NuvieActionDescription &n) {
	Common::String idstr(n._id);
	Common::Action *act = new Common::Action(n._id, _(n._desc));
	act->setCustomEngineActionEvent(idstr.hash());
	if (n._key1)
		act->addDefaultInputMapping(n._key1);
	if (n._key2)
		act->addDefaultInputMapping(n._key2);
	// Allow WALK operations to repeat so holding the key lets you keep moving
	// TODO: This would be nice to handle manually inside the event loop so
	// non-keyboard inputs also work and we can control the repeat rate.
	if (strncmp(n._id, "WALK", 4) == 0)
		act->allowKbdRepeats();
	return act;
}

Common::KeymapArray MetaEngine::initKeymaps(const Common::String &gameId) {
	Common::KeymapArray keymapArray;

	// Core keymaps
	Common::U32String desc;
	if (gameId == "ultima6" || gameId == "ultima6_enh")
		desc = _("Ultima VI");
	else if (gameId == "martiandreams" || gameId == "martiandreams_enh")
		desc = _("Martian Dreams");
	else
		desc = _("The Savage Empire");

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

	for (int i = 0; i < ARRAYSIZE(NuvieActionDescriptions); i++)
		keyMap->addAction(actionDescriptionFromNuvieAction(NuvieActionDescriptions[i]));

	if (gameId == "ultima6" || gameId == "ultima6_enh") {
		for (int i = 0; i < ARRAYSIZE(U6ActionDescriptions); i++)
			keyMap->addAction(actionDescriptionFromNuvieAction(U6ActionDescriptions[i]));
	} else if (gameId == "savageempire" || gameId == "savageempire_enh") {
		for (int i = 0; i < ARRAYSIZE(SEActionDescriptions); i++)
			keyMap->addAction(actionDescriptionFromNuvieAction(SEActionDescriptions[i]));
	}

	for (int i = 0; i < ARRAYSIZE(PerPartyMemberActionDescriptions); i++)
		keyMap->addAction(actionDescriptionFromNuvieAction(PerPartyMemberActionDescriptions[i]));

	for (int i = 0; i < ARRAYSIZE(CheatKeyDescriptions); ++i)
		keyMap->addAction(actionDescriptionFromNuvieAction(CheatKeyDescriptions[i]));

	return keymapArray;
}

} // End of namespace Nuvie
} // End of namespace Ultima
