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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/magic.h"
#include "mads/core/mcga.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _animMode;
	int32 _animFrame;
	int32 _nextAction1;
	int32 _nextAction2;
	int32 _slacheMode;
	int32 _slacheTopic;
	int32 _slachePosY;
	bool _slacheTalkingFl;
	bool _slacheReady;
	bool _slacheInitFl;
	char _subQuote2[256];
	Dialog _dialog1;
	Dialog _dialog2;
	Dialog _dialog3;
};

static Scratch local;


static void handleRexDialogues(int quote) {
	kernel_message_purge();

	char *curQuote = quote_string(kernel.quotes, quote);
	if (font_string_width(kernel_message_font, curQuote, kernel_message_spacing) > 200) {
		static char subQuote1[34], subQuote2[34];
		quote_split_string(curQuote, subQuote1, subQuote2);
		Common::strcpy_s(local._subQuote2, subQuote2);

		kernel_message_add(subQuote1, 160, 106, 0x1110, 120, 0, 32);
		kernel_message_add(local._subQuote2, 160, 120, 0x1110, 120, 1, 32);
	} else
		kernel_message_add(curQuote, 160, 120, 0x1110, 120, 1, 32);
}

static void handleSlacheDialogs(int quoteId, int counter, uint32 timer) {
	int curQuote = quoteId;
	int posY = 5 + (local._slachePosY * 14);

	for (int count = 0; count < counter; count++, curQuote++) {
		kernel_message_add(quote_string(kernel.quotes, curQuote), 8, posY, 0xFDFC, timer, 0, 0);
		posY += 14;
	}
}

static void room_319_init() {
	g_sprite_ids[5] = kernel_load_series(kernel_name('e', 0), 0);
	g_sprite_ids[0] = kernel_load_series(kernel_name('a', 0), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('a', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('a', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('a', 3), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('k', -1), 0);

	if (!player_has(OBJ_SCALPEL)) {
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
		kernel_seq_depth(g_sequence_ids[4], 1);
	}

	g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 18, 300, 0, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 67, 377, 0, 0);
	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 173, 233, 0, 0);
	kernel_seq_depth(g_sequence_ids[2], 14);
	kernel_seq_depth(g_sequence_ids[3], 14);

	g_sequence_ids[0] = kernel_seq_stamp(g_sprite_ids[0], false, 1);
	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);

	local._dialog1.setup(0x43, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0);
	local._dialog2.setup(0x44, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0);
	local._dialog3.setup(0x45, 0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local._dialog1.set(0x165, 0x166, 0x167, 0x168, 0);
		local._dialog2.set(0x171, 0x172, 0x173, 0x174, 0);
		local._dialog3.set(0x17D, 0x17E, 0x17F, 0x180, 0);
	}

	kernel.quotes = quote_load(351, 352, 353, 354, 355, 356, 363, 364, 365,
		366, 367, 368, 375, 376, 377, 378, 379, 380, 357, 358,
		359, 360, 361, 362, 369, 370, 371, 372, 373, 374, 381,
		382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392,
		393, 394, 395, 0);

	pal_change_color(252, 63, 30, 2);
	pal_change_color(253, 45, 15, 1);

	local._slachePosY = 0;
	local._slacheInitFl = false;
	local._slacheTalkingFl = false;
	local._slacheReady = false;
	local._animFrame = 0;

	kernel_run_animation(kernel_name('b', 0), 0);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local._animMode = 1;
		local._nextAction1 = 2;
		local._nextAction2 = 2;
		local._slacheMode = 1;
		local._slacheTopic = 1;
		local._slacheInitFl = true;

		if (global[kRexHasMetSlache]) {
			handleSlacheDialogs(words_walk_outside, 2, INDEFINITE_TIMEOUT);
			local._slachePosY = 3;
		} else {
			handleSlacheDialogs(0x186, 4, INDEFINITE_TIMEOUT);
			local._slachePosY = 5;
		}
	}

	switch (local._slacheTopic) {
	case 1:
		handleSlacheDialogs(0x15F, 2, INDEFINITE_TIMEOUT);
		local._dialog1.start();
		break;

	case 2:
		handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
		local._dialog2.start();
		break;

	case 3:
		handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
		local._dialog3.start();
		break;

	default:
		break;
	}

	local._slachePosY = 0;
	section_3_music();
}

