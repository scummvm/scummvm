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
#include "mads/phantom/phantom_scenes5.h"

namespace MADS {

namespace Phantom {

void Scene5xx::setAAName() {
	_game._aaName = Resources::formatAAName(1);
	_vm->_palette->setEntry(254, 43, 47, 51);
}

void Scene5xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	if ((_globals[kCoffinStatus] == 2) && !_game._visitedScenes.exists(506) && (_globals[kFightStatus] == 0) && (_scene->_currentSceneId == 504))
		_vm->_sound->command(33);
	else if (_scene->_currentSceneId == 505)
		_vm->_sound->command((_vm->_gameConv->restoreRunning() == 20) ? 39 : 16);
	else
		_vm->_sound->command(16);
}

void Scene5xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;
	if (!_game._player._forcePrefix)
		_game._player._spritesPrefix = "RAL";
	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

Scene501::Scene501(MADSEngine *vm) : Scene5xx(vm) {
	_anim0ActvFl = false;
	_skipFl = false;
}

void Scene501::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_skipFl);
}

void Scene501::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_CHRISTINE);
	_scene->addActiveVocab(VERB_LOOK_AT);
	_scene->addActiveVocab(VERB_WALK_TO);
}

void Scene501::enter() {
	_scene->_hotspots.activate(NOUN_CHRISTINE, false);
	_scene->_hotspots.activate(NOUN_BOAT, false);

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_skipFl = false;
	}

	_vm->_gameConv->load(26);

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_6", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_9", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_8", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RDRR_6");

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_globals[kChristineIsInBoat]) {
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 1), 100);
			_scene->setAnimFrame(_globals._animationIndexes[0], 124);
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(113, 93));
			_scene->_hotspots.activate(NOUN_BOAT, true);
		}

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
	}

	if (_scene->_priorSceneId == 506) {
		_game._player._playerPos = Common::Point(305, 112);
		_game._player._facing = FACING_WEST;
		_game._player._stepEnabled = false;

		if (_globals[kChristineIsInBoat]) {
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 1), 100);
			_scene->setAnimFrame(_globals._animationIndexes[0], 124);
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(113, 93));
			_scene->_hotspots.activate(NOUN_BOAT, true);
			_game._player.walk(Common::Point(260, 112), FACING_SOUTHWEST);
			_game._player.setWalkTrigger(80);
			_game._player.setWalkTrigger(55);
		} else {
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 1), 100);
			_globals[kChristineIsInBoat] = true;
			_scene->_hotspots.activate(NOUN_BOAT, true);
			_game._player.walk(Common::Point(260, 112), FACING_SOUTHWEST);
			_game._player.setWalkTrigger(80);
		}

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);

		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
	} else if ((_scene->_priorSceneId == 401) || (_scene->_priorSceneId == 408) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player.firstWalk(Common::Point(-20, 109), FACING_EAST, Common::Point(24, 109), FACING_EAST, true);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);

		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
	}

	sceneEntrySound();
}

void Scene501::step() {
	switch (_game._trigger) {
	case 55:
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 56);
		break;

	case 56:
		_vm->_sound->command(25);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		if (!_globals[kChrisWillTakeSeat])
			_game._player._stepEnabled = true;

		_globals[kChrisWillTakeSeat] = false;
		break;

	default:
		break;
	}


	switch (_game._trigger) {
	case 60:
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_vm->_sound->command(25);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
		_game._player._stepEnabled = true;
		break;

	case 80: {
		_game._player.walk(Common::Point(255, 118), FACING_NORTHWEST);
		_scene->setAnimFrame(_globals._animationIndexes[0], 2);
		int idx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 0);
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 1);
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 2);
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 3);
		}
		break;

	case 90:
		_globals[kPlayerScore] += 5;
		_scene->_nextSceneId = 310;
		break;

	case 100:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 9, 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		break;

	default:
		break;
	}

	if (_anim0ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 103)
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(125, 94));

		if ((_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 28) && !_skipFl) {
			_skipFl = true;
			_scene->_sequences.addTimer(1, 55);
		}

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 124) {
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, false, Common::Point(125, 94));
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true , Common::Point(113, 93));
		}

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 125)
			_scene->setAnimFrame(_globals._animationIndexes[0], 124);

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 1)
			_scene->setAnimFrame(_globals._animationIndexes[0], 0);

		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 84)
			_game._player._stepEnabled = true;
	}
}

void Scene501::actions() {
	if (_vm->_gameConv->activeConvId() == 26) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST) && (_globals[kChristineIsInBoat])) {
		_vm->_gameConv->run(26);
		_vm->_gameConv->exportValue(3);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_CHRISTINE)) {
		_vm->_gameConv->run(26);
		_vm->_gameConv->exportValue(1);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_INTO, NOUN_BOAT)) {
		if (_game._objects.isInInventory(OBJ_OAR))
			_anim0ActvFl = false;
		else
			_vm->_dialogs->show(50123);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_UNLOCK, NOUN_DOOR) || _action.isAction(VERB_LOCK, NOUN_DOOR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int idx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_SEQ, idx);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
			_scene->_sequences.addTimer(15, 2);
			_vm->_sound->command(74);
			}
			break;

		case 2:
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[3]);
			_game._player._visible = true;
			_vm->_dialogs->show(50122);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) {
		if (_scene->_customDest.x < 287) {
			if (!_globals[kChristineIsInBoat]) {
				switch (_game._trigger) {
				case 0:
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
					break;

				case 1: {
					int idx = _globals._sequenceIndexes[3];
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
					_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_SEQ, idx);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.addTimer(15, 2);
					_vm->_sound->command(74);
					}
					break;

				case 2:
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
					break;

				case 3:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[3]);
					_game._player._visible = true;
					_vm->_dialogs->show(50120);
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			} else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 4, 65);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 67);
					break;

				case 65:
					_scene->deleteSequence(_globals._sequenceIndexes[2]);
					_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
					_vm->_sound->command(24);
					break;

				case 66:
					_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -2);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
					_game._player.walk(Common::Point(305, 112), FACING_EAST);
					_game._player.setWalkTrigger(68);
					break;

				case 67:
					_game._player._visible = true;
					break;

				case 68:
					_vm->_gameConv->stop();
					_scene->_nextSceneId = 506;
					break;

				default:
					break;
				}
			}
		} else {
			if (!_globals[kChristineIsInBoat]) {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 4, 65);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 67);
					break;

				case 65: {
					int idx = _globals._sequenceIndexes[1];
					_scene->deleteSequence(_globals._sequenceIndexes[1]);
					_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
					_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_SEQ, idx);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
					_vm->_sound->command(24);
					}
					break;

				case 66:
					_game._player.walk(Common::Point(319, 116), FACING_NORTHWEST);
					_game._player.setWalkTrigger(68);
					break;

				case 67:
					_game._player._visible = true;
					break;

				case 68:
					_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 69);
					_vm->_sound->command(25);
					break;

				case 69:
					_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 5);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
					_scene->_nextSceneId = 502;
					break;

				default:
					break;
				}
			} else {
				switch (_game._trigger) {
				case (0):
					_game._player._stepEnabled = false;
					_game._player._visible = false;
					_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
					break;

				case 1: {
					int idx = _globals._sequenceIndexes[3];
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
					_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_SEQ, idx);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.addTimer(15, 2);
					_vm->_sound->command(73);
						}
						break;

				case 2:
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
					break;

				case 3:
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[3]);
					_game._player._visible = true;
					_vm->_dialogs->show(50120);
					_game._player._stepEnabled = true;
					break;

				default:
					break;
				}
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(50110);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(50111);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(50112);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(50113);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LAKE)) {
			_vm->_dialogs->show(50114);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOAT)) {
			_vm->_dialogs->show(50126);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TORCH)) {
			_vm->_dialogs->show(50117);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY_TO_WEST)) {
			_vm->_dialogs->show(50118);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			if (_scene->_customDest.x < 287) {
				if (_game._visitedScenes.exists(506))
					_vm->_dialogs->show(50127);
				else
					_vm->_dialogs->show(50119);
			} else {
				if (_game._visitedScenes.exists(506))
					_vm->_dialogs->show(50128);
				else
					_vm->_dialogs->show(50119);
			}
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COLUMN)) {
			_vm->_dialogs->show(50121);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHRISTINE)) {
			_vm->_dialogs->show(50124);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_TORCH)) {
		_vm->_dialogs->show(50125);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHRISTINE)) {
		_vm->_dialogs->show(50129);
		_action._inProgress = false;
	}
}

void Scene501::preActions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST)) {
		if (_globals[kLanternStatus] == 0) {
			_game._player._needToWalk = false;
			_vm->_dialogs->show(30918);
			_game._player.cancelCommand();
		} else if (!_globals[kChristineIsInBoat])
			_game.enterCatacombs(0);
	}

	if ((_action.isObject(NOUN_DOOR)) && (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_OPEN))) {
		if (_scene->_customDest.x < 287)
			_game._player.walk(Common::Point(266, 112), FACING_EAST);
		else
			_game._player.walk(Common::Point(287, 118), FACING_EAST);
	}

	if ((_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) && _game._visitedScenes.exists(506) && _scene->_customDest.x < 287) {
		switch (_game._trigger) {
		case 0:
			_game._player._readyToWalk = false;
			_game._player._needToWalk = false;
			_game._player._stepEnabled = false;
			_vm->_gameConv->run(26);
			_vm->_gameConv->exportValue(2);
			_scene->_sequences.addTimer(6, 1);
			break;

		case 1:
			if (_vm->_gameConv->activeConvId() >= 0)
				_scene->_sequences.addTimer(6, 1);
			else {
				_game._player._stepEnabled = true;
				_action._inProgress = true;
				_game._player._needToWalk = true;
				_game._player._readyToWalk = true;
			}
			break;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY_TO_WEST) && _globals[kLanternStatus] && _globals[kRightDoorIsOpen504])
		_game._player.walk(Common::Point(24, 110), FACING_WEST);
}

/*------------------------------------------------------------------------*/

Scene502::Scene502(MADSEngine *vm) : Scene5xx(vm) {
	_fire1ActiveFl = false;
	_fire2ActiveFl = false;
	_fire3ActiveFl = false;
	_fire4ActiveFl = false;
	_panelTurningFl = false;
	_trapDoorHotspotEnabled = false;
	_acceleratedFireActivationFl = false;

	for (int i = 0; i < 16; i++) {
		_puzzlePictures[i] = -1;
		_puzzleSprites[i] = -1;
		_puzzleSequences[i] = -1;
	}

	_panelPushedNum = -1;
	_messageLevel = -1;
	_cycleStage = -1;

	_nextPos = Common::Point(-1, -1);

	_lastFrameTime = 0;
	_timer = 0;
	_deathTimer = 0;

	_cyclePointer = nullptr;
}

