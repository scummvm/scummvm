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
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _detonatorHotspotId;
};

static Scratch local;


static void room_503_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('c', -1));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMBD_2");
	else
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*ROXBD_2");

	if (object[OBJ_DETONATORS].location == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		local._detonatorHotspotId = _scene->_dynamicHotspots.add(words_detonators, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._detonatorHotspotId, Common::Point(254, 135), FACING_SOUTH);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 191;
		player.y = 152;
		player.facing = FACING_NORTHWEST;
	}

	section_5_music();
}

static void room_503_parser() {
	if (player_said_2(walk, outside))
		_scene->_nextSceneId = 501;
	else if (player_said_2(take, detonators)) {
		if (kernel.trigger || !player_has(OBJ_DETONATORS)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 3);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 3, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], true, 8, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 4, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				g_engine->_soundManager->command(9, 0);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(local._detonatorHotspotId);
				inter_give_to_player(OBJ_DETONATORS);
				object_examine(OBJ_DETONATORS, 50326, 0);
				break;

			case 2:
				if (_globals[kSexOfRex] == REX_MALE)
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				else
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);

				player.walker_visible = true;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		}
	} else if (_action._lookFlag)
		text_show(50328);
	else if (player_said_2(look, monitoring_equipment))
		text_show(50310);
	else if (player_said_2(look, photon_rifles))
		text_show(50311);
	else if (player_said_2(take, photon_rifles) || player_said_2(take, nuclear_slingshot))
		text_show(50312);
	else if (player_said_2(look, display_case))
		text_show(50313);
	else if (player_said_2(look, nuclear_slingshot))
		text_show(50314);
	else if (player_said_2(look, water_cooler))
		text_show(50315);
	else if (player_said_2(look, storage_box))
		text_show(50316);
	else if (player_said_2(open, storage_box))
		text_show(50317);
	else if (player_said_2(look, warning_label))
		text_show(50318);
	else if (player_said_2(look, desk))
		text_show(50319);
	else if (player_said_2(look, monitor))
		text_show(50320);
	else if (player_said_2(look, file_cabinets))
		text_show(50322);
	else if (player_said_2(look, box)) {
		if (object_is_here(OBJ_DETONATORS))
			text_show(50323);
		else
			text_show(50324);
	} else if (player_said_2(look, detonators) && (_action._savedFields._mainObjectSource == 4))
		text_show(50325);
	else if (player_said_2(look, windows))
		text_show(50327);
	else if (player_said_2(open, display_case))
		text_show(50329);
	else if (player_said_2(throw, display_case) && player_has(object_named(_action._activeAction._objectNameId)))
		text_show(50330);
	else
		return;

	_action._inProgress = false;
}

void room_503_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._detonatorHotspotId);
}

void room_503_preload() {
	room_init_code_pointer = room_503_init;
	room_parser_code_pointer = room_503_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(words_detonators);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