static void room_319_daemon() {
	if (kernel_anim[0].anim == nullptr)
		return;

	if (local._animFrame != kernel_anim[0].frame) {
		local._animFrame = kernel_anim[0].frame;
		int nextFrame = -1;
		if (local._animMode == 1) {
			switch (local._animFrame) {
			case 6:
				local._slacheTalkingFl = true;
				break;

			case 29:
				local._slacheReady = true;
				break;

			case 39:
				if (local._slacheInitFl) {
					local._slacheInitFl = false;
					if (local._nextAction1 == 2)
						nextFrame = 0;
				} else if (local._nextAction1 == 2)
					local._nextAction1 = 1;
				break;

			case 50:
			case 60:
			case 70:
			case 85:
				if (local._nextAction1 == 2)
					nextFrame = 0;
				else if (local._nextAction1 == 3) {
					nextFrame = 85;
					local._slacheTalkingFl = true;
				} else if (local._animFrame == 85) {
					if (!player.commands_allowed)
						local._slacheTalkingFl = true;
					nextFrame = 40;
				}
				break;

			case 115:
				local._slacheReady = true;
				break;

			case 129:
				if (local._nextAction1 == 3) {
					nextFrame = 115;
					if (!player.commands_allowed)
						local._slacheTalkingFl = true;
				}
				break;

			case 145:
				nextFrame = 40;
				break;

			default:
				break;
			}

			if ((local._animFrame > 40) && (local._animFrame < 85) && (nextFrame < 0)) {
				switch (local._nextAction1) {
				case 4:
					local._animFrame = 0;
					kernel_abort_animation(0);
					local._animMode = 2;
					kernel_run_animation(kernel_name('b', 3), 70);
					break;

				case 5:
					local._animFrame = 0;
					kernel_abort_animation(0);
					local._animMode = 3;
					kernel_run_animation(kernel_name('b', 4), 71);
					break;

				case 6:
					local._animFrame = 0;
					kernel_abort_animation(0);
					local._animMode = 4;
					kernel_run_animation(kernel_name('b', 5), 72);
					break;

				default:
					break;
				}

				if (!local._animFrame) {
					kernel_seq_delete(g_sequence_ids[0]);
					kernel_seq_delete(g_sequence_ids[1]);

					for (int i = 0; i <= 1; i++) {
						g_sequence_ids[i] = kernel_seq_forward(g_sprite_ids[i], false, 8, 0, 0, 1);
						kernel_seq_range(g_sequence_ids[i], 1, 7);
					}
					kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_EXPIRE, 0, 73);
				}
			}
		}

		if (local._animMode == 2) {
			if (local._animFrame == 13)
				mcga_shakes = 40;

			if (local._animFrame == 16)
				mcga_shakes = 1;
		}

		if (local._animMode == 3) {
			if (local._animFrame == 11)
				mcga_shakes = 60;

			if (local._animFrame == 18)
				mcga_shakes = 1;
		}

		if ((local._animMode == 4) && (local._animFrame == 16))
			mcga_shakes = 80;

		if ((nextFrame >= 0) && (nextFrame != kernel_anim[0].frame)) {
			kernel_reset_animation(0, nextFrame);
			local._animFrame = nextFrame;
		}
	}

	switch (kernel.trigger) {
	case 70:
	case 71:
	{
		local._animMode = 1;
		local._nextAction1 = local._nextAction2;
		local._animFrame = 0;
		kernel_abort_animation(0);
		kernel_run_animation(kernel_name('b', 0), 0);
		if (local._nextAction1 == 3)
			kernel_reset_animation(0, 85);
		else if (local._nextAction1 == 1)
			kernel_reset_animation(0, 40);

		local._animFrame = kernel_anim[0].frame;
		local._slacheTalkingFl = true;
		mcga_shakes = 1;

		for (int i = 0; i <= 1; i++) {
			int oldIdx = g_sequence_ids[i];
			kernel_seq_delete(g_sequence_ids[i]);
			g_sequence_ids[i] = kernel_seq_forward(g_sprite_ids[i], false, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[i], 8, 13);
			kernel_seq_timeout(oldIdx, g_sequence_ids[i]);
		}
		kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_EXPIRE, 0, 74);

		// WORKAROUND: This fixes the game sometimes going into an endless waiting
		// loop even after the doctor has finished hitting Rex. Note sure if it's due
		// to a bug in room script or in the engine, but this at least fixes it
		int seqIndex = kernel_seq_find_by_trigger(2);
		sequence_list[seqIndex].expired = false;
		break;
	}

	case 72:
		magic_set_color_flags(0xFF, 0, 0);
		magic_set_color_values(0, 0, 0);
		magic_fade_to_grey(master_palette, nullptr, 18, 228,
			248, 0, 1, 16);
		mcga_shakes = 1;
		kernel.force_restart = true;
		break;

	case 73:
		for (int i = 0; i <= 1; i++) {
			int oldIdx = g_sequence_ids[i];
			kernel_seq_delete(g_sequence_ids[i]);
			g_sequence_ids[i] = kernel_seq_forward(g_sprite_ids[i], false, 8, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[i], 6, 7);
			kernel_seq_timeout(oldIdx, g_sequence_ids[i]);
		}
		break;

	case 74:
		for (int i = 0; i <= 1; i++) {
			int oldIdx = g_sequence_ids[i];
			kernel_seq_delete(g_sequence_ids[i]);
			g_sequence_ids[i] = kernel_seq_stamp(g_sprite_ids[i], false, 1);
			kernel_seq_timeout(oldIdx, g_sequence_ids[i]);
		}
		break;

	default:
		break;
	}
}

