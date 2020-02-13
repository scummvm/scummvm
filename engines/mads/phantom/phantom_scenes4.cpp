/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/conversations.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"
#include "mads/phantom/phantom_scenes4.h"

namespace MADS {

namespace Phantom {

void Scene4xx::setAAName() {
	_game._aaName = Resources::formatAAName(1);
	_vm->_palette->setEntry(254, 43, 47, 51);
}

void Scene4xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	_vm->_sound->command(16);
}

void Scene4xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;
	if (!_game._player._forcePrefix)
		_game._player._spritesPrefix = "RAL";
	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

Scene401::Scene401(MADSEngine *vm) : Scene4xx(vm) {
	_anim0ActvFl = false;
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHostpotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene401::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHostpotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene401::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kCatacombsMisc] & MAZE_EVENT_PUDDLE)
		_scene->_variant = 1;

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene401::enter() {
	_game.initCatacombs();
	_anim0ActvFl = false;

	_scene->_hotspots.activate(NOUN_PUDDLE, false);
	_scene->_hotspots.activate(NOUN_RATS_NEST, false);
	_scene->_hotspots.activate(NOUN_SKULL, false);
	_scene->_hotspots.activate(NOUN_POT, false);

	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('f', 3));

	if (_game.exitCatacombs(0) == -1) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 1));
		_scene->drawToBackground(_globals._spriteIndexes[1], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, false);
	}

	if (_game.exitCatacombs(3) == -1) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('c', 0));
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_WEST, false);
	}

	if (_game.exitCatacombs(1) == -1) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 2));
		_scene->drawToBackground(_globals._spriteIndexes[2], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_EAST, false);
	}

	if (_game.exitCatacombs(2) == -1)
		_scene->_hotspots.activate(NOUN_MORE_CATACOMBS, false);

	if (_globals[kCatacombsMisc] & MAZE_EVENT_RAT_NEST) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', 4));
		_scene->drawToBackground(_globals._spriteIndexes[4], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_RATS_NEST, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_SKULL) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('c', 5));
		_scene->drawToBackground(_globals._spriteIndexes[5], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_SKULL, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_POT) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 6));
		_scene->drawToBackground(_globals._spriteIndexes[6], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_POT, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_BRICK) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 7));
		_scene->drawToBackground(_globals._spriteIndexes[7], 1, Common::Point(-32000, -32000), 0, 100);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_PUDDLE) {
		_scene->_hotspots.activate(NOUN_PUDDLE, true);
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 0);
		_anim0ActvFl = true;
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 0:
			_game._player._playerPos = Common::Point(128, 78);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(128, 91), FACING_SOUTH);
			break;

		case 1:
			_game._player._playerPos = Common::Point(311, 115);
			_game._player._facing = FACING_WEST;
			_game._player.walk(Common::Point(271, 123), FACING_WEST);
			break;

		case 2:
			_game._player._playerPos = Common::Point(142, 146);
			_game._player._facing = FACING_NORTH;
			break;

		case 3:
			_game._player._playerPos = Common::Point(4, 113);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(48, 113), FACING_EAST);
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId , Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	sceneEntrySound();
}

void Scene401::step() {
	if (_anim0ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 20)
			_scene->setAnimFrame(_globals._animationIndexes[0], 0);
	}
}

void Scene401::actions() {
	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME)) {
			if (_frameInRoomFl)
				_vm->_dialogs->show(29);
			else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 5);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (_action.isObject(NOUN_RED_FRAME)) {
						_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
						_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_GREEN_FRAME)) {
						_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
						_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId , Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_BLUE_FRAME)) {
						_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
						_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_YELLOW_FRAME)) {
						_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
						_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}
					break;

				case 2:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
					_game._player._visible = true;
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME)) {
			if ((_takingFrameInRoomFl || _game._trigger)) {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 5);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (_action.isObject(NOUN_RED_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[9]);
						_scene->_dynamicHotspots.remove(_redFrameHotspotId);
						_game._objects.addToInventory(OBJ_RED_FRAME);
					}

					if (_action.isObject(NOUN_GREEN_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[10]);
						_scene->_dynamicHotspots.remove(_greenFrameHostpotId );
						_game._objects.addToInventory(OBJ_GREEN_FRAME);
					}

					if (_action.isObject(NOUN_BLUE_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[11]);
						_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
						_game._objects.addToInventory(OBJ_BLUE_FRAME);
					}

					if (_action.isObject(NOUN_YELLOW_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[12]);
						_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
						_game._objects.addToInventory(OBJ_YELLOW_FRAME);
					}

					_vm->_sound->command(26);
					break;

				case 2:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
					_game._player._visible = true;
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
				_action._inProgress = false;
				return;
			}
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_NORTH)) {
		_game.moveCatacombs(0);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		_game.moveCatacombs(3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS)) {
		_game.moveCatacombs(2);
		if ((_game._difficulty == DIFFICULTY_HARD) && (_globals[kCatacombsRoom] == 31))
			_globals[kPriestPistonPuke] = true;

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_EAST)) {
		_game.moveCatacombs(1);
		if ((_game._difficulty == DIFFICULTY_EASY) && (_globals[kCatacombsRoom] == 24))
			_globals[kPriestPistonPuke] = true;

		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(40110);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(40111);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(40112);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(40113);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(40114);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(40115);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLOCKED_ARCHWAY)) {
			_vm->_dialogs->show(40116);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PUDDLE)) {
			_vm->_dialogs->show(40117);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RATS_NEST)) {
			_vm->_dialogs->show(40118);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SKULL)) {
			_vm->_dialogs->show(40120);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}


		if (_action.isObject(NOUN_BROKEN_POT)) {
			_vm->_dialogs->show(40122);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_RATS_NEST)) {
		_vm->_dialogs->show(40119);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SKULL)) {
		_vm->_dialogs->show(40121);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BROKEN_POT)) {
		_vm->_dialogs->show(40123);
		_action._inProgress = false;
	}
}

void Scene401::preActions() {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME)
	 || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}
}

/*------------------------------------------------------------------------*/

Scene403::Scene403(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHostpotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene403::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHostpotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene403::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kCatacombsMisc] & MAZE_EVENT_HOLE)
		_scene->_variant = 1;

	if (_globals[kCatacombsMisc] & MAZE_EVENT_PLANK)
		_scene->_variant = 2;

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene403::enter() {
	_game.initCatacombs();

	_scene->_hotspots.activate(NOUN_HOLE, false);
	_scene->_hotspots.activate(NOUN_WEB, false);
	_scene->_hotspots.activate(NOUN_RATS_NEST, false);
	_scene->_hotspots.activate(NOUN_SKULL, false);
	_scene->_hotspots.activate(NOUN_PLANK, false);
	_scene->_hotspots.activate(NOUN_GATE, false);

	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('f', 3));

	if (_game.exitCatacombs(0) == -1) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 1));
		_scene->drawToBackground(_globals._spriteIndexes[1], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, false);
	}

	if (_game.exitCatacombs(3) == -1) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('c', 0));
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_WEST, false);
	}

	if (_game.exitCatacombs(1) == -1) {
		_scene->_hotspots.activate(NOUN_MORE_CATACOMBS, false);
		_scene->_hotspots.activate(NOUN_GATE, true);
		_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('c', 9));
		_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_HOLE) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 3));
		_scene->drawToBackground(_globals._spriteIndexes[2], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_HOLE, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_WEB) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('c', 4));
		_scene->drawToBackground(_globals._spriteIndexes[3], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_WEB, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_BRICK) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', 5));
		_scene->drawToBackground(_globals._spriteIndexes[4], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activateAtPos(NOUN_EXPOSED_BRICK, false, Common::Point(178, 35));
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_RAT_NEST) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('c', 6));
		_scene->drawToBackground(_globals._spriteIndexes[5], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_RATS_NEST, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_SKULL) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 7));
		_scene->drawToBackground(_globals._spriteIndexes[6], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_SKULL, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_PLANK) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 8));
		_scene->drawToBackground(_globals._spriteIndexes[7], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_PLANK, true);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 0:
			_game._player._playerPos = Common::Point(212, 86);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(212, 100), FACING_SOUTH);
			break;

		case 1:
			_game._player.firstWalk(Common::Point(330, 126), FACING_EAST, Common::Point(305, 126), FACING_WEST, true);
			break;

		case 3:
			_game._player._playerPos = Common::Point(3, 128);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(40, 128), FACING_EAST);
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if ((_game._difficulty == DIFFICULTY_EASY) && (_globals[kCatacombsRoom] == 19))
		_scene->_sequences.addTimer(120, 60);

	sceneEntrySound();
}

