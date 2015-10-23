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
#include "mads/dragonsphere/dragonsphere_scenes.h"
#include "mads/dragonsphere/dragonsphere_scenes1.h"

namespace MADS {

namespace Dragonsphere {

void Scene1xx::setAAName() {
	int interface;

	switch (_scene->_nextSceneId) {
	case 108:
	case 109:
		interface = 3;
		break;
	case 110:
		interface = 5;
		break;
	case 113:
	case 114:
	case 115:
	case 117:
	case 119:
		interface = 1;
		break;
	case 116:
		interface = 2;
		break;
	case 120:
		interface = 8;
		break;
	default:
		interface = 0;
		break;
	}

	_game._aaName = Resources::formatAAName(interface);
	_vm->_palette->setEntry(254, 56, 47, 32);

}

void Scene1xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	switch (_scene->_nextSceneId) {
	case 104:
		if (_globals[kPlayerPersona] == 1)
			_vm->_sound->command(44);
		else
			_vm->_sound->command(16);
		break;

	case 106:
		if (_globals[kEndOfGame])
			_vm->_sound->command(47);
		else
			_vm->_sound->command(16);
		break;

	case 108:
		if (_game._visitedScenes.exists(109))
			_vm->_sound->command(32);
		else
			_vm->_sound->command(33);
		break;

	case 109:
		_vm->_sound->command(32);
		break;

	case 110:
		_vm->_sound->command(37);
		break;

	case 111:
		_vm->_sound->command(34);
		break;

	case 112:
		_vm->_sound->command(38);
		break;

	case 113:
		_vm->_sound->command(5);
		if (_globals[kPlayerIsSeal])
			_vm->_sound->command(35);
		else
			_vm->_sound->command(36);
		break;

	case 114:
		_vm->_sound->command(36);
		break;

	case 115:
		_vm->_sound->command(39);
		break;

	case 116:
		_vm->_sound->command(40);
		break;

	case 117:
		_vm->_sound->command(35);
		break;

	case 119:
		_vm->_sound->command(41);
		break;

	case 120:
		_vm->_sound->command(46);
		break;

	default:
		_vm->_sound->command(16);
		break;
	}
}

void Scene1xx::setPlayerSpritesPrefix() {
	int darkSceneFl = false;
	int noPlayerFl = false;

	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	_globals[kPerformDisplacements] = true;

	switch (_scene->_nextSceneId) {
	case 106:
		if (_scene->_currentSceneId == 120)
			noPlayerFl = true;
		break;

	case 108:
	case 109:
	case 114:
	case 115:
		darkSceneFl = true;
		break;

	case 111:
	case 112:
	case 117:
	case 120:
	case 119:
		noPlayerFl = true;
		break;

	case 113:
		if (!_globals[kPlayerPersona])
			noPlayerFl = true;
		darkSceneFl = true;
		break;
	}

	if (noPlayerFl || _globals[kNoLoadWalker]) {
		_game._player._spritesPrefix = "";
	} else if (!_game._player._forcePrefix) {
		if (!_globals[kPlayerPersona] || _scene->_nextSceneId == 108 || _scene->_nextSceneId == 109) {
			if (_scene->_nextSceneId == 113 || _scene->_nextSceneId == 114 || _scene->_nextSceneId == 115 || _scene->_nextSceneId == 116)
				_game._player._spritesPrefix = "PD";
			else
				_game._player._spritesPrefix = "KG";
		} else
			_game._player._spritesPrefix = "PD";

		if (darkSceneFl)
			_game._player._spritesPrefix += "D";
	}

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

Scene101::Scene101(MADSEngine *vm) : Scene1xx(vm) {
}

void Scene101::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
}

void Scene101::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene101::enter() {
	sceneEntrySound();
}

void Scene101::step() {
}

void Scene101::actions() {
	if (_action.isObject(NOUN_BED)) {
		int sprIdx = _scene->_sprites.addSprites("*ob001i", false);
		int seqIdx = _scene->_sequences.addStampCycle(sprIdx, false, 1);
		_scene->_sequences.setDepth(seqIdx, 0);
		_scene->_sequences.setPosition(seqIdx, Common::Point(10, 50));
		_action._inProgress = false;
	}
}

void Scene101::preActions() {
}

/*------------------------------------------------------------------------*/

Scene102::Scene102(MADSEngine *vm) : Scene1xx(vm) {
	_diaryHotspotIdx1 = -1;
	_diaryHotspotIdx2 = -1;
	_diaryFrame = -1;
	_animRunning = -1;
}

void Scene102::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsSint16LE(_diaryHotspotIdx1);
	s.syncAsSint16LE(_diaryHotspotIdx2);
	s.syncAsSint16LE(_diaryFrame);
	s.syncAsSint16LE(_animRunning);
}

