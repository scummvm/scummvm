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

#include "math/utils.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section5.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _frame;
	int16 _nextButtonId;
	int16 _homeSelectedId;
	int16 _selectedId;
	int16 _activeCars;
	int16 _carLocations[9];
};

static Scratch local;


static void room_505_init() {
	for (int i = 0; i < 9; i++)
		g_sprite_ids[i] = kernel_load_series(kernel_name('a', i + 1), 0);

	g_sprite_ids[13] = kernel_load_series(kernel_name('b', 1), 0);
	g_sprite_ids[9] = kernel_load_series(kernel_name('g', 1), 0);
	g_sprite_ids[10] = kernel_load_series(kernel_name('g', 0), 0);
	g_sprite_ids[11] = kernel_load_series(kernel_name('t', -1), 0);
	g_sprite_ids[12] = kernel_load_series(kernel_name('e', -1), 0);

	if (previous_room != KERNEL_RESTORING_GAME)
		g_sequence_ids[12] = kernel_seq_backward(g_sprite_ids[12], false, 6, 0, 0, 1);

	g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 6, 0, 120, 1);
	kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 60);
	kernel_timing_trigger(30, 62);

	local._carLocations[0] = 501;
	local._carLocations[1] = 506;
	local._carLocations[2] = 511;
	local._carLocations[3] = 513;
	local._carLocations[4] = 601;
	local._carLocations[5] = 604;
	local._carLocations[6] = 607;
	local._carLocations[7] = 609;
	local._carLocations[8] = 612;

	local._activeCars = false;

	for (int i = 0; i < 9; i++) {
		if (global[kHoverCarLocation] == local._carLocations[i]) {
			local._homeSelectedId = i;
			if (previous_room != KERNEL_RESTORING_GAME)
				local._selectedId = i;
		}
	}

	player.walker_visible = false;
	player.commands_allowed = false;
	local._frame = -1;
	kernel_run_animation(kernel_name('a', -1), 0);
	kernel_reset_animation(0, 86);

	section_5_music();
	g_engine->_soundManager->command(16, 0);
}

static void room_505_daemon() {
	if (local._frame != kernel_anim[0].frame) {
		local._frame = kernel_anim[0].frame;
		int resetFrame = -1;

		switch (local._frame) {
		case 4:
		case 24:
		case 33:
		case 53:
		case 62:
		case 82:
			if (local._nextButtonId == 0x38A)
				resetFrame = 4;
			else if (local._nextButtonId == 0x38B)
				resetFrame = 33;
			else if (local._nextButtonId == 0x2DE)
				resetFrame = 62;

			break;

		case 15:
		case 44:
		case 73:
		{
			int this_button;
			int old_select;
			g_engine->_soundManager->command(17, 0);
			old_select = local._selectedId;
			if (local._frame == 15) {
				this_button = 0x38A;
				local._selectedId = (local._selectedId + 1) % 9;
			} else if (local._frame == 44) {
				this_button = 0x38B;
				local._selectedId--;
				if (local._selectedId < 0)
					local._selectedId = 8;
			} else {
				this_button = 0x2DE;
				if ((global[kTimebombStatus] == TIMEBOMB_ACTIVATED) && (local._carLocations[local._selectedId] == 501))
					text_show(431);
				else if (local._selectedId != local._homeSelectedId) {
					local._nextButtonId = 0;
					local._activeCars = true;
					player.commands_allowed = false;
					kernel_seq_delete(g_sequence_ids[1]);
					kernel_seq_delete(g_sequence_ids[0]);
					kernel_seq_delete(g_sequence_ids[13]);
					g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[13], false, 6, 0, 0, 1);
					kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 63);
					g_engine->_soundManager->command(18, 0);
				}
			}

			if (local._nextButtonId == this_button)
				local._nextButtonId = 0;

			if (old_select != local._selectedId) {
				kernel_seq_delete(g_sequence_ids[11]);
				g_sequence_ids[11] = kernel_seq_stamp(g_sprite_ids[11], false, local._selectedId + 1);
				if (old_select != local._homeSelectedId)
					kernel_seq_delete(g_sequence_ids[0]);

				if (local._selectedId != local._homeSelectedId) {
					g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0 + local._selectedId], false, 24, 0, 0, 0);
					kernel_seq_depth(g_sequence_ids[0], 1);
				}
			}
			break;
		}

		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			if (local._nextButtonId == 0x38A)
				resetFrame = 29 - local._frame;

			break;

		case 26:
		case 55:
		case 84:
			if (local._nextButtonId != 0)
				resetFrame = 3;

			break;

		case 27:
		case 56:
		case 85:
			if (local._nextButtonId != 0)
				resetFrame = 2;

			break;

		case 29:
		case 58:
		case 87:
			if (local._activeCars)
				global[kHoverCarDestination] = local._carLocations[local._selectedId];

			if (local._nextButtonId == 0x38A)
				resetFrame = 0;
			else if (local._nextButtonId == 0x38B)
				resetFrame = 29;
			else if (local._nextButtonId == 0x2DE)
				resetFrame = 58;
			else
				resetFrame = 86;
			break;

		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
			if (local._nextButtonId == 0x38B)
				resetFrame = 87 - local._frame;

			break;

		case 76:
		case 77:
		case 78:
		case 79:
		case 80:
		case 81:
			if (local._nextButtonId == 0x2DE)
				resetFrame = 145 - local._frame;

			break;

		default:
			break;
		}

		if ((resetFrame >= 0) && (resetFrame != kernel_anim[0].frame)) {
			kernel_reset_animation(0, resetFrame);
			local._frame = resetFrame;
		}
	}

	switch (kernel.trigger) {
	case 60:
	{
		player.commands_allowed = true;
		int syncIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, -2);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(syncIdx, g_sequence_ids[13]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[local._homeSelectedId], false, 1);
		kernel_seq_depth(g_sequence_ids[1], 1);
		g_sequence_ids[11] = kernel_seq_stamp(g_sprite_ids[11], false, local._selectedId + 1);

		if (local._selectedId != local._homeSelectedId) {
			g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0 + local._selectedId], false, 24, 0, 0, 0);
			kernel_seq_depth(g_sequence_ids[0], 1);
		}
		break;
	}

	case 61:
		g_sequence_ids[10] = kernel_seq_forward(g_sprite_ids[10], false, 8, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[10], 8);
		kernel_seq_timeout(g_sequence_ids[9], g_sequence_ids[10]);
		break;

	case 62:
		g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 8, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[9], 8);
		kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 61);
		break;

	case 63:
		global[kHoverCarDestination] = local._carLocations[local._selectedId];
		new_room = 504;
		break;

	default:
		break;
	}
}

static void room_505_parser() {
	if (player_said_1(press))
		local._nextButtonId = player2.words[1];
	else if (player_said_2(return_to, inside_of_car))
		new_room = 504;
	else if (player_said_2(look, view_screen))
		text_show(50510);
	else if (player_said_2(look, control_panel))
		text_show(50511);
	else
		return;

	player.command_ready = false;
}

void room_505_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._frame);
	s.syncAsSint16LE(local._nextButtonId);
	s.syncAsSint16LE(local._homeSelectedId);
	s.syncAsSint16LE(local._selectedId);
	s.syncAsSint16LE(local._activeCars);

	for (int i = 0; i < 9; i++)
		s.syncAsSint16LE(local._carLocations[i]);
}


void room_505_preload() {
	room_init_code_pointer = room_505_init;
	room_daemon_code_pointer = room_505_daemon;
	room_parser_code_pointer = room_505_parser;

	*player.series_name = '\0';
	section_5_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
