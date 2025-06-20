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

#ifndef ULTIMA_ULTIMA8_META_ENGINE
#define ULTIMA_ULTIMA8_META_ENGINE

#include "backends/keymapper/keymapper.h"
#include "engines/metaengine.h"

namespace Ultima {
namespace Ultima8 {

enum KeybindingAction {
	ACTION_QUICKSAVE, ACTION_SAVE, ACTION_LOAD, ACTION_BEDROLL, ACTION_COMBAT,
	ACTION_BACKPACK, ACTION_KEYRING, ACTION_MINIMAP, ACTION_RECALL,
	ACTION_INVENTORY, ACTION_NEXT_WEAPON, ACTION_NEXT_INVENTORY,
	ACTION_USE_INVENTORY, ACTION_USE_MEDIKIT, ACTION_USE_ENERGYCUBE,
	ACTION_SELECT_ITEMS, ACTION_DETONATE_BOMB, ACTION_DROP_WEAPON,
	ACTION_USE_SELECTION, ACTION_GRAB_ITEMS, ACTION_MENU, ACTION_CLOSE_GUMPS,
	ACTION_HIGHLIGHT_ITEMS, ACTION_TOGGLE_TOUCHING, ACTION_JUMP,
	ACTION_SHORT_JUMP, ACTION_TURN_LEFT, ACTION_TURN_RIGHT, ACTION_MOVE_FORWARD,
	ACTION_MOVE_BACK, ACTION_MOVE_UP, ACTION_MOVE_DOWN, ACTION_MOVE_LEFT,
	ACTION_MOVE_RIGHT, ACTION_MOVE_RUN, ACTION_MOVE_STEP, ACTION_ATTACK,
	ACTION_STEP_LEFT, ACTION_STEP_RIGHT, ACTION_STEP_FORWARD,
	ACTION_STEP_BACK, ACTION_ROLL_LEFT, ACTION_ROLL_RIGHT, ACTION_TOGGLE_CROUCH,
	ACTION_CAMERA_AVATAR,

	ACTION_CLIPPING, ACTION_DEC_SORT_ORDER, ACTION_INC_SORT_ORDER, 
	ACTION_MOVE_ASCEND, ACTION_MOVE_DESCEND,

	ACTION_TOGGLE_PAINT, ACTION_FRAME_BY_FRAME,
	ACTION_ADVANCE_FRAME, ACTION_TOGGLE_STASIS, ACTION_SHAPE_VIEWER,

	ACTION_NONE
};

class MetaEngine {
public:
	/**
	 * Initialize keymaps
	 */
	static Common::KeymapArray initKeymaps(const Common::String &gameId);

	/**
	 * Return meta information from the specified save state for saves that do not have ExtendedSavegameHeader
	 */
	static bool querySaveMetaInfos(const Common::String &filename, SaveStateDescriptor &desc);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