void Scene102::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_DIARIES);
	_scene->addActiveVocab(VERB_WALK_TO);
}

void Scene102::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', 0), false);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', 1), false);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('y', 0), false);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('y', 1), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', -1), false);
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*KGRD_6", false);
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('y', 2), false);

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 0);
	_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 7, 0);

	_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 12);
	_diaryHotspotIdx1 = _scene->_dynamicHotspots.add(NOUN_DIARIES, VERB_WALK_TO, SYNTAX_PLURAL, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(_diaryHotspotIdx1, Common::Point(47, 123), FACING_NORTHWEST);

	_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 12);
	_diaryHotspotIdx2 = _scene->_dynamicHotspots.add(NOUN_DIARIES, VERB_WALK_TO, SYNTAX_PLURAL, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(_diaryHotspotIdx2, Common::Point(47, 123), FACING_NORTHWEST);

	if (_scene->_priorSceneId == 103) {
		_game._player._playerPos = Common::Point(170, 152);
		_game._player._facing = FACING_NORTHWEST;
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
	} else if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
		_game._player.firstWalk(Common::Point(-10, 130), FACING_EAST, Common::Point(35, 144), FACING_EAST, false);
		_game._player.setWalkTrigger(70);
	} else {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
	}

	sceneEntrySound();
}

void Scene102::step() {
	int resetFrame;

	if ((_animRunning == 1) && _scene->_animation[_globals._animationIndexes[0]]) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() != _diaryFrame) {
			_diaryFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
			resetFrame = -1;

			switch (_diaryFrame) {
			case 6:
				_scene->deleteSequence(_globals._sequenceIndexes[1]);
				_game.syncTimers(1, _globals._sequenceIndexes[1], 3, _globals._animationIndexes[0]);
				break;

			case 10:
				_vm->_sound->command(65);
				break;

			case 26:
				_vm->_dialogs->show(10210);
				_vm->_dialogs->show(10211);
				_vm->_dialogs->show(10212);
				break;

			default:
				break;
			}

			if (resetFrame >= 0) {
				if (resetFrame != _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame()) {
					_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
					_diaryFrame = resetFrame;
				}
			}
		}
	}

	if ((_animRunning == 2) && _scene->_animation[_globals._animationIndexes[0]]) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() != _diaryFrame) {
			_diaryFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
			resetFrame = -1;

			switch (_diaryFrame) {
			case 6:
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_game.syncTimers(1, _globals._sequenceIndexes[2], 3, _globals._animationIndexes[0]);
				break;

			case 26:
				_vm->_dialogs->show(10213);
				_vm->_dialogs->show(10214);
				break;

			default:
				break;
			}

			if (resetFrame >= 0) {
				if (resetFrame != _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame()) {
					_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
					_diaryFrame = resetFrame;
				}
			}
		}
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_scene->deleteSequence(_globals._sequenceIndexes[6]);
			_vm->_sound->command(25);
			_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 9, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[6], 0, 0, 71);
			break;

		case 71: {
			int idx = _globals._sequenceIndexes[6];
			_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], -2);
			_game.syncTimers(1, _globals._sequenceIndexes[6], 1, idx);
			_game._player._stepEnabled = true;
			}
			break;

		default:
			break;
		}
	}
}