void Scene403::step() {
	if (_game._trigger == 60)
		_vm->_dialogs->show(31);
}

void Scene403::actions() {
	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_vm->_dialogs->show(29);
		else {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
					_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
					_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
					_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
					_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
					_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
					_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
					_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
					_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME))) {
		if ((_takingFrameInRoomFl || _game._trigger)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[9]);
					_scene->_dynamicHotspots.remove(_redFrameHotspotId);
					_game._objects.addToInventory(OBJ_RED_FRAME);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[10]);
					_scene->_dynamicHotspots.remove(_greenFrameHostpotId);
					_game._objects.addToInventory(OBJ_GREEN_FRAME);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[11]);
					_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
					_game._objects.addToInventory(OBJ_BLUE_FRAME);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[12]);
					_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
					_game._objects.addToInventory(OBJ_YELLOW_FRAME);
				}

				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_NORTH)) {
		_game.moveCatacombs(0);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		_game.moveCatacombs(3);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(40310);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(40311);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(40312);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(40313);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(40314);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(40315);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLOCKED_ARCHWAY)) {
			_vm->_dialogs->show(40316);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RATS_NEST)) {
			_vm->_dialogs->show(40318);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SKULL)) {
			_vm->_dialogs->show(40320);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOLE)) {
			_vm->_dialogs->show(40323);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WEB)) {
			_vm->_dialogs->show(40324);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GATE)) {
			_vm->_dialogs->show(45330);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PLANK)) {
			_vm->_dialogs->show(40325);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_GATE)) {
		_vm->_dialogs->show(45331);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_RATS_NEST)) {
		_vm->_dialogs->show(40319);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SKULL)) {
		_vm->_dialogs->show(40321);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_PLANK)) {
		_vm->_dialogs->show(40326);
		_action._inProgress = false;
		return;
	}
}

void Scene403::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS))
		_game.moveCatacombs(1);

	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}
}

/*------------------------------------------------------------------------*/

Scene404::Scene404(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;
	_anim0ActvFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHostpotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene404::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);
	s.syncAsByte(_anim0ActvFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHostpotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene404::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kCatacombsMisc] & MAZE_EVENT_BLOCK)
		_scene->_variant = 1;

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene404::enter() {
	_game.initCatacombs();

	_scene->_hotspots.activate(NOUN_RATS_NEST, false);
	_scene->_hotspots.activate(NOUN_WEB, false);
	_scene->_hotspots.activate(NOUN_BROKEN_POT, false);
	_scene->_hotspots.activate(NOUN_BLOCK, false);
	_scene->_hotspots.activate(NOUN_PUDDLE, false);

	_anim0ActvFl = false;

	_globals._spriteIndexes[9] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('f', 3));


	if (_game.exitCatacombs(0) == -1) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 1));
		_scene->drawToBackground(_globals._spriteIndexes[1], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, false);
	}

	if (_game.exitCatacombs(3) == -1) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('c', 0));
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_WEST, false);
	}

	if (_game.exitCatacombs(1) == -1) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 2));
		_scene->drawToBackground(_globals._spriteIndexes[2], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_EAST, false);
	}

	if (_game.exitCatacombs(2) == -1)
		_scene->_hotspots.activate(NOUN_MORE_CATACOMBS, false);

	if (_globals[kCatacombsMisc] & MAZE_EVENT_RAT_NEST) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('c', 3));
		_scene->drawToBackground(_globals._spriteIndexes[3], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_RATS_NEST, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_WEB) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', 4));
		_scene->drawToBackground(_globals._spriteIndexes[4], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_WEB, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_POT) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('c', 5));
		_scene->drawToBackground(_globals._spriteIndexes[5], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_BROKEN_POT, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_BLOCK) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 7));
		_scene->drawToBackground(_globals._spriteIndexes[7], 1, Common::Point(-32000, -32000), 0, 100);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_FALLEN_BLOCK) {
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('c', 8));
		_scene->drawToBackground(_globals._spriteIndexes[8], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_BLOCK, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_PUDDLE) {
		_scene->_hotspots.activate(NOUN_PUDDLE, true);
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 0);
		_anim0ActvFl = true;
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 0:
			_game._player._playerPos = Common::Point(156, 98);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(156, 117), FACING_SOUTH);
			break;

		case 1:
			_game._player._playerPos = Common::Point(319, 135);
			_game._player._facing = FACING_WEST;
			_game._player.walk(Common::Point(279, 135), FACING_WEST);
			break;

		case 2:
			_game._player._playerPos = Common::Point(175, 147);
			_game._player._facing = FACING_NORTH;
			break;

		case 3:
			_game._player._playerPos = Common::Point(17, 131);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(60, 131), FACING_EAST);
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);
		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
		_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_globals[kPriestPistonPuke])
		_scene->_sequences.addTimer(120, 60);

	sceneEntrySound();
}

void Scene404::step() {
	if (_anim0ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 20)
			_scene->setAnimFrame(_globals._animationIndexes[0], 0);
	}

	if (_game._trigger == 60) {
		_vm->_dialogs->show(30);
		_globals[kPriestPistonPuke] = false;
	}
}

void Scene404::actions() {
	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_vm->_dialogs->show(29);
		else {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[9] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[9], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[9], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
					_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
					_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
					_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
					_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
					_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
					_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
					_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
					_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[9]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE)) {
		if ((_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME)) && (_takingFrameInRoomFl || _game._trigger)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[9] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[9], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[9], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[10]);
					_scene->_dynamicHotspots.remove(_redFrameHotspotId);
					_game._objects.addToInventory(OBJ_RED_FRAME);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[11]);
					_scene->_dynamicHotspots.remove(_greenFrameHostpotId);
					_game._objects.addToInventory(OBJ_GREEN_FRAME);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[12]);
					_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
					_game._objects.addToInventory(OBJ_BLUE_FRAME);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[13]);
					_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
					_game._objects.addToInventory(OBJ_YELLOW_FRAME);
				}

				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[9]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_NORTH)) {
		_game.moveCatacombs(0);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		_game.moveCatacombs(3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS)) {
		_game.moveCatacombs(2);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_EAST)) {
		_game.moveCatacombs(1);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(40410);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(40411);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(40412);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(40413);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(40414);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(40415);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PUDDLE)) {
			_vm->_dialogs->show(40417);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RATS_NEST)) {
			_vm->_dialogs->show(40418);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BROKEN_POT)) {
			_vm->_dialogs->show(40421);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WEB)) {
			_vm->_dialogs->show(40424);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLOCK)) {
			_vm->_dialogs->show(40430);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_RATS_NEST)) {
		_vm->_dialogs->show(40419);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BROKEN_POT)) {
		_vm->_dialogs->show(40422);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BLOCK)) {
		_vm->_dialogs->show(40431);
		_action._inProgress = false;
		return;
	}
}

