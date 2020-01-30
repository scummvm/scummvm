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

#ifndef NUVIE_KEYBINDING_KEYS_ENUM_H
#define NUVIE_KEYBINDING_KEYS_ENUM_H

namespace Ultima {
namespace Nuvie {

// FIXME - I needed to reduce includes. Maybe use C++11 enum class in the future
enum ActionKeyType {
	WEST_KEY = 0,
	EAST_KEY,
	NORTH_KEY,
	SOUTH_KEY,
	NORTH_EAST_KEY,
	SOUTH_EAST_KEY,
	NORTH_WEST_KEY,
	SOUTH_WEST_KEY,
	TOGGLE_CURSOR_KEY,
	DO_ACTION_KEY, // don't change the order before this without checking MsgScroll.cpp, MapEditorView.cpp and MapWindow.cpp
	CANCEL_ACTION_KEY,
	NEW_COMMAND_BAR_KEY,
	NEXT_PARTY_MEMBER_KEY,
	PREVIOUS_PARTY_MEMBER_KEY,
	MSGSCROLL_UP_KEY,
	MSGSCROLL_DOWN_KEY,
	TOGGLE_AUDIO_KEY,
	TOGGLE_MUSIC_KEY,
	TOGGLE_SFX_KEY,
	TOGGLE_FPS_KEY,
	TOGGLE_FULLSCREEN_KEY,
	DECREASE_DEBUG_KEY,
	INCREASE_DEBUG_KEY,
	QUIT_KEY,
	HOME_KEY,
	END_KEY,
	OTHER_KEY
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