void Scene102::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10201);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_KINGS_ROOM) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_KINGS_ROOM) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_KINGS_ROOM)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], true, 8, 2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[7],true);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[7], 2, 2, 1);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[7], 0, 0, 3);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[6]);
			_vm->_sound->command(24);
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[6], 0, 0, 2);
			break;

		case 2: {
			int idx = _globals._sequenceIndexes[6];
			_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
			_game.syncTimers(1, _globals._sequenceIndexes[6], 1, idx);
			}
			break;

		case 3:
			_game._player._visible = true;
			_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[7]);
			_game._player.walk(Common::Point(0, 130), FACING_WEST);
			_game._player._walkOffScreenSceneId = 101;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_HALLWAY) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_HALLWAY) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_HALLWAY)) {
		_scene->_nextSceneId = 103;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_DIARIES) || _action.isAction(VERB_OPEN, NOUN_DIARIES)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_animRunning = 1;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('B',-1), 1);
			_game.syncTimers(3, _globals._animationIndexes[0], 2, 0);
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 12);
			_diaryHotspotIdx1 = _scene->_dynamicHotspots.add(NOUN_DIARIES, VERB_WALK_TO, SYNTAX_PLURAL, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_diaryHotspotIdx1, Common::Point(47, 123), FACING_NORTHWEST);
			_game._player._visible = true;
			_game.syncTimers(2, 0, 3, _globals._animationIndexes[0]);
			_scene->_sequences.setTimingTrigger(6, 2);
			break;

		case 2:
			_game._player.walk(Common::Point(51, 121), FACING_NORTHWEST);
			_game._player.setWalkTrigger(3);
			break;

		case 3:
			_game._player._visible = false;
			_animRunning = 2;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('A',-1), 4);
			_game.syncTimers(3, _globals._animationIndexes[0], 2, 0);
			break;

		case 4:
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 12);
			_diaryHotspotIdx2 = _scene->_dynamicHotspots.add(NOUN_DIARIES, VERB_WALK_TO, SYNTAX_PLURAL, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_diaryHotspotIdx2, Common::Point(47, 123), FACING_NORTHWEST);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_game.syncTimers(2, 0, 3, _globals._animationIndexes[0]);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FIREPLACE)) {
			_vm->_dialogs->show(10202);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BED)) {
			_vm->_dialogs->show(10203);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SHUTTERS)) {
			_vm->_dialogs->show(10204);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RUG)) {
			_vm->_dialogs->show(10206);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOOKCASE)) {
			_vm->_dialogs->show(10208);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DIARIES)) {
			_vm->_dialogs->show(10209);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_KINGS_ROOM)) {
			_vm->_dialogs->show(10215);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOWERS)) {
			_vm->_dialogs->show(10216);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WINDOW)) {
			_vm->_dialogs->show(10217);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WOOD_BASKET)) {
			_vm->_dialogs->show(10219);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FIREPLACE_SCREEN)) {
			_vm->_dialogs->show(10220);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_NIGHTSTAND)) {
			_vm->_dialogs->show(10222);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_HALLWAY)) {
			_vm->_dialogs->show(10223);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHEST)) {
			_vm->_dialogs->show(10224);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TAPESTRY)) {
			_vm->_dialogs->show(10226);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SCONCE)) {
			_vm->_dialogs->show(10227);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(10228);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10229);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DECORATION)) {
			_vm->_dialogs->show(10230);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(10231);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_CHEST)) {
		_vm->_dialogs->show(10224);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_FLOWERS)) {
		_vm->_dialogs->show(10225);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_WINDOW)) {
		_vm->_dialogs->show(10218);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_FIREPLACE_SCREEN)) {
		_vm->_dialogs->show(10221);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_SHUTTERS)) {
		_vm->_dialogs->show(10205);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PULL, NOUN_RUG)) {
		_vm->_dialogs->show(10207);
		_action._inProgress = false;
		return;
	}
}

