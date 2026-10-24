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

#include "mm/mm1/metaengine.h"
#include "mm/mm1/mm1.h"
#include "common/translation.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

namespace MM {
namespace MM1 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_key;
	const char *_joy;
};

static const KeybindingRecord MINIMAL_KEYS[] = {
	// I18N: ESC key
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Escape"), "ESCAPE", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord MENU_KEYS[] = {
	// I18N: ESC key
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Escape"), "ESCAPE", nullptr },
	// I18N: Return key
	{ KEYBIND_SELECT, "SELECT", _s("Select"), "RETURN", nullptr },
	// I18N: N key for No
	{ KEYBIND_KEY_N, "KEY_N", _s("No"), "n", nullptr },
	// I18N: Y key for Yes
	{ KEYBIND_KEY_Y, "KEY_Y", _s("Yes"), "y", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord PARTY_KEYS[] = {
	{ KEYBIND_VIEW_PARTY1, "PARTY1", _s("View party member 1"), "F1", nullptr },
	{ KEYBIND_VIEW_PARTY2, "PARTY2", _s("View party member 2"), "F2", nullptr },
	{ KEYBIND_VIEW_PARTY3, "PARTY3", _s("View party member 3"), "F3", nullptr },
	{ KEYBIND_VIEW_PARTY4, "PARTY4", _s("View party member 4"), "F4", nullptr },
	{ KEYBIND_VIEW_PARTY5, "PARTY5", _s("View party member 5"), "F5", nullptr },
	{ KEYBIND_VIEW_PARTY6, "PARTY6", _s("View party member 6"), "F6", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord QUEST_LOG_KEYS[] = {
	// I18N: ESC key
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Escape"), "ESCAPE", nullptr },
	// I18N: Return key
	{ KEYBIND_SELECT, "SELECT", _s("Select"), "RETURN", nullptr },
	// I18N: N key for No
	{ KEYBIND_KEY_N, "KEY_N", _s("No"), "n", nullptr },
	// I18N: Y key for Yes
	{ KEYBIND_KEY_Y, "KEY_Y", _s("Yes"), "y", nullptr },
	{ KEYBIND_QUEST_PREVIOUS, "QUEST_PREVIOUS", _s("Previous quest"), "UP", nullptr },
	{ KEYBIND_QUEST_NEXT, "QUEST_NEXT", _s("Next quest"), "DOWN", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord BESTIARY_KEYS[] = {
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Close bestiary"), "ESCAPE", nullptr },
	{ KEYBIND_BESTIARY, "BESTIARY_CLOSE", _s("Close bestiary"), "n", nullptr },
	{ KEYBIND_SELECT, "SELECT", _s("Show monster image"), "RETURN", nullptr },
	{ KEYBIND_BESTIARY_UP, "BESTIARY_UP", _s("Previous monster"), "UP", nullptr },
	{ KEYBIND_BESTIARY_DOWN, "BESTIARY_DOWN", _s("Next monster"), "DOWN", nullptr },
	{ KEYBIND_BESTIARY_PAGEUP, "BESTIARY_PAGEUP", _s("Previous page"), "PAGEUP", nullptr },
	{ KEYBIND_BESTIARY_PAGEDOWN, "BESTIARY_PAGEDOWN", _s("Next page"), "PAGEDOWN", nullptr },
	{ KEYBIND_BESTIARY_HOME, "BESTIARY_HOME", _s("First monster"), "HOME", nullptr },
	{ KEYBIND_BESTIARY_END, "BESTIARY_END", _s("Last monster"), "END", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord COMBAT_KEYS[] = {
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_ATTACK, "COMBAT_ATTACK", _s("Attack"), "a", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_BLOCK, "COMBAT_BLOCK", _s("Block"), "b", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_CAST, "COMBAT_CAST", _s("Cast"), "c", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_EXCHANGE, "COMBAT_EXCHANGE", _s("Exchange"), "e", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_FIGHT, "COMBAT_FIGHT", _s("Fight"), "f", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_QUICKREF, "QUICKREF", _s("Quick reference"), "q", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_RETREAT, "COMBAT_RETREAT", _s("Retreat"), "r", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_SHOOT, "COMBAT_SHOOT", _s("Shoot"), "s", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_COMBAT_USE, "COMBAT_USE", _s("Use"), "u", nullptr },

	{ KEYBIND_COMBAT_LOG, "COMBAT_LOG", _s("Combat log"), "l", nullptr },
	{ KEYBIND_COMBAT_LOG_UP, "COMBAT_LOG_UP", _s("Combat log up"), "UP", nullptr },
	{ KEYBIND_COMBAT_LOG_DOWN, "COMBAT_LOG_DOWN", _s("Combat log down"), "DOWN", nullptr },
	{ KEYBIND_ESCAPE, "COMBAT_LOG_CLOSE", _s("Close combat log"), "ESCAPE", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_DELAY, "DELAY", _s("Delay"), "d", nullptr },
	// I18N: Combat command in Might & Magic 1
	{ KEYBIND_PROTECT, "PROTECT", _s("Protect"), "p", nullptr },
	{ KEYBIND_BESTIARY, "BESTIARY", _s("Bestiary"), "n", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord NORMAL_KEYS[] = {
	// I18N: Select party member command in Might & Magic 1
	{ KEYBIND_SELECT, "SELECT", _s("Select"), "RETURN", nullptr },
	// I18N: Party arrangement command in Might & Magic 1
	{ KEYBIND_FORWARDS, "FORWARDS", _s("Forwards"), "UP", nullptr },
	// I18N: Party arrangement command in Might & Magic 1
	{ KEYBIND_BACKWARDS, "BACKWARDS", _s("Backwards"), "DOWN", nullptr },
	{ KEYBIND_TURN_LEFT, "TURN_LEFT", _s("Turn left"), "LEFT", nullptr },
	{ KEYBIND_TURN_RIGHT, "TURN_RIGHT", _s("Turn right"), "RIGHT", nullptr },
	{ KEYBIND_STRAFE_LEFT, "STRAFE_LEFT", _s("Strafe left"), "C+LEFT", nullptr },
	{ KEYBIND_STRAFE_RIGHT, "STRAFE_RIGHT", _s("Strafe right"), "C+RIGHT", nullptr },

	{ KEYBIND_SPELL, "CAST", _s("Cast spell"), "c", nullptr },
	{ KEYBIND_MAP, "MAP", _s("Show map"), "m", nullptr },
	{ KEYBIND_MINIMAP, "MINIMAP", _s("Toggle minimap"), "F9", nullptr },
	{ KEYBIND_ORDER, "ORDER", _s("Reorder party"), "o", nullptr },
	// I18N: Action of hero party in Might & Magic 1
	{ KEYBIND_PROTECT, "PROTECT", _s("Protect"), "p", nullptr },
	{ KEYBIND_BESTIARY, "BESTIARY", _s("Bestiary"), "n", nullptr },
	// I18N: Action of hero party in Might & Magic 1
	{ KEYBIND_REST, "REST", _s("Rest"), "r", nullptr },
	// I18N: Action of hero party in Might & Magic 1
	{ KEYBIND_SEARCH, "SEARCH", _s("Search"), "s", nullptr },
	// I18N: Action of hero party in Might & Magic 1
	{ KEYBIND_BASH, "BASH", _s("Bash"), "b", nullptr },
	// I18N: Action of hero party in Might & Magic 1
	{ KEYBIND_UNLOCK, "UNLOCK", _s("Unlock"), "u", nullptr },
	{ KEYBIND_QUICKREF, "QUICKREF", _s("Quick reference"), "q", nullptr },
	{ KEYBIND_ACTIVE_QUEST, "ACTIVE_QUEST", _s("Show active quest"), "k", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

#if 0
static const KeybindingRecord CHEAT_KEYS[] = {
	{ KEYBIND_CHEAT_GOTO, "CHEAT-GOTO", _s("Goto location"), "A+g", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};
#endif

static const char *const kMenuKeymapId = "mm1_menu";
static const char *const kMinimalKeymapId = "mm1_minimal";
static const char *const kPartyKeymapId = "mm1_party";
static const char *const kNormalKeymapId = "mm1_normal";
static const char *const kCombatKeymapId = "mm1_combat";
static const char *const kQuestLogKeymapId = "mm1_quest_log";
static const char *const kBestiaryKeymapId = "mm1_bestiary";
static const char *const kCheatsKeymapId = "mm1_cheats";

static Common::Keymap *g_menuKeymap = nullptr;
static Common::Keymap *g_minimalKeymap = nullptr;
static Common::Keymap *g_partyKeymap = nullptr;
static Common::Keymap *g_normalKeymap = nullptr;
static Common::Keymap *g_combatKeymap = nullptr;
static Common::Keymap *g_questLogKeymap = nullptr;
static Common::Keymap *g_bestiaryKeymap = nullptr;
static Common::Keymap *g_cheatsKeymap = nullptr;

struct KeysRecord {
	const char *_id;
	const char *_desc;
	const KeybindingRecord *_keys;
};

static const KeysRecord ALL_RECORDS[] = {
	{ kMenuKeymapId,    _s("Might and Magic 1 - Menus"), MENU_KEYS },
	{ kMinimalKeymapId, _s("Might and Magic 1 - Minimal Keys"), MINIMAL_KEYS },
	{ kNormalKeymapId,  _s("Might and Magic 1 - Main"), NORMAL_KEYS },
	{ kPartyKeymapId,   _s("Might and Magic 1 - Party"), PARTY_KEYS },
	{ kCombatKeymapId,  _s("Might and Magic 1 - Combat"), COMBAT_KEYS },
	{ kQuestLogKeymapId, _s("Might and Magic 1 - Quest Log"), QUEST_LOG_KEYS },
	{ kBestiaryKeymapId, _s("Might and Magic 1 - Bestiary"), BESTIARY_KEYS },
#if 0
	{ kCheatsKeymapId,  _s("Might and Magic 1 - Cheats"), CHEAT_KEYS },
#endif
	{ nullptr, nullptr, nullptr }
};

static void initKeymapPointers() {
	Common::Keymapper *const mapper = g_engine->getEventManager()->getKeymapper();

	// Always refetch, since keymaps are recreated each time the engine starts
	g_menuKeymap = mapper->getKeymap(kMenuKeymapId);
	g_minimalKeymap = mapper->getKeymap(kMinimalKeymapId);
	g_partyKeymap = mapper->getKeymap(kPartyKeymapId);
	g_normalKeymap = mapper->getKeymap(kNormalKeymapId);
	g_combatKeymap = mapper->getKeymap(kCombatKeymapId);
	g_questLogKeymap = mapper->getKeymap(kQuestLogKeymapId);
	g_bestiaryKeymap = mapper->getKeymap(kBestiaryKeymapId);
	g_cheatsKeymap = mapper->getKeymap(kCheatsKeymapId);
}

Common::KeymapArray MetaEngine::initKeymaps() {
	Common::KeymapArray keymapArray;
	Common::Keymap *keyMap;
	Common::Action *act;

	for (const KeysRecord *recPtr = ALL_RECORDS; recPtr->_id; ++recPtr) {
		// Core keymaps
		keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame,
			recPtr->_id, recPtr->_desc);
		keyMap->setEnabled(false);
		keymapArray.push_back(keyMap);

		for (const KeybindingRecord *r = recPtr->_keys; r->_id; ++r) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			if (r->_action == KEYBIND_SELECT)
				act->addDefaultInputMapping("SPACE");
			if (recPtr->_keys == BESTIARY_KEYS && r->_action == KEYBIND_SELECT)
				act->addDefaultInputMapping("KP_ENTER");

			if (r->_action == KEYBIND_FORWARDS || r->_action == KEYBIND_BACKWARDS ||
					r->_action == KEYBIND_TURN_LEFT || r->_action == KEYBIND_TURN_RIGHT ||
					r->_action == KEYBIND_STRAFE_LEFT || r->_action == KEYBIND_STRAFE_RIGHT ||
					r->_action == KEYBIND_BESTIARY_UP || r->_action == KEYBIND_BESTIARY_DOWN ||
					r->_action == KEYBIND_BESTIARY_PAGEUP || r->_action == KEYBIND_BESTIARY_PAGEDOWN)
				// Allow movement actions to be triggered on keyboard repeats
				act->allowKbdRepeats();

			keyMap->addAction(act);
		}
	}

	return keymapArray;
}

static bool isKeymapEnabledForMode(const Common::Keymap *keymap, KeybindingMode mode) {
	if (keymap == g_menuKeymap)
		return mode == KBMODE_MENUS || mode == KBMODE_PARTY_MENUS;
	if (keymap == g_minimalKeymap)
		return mode == KBMODE_MINIMAL;
	if (keymap == g_partyKeymap)
		return mode == KBMODE_PARTY_MENUS || mode == KBMODE_NORMAL || mode == KBMODE_COMBAT;
	if (keymap == g_normalKeymap)
		return mode == KBMODE_NORMAL;
	if (keymap == g_combatKeymap)
		return mode == KBMODE_COMBAT;
	if (keymap == g_questLogKeymap)
		return mode == KBMODE_QUEST_LOG;
	if (keymap == g_bestiaryKeymap)
		return mode == KBMODE_BESTIARY;
	if (keymap == g_cheatsKeymap)
		return mode == KBMODE_NORMAL;

	return false;
}

void MetaEngine::setKeybindingMode(KeybindingMode mode) {
	initKeymapPointers();

	Common::Keymap *allKeymaps[] = {
		g_menuKeymap,
		g_minimalKeymap,
		g_partyKeymap,
		g_normalKeymap,
		g_combatKeymap,
		g_questLogKeymap,
		g_bestiaryKeymap,
		g_cheatsKeymap
	};

	for (Common::Keymap *keymap : allKeymaps) {
		if (keymap)
			keymap->setEnabled(isKeymapEnabledForMode(keymap, mode));
	}
}

void MetaEngine::executeAction(KeybindingAction keyAction) {
	g_engine->send(ActionMessage(keyAction));
}

Common::KeyState MetaEngine::getActionKeyState(KeybindingAction keyAction) {
	switch (keyAction) {
	case KEYBIND_KEY_N:
		return Common::KeyState(Common::KEYCODE_n, 'n');
	case KEYBIND_KEY_Y:
		return Common::KeyState(Common::KEYCODE_y, 'y');
	default:
		return Common::KeyState(Common::KEYCODE_INVALID, 0);
	}
}

} // End of namespace MM1
} // End of namespace MM
