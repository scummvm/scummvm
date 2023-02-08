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

#ifndef MM_MM1_META_ENGINE
#define MM_MM1_META_ENGINE

#include "backends/keymapper/keymapper.h"

namespace MM {
namespace MM1 {

enum KeybindingAction {
	KEYBIND_ESCAPE,
	KEYBIND_SELECT,

	KEYBIND_FORWARDS,
	KEYBIND_BACKWARDS,
	KEYBIND_TURN_LEFT,
	KEYBIND_TURN_RIGHT,
	KEYBIND_STRAFE_LEFT,
	KEYBIND_STRAFE_RIGHT,
	KEYBIND_DELAY,
	KEYBIND_MAP,
	KEYBIND_ORDER,
	KEYBIND_PROTECT,
	KEYBIND_REST,
	KEYBIND_SEARCH,
	KEYBIND_BASH,
	KEYBIND_UNLOCK,
	KEYBIND_QUICKREF,
	KEYBIND_VERSION,
	KEYBIND_MENU,
	KEYBIND_MINIMAP,
	KEYBIND_VIEW_PARTY1,
	KEYBIND_VIEW_PARTY2,
	KEYBIND_VIEW_PARTY3,
	KEYBIND_VIEW_PARTY4,
	KEYBIND_VIEW_PARTY5,
	KEYBIND_VIEW_PARTY6,

	// Encounters
	KEYBIND_ATTACK,
	KEYBIND_BRIBE,
	KEYBIND_RETREAT,
	KEYBIND_SURRENDER,

	// Combat
	KEYBIND_COMBAT_ATTACK,
	KEYBIND_COMBAT_BLOCK,
	KEYBIND_COMBAT_CAST,
	KEYBIND_COMBAT_EXCHANGE,
	KEYBIND_COMBAT_FIGHT,
	KEYBIND_COMBAT_RETREAT,
	KEYBIND_COMBAT_SHOOT,
	KEYBIND_COMBAT_USE,

	// Cheats
	KEYBIND_CHEAT_COLLISIONS, KEYBIND_CHEAT_DESTROY,
	KEYBIND_CHEAT_DESTROY_CREATURES, KEYBIND_CHEAT_EQUIPMENT,
	KEYBIND_CHEAT_FLEE, KEYBIND_CHEAT_GOTO, KEYBIND_CHEAT_HELP,
	KEYBIND_CHEAT_ITEMS, KEYBIND_CHEAT_KARMA, KEYBIND_CHEAT_LEAVE,
	KEYBIND_CHEAT_MIXTURES, KEYBIND_CHEAT_NO_COMBAT,
	KEYBIND_CHEAT_OVERHEAD, KEYBIND_CHEAT_PARTY, KEYBIND_CHEAT_REAGENTS,
	KEYBIND_CHEAT_STATS, KEYBIND_CHEAT_TRANSPORT, KEYBIND_CHEAT_UP,
	KEYBIND_CHEAT_DOWN, KEYBIND_CHEAT_VIRTUE, KEYBIND_CHEAT_WIND,

	KEYBIND_NONE
};

enum KeybindingMode {
	KBMODE_MENUS,       ///< Keys for when showing menus
	KBMODE_PARTY_MENUS, ///< Keys for menus that allow switching party members
	KBMODE_NORMAL,      ///< Keys available when normal in-game
	KBMODE_COMBAT       ///< Keys when in combat mode
};

class MetaEngine {
public:
	/**
	 * Initialize keymaps
	 */
	static Common::KeymapArray initKeymaps(KeybindingMode mode = KBMODE_MENUS);

	/**
	 * Execute an engine keymap action
	 */
	static void executeAction(KeybindingAction keyAction);

	/**
	 * Sets the current set of actions which are active
	 */
	static void setKeybindingMode(KeybindingMode mode);
};

} // End of namespace MM1
} // End of namespace MM

#endif
