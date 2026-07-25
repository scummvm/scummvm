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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/forcefield.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	Forcefield _forcefield;
};

static Scratch local;


static void room_308_init() {
	g_sprite_ids[1] = kernel_load_series("*SC003x0", 0);
	g_sprite_ids[0] = kernel_load_series("*SC003x1", 0);
	g_sprite_ids[2] = kernel_load_series("*SC003x2", 0);

	init_forcefield(&local._forcefield, true);

	g_sprite_ids[3] = kernel_load_series(kernel_name('b', 0), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_full_name(307, 'X', 0, "", EXT_SS), 0);

	pal_change_color(252, 63, 30, 20);
	pal_change_color(253, 45, 15, 12);

	g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 1);
	_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(g_sequence_ids[4], 15);
	g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 1);
	_scene->_sequences.setDepth(g_sequence_ids[3], 9);
	_scene->_sequences.addTimer(48, 70);

	player.walker_visible = false;
	player.commands_allowed = false;
	kernel_run_animation(kernel_name('a', -1), 60);

	section_3_music();
	kernel.quotes = quote_load(0xF4, 0xF5, 0xF6, 0);
}

static void room_308_daemon() {
	handle_forcefield(&local._forcefield, &g_sprite_ids[0]);

	if (kernel.trigger == 60)
		new_room = 307;

	if (kernel.trigger < 70)
		return;

	switch (kernel.trigger) {
	case 70:
	{
		_scene->_sequences.remove(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 18, 9, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 2, 3);
		_scene->_sequences.setDepth(g_sequence_ids[3], 9);
		kernel_message_purge();
		int idx = kernel_message_add(quote_string(kernel.quotes, 244), 171, 21, 0xFDFC, 120, 0, 0);
		kernel_message_teletype(idx, 2, true);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	}
	break;

	case 71:
	{
		int seqIdx = g_sequence_ids[3];
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 4);
		_scene->_sequences.setDepth(g_sequence_ids[3], 9);
		_scene->_sequences.updateTimeout(g_sequence_ids[3], seqIdx);
		_scene->_sequences.addTimer(48, 72);
	}
	break;

	case 72:
		_scene->_sequences.remove(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 20, 5, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 3, 4);
		_scene->_sequences.setDepth(g_sequence_ids[3], 9);
		kernel_message_purge();
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		break;

	case 73:
	{
		int seqIdx = g_sequence_ids[3];
		g_sequence_ids[3] = _scene->_sequences.startCycle(g_sprite_ids[3], false, 5);
		_scene->_sequences.setDepth(g_sequence_ids[3], 9);
		_scene->_sequences.updateTimeout(g_sequence_ids[3], seqIdx);
		_scene->_sequences.addTimer(48, 74);
	}
	break;

	case 74:
	{
		_scene->_sequences.remove(g_sequence_ids[3]);
		g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 20, 8, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 6, 7);
		_scene->_sequences.setDepth(g_sequence_ids[3], 9);
		kernel_message_purge();
		int idx = kernel_message_add(quote_string(kernel.quotes, 245), 171, 21, 0xFDFC, 120, 0, 0);
		kernel_message_teletype(idx, 2, true);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
	}
	break;

	case 75:
	{
		int seqIdx = g_sequence_ids[3];
		g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 23, 5, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 8, 10);
		_scene->_sequences.setDepth(g_sequence_ids[3], 9);
		_scene->_sequences.updateTimeout(g_sequence_ids[3], seqIdx);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
	}
	break;

	case 76:
	{
		int seqIdx = g_sequence_ids[3];
		g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 26, 0, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 2, 3);
		_scene->_sequences.setDepth(g_sequence_ids[3], 9);
		kernel_message_purge();
		int idx = kernel_message_add(quote_string(kernel.quotes, 246), 171, 21, 0xFDFC, 120, 0, 0);
		kernel_message_teletype(idx, 2, true);
		_scene->_sequences.updateTimeout(g_sequence_ids[3], seqIdx);
	}
	break;

	default:
		break;
	}
}

void room_308_synchronize(Common::Serializer &s) {
	local._forcefield.synchronize(s);
}

void room_308_preload() {
	local._forcefield.init();

	room_init_code_pointer = room_308_init;
	room_daemon_code_pointer = room_308_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