Scene502::~Scene502() {
	if (_cyclePointer)
		delete(_cyclePointer);
}

void Scene502::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_fire1ActiveFl);
	s.syncAsByte(_fire2ActiveFl);
	s.syncAsByte(_fire3ActiveFl);
	s.syncAsByte(_fire4ActiveFl);
	s.syncAsByte(_panelTurningFl);
	s.syncAsByte(_trapDoorHotspotEnabled);
	s.syncAsByte(_acceleratedFireActivationFl);

	for (int i = 0; i < 16; i++) {
		s.syncAsSint16LE(_puzzlePictures[i]);
		s.syncAsSint16LE(_puzzleSprites[i]);
		s.syncAsSint16LE(_puzzleSequences[i]);
	}

	s.syncAsSint16LE(_panelPushedNum);
	s.syncAsSint16LE(_messageLevel);
	s.syncAsSint16LE(_cycleStage);

	s.syncAsSint16LE(_nextPos.x);
	s.syncAsSint16LE(_nextPos.y);

	s.syncAsUint32LE(_lastFrameTime);
	s.syncAsUint32LE(_timer);
	s.syncAsUint32LE(_deathTimer);

	warning("more syncing required");
}

void Scene502::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene502::enter() {
	loadCyclingInfo();
	_scene->loadSpeech(7);

	_panelPushedNum = -1;
	_panelTurningFl = false;
	_fire1ActiveFl = false;
	_fire2ActiveFl = false;
	_fire3ActiveFl = false;
	_fire4ActiveFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_lastFrameTime = _scene->_frameStartTime;
		_cycleStage = 0;
		_timer = 0;
		_deathTimer = 0;
		_messageLevel = 1;
		_acceleratedFireActivationFl = true;
		_trapDoorHotspotEnabled = false;
	}

	_scene->_hotspots.activate(NOUN_ROPE, false);
	_scene->_hotspots.activateAtPos(NOUN_TRAP_DOOR, false, Common::Point(225, 28));

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 3));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('x', 6));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('j', 0));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('k', 0));
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('l', 0));
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('m', 0));
	_globals._spriteIndexes[16] = _scene->_sprites.addSprites(formAnimName('h', 0));

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
	} else
		_scene->drawToBackground(_globals._spriteIndexes[5], -2, Common::Point(-32000, -32000), 0, 100);

	if ((_scene->_priorSceneId == 501) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		if (!_game._visitedScenes._sceneRevisited) {
			if (_game._objects.isInInventory(OBJ_ROPE))
				_globals[kCableHookWasSeparate] = true;
			else
				_globals[kCableHookWasSeparate] = false;
		} else if (_globals[kCableHookWasSeparate]) {
			_game._objects.addToInventory(OBJ_ROPE);
			_game._objects.addToInventory(OBJ_CABLE_HOOK);
			_game._objects.setRoom(OBJ_ROPE_WITH_HOOK, NOWHERE);
		} else {
			_game._objects.setRoom(OBJ_ROPE, NOWHERE);
			_game._objects.setRoom(OBJ_CABLE_HOOK, NOWHERE);
			_game._objects.addToInventory(OBJ_ROPE_WITH_HOOK);
		}

		_game._player._playerPos = Common::Point(43, 154);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(87, 153), FACING_EAST);
		_game._player.setWalkTrigger(77);
	}

	room_502_initialize_panels();

	if (_trapDoorHotspotEnabled) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
		_scene->_hotspots.activate(NOUN_TRAP_DOOR, false);
		_scene->_hotspots.activateAtPos(NOUN_TRAP_DOOR, true, Common::Point(225, 28));
		if (!_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK) && !_game._objects.isInInventory(OBJ_CABLE_HOOK)) {
			_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 12);
			_scene->_hotspots.activate(NOUN_ROPE, true);
		}
	} else {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
	}

	sceneEntrySound();
}

void Scene502::step() {
	if (_acceleratedFireActivationFl) {
		int32 diff = _scene->_frameStartTime - _lastFrameTime;
		if ((diff >= 0) && (diff <= 4)) {
			_timer += diff;
			_deathTimer += diff;
		} else {
			_timer += 1;
			_deathTimer += 1;
		}
		_lastFrameTime = _scene->_frameStartTime;

		if (_timer >= 300) {
			_timer = 0;
			if (_cycleStage < 8)
				++_cycleStage;
		}
	}

	if ((_deathTimer >= 7200) && !_panelTurningFl) {
		_vm->_dialogs->show(50215);
		_game._player.walk(Common::Point(160, 148), FACING_NORTH);
		_game._player.setWalkTrigger(71);
		_game._player._stepEnabled = false;
		_panelTurningFl = true;
		_deathTimer = 0;
	}

	if ((_deathTimer > 900) && (_messageLevel == 1) && !_panelTurningFl) {
		_messageLevel = 2;
		_vm->_dialogs->show(50212);
	}

	if ((_deathTimer > 3600) && (_messageLevel == 2) && !_panelTurningFl) {
		_messageLevel = 3;
		_vm->_dialogs->show(50213);
	}

	if ((_deathTimer > 5400) && (_messageLevel == 3) && !_panelTurningFl) {
		_messageLevel = 4;
		_vm->_dialogs->show(50214);
	}

	switch (_game._trigger) {
	case 71:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 44, 73);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 51, 74);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 32, 75);
		break;

	case 72:
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_scene->_userInterface.noInventoryAnim();
		// CHECKME: Not sure about the next function call
		_scene->_userInterface.refresh();
		_scene->_sequences.addTimer(120, 76);
		break;

	case 73:
		_vm->_sound->command(1);
		_vm->_sound->command(67);
		break;

	case 74:
		_vm->_sound->command(27);
		break;

	case 75:
		_scene->playSpeech(7);
		break;

	case 76:
		_scene->_reloadSceneFlag = true;
		break;

	case 77:
		_scene->deleteSequence(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 78);
		break;

	case 78:
		_vm->_dialogs->show(50211);
		_scene->drawToBackground(_globals._spriteIndexes[5], -2, Common::Point(-32000, -32000), 0, 100);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	if (!_trapDoorHotspotEnabled)
		animateFireBursts();

	setPaletteCycle();
}

void Scene502::actions() {
	if (_game._trigger >= 110) {
		handlePanelAnimation();
		_action._inProgress = false;
		return;
	}

	switch (_game._trigger) {
	case 80:
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 6);
		_scene->_hotspots.activateAtPos(NOUN_ROPE, true, Common::Point(225, 28));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
		_scene->_hotspots.activate(NOUN_TRAP_DOOR, false);
		_scene->_hotspots.activateAtPos(NOUN_TRAP_DOOR, true, Common::Point(225, 28));
		if (!_panelTurningFl)
			_vm->_dialogs->show(50216);

		_action._inProgress = false;
		return;

	case 90:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 7, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 14, 18);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_SPRITE, 18, 110);
		_action._inProgress = false;
		return;

	case 91:
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_scene->_sequences.addTimer(5, 102);
		_action._inProgress = false;
		return;

	case 95:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 7, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 8, 13);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 96);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_SPRITE, 13, 110);
		_action._inProgress = false;
		return;

	case 96:
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_scene->_sequences.addTimer(5, 102);
		_action._inProgress = false;
		return;

	case 100:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 5, 7);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 101);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_SPRITE, 7, 110);
		_action._inProgress = false;
		return;

	case 101:
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_scene->_sequences.addTimer(5, 102);
		_action._inProgress = false;
		return;

	case 102:
		_panelTurningFl = false;
		_game._player._stepEnabled = true;
		_action._inProgress = false;
		return;

	case 105:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 8, 2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[10], true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 106);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_SPRITE, 4, 110);
		_action._inProgress = false;
		return;

	case 106:
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[10]);
		_game._player._visible = true;
		_scene->_sequences.addTimer(5, 102);
		_action._inProgress = false;
		return;

	default:
		break;
	}

	if (_action.isAction(VERB_PUSH, NOUN_PANEL)) {
		if (_panelTurningFl) {
			_action._inProgress = false;
			return;
		}

		Common::Point walkToPos;
		getPanelInfo(&walkToPos, &_panelPushedNum, _scene->_customDest, &_nextPos);
		_panelTurningFl = true;

		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_scene->_sequences.addTimer(1, 90);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_scene->_sequences.addTimer(1, 95);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_scene->_sequences.addTimer(1, 100);
			break;

		default:
			_scene->_sequences.addTimer(1, 105);
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_THROW, NOUN_ROPE_WITH_HOOK, NOUN_TRAP_DOOR) || _action.isAction(VERB_GRAPPLE, NOUN_TRAP_DOOR)) {
		if (_trapDoorHotspotEnabled) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_panelTurningFl = true;
				_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 13);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 10, 83);
				_game._objects.setRoom(OBJ_ROPE_WITH_HOOK, NOWHERE);
				break;

			case 82:
				_game._player._stepEnabled = true;
				_game._player._visible = true;
				_panelTurningFl = false;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[7]);
				_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 13);
				_scene->_hotspots.activate(NOUN_ROPE, true);
				break;

			case 83:
				_vm->_sound->command(69);
				break;

			default:
				break;
			}
		} else
			_vm->_dialogs->show(50229);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)
	 && (_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK) || _game._objects.isInInventory(OBJ_CABLE_HOOK))) {
		_vm->_dialogs->show(50228);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_THROW, NOUN_ROPE, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(50226);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_THROW, NOUN_CABLE_HOOK, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(50227);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB, NOUN_ROPE) || _action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)) {
		switch (_game._trigger) {
		case 0:
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_panelTurningFl = true;
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 10);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -1, -2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
			break;

		case 82:
			_scene->_nextSceneId = 504;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(50210);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(50217);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TRAP_DOOR)) {
			_vm->_dialogs->show(_trapDoorHotspotEnabled ? 50220 : 50225);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(50219);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(50221);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PANELS)) {
			_vm->_dialogs->show(50222);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PANEL)) {
			_vm->_dialogs->show(50223);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(50224);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ROPE) && !_game._objects.isInInventory(OBJ_ROPE)
		 && !_game._objects.isInInventory(OBJ_CABLE_HOOK) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)) {
			_vm->_dialogs->show(50233);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(_trapDoorHotspotEnabled ? 50230 : 50228);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(_trapDoorHotspotEnabled ? 50228 : 50231);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_ROPE) && !_game._objects.isInInventory(OBJ_ROPE)
	 && !_game._objects.isInInventory(OBJ_CABLE_HOOK) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)) {
		_vm->_dialogs->show(50234);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LASSO, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(50232);
		_action._inProgress = false;
		return;
	}
}