void Scene102::preActions() {
}

/*------------------------------------------------------------------------*/

Scene103::Scene103(MADSEngine *vm) : Scene1xx(vm) {
}

void Scene103::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
}

void Scene103::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene103::enter() {
	_vm->_disableFastwalk = true;

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('y', 1), false);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('y', 2), false);
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('y', 7), false);
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('y', 0), false);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('y', 3), false);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('y', 4), false);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('y', 5), false);
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('y', 6), false);
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('x', 0), false);
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('x', 1), false);
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites("*KGRD_9", false);

	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 0, 0);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 0, 4);
	_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 7, 0, 0);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 0, 0);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 0, 3);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 0, 2);
	_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 0, 0);
	_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 7, 0, 5);

	if ((_scene->_priorSceneId == 104) || (_scene->_priorSceneId == 105)) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
	}

	if (_scene->_priorSceneId == 102) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 6);
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);
		_scene->setCamera(Common::Point(320, 0));
		_game._player.walk(Common::Point(482, 128), FACING_SOUTH);
		_game._player.firstWalk(Common::Point(471, 108), FACING_SOUTH, Common::Point(482, 128), FACING_SOUTH, false);
		_game._player.setWalkTrigger(72);
	} else if (_scene->_priorSceneId == 104) {
		_game._player._playerPos = Common::Point(130, 152);
		_game._player._facing = FACING_NORTHEAST;
	} else if (_scene->_priorSceneId == 105) {
		_game._player._playerPos = Common::Point(517, 152);
		_game._player._facing = FACING_NORTHWEST;
		_scene->setCamera(Common::Point(320, 0));
	} else if ((_scene->_priorSceneId == 101) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);
		_game._player.firstWalk(Common::Point(173, 108), FACING_SOUTH, Common::Point(162, 127), FACING_SOUTH, false);
		_game._player.setWalkTrigger(70);
	} else {
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 6);
	}

	sceneEntrySound();
}

void Scene103::step() {
	switch (_game._trigger) {
	case 70:
		_scene->deleteSequence(_globals._sequenceIndexes[9]);
		_vm->_sound->command(25);
		_globals._sequenceIndexes[9] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 6);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 3);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[9], 0, 0, 71);
		break;

	case 71: {
		int tmpIdx = _globals._sequenceIndexes[9];
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 6);
		_game.syncTimers(1, _globals._sequenceIndexes[9], 1, tmpIdx);
		_game._player._stepEnabled = true;
		}
		break;

	case 72:
		_scene->deleteSequence(_globals._sequenceIndexes[10]);
		_vm->_sound->command(25);
		_globals._sequenceIndexes[10] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[10], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 3);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 0, 0, 73);
		break;

	case 73: {
		int tmpIdx = _globals._sequenceIndexes[10];
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		_game.syncTimers(1, _globals._sequenceIndexes[10], 1, tmpIdx);
		_game._player._stepEnabled = true;
		}
		break;

	default:
		break;
	}
}