void Scene404::preActions() {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[10]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}
}

/*------------------------------------------------------------------------*/

Scene406::Scene406(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHostpotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene406::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHostpotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene406::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene406::enter() {
	_game.initCatacombs();

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 3));

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 1:
			_game._player._playerPos = Common::Point(310, 118);
			_game._player._facing = FACING_WEST;
			_game._player.walk(Common::Point(271, 118), FACING_WEST);
			break;

		case 3:
			_game._player._playerPos = Common::Point(20, 122);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(66, 122), FACING_EAST);
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	sceneEntrySound();
}

void Scene406::step() {
}

void Scene406::actions() {
	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_vm->_dialogs->show(29);
		else {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
					_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
					_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
					_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
					_greenFrameHostpotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_greenFrameHostpotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
					_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
					_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
					_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
					_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME))) {
		if ((_takingFrameInRoomFl || _game._trigger)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[1]);
					_scene->_dynamicHotspots.remove(_redFrameHotspotId);
					_game._objects.addToInventory(OBJ_RED_FRAME);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[2]);
					_scene->_dynamicHotspots.remove(_greenFrameHostpotId);
					_game._objects.addToInventory(OBJ_GREEN_FRAME);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
					_game._objects.addToInventory(OBJ_BLUE_FRAME);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[4]);
					_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
					_game._objects.addToInventory(OBJ_YELLOW_FRAME);
				}

				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		_game.moveCatacombs(3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_EAST)) {
		_game.moveCatacombs(1);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(40610);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(40611);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(40612);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(40613);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(40614);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(40615);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLOCKED_ARCHWAY)) {
			_vm->_dialogs->show(40616);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRATE)) {
			_vm->_dialogs->show(40617);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_GRATE) || _action.isAction(VERB_PUSH, NOUN_GRATE) || _action.isAction(VERB_PULL, NOUN_GRATE)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], true, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], true, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
			_scene->_sequences.addTimer(30, 3);
			break;

		case 3:
			_scene->deleteSequence(_globals._sequenceIndexes[0]);
			_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], true, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], false);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
			_scene->_sequences.addTimer(6, 5);
			break;

		case 5:
			_game._player._stepEnabled = true;
			_vm->_dialogs->show(40618);
			break;

		default:
			break;
		}
		_action._inProgress = false;
	}
}

void Scene406::preActions() {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
			pos.x += 12;
			_game._player.walk(pos, FACING_NORTHWEST);
		}
	}
}

/*------------------------------------------------------------------------*/

Scene407::Scene407(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHotspotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene407::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHotspotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene407::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene407::enter() {
	_game.initCatacombs();

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 3));

	if (_game.exitCatacombs(3) == -1) {
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(9, 46));
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(8, 138));
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(12, 149));
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(0, 151));
	}

	if (_game.exitCatacombs(1) == -1) {
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(310, 107));
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(308, 175));
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(308, 146));
		_scene->_hotspots.activateAtPos(NOUN_MORE_CATACOMBS, false, Common::Point(309, 152));
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 0:
			_game._player._playerPos = Common::Point(197, 14);
			_game._player._facing = FACING_WEST;
			_game._player.walk(Common::Point(181, 14), FACING_WEST);
			break;

		case 1:
			_game._player.firstWalk(Common::Point(330, 146), FACING_WEST, Common::Point(298, 146), FACING_WEST, true);
			break;

		case 2:
			_game._player._playerPos = Common::Point(147, 14);
			_game._player._facing = FACING_EAST;
			_game._player.walk(Common::Point(165, 14), FACING_EAST);
			break;

		case 3:
			_game._player.firstWalk(Common::Point(-20, 143), FACING_WEST, Common::Point(20, 143), FACING_WEST, true);
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	sceneEntrySound();
}

void Scene407::step() {
}

void Scene407::actions() {
	if (_action.isAction(VERB_WALK_TO, NOUN_WALL) && (_game._player._playerPos.y > 30) && (_scene->_customDest.x > 160) && (_scene->_customDest.x < 190)) {
		_vm->_dialogs->show(40718);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_TO) && (_scene->_customDest.x < 130) && (_game._player._playerPos.y < 30)) {
		_vm->_dialogs->show(40718);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_TO) && (_scene->_customDest.x > 203) && (_game._player._playerPos.y < 30)) {
		_vm->_dialogs->show(40718);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_TO, NOUN_FLOOR)) {
		if ((_game._player._playerPos.y < 30) && (_scene->_customDest.y > 29)) {
			_vm->_dialogs->show(40718);
			_action._inProgress = false;
			return;
		} else if ((_game._player._playerPos.y > 29) && (_scene->_customDest.y < 30)) {
			_vm->_dialogs->show(40718);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME)) {
			if ((_game._player._playerPos.y < 30) && (_scene->_customDest.y < 30))
				_vm->_dialogs->show(40717);
			else if ((_game._player._playerPos.y < 30) && (_scene->_customDest.y > 29))
				_vm->_dialogs->show(40718);
			else if (_frameInRoomFl)
				_vm->_dialogs->show(29);
			else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], true, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (_action.isObject(NOUN_RED_FRAME)) {
						_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
						_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
						_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_GREEN_FRAME)) {
						_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
						_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
						_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_BLUE_FRAME)) {
						_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
						_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
						_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_YELLOW_FRAME)) {
						_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
						_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
						_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}
					break;

				case 2:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
					_game._player._visible = true;
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME)) {
			if ((_takingFrameInRoomFl || _game._trigger)) {
				if (_game._player._playerPos.y < 30)
					_vm->_dialogs->show(40718);
				else {
					switch (_game._trigger) {
					case (0):
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], true, 5, 2);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
						_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 5, 1);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
						break;

					case 1:
						if (_action.isObject(NOUN_RED_FRAME)) {
							_scene->deleteSequence(_globals._sequenceIndexes[1]);
							_scene->_dynamicHotspots.remove(_redFrameHotspotId);
							_game._objects.addToInventory(OBJ_RED_FRAME);
						}

						if (_action.isObject(NOUN_GREEN_FRAME)) {
							_scene->deleteSequence(_globals._sequenceIndexes[2]);
							_scene->_dynamicHotspots.remove(_greenFrameHotspotId);
							_game._objects.addToInventory(OBJ_GREEN_FRAME);
						}

						if (_action.isObject(NOUN_BLUE_FRAME)) {
							_scene->deleteSequence(_globals._sequenceIndexes[3]);
							_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
							_game._objects.addToInventory(OBJ_BLUE_FRAME);
						}

						if (_action.isObject(NOUN_YELLOW_FRAME)) {
							_scene->deleteSequence(_globals._sequenceIndexes[4]);
							_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
							_game._objects.addToInventory(OBJ_YELLOW_FRAME);
						}

						_vm->_sound->command(26);
						break;

					case 2:
						_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
						_game._player._visible = true;
						_game._player._stepEnabled = true;
						break;

					default:
						break;
					}
				}
				_action._inProgress = false;
				return;
			}
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		if (_game._player._playerPos.y < 30)
			_game.moveCatacombs(2);
		else
			_vm->_dialogs->show(40718);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_EAST)) {
		if (_game._player._playerPos.y < 30)
			_game.moveCatacombs(0);
		else
			_vm->_dialogs->show(40718);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS) && (_game._player._playerPos.y < 30)) {
		_vm->_dialogs->show(40718);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(40710);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(40711);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(40712);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(40713);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(40714);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COLUMN)) {
			_vm->_dialogs->show(40715);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 818, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LAKE)) {
			_vm->_dialogs->show(40716);
			_action._inProgress = false;
		}
	}
}