void Scene502::preActions() {
	int panel;

	if (_action.isAction(VERB_PUSH, NOUN_PANEL)) {
		Common::Point walkToPos;
		Common::Point tmpPos;
		getPanelInfo(&walkToPos, &panel, _scene->_customDest, &tmpPos);
		_game._player.walk(walkToPos, FACING_NORTH);
	}

	if (_trapDoorHotspotEnabled && (_action.isAction(VERB_CLIMB, NOUN_ROPE) || _action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)))
		_game._player.walk(Common::Point(211, 149), FACING_NORTH);

	if (_trapDoorHotspotEnabled && (_action.isAction(VERB_THROW, NOUN_ROPE_WITH_HOOK, NOUN_TRAP_DOOR) || _action.isAction(VERB_GRAPPLE, NOUN_TRAP_DOOR)))
		_game._player.walk(Common::Point(200, 149), FACING_NORTH);
}

void Scene502::room_502_initialize_panels() {
	for (int i = 0, curPuzzleSprite = 2, count = 1; i < 16; i++) {
		if (_scene->_priorSceneId != RETURNING_FROM_LOADING)
			_puzzlePictures[i] = _vm->getRandomNumber(1, 4);

		curPuzzleSprite += (_puzzlePictures[i] * 3) - 3;
		_puzzleSprites[i] = curPuzzleSprite;

		int sprIdx;

		switch (i) {
		case 0:
		case 1:
		case 2:
		case 3:
			sprIdx = _globals._spriteIndexes[11];
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			sprIdx = _globals._spriteIndexes[12];
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			sprIdx = _globals._spriteIndexes[13];
			break;

		default:
			sprIdx = _globals._spriteIndexes[14];
			break;
		}

		_globals._sequenceIndexes[15] = _scene->_sequences.addStampCycle(sprIdx, false, curPuzzleSprite);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[15], 14);
		_puzzleSequences[i] = _globals._sequenceIndexes[15];

		++count;
		if (count >= 5)
			count = 1;

		switch (count) {
		case 1:
			curPuzzleSprite = 2;
			break;

		case 2:
			curPuzzleSprite = 14;
			break;

		case 3:
			curPuzzleSprite = 26;
			break;

		case 4:
			curPuzzleSprite = 38;
			break;

		default:
			break;
		}
	}
}

void Scene502::loadCyclingInfo() {
	warning("TODO: loadCyclingInfo");
}

void Scene502::animateFireBursts() {
	int rndTrigger;

	if (_acceleratedFireActivationFl)
		rndTrigger = _vm->getRandomNumber(1, 50);
	else
		rndTrigger = _vm->getRandomNumber(1, 400);

	if (rndTrigger == 1) {
		rndTrigger = _vm->getRandomNumber(1, 4);

		switch (rndTrigger) {
		case 1:
			if (!_fire1ActiveFl) {
				_scene->_sequences.addTimer(_vm->getRandomNumber(300, 600), 60);
				_fire1ActiveFl = true;
			}
			break;

		case 2:
			if (!_fire2ActiveFl) {
				_scene->_sequences.addTimer(_vm->getRandomNumber(300, 600), 63);
				_fire2ActiveFl = true;
			}
			break;

		case 3:
			if (!_fire3ActiveFl) {
				_scene->_sequences.addTimer(_vm->getRandomNumber(300, 600), 66);
				_fire3ActiveFl = true;
			}
			break;

		case 4:
			if (!_fire4ActiveFl) {
				_scene->_sequences.addTimer(_vm->getRandomNumber(300, 600), 69);
				_fire4ActiveFl = true;
			}
			break;

		default:
			break;
		}
	}

	switch (_game._trigger) {
	case 60:
		if ((_game._player._playerPos.x < 198) || (_game._player._playerPos.y > 150)) {
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		}
		break;

	case 61:
		_fire1ActiveFl = false;
		break;

	case 63:
		if ((_game._player._playerPos.x > 127) || (_game._player._playerPos.y < 150)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 64);
		}
		break;

	case 64:
		_fire2ActiveFl = false;
		break;

	case 66:
		if (_game._player._playerPos.x < 198) {
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 67);
		}
		break;

	case 67:
		_fire3ActiveFl = false;
		break;

	case 69:
		if ((_game._player._playerPos.x > 110) || (_game._player._playerPos.y > 150)) {
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		}
		break;

	case 70:
		_fire4ActiveFl = false;
		break;

	default:
		break;
	}
}

void Scene502::setPaletteCycle() {
	warning("TODO: setPaletteCycle");
}

void Scene502::getPanelInfo(Common::Point *walkToPos, int *panel, Common::Point mousePos, Common::Point *interimPos) {
	walkToPos->y = 148;

	if ((mousePos.x < 120) || (mousePos.y < 75) || (mousePos.y > 137))
		return;

	if (mousePos.x <= 139) {
		interimPos->x = 129;
		if (mousePos.y <= 90) {
			*panel = 0;
			interimPos->y = 90;
			walkToPos->x = 107;
		} else if (mousePos.y <= 106) {
			*panel = 4;
			interimPos->y = 106;
			walkToPos->x = 107;
		} else if (mousePos.y <= 122) {
			*panel = 8;
			interimPos->y = 122;
			walkToPos->x = 107;
		} else {
			*panel = 12;
			interimPos->y = 138;
			walkToPos->x = 107;
		}
	} else if (mousePos.x <= 159) {
		interimPos->x = 149;
		if (mousePos.y <= 90) {
			*panel = 1;
			interimPos->y = 90;
			walkToPos->x = 127;
		} else if (mousePos.y <= 106) {
			*panel = 5;
			interimPos->y = 106;
			walkToPos->x = 127;
		} else if (mousePos.y <= 122) {
			*panel = 9;
			interimPos->y = 122;
			walkToPos->x = 127;
		} else {
			*panel = 13;
			interimPos->y = 138;
			walkToPos->x = 127;
		}
	} else if (mousePos.x <= 179) {
		interimPos->x = 169;
		if (mousePos.y <= 90) {
			*panel = 2;
			interimPos->y = 90;
			walkToPos->x = 147;
		} else if (mousePos.y <= 106) {
			*panel = 6;
			interimPos->y = 106;
			walkToPos->x = 147;
		} else if (mousePos.y <= 122) {
			*panel = 10;
			interimPos->y = 122;
			walkToPos->x = 147;
		} else {
			*panel = 14;
			interimPos->y = 138;
			walkToPos->x = 147;
		}
	} else if (mousePos.x <= 199) {
		interimPos->x = 189;
		if (mousePos.y <= 90) {
			*panel = 3;
			interimPos->y = 90;
			walkToPos->x = 167;
		} else if (mousePos.y <= 106) {
			*panel = 7;
			interimPos->y = 106;
			walkToPos->x = 167;
		} else if (mousePos.y <= 122) {
			*panel = 11;
			interimPos->y = 122;
			walkToPos->x = 167;
		} else {
			*panel = 15;
			interimPos->y = 138;
			walkToPos->x = 167;
		}
	}
}

void Scene502::handlePanelAnimation() {
	switch (_game._trigger) {
	case 110:
		_vm->_sound->command(65);
		_scene->deleteSequence(_puzzleSequences[_panelPushedNum]);
		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);
			_scene->_sequences.addTimer(5, 111);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);
			_scene->_sequences.addTimer(5, 111);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);
			_scene->_sequences.addTimer(5, 111);
			break;

		default:
			_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, _puzzleSprites[_panelPushedNum] - 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 14);
			_scene->_sequences.addTimer(5, 111);
			break;
		}
		break;

	case 111:
		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_scene->deleteSequence(_globals._sequenceIndexes[11]);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_scene->deleteSequence(_globals._sequenceIndexes[12]);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_scene->deleteSequence(_globals._sequenceIndexes[13]);
			break;

		default:
			_scene->deleteSequence(_globals._sequenceIndexes[14]);
			break;
		}

		_globals._sequenceIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[16], false, 5, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[16], 14);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[16], _nextPos);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[16], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[16], SEQUENCE_TRIGGER_EXPIRE, 0, 112);
		break;

	case 112: {
		int idx = _globals._sequenceIndexes[16];
		int newSprId = _puzzleSprites[_panelPushedNum] + 4;

		switch (_panelPushedNum) {
		case 0:
		case 4:
		case 8:
		case 12:
			if (newSprId > 12)
				newSprId = 3;
			break;

		case 1:
		case 5:
		case 9:
		case 13:
			if (newSprId > 24)
				newSprId = 15;
			break;

		case 2:
		case 6:
		case 10:
		case 14:
			if (newSprId > 36)
				newSprId = 27;
			break;

		default:
			if (newSprId > 48)
				newSprId = 39;
			break;
		}

		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[11], SYNC_SEQ, idx);
			_scene->_sequences.addTimer(5, 113);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[12], SYNC_SEQ, idx);
			_scene->_sequences.addTimer(5, 113);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[13], SYNC_SEQ, idx);
			_scene->_sequences.addTimer(5, 113);
			break;

		default:
			_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 14);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[14], SYNC_SEQ, idx);
			_scene->_sequences.addTimer(5, 113);
			break;
		}
		}
		break;

	case 113: {
		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_scene->deleteSequence(_globals._sequenceIndexes[11]);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_scene->deleteSequence(_globals._sequenceIndexes[12]);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_scene->deleteSequence(_globals._sequenceIndexes[13]);
			break;

		default:
			_scene->deleteSequence(_globals._sequenceIndexes[14]);
			break;
		}

		int newSprId = _puzzleSprites[_panelPushedNum] + 3;

		switch (_panelPushedNum) {
		case 0:
		case 4:
		case 8:
		case 12:
			if (newSprId > 12)
				newSprId = 2;
			break;

		case 1:
		case 5:
		case 9:
		case 13:
			if (newSprId > 24)
				newSprId = 14;
			break;

		case 2:
		case 6:
		case 10:
		case 14:
			if (newSprId > 36)
				newSprId = 26;
			break;

		default:
			if (newSprId > 48)
				newSprId = 38;
			break;
		}
		_puzzleSprites[_panelPushedNum] = newSprId;
		++_puzzlePictures[_panelPushedNum];
		if (_puzzlePictures[_panelPushedNum] >= 5)
			_puzzlePictures[_panelPushedNum] = 1;

		switch (_panelPushedNum) {
		case 0:
		case 1:
		case 2:
		case 3:
			_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[11];
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[12];
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[13];
			break;

		default:
			_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, newSprId);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 14);
			_puzzleSequences[_panelPushedNum] = _globals._sequenceIndexes[14];
			break;
		}

		int puzzleSolvedFl = true;
		for (int i = 0; i < 16; i++) {
			if (_puzzlePictures[i] != 1)
				puzzleSolvedFl = false;
		}

		if (puzzleSolvedFl && !_trapDoorHotspotEnabled) {
			_trapDoorHotspotEnabled = true;
			_scene->deleteSequence(_globals._sequenceIndexes[6]);
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('t', 1), 80);
		}
		}
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

