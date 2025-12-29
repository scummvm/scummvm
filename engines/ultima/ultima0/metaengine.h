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
	KEYBIND_SELECT, KEYBIND_ESCAPE, KEYBIND_INFO,

	KEYBIND_NONE
};

enum KeybindingMode {
	KBMODE_NORMAL		///< Keys available when normal in-game
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
	static Common::KeymapArray initKeymaps(KeybindingMode mode = KBMODE_NORMAL);

	/**
	 * Sets the current set of actions which are active
	 */
	static void setKeybindingMode(KeybindingMode mode);
};

} // End of namespace Ultima0
} // End of namespace Ultima

#endif
