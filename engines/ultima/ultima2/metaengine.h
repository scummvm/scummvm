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

#ifndef ULTIMA2_METAENGINE_H
#define ULTIMA2_METAENGINE_H

#include "backends/keymapper/keymapper.h"
#include "ultima/shared/engine/hotkeys.h"

namespace Ultima {
namespace Ultima2 {

enum KeybindingAction {
	KEYBIND_UP, KEYBIND_DOWN, KEYBIND_LEFT, KEYBIND_RIGHT,
	KEYBIND_ATTACK, KEYBIND_BOARD, KEYBIND_CAST, KEYBIND_DESCEND, KEYBIND_ENTER, KEYBIND_FIRE,
	KEYBIND_GET, KEYBIND_HYPER, KEYBIND_IGNITE_TORCH, KEYBIND_JUMP, KEYBIND_KLIMB, KEYBIND_LAUNCH,
	KEYBIND_MAGIC, KEYBIND_NEGATE_TIME, KEYBIND_OFFER, KEYBIND_PASS, KEYBIND_QUIT, KEYBIND_READY,
	KEYBIND_STEAL, KEYBIND_TRANSACT, KEYBIND_UNLOCK, KEYBIND_VIEW, KEYBIND_WEAR_ARMOR,
	KEYBIND_X_IT, KEYBIND_YELL, KEYBIND_ZSTATS,
	KEYBIND_NONE,

	// Actions common to the Ultima games
	KEYBIND_SAVE = Shared::HOTKEY_SAVE,
	KEYBIND_RESTORE = Shared::HOTKEY_RESTORE,
	KEYBIND_EXIT_GAME = Shared::HOTKEY_EXIT,
	KEYBIND_MAIN_MENU = Shared::HOTKEY_MAIN_MENU
};

enum KeybindingMode {
	KBMODE_ALL,
	KBMODE_MINIMAL,
	KBMODE_GAMEPLAY
};

class MetaEngine {
private:
	/**
	 * Adds the default actions for the mouse buttons
	 */
	static void addMouseClickActions(Common::Keymap &keyMap);
public:
	/**
	 * Initialize keymaps
	 */
	static Common::KeymapArray initKeymaps(KeybindingMode mode = KBMODE_ALL);

	/**
	 * Sets the current set of actions which are active
	 */
	static void setKeybindingMode(KeybindingMode mode);
};

} // namespace Ultima2
} // namespace Ultima

#endif
