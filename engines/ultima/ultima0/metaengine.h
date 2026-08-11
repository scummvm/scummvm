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

#ifndef ULTIMA0_METAENGINE_H
#define ULTIMA0_METAENGINE_H

#include "backends/keymapper/keymapper.h"

namespace Ultima {
namespace Ultima0 {

enum KeybindingAction {
	KEYBIND_UP, KEYBIND_DOWN, KEYBIND_LEFT, KEYBIND_RIGHT,
	KEYBIND_ESCAPE, KEYBIND_QUIT, KEYBIND_ENTER, KEYBIND_INFO,
	KEYBIND_PASS, KEYBIND_ATTACK, KEYBIND_SWING, KEYBIND_THROW,
	KEYBIND_AMULET1, KEYBIND_AMULET2, KEYBIND_AMULET3, KEYBIND_AMULET4,
	KEYBIND_MINIMAP,

	KEYBIND_SELECT, KEYBIND_FOOD, KEYBIND_RAPIER, KEYBIND_AXE,
	KEYBIND_SHIELD, KEYBIND_BOW, KEYBIND_AMULET,

	KEYBIND_NONE
};

enum KeybindingMode {
	KBMODE_ALL,
	KBMODE_MINIMAL,
	KBMODE_MENUS,
	KBMODE_OVERWORLD,
	KBMODE_DUNGEONS
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

} // End of namespace Ultima0
} // End of namespace Ultima

#endif
