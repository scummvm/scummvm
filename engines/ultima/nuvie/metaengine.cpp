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
	{ "WALK_WEST", _s("Walk west"), "LEFT", "KP4" },
	{ "WALK_EAST", _s("Walk east"), "RIGHT", "KP6" },
	{ "WALK_NORTH", _s("Walk north"), "UP", "KP8" },
	{ "WALK_SOUTH", _s("Walk south"), "DOWN", "KP2" },
	{ "WALK_NORTH_EAST", _s("Walk north-east"), "KP9", nullptr },
	{ "WALK_SOUTH_EAST", _s("Walk south-east"), "KP3", nullptr },
	{ "WALK_NORTH_WEST", _s("Walk north-west"), "KP7", nullptr },
	{ "WALK_SOUTH_WEST", _s("Walk south-west"), "KP1", nullptr },
	{ "LOOK", _s("Look"), "l", nullptr },
	{ "TALK", _s("Talk"), "t", nullptr },
	{ "USE", _s("Use"), "u", nullptr },
	{ "GET", _s("Get"), "g", nullptr },
	{ "MOVE", _s("Move"), "m", nullptr },
	{ "DROP", _s("Drop"), "d", nullptr },
	{ "TOGGLE_COMBAT", _s("Toggle combat"), "b", nullptr },
	{ "ATTACK", _s("Attack"), "a", nullptr },
	{ "MULTI_USE", _s("Multi-use"), "SEMICOLON", nullptr },
	// I18N: command bar is the bar of commands at the bottom of the screen
	{ "SELECT_COMMAND_BAR", _s("Select command bar"), nullptr, nullptr },
	// I18N: command bar is the bar of commands at the bottom of the screen
	{ "NEW_COMMAND_BAR", _s("Select new command bar"), "COMMA", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP", _s("Doll gump"), "i", nullptr },
	{ "PREVIOUS_PARTY_MEMBER", _s("Previous party member"), "MINUS", "KP_MINUS" },
	{ "NEXT_PARTY_MEMBER", _s("Next party member"), "PLUS", "KP_PLUS" },
	{ "HOME_KEY", _s("Home key"), "HOME", nullptr },
	{ "END_KEY", _s("End key"), "END", nullptr },
	{ "TOGGLE_VIEW", _s("Toggle between inventory and actor view"), "KP_ASTERISK", "ASTERISK" },
	{ "PARTY_VIEW", _s("Party view"), "F10", "SLASH" },
	{ "PARTY_MODE", _s("Party mode"), "0", nullptr },
	{ "SAVE_MENU", _s("Save menu"), "s", nullptr },
	{ "LOAD_LATEST_SAVE", _s("Load latest save"), "C+l", "C+r" },
	{ "QUICK_SAVE", _s("Quick save"), nullptr, nullptr },
	{ "QUICK_LOAD", _s("Quick load"), nullptr, nullptr },
	{ "QUIT", _s("Quit"), "q", "C+q" },
	//{ "QUIT_NO_DIALOG", "_s(Quit without confirmation"), "M+q", nullptr },
	{ "GAME_MENU_DIALOG", _s("Show game menu; Cancel action if in middle of action"), "ESCAPE", nullptr },
	//{ "TOGGLE_FULLSCREEN", "_s(Toggle fullscreen"), "M+RETURN", nullptr },
	{ "TOGGLE_CURSOR", _s("Toggle Cursor"), "TAB", nullptr },
	{ "TOGGLE_COMBAT_STRATEGY", _s("Toggle combat strategy"), "BACKQUOTE", nullptr },
	{ "TOGGLE_FPS_DISPLAY", _s("Toggle frames per second display"), "C+PLUS", nullptr },
	{ "TOGGLE_AUDIO", _s("Toggle audio"), "C+a", nullptr },
	{ "TOGGLE_MUSIC", _s("Toggle music"), "C+m", nullptr },
	{ "TOGGLE_SFX", _s("Toggle sfx"), "C+s", nullptr },
	// I18N: command bar is the bar of commands at the bottom of the screen
	{ "TOGGLE_ORIGINAL_STYLE_COMMAND_BAR", _s("Toggle original style command bar"), nullptr, nullptr },
	{ "DO_ACTION", _s("Do action"), "RETURN", "KP_ENTER" },
	{ "CANCEL_ACTION", _s("Cancel action"), "SPACE", nullptr },
	{ "MSG_SCROLL_UP", _s("Msg scroll up"), "PAGEUP", nullptr },
	{ "MSG_SCROLL_DOWN", _s("Msg scroll down"), "PAGEDOWN", nullptr },
	{ "SHOW_KEYS", _s("Show keys"), "h", nullptr },
	{ "DECREASE_DEBUG", _s("Decrease debug"), "C+d", nullptr },
	{ "INCREASE_DEBUG", _s("Increase debug"), "C+i", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "CLOSE_GUMPS", _s("Close gumps"), "z", nullptr },
	//{ "USE_ITEM", _s("Use item"), nullptr, nullptr }, // TODO: this takes an item no parameter
	{ "TOGGLE_ALT_CODE_MODE", _s("Enter ALT code (hold)"), "LALT", nullptr },
	{ "ALT_CODE_0", _s("ALT code 0"), "A+KP0", "A+0" },
	{ "ALT_CODE_1", _s("ALT code 1"), "A+KP1", "A+1" },
	{ "ALT_CODE_2", _s("ALT code 2"), "A+KP2", "A+2" },
	{ "ALT_CODE_3", _s("ALT code 3"), "A+KP3", "A+3" },
	{ "ALT_CODE_4", _s("ALT code 4"), "A+KP4", "A+4" },
	{ "ALT_CODE_5", _s("ALT code 5"), "A+KP5", "A+5" },
	{ "ALT_CODE_6", _s("ALT code 6"), "A+KP6", "A+6" },
	{ "ALT_CODE_7", _s("ALT code 7"), "A+KP7", "A+7" },
	{ "ALT_CODE_8", _s("ALT code 8"), "A+KP8", "A+8" },
	{ "ALT_CODE_9", _s("ALT code 9"), "A+KP9", "A+9" },
};