Scene504::Scene504(MADSEngine *vm) : Scene5xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;
	_anim4ActvFl = false;
	_anim5ActvFl = false;
	_playingMusicFl = false;
	_chairDialogDoneFl = false;
	_fireBreathFl = false;

	_songNum = -1;
	_input3Count = -1;
	_playCount = -1;
	_listenStatus = -1;
	_listenFrame = -1;
	_chairStatus = -1;
	_chairFrame = -1;
	_playStatus = -1;
	_playFrame = -1;
	_phantomStatus = -1;
	_phantomFrame = -1;
	_christineTalkCount = -1;
	_deathCounter = -1;
}

void Scene504::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_anim3ActvFl);
	s.syncAsByte(_anim4ActvFl);
	s.syncAsByte(_anim5ActvFl);
	s.syncAsByte(_playingMusicFl);
	s.syncAsByte(_chairDialogDoneFl);
	s.syncAsByte(_fireBreathFl);

	s.syncAsSint16LE(_songNum);
	s.syncAsSint16LE(_input3Count);
	s.syncAsSint16LE(_playCount);
	s.syncAsSint16LE(_listenStatus);
	s.syncAsSint16LE(_listenFrame);
	s.syncAsSint16LE(_chairStatus);
	s.syncAsSint16LE(_chairFrame);
	s.syncAsSint16LE(_playStatus);
	s.syncAsSint16LE(_playFrame);
	s.syncAsSint16LE(_phantomStatus);
	s.syncAsSint16LE(_phantomFrame);
	s.syncAsSint16LE(_christineTalkCount);
	s.syncAsSint16LE(_deathCounter);
}

void Scene504::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_PHANTOM);
	_scene->addActiveVocab(NOUN_CHRISTINE);
}

void Scene504::enter() {
	_vm->_disableFastwalk = true;

	_input3Count = 0;
	_deathCounter = 0;
	_anim2ActvFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_playCount = 0;
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim3ActvFl = false;
		_anim4ActvFl = false;
		_anim5ActvFl = false;
		_playingMusicFl = false;
		_fireBreathFl = false;
		_chairDialogDoneFl = false;
		_songNum = 0;
		_phantomStatus = 0;
	}

	_scene->_hotspots.activate(NOUN_CHRISTINE, false);

	if (!_globals[kRightDoorIsOpen504]) {
		_vm->_gameConv->load(19);
		_vm->_gameConv->load(27);
	} else
		_vm->_gameConv->load(21);

	_vm->_gameConv->load(26);

	_globals._spriteIndexes[14] = _scene->_sprites.addSprites("*RDR_9");
	_globals._spriteIndexes[15] = _scene->_sprites.addSprites(formAnimName('x', 8));

	if (!_game._objects.isInRoom(OBJ_MUSIC_SCORE)) {
		_globals._sequenceIndexes[15] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[15], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[15], 14);
		_scene->_hotspots.activate(NOUN_MUSIC_SCORE, false);
	}

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('x', 7), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

	if (_globals[kFightStatus] == 0)
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));

	if ((_scene->_priorSceneId == 505) || ((_scene->_priorSceneId == 504) && _globals[kRightDoorIsOpen504])) {
		if ((_globals[kFightStatus] == 0) && (_globals[kCoffinStatus] == 2)) {
			_scene->_hotspots.activate(NOUN_CHRISTINE, true);
			_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);

			_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('x', 7), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 0));

			_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_6", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACERAL", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEXDFR", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEPHN", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

			_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 5));
			_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 3), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 6), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 7), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('a', 0), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

			_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('p', 1), 0);
			int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_PHANTOM, VERB_LOOK_AT, SYNTAX_MASC_NOT_PROPER, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(-2, -2), FACING_NONE);

			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 4);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 8);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 13);

			_phantomStatus = 0;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_anim3ActvFl = true;
		} else {
			if (_globals[kFightStatus])
				_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);

			if (_globals[kCoffinStatus] != 2) {
				_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
			}

			_game._player._playerPos = Common::Point(317, 115);
			_game._player._facing = FACING_SOUTHWEST;
			_game._player.walk(Common::Point(279, 121), FACING_SOUTHWEST);

			_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);

			if (!_game._visitedScenes.exists(506) && (_globals[kCoffinStatus] == 2)) {
				_scene->changeVariant(1);

				_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
				_anim5ActvFl = true;

				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_2");
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACERAL", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEXDFR",PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEPHN", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

				_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('p', 3), 0);
				int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
				_phantomStatus = 5;
				_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
				_scene->setAnimFrame(_globals._animationIndexes[3], 79);
				_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(66, 119), FACING_NORTHWEST);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 0);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 1);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 2);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 3);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 4);
			}
		}
	} else if (_scene->_priorSceneId == 506) {
		_game._player._playerPos = Common::Point(0, 109);
		_game._player._facing = FACING_SOUTHEAST;
		_game._player.walk(Common::Point(39, 118), FACING_SOUTHEAST);
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
	} else if (_scene->_priorSceneId == 504) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);

		_game._player._playerPos = Common::Point(147, 131);
		_game._player._facing = FACING_EAST;
	} else if ((_scene->_priorSceneId == 502) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('t', 1), 60);
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._player._playerPos = Common::Point(147, 131);
		_game._player._facing = FACING_EAST;
	}

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (!_globals[kRightDoorIsOpen504]) {
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		}

		if (_vm->_gameConv->restoreRunning() == 19) {
			_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('l', 1), 67);
			_anim0ActvFl = true;
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_game._player._playerPos = Common::Point(286, 120);
			_game._player._facing = FACING_EAST;
			_listenStatus = 0;
			_scene->setAnimFrame(_globals._animationIndexes[0], 8);
			_vm->_gameConv->run(19);
			_vm->_gameConv->exportValue(_game._difficulty);
		} else if (_vm->_gameConv->restoreRunning() == 27) {
			_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

			_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('o', 1), 100);

			_scene->setAnimFrame(_globals._animationIndexes[1], 22);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_playingMusicFl = false;
			_anim1ActvFl = true;
			_playStatus = 0;
			_vm->_gameConv->run(27);
		} else if ((_globals[kFightStatus] <= 1) && (_globals[kCoffinStatus] == 2)) {
			if ((_phantomStatus == 1) || (_phantomStatus == 2)) {
				_scene->_hotspots.activate(NOUN_CHRISTINE, true);
				_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);

				_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 0));
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_6", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACERAL", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEXDFR", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEPHN", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

				_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 5));
				_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 3), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 6), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 7), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('a', 0), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

				_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('p', 1), 0);
				int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_PHANTOM, VERB_LOOK_AT, SYNTAX_MASC_NOT_PROPER, EXT_NONE, Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(-2, -2), FACING_NONE);

				if (_phantomStatus == 1)
					_scene->setAnimFrame(_globals._animationIndexes[3], _vm->getRandomNumber(109, 112));
				else if (_phantomStatus == 2) {
					_scene->setAnimFrame(_globals._animationIndexes[3], _vm->getRandomNumber(148, 150));
					_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
				}

				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 4);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 8);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 13);

				_game._player._visible = false;
				_anim3ActvFl = true;

				if (_vm->_gameConv->restoreRunning() == 21) {
					_game._player._stepEnabled = false;
					_vm->_gameConv->run(21);
					_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_MUSIC_SCORE));
				}
			} else if (_phantomStatus == 4) {
				_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
				_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);

				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_3");
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACERAL", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEXDFR", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEPHN", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('a', 8), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

				_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('p', 2), 130);
				_scene->setAnimFrame(_globals._animationIndexes[3], 159);

				_game._player._playerPos = Common::Point(130, 135);
				_game._player._facing = FACING_NORTHEAST;
				_game._player._visible = true;
				_anim4ActvFl = true;

				_game._player._stepEnabled = false;
				_vm->_gameConv->run(21);
				_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_MUSIC_SCORE));
			}
		} else if (_globals[kFightStatus] == 2) {
			if (!_game._visitedScenes.exists(506)) {
				_scene->changeVariant(1);

				_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
				_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
				_anim5ActvFl = true;

				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_3");
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACERAL", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEXDFR",PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
				_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*FACEPHN", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

				_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('p', 3), 0);
				int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
				_phantomStatus = 5;
				_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
				_scene->setAnimFrame(_globals._animationIndexes[3], 79);
				_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(66, 119), FACING_NORTHWEST);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 0);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 1);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 2);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 3);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 4);
			} else {
				_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
				_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
			}
		} else {
			_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
			if (!_globals[kHeListened])
				_scene->_sequences.addTimer(30, 62);
		}
	}

	sceneEntrySound();
}

void Scene504::step() {
	if (_anim0ActvFl)
		handleListenAnimation();

	if (_anim1ActvFl)
		handleOrganAnimation();

	if (_anim2ActvFl)
		handleChairAnimation();

	if (_anim3ActvFl)
		handlePhantomAnimation1();

	if (_anim4ActvFl)
		handlePhantomAnimation2();

	if (_anim5ActvFl)
		handlePhantomAnimation3();

	if (_game._trigger == 120) {
		_game._player._stepEnabled = false;
		_vm->_gameConv->run(21);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_MUSIC_SCORE));
		_vm->_gameConv->exportValue(1);
		_globals[kFightStatus] = 1;
	}

	if (_game._trigger == 60) {
		_game._player._visible = true;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_sequences.addTimer(30, 61);
	}

	if (_game._trigger == 61) {
		_game._player._stepEnabled = true;
		_scene->_sequences.addTimer(60, 62);
	}

	if (_game._trigger == 62) {
		_globals[kHeListened] = true;
		_game._player._stepEnabled = false;
		_vm->_gameConv->run(19);
		_vm->_gameConv->exportValue(_game._difficulty);
	}

	if (_game._trigger == 80) {
		_vm->_sound->command(73);
		_globals[kRightDoorIsOpen504] = true;
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);

		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
	}

	if (_game._trigger == 90) {
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
	}

	if (_game._trigger == 91) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 92);
	}

	if (_game._trigger == 92)
		_fireBreathFl = true;

	if (_fireBreathFl) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_fireBreathFl = false;
	}

	if (_game._trigger == 130) {
		_scene->freeAnimation(_globals._animationIndexes[3]);
		_scene->_sprites.remove(_globals._spriteIndexes[12]);

		_anim4ActvFl = false;
		_anim5ActvFl = true;

		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_2", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_3", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

		_scene->changeVariant(1);

		_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('p', 3), 0);
		int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(66, 119), FACING_NORTHWEST);
		_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 0);
		_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 1);
		_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 2);
		_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 3);
		_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[3], 4);
	}

	if (_game._trigger == 67) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_anim0ActvFl = false;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
	}

	if (_game._trigger == 136)
		_scene->_nextSceneId = 506;

	if (_game._trigger == 100) {
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		_game._player._playerPos = Common::Point(156, 114);
		_game._player._visible = true;
		_anim1ActvFl = false;
		_game._player.resetFacing(FACING_EAST);
		_scene->_sequences.addTimer(10, 101);
	}

	if (_game._trigger == 101) {
		_game._player._stepEnabled = true;
		_scene->_sprites.remove(_globals._spriteIndexes[5]);
		_scene->_sprites.remove(_globals._spriteIndexes[4]);
	}
}