void Scene407::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS)) {
		if (_game._player._playerPos.y > 30) {
			if (_scene->_customDest.x < 100)
				_game.moveCatacombs(3);
			else
				_game.moveCatacombs(1);
		} else
			_game._player._needToWalk = false;
	}

	if (_action.isAction(VERB_WALK_THROUGH) && (_game._player._playerPos.y > 30))
		_game._player._needToWalk = false;

	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if ((_frameInRoomFl) || (_game._player._playerPos.y < 30) || (_scene->_customDest.y < 30))
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[1]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}

	if (_action.isAction(VERB_TAKE)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME))) {
		if (_takingFrameInRoomFl && (_game._player._playerPos.y < 30))
			_game._player._needToWalk = false;
	}

	if (_action.isAction(VERB_WALK_ACROSS, NOUN_FLOOR) && (_game._player._playerPos.y < 30) && (_scene->_customDest.y > 29))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_WALK_TO, NOUN_LAKE) && (_game._player._playerPos.y < 30))
		_game._player.walk(Common::Point(172, 18), FACING_SOUTH);

	if (_action.isAction(VERB_WALK_TO) && (_scene->_customDest.x < 130) && (_game._player._playerPos.y < 30))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_WALK_TO) && (_scene->_customDest.x > 203) && (_game._player._playerPos.y < 30))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_WALK_TO, NOUN_WALL) && (_game._player._playerPos.y > 30) && (_scene->_customDest.x > 160) && (_scene->_customDest.x < 190))
			_game._player._needToWalk = false;
}

/*------------------------------------------------------------------------*/

Scene408::Scene408(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHotspotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene408::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHotspotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene408::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if ((_globals[kCatacombsMisc] & MAZE_EVENT_WEB) && (!_globals[kCobwebIsCut]))
		_scene->_variant = 1;

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene408::enter() {
	_game.initCatacombs();
	_scene->_hotspots.activate(NOUN_COBWEB, false);

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', 3));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 2));

	if (_game.exitCatacombs(0) == -1) {
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, false);
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(147, 121));
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(153, 121));
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(154, 115));
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(161, 101));
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(162, 106));
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(187, 107));
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(185, 101));
		_scene->_hotspots.activateAtPos(NOUN_FLOOR, false, Common::Point(192, 119));
		_scene->_hotspots.activateAtPos(NOUN_WALL, false, Common::Point(147, 76));
		_scene->_hotspots.activateAtPos(NOUN_WALL, false, Common::Point(159, 108));
		_scene->_hotspots.activateAtPos(NOUN_WALL, false, Common::Point(185, 93));
		_scene->_hotspots.activateAtPos(NOUN_WALL, false, Common::Point(199, 91));
		_scene->changeVariant(1);
	} else
		_scene->_hotspots.activate(NOUN_GATE, false);

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 0:
			_game._player._playerPos = Common::Point(174, 100);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(174, 106), FACING_SOUTH);
			break;

		case 2:
			_game._player._playerPos = Common::Point(175, 145);
			_game._player._facing = FACING_NORTH;
			break;

		default:
			break;
		}
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_WEB) {
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('x', -1));
		if (!_globals[kCobwebIsCut]) {
			_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 1));
			_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 10);
			_scene->_hotspots.activate(NOUN_COBWEB, true);
		} else {
			_globals._sequenceIndexes[8] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[8], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 10);

			int idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(199, 112, 199 + 6, 112 + 12));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 125), FACING_NORTHEAST);
			idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(202, 81, 202 + 5, 81 + 31));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 125), FACING_NORTHEAST);

			idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(138, 74, 138 + 7, 74 + 33));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(154, 124), FACING_NORTHWEST);
			idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(143, 107, 143 + 6, 107 + 15));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(154, 124), FACING_NORTHWEST);
		}
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_BRICK) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 2));
		_scene->drawToBackground(_globals._spriteIndexes[7], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_EXPOSED_BRICK, false);
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if ((_game._difficulty == DIFFICULTY_HARD) && (_globals[kCatacombsRoom] == 52))
		_scene->_sequences.addTimer(120, 60);

	sceneEntrySound();
}

void Scene408::step() {
	if (_game._trigger == 60)
		_vm->_dialogs->show(31);
}

void Scene408::actions() {
	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_vm->_dialogs->show(29);
		else {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
					_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
					_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
					_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
					_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
					_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
					_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
					_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
					_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

					Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
					_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
					_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
				}
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}


	if (_action.isAction(VERB_TAKE)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME))) {
		if ((_takingFrameInRoomFl || _game._trigger)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[2]);
					_scene->_dynamicHotspots.remove(_redFrameHotspotId);
					_game._objects.addToInventory(OBJ_RED_FRAME);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_scene->_dynamicHotspots.remove(_greenFrameHotspotId);
					_game._objects.addToInventory(OBJ_GREEN_FRAME);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[4]);
					_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
					_game._objects.addToInventory(OBJ_BLUE_FRAME);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[5]);
					_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
					_game._objects.addToInventory(OBJ_YELLOW_FRAME);
				}

				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_NORTH)) {
		_game.moveCatacombs(0);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS)) {
		_game.moveCatacombs(2);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(40810);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(40811);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(40812);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(40813);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(40814);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(40815);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(40816);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GATE)) {
			_vm->_dialogs->show(40817);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COBWEB)) {
			if (_globals[kCobwebIsCut])
				_vm->_dialogs->show(40820);
			else
				_vm->_dialogs->show(40819);

			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isAction(VERB_ATTACK, NOUN_COBWEB) && !_globals[kCobwebIsCut])) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('W', 1), 70);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals[kCobwebIsCut] = true;
		_scene->deleteSequence(_globals._sequenceIndexes[6]);
		_action._inProgress = false;
		return;
	}

	if (_game._trigger == 70) {
		_globals._sequenceIndexes[8] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[8], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 14);
		_scene->changeVariant(0);
		_scene->_hotspots.activate(NOUN_COBWEB, false);
		_game._player._visible = true;
		_game._player._stepEnabled = true;

		int idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(199, 112, 199 + 6, 112 + 12));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 125), FACING_NORTHEAST);
		idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(202, 81, 202 + 5, 81 + 31));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 125), FACING_NORTHEAST);

		idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(138, 74, 138 + 7, 74 + 33));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(154, 124), FACING_NORTHWEST);
		idx = _scene->_dynamicHotspots.add(NOUN_COBWEB, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(143, 107, 143 + 6, 107 + 15));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(154, 124), FACING_NORTHWEST);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_GATE)) {
		_vm->_dialogs->show(40818);
		_action._inProgress = false;
	}
}

void Scene408::preActions() {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}
}

/*------------------------------------------------------------------------*/

Scene409::Scene409(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHotspotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene409::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHotspotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene409::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene409::enter() {
	_game.initCatacombs();

	_scene->loadSpeech(3);
	_scene->_hotspots.activate(NOUN_SWORD, false);

	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RDR_9");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', 3));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 0));

	if (_game._objects.isInRoom(OBJ_SWORD)) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', 0));
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RRD_9");
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_scene->_hotspots.activate(NOUN_SWORD, true);
	}

	if (!_globals[kDoorIn409IsOpen]) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, false);
	} else
		_scene->_hotspots.activate(NOUN_DOOR, false);

	if (_scene->_priorSceneId == 410) {
		_game._player._facing = FACING_NORTH;
		_game._player._playerPos = Common::Point(229, 106);

		if (_globals[kFlickedLever1] && _globals[kFlickedLever2] && _globals[kFlickedLever3] && _globals[kFlickedLever4]) {
			if ((_globals[kFlickedLever1] == 5) && (_globals[kFlickedLever2] == 18) && (_globals[kFlickedLever3] == 9) && (_globals[kFlickedLever4] == 11)) {
				if (!_globals[kDoorIn409IsOpen]) {
					_globals[kPlayerScore] += 5;
					_vm->_sound->command(24);
					_scene->deleteSequence(_globals._sequenceIndexes[6]);
					_game._player._stepEnabled = false;
					_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], -1, -2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 65);
				}
			} else {
				_globals[kFlickedLever1] = 0;
				_globals[kFlickedLever2] = 0;
				_globals[kFlickedLever3] = 0;
				_globals[kFlickedLever4] = 0;
				_game._player._stepEnabled = false;
				_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 9, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 15, 60);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
				_vm->_sound->command(68);
			}
		} else {
			_globals[kFlickedLever1] = 0;
			_globals[kFlickedLever2] = 0;
			_globals[kFlickedLever3] = 0;
			_globals[kFlickedLever4] = 0;
		}
	} else if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 0:
			_game._player._playerPos = Common::Point(195, 92);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(195, 107), FACING_SOUTH);
			break;

		case 2:
			_game._player._playerPos = Common::Point(184, 45);
			_game._player._facing = FACING_NORTH;
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	sceneEntrySound();
}

