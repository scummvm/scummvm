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
#include "mads/madsv2/nebular/rooms/section2.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _pterodactylFlag;
};

static Scratch local;


static void room_201_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('m', -1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*SC002Z1");
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 1, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 50);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 4, 0, 0, 0);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 8);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(185, 46));

	int idx = _scene->_dynamicHotspots.add(words_birds, words_look_at, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(186, 81), FACING_NORTH);

	if ((_scene->_priorSceneId == 202) || (_scene->_priorSceneId == RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(165, 152);
	} else {
		_game._player._playerPos = Common::Point(223, 149);
		_game._player._facing = FACING_SOUTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		int sepChar = (_globals[kSexOfRex] == SEX_MALE) ? 't' : 'u';
		// Guess values. What is the default value used by the compiler?
		int suffixNum = -1;
		int endTrigger = -1;
		switch (_globals[kTeleporterCommand]) {
		case 1:
			suffixNum = 3;
			endTrigger = 76;
			_globals[kTeleporterUnderstood] = true;
			break;
		case 2:
			suffixNum = 1;
			endTrigger = 77;
			break;
		case 3:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			suffixNum = -1;
			break;
		case 4:
			suffixNum = 2;
			endTrigger = 78;
			break;
		default:
			break;
		}
		_globals[kTeleporterCommand] = 0;
		if (suffixNum >= 0)
			_scene->loadAnimation(formAnimName(sepChar, suffixNum), endTrigger);
	}

	if ((_scene->_priorSceneId == 202) && (_globals[kMeteorologistStatus] == METEOROLOGIST_PRESENT) && !_scene->_roomChanged) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 0));
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_game.loadQuoteSet(90, 91, 0);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -1, 12);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 12, 70);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
		local._pterodactylFlag = false;
		_game._player.walk(Common::Point(157, 143), FACING_NORTH);
		_vm->_palette->setEntry(252, 45, 63, 45);
		_vm->_palette->setEntry(253, 20, 45, 20);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 2, 0, 120, _game.getQuote(90));
	} else
		local._pterodactylFlag = true;

	if (_globals[kTeleporterUnderstood])
		_scene->_hotspots.activate(words_strange_device, false);

	section_2_music();
}

static void room_201_daemon() {
	if (local._pterodactylFlag && (_vm->getRandomNumber(5000) == 9)) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 1, 6, 0);
		int idx = _scene->_dynamicHotspots.add(words_swooping_creature, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(270, 80), FACING_EAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 8);
		_vm->_sound->command(14);
		local._pterodactylFlag = false;
	}

	if (_game._trigger == 70) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
		_game._player._visible = false;
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 12, 16);
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 9, 1, 0, 0);
		_vm->_sound->command(42);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 3, 81);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
	}

	if (_game._trigger == 81) {
		_scene->_kernelMessages.reset();
	}

	if (_game._trigger == 71) {
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 9, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], -2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
	}

	if (_game._trigger == 73) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 17, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
	}

	if (_game._trigger == 74) {
		_vm->_sound->command(40);

		_scene->_kernelMessages.add(Common::Point(125, 56), 0xFDFC, 32, 82, 180, _game.getQuote(91));
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -2, -2);
		_scene->_sequences.addTimer(180, 75);
	}

	if (_game._trigger == 75) {
		_globals[kMeteorologistEverSeen] = 0;
		_scene->_nextSceneId = 202;
	}

	if (_game._trigger == 76) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
	}

	if (_game._trigger == 77) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (_game._trigger == 78) {
		_vm->_sound->command(40);
		_vm->_dialogs->show(20114);
		_scene->_reloadSceneFlag = true;
	}
}

static void room_201_parser() {
	if (_action._lookFlag == false) {
		if (_action.isAction(words_walk_towards, words_field_to_south))
			_scene->_nextSceneId = 202;
		else if (_action.isAction(words_climb_up, words_steps) || (_action.isAction(words_walk_inside, words_teleporter)) || (_action.isAction(words_walk_inside, words_strange_device))) {
			if (_game._trigger == 0) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				int sepChar = (_globals[kSexOfRex] == SEX_MALE) ? 't' : 'u';
				_scene->loadAnimation(formAnimName(sepChar, 0), 1);
			} else if (_game._trigger == 1) {
				_scene->_nextSceneId = 213;
			}
		} else if (_action.isAction(words_look, words_grassy_field)) {
			_vm->_dialogs->show(20101);
		} else if (_action.isAction(words_look, words_rocks)) {
			_vm->_dialogs->show(20102);
		} else if (_action.isAction(words_look, words_thorny_bush)) {
			_vm->_dialogs->show(20103);
		} else if (_action.isAction(words_look, words_sky)) {
			_vm->_dialogs->show(20104);
		} else if (_action.isAction(words_look, words_water)) {
			_vm->_dialogs->show(20105);
		} else if (_action.isAction(words_look, words_island_in_distance)) {
			_vm->_dialogs->show(20106);
		} else if (_action.isAction(words_look, words_weather_station)) {
			_vm->_dialogs->show(20107);
		} else if (_action.isAction(words_look, words_path)) {
			_vm->_dialogs->show(20108);
		} else if (_action.isAction(words_look, words_field_to_south)) {
			_vm->_dialogs->show(20110);
		} else if (_action.isAction(words_look, words_strange_device)) {
			if (_globals[kMeteorologistEverSeen])
				_vm->_dialogs->show(20112);
			else
				_vm->_dialogs->show(20109);
		} else if (_action.isAction(words_look, words_teleporter)) {
			_vm->_dialogs->show(20113);
		} else
			return;
	} else {
		_vm->_dialogs->show(20111);
	}
	_action._inProgress = false;
}

void room_201_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._pterodactylFlag);
}

void room_201_preload() {
	room_init_code_pointer = room_201_init;
	room_parser_code_pointer = room_201_parser;
	room_daemon_code_pointer = room_201_daemon;

	section_2_walker();
	section_2_interface();

	_scene->addActiveVocab(words_swooping_creature);
	_scene->addActiveVocab(words_birds);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