void Scene504::actions() {
	if (_vm->_gameConv->activeConvId() == 26) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_SIT_IN, NOUN_LARGE_CHAIR)) {
		if (!_anim2ActvFl) {
			_chairStatus = 0;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_anim2ActvFl = true;
			_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('c', 1), 0);
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
		} else
			_vm->_dialogs->show(50436);
		_action._inProgress = false;
		return;
	}

	if (_game._trigger == 95) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('l', 1), 67);
		_listenStatus = 0;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_anim0ActvFl = true;
		_globals[kHeListened] = true;
		_vm->_gameConv->run(19);
		_vm->_gameConv->exportValue(_game._difficulty);
		_action._inProgress = false;
		return;
	}

	if (_game._trigger == 67) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_anim0ActvFl = false;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 19) {
		handleListenConversation();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 27) {
		handlePlayConversation();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 21) {
		handleFightConversation();
		_action._inProgress = false;
		return;
	}

	if (_game._trigger == 100) {
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		_game._player._playerPos = Common::Point(156, 114);
		_game._player._visible = true;
		_anim1ActvFl = false;
		_game._player.resetFacing(FACING_EAST);
		_scene->_sequences.addTimer(10, 101);
		_action._inProgress = false;
		return;
	}

	if (_game._trigger == 101) {
		_game._player._stepEnabled = true;
		_scene->_sprites.remove(_globals._spriteIndexes[5]);
		_scene->_sprites.remove(_globals._spriteIndexes[4]);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_SIT_ON, NOUN_ORGAN_BENCH)) {
		if (_globals[kRightDoorIsOpen504])
			_vm->_dialogs->show(50427);
		else {
			_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('o', 1), 100);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_playingMusicFl = false;
			_anim1ActvFl = true;
			_playStatus = 0;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_DOOR) || _action.isAction(VERB_OPEN, NOUN_RIGHT_DOOR)) {
		if (_globals[kRightDoorIsOpen504]) {
			if (_vm->_gameConv->activeConvId() == 26)
				_vm->_gameConv->stop();

			_scene->_nextSceneId = 505;
		} else
			_vm->_dialogs->show(50418);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_LEFT_DOOR) || _action.isAction(VERB_OPEN, NOUN_LEFT_DOOR)) {
		if (_globals[kFightStatus]) {
			if (_game._visitedScenes.exists(506))
				_scene->_nextSceneId = 506;
			else if (!_game._objects.isInInventory(OBJ_MUSIC_SCORE))
				_vm->_dialogs->show(50425);
			else {
				_phantomStatus = 6;
				_game._player._stepEnabled = false;
			}
		} else
			_vm->_dialogs->show(50418);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_MUSIC_SCORE)) {
		switch (_game._trigger) {
		case (0):
			if (_game._objects.isInRoom(OBJ_MUSIC_SCORE)) {
				_globals[kPlayerScore] += 5;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[14] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[14], false, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[14], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[14], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[14], SEQUENCE_TRIGGER_SPRITE, 4, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[14], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				_action._inProgress = false;
				return;
			}
			break;

		case 1:
			_globals._sequenceIndexes[15] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[15], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[15], 14);
			_scene->_hotspots.activate(NOUN_MUSIC_SCORE, false);
			_game._objects.addToInventory(OBJ_MUSIC_SCORE);
			_vm->_sound->command(26);
			_action._inProgress = false;
			return;
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[14]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			_action._inProgress = false;
			return;
			break;

		case 3:
			_vm->_dialogs->showItem(OBJ_MUSIC_SCORE, 820, 0);
			_game._player._stepEnabled = true;
			_action._inProgress = false;
			return;
			break;

		default:
			break;
		}
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(50410);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(50411);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(50412);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ORGAN)) {
			_vm->_dialogs->show(50413);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ORGAN_BENCH)) {
			_vm->_dialogs->show(50414);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MUSIC_SCORE) && _game._objects.isInRoom(OBJ_MUSIC_SCORE)) {
			_vm->_dialogs->show(50415);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEFT_DOOR)) {
			_vm->_dialogs->show(50416);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RIGHT_DOOR)) {
			_vm->_dialogs->show(_globals[kRightDoorIsOpen504] ? 50434 : 50417);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TABLE)) {
			_vm->_dialogs->show(50419);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TRAP_DOOR)) {
			_vm->_dialogs->show(50420);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LARGE_CHAIR)) {
			_vm->_dialogs->show(50422);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHRISTINE)) {
			_vm->_dialogs->show(_globals[kFightStatus] ? 50426 : 50429);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PHANTOM)) {
			_vm->_dialogs->show(50428);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_TRAP_DOOR)) {
		_vm->_dialogs->show(50421);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE) && _action.isObject(NOUN_RIGHT_DOOR) && !_globals[kRightDoorIsOpen504]) {
		_vm->_dialogs->show(50433);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE) && _action.isObject(NOUN_LEFT_DOOR) && !_globals[kFightStatus] && !_game._visitedScenes.exists(506)) {
		_vm->_dialogs->show(50433);
		_action._inProgress = false;
		return;
	}

	if (_anim3ActvFl && (_action.isAction(VERB_TAKE, NOUN_SWORD) || _action.isAction(VERB_ATTACK, NOUN_PHANTOM))) {
		_game._player._stepEnabled = false;
		_input3Count = 0;
		_phantomStatus = 4;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_CHRISTINE)) {
		_vm->_gameConv->run(21);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_MUSIC_SCORE));
		_vm->_gameConv->exportValue(0);
		_phantomStatus = 7;
		_christineTalkCount = 0;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_PHANTOM)) {
		_vm->_dialogs->show(50431);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHRISTINE)) {
		_vm->_dialogs->show(50435);
		_action._inProgress = false;
		return;
	}
}

void Scene504::preActions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_DOOR) || _action.isAction(VERB_OPEN, NOUN_RIGHT_DOOR)) {
		if (_globals[kRightDoorIsOpen504]) {
			if ((_globals[kFightStatus] == 2) && !_game._visitedScenes.exists(506)) {
				switch (_game._trigger) {
				case 0:
					_game._player.walk(Common::Point(317, 115), FACING_NORTHEAST);
					_game._player._readyToWalk = false;
					_game._player._needToWalk = false;
					_game._player._stepEnabled = false;
					_vm->_gameConv->run(26);
					_vm->_gameConv->exportValue(2);
					_scene->_sequences.addTimer(6, 1);
					break;

				case 1:
					if (_vm->_gameConv->activeConvId() >= 0)
						_scene->_sequences.addTimer(6, 1);
					else {
						_game._player._stepEnabled = true;
						_action._inProgress = true;
						_game._player._needToWalk = true;
						_game._player._readyToWalk = true;
					}
					break;

				default:
					break;
				}
			} else
				_game._player.walk(Common::Point(317, 115), FACING_NORTHEAST);
		} else if (_globals[kHeListened] || (_globals[kFightStatus] == 0))
			_game._player.walk(Common::Point(286, 120), FACING_NORTHEAST);
	}

	if (_anim2ActvFl && !_action.isAction(VERB_SIT_IN) && _game._player._needToWalk) {
		_chairStatus = 1;
		_game._player._stepEnabled = false;
		_game._player._readyToWalk = false;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_LEFT_DOOR) || _action.isAction(VERB_OPEN, NOUN_LEFT_DOOR)) {
		if ((_globals[kFightStatus] == 0) && (_globals[kCoffinStatus] == 2) && !_game._visitedScenes.exists(506)) {
			_vm->_dialogs->show(50432);
			_game._player.cancelCommand();
			return;
		} else if (_game._visitedScenes.exists(506))
			_game._player.walk(Common::Point(0, 109), FACING_NORTHWEST);
		else if (!_game._objects.isInInventory(OBJ_MUSIC_SCORE) || (_globals[kFightStatus] == 0))
			_game._player.walk(Common::Point(33, 116), FACING_NORTHWEST);
	}

	if ((_globals[kFightStatus] == 0) && (_globals[kCoffinStatus] == 2) && !_game._visitedScenes.exists(506)
	 && !_action.isAction(VERB_LOOK) && !_action.isAction(VERB_LOOK_AT) && !_action.isAction(VERB_ATTACK)
	 && !_action.isAction(VERB_TAKE, NOUN_SWORD) && !_action.isAction(VERB_TALK_TO, NOUN_PHANTOM)) {
		_vm->_dialogs->show(50430);
		_game._player.cancelCommand();
	}
}

void Scene504::handleListenAnimation() {
	int curFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	if (curFrame == _listenFrame)
		return;

	_listenFrame = curFrame;
	int resetFrame = -1;

	switch (_listenFrame) {
	case 8:
		_vm->_gameConv->release();
		break;

	case 9:
		resetFrame = (_listenStatus == 0) ? 8 : 9;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_listenFrame = resetFrame;
	}
}

void Scene504::handleOrganAnimation() {
	int curFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	if (curFrame == _playFrame)
		return;

	_playFrame = curFrame;
	int resetFrame = -1;

	switch (_playFrame) {
	case 22:
		_game._player._stepEnabled = true;
		_vm->_gameConv->run(27);
		break;

	case 23:
		if (_playStatus == 0)
			resetFrame = 22;
		else {
			_game._player._stepEnabled = false;
			if (_songNum == 5) {
				_playingMusicFl = false;
				_fireBreathFl = false;
				resetFrame = 104;
			}
		}
		break;

	case 28:
		if (!_playingMusicFl) {
			_playingMusicFl = true;
			_fireBreathFl = true;
			_game._player._stepEnabled = false;

			switch (_songNum) {
			case 1:
				_vm->_sound->command(34);
				break;

			case 2:
				_vm->_sound->command(37);
				break;

			case 3:
				_vm->_sound->command(35);
				break;

			case 4:
				_vm->_sound->command(36);
				break;

			default:
				break;
			}
		}
		break;

	case 69:
		if ((_globals[kRightDoorIsOpen504]) && (_playCount >= 2)) {
			_playCount = 0;
			resetFrame = 102;
			_vm->_sound->command(2);
			_vm->_sound->command(16);
		} else if (_songNum != _globals[kMusicSelected]) {
			_vm->_sound->command(2);
			_fireBreathFl = true;
			resetFrame = 75;
		} else {
			resetFrame = 25;
			++_playCount;
			if (!_globals[kRightDoorIsOpen504]) {
				_scene->_sequences.addTimer(1, 80);
				_globals[kPlayerScore] += 5;
			}
		}
		break;

	case 76:
		_scene->playSpeech(7);
		break;

	case 90:
		_vm->_sound->command(27);
		break;

	case 102:
		++_deathCounter;
		if (_deathCounter >= 17)
			_scene->_reloadSceneFlag = true;
		else
			resetFrame = 101;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_playFrame = resetFrame;
	}
}

