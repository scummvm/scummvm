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

#include "common/system.h"
#include "common/savefile.h"
#include "graphics/surface.h"
#include "graphics/thumbnail.h"
#include "mads/core/config.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/object.h"
#include "mads/core/player.h"
#include "mads/nebular/console.h"
#include "mads/nebular/nebular.h"

namespace MADS {
namespace RexNebular {

Console::Console() : MADS::Console() {
	registerCmd("load", WRAP_METHOD(Console, cmdLoad));
}

bool Console::cmdLoad(int argc, const char **argv) {
	// Check a slot number is provided
	if (argc < 2) {
		debugPrintf("Usage: load <slot> - Load old Rex Nebular savegame\n");
		return true;
	}

	// Open the save file for reading
	Common::String filename = g_engine->getSaveStateName(strToInt(argv[1]));
	Common::InSaveFile *sf = g_system->getSavefileManager()->openForLoading(filename);
	if (!sf) {
		debugPrintf("Could not open %s\n", filename.c_str());
		return true;
	}

	// Validate it has a correct header
	char ident[5];
	sf->read(ident, 5);
	if (strncmp(ident, "MADS", 4) || sf->readByte() != 1) {
		debugPrintf("Specified file is not an old style Rex savegame.\n");
		return true;
	}

	// Skip the remainder of the header
	(void)sf->readString();
	Graphics::Surface *dummy;
	Graphics::loadThumbnail(*sf, dummy, true);
	sf->skip(14);

	// Read the actual savegame content
	// Kernel info
	kernel.fx = sf->readSint16LE();
	kernel.trigger = sf->readSint16LE();
	kernel.trigger_setup_mode = sf->readSint16LE();
	kernel.trigger_mode = sf->readSint16LE();

	Common::String name = Common::Path(sf->readString(), '/').toString();
	Common::strcpy_s(kernel.interface, name.c_str());
	sf->skip(2);

	// Scene info
	// Action
	player2.words[0] = sf->readSint16LE();
	player2.words[1] = sf->readSint16LE();
	player2.words[2] = sf->readSint16LE();
	// Active action
	sf->skip(6);
	// Misc fields
	sf->skip(12);
	// Saved fields
	sf->skip(17);
	(void)sf->readString();
	sf->skip(22);
	// Rails
	sf->skip(4);
	// User Interface
	active_inven = -1;
	sf->skip(8 * 2);	// category indexes
	// Misc
	kernel.force_restart = sf->readByte();
	kernel.teleported_in = sf->readByte();
	new_room = sf->readSint16LE();
	previous_room = sf->readSint16LE();
	room_variant = sf->readSint16LE();
	// Dynamic objects - only the count was ever saved
	sf->skip(2);

	// Objects
	num_objects = sf->readUint16LE();
	for (int i = 0; i < num_objects; ++i) {
		Object &obj = object[i];
		obj.vocab_id = sf->readSint16LE();
		obj.location = sf->readSint16LE();
		obj.prep = sf->readByte();
		obj.num_verbs = sf->readByte();
		obj.num_qualities = sf->readByte();
		sf->skip(1);

		for (auto &v : obj.verb) {
			v.id = sf->readSint16LE();
			v.verb_type = sf->readByte();
			v.prep_type = sf->readByte();
		}

		for (auto &q : obj.quality_id)
			q = sf->readByte();
		for (auto &q : obj.quality_value)
			q = sf->readSint32LE();
	}

	// Inventory
	inven_num_objects = sf->readSint16LE();
	for (int i = 0; i < inven_num_objects; ++i)
		inven[i] = sf->readSint32LE();

	// Visited scenes
	player.num_rooms_been_in = sf->readUint16LE();
	for (int i = 0; i < player.num_rooms_been_in; ++i)
		player.rooms_been_in[i] = sf->readSint32LE();
	sf->skip(1); // scene revisited

	// Player fields
	player.walking = sf->readByte();
	player.x = sf->readSint16LE();
	player.y = sf->readSint16LE();
	player.target_x = sf->readSint16LE();
	player.target_y = sf->readSint16LE();
	player.sign_x = sf->readSint16LE();
	player.sign_y = sf->readSint16LE();
	player.x_count = sf->readSint16LE();
	player.y_count = sf->readSint16LE();
	player.x_counter = sf->readSint16LE();
	player.y_counter = sf->readSint16LE();
	player.target_facing = sf->readSint16LE();
	player.special_code = sf->readSint16LE();
	sf->skip(3);
	player.walk_anywhere = sf->readByte();
	player.walk_off_edge_to_room = sf->readSint16LE();
	player.walk_off_edge = sf->readByte();
	player.need_to_walk = sf->readByte();
	player.ready_to_walk = sf->readByte();
	player.prepare_walk_facing = sf->readSint16LE();
	player.prepare_walk_x = sf->readSint16LE();
	player.prepare_walk_y = sf->readSint16LE();
	player.commands_allowed = sf->readByte();
	player.walker_visible = sf->readByte();
	player.walker_previously_visible = sf->readByte();
	sf->skip(8);
	player.facing = sf->readByte();
	player.turn_to_facing = sf->readByte();
	sf->skip(10);
	// Stop walkers
	player.stop_walker_pointer = sf->readUint16LE();
	for (int i = 0; i < player.stop_walker_pointer; ++i) {
		player.stop_walker_stack[i] = sf->readSint16LE();
		player.stop_walker_trigger[i] = sf->readSint16LE();
	}
	// Walk Trigger Action
	sf->skip(6);
	player.walk_trigger_dest = sf->readSint16LE();
	// Misc
	sf->skip(18);
	(void)sf->readString();
	sf->skip(12);

	// Screen objects
	sf->skip(4);

	// Naughtiness and difficulty
	config_file.naughtiness = sf->readByte();
	game.difficulty = sf->readByte();

	// Load room scratch data
	Common::Serializer s(sf, nullptr);
	g_engine->syncRoom(s);

	return false;
}

} // namespace RexNebular
} // namespace MADS