void Scene409::step() {
	if (_game._trigger == 60) {
		_game._player._visible = false;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[7]);
		_scene->playSpeech(3);
	}

	if (_game._trigger == 61) {
		_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
		_scene->_sequences.addTimer(60, 62);
		_vm->_sound->command(27);
	}

	if (_game._trigger == 62)
		_scene->_reloadSceneFlag = true;

	if (_game._trigger == 65) {
		_game._player._stepEnabled = true;
		_globals[kDoorIn409IsOpen] = true;
		_scene->_hotspots.activate(NOUN_DOOR, false);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, true);
	}
}

void Scene409::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 4, 4);
			_action._inProgress = false;
			return;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[8];
			_globals._sequenceIndexes[8] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[8], false, 4);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[8], SYNC_SEQ, syncIdx);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], false);
			_scene->_sequences.addTimer(30, 2);
			_action._inProgress = false;
			}
			return;

		case 2:
			_scene->deleteSequence(_globals._sequenceIndexes[8]);
			_globals._sequenceIndexes[8] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[8], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], false);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			_action._inProgress = false;
			return;

		case 3:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
			_game._player._visible = true;
			_vm->_dialogs->show(40923);
			_game._player._stepEnabled = true;
			_action._inProgress = false;
			return;

		case 4:
			_vm->_sound->command(70);
			_action._inProgress = false;
			return;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME)) {
			if (_frameInRoomFl) {
				_vm->_dialogs->show(29);
			} else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], true, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (_action.isObject(NOUN_RED_FRAME)) {
						_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
						_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_GREEN_FRAME)) {
						_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
						_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_BLUE_FRAME)) {
						_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
						_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_YELLOW_FRAME)) {
						_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
						_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}
					break;

				case 2:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
					_game._player._visible = true;
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE)) {
		if ((_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME))
		 && (_takingFrameInRoomFl || _game._trigger)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[2]);
					_scene->_dynamicHotspots.remove(_redFrameHotspotId);
					_game._objects.addToInventory(OBJ_RED_FRAME);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_scene->_dynamicHotspots.remove(_greenFrameHotspotId);
					_game._objects.addToInventory(OBJ_GREEN_FRAME);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[4]);
					_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
					_game._objects.addToInventory(OBJ_BLUE_FRAME);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[5]);
					_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
					_game._objects.addToInventory(OBJ_YELLOW_FRAME);
				}

				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_NORTH)) {
		_game.enterCatacombs(true);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS)) {
		_game.enterCatacombs(false);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(40910);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SWORD) && (_game._objects.isInRoom(OBJ_SWORD) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], true, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 5, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[1]);
			_scene->_hotspots.activate(NOUN_SWORD, false);
			_game._objects.addToInventory(OBJ_SWORD);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_vm->_dialogs->showItem(OBJ_SWORD, 808, 0);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(40911);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(40912);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(40913);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(40914);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(40915);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLOCKED_ARCHWAY)) {
			_vm->_dialogs->show(40916);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRATE)) {
			_vm->_dialogs->show(40917);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_UNLUCKY_ADVENTURER)) {
			_vm->_dialogs->show(40920);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
				_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
				_action._inProgress = false;
				return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
				_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
				_action._inProgress = false;
				return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
				_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
				_action._inProgress = false;
				return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
				_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
				_action._inProgress = false;
				return;
		}

		if (_action.isObject(NOUN_SWORD) && _game._objects.isInRoom(OBJ_SWORD)) {
			_vm->_dialogs->show(40921);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_LOOK, NOUN_SWITCH_PANEL)) {
		_vm->_dialogs->show(40919);
		_scene->_nextSceneId = 410;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_TO, NOUN_SWITCH_PANEL)) {
		_scene->_nextSceneId = 410;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_GRATE) || _action.isAction(VERB_PUSH, NOUN_GRATE) || _action.isAction(VERB_PULL, NOUN_GRATE)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
			_scene->_sequences.addTimer(30, 3);
			break;

		case 3:
			_scene->deleteSequence(_globals._sequenceIndexes[0]);
			_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], false);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
			_scene->_sequences.addTimer(6, 5);
			break;

		case 5:
			_game._player._stepEnabled = true;
			_vm->_dialogs->show(40918);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_UNLUCKY_ADVENTURER)) {
		_vm->_dialogs->show(40924);
		_action._inProgress = false;
	}
}

void Scene409::preActions() {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}

	if (_action.isAction(VERB_LOOK, NOUN_SWITCH_PANEL))
		_game._player.walk(Common::Point(229, 106), FACING_NORTH);

	if (_action.isAction(VERB_OPEN, NOUN_DOOR))
		_game._player.walk(Common::Point(191, 104), FACING_NORTHEAST);

	if (_action.isAction(VERB_OPEN, NOUN_GRATE))
		_game._player._needToWalk = true;
}

/*------------------------------------------------------------------------*/

Scene410::Scene410(MADSEngine *vm) : Scene4xx(vm) {
	for (int i = 0; i < 26; i++)
		_skullSequence[i] = 0;
}

void Scene410::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	for (int i = 0; i < 26; i++)
		s.syncAsSint16LE(_skullSequence[i]);
}

