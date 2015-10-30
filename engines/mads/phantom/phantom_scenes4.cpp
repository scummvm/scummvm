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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

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
		_scene->_initialVariant = 1;

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

	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RRD_9", false);
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('f', 0), false);
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('f', 1), false);
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('f', 2), false);
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('f', 3), false);

	if (_game.exitCatacombs(0) == -1) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 1), false);
		_scene->drawToBackground(_globals._spriteIndexes[1], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_NORTH, false);
	}

	if (_game.exitCatacombs(3) == -1) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('c', 0), false);
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_WEST, false);
	}

	if (_game.exitCatacombs(1) == -1) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 2), false);
		_scene->drawToBackground(_globals._spriteIndexes[2], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_ARCHWAY_TO_EAST, false);
	}

	if (_game.exitCatacombs(2) == -1)
		_scene->_hotspots.activate(NOUN_MORE_CATACOMBS, false);

	if (_globals[kCatacombsMisc] & MAZE_EVENT_RAT_NEST) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites (formAnimName('c', 4), false);
		_scene->drawToBackground(_globals._spriteIndexes[4], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_RATS_NEST, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_SKULL) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites (formAnimName('c', 5), false);
		_scene->drawToBackground(_globals._spriteIndexes[5], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_SKULL, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_POT) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites (formAnimName('c', 6), false);
		_scene->drawToBackground(_globals._spriteIndexes[6], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_POT, true);
	}

	if (_globals[kCatacombsMisc] & MAZE_EVENT_BRICK) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites (formAnimName('c', 7), false);
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
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[8], 2, 5, 1);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[8], 0, 0, 2);
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
					_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[8]);
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
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[8], 2, 5, 1);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[8], 0, 0, 2);
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
					_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[8]);
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

} // End of namespace Phantom
} // End of namespace MADS
