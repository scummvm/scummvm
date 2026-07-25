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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _cardId;
};

static Scratch local;


static void room_752_init() {
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(kernel_name('l', -1));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_8");

	if (_scene->_priorSceneId == 751) {
		player.x = 13;
		player.y = 145;
		player.facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 289;
		player.y = 138;
		player_walk(262, 148, FACING_WEST);
		player.facing = FACING_WEST;
		player.walker_visible = true;
	}

	if (object[OBJ_ID_CARD].location == 752) {
		_globals._spriteIndexes[13] = _scene->_sprites.addSprites(kernel_name('i', -1));
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		int idx = _scene->_dynamicHotspots.add(words_id_card, words_walkto, _globals._sequenceIndexes[13], Common::Rect(0, 0, 0, 0));
		local._cardId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 135), FACING_NORTH);
	}

	if (_globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[14] = _scene->_sequences.startCycle(_globals._spriteIndexes[14], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 13);
		int idx = _scene->_dynamicHotspots.add(words_laser_beam, words_look_at, _globals._sequenceIndexes[14], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(215, 130), FACING_NORTHWEST);
	}

	if (_globals[kTeleporterCommand]) {
		switch (_globals[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}

		_globals[kTeleporterCommand] = TELEPORTER_NONE;
	}

	int32 timer = (global[kTimebombTimer + 1] << 16) | global[kTimebombTimer];
	if (timer > 0) {
		global[kTimebombTimer] = 10800 - 600;
		global[kTimebombTimer + 1] = 0;
	}

	section_7_music();
}

static void room_752_daemon() {
	int32 timer = (global[kTimebombTimer + 1] << 16) | global[kTimebombTimer];

	if (timer >= 10800 && _globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		global[kTimebombStatus] = TIMEBOMB_DEAD;
		global[kTimebombTimer] = global[kTimebombTimer + 1] = 0;
		_globals[kCheckDaemonTimebomb] = false;
		_scene->_nextSceneId = 620;
	}
}

static void room_752_pre_parser() {
	if (player_said_2(walkto, west_end_of_platform)) {
		player.walk_off_edge_to_room = 751;
	}
}

static void room_752_parser() {
	if (player_said_2(walk_along, platform))
		;
	else if (player_said_2(step_into, teleporter)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		_scene->_nextSceneId = 711;
	} else if (player_said_2(take, id_card) && (!player_has(OBJ_ID_CARD) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			g_engine->_soundManager->command(15, 0);
			_scene->_sequences.remove(_globals._sequenceIndexes[13]);
			inter_give_to_player(OBJ_ID_CARD);
			_scene->_dynamicHotspots.remove(local._cardId);
			object_examine(OBJ_ID_CARD, 830, 0);
			break;
		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}
	} else if (player_said_2(take, bones) && (_action._savedFields._mainObjectSource == CAT_HOTSPOT) &&
		(!player_has(OBJ_BONES) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			g_engine->_soundManager->command(15, 0);
			if (player_has(OBJ_BONE))
				inter_move_object(OBJ_BONE, NOWHERE);
			inter_give_to_player(OBJ_BONES);
			object_examine(OBJ_BONES, 75221, 0);
			break;
		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[12]);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}
	} else if (_action._lookFlag || player_said_2(look, city)) {
		if (_globals[kLaserHoleIsThere])
			text_show(75212);
		else
			text_show(75210);
	} else if (player_said_2(look, platform))
		text_show(75213);
	else if (player_said_2(look, cement_block))
		text_show(75214);
	else if (player_said_2(look, rock))
		text_show(75215);
	else if (player_said_2(take, rock))
		text_show(75216);
	else if (player_said_2(look, west_end_of_platform))
		text_show(75217);
	else if (player_said_2(look, teleporter))
		text_show(75218);
	else if ((player_said_2(look, bones) || player_said_2(look, id_card)) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		if (object[OBJ_ID_CARD].location == 752)
			text_show(75219);
		else
			text_show(75220);
	} else if (player_said_2(take, bones) && (_action._savedFields._mainObjectSource == CAT_HOTSPOT)) {
		if (player_has(OBJ_BONES))
			text_show(75222);
	} else
		return;

	_action._inProgress = false;
}

void room_752_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._cardId);
}

void room_752_preload() {
	room_init_code_pointer = room_752_init;
	room_daemon_code_pointer = room_752_daemon;
	room_pre_parser_code_pointer = room_752_pre_parser;
	room_parser_code_pointer = room_752_parser;

	section_7_walker();
	section_7_interface();
	_scene->addActiveVocab(words_id_card);
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_look_at);
	_scene->addActiveVocab(words_laser_beam);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
