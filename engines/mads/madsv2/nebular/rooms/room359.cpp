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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _cardHotspotId;
};

static Scratch local;


static void room_359_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMBD_2");
	else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXBD_2");

	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);

	if (_game._objects.isInRoom(OBJ_SECURITY_CARD)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		local._cardHotspotId = _scene->_dynamicHotspots.add(words_security_card, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._cardHotspotId, Common::Point(107, 107), FACING_SOUTH);
	}

	if (_scene->_priorSceneId == 358)
		_game._player._playerPos = Common::Point(301, 141);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(15, 148);

	section_3_music();
}

static void room_359_pre_parser() {
	if (_action.isAction(words_walk_down, words_corridor_to_east))
		_game._player._walkOffScreenSceneId = 358;

	if (_action.isAction(words_walk_down, words_corridor_to_west))
		_game._player._walkOffScreenSceneId = 360;
}

static void room_359_parser() {
	if (_action._lookFlag) {
		if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
			_vm->_dialogs->show(35914);
		else
			_vm->_dialogs->show(35915);
	} else if (_action.isAction(words_take, words_security_card)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_SECURITY_CARD)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_vm->_dialogs->show(35920);
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 4, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], true, 7, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
					_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(106, 110));
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(local._cardHotspotId);
				_vm->_sound->command(57);
				_game._objects.addToInventory(OBJ_SECURITY_CARD);
				_vm->_dialogs->showItem(OBJ_SECURITY_CARD, 0x330);
				_scene->changeVariant(1);
				break;

			case 2:
				if (_globals[kSexOfRex] == REX_MALE)
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				else
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);

				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(words_look, words_bloody_cell_wall))
		_vm->_dialogs->show(35910);
	else if (_action.isAction(words_look, words_bed))
		_vm->_dialogs->show(35911);
	else if (_action.isAction(words_look, words_sink))
		_vm->_dialogs->show(35912);
	else if (_action.isAction(words_look, words_toilet))
		_vm->_dialogs->show(35913);
	else if (_action.isAction(words_look, words_corridor_to_east))
		_vm->_dialogs->show(35916);
	else if (_action.isAction(words_look, words_corridor_to_west))
		_vm->_dialogs->show(35917);
	else if (_action.isAction(words_look, words_limb))
		_vm->_dialogs->show(35918);
	else if (_action.isAction(words_take, words_limb))
		_vm->_dialogs->show(35919);
	else if (_action.isAction(words_look, words_security_card) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(35921);
	else if (_action.isAction(words_look, words_blood_stain)) {
		if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
			_vm->_dialogs->show(35922);
		else
			_vm->_dialogs->show(35923);
	} else if (_action.isAction(words_look, words_wall_board))
		_vm->_dialogs->show(35924);
	else if (_action.isAction(words_take, words_wall_board))
		_vm->_dialogs->show(35925);
	else if (_action.isAction(words_look, words_rip_in_floor))
		_vm->_dialogs->show(35926);
	else if (_action.isAction(words_look, words_corridor))
		_vm->_dialogs->show(35927);
	else if (_action.isAction(words_look, words_floor)) {
		if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
			_vm->_dialogs->show(35928);
		else
			_vm->_dialogs->show(35929);
	} else if (_action.isAction(words_open, words_air_vent) || _action.isAction(words_look, words_air_vent))
		_vm->_dialogs->show(36016);
	else
		return;

	_action._inProgress = false;
}

void room_359_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._cardHotspotId);
}

void room_359_preload() {
	room_init_code_pointer = room_359_init;
	room_pre_parser_code_pointer = room_359_pre_parser;
	room_parser_code_pointer = room_359_parser;

	section_3_walker();
	section_3_interface();
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
