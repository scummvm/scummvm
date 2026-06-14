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

Scene610::Scene610(RexNebularEngine *vm) : Scene6xx(vm) {
	_handsetHotspotId = -1;
	_checkVal = -1;

	_cellCharging = false;

	_cellChargingTimer = -1;
	_lastFrameTimer = 0;
}

void room_610_synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_handsetHotspotId);
	s.syncAsSint16LE(_checkVal);

	s.syncAsByte(_cellCharging);

	s.syncAsSint32LE(_cellChargingTimer);
	s.syncAsUint32LE(_lastFrameTimer);
}

void Scene610::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_PHONE_HANDSET);
	_scene->addActiveVocab(VERB_WALKTO);
}

static void room_610_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 1));

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 60, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 13);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 30, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);

	if (!_game._visitedScenes._sceneRevisited)
		_cellCharging = false;

	if (_game._objects[OBJ_PHONE_HANDSET]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_handsetHotspotId = _scene->_dynamicHotspots.add(NOUN_PHONE_HANDSET, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_handsetHotspotId, Common::Point(132, 121), FACING_NORTHWEST);
		if ((_globals[kHandsetCellStatus] == 2) && (_game._difficulty == DIFFICULTY_HARD) && !_globals[kDurafailRecharged])
			_globals[kHandsetCellStatus] = 1;
	}

	if (_scene->_roomChanged && _game._difficulty != DIFFICULTY_EASY)
		_game._objects.addToInventory(OBJ_PENLIGHT);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(175, 152);
		_game._player._facing = FACING_NORTHWEST;
	}

	sceneEntrySound();
}

void Scene610::step() {
	if (_cellCharging) {
		long diff = _scene->_frameStartTime - _lastFrameTimer;
		if ((diff >= 0) && (diff <= 60))
			_cellChargingTimer += diff;
		else
			_cellChargingTimer++;

		_lastFrameTimer = _scene->_frameStartTime;
	}

	// CHECKME: _checkVal is always false, could be removed
	if ((_cellChargingTimer >= 60) && !_checkVal) {
		_checkVal = true;
		_globals[kHandsetCellStatus] = 1;
		_cellCharging = false;
		_checkVal = false;
		_cellChargingTimer = 0;
	}
}

static void room_610_parser() {
	if (_action.isAction(VERB_EXIT_FROM, NOUN_VIDEO_STORE))
		_scene->_nextSceneId = 609;
	else if (_action.isAction(VERB_TAKE, NOUN_PHONE_HANDSET)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_PHONE_HANDSET)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_handsetHotspotId);
				_game._objects.addToInventory(OBJ_PHONE_HANDSET);
				_vm->_dialogs->showItem(OBJ_PHONE_HANDSET, 61017);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_PUT, NOUN_PHONE_HANDSET, NOUN_PHONE_CRADLE)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
			_handsetHotspotId = _scene->_dynamicHotspots.add(NOUN_PHONE_HANDSET, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_handsetHotspotId, Common::Point(132, 121), FACING_NORTHWEST);
			_game._objects.setRoom(OBJ_PHONE_HANDSET, _scene->_currentSceneId);
			break;

		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			if ((_globals[kHandsetCellStatus] == 2) && (_game._difficulty == DIFFICULTY_HARD) && !_globals[kDurafailRecharged])
				_cellCharging = true;

			_vm->_dialogs->show(61032);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_PIPPY_BILLBOARD))
		_vm->_dialogs->show(61010);
	else if (_action.isAction(VERB_LOOK, NOUN_CIVILIZATION_AD))
		_vm->_dialogs->show(61011);
	else if (_action.isAction(VERB_LOOK, NOUN_MARX_BROS_POSTER))
		_vm->_dialogs->show(61012);
	else if (_action.isAction(VERB_LOOK, NOUN_VIDEO_MONITOR))
		_vm->_dialogs->show(61013);
	else if (_action.isAction(VERB_LOOK, NOUN_VIDEO_STORE))
		_vm->_dialogs->show(61014);
	else if (_action._lookFlag)
		_vm->_dialogs->show(61015);
	else if (_action.isAction(VERB_LOOK, NOUN_LOGO))
		_vm->_dialogs->show(61018);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT)) {
		if (_game._visitedScenes.exists(601))
			_vm->_dialogs->show(61020);
		else
			_vm->_dialogs->show(61019);
	} else if (_action.isAction(VERB_LOOK, NOUN_COUNTER))
		_vm->_dialogs->show(61021);
	else if (_action.isAction(VERB_LOOK, NOUN_PHONE_ANTENNA))
		_vm->_dialogs->show(61022);
	else if (_action.isAction(VERB_LOOK, NOUN_SMELLY_SNEAKER))
		_vm->_dialogs->show(61023);
	else if (_action.isAction(VERB_TAKE, NOUN_SMELLY_SNEAKER))
		_vm->_dialogs->show(61024);
	else if (_action.isAction(VERB_LOOK, NOUN_SPOTLIGHT))
		_vm->_dialogs->show(61025);
	else if (_action.isAction(VERB_LOOK, NOUN_PHONE_HANDSET) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(61026);
	else if (_action.isAction(VERB_LOOK, NOUN_PHONE_CRADLE))
		_vm->_dialogs->show(61027);
	else if (_action.isAction(VERB_LOOK, NOUN_RETURN_SLOT))
		_vm->_dialogs->show(61028);
	else if (_action.isAction(VERB_PUT, NOUN_RETURN_SLOT)
		&& _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(61029);
	else if (_action.isObject(NOUN_CLASSIC_VIDEOS) || _action.isObject(NOUN_MORE_CLASSIC_VIDEOS) || _action.isObject(NOUN_DRAMA_VIDEOS)
		|| _action.isObject(NOUN_NEW_RELEASE_VIDEOS) || _action.isObject(NOUN_PORNO_VIDEOS) || _action.isObject(NOUN_EDUCATIONAL_VIDEOS)
		|| _action.isObject(NOUN_INSTRUCTIONAL_VIDEOS) || _action.isObject(NOUN_WORKOUT_VIDEOS) || _action.isObject(NOUN_FOREIGN_VIDEOS)
		|| _action.isObject(NOUN_ADVENTURE_VIDEOS) || _action.isObject(NOUN_COMEDY_VIDEOS)) {
		if (_action.isAction(VERB_LOOK))
			_vm->_dialogs->show(61030);
		else if (_action.isAction(VERB_TAKE))
			_vm->_dialogs->show(61031);
		else
			return;
	} else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
