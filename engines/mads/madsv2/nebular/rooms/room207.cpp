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

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/rooms/room201.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Scene207::Scene207(RexNebularEngine *vm) : Scene2xx(vm) {
	_vultureFl = false;
	_spiderFl = false;
	_eyeFl = false;
	_spiderHotspotId = -1;
	_vultureHotspotId = -1;

	_spiderTime = _game._player._priorTimer;
	_vultureTime = _game._player._priorTimer;
}

void Scene207::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_vultureFl);
	s.syncAsByte(_spiderFl);
	s.syncAsByte(_eyeFl);

	s.syncAsSint32LE(_spiderHotspotId);
	s.syncAsSint32LE(_vultureHotspotId);
}

void Scene207::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_VULTURE);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_SPIDER);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene207::enter() {
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
		_vultureFl = false;
	else
		_vultureFl = true;

	_spiderFl = (var2 & 1);

	if (_vultureFl) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 30, 0, 0, 400);
		_vultureHotspotId = _scene->_dynamicHotspots.add(389, 13, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_vultureHotspotId, Common::Point(254, 94), FACING_WEST);
	}

	if (_spiderFl) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -1);
		_spiderHotspotId = _scene->_dynamicHotspots.add(333, 13, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_spiderHotspotId, Common::Point(59, 132), FACING_SOUTH);
	}

	_eyeFl = false;
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

void Scene207::moveVulture() {
	_scene->_sequences.remove(_globals._sequenceIndexes[1]);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
	_vm->_sound->command(43);
	_vultureFl = false;
	_vultureTime = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(_vultureHotspotId);
}

void Scene207::moveSpider() {
	_scene->_sequences.remove(_globals._sequenceIndexes[4]);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
	_spiderFl = false;
	_spiderTime = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(_spiderHotspotId);
}

void Scene207::step() {
	Player &player = _game._player;

	if (_vultureFl) {
		if (((int32)player._priorTimer - _vultureTime) > 1700)
			moveVulture();
	}

	if (_spiderFl) {
		if (((int32)player._priorTimer - _spiderTime) > 800)
			moveSpider();
	}

	if (_game._trigger == 70) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 23, 34);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
	}

	if (_game._trigger == 71)
		_eyeFl = false;

	if (_eyeFl)
		return;

	if ((_game._player._playerPos.x >= 124) && (_game._player._playerPos.x <= 201)) {
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 6);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_eyeFl = true;
	}
}

void Scene207::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_PATH_TO_WEST))
		_game._player._walkOffScreenSceneId = 211;

	if (_action.isAction(VERB_WALK_TOWARDS, NOUN_OPEN_FIELD_TO_EAST))
		_game._player._walkOffScreenSceneId = 208;

	if (_action.isAction(VERB_WALKTO) || _action.isAction(VERB_LOOK)) {
		if (_action.isObject(NOUN_VULTURE)) {
			_vultureTime = -9999;
		} else if (_action.isObject(NOUN_SPIDER)) {
			_spiderTime = -9999;
		}
	}
}

void Scene207::actions() {
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
		} else if (_eyeFl) {
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_eyeFl = false;
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

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