void Scene504::handlePhantomAnimation1() {
	int curFrame = _scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame();
	if (curFrame == _phantomFrame)
		return;

	_phantomFrame = curFrame;
	int resetFrame = -1;

	switch (_phantomFrame) {
	case 41:
		_vm->_gameConv->run(21);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_MUSIC_SCORE));
		_vm->_gameConv->exportValue(0);
		break;

	case 52:
	case 53:
	case 54:
	case 55:
		resetFrame = (_phantomStatus == 0) ? _vm->getRandomNumber(51, 54) : 55;
		break;

	case 78:
		_vm->_gameConv->release();
		break;

	case 110:
	case 111:
	case 112:
	case 113:
		resetFrame = (_phantomStatus == 1) ? _vm->getRandomNumber(109, 112) : 113;
		break;

	case 142:
		_scene->drawToBackground(_globals._spriteIndexes[13], 1, Common::Point(-32000, -32000), 0, 100);
		break;

	case 143:
		_game._player._stepEnabled = true;
		break;

	case 149:
	case 150:
	case 151:
		++_input3Count;

		if (_phantomStatus == 4) {
			_game._player._stepEnabled = false;
			resetFrame = 200;
		} else if (_input3Count >= 9) {
			_game._player._stepEnabled = false;
			resetFrame = 151;
		} else if (_phantomStatus == 2)
			resetFrame = _vm->getRandomNumber(148, 150);
		break;

	case 169:
		_vm->_sound->command(1);
		_scene->playSpeech(7);
		break;

	case 180:
		_vm->_sound->command(27);
		break;

	case 187:
		_deathCounter = 0;
		break;

	case 189:
		++_deathCounter;
		if (_deathCounter >= 29)
			_scene->_reloadSceneFlag = true;
		else
			resetFrame = 188;
		break;

	case 227:

		_scene->freeAnimation(_globals._animationIndexes[3]);
		_scene->_sprites.remove(_globals._spriteIndexes[9]);
		_scene->_sprites.remove(_globals._spriteIndexes[8]);
		_scene->_sprites.remove(_globals._spriteIndexes[7]);
		_scene->_sprites.remove(_globals._spriteIndexes[10]);
		_scene->_sprites.remove(_globals._spriteIndexes[11]);

		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_3", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
		_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('a', 8), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
		_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('p', 2), 130);

		_scene->setAnimFrame(_globals._animationIndexes[3], 27);
		resetFrame = -1;
		_anim3ActvFl = false;
		_anim4ActvFl = true;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[3], resetFrame);
		_phantomFrame = resetFrame;
	}
}

void Scene504::handlePhantomAnimation2() {
	int curFrame = _scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame();
	if (curFrame == _phantomFrame)
		return;

	_phantomFrame = curFrame;
	int resetFrame = -1;

	switch (_phantomFrame) {
	case 78:
		_scene->playSpeech(9);
		break;

	case 119:
		_game._player._playerPos = Common::Point(114, 137);
		_game._player._facing = FACING_WEST;
		_game._player._visible = true;
		_globals[kPlayerScore] += 5;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[3]);
		_game._player.walk(Common::Point(130, 135), FACING_NORTHEAST);
		_scene->_hotspots.activate(NOUN_CHRISTINE, false);
		_game._player.setWalkTrigger(120);
		_vm->_sound->command(1);
		break;

	case 150:
		_vm->_sound->command(16);
		break;

	case 160:
		switch (_playStatus) {
		case 5:
			resetFrame = 164;
			break;

		case 7:
			resetFrame = 160;
			break;

		default:
			resetFrame = 159;
			break;
		}
		break;

	case 161:
	case 162:
	case 163:
	case 164:
		resetFrame = (_phantomStatus == 5) ? 159 : _vm->getRandomNumber(160, 162);

		++_christineTalkCount;
		if (_christineTalkCount > 10) {
			resetFrame = 159;
			if (_phantomStatus != 5)
				_phantomStatus = 4;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[3], resetFrame);
		_phantomFrame = resetFrame;
	}
}

void Scene504::handlePhantomAnimation3() {
	int curFrame = _scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame();
	if (curFrame == _phantomFrame)
		return;

	_phantomFrame = curFrame;
	int resetFrame = -1;

	switch (_phantomFrame) {
	case 58:
		_game._player._stepEnabled = true;
		break;

	case 80:
		if (_phantomStatus == 7)
			resetFrame = 116;
		else if (_phantomStatus != 6)
			resetFrame = 79;
		break;

	case 115:
		_game._player.walk(Common::Point(0, 109), FACING_NORTHWEST);
		_game._player.setWalkTrigger(136);
		break;

	case 116:
		resetFrame = 115;
		break;

	case 117:
	case 118:
	case 119:
		resetFrame = _vm->getRandomNumber(116, 118);
		++_christineTalkCount;
		if (_christineTalkCount > 10) {
			resetFrame = 79;
			if (_phantomStatus != 6)
				_phantomStatus = 5;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[3], resetFrame);
		_phantomFrame = resetFrame;
	}
}

void Scene504::handleChairAnimation() {
	int curFrame = _scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame();
	if (curFrame == _chairFrame)
		return;

	_chairFrame = curFrame;
	int resetFrame = -1;

	switch (_chairFrame) {
	case 24:
		_game._player._stepEnabled = true;
		break;

	case 25:
	case 26:
	case 30:
	case 31:
		if (!_chairDialogDoneFl) {
			_chairDialogDoneFl = true;
			_vm->_dialogs->show(50424);
		}

		if (_chairStatus == 0) {
			if (_vm->getRandomNumber(1,5) == 1)
				resetFrame = _vm->getRandomNumber(24, 30);
			else
				resetFrame = _chairFrame - 1;
		} else
			resetFrame = 31;
		break;

	case 47:
		resetFrame = -1;
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._readyToWalk = true;
		_anim2ActvFl = false;
		_chairDialogDoneFl = false;
		_scene->freeAnimation(_globals._animationIndexes[2]);
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[2], resetFrame);
		_chairFrame = resetFrame;
	}
}

void Scene504::handleListenConversation() {
	if ((_action._activeAction._verbId == 2) && !_game._trigger) {
		_game._player.walk(Common::Point(286, 120), FACING_EAST);
		_game._player.setWalkTrigger(95);
	}

	if (_action._activeAction._verbId == 12)
		_listenStatus = 1;
}

void Scene504::handlePlayConversation() {
	switch (_action._activeAction._verbId) {
	case 2:
		_vm->_gameConv->setStartNode(1);
		_vm->_gameConv->stop();
		_playStatus = 1;
		_songNum = 1;
		break;

	case 3:
		_vm->_gameConv->setStartNode(1);
		_vm->_gameConv->stop();
		_playStatus = 1;
		_songNum = 2;
		break;

	case 4:
		_vm->_gameConv->setStartNode(1);
		_vm->_gameConv->stop();
		_playStatus = 1;
		_songNum = 3;
		break;

	case 5:
		_vm->_gameConv->setStartNode(1);
		_vm->_gameConv->stop();
		_playStatus = 1;
		_songNum = 4;
		break;

	case 6:
		_vm->_gameConv->setStartNode(1);
		_vm->_gameConv->stop();
		_playStatus = 1;
		_songNum = 5;
		break;

	case 8:
		_vm->_gameConv->setStartNode(1);
		_vm->_gameConv->stop();
		_playStatus = 1;
		break;

	default:
		break;
	}
}

void Scene504::handleFightConversation() {
	switch (_action._activeAction._verbId) {
	case 3:
		_vm->_gameConv->hold();
		_phantomStatus = 1;
		break;

	case 8:
		_phantomStatus = 2;
		break;

	case 10:
	case 11:
	case 12:
	case 15:
		_vm->_gameConv->setInterlocutorTrigger(145);
		break;

	case 14:
	case 17:
		_phantomStatus = 5;
		_globals[kFightStatus] = 2;
		break;

	default:
		break;
	}

	if ((_game._trigger == 145) && (_phantomStatus != 5)) {
		_phantomStatus = 7;
		_christineTalkCount = 0;
	}
}

/*------------------------------------------------------------------------*/

Scene505::Scene505(MADSEngine *vm) : Scene5xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_checkFrame106 = false;
	_leaveRoomFl = false;
	_partedFl = false;

	_raoulStatus = -1;
	_raoulFrame = -1;
	_raoulCount = -1;
	_bothStatus = -1;
	_bothFrame = -1;
	_bothCount = -1;
	_partStatus = -1;
	_partFrame = -1;
	_partCount = -1;
}

void Scene505::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_checkFrame106);
	s.syncAsByte(_leaveRoomFl);
	s.syncAsByte(_partedFl);

	s.syncAsSint16LE(_raoulStatus);
	s.syncAsSint16LE(_raoulFrame);
	s.syncAsSint16LE(_raoulCount);
	s.syncAsSint16LE(_bothStatus);
	s.syncAsSint16LE(_bothFrame);
	s.syncAsSint16LE(_bothCount);
	s.syncAsSint16LE(_partStatus);
	s.syncAsSint16LE(_partFrame);
	s.syncAsSint16LE(_partCount);
}

void Scene505::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if ((_globals[kCoffinStatus] == 2) && (!_globals[kChrisLeft505]))
		_scene->_variant = 1;

	_scene->addActiveVocab(NOUN_CHRISTINE);
}

