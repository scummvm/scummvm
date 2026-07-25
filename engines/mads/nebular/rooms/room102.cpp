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
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	byte _fridgeOpenedFl;
	byte _fridgeOpenedDescr;
	byte _fridgeFirstOpenFl;
	byte _chairDescrFl;
	byte _drawerDescrFl;
	byte _activeMsgFl;

	int16 _fridgeCommentCount;
};

static Scratch local;

static void addRandomMessage() {
	_scene->_kernelMessages.reset();
	kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
	int quoteId = g_engine->getRandomNumber(65, 69);
	_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 73, 120, quote_string(kernel.quotes, quoteId));
	local._activeMsgFl = true;
}

static void room_102_init() {
	section_1_music();

	g_sprite_ids[1] = _scene->_sprites.addSprites(kernel_name('x', 1));
	g_sprite_ids[2] = _scene->_sprites.addSprites(kernel_name('x', 2));
	g_sprite_ids[3] = _scene->_sprites.addSprites(kernel_name('x', 3));
	g_sprite_ids[4] = _scene->_sprites.addSprites(kernel_name('x', 4));
	g_sprite_ids[5] = _scene->_sprites.addSprites(kernel_name('x', 5));
	g_sprite_ids[6] = _scene->_sprites.addSprites(kernel_name('b', -1));
	g_sprite_ids[7] = _scene->_sprites.addSprites(kernel_name('c', -1));
	g_sprite_ids[8] = _scene->_sprites.addSprites(kernel_name('e', -1));
	g_sprite_ids[9] = _scene->_sprites.addSprites(kernel_name('n', -1));
	g_sprite_ids[10] = _scene->_sprites.addSprites(kernel_name('g', -1));
	g_sprite_ids[11] = _scene->_sprites.addSprites("*RXMRC_8");
	g_sprite_ids[13] = _scene->_sprites.addSprites(kernel_name('x', 0));

	g_sequence_ids[1] = _scene->_sequences.startPingPongCycle(g_sprite_ids[1], false, 8, 0, 0, 0);
	g_sequence_ids[2] = _scene->_sequences.addSpriteCycle(g_sprite_ids[2], false, 170, 0, 1, 6);
	g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 11, 0, 2, 3);
	g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4], false, 4, 0, 1, 0);
	g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 3, 0, 0, 5);

	if (object_is_here(OBJ_BINOCULARS))
		g_sequence_ids[9] = _scene->_sequences.addSpriteCycle(g_sprite_ids[9], false, 24, 0, 0, 24);
	else
		_scene->_hotspots.activate(words_binoculars, false);

	_scene->_hotspots.activate(words_burger, false);

	if (global[kMedicineCabinetOpen]) {
		g_sequence_ids[8] = _scene->_sequences.addSpriteCycle(g_sprite_ids[8], false, 6, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[8], -2, -2);
	}

	if (_scene->_priorSceneId == 101) {
		player.x = 229;
		player.y = 109;
		player.commands_allowed = false;
		g_sequence_ids[6] = _scene->_sequences.addReverseSpriteCycle(g_sprite_ids[6], false, 6, 1, 2, 0);
		_scene->_sequences.addSubEntry(g_sequence_ids[6], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
	} else if (_scene->_priorSceneId == 103) {
		player.x = 47;
		player.y = 152;
	}
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.facing = FACING_NORTHWEST;
		player.x = 32;
		player.y = 129;
	}

	if (_scene->_priorSceneId != 106) {
		if (global[kWaterInAPuddle]) {
			g_sequence_ids[13] = _scene->_sequences.addSpriteCycle(g_sprite_ids[13], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[13], -2, -2);
			_scene->_sequences.setDepth(g_sequence_ids[13], 5);
		}
	} else {
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[13] = _scene->_sequences.addSpriteCycle(g_sprite_ids[13], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(g_sequence_ids[13], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		_scene->_sequences.setDepth(g_sequence_ids[13], 5);
		g_engine->_soundManager->command(24, 0);
		g_engine->_soundManager->command(28, 0);
	}

	local._fridgeOpenedFl = false;
	local._fridgeOpenedDescr = false;
	local._fridgeCommentCount = 0;
	local._fridgeFirstOpenFl = true;
	local._chairDescrFl = false;
	local._activeMsgFl = false;

	kernel.quotes = quote_load(0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0);

	if (_scene->_priorSceneId == 101)
		g_engine->_soundManager->command(20, 0);
}

static void room_102_daemon() {
	if (kernel.trigger == 70)
		player.commands_allowed = true;

	if (kernel.trigger == 72) {
		g_sequence_ids[13] = _scene->_sequences.addSpriteCycle(g_sprite_ids[13], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[13], -2, -2);
		_scene->_sequences.setDepth(g_sequence_ids[13], 5);
		_scene->_sequences.addTimer(48, 90);
	}

	if (kernel.trigger >= 90) {
		if (kernel.trigger >= 94) {
			_scene->loadAnimation(kernel_name('B', -1), 71);
			player.commands_allowed = false;
			player.walker_visible = false;

			global[kWaterInAPuddle] = true;
			g_engine->_soundManager->command(24, 0);
		} else {
			g_engine->_soundManager->command(23, 0);
			_scene->_sequences.addTimer(48, kernel.trigger + 1);
		}
	}

	if (kernel.trigger == 71) {
		player.commands_allowed = true;
		player.walker_visible = true;
		player.clock = _scene->_frameStartTime - player.frame_delay;
	}

	if (local._fridgeOpenedFl && !local._fridgeOpenedDescr) {
		local._fridgeCommentCount++;
		if (local._fridgeCommentCount > 16384) {
			local._fridgeOpenedDescr = true;
			text_show(10213);
		}
	}

	if (!local._activeMsgFl && (Common::Point(player.x, player.y) == Common::Point(177, 114)) && (player.facing == FACING_NORTH)
		&& (g_engine->getRandomNumber(1, 5000) == 1)) {
		_scene->_kernelMessages.reset();
		local._activeMsgFl = false;
		addRandomMessage();
	}

	if (kernel.trigger == 73)
		local._activeMsgFl = false;
}

static void room_102_pre_parser() {
	if (player_said_1(refrigerator) || player_said_1(poster))
		player.need_to_walk = player.ready_to_walk;

	if (local._fridgeOpenedFl && !player_said_1(refrigerator)) {
		switch (kernel.trigger) {
		case 0:
			if (player.need_to_walk) {
				_scene->_sequences.remove(g_sequence_ids[7]);
				g_sequence_ids[7] = _scene->_sequences.addReverseSpriteCycle(g_sprite_ids[7], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(g_sequence_ids[7], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setDepth(g_sequence_ids[7], 15);
				player.commands_allowed = false;
				g_engine->_soundManager->command(20, 0);
			}
			break;

		case 1:
			if (object_is_here(OBJ_BURGER)) {
				_scene->_sequences.remove(g_sequence_ids[10]);
				_scene->_hotspots.activate(words_burger, false);
			}
			local._fridgeOpenedFl = false;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	}

	if (player.need_to_walk)
		_scene->_kernelMessages.reset();
}

static void room_102_parser() {
	if (player.look_around) {
		text_show(10234);
		player.command_ready = false;
		return;
	}

	bool justOpenedFl = false;
	if (player_said_1(refrigerator) && !local._fridgeOpenedFl) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[7] = _scene->_sequences.addSpriteCycle(g_sprite_ids[7], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(g_sequence_ids[7], 15);
			_scene->_sequences.addSubEntry(g_sequence_ids[7], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			if (object_is_here(OBJ_BURGER)) {
				g_sequence_ids[10] = _scene->_sequences.addSpriteCycle(g_sprite_ids[10], false, 7, 0, 0, 0);
				_scene->_sequences.setDepth(g_sequence_ids[10], 14);
			}
			player.commands_allowed = false;
			g_engine->_soundManager->command(20, 0);
			player.command_ready = false;
			return;

		case 1:
			g_sequence_ids[7] = _scene->_sequences.addSpriteCycle(g_sprite_ids[7], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[7], -2, -2);
			_scene->_sequences.setDepth(g_sequence_ids[7], 15);
			int delay;
			if (player_said_1(walkto) && !local._fridgeFirstOpenFl)
				delay = 0;
			else
				delay = 48;
			_scene->_sequences.addTimer(delay, 2);
			player.command_ready = false;
			return;

		case 2:
			local._fridgeOpenedFl = true;
			local._fridgeOpenedDescr = false;
			local._fridgeCommentCount = 0;
			player.commands_allowed = true;
			justOpenedFl = true;
			if (object_is_here(OBJ_BURGER))
				_scene->_hotspots.activate(words_burger, true);
			break;

		default:
			break;
		}
	}

	if (player_said_2(look, refrigerator) || player_said_2(open, refrigerator)) {
		if (object_is_here(OBJ_BURGER))
			text_show(10230);
		else
			text_show(10229);

		local._fridgeFirstOpenFl = false;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walkto, refrigerator) && justOpenedFl) {
		local._fridgeFirstOpenFl = false;
		int quoteId = g_engine->getRandomNumber(59, 63);
		const char *curQuote = quote_string(kernel.quotes, quoteId);
		int width = _scene->_kernelMessages._talkFont->getWidth(curQuote, -1);
		_scene->_kernelMessages.reset();
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_kernelMessages.add(Common::Point(210, 60), 0x1110, 0, 73, 120, curQuote);
		_scene->_kernelMessages.add(Common::Point(214 + width, 60), 0x1110, 0, 73, 120, quote_string(kernel.quotes, 64));
		local._activeMsgFl = true;
		player.command_ready = false;
		return;
	}

	if (player_said_2(close, refrigerator)) {
		text_show(10213);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, refrigerator)) {
		text_show(8);
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, door)) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[6] = _scene->_sequences.addSpriteCycle(g_sprite_ids[6], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(g_sequence_ids[6], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(20, 0);
			break;

		case 1:
			_scene->_nextSceneId = 101;
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(walkto, engineering_section)) {
		_scene->_nextSceneId = 103;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walkto, poster) || player_said_2(look, poster) || player_said_2(walkto, binoculars)) {
		addRandomMessage();
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, weight_machine)) {
		text_show(10212);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, engineering_section)) {
		text_show(10205);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, door)) {
		text_show(10204);
		player.command_ready = false;
		return;
	}

	if (player_said_2(stare_at, ceiling) || player_said_2(look, ceiling)) {
		text_show(10203);
		player.command_ready = false;
		return;
	}

	if (player_said_2(stare_at, overhead_lamp) || player_said_2(look, overhead_lamp)) {
		text_show(10202);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, robo_kitchen)) {
		text_show(10215);
		player.command_ready = false;
		return;
	}

	if (player_said_3(put, burger, robo_kitchen) && player_has(OBJ_BURGER)) {
		text_show(10216);
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, refrigerator) && player_has(object_named(player2.words[1]))) {
		text_show(10217);
		player.command_ready = false;
		return;
	}

	if (player_said_3(put, dead_fish, robo_kitchen) || player_said_3(put, stuffed_fish, robo_kitchen)) {
		text_show(10230);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, robo_kitchen)) {
		text_show(10218);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, closet)) {
		text_show(10219);
		player.command_ready = false;
		return;
	}

	if ((player_said_1(ladder) || player_said_1(hatchway)) && (player_said_1(look) || player_said_1(climb_up) || player_said_1(climb_through))) {
		if (player_has(OBJ_REBREATHER)) {
			if (!player_said_1(climb_up) && !player_said_1(climb_through)) {
				text_show(10231);
				player.command_ready = false;
				return;
			}
		} else if (player_said_1(look) || (game.difficulty != DIFFICULTY_HARD)) {
			text_show(10222);
			player.command_ready = false;
			return;
		}
	}

	if ((player_said_1(ladder) || player_said_1(hatchway)) && (player_said_1(climb_up) || player_said_1(climb_through))) {
		switch (kernel.trigger) {
		case 0:
			_scene->loadAnimation(kernel_name('A', -1), 1);
			player.commands_allowed = false;
			player.walker_visible = false;
			break;

		case 1:
			g_engine->_soundManager->command(24, 0);
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
		case 3:
		case 4:
			g_engine->_soundManager->command(23, 0);
			_scene->_sequences.addTimer(48, kernel.trigger + 1);
			break;

		case 5:
			g_engine->_soundManager->command(24, 0);
			_scene->_sequences.addTimer(48, kernel.trigger + 1);
			break;

		case 6:
			if (player_has(OBJ_REBREATHER) && !player_has_been_in_room(106))
				text_show(10237);
			_scene->_nextSceneId = 106;
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, power_status_panel)) {
		text_show(10226);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, window) || player_said_2(look_through, window)) {
		text_show(10227);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, doorway) || player_said_2(walkto, doorway)) {
		text_show(10228);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, drawer) || ((player_said_2(close, drawer) || player_said_2(push, drawer)) && !local._drawerDescrFl)) {
		text_show(10220);
		local._drawerDescrFl = true;
		player.command_ready = false;
		return;
	}

	if (player_said_2(close, drawer) || player_said_2(push, drawer)) {
		text_show(10221);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, drawer)) {
		text_show(10236);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, chair) || (player_said_2(sit_in, chair) && !local._chairDescrFl)) {
		local._chairDescrFl = true;
		text_show(10210);
		player.command_ready = false;
		return;
	}

	if (player_said_2(sit_in, chair)) {
		text_show(10211);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, medicine_cabinet)) {
		if (global[kMedicineCabinetOpen])
			text_show(10207);
		else
			text_show(10206);

		player.command_ready = false;
		return;
	}

	if (player_said_2(close, medicine_cabinet) && global[kMedicineCabinetOpen]) {
		switch (kernel.trigger) {
		case 0:
			_scene->_sequences.remove(g_sequence_ids[8]);
			g_sequence_ids[8] = _scene->_sequences.addReverseSpriteCycle(g_sprite_ids[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(21, 0);
			break;

		case 1:
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
			player.commands_allowed = true;
			global[kMedicineCabinetOpen] = false;
			text_show(10209);
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, medicine_cabinet) && !global[kMedicineCabinetOpen]) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[8] = _scene->_sequences.addSpriteCycle(g_sprite_ids[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(g_sequence_ids[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(21, 0);
			break;

		case 1:
			g_sequence_ids[8] = _scene->_sequences.addSpriteCycle(g_sprite_ids[8], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[8], -2, -2);
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
			player.commands_allowed = true;
			global[kMedicineCabinetOpen] = true;
			if (global[kMedicineCabinetVirgin]) {
				text_show(10208);
			} else {
				text_show(10207);
			}
			global[kMedicineCabinetVirgin] = false;
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, binoculars) && object_is_here(OBJ_BINOCULARS)) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[11] = _scene->_sequences.startPingPongCycle(g_sprite_ids[11], false, 3, 1, 0, 0);
			_scene->_sequences.setMsgLayout(g_sequence_ids[11]);
			_scene->_sequences.addSubEntry(g_sequence_ids[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			player.walker_visible = false;
			player.commands_allowed = false;
			break;

		case 1:
			inter_give_to_player(OBJ_BINOCULARS);
			_scene->_sequences.remove(g_sequence_ids[9]);
			_scene->_hotspots.activate(words_binoculars, false);
			player.walker_visible = true;
			player.commands_allowed = true;
			g_engine->_soundManager->command(22, 0);
			object_examine(OBJ_BINOCULARS, 10201, 0);
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, burger) && object_is_here(OBJ_BURGER)) {
		if (kernel.trigger == 0) {
			object_examine(OBJ_BURGER, 10235, 0);
			_scene->_sequences.remove(g_sequence_ids[10]);
			inter_give_to_player(OBJ_BURGER);
			_scene->_hotspots.activate(words_burger, false);
			g_engine->_soundManager->command(22, 0);
			player.walker_visible = true;
			player.commands_allowed = true;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, poster)) {
		text_show(10224);
		player.command_ready = false;
		return;
	}

	if ((player_said_1(push) || player_said_1(pull)) && player_said_1(weight_machine)) {
		text_show(10225);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, floor)) {
		text_show(10232);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, binoculars) && !player_has(OBJ_BINOCULARS)) {
		text_show(10233);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, burger) && (player.main_object_source == CAT_HOTSPOT)) {
		text_show(801);
		player.command_ready = false;
	}
}

static void room_102_error() {
	if (player_said_2(put, robo_kitchen) && player_has(object_named(player2.words[1]))) {
		text_show(10217);
		player.command_ready = false;
	}
}

void room_102_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._fridgeOpenedFl);
	s.syncAsByte(local._fridgeOpenedDescr);
	s.syncAsByte(local._fridgeFirstOpenFl);
	s.syncAsByte(local._chairDescrFl);
	s.syncAsByte(local._drawerDescrFl);
	s.syncAsByte(local._activeMsgFl);

	s.syncAsSint16LE(local._fridgeCommentCount);
}

void room_102_preload() {
	room_init_code_pointer = room_102_init;
	room_pre_parser_code_pointer = room_102_pre_parser;
	room_parser_code_pointer = room_102_parser;
	room_daemon_code_pointer = room_102_daemon;
	room_error_code_pointer = room_102_error;

	anim_himem_preload(kernel_name('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