void Scene410::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene410::enter() {
	_game._player._visible = false;
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('l', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('l', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('l', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 0));

	for (int i = 1; i < 27; i++) {
		Common::Point pos;
		int type;

		getLeverInfo(&pos, &type, i, NULL);
		int stampType = -1;

		switch (type) {
		case 1:
			stampType = _globals._spriteIndexes[0];
			break;

		case 2:
			stampType = _globals._spriteIndexes[1];
			break;

		case 3:
			stampType = _globals._spriteIndexes[2];
			break;

		default:
			break;
		}

		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(stampType, false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[0], pos);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
		_skullSequence[i - 1] = _globals._sequenceIndexes[0];
	}

	if (_globals[kFlickedLever1]) {
		Common::Point pos;
		int type;

		getLeverInfo(&pos, &type, _globals[kFlickedLever1], NULL);
		int stampType = -1;

		switch (type) {
		case 1:
			stampType = _globals._spriteIndexes[0];
			break;

		case 2:
			stampType = _globals._spriteIndexes[1];
			break;

		case 3:
			stampType = _globals._spriteIndexes[2];
			break;

		default:
			break;
		}
		_scene->deleteSequence(_skullSequence[_globals[kFlickedLever1] - 1]);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(stampType, false, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[0], pos);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
	}

	if (_globals[kFlickedLever2]) {
		Common::Point pos;
		int type;

		getLeverInfo(&pos, &type, _globals[kFlickedLever2], NULL);
		int stampType = -1;
		switch (type) {
		case 1:
			stampType = _globals._spriteIndexes[0];
			break;

		case 2:
			stampType = _globals._spriteIndexes[1];
			break;

		case 3:
			stampType = _globals._spriteIndexes[2];
			break;

		default:
			break;
		}
		_scene->deleteSequence(_skullSequence[_globals[kFlickedLever2] - 1]);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(stampType, false, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[0], pos);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
	}

	if (_globals[kFlickedLever3]) {
		Common::Point pos;
		int type;

		getLeverInfo(&pos, &type, _globals[kFlickedLever3], NULL);
		int stampType = -1;
		switch (type) {
		case 1:
			stampType = _globals._spriteIndexes[0];
			break;

		case 2:
			stampType = _globals._spriteIndexes[1];
			break;

		case 3:
			stampType = _globals._spriteIndexes[2];
			break;

		default:
			break;
		}
		_scene->deleteSequence(_skullSequence[_globals[kFlickedLever3] - 1]);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(stampType, false, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[0], pos);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
	}

	if (_globals[kFlickedLever4]) {
		Common::Point pos;
		int type;

		getLeverInfo(&pos, &type, _globals[kFlickedLever4], NULL);
		int stampType = -1;
		switch (type) {
		case 1:
			stampType = _globals._spriteIndexes[0];
			break;

		case 2:
			stampType = _globals._spriteIndexes[1];
			break;

		case 3:
			stampType = _globals._spriteIndexes[2];
			break;

		default:
			break;
		}
		_scene->deleteSequence(_skullSequence[_globals[kFlickedLever4] - 1]);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(stampType, false, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[0], pos);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
	}

	sceneEntrySound();
}

void Scene410::step() {
}

void Scene410::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(41013);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) {
		if (_globals[kDoorIn409IsOpen])
			_vm->_dialogs->show(41014);
		else {
			Common::Point pos;
			int type;
			int number;

			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				getLeverInfo(&pos, &type, 0, &number);
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 4, 2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
				if (pos.y == 46)
					pos.y = 48;

				_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(pos.x + 4, pos.y + 107));
				if (!_globals[kDoorIn409IsOpen])
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 16, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				getLeverInfo(&pos, &type, 0, &number);

				if ((_globals[kFlickedLever1] != number) && (_globals[kFlickedLever2] != number) && (_globals[kFlickedLever3] != number) && (_globals[kFlickedLever4] != number)) {
					_vm->_sound->command(65);

					if (!_globals[kFlickedLever1]) {
						_globals[kFlickedLever1] = number;
						if (_globals[kFlickedLever1] == 5)
							_vm->_sound->command(66);
					} else if (!_globals[kFlickedLever2]) {
						_globals[kFlickedLever2] = number;
						if ((_globals[kFlickedLever1] == 5) && (_globals[kFlickedLever2] == 18))
							_vm->_sound->command(66);
					} else if (!_globals[kFlickedLever3]) {
						_globals[kFlickedLever3] = number;
						if ((_globals[kFlickedLever1] == 5) && (_globals[kFlickedLever2] == 18) && (_globals[kFlickedLever3] == 9))
							_vm->_sound->command(66);
					} else if (!_globals[kFlickedLever4]) {
						_globals[kFlickedLever4] = number;
						if ((_globals[kFlickedLever1] == 5) && (_globals[kFlickedLever2] == 18) && (_globals[kFlickedLever3] == 9) && (_globals[kFlickedLever4] == 11))
							_vm->_sound->command(66);
					}

					if (_game._difficulty == DIFFICULTY_EASY)
						_scene->drawToBackground(_globals._spriteIndexes[4], number, Common::Point(-32000, -32000), 0, 100);

					switch (type) {
					case 1:
						_scene->deleteSequence(_skullSequence[number - 1]);
						_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 4, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
						_scene->_sequences.setPosition(_globals._sequenceIndexes[0], pos);
						break;

					case 2:
						_scene->deleteSequence(_skullSequence[number - 1]);
						_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
						_scene->_sequences.setPosition(_globals._sequenceIndexes[1], pos);
						break;

					case 3:
						_scene->deleteSequence(_skullSequence[number - 1]);
						_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
						_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
						_scene->_sequences.setPosition(_globals._sequenceIndexes[2], pos);
						break;

					default:
						break;
					}
				}
				break;

			case 2:
				_game._player._stepEnabled = true;

				if ((_globals[kFlickedLever1] == 5) && (_globals[kFlickedLever2] == 18) && (_globals[kFlickedLever3] == 9) && (_globals[kFlickedLever4] == 11) && !_globals[kDoorIn409IsOpen])
					_vm->_sound->command(67);

				if (_globals[kFlickedLever1] && _globals[kFlickedLever2] && _globals[kFlickedLever3] && _globals[kFlickedLever4])
					_scene->_nextSceneId = 409;

				break;

			case 4:
				getLeverInfo(&pos, &type, 0, &number);
				_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[0], pos);
				break;

			case 5:
				getLeverInfo(&pos, &type, 0, &number);
				_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[1], pos);
				break;

			case 6:
				getLeverInfo(&pos, &type, 0, &number);
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], pos);
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(41011);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SWITCH_PANEL)) {
			_vm->_dialogs->show(41011);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CATACOMB_ROOM)) {
			_vm->_dialogs->show(41015);
			_action._inProgress = false;
			return;
		}

		switch (_action._activeAction._objectNameId) {
		case NOUN_SKULL_SWITCH_1:
		case NOUN_SKULL_SWITCH_2:
		case NOUN_SKULL_SWITCH_3:
		case NOUN_SKULL_SWITCH_4:
		case NOUN_SKULL_SWITCH_5:
		case NOUN_SKULL_SWITCH_6:
		case NOUN_SKULL_SWITCH_7:
		case NOUN_SKULL_SWITCH_8:
		case NOUN_SKULL_SWITCH_9:
		case NOUN_SKULL_SWITCH_10:
		case NOUN_SKULL_SWITCH_11:
		case NOUN_SKULL_SWITCH_12:
		case NOUN_SKULL_SWITCH_13:
		case NOUN_SKULL_SWITCH_14:
		case NOUN_SKULL_SWITCH_15:
		case NOUN_SKULL_SWITCH_16:
		case NOUN_SKULL_SWITCH_17:
		case NOUN_SKULL_SWITCH_18:
		case NOUN_SKULL_SWITCH_19:
		case NOUN_SKULL_SWITCH_20:
		case NOUN_SKULL_SWITCH_21:
		case NOUN_SKULL_SWITCH_22:
		case NOUN_SKULL_SWITCH_23:
		case NOUN_SKULL_SWITCH_24:
		case NOUN_SKULL_SWITCH_25:
		case NOUN_SKULL_SWITCH_26:
			_vm->_dialogs->show(41012);
			_action._inProgress = false;
			return;
			break;
		default:
			break;
		}
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_CATACOMB_ROOM)) {
		_scene->_nextSceneId = 409;
		_action._inProgress = false;
	}
}

void Scene410::preActions() {
}

