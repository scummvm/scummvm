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

#ifndef NEVERHOOD_GAMEVARS_H
#define NEVERHOOD_GAMEVARS_H

#include "common/array.h"
#include "common/savefile.h"
#include "neverhood/neverhood.h"

namespace Neverhood {

enum {
	// Misc
	V_MODULE_NAME				= 0x91080831,			// Currently active module name hash
	V_CURRENT_SCENE				= 0x108A4870,			// Current scene in the current module
	V_CURRENT_SCENE_WHICH		= 0x82C80875,
	V_DEBUG						= 0xA4014072,			// Original debug-flag, can probably be removed
	V_SMACKER_CAN_ABORT			= 0x06C02850,			// Not set anywhere (yet), seems like a debug flag
	V_KEY3_LOCATION				= 0x13382860,			// Location of the third key
	V_TEXT_FLAG1				= 0x8440001F,
	V_TEXT_INDEX				= 0x01830201,
	V_TEXT_COUNTING_INDEX1		= 0x29408F00,
	V_TEXT_COUNTING_INDEX2		= 0x8A140C21,
	V_TALK_COUNTING_INDEX		= 0xA0808898,
	V_FRUIT_COUNTING_INDEX		= 0x40040831,
	V_NOISY_SYMBOL_INDEX		= 0x2414C2F2,
	V_COLUMN_BACK_NAME			= 0x4CE79018,
	V_COLUMN_TEXT_NAME			= 0xC8C28808,
	V_CLICKED_COLUMN_INDEX		= 0x48A68852,
	V_CLICKED_COLUMN_ROW		= 0x49C40058,
	// Klaymen
	V_KLAYMEN_SMALL				= 0x1860C990,			// Is Klaymen small?
	V_KLAYMEN_FRAMEINDEX		= 0x18288913,
	V_KLAYMEN_IS_DELTA_X		= 0xC0418A02,
	V_KLAYMEN_SAVED_X			= 0x00D30138,
	V_CAR_DELTA_X				= 0x21E60190,
	// Flags
	V_CRYSTAL_COLORS_INIT		= 0xDE2EC914,
	V_TV_JOKE_TOLD				= 0x92603A79,
	V_NOTES_DOOR_UNLOCKED		= 0x0045D021,
	V_WATER_RUNNING				= 0x4E0BE910,
	V_CREATURE_ANGRY			= 0x0A310817,			// After having played with the music box
	V_BEEN_SHRINKING_ROOM		= 0x1C1B8A9A,
	V_BEEN_STATUE_ROOM			= 0xCB45DE03,
	V_MOUSE_PUZZLE_SOLVED		= 0x70A1189C,
	V_NOTES_PUZZLE_SOLVED		= 0x86615030,
	V_TILE_PUZZLE_SOLVED		= 0x404290D5,
	V_STAIRS_PUZZLE_SOLVED		= 0xA9035F60,
	V_CODE_SYMBOLS_SOLVED		= 0x2C531AF8,
	V_SPIKES_RETRACTED			= 0x18890C91,
	V_LARGE_DOOR_NUMBER			= 0x9A500914,			// Number of the currently "large" door
	V_LIGHTS_ON					= 0x4D080E54,
	V_SHRINK_LIGHTS_ON			= 0x190A1D18,			// Lights on in the room with the shrinking device
	V_STAIRS_DOWN_ONCE			= 0x2050861A,			// Stairs have been down once before
	V_STAIRS_DOWN				= 0x09221A62,
	V_LADDER_DOWN				= 0x0018CA22,			// Is the ladder in the statue room down?
	V_LADDER_DOWN_ACTION		= 0x00188211,
	V_WALL_BROKEN				= 0x10938830,
	V_BOLT_DOOR_OPEN			= 0x01BA1A52,
	V_BOLT_DOOR_UNLOCKED		= 0x00040153,
	V_SEEN_SYMBOLS_NO_LIGHT		= 0x81890D14,
	V_FELL_DOWN_HOLE			= 0xE7498218,
	V_DOOR_PASSED				= 0x2090590C,			// Auto-closing door was passed
	V_ENTRANCE_OPEN				= 0xD0A14D10,			// Is the entrance to Module1300 open (after the robot got his teddy)
	V_WINDOW_OPEN				= 0x03C698DA,
	V_DOOR_STATUS				= 0x52371C95,
	V_DOOR_BUSTED				= 0xD217189D,
	V_WORLDS_JOINED				= 0x98109F12,			// Are the worlds joined?
	V_KEYDOOR_UNLOCKED			= 0x80455A41,			// Is the keyboard-door unlocked?
	V_MOUSE_SUCKED_IN			= 0x01023818,			// Are mouse/cheese in Scene1308?
	V_BALLOON_POPPED			= 0xAC00C0D0,			// Has the balloon with the key been popped?
	V_TNT_DUMMY_BUILT			= 0x000CF819,			// Are all TNT parts on the dummy?
	V_TNT_DUMMY_FUSE_LIT		= 0x20A0C516,
	V_RADIO_ENABLED				= 0x4DE80AC0,
	V_SEEN_CREATURE_EXPLODE_VID	= 0x2A02C07B,
	V_CREATURE_EXPLODED			= 0x0A18CA33,
	V_UNUSED					= 0x89C669AA,			// Seems to be unused, confirmed by checking the exe for this constant value (still left in atm)
	// Radio
	V_RADIO_ROOM_LEFT_DOOR		= 0x09880D40,
	V_RADIO_ROOM_RIGHT_DOOR		= 0x08180ABC,
	V_CURR_RADIO_MUSIC_INDEX	= 0x08CC0828,
	V_GOOD_RADIO_MUSIC_INDEX	= 0x88880915,
	V_GOOD_RADIO_MUSIC_NAME		= 0x89A82A15,
	V_RADIO_MOVE_DISH_VIDEO		= 0x28D8C940,
	// Match
	V_MATCH_STATUS				= 0x0112090A,
	// Venus fly trap
	V_FLYTRAP_RING_EATEN		= 0x2B514304,
	V_FLYTRAP_RING_DOOR			= 0x8306F218,
	V_FLYTRAP_RING_FENCE		= 0x80101B1E,
	V_FLYTRAP_RING_BRIDGE		= 0x13206309,
	V_FLYTRAP_POSITION_1		= 0x1B144052,
	V_FLYTRAP_POSITION_2		= 0x86341E88,
	// Navigation
	V_NAVIGATION_INDEX			= 0x4200189E,			// Navigation scene: Current navigation index
	// Cannon
	V_CANNON_RAISED				= 0x000809C2,			// Is the cannon raised?
	V_CANNON_TURNED				= 0x9040018A,			// Is the cannon turned?
	V_ROBOT_HIT					= 0x0C0288F4,			// Was the robot hit by the cannon?
	V_ROBOT_TARGET				= 0x610210B7,			// Is the robot at the cannon target position? (teddy)
	V_CANNON_SMACKER_NAME		= 0xF0402B0A,
	V_CANNON_TARGET_STATUS		= 0x20580A86,
	// Projector
	V_PROJECTOR_SLOT			= 0x04A10F33,			// Projector x slot index
	V_PROJECTOR_LOCATION		= 0x04A105B3,			// Projector scene location
	V_PROJECTOR_ACTIVE			= 0x12A10DB3,			// Is the projecor projecting?
	// Teleporter
	V_TELEPORTER_CURR_LOCATION	= 0x0152899A,
	V_TELEPORTER_WHICH			= 0x60826830,
	V_TELEPORTER_DEST_AVAILABLE	= 0x2C145A98,
	// Inventory
	V_HAS_NEEDLE				= 0x31C63C51,			// Has Klaymen the needle?
	V_HAS_FINAL_KEY				= 0xC0780812,			// Has Klaymen the key from the diskplayer?
	V_HAS_TEST_TUBE				= 0x45080C38,
	// Arrays
	// NOTE "GOOD" means the solution, "CURR" is the current setup of the puzzle variables
	VA_IS_PUZZLE_INIT			= 0x40050052,
	VA_SMACKER_PLAYED			= 0x00800410,
	VA_CURR_CRYSTAL_COLORS		= 0xE11A1929,
	VA_GOOD_CRYSTAL_COLORS		= 0xD4B2089C,
	VA_GOOD_TEST_TUBES_LEVEL_1	= 0x0C601058,
	VA_GOOD_TEST_TUBES_LEVEL_2	= 0x40005834,
	VA_CURR_CANNON_SYMBOLS		= 0x00000914,
	VA_GOOD_CANNON_SYMBOLS_1	= 0x00504B86,
	VA_GOOD_CANNON_SYMBOLS_2	= 0x0A4C0A9A,
	VA_CURR_WATER_PIPES_LEVEL	= 0x0800547C,
	VA_GOOD_WATER_PIPES_LEVEL	= 0x90405038,
	VA_CURR_DICE_NUMBERS		= 0x61084036,
	VA_GOOD_DICE_NUMBERS		= 0x7500993A,
	VA_CURR_KEY_SLOT_NUMBERS	= 0xA010B810,
	VA_GOOD_KEY_SLOT_NUMBERS	= 0x0C10A000,
	VA_CUBE_POSITIONS			= 0x484498D0,
	VA_CODE_SYMBOLS				= 0x04909A50,
	VA_TILE_SYMBOLS				= 0x0C65F80B,
	VA_IS_TILE_MATCH			= 0xCCE0280F,
	VA_TNT_POSITIONS			= 0x10055D14,
	VA_DICE_MEMORY_SYMBOLS		= 0x13100631,
	VA_HAS_TAPE					= 0x02038314,
	VA_IS_TAPE_INSERTED			= 0x02720344,
	VA_HAS_KEY					= 0x0090EA95,
	VA_IS_KEY_INSERTED			= 0x08D0AB11,
	VA_LOCKS_DISABLED			= 0x14800353,
	V_END_
};

struct GameVar {
	uint32 nameHash;
	uint32 value;
	int16 firstIndex, nextIndex;
};

class Console;

class GameVars {
public:
	GameVars();
	void clear();
	void loadState(Common::InSaveFile *in);
	void saveState(Common::OutSaveFile *out);
	uint32 getGlobalVar(uint32 nameHash);
	void setGlobalVar(uint32 nameHash, uint32 value);
	uint32 getSubVar(uint32 nameHash, uint32 subNameHash);
	void setSubVar(uint32 nameHash, uint32 subNameHash, uint32 value);
	void dumpVars(Console *con);
protected:
	Common::Array<GameVar> _vars;
	int16 addVar(uint32 nameHash, uint32 value);
	int16 findSubVarIndex(int16 varIndex, uint32 subNameHash);
	int16 addSubVar(int16 varIndex, uint32 subNameHash, uint32 value);
	int16 getSubVarIndex(int16 varIndex, uint32 subNameHash);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_GAMEVARS_H */
