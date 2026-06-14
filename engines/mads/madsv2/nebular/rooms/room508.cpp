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

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Scene508::Scene508(RexNebularEngine *vm) : Scene5xx(vm) {
	_chosenObject = -1;
}

void room_508_synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_chosenObject);
}

void Scene508::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_SPINACH_PATCH_DOLL);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_LASER_BEAM);
}

static void room_508_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('h', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('l', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('t', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('l', 3));

	if (!_game._visitedScenes._sceneRevisited) {
		_globals[kLaserOn] = false;
		_chosenObject = 0;
	}

	if (!_globals[kLaserOn]) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
		int idx = _scene->_dynamicHotspots.add(NOUN_SPINACH_PATCH_DOLL, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
		_scene->_hotspots.activate(NOUN_HOLE, false);
		_scene->_hotspots.activate(NOUN_LASER_BEAM, false);
	} else {
		_scene->changeVariant(1);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 11);
		int idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
		if (_globals[kLaserHoleIsThere]) {
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
			_scene->_hotspots.activate(NOUN_HOLE, true);
			_scene->_hotspots.activate(NOUN_LASER_BEAM, true);
		}
		_vm->_sound->command(21);
	}
	_vm->_sound->command(20);

	if (_scene->_priorSceneId == 515) {
		_game._player._playerPos = Common::Point(57, 116);
		_game._player._facing = FACING_NORTHEAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(289, 139);
		_game._player._facing = FACING_WEST;
	}

	sceneEntrySound();
	_game.loadQuoteSet(0x273, 0);

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_COMPACT_CASE);
		_game._objects.addToInventory(OBJ_REARVIEW_MIRROR);
	}
}

static void room_508_pre_parser() {
	if (_action.isAction(VERB_WALK, NOUN_OUTSIDE))
		_game._player._walkOffScreenSceneId = 506;
}

void Scene508::handlePedestral() {
	if (!_globals[kLaserOn])
		_vm->_dialogs->show(50835);

	if (_globals[kLaserHoleIsThere])
		_vm->_dialogs->show(50836);

	if (_globals[kLaserOn] && !_globals[kLaserHoleIsThere]) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			if (_chosenObject == 2)
				_game._objects.removeFromInventory(OBJ_COMPACT_CASE, 1);
			else
				_game._objects.removeFromInventory(OBJ_REARVIEW_MIRROR, 1);

			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
			_scene->_hotspots.activate(NOUN_HOLE, true);
			_scene->_hotspots.activate(NOUN_LASER_BEAM, true);
			break;

		case 3:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[6]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(120, 4);
			break;

		case 4:
			_vm->_dialogs->show(50834);
			_globals[kLaserHoleIsThere] = true;
			_scene->_nextSceneId = 515;
			break;

		default:
			break;
		}
	}
}

static void room_508_parser() {
	if (_action.isAction(VERB_PULL, NOUN_LEVER)) {
		if (!_globals[kLaserOn]) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 2, 120, _game.getQuote(0x273));
				break;

			case 2:
				_game._player._visible = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 7);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], -1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_vm->_sound->command(19);
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[1]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(15, 5);
				break;

			case 4:
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
				break;

			case 5:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_scene->loadAnimation(formAnimName('B', 1), 6);
				break;

			case 6:
			{
				_vm->_sound->command(22);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 11);
				int idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
				_scene->_kernelMessages.reset();
				_scene->changeVariant(1);
				_scene->_sequences.addTimer(30, 7);
			}
			break;

			case 7:
				_globals[kLaserOn] = true;
				_vm->_dialogs->show(50833);
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else {
			_vm->_dialogs->show(50837);
		}
	} else if (_action.isAction(VERB_REFLECT, NOUN_REARVIEW_MIRROR, NOUN_LASER_BEAM) || _action.isAction(VERB_PUT, NOUN_REARVIEW_MIRROR, NOUN_PEDESTAL) || _action.isAction(VERB_PUT, NOUN_REARVIEW_MIRROR, NOUN_LASER_BEAM)) {
		_chosenObject = 1;
		handlePedestral();
	} else if (_action.isAction(VERB_PUT, NOUN_COMPACT_CASE, NOUN_PEDESTAL) || _action.isAction(VERB_PUT, NOUN_COMPACT_CASE, NOUN_LASER_BEAM) || _action.isAction(VERB_REFLECT, NOUN_COMPACT_CASE, NOUN_LASER_BEAM)) {
		_chosenObject = 2;
		handlePedestral();
	} else if (_action._lookFlag)
		_vm->_dialogs->show(50822);
	else if (_action.isAction(VERB_LOOK, NOUN_TARGET_AREA))
		_vm->_dialogs->show(50810);
	else if (_action.isAction(VERB_LOOK, NOUN_SPINACH_PATCH_DOLL))
		_vm->_dialogs->show(50811);
	else if (_action.isAction(VERB_TAKE, NOUN_SPINACH_PATCH_DOLL))
		_vm->_dialogs->show(50812);
	else if (_action.isAction(VERB_LOOK, NOUN_SAND_BAGS))
		_vm->_dialogs->show(50816);
	else if (_action.isAction(VERB_TAKE, NOUN_SAND_BAGS))
		_vm->_dialogs->show(50817);
	else if (_action.isAction(VERB_LOOK, NOUN_CONTROL_STATION))
		_vm->_dialogs->show(50818);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR)) {
		if (_globals[kLaserOn])
			_vm->_dialogs->show(50820);
		else
			_vm->_dialogs->show(50819);
	} else if (_action.isAction(VERB_LOOK, NOUN_LASER_CANNON)) {
		if (_globals[kLaserOn])
			_vm->_dialogs->show(50822);
		else
			_vm->_dialogs->show(50821);
	} else if (_action.isAction(VERB_TAKE, NOUN_LASER_CANNON))
		_vm->_dialogs->show(50823);
	else if (_action.isAction(VERB_LOOK, NOUN_LEVER)) {
		if (_globals[kLaserOn])
			_vm->_dialogs->show(50825);
		else
			_vm->_dialogs->show(50824);
	} else if (_action.isAction(VERB_PUSH, NOUN_LEVER))
		_vm->_dialogs->show(50826);
	else if (_action.isAction(VERB_LOOK, NOUN_LASER_BEAM)) {
		if (_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(50828);
		else
			_vm->_dialogs->show(50827);
	} else if (_action.isAction(VERB_TAKE, NOUN_LASER_BEAM))
		_vm->_dialogs->show(50829);
	else if (_action.isAction(VERB_LOOK, NOUN_CEILING)) {
		if (_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(50831);
		else
			_vm->_dialogs->show(50830);
	} else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(50832);
	else if (_action.isAction(VERB_LOOK, NOUN_PEDESTAL)) {
		if (!_globals[kLaserOn])
			_vm->_dialogs->show(50813);
		else if (!_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(50814);
		else
			_vm->_dialogs->show(50815);
	} else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