void Scene410::getLeverInfo(Common::Point *pos, int *type, int lever, int *noun) {
	if (noun != NULL) {
		switch (_action._activeAction._objectNameId) {
		case NOUN_SKULL_SWITCH_1:
			lever = 1;
			break;

		case NOUN_SKULL_SWITCH_2:
			lever = 2;
			break;

		case NOUN_SKULL_SWITCH_3:
			lever = 3;
			break;

		case NOUN_SKULL_SWITCH_4:
			lever = 4;
			break;

		case NOUN_SKULL_SWITCH_5:
			lever = 5;
			break;

		case NOUN_SKULL_SWITCH_6:
			lever = 6;
			break;

		case NOUN_SKULL_SWITCH_7:
			lever = 7;
			break;

		case NOUN_SKULL_SWITCH_8:
			lever = 8;
			break;

		case NOUN_SKULL_SWITCH_9:
			lever = 9;
			break;

		case NOUN_SKULL_SWITCH_10:
			lever = 10;
			break;

		case NOUN_SKULL_SWITCH_11:
			lever = 11;
			break;

		case NOUN_SKULL_SWITCH_12:
			lever = 12;
			break;

		case NOUN_SKULL_SWITCH_13:
			lever = 13;
			break;

		case NOUN_SKULL_SWITCH_14:
			lever = 14;
			break;

		case NOUN_SKULL_SWITCH_15:
			lever = 15;
			break;

		case NOUN_SKULL_SWITCH_16:
			lever = 16;
			break;

		case NOUN_SKULL_SWITCH_17:
			lever = 17;
			break;

		case NOUN_SKULL_SWITCH_18:
			lever = 18;
			break;

		case NOUN_SKULL_SWITCH_19:
			lever = 19;
			break;

		case NOUN_SKULL_SWITCH_20:
			lever = 20;
			break;

		case NOUN_SKULL_SWITCH_21:
			lever = 21;
			break;

		case NOUN_SKULL_SWITCH_22:
			lever = 22;
			break;

		case NOUN_SKULL_SWITCH_23:
			lever = 23;
			break;

		case NOUN_SKULL_SWITCH_24:
			lever = 24;
			break;

		case NOUN_SKULL_SWITCH_25:
			lever = 25;
			break;

		case NOUN_SKULL_SWITCH_26:
			lever = 26;
			break;

		default:
			break;
		}
		*noun = lever;
	}

	switch (lever) {
	case 1:
		*pos = Common::Point(124, 46);
		*type = 3;
		break;

	case 2:
		*pos = Common::Point(143, 46);
		*type = 2;
		break;

	case 3:
		*pos = Common::Point(162, 46);
		*type = 1;
		break;

	case 4:
		*pos = Common::Point(181, 46);
		*type = 3;
		break;

	case 5:
		*pos = Common::Point(200, 46);
		*type = 1;
		break;

	case 6:
		*pos = Common::Point(219, 46);
		*type = 2;
		break;

	case 7:
		*pos = Common::Point(238, 46);
		*type = 1;
		break;

	case 8:
		*pos = Common::Point(133, 71);
		*type = 3;
		break;

	case 9:
		*pos = Common::Point(152, 71);
		*type = 2;
		break;

	case 10:
		*pos = Common::Point(171, 71);
		*type = 1;
		break;

	case 11:
		*pos = Common::Point(190, 71);
		*type = 3;
		break;

	case 12:
		*pos = Common::Point(209, 71);
		*type = 2;
		break;

	case 13:
		*pos = Common::Point(228, 71);
		*type = 1;
		break;

	case 14:
		*pos = Common::Point(124, 98);
		*type = 1;
		break;

	case 15:
		*pos = Common::Point(143, 98);
		*type = 3;
		break;

	case 16:
		*pos = Common::Point(162, 98);
		*type = 2;
		break;

	case 17:
		*pos = Common::Point(181, 98);
		*type = 1;
		break;

	case 18:
		*pos = Common::Point(200, 98);
		*type = 1;
		break;

	case 19:
		*pos = Common::Point(219, 98);
		*type = 2;
		break;

	case 20:
		*pos = Common::Point(238, 98);
		*type = 1;
		break;

	case 21:
		*pos = Common::Point(133, 125);
		*type = 3;
		break;

	case 22:
		*pos = Common::Point(152, 125);
		*type = 1;
		break;

	case 23:
		*pos = Common::Point(171, 125);
		*type = 3;
		break;

	case 24:
		*pos = Common::Point(190, 125);
		*type = 2;
		break;

	case 25:
		*pos = Common::Point(209, 125);
		*type = 1;
		break;

	case 26:
		*pos = Common::Point(228, 125);
		*type = 2;
		break;

	default:
		*pos = Common::Point(-1, -1);
		*type = -1;
		break;
	}
}

/*------------------------------------------------------------------------*/

Scene453::Scene453(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHotspotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene453::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHotspotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene453::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kCatacombsMisc] & MAZE_EVENT_STONE)
		_scene->_variant = 1;

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene453::enter() {
	_game.initCatacombs();

	_scene->_hotspots.activate(NOUN_SKULL, false);
	_scene->_hotspots.activate(NOUN_DRAIN, false);
	_scene->_hotspots.activate(NOUN_RATS_NEST, false);
	_scene->_hotspots.activate(NOUN_WEB, false);
	_scene->_hotspots.activate(NOUN_STONE, false);
	_scene->_hotspots.activate(NOUN_HOLE, false);
	_scene->_hotspots.activate(NOUN_GATE, false);

	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('f', 3));


	if (_game.exitCatacombs(0) == -1) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 1));
		_scene->drawToBackground(_globals._spriteIndexes[1], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, false);
	}

	if (_game.exitCatacombs(3) == -1) {
		_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('c', 8));
		_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_MORE_CATACOMBS, false);
		_scene->_hotspots.activate(NOUN_GATE, true);
	}

	if (_game.exitCatacombs(1) == -1) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 2));
		_scene->drawToBackground(_globals._spriteIndexes[2], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_EAST, false);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_DRAIN) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('c', 3));
		_scene->drawToBackground(_globals._spriteIndexes[3], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_DRAIN, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_RAT_NEST) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', 4));
		_scene->drawToBackground(_globals._spriteIndexes[4], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_SKULL, true);
		_scene->_hotspots.activate(NOUN_RATS_NEST, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_WEB) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('c', 5));
		_scene->drawToBackground(_globals._spriteIndexes[5], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_WEB, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_BRICK) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 6));
		_scene->drawToBackground(_globals._spriteIndexes[6], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activateAtPos(NOUN_EXPOSED_BRICK, false, Common::Point(138, 35));
		_scene->_hotspots.activateAtPos(NOUN_EXPOSED_BRICK, false, Common::Point(84, 27));
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_STONE) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 7));
		_scene->drawToBackground(_globals._spriteIndexes[7], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_HOLE, true);
		_scene->_hotspots.activate(NOUN_STONE, true);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 0:
			_game._player._playerPos = Common::Point(107, 87);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(107, 98), FACING_SOUTH);
			break;

		case 1:
			_game._player._playerPos = Common::Point(316, 129);
			_game._player._facing = FACING_WEST;
			_game._player.walk(Common::Point(277, 129), FACING_WEST);
			break;

		case 3:
			_game._player.firstWalk(Common::Point(-20, 128), FACING_EAST, Common::Point(19, 128), FACING_EAST, true);
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	sceneEntrySound();
}

void Scene453::step() {
}

