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
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section5.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _handingLine;
	bool _lineMoving;
	int16 _lineAnimationMode;
	int16 _lineFrame;
	int16 _lineAnimationPosition;
};

static Scratch local;


static void room_511_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_6");

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		local._handingLine = false;

	if (_globals[kBoatRaised]) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_scene->_hotspots.activate(NOUN_BOAT, false);
		int idx = _scene->_dynamicHotspots.add(NOUN_BOAT, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(75, 124), FACING_NORTH);
		_scene->_hotspots.activate(NOUN_ROPE, false);
	} else {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('b', 2));
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 3));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 1));

		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 1, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);

		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);

		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);

		_scene->_hotspots.activate(NOUN_ROPE, true);
		_scene->_hotspots.activate(NOUN_BOAT, true);
		_scene->changeVariant(1);
	}

	int frame = 0;
	if (_globals[kLineStatus] == 2)
		frame = -1;
	else if (_globals[kLineStatus] == 3)
		frame = -2;

	if (_globals[kLineStatus] == 2 || _globals[kLineStatus] == 3) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('b', 4));
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, frame);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(26, 153), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 3);
		if (_globals[kBoatRaised])
			_scene->changeVariant(2);
	}

	local._lineFrame = -1;
	local._lineMoving = false;

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if (_scene->_priorSceneId == 512) {
		_game._player._playerPos = Common::Point(60, 112);
		_game._player._facing = FACING_SOUTHEAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(55, 152);
		_game._player._facing = FACING_NORTHWEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	} else if (local._handingLine) {
		_game._player._visible = false;
		local._lineAnimationMode = 1;
		local._lineAnimationPosition = 1;
		_scene->loadAnimation(formAnimName('R', -1));
		local._lineFrame = 2;
	}
	section_5_music();
}

static void room_511_daemon() {
	if ((local._lineAnimationMode == 1) && _scene->_animation[0]) {
		if (local._lineFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._lineFrame = _scene->_animation[0]->getCurrentFrame();
			int resetFrame = -1;

			if ((local._lineAnimationPosition == 2) && (local._lineFrame == 14))
				local._lineMoving = false;

			if (local._lineAnimationPosition == 1) {
				if (local._lineFrame == 3) {
					local._lineMoving = false;
					resetFrame = 2;
				}

				if (local._handingLine)
					resetFrame = 2;
			}

			if ((resetFrame >= 0) && (resetFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(resetFrame);
				local._lineFrame = resetFrame;
			}
		}
	}

	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void room_511_pre_parser() {
	if (!local._handingLine)
		return;

	if (_action.isAction(VERB_LOOK) || _action.isObject(NOUN_FISHING_LINE) || _action.isAction(VERB_TALKTO))
		_game._player._needToWalk = false;

	if ((!_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_BOAT) || !_action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_BOAT)) && _game._player._needToWalk) {
		if (_game._trigger == 0) {
			_game._player._readyToWalk = false;
			_game._player._stepEnabled = false;
			_scene->freeAnimation();
			local._lineAnimationMode = 2;
			_scene->loadAnimation(formAnimName('R', 2), 1);
		} else if (_game._trigger == 1) {
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			_game._objects.setRoom(OBJ_FISHING_LINE, 1);
			local._handingLine = false;
			_game._player._stepEnabled = true;
			_game._player._readyToWalk = true;
		}
	}
}

