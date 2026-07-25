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
#include "mads/core/quote.h"
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
	bool _kargShootingFl;
	bool _loseFl;
};

static Scratch local;

static void room_104_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('h', -1));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 0, 0, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 8);

	if (_scene->_priorSceneId == 105) {
		player.x = 302;
		player.y = 107;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 160;
		player.y = 134;
	}

	local._loseFl = false;
	kernel.quotes = quote_load(0x35, 0x34, 0);
	local._kargShootingFl = false;

	if (g_engine->getRandomNumber(1, 3) == 1) {
		_scene->loadAnimation(kernel_full_name(104, 'B', -1, "", EXT_AA), 0);
		local._kargShootingFl = true;
	}

	section_1_music();
}

static void room_104_daemon() {
	if ((Common::Point(player.x, player.y) == Common::Point(189, 70)) && (kernel.trigger || !local._loseFl)) {
		if (player.facing == FACING_SOUTHWEST || player.facing == FACING_SOUTHEAST)
			player.facing = FACING_SOUTH;

		if (player.facing == FACING_NORTHWEST || player.facing == FACING_NORTHEAST)
			player.facing = FACING_NORTH;

		bool mirrorFl = false;
		if (player.facing == FACING_WEST) {
			player.facing = FACING_EAST;
			mirrorFl = true;
		}

		local._loseFl = true;

		switch (player.facing) {
		case FACING_EAST:
			switch (kernel.trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				player.commands_allowed = false;
				player.walker_visible = false;
				_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('a', 0));
				kernel_new_palette();
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], mirrorFl, 7, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				text_show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;

		case FACING_SOUTH:
			switch (kernel.trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				player.commands_allowed = false;
				player.walker_visible = false;
				_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('a', 1));
				kernel_new_palette();
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 14);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 15, 32);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 3, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -2, -2);
				_scene->_sequences.addTimer(90, 3);
				break;

			case 3:
				text_show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;

		case FACING_NORTH:
			switch (kernel.trigger) {
			case 0:
				_scene->_kernelMessages.reset();
				_scene->freeAnimation();
				_scene->resetScene();
				player.commands_allowed = false;
				player.walker_visible = false;
				_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('a', 2));
				kernel_new_palette();
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				if (config_file.naughtiness >= STORYMODE_NICE)
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 15, 2);
				break;

			case 1:
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 0, 0, 0);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(198, 143));
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -2, -2);
				_scene->_sequences.addTimer(90, 2);
				break;

			case 2:
				text_show(10406);
				_scene->_reloadSceneFlag = true;
				break;

			default:
				break;
			}
			break;
		default:
			break;
		}

		if (!kernel.trigger)
			g_engine->_soundManager->command(34, 0);
	}

	if (player.walking && (_scene->_rails.getNext() > 0)) {
		player_cancel_command();
		player_start_walking(189, 70, FACING_NONE);
		_scene->_rails.resetNext();
	}

	if ((player.special_code > 0) && player.commands_allowed)
		player.commands_allowed = false;

	if (local._kargShootingFl && (_scene->_animation[0]->getCurrentFrame() >= 19)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 52));
		local._kargShootingFl = false;
	}
}

static void room_104_pre_parser() {
	if (player_said_2(swim_towards, eastern_cliff_face))
		player.walk_off_edge_to_room = 105;

	if (player_said_2(swim_towards, open_area_to_south))
		player.walk_off_edge_to_room = 106;
}

static void room_104_parser() {
	if (_action._lookFlag)
		text_show(10405);
	else if (player_said_2(look, curious_weed_patch))
		text_show(10404);
	else if (player_said_2(look, surface))
		text_show(10403);
	else if (player_said_2(look, cliff_face))
		text_show(10401);
	else if (player_said_2(look, ocean_floor))
		text_show(10402);
	else
		return;

	_action._inProgress = false;
}

void room_104_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._kargShootingFl);
	s.syncAsByte(local._loseFl);
}

void room_104_preload() {
	room_init_code_pointer = room_104_init;
	room_pre_parser_code_pointer = room_104_pre_parser;
	room_parser_code_pointer = room_104_parser;
	room_daemon_code_pointer = room_104_daemon;

	anim_himem_preload(kernel_name('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