void Scene453::actions() {
	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME)) {
			if (_frameInRoomFl)
				_vm->_dialogs->show(29);
			else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 5);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (_action.isObject(NOUN_RED_FRAME)) {
						_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
						_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_GREEN_FRAME)) {
						_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
						_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_BLUE_FRAME)) {
						_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
						_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_YELLOW_FRAME)) {
						_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
						_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
						_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}
					break;

				case 2:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
					_game._player._visible = true;
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME)) {
			if ((_takingFrameInRoomFl || _game._trigger)) {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 5);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (_action.isObject(NOUN_RED_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[9]);
						_scene->_dynamicHotspots.remove(_redFrameHotspotId);
						_game._objects.addToInventory(OBJ_RED_FRAME);
					}

					if (_action.isObject(NOUN_GREEN_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[10]);
						_scene->_dynamicHotspots.remove(_greenFrameHotspotId);
						_game._objects.addToInventory(OBJ_GREEN_FRAME);
					}

					if (_action.isObject(NOUN_BLUE_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[11]);
						_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
						_game._objects.addToInventory(OBJ_BLUE_FRAME);
					}

					if (_action.isObject(NOUN_YELLOW_FRAME)) {
						_scene->deleteSequence(_globals._sequenceIndexes[12]);
						_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
						_game._objects.addToInventory(OBJ_YELLOW_FRAME);
					}

					_vm->_sound->command(26);
					break;

				case 2:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
					_game._player._visible = true;
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
				_action._inProgress = false;
				return;
			}
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_NORTH)) {
		_game.moveCatacombs(0);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_EAST)) {
		_game.moveCatacombs(1);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(45310);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(45311);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(45312);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(45313);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(45314);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOLE)) {
			_vm->_dialogs->show(45317);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SKULL)) {
			_vm->_dialogs->show(45318);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WEB)) {
			_vm->_dialogs->show(45324);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RATS_NEST)) {
			_vm->_dialogs->show(45325);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DRAIN)) {
			_vm->_dialogs->show(45327);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STONE)) {
			_vm->_dialogs->show(45328);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(45315);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GATE)) {
			_vm->_dialogs->show(45330);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_SKULL)) {
		_vm->_dialogs->show(45319);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_GATE)) {
		_vm->_dialogs->show(45331);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_RATS_NEST)) {
		_vm->_dialogs->show(45326);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_STONE)) {
		_vm->_dialogs->show(45329);
		_action._inProgress = false;
	}
}

void Scene453::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_MORE_CATACOMBS))
		_game.moveCatacombs(3);

	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR) && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[9]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}
}

/*------------------------------------------------------------------------*/

Scene456::Scene456(MADSEngine *vm) : Scene4xx(vm) {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	_redFrameHotspotId = -1;
	_greenFrameHotspotId = -1;
	_blueFrameHotspotId = -1;
	_yellowFrameHotspotId = -1;
}

void Scene456::synchronize(Common::Serializer &s) {
	Scene4xx::synchronize(s);

	s.syncAsByte(_frameInRoomFl);
	s.syncAsByte(_takingFrameInRoomFl);

	s.syncAsSint16LE(_redFrameHotspotId);
	s.syncAsSint16LE(_greenFrameHotspotId);
	s.syncAsSint16LE(_blueFrameHotspotId);
	s.syncAsSint16LE(_yellowFrameHotspotId);
}

void Scene456::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_RED_FRAME);
	_scene->addActiveVocab(NOUN_YELLOW_FRAME);
	_scene->addActiveVocab(NOUN_BLUE_FRAME);
	_scene->addActiveVocab(NOUN_GREEN_FRAME);
}

void Scene456::enter() {
	_game.initCatacombs();

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('c', 1));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', 3));

	if (_game.exitCatacombs(1) == -1) {
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_EAST, false);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		switch (_globals[kCatacombsFrom]) {
		case 1:
			_game._player._playerPos = Common::Point(298, 123);
			_game._player._facing = FACING_WEST;
			_game._player.walk(Common::Point(254, 123), FACING_WEST);
			break;

		case 3:
			_game._player._playerPos = Common::Point(14, 117);
			_game._player._facing = FACING_SOUTH;
			_game._player.walk(Common::Point(46, 117), FACING_EAST);
			break;

		default:
			break;
		}
	}

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

		Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
		_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
		_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
	}

	sceneEntrySound();
}

void Scene456::step() {
}

void Scene456::actions() {
	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)) {
		if (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME)) {
			if (_frameInRoomFl) {
				_vm->_dialogs->show(29);
			} else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], true, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 5);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (_action.isObject(NOUN_RED_FRAME)) {
						_game._objects.setRoom(OBJ_RED_FRAME, NOWHERE);
						_game._objects[OBJ_RED_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_redFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_RED_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_redFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_GREEN_FRAME)) {
						_game._objects.setRoom(OBJ_GREEN_FRAME, NOWHERE);
						_game._objects[OBJ_GREEN_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_greenFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_GREEN_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_greenFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_BLUE_FRAME)) {
						_game._objects.setRoom(OBJ_BLUE_FRAME, NOWHERE);
						_game._objects[OBJ_BLUE_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_blueFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_BLUE_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_blueFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}

					if (_action.isObject(NOUN_YELLOW_FRAME)) {
						_game._objects.setRoom(OBJ_YELLOW_FRAME, NOWHERE);
						_game._objects[OBJ_YELLOW_FRAME]._roomNumber = _globals[kCatacombsRoom] + 600;
						_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

						Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
						_yellowFrameHotspotId = _scene->_dynamicHotspots.add(NOUN_YELLOW_FRAME, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(pos.x - 5, pos.y - 5, pos.x + 5, pos.y + 1));
						_scene->_dynamicHotspots.setPosition(_yellowFrameHotspotId, Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
					}
					break;

				case 2:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
					_game._player._visible = true;
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			}
			_action._inProgress = false;
			return;
		}
	}


	if (_action.isAction(VERB_TAKE)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_GREEN_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME))) {
		if ((_takingFrameInRoomFl || _game._trigger)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_action.isObject(NOUN_RED_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[2]);
					_scene->_dynamicHotspots.remove(_redFrameHotspotId);
					_game._objects.addToInventory(OBJ_RED_FRAME);
				}

				if (_action.isObject(NOUN_GREEN_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_scene->_dynamicHotspots.remove(_greenFrameHotspotId);
					_game._objects.addToInventory(OBJ_GREEN_FRAME);
				}

				if (_action.isObject(NOUN_BLUE_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[4]);
					_scene->_dynamicHotspots.remove(_blueFrameHotspotId);
					_game._objects.addToInventory(OBJ_BLUE_FRAME);
				}

				if (_action.isObject(NOUN_YELLOW_FRAME)) {
					_scene->deleteSequence(_globals._sequenceIndexes[5]);
					_scene->_dynamicHotspots.remove(_yellowFrameHotspotId);
					_game._objects.addToInventory(OBJ_YELLOW_FRAME);
				}

				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		_game.moveCatacombs(3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_EAST)) {
		_game.moveCatacombs(1);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(45610);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(45611);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(45612);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(45613);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(45614);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && !_game._objects.isInInventory(OBJ_RED_FRAME)) {
			_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GREEN_FRAME) && !_game._objects.isInInventory(OBJ_GREEN_FRAME)) {
			_vm->_dialogs->showItem(OBJ_GREEN_FRAME, 819, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && !_game._objects.isInInventory(OBJ_BLUE_FRAME)) {
			_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && !_game._objects.isInInventory(OBJ_YELLOW_FRAME)) {
			_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MORE_CATACOMBS)) {
			_vm->_dialogs->show(45615);
			_action._inProgress = false;
		}
	}
}

void Scene456::preActions() {
	_frameInRoomFl = false;
	_takingFrameInRoomFl = false;

	if (_game._objects[OBJ_RED_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_YELLOW_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_BLUE_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_game._objects[OBJ_GREEN_FRAME]._roomNumber == _globals[kCatacombsRoom] + 600) {
		_frameInRoomFl = true;
		if (_action.isAction(VERB_TAKE, NOUN_GREEN_FRAME))
			_takingFrameInRoomFl = true;
	}

	if (_action.isAction(VERB_PUT) && _action.isTarget(NOUN_FLOOR)
	 && (_action.isObject(NOUN_RED_FRAME) || _action.isObject(NOUN_BLUE_FRAME) || _action.isObject(NOUN_YELLOW_FRAME) || _action.isObject(NOUN_GREEN_FRAME))) {
		if (_frameInRoomFl)
			_game._player._needToWalk = false;
		else {
			Common::Point pos = _scene->_sprites[_globals._spriteIndexes[2]]->getFramePos(0);
			_game._player.walk(Common::Point(pos.x + 12, pos.y), FACING_NORTHWEST);
		}
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