static void room_319_parser() {
	if (kernel.trigger == 0) {
		player.commands_allowed = false;
		handleRexDialogues(player2.words[0]);
	} else {
		if ((player2.words[0] == 0x165) || (player2.words[0] == 0x166)) {
			if (kernel.trigger == 1) {
				local._nextAction1 = 3;
				local._slacheTalkingFl = false;
				local._slacheMode = 1;
				local._slacheTopic = 2;
			}

			if (!local._slacheTalkingFl) {
				kernel_timing_trigger(4, 2);
			} else {
				handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
				local._dialog2.start();
				player.commands_allowed = true;
			}
		}

		if ((player2.words[0] == 0x171) || (player2.words[0] == 0x172)) {
			if (kernel.trigger == 1) {
				local._nextAction1 = 2;
				local._slacheTalkingFl = false;
				local._slacheMode = 1;
				local._slacheTopic = 3;
			}

			if (!local._slacheTalkingFl) {
				kernel_timing_trigger(4, 2);
			} else {
				handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
				local._dialog3.start();
				player.commands_allowed = true;
			}
		}

		if ((player2.words[0] == 0x17D) || (player2.words[0] == 0x17E)) {
			if (kernel.trigger == 1) {
				local._nextAction1 = 3;
				local._slacheTalkingFl = false;
				local._slacheReady = false;
				local._slacheMode = 1;
				local._slacheTopic = 1;
			}

			if (!local._slacheTalkingFl) {
				kernel_timing_trigger(4, 2);
			} else {
				if (kernel.trigger == 2)
					handleSlacheDialogs(0x184, 2, 180);

				if (!local._slacheReady) {
					kernel_timing_trigger(120, 3);
				} else {
					global[kRexHasMetSlache] = true;
					new_room = 318;
				}
			}
		}

		if ((player2.words[0] == 0x168) || (player2.words[0] == 0x174) ||
			(player2.words[0] == 0x180) || (player2.words[0] == 0x169) ||
			(player2.words[0] == 0x175) || (player2.words[0] == 0x181) ||
			(player2.words[0] == 0x16A) || (player2.words[0] == 0x176) ||
			(player2.words[0] == 0x182) || (player2.words[0] == 0x183) ||
			(player2.words[0] == 0x167) || (player2.words[0] == 0x173) ||
			(player2.words[0] == 0x17F)) {

			bool addDialogLine = !((player2.words[0] == 0x167) || (player2.words[0] == 0x173) ||
				(player2.words[0] == 0x17F) || (player2.words[0] == 0x16A) ||
				(player2.words[0] == 0x176) || (player2.words[0] == 0x182) ||
				(player2.words[0] == 0x183));

			int addVerbId = player2.words[0] + 1;
			if ((addVerbId == 0x182) && (config_file.naughtiness != NAUGHTY))
				addVerbId = 0x183;

			if (local._slacheMode == 1) {
				if (kernel.trigger == 1) {
					local._nextAction2 = local._nextAction1;
					local._nextAction1 = 4;
				}

				if (local._nextAction1 != local._nextAction2) {
					kernel_timing_trigger(4, 2);
				} else {
					Dialog *curDialog;
					int nextDocQuote;
					if ((player2.words[0] == 0x168) || (player2.words[0] == 0x167)) {
						curDialog = &local._dialog1;
						nextDocQuote = 0x161;
					} else if ((player2.words[0] == 0x174) || (player2.words[0] == 0x1753)) {
						nextDocQuote = 0x16D;
						curDialog = &local._dialog2;
					} else {
						nextDocQuote = 0x179;
						curDialog = &local._dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(player2.words[0], false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					player.commands_allowed = true;
					local._slacheMode = 2;
				}
			} else if (local._slacheMode == 2) {
				if (kernel.trigger == 1) {
					local._nextAction2 = local._nextAction1;
					local._nextAction1 = 5;
				}

				if (local._nextAction1 != local._nextAction2) {
					kernel_timing_trigger(4, 2);
				} else {
					Dialog *curDialog;
					int nextDocQuote;
					if ((player2.words[0] == 0x168) || (player2.words[0] == 0x169) || (player2.words[0] == 0x167)) {
						curDialog = &local._dialog1;
						nextDocQuote = 0x163;
					} else if ((player2.words[0] == 0x174) || (player2.words[0] == 0x175) || (player2.words[0] == 0x173)) {
						nextDocQuote = 0x16F;
						curDialog = &local._dialog2;
					} else {
						nextDocQuote = 0x17B;
						curDialog = &local._dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(player2.words[0], false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					player.commands_allowed = true;
					local._slacheMode = 3;
				}
			} else {
				local._nextAction2 = local._nextAction1;
				local._nextAction1 = 6;
			}
		}
	}

	player.command_ready = false;
}

void room_319_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._animMode);
	s.syncAsUint32LE(local._animFrame);
	s.syncAsUint32LE(local._nextAction1);
	s.syncAsUint32LE(local._nextAction2);
	s.syncAsUint32LE(local._slacheMode);
	s.syncAsUint32LE(local._slacheTopic);
	s.syncAsUint32LE(local._slachePosY);

	s.syncAsByte(local._slacheTalkingFl);
	s.syncAsByte(local._slacheReady);
	s.syncAsByte(local._slacheInitFl);

	Common::String subQuote2 = local._subQuote2;
	s.syncString(subQuote2);
	if (s.isLoading())
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());
}


void room_319_preload() {
	room_init_code_pointer = room_319_init;
	room_parser_code_pointer = room_319_parser;
	room_daemon_code_pointer = room_319_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