void Scene505::enter() {
	_vm->_disableFastwalk = true;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_partedFl = false;
		_leaveRoomFl = false;
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim2ActvFl = false;
		_checkFrame106 = false;
	}

	_vm->_gameConv->load(20);
	_scene->_hotspots.activateAtPos(NOUN_LID, false, Common::Point(216, 44));
	_scene->_hotspots.activate(NOUN_CHRISTINE, false);

	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('x', 6));
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 4));

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_vm->_gameConv->restoreRunning() == 20) {
			_scene->_hotspots.activate(NOUN_LID, false);
			_scene->_hotspots.activateAtPos(NOUN_LID, true, Common::Point(216, 44));
			_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, 12);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);

			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('c', 1), 65);
			_scene->setAnimFrame(_globals._animationIndexes[1], 109);
			_anim1ActvFl = true;
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_bothStatus = 3;

			_vm->_gameConv->run(20);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		} else if (_partedFl && (_globals[kFightStatus] == 0)) {
			_scene->_hotspots.activate(NOUN_LID, false);
			_scene->_hotspots.activateAtPos(NOUN_LID, true, Common::Point(216, 44));
			_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, 12);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);

			_anim2ActvFl = true;
			_bothStatus = 3;
			_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('b', 1), 0);
			int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(91, 108), FACING_NORTHWEST);
			_scene->setAnimFrame(_globals._animationIndexes[2], 89);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[2], 3);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[2], 4);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[2], 5);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[2], 6);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[2], 7);
		} else if (_globals[kFightStatus]) {
			_scene->_hotspots.activate(NOUN_LID, false);
			_scene->_hotspots.activateAtPos(NOUN_LID, true, Common::Point(216, 44));
			_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, 12);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
		} else {
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('c', 1), 65);
			_anim1ActvFl = true;
			_bothStatus = 0;
			_scene->_hotspots.activate(NOUN_CHRISTINE, true);
		}
	}

	if ((_scene->_priorSceneId == 504) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(5, 87);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(58, 104), FACING_SOUTHEAST);
		if (_globals[kCoffinStatus] != 2) {
			_game._player.setWalkTrigger(70);
			_anim1ActvFl = true;
			_bothStatus = 0;
			_scene->_hotspots.activate(NOUN_CHRISTINE, true);
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('c', 1), 65);
		} else {
			_scene->_hotspots.activate(NOUN_LID, false);
			_scene->_hotspots.activateAtPos(NOUN_LID, true, Common::Point(216, 44));
			_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, 12);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
			_game._player._stepEnabled = true;
		}
	}

	sceneEntrySound();
}

void Scene505::step() {
	if (_anim0ActvFl)
		handleRaoulAnimation();

	if (_anim1ActvFl)
		handleBothanimation();

	if (_anim2ActvFl)
		handlePartedAnimation();

	if (_game._trigger == 65) {
		_scene->freeAnimation(_globals._animationIndexes[1]);
		_vm->_sound->command(1);
		_partedFl = true;
		_anim2ActvFl = true;
		_anim1ActvFl = false;
		_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('b', 1), 0);

		int hotspotIDx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(hotspotIDx, Common::Point(91, 108), FACING_NORTHWEST);
		_scene->_dynamicHotspots[hotspotIDx]._articleNumber = PREP_ON;
		_scene->setDynamicAnim(hotspotIDx, _globals._animationIndexes[2], 3);
		_scene->setDynamicAnim(hotspotIDx, _globals._animationIndexes[2], 4);
		_scene->setDynamicAnim(hotspotIDx, _globals._animationIndexes[2], 5);
		_scene->setDynamicAnim(hotspotIDx, _globals._animationIndexes[2], 6);
		_scene->setDynamicAnim(hotspotIDx, _globals._animationIndexes[2], 7);
	}

	if (_game._trigger == 70) {
		_game._player._stepEnabled = true;
		if (!_game._visitedScenes._sceneRevisited) {
			_vm->_gameConv->run(20);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		}
	}
}

void Scene505::actions() {
	if (_game._trigger == 80) {
		_bothStatus = 2;
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 20) {
		handleCoffinDialog();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_CHRISTINE)) {
		if (_globals[kCoffinStatus] != 2)
			_vm->_dialogs->show(50536);
		else {
			_vm->_gameConv->run(20);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_partStatus = 10;
			_partCount = 0;
		}
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_UNLOCK, NOUN_SARCOPHAGUS) || _action.isAction(VERB_UNLOCK, NOUN_LID)) && !_globals[kLookedAtSkullFace]) {
		_vm->_dialogs->show(50539);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_UNLOCK, NOUN_SKULL_FACE) || _action.isAction(VERB_PUT, NOUN_KEY, NOUN_SKULL_FACE)) || ((_action.isAction(VERB_UNLOCK, NOUN_SARCOPHAGUS) || _action.isAction(VERB_UNLOCK, NOUN_LID)) && _globals[kLookedAtSkullFace])) {
		if (_globals[kCoffinStatus] == 0) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -1, -2);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 9, 95);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 96);
				_action._inProgress = false;
				return;

			case 95:
				_vm->_sound->command(76);
				_vm->_dialogs->show(50528);
				_action._inProgress = false;
				return;

			case 96:
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_globals[kCoffinStatus] = 1;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
				_action._inProgress = false;
				return;

			default:
				break;
			}
		} else {
			_vm->_dialogs->show(50534);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_PUSH, NOUN_SKULL) && (_scene->_customDest.x >= 19)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[0] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[0], false, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_SPRITE, 6, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1: {
			int sprIdx;
			_vm->_sound->command(77);
			if (_scene->_customDest.x <= 44)
				sprIdx = _globals._spriteIndexes[4];
			else if (_scene->_customDest.x <= 58)
				sprIdx = _globals._spriteIndexes[3];
			else if (_scene->_customDest.x <= 71)
				sprIdx = _globals._spriteIndexes[2];
			else if (_scene->_customDest.x <= 84) {
				sprIdx = _globals._spriteIndexes[1];
				if (_globals[kCoffinStatus] == 1) {
					_bothStatus = 1;
					_scene->_hotspots.activate(NOUN_LID, false);
					_scene->_hotspots.activate(NOUN_CHRISTINE, false);
					_scene->_hotspots.activateAtPos(NOUN_LID, true, Common::Point(216, 44));
					_scene->changeVariant(1);
				}
			} else if (_scene->_customDest.x <= 100)
				sprIdx = _globals._spriteIndexes[5];
			else
				sprIdx = _globals._spriteIndexes[6];

			int skullSeqIdx = _scene->_sequences.startPingPongCycle(sprIdx, false, 5, 2);
			_scene->_sequences.setAnimRange(skullSeqIdx, -1, -2);
			_scene->_sequences.setDepth(skullSeqIdx, 1);
				}
				break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			if (_bothStatus == 1) {
				_game._player.walk(Common::Point(136, 126), FACING_EAST);
				_game._player.setWalkTrigger(80);
				_game._player._stepEnabled = false;
			}
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR)) {
		if (_anim2ActvFl) {
			_leaveRoomFl = true;
			_game._player._stepEnabled = false;
		} else {
			_globals[kChrisLeft505] = true;
			_scene->_nextSceneId = 504;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(50510);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(50511);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(50512);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SARCOPHAGUS)) {
			_vm->_dialogs->show((_globals[kCoffinStatus] <= 1) ? 50513 : 50514);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SKULL_FACE)) {
			_globals[kLookedAtSkullFace] = true;
			_vm->_dialogs->show(50529);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(50519);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SKULL)) {
			_vm->_dialogs->show((_scene->_customDest.x < 19) ? 50521 : 50520);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SKULLS)) {
			_vm->_dialogs->show(50521);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TOTEM)) {
			_vm->_dialogs->show(50522);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DESK)) {
			_vm->_dialogs->show(50523);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_POLE)) {
			_vm->_dialogs->show(50524);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CURTAIN)) {
			_vm->_dialogs->show(50525);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHRISTINE)) {
			_vm->_dialogs->show((_globals[kCoffinStatus] == 2) ? 50530 : 50537);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LID)) {
			_vm->_dialogs->show((_globals[kCoffinStatus] == 2) ? 50531 : 50532);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_LOCK, NOUN_SARCOPHAGUS) || _action.isAction(VERB_LOCK, NOUN_LID) || _action.isAction(VERB_LOCK, NOUN_SKULL_FACE)) {
		_vm->_dialogs->show(50535);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_SARCOPHAGUS) || _action.isAction(VERB_OPEN, NOUN_LID)) {
		if (_globals[kCoffinStatus] == 2)
			_vm->_dialogs->show(50533);
		else if (_globals[kCoffinStatus] == 1)
			_vm->_dialogs->show(50518);
		else
			_vm->_dialogs->show(50515);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHRISTINE) && (_globals[kCoffinStatus] != 2)) {
		_vm->_dialogs->show(50538);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHRISTINE)) {
		_vm->_dialogs->show((_globals[kCoffinStatus] <= 1) ? 50538 : 50540);
		_action._inProgress = false;
	}
}

void Scene505::preActions() {
	if ((_globals[kCoffinStatus] == 0) && (_action.isAction(VERB_UNLOCK, NOUN_SKULL_FACE) || _action.isAction(VERB_UNLOCK, NOUN_SARCOPHAGUS) || _action.isAction(VERB_PUT, NOUN_KEY, NOUN_SKULL_FACE) || _action.isAction(VERB_UNLOCK, NOUN_LID))) {
		if (_action.isObject(NOUN_SKULL_FACE) || _globals[kLookedAtSkullFace])
			_game._player.walk(Common::Point(279, 150), FACING_SOUTHWEST);
	}

	if (_action.isObject(NOUN_SKULL_FACE) && (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)))
		_game._player.walk(Common::Point(279, 150), FACING_SOUTHWEST);

	if (_action.isObject(NOUN_CURTAIN) && (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)))
		_game._player._needToWalk = true;

	if (_action.isObject(NOUN_SKULL) && (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)))
		_game._player._needToWalk = true;
}

void Scene505::handleCoffinDialog() {
	int interlocutorFl = false;
	int heroFl = false;

	switch (_action._activeAction._verbId) {
	case 8:
		heroFl = true;
		interlocutorFl = true;
		_bothStatus = 6;
		break;

	case 14:
		heroFl = true;
		interlocutorFl = true;
		if (!_checkFrame106)
			_vm->_gameConv->hold();
		break;

	case 17:
		heroFl = true;
		interlocutorFl = true;
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_raoulStatus = 2;
		}
		break;

	case 20:
		heroFl = true;
		interlocutorFl = true;
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_game._player.walk(Common::Point(244, 130), FACING_SOUTHWEST);
			_game._player.setWalkTrigger(71);
		}
		break;

	case 22:
		heroFl = true;
		interlocutorFl = true;
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_bothStatus = 7;
		}
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 70:
	case 76:
		_vm->_gameConv->release();
		break;

	case 71:
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('r', 1), 75);
		_anim0ActvFl = true;
		_raoulStatus = 0;
		_raoulCount = 0;
		_game._player._visible = false;
		break;

	case 75:
		_game._player._visible = true;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
		_scene->_sequences.addTimer(10, 76);
		break;

	case 85:
		if ((_bothStatus != 6) && (_bothStatus != 0))
			_bothStatus = 5;
		break;

	case 90:
		if ((_bothStatus != 6) && (_bothStatus != 0))
			_bothStatus = 4;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(85);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(90);

	_bothCount = 0;
}

