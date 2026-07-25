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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _checkGuardFl;
};

static Scratch local;


static void room_311_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_full_name(307, 'X', 0, "", EXT_SS), 0);
	g_sprite_ids[2] = kernel_load_series("*RXCL_8", 0);
	g_sprite_ids[3] = kernel_load_series("*RXCL_2", 0);

	g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 1);
	_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
	_scene->_sequences.setDepth(g_sequence_ids[1], 15);

	local._checkGuardFl = false;
	kernel.quotes = quote_load(0xFA, 0);

	if (previous_room == 391) {
		global[kSexOfRex] = REX_MALE;
		player.commands_allowed = false;
		player.walker_visible = false;
		player.facing = FACING_SOUTH;
		player.x = 166;
		player.y = 101;
		_scene->_sequences.addTimer(120, 71);
	} else if (previous_room == 310) {
		player.x = 302;
		player.y = 145;
	}
	else if (previous_room == 320) {
		player.x = 129;
		player.y = 113;
		player.facing = FACING_SOUTH;
	} else if (previous_room != RETURNING_FROM_DIALOG) {
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_run_animation(kernel_name('a', -1), 70);
	}

	section_3_music();
}

static void room_311_daemon() {
	int oldIdx;

	switch (kernel.trigger) {
	case 70:
		new_room = 310;
		break;

	case 71:
		_scene->_sequences.remove(g_sequence_ids[1]);
		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 2);
		_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(g_sequence_ids[1], 1);

		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 1);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addTimer(15, 72);
		break;

	case 72:
		_scene->_sequences.setDone(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 2);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addTimer(15, 73);
		break;

	case 73:
		_scene->_sequences.remove(g_sequence_ids[1]);
		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 3);
		_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(g_sequence_ids[1], 1);

		_scene->_sequences.setDone(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 3);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addTimer(15, 74);
		break;

	case 74:
		_scene->_sequences.setDone(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 4, 5);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		break;

	case 75:
		oldIdx = g_sequence_ids[3];
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 6);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.updateTimeout(g_sequence_ids[3], oldIdx);
		_scene->_sequences.addTimer(15, 76);
		break;

	case 76:
		_scene->_sequences.setDone(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 7);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addTimer(15, 77);
		break;

	case 77:
		_scene->_sequences.remove(g_sequence_ids[1]);
		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 2);
		_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(g_sequence_ids[1], 1);

		_scene->_sequences.setDone(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 8);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addTimer(15, 78);
		break;

	case 78:
		_scene->_sequences.remove(g_sequence_ids[1]);
		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 1);
		_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(g_sequence_ids[1], 15);

		_scene->_sequences.setDone(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 9);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addTimer(15, 79);
		break;

	case 79:
		_scene->_sequences.setDone(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 10, -2);
		_scene->_sequences.setMsgLayout(g_sequence_ids[3]);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		break;

	case 80:
		_scene->_sequences.updateTimeout(-1, g_sequence_ids[3]);
		player.commands_allowed = true;
		player.walker_visible = true;
		break;

	default:
		break;
	}

	if (player.walking && (_scene->_rails.getNext() > 0)) {
		int x = player.prepare_walk_x;
		if (x < 75)
			x = 75;
		if (x > 207)
			x = 207;

		local._checkGuardFl = true;
		player_start_walking(x, 122, FACING_SOUTH);
		_scene->_rails.resetNext();
	}
}

static void room_311_parser() {
	if (player.look_around)
		text_show(31119);
	else if (local._checkGuardFl) {
		local._checkGuardFl = false;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.addQuote(250, 0, 240);
	} else if (player_said_2(sit_at, desk))
		new_room = 320;
	else if (player_said_2(climb_into, air_vent)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			_scene->_sequences.remove(g_sequence_ids[1]);
			g_sequence_ids[1] = _scene->_sequences.addSpriteCycle(g_sprite_ids[1], false, 50, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[1], 3, -2);
			_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(g_sequence_ids[1], 15);

			g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 15, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[2], -1, 4);
			_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
			_scene->_sequences.addSubEntry(g_sequence_ids[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
		{
			int oldIdx = g_sequence_ids[1];
			g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, -2);
			_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(g_sequence_ids[1], 15);
			_scene->_sequences.updateTimeout(g_sequence_ids[1], oldIdx);
		}
		break;

		case 2:
		{
			int oldIdx = g_sequence_ids[2];
			g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[2], 4, 10);
			_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
			_scene->_sequences.updateTimeout(g_sequence_ids[2], oldIdx);
			_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

		case 3:
		{
			_scene->_sequences.remove(g_sequence_ids[1]);
			g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 3);
			_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(g_sequence_ids[1], 1);

			int oldIdx = g_sequence_ids[2];
			g_sequence_ids[2] = _scene->_sequences.startCycle(g_sprite_ids[2], false, 11);
			_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
			_scene->_sequences.setPosition(g_sequence_ids[2], Common::Point(167, 100));
			_scene->_sequences.updateTimeout(g_sequence_ids[2], oldIdx);
			_scene->_sequences.addTimer(15, 4);
		}
		break;

		case 4:
			_scene->_sequences.remove(g_sequence_ids[1]);
			g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 2);
			_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(g_sequence_ids[1], 1);

			_scene->_sequences.remove(g_sequence_ids[2]);
			g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[2], 12, 14);
			_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
			_scene->_sequences.setPosition(g_sequence_ids[2], Common::Point(167, 100));
			_scene->_sequences.addSubEntry(g_sequence_ids[2], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
			break;

		case 5:
		{
			int oldIdx = g_sequence_ids[2];
			g_sequence_ids[2] = _scene->_sequences.startCycle(g_sprite_ids[2], false, 15);
			_scene->_sequences.setMsgLayout(g_sequence_ids[2]);
			_scene->_sequences.setPosition(g_sequence_ids[2], Common::Point(167, 100));
			_scene->_sequences.updateTimeout(g_sequence_ids[2], oldIdx);
			_scene->_sequences.addTimer(15, 6);
		}
		break;

		case 6:
			_scene->_sequences.remove(g_sequence_ids[2]);
			_scene->_sequences.remove(g_sequence_ids[1]);
			g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 1);
			_scene->_sequences.setPosition(g_sequence_ids[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(g_sequence_ids[1], 1);
			_scene->_sequences.addTimer(15, 7);
			break;

		case 7:
			new_room = 313;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, desk))
		text_show(31110);
	else if (player_said_2(look, wall))
		text_show(31111);
	else if (player_said_2(look, lighting_fixture) || player_said_2(stare_at, lighting_fixture))
		text_show(31112);
	else if (player_said_2(look, lights) || player_said_2(stare_at, lights))
		text_show(31113);
	else if (player_said_2(take, lights))
		text_show(31114);
	else if (player_said_2(look, light) || player_said_2(stare_at, light))
		text_show(31115);
	else if (player_said_2(take, light))
		text_show(31116);
	else if (player_said_2(look, corridor_to_west))
		text_show(31117);
	else if (player_said_2(look, corridor_to_east))
		text_show(31118);
	else if (player_said_2(look, air_vent))
		text_show(31120);
	else
		return;

	player.command_ready = false;
}

void room_311_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._checkGuardFl);
}

void room_311_preload() {
	room_init_code_pointer = room_311_init;
	room_parser_code_pointer = room_311_parser;
	room_daemon_code_pointer = room_311_daemon;

	if (room_id == 391)
		global[kSexOfRex] = REX_MALE;

	section_3_walker();
	section_3_interface();

	if (room_id == 304)
		*player.series_name = '\0';
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
