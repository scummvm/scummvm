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

#ifndef ULTIMA4_META_ENGINE
#define ULTIMA4_META_ENGINE

#include "backends/keymapper/keymapper.h"

namespace Ultima {
namespace Ultima4 {

enum KeybindingAction {
	KEYBIND_UP, KEYBIND_DOWN, KEYBIND_LEFT, KEYBIND_RIGHT,
	KEYBIND_ATTACK, KEYBIND_BOARD, KEYBIND_CAST, KEYBIND_CLIMB,
	KEYBIND_DESCEND, KEYBIND_ENTER, KEYBIND_EXIT, KEYBIND_FIRE,
	KEYBIND_GET, KEYBIND_HOLE_UP, KEYBIND_IGNITE, KEYBIND_JIMMY,
	KEYBIND_LOCATE, KEYBIND_MIX, KEYBIND_NEW_ORDER,
	KEYBIND_OPEN_DOOR, KEYBIND_PASS, KEYBIND_PEER,
	KEYBIND_QUIT_SAVE, KEYBIND_READY_WEAPON, KEYBIND_SEARCH,
	KEYBIND_STATS, KEYBIND_TALK, KEYBIND_USE, KEYBIND_WEAR,
	KEYBIND_YELL, KEYBIND_INTERACT, KEYBIND_ESCAPE,

	KEYBIND_SPEED_UP, KEYBIND_SPEED_DOWN, KEYBIND_SPEED_NORMAL,
	KEYBIND_COMBATSPEED_UP, KEYBIND_COMBATSPEED_DOWN,
	KEYBIND_COMBATSPEED_NORMAL,

	KEYBIND_PARTY0, KEYBIND_PARTY1, KEYBIND_PARTY2, KEYBIND_PARTY3,
	KEYBIND_PARTY4, KEYBIND_PARTY5, KEYBIND_PARTY6, KEYBIND_PARTY7,
	KEYBIND_PARTY8,

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
	KBMODE_NORMAL,		///< Keys available when normal in-game
	KBMODE_MINIMAL,		///< Minimal list available when reading input
	KBMODE_DIRECTION,	///< Mode for selecting direction
	KBMODE_MENU,		///< Intro config menus
	KBMODE_COMBAT		///< Keys when in combat mode
};

class MetaEngine {
private:
	/**
	 * Get the method to execute
	 */
	static Common::String getMethod(KeybindingAction keyAction);

	/**
	 * Adds the default actions for the mouse buttons
	 */
	static void addMouseClickActions(Common::Keymap &keyMap);
public:
	/**
	 * Initialize keymaps
	 */
	static Common::KeymapArray initKeymaps(KeybindingMode mode = KBMODE_NORMAL);

	/**
	 * Execute an engine keymap action
	 */
	static void executeAction(KeybindingAction keyAction);

	/**
	 * Sets the current set of actions which are active
	 */
	static void setKeybindingMode(KeybindingMode mode);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