void Scene103::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10301);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_KINGS_ROOM) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_KINGS_ROOM) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_KINGS_ROOM)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], false, 7, 2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[11],true);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[11], 2, 2, 1);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[11], 0, 0, 3);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[9]);
			_vm->_sound->command(24);
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[9], 0, 0, 2);
			break;

		case 2: {
			int tmpIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);
			_game.syncTimers(1, _globals._sequenceIndexes[9], 1, tmpIdx);
			}
			break;

		case 3:
			_game._player._visible = true;
			_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[11]);
			_scene->_sequences.setTimingTrigger(1,4);
			break;

		case 4:
			_game._player.walk(Common::Point(173, 108), FACING_NORTH);
			_game._player.setWalkTrigger(5);
			break;

		case 5:
			_scene->deleteSequence(_globals._sequenceIndexes[9]);
			_vm->_sound->command(25);
			_globals._sequenceIndexes[9] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[9], 0, 0, 6);
			break;

		case 6: {
			int tmpIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
			_game.syncTimers(1, _globals._sequenceIndexes[9], 1, tmpIdx);
			_scene->_sequences.setTimingTrigger(6, 7);
			}
			break;

		case 7:
			_scene->_nextSceneId = 101;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_QUEENS_ROOM) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_QUEENS_ROOM) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_QUEENS_ROOM)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], true, 7, 2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[11],true);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[11], 2, 2, 1);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[11], 0, 0, 3);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[10]);
			_vm->_sound->command(24);
			_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 0, 0, 2);
			break;

		case 2: {
			int tmpIdx = _globals._sequenceIndexes[10];
			_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);
			_game.syncTimers(1, _globals._sequenceIndexes[10], 1, tmpIdx);
			}
			break;

		case 3:
			_game._player._visible = true;
			_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[11]);
			_scene->_sequences.setTimingTrigger(1,4);
			break;

		case 4:
			_game._player.walk(Common::Point(471, 108), FACING_NORTH);
			_game._player.setWalkTrigger(5);
			break;

		case 5:
			_scene->deleteSequence(_globals._sequenceIndexes[10]);
			_vm->_sound->command(25);
			_globals._sequenceIndexes[10] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[10], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[10], 0, 0, 6);
			break;

		case 6: {
			int tmpIdx = _globals._sequenceIndexes[10];
			_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
			_game.syncTimers(1, _globals._sequenceIndexes[10], 1, tmpIdx);
			_scene->_sequences.setTimingTrigger(6, 7);
			}
			break;

		case 7:
			_scene->_nextSceneId = 102;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_MEETING_ROOM) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_MEETING_ROOM) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_MEETING_ROOM)) {
		_scene->_nextSceneId = 104;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_BALLROOM) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_BALLROOM) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_BALLROOM)) {
		_scene->_nextSceneId = 105;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_TAPESTRY)) {
			if ((_scene->_customDest.x <= 75) && (_scene->_customDest.y <= 130))
				_vm->_dialogs->show(10302);
			else
				_vm->_dialogs->show(10303);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COAT_OF_ARMS)) {
			_vm->_dialogs->show(10305);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_QUEENS_ROOM)) {
			_vm->_dialogs->show(10307);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_KINGS_ROOM)) {
			_vm->_dialogs->show(10308);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEDAR_CHEST)) {
			_vm->_dialogs->show(10309);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TABLE)) {
			_vm->_dialogs->show(10311);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SMALL_WINDOW)) {
			_vm->_dialogs->show(10312);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LARGE_WINDOW)) {
			_vm->_dialogs->show(10314);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BATTLE_AXES)) {
			_vm->_dialogs->show(10315);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BUST_ON_WALL)) {
			_vm->_dialogs->show(10317);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DECORATION)) {
			_vm->_dialogs->show(10320);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL_PLAQUE)) {
			_vm->_dialogs->show(10322);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_BALLROOM)) {
			_vm->_dialogs->show(10323);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_MEETING_ROOM)) {
			_vm->_dialogs->show(10324);
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isAction(VERB_TAKE) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_BATTLE_AXES)) {
		_vm->_dialogs->show(10316);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_TAPESTRY)) {
		_vm->_dialogs->show(10304);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PULL, NOUN_COAT_OF_ARMS)) {
		_vm->_dialogs->show(10306);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_CEDAR_CHEST)) {
		_vm->_dialogs->show(10310);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_SMALL_WINDOW) || _action.isAction(VERB_OPEN, NOUN_LARGE_WINDOW)) {
		_vm->_dialogs->show(10313);
		_action._inProgress = false;
		return;
	}
}

void Scene103::preActions() {
}

/*------------------------------------------------------------------------*/

} // End of namespace Dragonsphere
} // End of namespace MADS
