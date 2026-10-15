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


#ifndef ULTIMA_SHARED_ENGINE_HOTKEYS_H
#define ULTIMA_SHARED_ENGINE_HOTKEYS_H

namespace Ultima {
namespace Shared {

/**
 * Keymap actions that every Ultima engine handles in the same way, from any
 * view. The values are kept well clear of the actions of the games themselves
 */
enum HotkeyAction {
	HOTKEY_SAVE = 1000,		// Opens the save dialog
	HOTKEY_RESTORE,			// Opens the restore dialog
	HOTKEY_EXIT,			// Exits the game
	HOTKEY_MAIN_MENU		// Goes back to the start of the game
};

} // namespace Shared
} // namespace Ultima

#endif