void Scene505::handleRaoulAnimation() {
	int curFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	if (curFrame == _raoulFrame)
		return;

	_raoulFrame = curFrame;
	int resetFrame = -1;

	switch (_raoulFrame) {
	case 3:
		_vm->_gameConv->release();
		break;

	case 4:
	case 5:
	case 6:
		if (_raoulStatus == 0) {
			resetFrame = _vm->getRandomNumber(3, 5);
			++_raoulCount;
			if (_raoulCount > 20) {
				_raoulStatus = 1;
				resetFrame = 3;
			}
			break;
		}

		if (_raoulStatus == 1)
			resetFrame = 3;

		if (_raoulStatus == 2)
			resetFrame = 6;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_raoulFrame = resetFrame;
	}
}

void Scene505::handleBothanimation() {
	int curFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	if (curFrame == _bothFrame)
		return;

	_bothFrame = curFrame;
	int resetFrame = -1;

	switch (_bothFrame) {
	case 1:
	case 20:
	case 39:
		if (_bothStatus == 0) {
			if (_vm->getRandomNumber(1, 35) == 1) {
				if (_vm->getRandomNumber(1, 2) == 1)
					resetFrame = 1;
				else
					resetFrame = 20;
			} else
				resetFrame = 0;
		} else if (_bothStatus == 1)
			resetFrame = 39;
		else
			resetFrame = 0;
		break;

	case 14:
		if (_vm->getRandomNumber(1, 3) == 1)
			resetFrame = 8;
		break;

	case 32:
		if (_vm->getRandomNumber(1, 2) == 1)
			resetFrame = 28;
		break;

	case 41:
		_vm->_sound->command(39);
		break;

	case 51:
		_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, 12);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
		_globals[kCoffinStatus] = 2;
		break;

	case 66:
		_game._player._stepEnabled = false;
		_vm->_gameConv->run(20);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		break;

	case 67:
		if (_bothStatus == 1)
			resetFrame = 66;
		break;

	case 68:
		_game._player._visible = false;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		break;

	case 106:
		_checkFrame106 = true;
		_vm->_gameConv->release();
		break;

	case 109:
	case 130:
		_vm->_gameConv->release();
		break;

	case 110:
	case 111:
	case 112:
	case 113:
	case 114:
	case 115:
	case 131:
		switch (_bothStatus) {
		case 4:
			resetFrame = _vm->getRandomNumber(112, 114);
			++_bothCount;
			if (_bothCount > 20) {
				_bothStatus = 3;
				resetFrame = 109;
			}
			break;

		case 5:
			resetFrame = _vm->getRandomNumber(109, 111);
			++_bothCount;
			if (_bothCount > 20) {
				_bothStatus = 3;
				resetFrame = 109;
			}
			break;

		case 6:
			resetFrame = 131;
			_bothStatus = 8;
			_game._player._stepEnabled = false;
			break;

		case 7:
			resetFrame = 115;
			_bothStatus = 3;
			break;

		default:
			resetFrame = 109;
			break;
		}
		break;

	case 127:
		_vm->_sound->command(26);
		_game._objects.addToInventory(OBJ_WEDDING_RING);
		_vm->_dialogs->showItem(OBJ_WEDDING_RING, 821, 0);
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_bothFrame = resetFrame;
	}
}

void Scene505::handlePartedAnimation() {
	int curFrame = _scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame();
	if (curFrame == _partFrame)
		return;

	_partFrame = curFrame;
	int resetFrame = -1;

	switch (_partFrame) {
	case 20:
		_vm->_sound->command(16);
		break;

	case 25:
		_game._player._playerPos = Common::Point(93, 133);
		_game._player.resetFacing(FACING_WEST);
		_game._player._visible = true;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[2]);
		break;

	case 70:
		_game._player._stepEnabled = true;
		break;

	case 90:
		if (_partStatus == 10)
			resetFrame = 146;
		else if (!_leaveRoomFl)
			resetFrame = 89;
		break;

	case 145:
		_scene->_nextSceneId = 504;
		break;

	case 147:
	case 148:
	case 149:
		resetFrame = _vm->getRandomNumber(146, 148);
		++_partCount;
		if (_partCount > 10) {
			resetFrame = 89;
			_partStatus = 8;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[2], resetFrame);
		_partFrame = resetFrame;
	}
}

/*------------------------------------------------------------------------*/

Scene506::Scene506(MADSEngine *vm) : Scene5xx(vm) {
	_anim0ActvFl = false;
	_skipFl = false;
	_ascendingFl = false;
}

void Scene506::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_skipFl);
	s.syncAsByte(_ascendingFl);
}

void Scene506::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_CHRISTINE);
	_scene->addActiveVocab(VERB_LOOK_AT);

	if (!_globals[kChristineIsInBoat])
		_scene->_variant = 1;
}

void Scene506::enter() {
	_vm->_disableFastwalk = true;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_skipFl = false;
		_ascendingFl = false;
	}

	_vm->_gameConv->load(26);

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('p', 0));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 0), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RDR_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_6", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_3", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_2", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);

	if (_game._objects.isInRoom(OBJ_OAR)) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
	} else
		_scene->_hotspots.activate(NOUN_OAR, false);

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);

	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
		if (!_globals[kChristineIsInBoat]) {
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('r', 1), 0);
			_scene->setAnimFrame(_globals._animationIndexes[0], 239);
			int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(79, 133), FACING_SOUTHWEST);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[0], 6);
		}
	} else if (_scene->_priorSceneId == 504) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		if (_game._visitedScenes._sceneRevisited) {
			_game._player._playerPos = Common::Point(189, 123);
			_game._player._facing = FACING_SOUTHWEST;
			_game._player._stepEnabled = false;
			_game._player._visible = false;

			if (!_globals[kChristineIsInBoat]) {
				_anim0ActvFl = true;
				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('r', 1), 0);
				_scene->setAnimFrame(_globals._animationIndexes[0], 239);
				int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(79, 133), FACING_SOUTHWEST);
				_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[0], 6);
			}
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('r', 2), 95);
		} else {
			_game._player._playerPos = Common::Point(186, 122);
			_game._player._facing = FACING_SOUTHWEST;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_anim0ActvFl = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('r', 1), 0);
			int hotspotIdx = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[hotspotIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(hotspotIdx, Common::Point(79, 133), FACING_SOUTHWEST);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[0], 2);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[0], 3);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[0], 4);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[0], 5);
			_scene->setDynamicAnim(hotspotIdx, _globals._animationIndexes[0], 6);
		}
	} else if ((_scene->_priorSceneId == 501) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(0, 142);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(23, 145), FACING_EAST);
		_game._player.setWalkTrigger(60);
	}

	sceneEntrySound();
}

void Scene506::step() {
	switch (_game._trigger) {
	case 60:
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
		_game._player._stepEnabled = true;
		break;

	case 95:
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		break;

	default:
		break;
	}

	if (_anim0ActvFl) {
		int curFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
		if ((curFrame == 141) && !_skipFl) {
			_game._player._visible = true;
			_skipFl = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
		}

		if (curFrame == 240)
			_scene->setAnimFrame(_globals._animationIndexes[0], 239);

		if (curFrame == 300)
			_scene->setAnimFrame(_globals._animationIndexes[0], 239);

		if (curFrame == 168)
			_game._player._stepEnabled = true;

		if (curFrame == 289)
			_scene->_nextSceneId = 501;
	}

	if (_ascendingFl && (_vm->_gameConv->activeConvId() != 26)) {
		_ascendingFl = false;
		_game._player._stepEnabled = false;
	}
}

void Scene506::actions() {
	if (_action.isAction(VERB_TALK_TO, NOUN_CHRISTINE)) {
		_vm->_gameConv->run(26);
		_vm->_gameConv->exportValue(1);
		_scene->setAnimFrame(_globals._animationIndexes[0], 290);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_OAR)) {
		switch (_game._trigger) {
		case (0):
			if (_game._objects.isInRoom(OBJ_OAR)) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[4], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 4, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[5]);
			_scene->_hotspots.activate(NOUN_OAR, false);
			_game._objects.addToInventory(OBJ_OAR);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[4]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_vm->_dialogs->showItem(OBJ_OAR, 824, 0);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}


	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) {
		if (_scene->_customDest.x < 150) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[4], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 4, 65);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 67);
				break;

			case 65:
				_scene->deleteSequence(_globals._sequenceIndexes[3]);
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
				_vm->_sound->command(24);
				break;

			case 66:
				_game._player.walk(Common::Point(0, 142), FACING_WEST);
				_game._player.setWalkTrigger(68);
				break;

			case 67:
				_game._player._visible = true;
				break;

			case 68:
				if (_globals[kChristineIsInBoat])
					_scene->_nextSceneId = 501;
				else
					_scene->setAnimFrame(_globals._animationIndexes[0], 241);
				break;

			default:
				break;
			}
		} else {
			switch (_game._trigger) {
			case (0):
				if (!_globals[kChristineIsInBoat]) {
					_vm->_gameConv->run(26);
					_vm->_gameConv->exportValue(2);
					int curFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
					if (curFrame == 240 || curFrame == 239)
						_scene->setAnimFrame(_globals._animationIndexes[0], 290);
					_ascendingFl = true;
				}
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], -1, -2);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[7], SYNC_PLAYER, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
				break;

			case 90:
				_vm->_gameConv->stop();
				_scene->_nextSceneId = 504;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 26) {
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(50610);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(50611);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(50612);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TORCH)) {
			_vm->_dialogs->show(50613);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COLUMN)) {
			_vm->_dialogs->show(50614);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(50615);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RAMP)) {
			_vm->_dialogs->show(50616);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			if (_scene->_customDest.x < 150)
				_vm->_dialogs->show(50617);
			else
				_vm->_dialogs->show(50618);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_OAR) && _game._objects.isInRoom(OBJ_OAR)) {
			_vm->_dialogs->show(50619);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHRISTINE)) {
			if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() < 235)
				_vm->_dialogs->show(50621);
			else
				_vm->_dialogs->show(50620);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_TORCH)) {
		_vm->_dialogs->show(50613);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHRISTINE)) {
		_vm->_dialogs->show(50622);
		_action._inProgress = false;
	}
}

void Scene506::preActions() {
	if (_action.isAction(VERB_UNLOCK, NOUN_DOOR) || _action.isAction(VERB_LOCK, NOUN_DOOR))
		_game._player.walk(Common::Point(33, 142), FACING_NORTHWEST);

	if (_action.isAction(VERB_OPEN, NOUN_DOOR)) {
		if (_scene->_customDest.x < 150)
			_game._player.walk(Common::Point(33, 142), FACING_NORTHWEST);
		else
			_game._player.walk(Common::Point(191, 118), FACING_EAST);
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