static const NuvieActionDescription CheatKeyDescriptions[] = {
	{ "ASSET_VIEWER", _s("Open the asset viewer"), nullptr, nullptr },
	// I18N: Eggs are in-game hatchable objects that are normally invisible
	{ "SHOW_EGGS", _s("Show eggs"), "e", nullptr },
	{ "TOGGLE_HACKMOVE", _s("Toggle hack move"), "A+h", nullptr },
	// I18N: Eggs are in-game hatchable objects that are normally invisible
	{ "TOGGLE_EGG_SPAWN", _s("Toggle egg spawn"), "A+C+e", nullptr },
	{ "TOGGLE_NO_DARKNESS", _s("Toggle no darkness"), "A+i", nullptr },
	{ "TOGGLE_PICKPOCKET_MODE", _s("Toggle pickpocket mode"), "A+C+p", nullptr },
	{ "TOGGLE_GOD_MODE", _s("Toggle god mode"), "A+C+g", nullptr },
	{ "TOGGLE_ETHEREAL", _s("Toggle ethereal mode"), "A+e", nullptr},
	{ "TOGGLE_X_RAY", _s("Toggle X-ray mode"), "x", nullptr },
	{ "HEAL_PARTY", _s("Heal party"), "A+C+h", nullptr },
	{ "TELEPORT_TO_CURSOR", _s("Teleport to cursor"), "C+t", nullptr },
	{ "TOGGLE_CHEATS", _s("Toggle cheats"), "C+c", nullptr },
};