static void room_511_parser() {
	if (_action.isAction(VERB_WALK_INTO, NOUN_RESTAURANT))
		_scene->_nextSceneId = 512;
	else if (_action.isAction(VERB_GET_INTO, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else 	if (_action.isAction(VERB_TAKE, NOUN_FISHING_LINE)) {
		if (!_globals[kBoatRaised]) {
			if (_globals[kLineStatus] == 2) {
				if (_globals[kLineStatus] != 3) {
					if (_game._trigger == 0) {
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_game._player.update();
						local._lineAnimationMode = 1;
						local._lineAnimationPosition = 1;
						local._lineMoving = true;
						_scene->loadAnimation(formAnimName('R', -1));
						_scene->_sequences.addTimer(1, 1);
					} else if (_game._trigger == 1) {
						if (local._lineMoving) {
							_scene->_sequences.addTimer(1, 1);
						} else {
							_game._objects.addToInventory(OBJ_FISHING_LINE);
							local._lineMoving = true;
							local._handingLine = true;
							_game._player._stepEnabled = true;
						}
					}
				} else
					_vm->_dialogs->show(51129);
			} else
				return;
		} else {
			_vm->_dialogs->show(51130);
		}
	} else if (_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_BOAT) ||
		_action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_BOAT)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(51131);
		else if (_globals[kLineStatus] == 1)
			_vm->_dialogs->show(51130);
		else if (!_globals[kBoatRaised] && local._handingLine) {
			if (_globals[kLineStatus] != 3) {
				if (_game._trigger == 0) {
					_game._player._stepEnabled = false;
					_scene->_sequences.remove(_globals._sequenceIndexes[7]);
					local._lineMoving = true;
					local._lineAnimationPosition = 2;
					_scene->_sequences.addTimer(1, 1);
				} else if (_game._trigger == 1) {
					if (local._lineMoving)
						_scene->_sequences.addTimer(1, 1);
					else {
						_game._player._visible = true;
						_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 4);
						int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(26, 153), FACING_NORTHEAST);
						_game._objects.removeFromInventory(OBJ_FISHING_LINE, 1);
						local._handingLine = false;
						local._lineMoving = true;
						_globals[kLineStatus] = 3;
						_game._player._stepEnabled = true;

						if (_scene->_animation[0])
							_scene->_animation[0]->eraseSprites();
						_game._player.update();
					}
				}
			}
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_STREET) || _action._lookFlag) {
		if (_globals[kLineStatus] == 2)
			_vm->_dialogs->show(51110);
		else {
			if (_globals[kLineStatus] == 3)
				_vm->_dialogs->show(51111);
			else
				_vm->_dialogs->show(51112);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(51113);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK))
		_vm->_dialogs->show(51114);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_SIDEWALK_TO_EAST) || _action.isAction(VERB_WALK_DOWN, NOUN_SIDEWALK_TO_WEST) || _action.isAction(VERB_WALK_DOWN, NOUN_STREET_TO_EAST))
		_vm->_dialogs->show(51115);
	else if (_action.isAction(VERB_LOOK, NOUN_PLEASURE_DOME))
		_vm->_dialogs->show(51116);
	else if (_action.isAction(VERB_LOOK, NOUN_TICKET_BOOTH))
		_vm->_dialogs->show(51117);
	else if (_action.isAction(VERB_LOOK, NOUN_DOME_ENTRANCE))
		_vm->_dialogs->show(51118);
	else if (_action.isAction(VERB_UNLOCK, NOUN_PADLOCK_KEY, NOUN_DOME_ENTRANCE) || _action.isAction(VERB_UNLOCK, NOUN_DOOR_KEY, NOUN_DOME_ENTRANCE))
		_vm->_dialogs->show(51119);
	else if ((_action.isAction(VERB_PUT) || _action.isAction(VERB_THROW))
		&& (_action.isObject(NOUN_TIMEBOMB) || _action.isObject(NOUN_BOMB) || _action.isObject(NOUN_BOMBS))
		&& _action.isObject(NOUN_DOME_ENTRANCE))
		_vm->_dialogs->show(51120);
	else if (_action.isAction(VERB_LOOK, NOUN_RESTAURANT)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(51121);
		else
			_vm->_dialogs->show(51128);
	} else if (_action.isAction(VERB_LOOK, NOUN_PORTHOLE))
		_vm->_dialogs->show(51122);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_LINE) && (_action._mainObjectSource == CAT_HOTSPOT) && (_globals[kLineStatus] == 2))
		_vm->_dialogs->show(51126);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_LINE) && (_action._mainObjectSource == CAT_HOTSPOT) && (_globals[kLineStatus] == 3))
		_vm->_dialogs->show(51133);
	else if (_action.isAction(VERB_LOOK, NOUN_STATUE))
		_vm->_dialogs->show(51127);
	else if (_action.isAction(VERB_LOOK, NOUN_BOAT))
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(51123);
		else if (_globals[kLineStatus] != 3)
			_vm->_dialogs->show(51124);
		else
			_vm->_dialogs->show(51125);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_LINE) && (_globals[kLineStatus] == 3))
		_vm->_dialogs->show(51125);
	else
		return;

	_action._inProgress = false;
}

void room_511_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._handingLine);
	s.syncAsByte(local._lineMoving);

	s.syncAsSint16LE(local._lineAnimationMode);
	s.syncAsSint16LE(local._lineFrame);
	s.syncAsSint16LE(local._lineAnimationPosition);
}

void room_511_preload() {
	room_init_code_pointer = room_511_init;
	room_daemon_code_pointer = room_511_daemon;
	room_pre_parser_code_pointer = room_511_pre_parser;
	room_parser_code_pointer = room_511_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(NOUN_BOAT);
	_scene->addActiveVocab(NOUN_FISHING_LINE);
	_scene->addActiveVocab(VERB_WALKTO);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
