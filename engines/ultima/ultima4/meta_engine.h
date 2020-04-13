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
	KEYBIND_ATTACK, KEYBIND_BOARD, KEYBIND_CAST, KEYBIND_ENTER,
	KEYBIND_FIRE, KEYBIND_GET, KEYBIND_HOLE_UP, KEYBIND_IGNITE,
	KEYBIND_JIMMY, KEYBIND_LOCATE,
	KEYBIND_PASS,

	KEYBIND_NONE
};

class MetaEngine {
private:
	/**
	 * Get the method to execute
	 */
	static Common::String getMethod(KeybindingAction keyAction);
public:
	/**
	 * Initialize keymaps
	 */
	static Common::KeymapArray initKeymaps();

	/**
	 * Execute an engine keymap action
	 */
	static void executeAction(KeybindingAction keyAction);

	/**
	 * Enables/disables the keymaps when not waiting for an in-game action
	 */
	static void setKeybindingsActive(bool isActive);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
