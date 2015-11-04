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
		_vm->_sound->command((_vm->_gameConv->_restoreRunning == 20) ? 39 : 16);
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
	_anim_0_running = false;
	_prevent_2 = false;
}

void Scene501::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_anim_0_running);
	s.syncAsByte(_prevent_2);
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
		_anim_0_running = false;
		_prevent_2 = false;
	}

	_vm->_gameConv->get(26);

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0), false);
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1), false);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2), false);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('a', 1), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 0), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_6", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_9", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*CHR_8", PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RDRR_6", false);

	_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_globals[kChristineIsInBoat]) {
			_anim_0_running = true;
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
			_anim_0_running = true;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 1), 100);
			_scene->setAnimFrame(_globals._animationIndexes[0], 124);
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(113, 93));
			_scene->_hotspots.activate(NOUN_BOAT, true);
			_game._player.walk(Common::Point(260, 112), FACING_SOUTHWEST);
			_game._player.setWalkTrigger(80);
			_game._player.setWalkTrigger(55);
		} else {
			_anim_0_running = true;
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
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[2], 0, 0, 56);
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
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 61);
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
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[5], 0, 0, 90);
		break;

	default:
		break;
	}

	if (_anim_0_running) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 103)
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(125, 94));

		if ((_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 28) && !_prevent_2) {
			_prevent_2 = true;
			_scene->_sequences.setTimingTrigger(1, 55);
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
	if (_vm->_gameConv->_running == 26) {
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
			_anim_0_running = false;
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
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 1);
			break;

		case 1: {
			int idx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
			_game.syncTimers(1, _globals._sequenceIndexes[3], 1, idx);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
			_scene->_sequences.setTimingTrigger(15, 2);
			_vm->_sound->command(74);
			}
			break;

		case 2:
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 3);
			break;

		case 3:
			_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[3]);
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
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 1);
					break;

				case 1: {
					int idx = _globals._sequenceIndexes[3];
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
					_game.syncTimers(1, _globals._sequenceIndexes[3], 1, idx);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTimingTrigger(15, 2);
					_vm->_sound->command(74);
					}
					break;

				case 2:
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 3);
					break;

				case 3:
					_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[3]);
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
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 2, 4, 65);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 67);
					break;

				case 65:
					_scene->deleteSequence(_globals._sequenceIndexes[2]);
					_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[2], 0, 0, 66);
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
					_vm->_gameConv->abortConv();
					_scene->_nextSceneId = 506;
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
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 2, 4, 65);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 67);
					break;

				case 65: {
					int idx = _globals._sequenceIndexes[1];
					_scene->deleteSequence(_globals._sequenceIndexes[1]);
					_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
					_game.syncTimers(1, _globals._sequenceIndexes[1], 1, idx);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 66);
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
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 69);
					_vm->_sound->command(25);
					break;

				case 69:
					_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 5);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
					_scene->_nextSceneId = 502;
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
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 1);
					break;

				case 1: {
					int idx = _globals._sequenceIndexes[3];
					_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 4);
					_game.syncTimers(1, _globals._sequenceIndexes[3], 1, idx);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTimingTrigger(15, 2);
					_vm->_sound->command(73);
						}
						break;

				case 2:
					_scene->deleteSequence(_globals._sequenceIndexes[3]);
					_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 5, 1);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], false);
					_scene->_sequences.setTrigger(_globals._sequenceIndexes[3], 0, 0, 3);
					break;

				case 3:
					_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[3]);
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
			_scene->_sequences.setTimingTrigger(6, 1);
			break;

		case 1:
			if (_vm->_gameConv->_running >= 0)
				_scene->_sequences.setTimingTrigger(6, 1);
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

} // End of namespace Phantom
} // End of namespace MADS
