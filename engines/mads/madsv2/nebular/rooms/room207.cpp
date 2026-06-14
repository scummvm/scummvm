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
	bool _vultureFl;
	bool _spiderFl;
	bool _eyeFl;
	int16 _spiderHotspotId;
	int16 _vultureHotspotId;
	long _spiderTime;
	long _vultureTime;
};

static Scratch local;


static void room_207_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('h', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('h', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('e', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('e', 1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('g', 1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('g', 0));
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 7);

	int var2;
	if (!_game._visitedScenes._sceneRevisited) {
		var2 = 1;
	} else {
		var2 = _vm->getRandomNumber(4) + 1;
	}

	if (var2 > 2)
		local._vultureFl = false;
	else
		local._vultureFl = true;

	local._spiderFl = (var2 & 1);

	if (local._vultureFl) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 30, 0, 0, 400);
		local._vultureHotspotId = _scene->_dynamicHotspots.add(389, 13, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._vultureHotspotId, Common::Point(254, 94), FACING_WEST);
	}

	if (local._spiderFl) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -1);
		local._spiderHotspotId = _scene->_dynamicHotspots.add(333, 13, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._spiderHotspotId, Common::Point(59, 132), FACING_SOUTH);
	}

	local._eyeFl = false;
	if (_scene->_priorSceneId == 211) {
		_game._player._playerPos = Common::Point(13, 105);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 214) {
		_game._player._playerPos = Common::Point(164, 117);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(305, 131);
	}

	_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 1, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 22);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
}

static void moveVulture() {
	_scene->_sequences.remove(_globals._sequenceIndexes[1]);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
	_vm->_sound->command(43);
	local._vultureFl = false;
	local._vultureTime = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(local._vultureHotspotId);
}

static void moveSpider() {
	_scene->_sequences.remove(_globals._sequenceIndexes[4]);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
	local._spiderFl = false;
	local._spiderTime = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(local._spiderHotspotId);
}

static void room_207_daemon() {
	auto &gplayer = _game._player;

	if (local._vultureFl) {
		if (((int32)gplayer._priorTimer - local._vultureTime) > 1700)
			moveVulture();
	}

	if (local._spiderFl) {
		if (((int32)gplayer._priorTimer - local._spiderTime) > 800)
			moveSpider();
	}

	if (_game._trigger == 70) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 23, 34);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
	}

	if (_game._trigger == 71)
		local._eyeFl = false;

	if (local._eyeFl)
		return;

	if ((_game._player._playerPos.x >= 124) && (_game._player._playerPos.x <= 201)) {
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 6);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		local._eyeFl = true;
	}
}

static void room_207_pre_parser() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_WEST))
		_game._player._walkOffScreenSceneId = 211;

	if (_action.isAction(VERB_WALK_TOWARDS, NOUN_OPEN_FIELD_TO_EAST))
		_game._player._walkOffScreenSceneId = 208;

	if (_action.isAction(VERB_WALKTO) || _action.isAction(VERB_LOOK)) {
		if (_action.isObject(NOUN_VULTURE)) {
			local._vultureTime = -9999;
		} else if (_action.isObject(NOUN_SPIDER)) {
			local._spiderTime = -9999;
		}
	}
}

static void room_207_parser() {
	if (_action._savedFields._lookFlag)
		_vm->_dialogs->show(20711);
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY))
		_scene->_nextSceneId = 214;
	else {
		if ((_game._player._playerPos.x > 150) && (_game._player._playerPos.x < 189) &&
			(_game._player._playerPos.y > 111) && (_game._player._playerPos.y < 130)) {
			if ((_game._player._playerPos.x <= 162) || (_game._player._playerPos.x >= 181) ||
				(_game._player._playerPos.y <= 115) || (_game._player._playerPos.y >= 126)) {
				_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 2, 0, 0);
				_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 2, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 6);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 6);
			}
		} else if (local._eyeFl) {
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			local._eyeFl = false;
		}

		if (_action.isAction(VERB_LOOK, NOUN_DENSE_FOREST))
			_vm->_dialogs->show(20701);
		else if (_action.isAction(VERB_LOOK, NOUN_HEDGE))
			_vm->_dialogs->show(20702);
		else if (_action.isAction(VERB_LOOK, NOUN_SKULL_AND_CROSSBONES))
			_vm->_dialogs->show(20703);
		else if (_action.isAction(VERB_LOOK, NOUN_CAULDRON))
			_vm->_dialogs->show(20704);
		else if (_action.isAction(VERB_LOOK, NOUN_WITCHDOCTOR_HUT))
			_vm->_dialogs->show(20705);
		else if (_action.isAction(VERB_LOOK, NOUN_PATH_TO_WEST))
			_vm->_dialogs->show(20706);
		else if (_action.isAction(VERB_LOOK, NOUN_MOUNTAINS))
			_vm->_dialogs->show(20707);
		else if (_action.isAction(VERB_LOOK, NOUN_ALOE_PLANT))
			_vm->_dialogs->show(20708);
		else if (_action.isAction(VERB_LOOK, NOUN_LAWN))
			_vm->_dialogs->show(20709);
		else if (_action.isAction(VERB_LOOK, NOUN_VULTURE))
			_vm->_dialogs->show(20710);
		else if (_action.isAction(VERB_TAKE, NOUN_SKULL_AND_CROSSBONES))
			_vm->_dialogs->show(20712);
		else if (_action.isAction(VERB_TAKE, NOUN_ALOE_PLANT))
			_vm->_dialogs->show(20713);
		else if (_action.isAction(VERB_LOOK, NOUN_SPIDER))
			_vm->_dialogs->show(20714);
		else if (_action.isAction(VERB_TAKE, NOUN_SPIDER))
			_vm->_dialogs->show(20715);
		else
			return;
	}

	_action._inProgress = false;
}

void room_207_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._vultureFl);
	s.syncAsByte(local._spiderFl);
	s.syncAsByte(local._eyeFl);

	s.syncAsSint32LE(local._spiderHotspotId);
	s.syncAsSint32LE(local._vultureHotspotId);
}

void room_207_preload() {
	local._spiderTime = _game._player._priorTimer;
	local._vultureTime = _game._player._priorTimer;

	room_init_code_pointer = room_207_init;
	room_pre_parser_code_pointer = room_207_pre_parser;
	room_parser_code_pointer = room_207_parser;
	room_daemon_code_pointer = room_207_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(NOUN_VULTURE);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_SPIDER);
	_scene->addActiveVocab(VERB_WALKTO);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
