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

#ifndef ULTIMA_ULTIMA8_META_ENGINE
#define ULTIMA_ULTIMA8_META_ENGINE

#include "backends/keymapper/keymapper.h"

namespace Ultima {
namespace Ultima8 {

enum KeybindingAction {
	ACTION_QUICKSAVE, ACTION_SAVE, ACTION_LOAD, ACTION_BEDROLL, ACTION_COMBAT,
	ACTION_BACKPACK, ACTION_KEYRING, ACTION_MINIMAP, ACTION_RECALL,
	ACTION_INVENTORY, ACTION_NEXT_WEAPON, ACTION_NEXT_INVENTORY,
	ACTION_USE_INVENTORY, ACTION_USE_MEDIKIT, ACTION_SELECT_ITEMS,
	ACTION_DETONATE_BOMB, ACTION_USE_SELECTION, ACTION_MENU,
	ACTION_CLOSE_GUMPS, ACTION_HIGHLIGHT_ITEMS, ACTION_TOGGLE_TOUCHING,
	ACTION_JUMP, ACTION_TURN_LEFT, ACTION_TURN_RIGHT, ACTION_MOVE_FORWARD,
	ACTION_MOVE_BACK, ACTION_MOVE_UP, ACTION_MOVE_DOWN, ACTION_MOVE_LEFT,
	ACTION_MOVE_RIGHT, ACTION_MOVE_RUN, ACTION_MOVE_STEP, ACTION_ATTACK,
	ACTION_CAMERA_AVATAR,

	ACTION_CHEAT_MODE, ACTION_CLIPPING, ACTION_DEC_SORT_ORDER,
	ACTION_INC_SORT_ORDER, ACTION_QUICK_MOVE_ASCEND, ACTION_QUICK_MOVE_DESCEND,
	ACTION_QUICK_MOVE_UP, ACTION_QUICK_MOVE_DOWN, ACTION_QUICK_MOVE_LEFT,
	ACTION_QUICK_MOVE_RIGHT,

#ifndef RELEASE_BUILD
	ACTION_TOGGLE_PAINT, ACTION_ENGINE_STATS, ACTION_FRAME_BY_FRAME,
	ACTION_ADVANCE_FRAME, ACTION_TOGGLE_STASIS, ACTION_SHAPE_VIEWER,
#endif

	ACTION_NONE
};

class MetaEngine {
private:
	/**
	 * Get the method to execute
	 */
	static Common::String getMethod(KeybindingAction keyAction, bool isPress);

public:
	/**
	 * Initialize keymaps
	 */
	static Common::KeymapArray initKeymaps(const Common::String &gameId, bool isMenuActive = false);

	/**
	 * Execute an engine keymap press action
	 */
	static void pressAction(KeybindingAction keyAction);

	/**
	 * Execute an engine keymap release action
	 */
	static void releaseAction(KeybindingAction keyAction);

	/**
	 * Enables/disables keymaps based on whether the in-game menu is active
	 */
	static void setGameMenuActive(bool isActive);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