static const NuvieActionDescription PerPartyMemberActionDescriptions[] {
	{ "SOLO_MODE_1", _s("Solo mode as Avatar"), "1", nullptr },
	{ "SOLO_MODE_2", _s("Solo mode as party member 2"), "2", nullptr },
	{ "SOLO_MODE_3", _s("Solo mode as party member 3"), "3", nullptr },
	{ "SOLO_MODE_4", _s("Solo mode as party member 4"), "4", nullptr },
	{ "SOLO_MODE_5", _s("Solo mode as party member 5"), "5", nullptr },
	{ "SOLO_MODE_6", _s("Solo mode as party member 6"), "6", nullptr },
	{ "SOLO_MODE_7", _s("Solo mode as party member 7"), "7", nullptr },
	{ "SOLO_MODE_8", _s("Solo mode as party member 8"), "8", nullptr },
	{ "SOLO_MODE_9", _s("Solo mode as party member 9"), "9", nullptr },
	{ "SHOW_STATS_1", _s("Show stats for Avatar"), "C+1", nullptr },
	{ "SHOW_STATS_2", _s("Show stats for party member 2"), "C+2", nullptr },
	{ "SHOW_STATS_3", _s("Show stats for party member 3"), "C+3", nullptr },
	{ "SHOW_STATS_4", _s("Show stats for party member 4"), "C+4", nullptr },
	{ "SHOW_STATS_5", _s("Show stats for party member 5"), "C+5", nullptr },
	{ "SHOW_STATS_6", _s("Show stats for party member 6"), "C+6", nullptr },
	{ "SHOW_STATS_7", _s("Show stats for party member 7"), "C+7", nullptr },
	{ "SHOW_STATS_8", _s("Show stats for party member 8"), "C+8", nullptr },
	{ "SHOW_STATS_9", _s("Show stats for party member 9"), "C+9", nullptr },
	{ "INVENTORY_1", _s("Show inventory for Avatar"), "F1", nullptr },
	{ "INVENTORY_2", _s("Show inventory for party member 2"), "F2", nullptr },
	{ "INVENTORY_3", _s("Show inventory for party member 3"), "F3", nullptr },
	{ "INVENTORY_4", _s("Show inventory for party member 4"), "F4", nullptr },
	{ "INVENTORY_5", _s("Show inventory for party member 5"), "F5", nullptr },
	{ "INVENTORY_6", _s("Show inventory for party member 6"), "F6", nullptr },
	{ "INVENTORY_7", _s("Show inventory for party member 7"), "F7", nullptr },
	{ "INVENTORY_8", _s("Show inventory for party member 8"), "F8", nullptr },
	{ "INVENTORY_9", _s("Show inventory for party member 9"), "F9", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_1", _s("Show doll gump for Avatar"), "S+F1", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_2", _s("Show doll gump for party member 2"), "S+F2", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_3", _s("Show doll gump for party member 3"), "S+F3", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_4", _s("Show doll gump for party member 4"), "S+F4", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_5", _s("Show doll gump for party member 5"), "S+F5", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_6", _s("Show doll gump for party member 6"), "S+F6", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_7", _s("Show doll gump for party member 7"), "S+F7", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_8", _s("Show doll gump for party member 8"), "S+F8", nullptr },
	// I18N: gump is Graphical User Menu Pop-up
	{ "DOLL_GUMP_9", _s("Show doll gump for party member 9"), "S+F9", nullptr },
};

static const NuvieActionDescription U6ActionDescriptions[] = {
	{ "CAST", _s("Cast"), "c", nullptr },
	{ "REST", _s("Rest"), "r", nullptr },
	{ "TOGGLE_UNLIMITED_CASTING", _s("Toggle unlimited casting"), "A+w", nullptr },
};

static const NuvieActionDescription SEActionDescriptions[] = {
	{ "REST", _s("Rest"), "r", nullptr },
	{ "TOGGLE_UNLIMITED_CASTING", _s("Toggle unlimited casting"), "A+w", nullptr },
};


static Common::Action *actionDescriptionFromNuvieAction(const NuvieActionDescription &n) {
	Common::String idstr(n._id);
	Common::Action *act = new Common::Action(n._id, _(n._desc));
	act->setCustomEngineActionEvent(idstr.hash());
	if (n._key1)
		act->addDefaultInputMapping(n._key1);
	if (n._key2)
		act->addDefaultInputMapping(n._key2);
	// Allow WALK and TELEPORT_TO_CURSOR operations to repeat so holding the key lets you keep moving
	// TODO: This would be nice to handle manually inside the event loop so
	// non-keyboard inputs also work and we can control the repeat rate.
	if (strncmp(n._id, "WALK", 4) == 0 || strncmp(n._id, "TELE", 4) == 0)
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
