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
#include "mads/dragonsphere/dragonsphere_scenes.h"
#include "mads/dragonsphere/dragonsphere_scenes1.h"

namespace MADS {

namespace Dragonsphere {

void Scene1xx::setAAName() {
	int idx;

	switch (_scene->_nextSceneId) {
	case 108:
	case 109:
		idx = 3;
		break;
	case 110:
		idx = 5;
		break;
	case 113:
	case 114:
	case 115:
	case 117:
	case 119:
		idx = 1;
		break;
	case 116:
		idx = 2;
		break;
	case 120:
		idx = 8;
		break;
	default:
		idx = 0;
		break;
	}

	_game._aaName = Resources::formatAAName(idx);
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

	default:
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
		int sprIdx = _scene->_sprites.addSprites("*ob001i");
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
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('y', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('y', 1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', -1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*KGRD_6");
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('y', 2));

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
	if ((_animRunning == 1) && _scene->_animation[_globals._animationIndexes[0]]) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() != _diaryFrame) {
			_diaryFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();

			switch (_diaryFrame) {
			case 6:
				_scene->deleteSequence(_globals._sequenceIndexes[1]);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_ANIM, _globals._animationIndexes[0]);
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
		}
	}

	if ((_animRunning == 2) && _scene->_animation[_globals._animationIndexes[0]]) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() != _diaryFrame) {
			_diaryFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();

			switch (_diaryFrame) {
			case 6:
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[2], SYNC_ANIM, _globals._animationIndexes[0]);
				break;

			case 26:
				_vm->_dialogs->show(10213);
				_vm->_dialogs->show(10214);
				break;

			default:
				break;
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
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 71: {
			int idx = _globals._sequenceIndexes[6];
			_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], -2);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[6], SYNC_SEQ, idx);
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
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[7], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[6]);
			_vm->_sound->command(24);
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2: {
			int idx = _globals._sequenceIndexes[6];
			_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[6], SYNC_SEQ, idx);
			}
			break;

		case 3:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[7]);
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
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 12);
			_diaryHotspotIdx1 = _scene->_dynamicHotspots.add(NOUN_DIARIES, VERB_WALK_TO, SYNTAX_PLURAL, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_diaryHotspotIdx1, Common::Point(47, 123), FACING_NORTHWEST);
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
			_scene->_sequences.addTimer(6, 2);
			break;

		case 2:
			_game._player.walk(Common::Point(51, 121), FACING_NORTHWEST);
			_game._player.setWalkTrigger(3);
			break;

		case 3:
			_game._player._visible = false;
			_animRunning = 2;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('A',-1), 4);
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
			break;

		case 4:
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 12);
			_diaryHotspotIdx2 = _scene->_dynamicHotspots.add(NOUN_DIARIES, VERB_WALK_TO, SYNTAX_PLURAL, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_diaryHotspotIdx2, Common::Point(47, 123), FACING_NORTHWEST);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
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

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('y', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('y', 2));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('y', 7));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('y', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('y', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('y', 4));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('y', 5));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('y', 6));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites("*KGRD_9");

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
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71: {
		int tmpIdx = _globals._sequenceIndexes[9];
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 6);
		_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[9], SYNC_SEQ, tmpIdx);
		_game._player._stepEnabled = true;
		}
		break;

	case 72:
		_scene->deleteSequence(_globals._sequenceIndexes[10]);
		_vm->_sound->command(25);
		_globals._sequenceIndexes[10] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[10], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		break;

	case 73: {
		int tmpIdx = _globals._sequenceIndexes[10];
		_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[10], SYNC_SEQ, tmpIdx);
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
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[11], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[9]);
			_vm->_sound->command(24);
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2: {
			int tmpIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 14);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[9], SYNC_SEQ, tmpIdx);
			}
			break;

		case 3:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[11]);
			_scene->_sequences.addTimer(1, 4);
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
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6: {
			int tmpIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 1);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[9], SYNC_SEQ, tmpIdx);
			_scene->_sequences.addTimer(6, 7);
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
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[11], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[10]);
			_vm->_sound->command(24);
			_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2: {
			int tmpIdx = _globals._sequenceIndexes[10];
			_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[10], SYNC_SEQ, tmpIdx);
			}
			break;

		case 3:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[11]);
			_scene->_sequences.addTimer(1, 4);
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
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6: {
			int tmpIdx = _globals._sequenceIndexes[10];
			_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 1);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[10], SYNC_SEQ, tmpIdx);
			_scene->_sequences.addTimer(6, 7);
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

Scene104::Scene104(MADSEngine *vm) : Scene1xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;
	_anim4ActvFl = false;
	_anim5ActvFl = false;
	_activateTimerFl = false;
	_wasBearFl = false;
	_amuletWorksFl = false;
	_pidDrawnSword = false;
	_anim6ActvFl = false;

	_animationRunning = -1;
	_deathTimer = -1;
	_deathFrame = -1;
	_doorwayHotspotId = -1;

	_kingStatus = -1;
	_kingFrame = -1;
	_kingCount = -1;
	_macCount = -1;
	_macFrame = -1;
	_macStatus = -1;
	_queenStatus = -1;
	_queenFrame = -1;
	_queenCount = -1;
	_pidStatus = -1;
	_pidFrame = -1;
	_pidCount = -1;
	_twinklesFrame = -1;
	_twinklesStatus = -1;
	_twinklesCount = -1;

	_tapestryFrame = -1;
	_clock = -1;
}

void Scene104::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_anim3ActvFl);
	s.syncAsByte(_anim4ActvFl);
	s.syncAsByte(_anim5ActvFl);
	s.syncAsByte(_activateTimerFl);
	s.syncAsByte(_wasBearFl);
	s.syncAsByte(_amuletWorksFl);
	s.syncAsByte(_pidDrawnSword);
	s.syncAsByte(_anim6ActvFl);

	s.syncAsSint16LE(_animationRunning);
	s.syncAsSint16LE(_deathTimer);
	s.syncAsSint16LE(_deathFrame);
	s.syncAsSint16LE(_doorwayHotspotId);

	s.syncAsSint16LE(_kingStatus);
	s.syncAsSint16LE(_kingFrame);
	s.syncAsSint16LE(_kingCount);
	s.syncAsSint16LE(_queenStatus);
	s.syncAsSint16LE(_queenFrame);
	s.syncAsSint16LE(_queenCount);
	s.syncAsSint16LE(_macStatus);
	s.syncAsSint16LE(_macFrame);
	s.syncAsSint16LE(_macCount);
	s.syncAsSint16LE(_pidStatus);
	s.syncAsSint16LE(_pidFrame);
	s.syncAsSint16LE(_pidCount);
	s.syncAsSint16LE(_twinklesStatus);
	s.syncAsSint16LE(_twinklesFrame);
	s.syncAsSint16LE(_twinklesCount);

	s.syncAsSint16LE(_tapestryFrame);
	s.syncAsSint32LE(_clock);
}

void Scene104::setup() {
	if (_scene->_currentSceneId == 119)
		_globals[kNoLoadWalker] = true;

	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_DOORWAY);
	_scene->addActiveVocab(NOUN_QUEEN_MOTHER);
	_scene->addActiveVocab(NOUN_KING);
}

void Scene104::enter() {
	_vm->_gameConv->load(1);

	if (_globals[kPlayerPersona] == 1) {
		_scene->_sprites.addSprites(formAnimName('e', 8));
		_scene->_sprites.addSprites(formAnimName('b', 5));
	}

	_scene->_hotspots.activate(NOUN_MACMORN, false);

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_animationRunning = 0;
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim2ActvFl = false;
		_anim3ActvFl = false;
		_anim4ActvFl = false;
		_anim5ActvFl = false;
		_kingCount = 0;
		_macCount = 0;
		_queenCount = 0;
		_twinklesCount = 0;
		_deathTimer = 0;
		_clock = 0;
		_activateTimerFl = false;
		_wasBearFl = false;
		_amuletWorksFl = false;
		_pidDrawnSword = false;
	}

	_anim6ActvFl = false;

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 4));

	if (_globals[kNoLoadWalker]) {
		_scene->drawToBackground(_globals._spriteIndexes[6], 5, Common::Point(-32000, -32000), 0, 100);
		_scene->_sprites.remove(_globals._spriteIndexes[6]);
	}

	if (_globals[kPlayerPersona] == 0) {
		_scene->_hotspots.activateAtPos(NOUN_TABLE, false, Common::Point(139, 132));

		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*KGRD_8");
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*KGRM1_8");
		_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('y', 5));
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('y', 4));
		_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('y', 1));

		if (_globals[kTapestryStatus] == 0 || _globals[kTapestryStatus] == 2) {
			_scene->_hotspots.activate(NOUN_WALL_PANEL, false);
			_scene->_hotspots.activate(NOUN_SECRET_DOOR, false);
			_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 6);
		} else {
			_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
			_scene->_hotspots.activateAtPos(NOUN_TAPESTRY, false, Common::Point(310, 70));
		}

		if (_globals[kBooksStatus] == 1 || _globals[kBooksStatus] == 3) {
			if (_globals[kTapestryStatus] == 1 || _globals[kTapestryStatus] == 3)
				_scene->_hotspots.activate(NOUN_SECRET_DOOR, false);
		} else if (_globals[kBooksStatus] == 2 || _globals[kBooksStatus] == 4) {
			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);

			if (_globals[kTapestryStatus] == 1 || _globals[kTapestryStatus] == 3) {
				_scene->_hotspots.activate(NOUN_WALL_PANEL, false);
				_scene->_hotspots.activate(NOUN_SECRET_DOOR, true);
			}
		}
	} else {
		_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('e', 5));
		_scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('b', 3));
		_scene->_hotspots.activateAtPos(NOUN_TABLE, false, Common::Point(140, 107));
		_scene->_hotspots.activateAtPos(NOUN_TABLE, true, Common::Point(139, 132));
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('y', 4));

		if (_globals[kNoLoadWalker]) {
			_scene->drawToBackground(_globals._spriteIndexes[10], 1, Common::Point(-32000, -32000), 0, 100);
			_scene->_sprites.remove(_globals._spriteIndexes[10]);
		}

		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('y', 3));
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_doorwayHotspotId = _scene->_dynamicHotspots.add(NOUN_DOORWAY, VERB_WALK_THROUGH, SYNTAX_SINGULAR, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_doorwayHotspotId, Common::Point(295, 145), FACING_NORTHEAST);

		_globals[kBooksStatus] = 0;
		_scene->_hotspots.activateAtPos(NOUN_TAPESTRY, false, Common::Point(310, 70));
	}

	if (_globals[kBooksStatus] == 0)
		_scene->_hotspots.activate(NOUN_BOOKS, false);

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 0, 5);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 0);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 0);


	if (_globals[kPlayerPersona] == 0) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
	}

	if (_scene->_priorSceneId == 106) {
		_game._player._playerPos = Common::Point(201, 152);
		_game._player._facing = FACING_NORTHWEST;
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
	} else if ((_scene->_priorSceneId == 119) || (_scene->_priorSceneId == 104)) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
		_scene->_sequences.addTimer(60, 77);

		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('k', 1), 0);
		_anim0ActvFl = true;
		_kingStatus = 2;
		_scene->setAnimFrame(_globals._animationIndexes[0], 22);

		_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('q', 1), 0);
		_anim2ActvFl = true;
		_queenStatus = 0;

		_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 6);

		_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('p', 1), 79);

		int idx = _scene->_dynamicHotspots.add(NOUN_QUEEN_MOTHER, VERB_WALK_TO, SYNTAX_FEM_NOT_PROPER, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
		_scene->setDynamicAnim(idx, _globals._animationIndexes[2], 0);

		_scene->_hotspots.activate(NOUN_MACMORN, true);

		idx = _scene->_dynamicHotspots.add(NOUN_KING, VERB_WALK_TO, SYNTAX_MASC_NOT_PROPER, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 0);
	} else if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
		_game._player.firstWalk(Common::Point(77, 93), FACING_SOUTH, Common::Point(74, 107), FACING_SOUTH, false);
		_game._player.setWalkTrigger(70);
	} else if (_globals[kNoLoadWalker]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);

		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('k', 1), 0);
		_anim0ActvFl = true;
		_kingStatus = 0;
		_scene->setAnimFrame(_globals._animationIndexes[0], 14);

		_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('q', 1), 0);
		_anim2ActvFl = true;
		_queenStatus = 0;

		_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('p', 2), 0);
		_anim4ActvFl = true;
		_pidStatus = 0;
		if (_amuletWorksFl)
			_scene->setAnimFrame(_globals._animationIndexes[4], 89);

		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('m', 1), 0);
		_anim1ActvFl = true;
		_macStatus = 0;
		_scene->setAnimFrame(_globals._animationIndexes[1], 25);

		int idx = _scene->_dynamicHotspots.add(NOUN_QUEEN_MOTHER, VERB_WALK_TO, SYNTAX_FEM_NOT_PROPER, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
		_scene->setDynamicAnim(idx, _globals._animationIndexes[2], 0);

		_scene->_hotspots.activate(NOUN_MACMORN, true);

		idx = _scene->_dynamicHotspots.add(NOUN_KING, VERB_WALK_TO, SYNTAX_MASC_NOT_PROPER, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
		_scene->setDynamicAnim(idx, _globals._animationIndexes[0], 0);

		if (_vm->_gameConv->restoreRunning() == 1) {
			_game._player._stepEnabled = false;
			_vm->_gameConv->run(1);
			_vm->_gameConv->exportValue(0);
			_vm->_gameConv->exportValue(0);
			_vm->_gameConv->exportValue(0);
			if (_globals[kLlanieStatus] != 2)
				_vm->_gameConv->exportValue(1);
			else
				_vm->_gameConv->exportValue(0);
		} else {
			_activateTimerFl = true;
			_deathTimer = 0;
			_clock = 0;
			_game._player._stepEnabled = true;
		}
	} else {
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 12);
	}

	sceneEntrySound();
}

void Scene104::step() {
	if (_anim0ActvFl)
		handleKingAnimation();

	if (_anim1ActvFl)
		handleMacAnimation1();

	if (_anim2ActvFl)
		handleQueenAnimation();

	if (_anim3ActvFl)
		handleTwinklesAnimation();

	if (_anim4ActvFl)
		handlePidAnimation();

	if (_anim5ActvFl)
		handleMacAnimation2();

	if (_anim6ActvFl)
		handleDeathAnimation();

	if ((_animationRunning == 1) && _scene->_animation[_globals._animationIndexes[0]]) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() != _tapestryFrame) {
			_tapestryFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();

			if (_tapestryFrame == 13) {
				_game._player._visible = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
			}
		}
	}

	switch (_game._trigger) {
	case 70:
		_scene->deleteSequence(_globals._sequenceIndexes[6]);
		_vm->_sound->command(25);
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71: {
		int idx = _globals._sequenceIndexes[6];
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
		_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[6], SYNC_SEQ, idx);
		_game._player._stepEnabled = true;
		}
		break;

	default:
		break;
	}

	if (_game._trigger == 77)
		_kingStatus = 0;

	if (_game._trigger == 79) {
		_scene->freeAnimation(_globals._animationIndexes[4]);

		_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('p', 2), 0);
		_anim4ActvFl = true;
		_pidStatus = 0;
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[4], SYNC_CLOCK, 0);

		_scene->deleteSequence(_globals._sequenceIndexes[14]);
		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('m', 1), 0);
		_anim1ActvFl = true;
		_macStatus = 0;

		_game._player._stepEnabled = false;
		_vm->_gameConv->run(1);
		_vm->_gameConv->exportValue(0);
		_vm->_gameConv->exportValue(0);
		_vm->_gameConv->exportValue(0);

		if (_globals[kLlanieStatus] != 2)
			_vm->_gameConv->exportValue(1);
		else
			_vm->_gameConv->exportValue(0);
	}

	if (_game._trigger == 85) {
		_vm->_sound->command(100);
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
	}

	if (_activateTimerFl) {
		long clockDiff = _scene->_frameStartTime - _clock;
		if ((clockDiff >= 0) && (clockDiff <= 4))
			_deathTimer += clockDiff;
		else
			_deathTimer += 1;

		_clock = _scene->_frameStartTime;

		if (_deathTimer >= 1300) {
			_activateTimerFl = false;
			if (_pidDrawnSword) {
				_pidStatus = 6;
				_vm->_gameConv->run(1);
				_vm->_gameConv->exportValue(0);
				_vm->_gameConv->exportValue(1);
				_vm->_gameConv->exportValue(0);
				if (_globals[kLlanieStatus] != 2)
					_vm->_gameConv->exportValue(1);
				else
					_vm->_gameConv->exportValue(0);
				_vm->_gameConv->hold();
			} else
				_pidStatus = 8;

			_game._player._stepEnabled = false;
		}
	}


	if ((_globals[kTapestryStatus] == 1 || _globals[kTapestryStatus] == 3)
		&& (_globals[kBooksStatus] == 2 || _globals[kBooksStatus] == 4))
		_globals[kCanViewCrownHole] = true;

	if (_game._trigger == 95) {
		_vm->_gameConv->reset(1);
		_vm->_dialogs->show(10466);
		_globals[kNoLoadWalker] = false;
		_scene->_nextSceneId = 119;
	}
}

void Scene104::actions() {
	if (_vm->_gameConv->activeConvId() == 1) {
		handleFinalConversation();
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if (_globals[kPlayerPersona] == 0)
			_vm->_dialogs->show(10401);
		else
			_vm->_dialogs->show(10437);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_THRONE_ROOM) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_THRONE_ROOM) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_THRONE_ROOM)) {
		if (_globals[kPlayerPersona] == 0)
			_scene->_nextSceneId = 106;
		else
			_vm->_dialogs->show(10434);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_HALLWAY) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_HALLWAY) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_HALLWAY)) {
		if (_globals[kPlayerPersona] == 0) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], false, 8, 2);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[7], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 1:
				_scene->deleteSequence(_globals._sequenceIndexes[6]);
				_vm->_sound->command(24);
				_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2: {
				int tmpIdx = _globals._sequenceIndexes[6];
				_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[6], SYNC_SEQ, tmpIdx);
				}
				break;

			case 3:
				_game._player._visible = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[7]);
				_scene->_sequences.addTimer(1, 4);
				break;

			case 4:
				_game._player.walk(Common::Point(77, 93), FACING_NORTH);
				_game._player.setWalkTrigger(5);
				break;

			case 5:
				_scene->deleteSequence(_globals._sequenceIndexes[6]);
				_vm->_sound->command(25);
				_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 7, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
				break;

			case 6: {
				int tmpIdx = _globals._sequenceIndexes[6];
				_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 1);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[6], SYNC_SEQ, tmpIdx);
				_scene->_sequences.addTimer(6, 7);
				}
				break;

			case 7:
				_scene->_nextSceneId = 103;
				break;

			default:
				break;
			}
		} else
			_vm->_dialogs->show(10434);

		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PULL) || _action.isAction(VERB_TAKE) || _action.isAction(VERB_OPEN)) && _action.isObject(NOUN_BOOKS)) {
		if ((_globals[kBooksStatus] == 1) || (_globals[kBooksStatus] == 3) || _game._trigger) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 8, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -1, 3);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 3, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 1:
				_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2: {
				int tmpIdx = _globals._sequenceIndexes[12];
				_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, -2);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[12], SYNC_SEQ, tmpIdx);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
				if ((_globals[kTapestryStatus] == 1) || (_globals[kTapestryStatus] == 3)) {
					_scene->_hotspots.activate(NOUN_WALL_PANEL, false);
					_scene->_hotspots.activate(NOUN_SECRET_DOOR, true);
				}
				}
				break;

			case 3:
				_game._player._visible = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
				_scene->_sequences.addTimer(5, 4);
				break;

			case 4:
				if (_globals[kBooksStatus] == 1) {
					_globals[kPlayerScore] += 2;
					_globals[kBooksStatus] = 2;
					if ((_globals[kTapestryStatus] == 1) || (_globals[kTapestryStatus] == 3))
						_vm->_dialogs->show(10428);
					else {
						_vm->_sound->command(94);
						_vm->_sound->command(67);
						_vm->_dialogs->show(10427);
					}
				} else {
					_vm->_sound->command(94);
					_vm->_sound->command(67);
					_globals[kBooksStatus] = 4;
				}
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_PUSH, NOUN_BOOKS)) {
		if ((_globals[kBooksStatus] == 2) || (_globals[kBooksStatus] == 4)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 8, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -1, 3);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 3, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 1:
				_vm->_sound->command(94);
				_vm->_sound->command(67);
				_scene->deleteSequence(_globals._sequenceIndexes[12]);
				_globals._sequenceIndexes[12] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[12], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
				_scene->_sequences.addTimer(6, 2);
				break;

			case 2:
				if ((_globals[kTapestryStatus] == 1) || (_globals[kTapestryStatus] == 3)) {
					_scene->_hotspots.activate(NOUN_WALL_PANEL, true);
					_scene->_hotspots.activate(NOUN_SECRET_DOOR, false);
				}
				break;

			case 3:
				_game._player._visible = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
				_scene->_sequences.addTimer(5, 4);
				break;

			case 4:
				if (_globals[kBooksStatus] == 2)
					_vm->_dialogs->show(10429);
				_globals[kBooksStatus] = 3;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isAction(VERB_PULL) || _action.isAction(VERB_OPEN) || _action.isAction(VERB_PUSH)) && _action.isObject(NOUN_TAPESTRY)) {
		if (_globals[kPlayerPersona] == 0) {
			if (_scene->_customDest.x >= 279) {
				if ((_globals[kTapestryStatus] == 0) || (_globals[kTapestryStatus] == 2)) {
					switch (_game._trigger) {
					case 0:
						_scene->deleteSequence(_globals._sequenceIndexes[11]);
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_animationRunning = 1;
						_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('a', -1), 1);
						break;

					case 1:
						_game._player._visible = true;
						_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
						_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
						_scene->_hotspots.activateAtPos(NOUN_TAPESTRY, true, Common::Point(290, 20));
						_scene->_hotspots.activateAtPos(NOUN_TAPESTRY, false, Common::Point(310, 70));
						if ((_globals[kBooksStatus] == 2) || (_globals[kBooksStatus] == 4)) {
							_scene->_hotspots.activate(NOUN_WALL_PANEL, false);
							_scene->_hotspots.activate(NOUN_SECRET_DOOR, true);
						} else {
							_scene->_hotspots.activate(NOUN_WALL_PANEL, true);
							_scene->_hotspots.activate(NOUN_SECRET_DOOR, false);
						}
						_scene->_sequences.addTimer(6, 2);
						break;

					case 2:
						if (_globals[kTapestryStatus] == 0) {
							_globals[kTapestryStatus] = 1;
							_globals[kPlayerScore] += 2;
							if (_globals[kBooksStatus] == 0 || _globals[kBooksStatus] == 1 || _globals[kBooksStatus] == 3)
								_vm->_dialogs->show(10424);
							else if (_globals[kBooksStatus] == 2 || _globals[kBooksStatus] == 4)
								_vm->_dialogs->show(10425);
						} else
							_globals[kTapestryStatus] = 3;

						_game._player._stepEnabled = true;
						break;

					default:
						break;
					}
				}
			} else
				_vm->_dialogs->show(10404);
		} else
			_vm->_dialogs->show(10445);

		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PULL) || _action.isAction(VERB_CLOSE) || _action.isAction(VERB_PUSH)) && _action.isObject(NOUN_TAPESTRY)) {
		if (_globals[kPlayerPersona] == 0) {
			if (_scene->_customDest.x >= 279) {
				if (_globals[kTapestryStatus] == 1 || _globals[kTapestryStatus] == 3) {
					switch (_game._trigger) {
					case 0:
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_scene->deleteSequence(_globals._sequenceIndexes[10]);
						_animationRunning = 2;
						_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', -1), 1);
						break;

					case 1:
						_game._player._visible = true;
						_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
						_globals._sequenceIndexes[11] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[11], false, -1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 6);
						_scene->_hotspots.activateAtPos(NOUN_TAPESTRY, false, Common::Point(290, 20));
						_scene->_hotspots.activateAtPos(NOUN_TAPESTRY, true, Common::Point(310, 70));
						_scene->_hotspots.activate(NOUN_SECRET_DOOR, false);
						_scene->_hotspots.activate(NOUN_WALL_PANEL, false);
						_scene->_sequences.addTimer(6, 2);
						break;

					case 2:
						_globals[kTapestryStatus] = 2;
						_game._player._stepEnabled = true;
						break;

					default:
						break;
					}
					_action._inProgress = false;
					return;
				}
			}
		} else {
			_vm->_dialogs->show(10445);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_BOOKS) || _action.isObject(NOUN_BOOKSHELF)) {
			if (_globals[kPlayerPersona] == 0) {
				switch (_globals[kBooksStatus]) {
				case 0:
					_scene->_hotspots.activate(NOUN_BOOKS, true);
					_globals[kBooksStatus] = 1;
					_vm->_dialogs->show(10418);
					_action._inProgress = false;
					return;

				case 1:
					_vm->_dialogs->show(10418);
					_action._inProgress = false;
					return;

				case 2:
				case 4:
					_vm->_dialogs->show(10419);
					_action._inProgress = false;
					return;

				case 3:
					_vm->_dialogs->show(10420);
					_action._inProgress = false;
					return;

				default:
					break;
				}
			} else {
				_vm->_dialogs->show(10439);
				_action._inProgress = false;
				return;
			}
		}

		if (_action.isObject(NOUN_FIREPLACE)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10402);
			else
				_vm->_dialogs->show(10438);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TAPESTRY)) {
			if (_globals[kPlayerPersona] == 0) {
				if (_scene->_customDest.x >= 209 && _scene->_customDest.x <= 278)
					_vm->_dialogs->show(10403);
				else if (_scene->_customDest.x >= 107 && _scene->_customDest.x <= 190)
						_vm->_dialogs->show(10422);
				else if (_globals[kTapestryStatus] == 1 || _globals[kTapestryStatus] == 3)
					_vm->_dialogs->show(10460);
				else
					_vm->_dialogs->show(10423);
			} else
				_vm->_dialogs->show(10439);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RUG)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10405);
			else
				_vm->_dialogs->show(10439);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FIREPLACE_SCREEN)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10407);
			else
				_vm->_dialogs->show(10439);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_THRONE_ROOM)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10409);
			else
				_vm->_dialogs->show(10434);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SCONCE)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10410);
			else
				_vm->_dialogs->show(10440);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WOOD_BASKET)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10411);
			else
				_vm->_dialogs->show(10439);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TROPHY)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10412);
			else
				_vm->_dialogs->show(10441);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_READING_BENCH)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10414);
			else
				_vm->_dialogs->show(10439);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LOVESEAT)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10416);
			else
				_vm->_dialogs->show(10439);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_HALLWAY)) {
			if (_globals[kPlayerPersona] == 0)
				_vm->_dialogs->show(10421);
			else
				_vm->_dialogs->show(10434);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SECRET_DOOR)) {
			_vm->_dialogs->show(10430);
			_action._inProgress = false;
			return;
		}

		if ((_action.isObject(NOUN_DOORWAY) || _action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY) || _action.isAction(VERB_OPEN, NOUN_DOORWAY)) && (_globals[kPlayerPersona] == 1)) {
			_vm->_dialogs->show(10432);
			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_LOOK, NOUN_WALL_PANEL)) {
			if ((_globals[kBooksStatus] == 0) || (_globals[kBooksStatus] == 1))
				_vm->_dialogs->show(10435);
			else
				_vm->_dialogs->show(10436);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_KING) && _globals[kPlayerPersona] == 1) {
			_vm->_dialogs->show(10443);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MUSIC_BOX) && _globals[kNoLoadWalker]) {
			_vm->_dialogs->showItem(OBJ_MAGIC_MUSIC_BOX, 843, 0);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MACMORN) && (_globals[kPlayerPersona] == 1)) {
			_vm->_dialogs->show(10444);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TABLE)) {
			if (_globals[kPlayerPersona] == 1)
				_vm->_dialogs->show(10455);
			else if (_scene->_customDest.x < 174)
				_vm->_dialogs->show(10451);
			else
				_vm->_dialogs->show(10448);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DECORATION)) {
			if (_globals[kPlayerPersona] == 1)
				_vm->_dialogs->show(10439);
			else
				_vm->_dialogs->show(10449);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SWORD)) {
			if (_globals[kPlayerPersona] == 1)
				_vm->_dialogs->show(10439);
			else
				_vm->_dialogs->show(10450);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR) && (_globals[kPlayerPersona] == 1)) {
			_vm->_dialogs->show(10439);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL) && (_globals[kPlayerPersona] == 1)) {
			_vm->_dialogs->show(10439);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CANDLESTICK)) {
			if (_globals[kPlayerPersona] == 1)
				_vm->_dialogs->show(10439);
			else
				_vm->_dialogs->show(10461);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_QUEEN_MOTHER)) {
			_vm->_dialogs->show(10456);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MACMORN)) {
			_vm->_dialogs->show(10444);
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_RUG)) {
		if (_globals[kPlayerPersona] == 0)
			_vm->_dialogs->show(10406);
		else
			_vm->_dialogs->show(10445);

		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_FIREPLACE_SCREEN)) {
		if (_globals[kPlayerPersona] == 0)
			_vm->_dialogs->show(10408);
		else
			_vm->_dialogs->show(10445);

		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_TROPHY)) {
		if (_globals[kPlayerPersona] == 0)
			_vm->_dialogs->show(10413);
		else
			_vm->_dialogs->show(10445);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_READING_BENCH)) {
		if (_globals[kPlayerPersona] == 0)
			_vm->_dialogs->show(10415);
		else
			_vm->_dialogs->show(10445);

		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_LOVESEAT)) {
		if (_globals[kPlayerPersona] == 0)
			_vm->_dialogs->show(10417);
		else
			_vm->_dialogs->show(10445);

		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_OPEN) || _action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && (_action.isObject(NOUN_SECRET_DOOR) || _action.isObject(NOUN_WALL_PANEL))) {
		_vm->_dialogs->show(10431);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_INVOKE, NOUN_SIGNET_RING) && (_globals[kPlayerPersona] == 1)) {
		_vm->_dialogs->show(10433);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_TENTACLE_PARTS, NOUN_WALL_PANEL) && (_globals[kPlayerPersona] == 0)) {
		_vm->_dialogs->show(10446);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_SHIFT_INTO_BEAR) && _anim0ActvFl) {
		if (_wasBearFl)
			_vm->_dialogs->show(10457);
		else if (_amuletWorksFl)
			_vm->_dialogs->show(10459);
		else {
			_wasBearFl = true;
			_globals[kPlayerScore] += 2;
			_vm->_gameConv->run(1);
			_vm->_gameConv->exportValue(1);
			_vm->_gameConv->exportValue(0);
			_vm->_gameConv->exportValue(0);
			if (_globals[kLlanieStatus] != 2)
				_vm->_gameConv->exportValue(1);
			else
				_vm->_gameConv->exportValue(0);
		}
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_ATTACK, NOUN_SWORD, NOUN_MACMORN) || _action.isAction(VERB_CARVE_UP, NOUN_SWORD, NOUN_MACMORN) || _action.isAction(VERB_THRUST, NOUN_SWORD, NOUN_MACMORN) || _action.isAction(VERB_TAKE, NOUN_SWORD)) && _anim0ActvFl) {
		_activateTimerFl = false;
		_pidStatus = 6;

		if (!_amuletWorksFl) {
			_globals[kPlayerScore] += 5;
			_vm->_gameConv->run(1);
			_vm->_gameConv->exportValue(0);
			_vm->_gameConv->exportValue(1);
			_vm->_gameConv->exportValue(0);
			if (_globals[kLlanieStatus] != 2)
				_vm->_gameConv->exportValue(1);
			else
				_vm->_gameConv->exportValue(0);

			_vm->_gameConv->hold();
		} else {
			_macStatus = 6;
			_game._player._stepEnabled = false;
		}
		_amuletWorksFl = true;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_INVOKE, NOUN_AMULET) && _anim0ActvFl) {
		if (_amuletWorksFl) {
			_activateTimerFl = false;
			_pidStatus = 5;
			_globals[kPlayerScore] += 15;
			_globals[kAmuletStatus] = 2;
			_game._player._stepEnabled = false;

			_scene->freeAnimation(_globals._animationIndexes[1]);

			_globals._sequenceIndexes[14] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[14], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 6);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[14], SYNC_CLOCK, 0);

			_globals._spriteIndexes[15] = _scene->_sprites.addSprites(formAnimName('e', 3), PALFLAG_ALL_TO_CLOSEST | PALFLAG_ANY_TO_CLOSEST);
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('m', 2), 0);
			_anim1ActvFl = false;
			_anim5ActvFl = true;
		} else
			_vm->_dialogs->showItem(OBJ_AMULET, 945, 0);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_ATTACK, NOUN_SWORD, NOUN_QUEEN_MOTHER) || _action.isAction(VERB_CARVE_UP, NOUN_SWORD, NOUN_QUEEN_MOTHER) || _action.isAction(VERB_THRUST, NOUN_SWORD, NOUN_QUEEN_MOTHER)) {
		_vm->_dialogs->show(10458);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_MACMORN)) {
		_vm->_dialogs->show(10464);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_QUEEN_MOTHER)) {
		_vm->_dialogs->show(10463);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_KING)) {
		_vm->_dialogs->show(10465);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_POUR_CONTENTS_OF, NOUN_MACMORN)) {
		_vm->_dialogs->show(10462);
		_action._inProgress = false;
		return;
	}


	if (_anim2ActvFl && (_action.isAction(VERB_WALK_ACROSS) || _action.isAction(VERB_WALK_TO))) {
		_vm->_dialogs->show(10445);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CANDLESTICK)) {
		_vm->_dialogs->show(10468);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_MUSIC_BOX) && _globals[kNoLoadWalker]) {
		_vm->_dialogs->show(10470);
		_action._inProgress = false;
		return;
	}
}

void Scene104::preActions() {
	if ((_action.isAction(VERB_PULL) || _action.isAction(VERB_OPEN) || _action.isAction(VERB_CLOSE) || _action.isAction(VERB_PUSH))
	 && _action.isObject(NOUN_TAPESTRY) && (_scene->_customDest.x > 280))
		_game._player.walk(Common::Point(295, 145), FACING_NORTHEAST);

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_THRONE_ROOM)
	 || _action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_HALLWAY)
	 || _action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY)
	 || ((_action.isAction(VERB_PULL) || _action.isAction(VERB_OPEN) || _action.isAction(VERB_PUSH)) && _action.isObject(NOUN_TAPESTRY))
	 || ((_action.isAction(VERB_PULL) || _action.isAction(VERB_CLOSE) || _action.isAction(VERB_PUSH)) && _action.isObject(NOUN_TAPESTRY))
	 || ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL))
		 && (_action.isObject(NOUN_RUG) || _action.isObject(NOUN_FIREPLACE_SCREEN) || _action.isObject(NOUN_TROPHY) || _action.isObject(NOUN_LOVESEAT)))
	 || _action.isAction(VERB_OPEN, NOUN_READING_BENCH)) {
		if (_globals[kPlayerPersona] == 1)
			_game._player.cancelWalk();
	}

	if (_globals[kNoLoadWalker])
		_game._player._needToWalk = false;
}

void Scene104::handleFinalConversation() {
	bool interlocutorFl = false;
	bool heroFl = false;

	switch (_action._activeAction._verbId) {
	case 11:
		_deathTimer = 0;
		_clock = 0;
		_activateTimerFl = true;
		interlocutorFl = true;
		heroFl = true;
		break;

	case 14:
		if (!_game._trigger)
			_macStatus = 3;

		interlocutorFl = true;
		heroFl = true;
		break;

	case 17:
		_pidStatus = 3;
		_vm->_gameConv->hold();
		interlocutorFl = true;
		heroFl = true;
		break;

	case 20:
		if (!_game._trigger)
			_macStatus = 2;
		interlocutorFl = true;
		heroFl = true;
		break;

	case 30:
		_vm->_gameConv->setStartNode(31);
		_vm->_gameConv->stop();

		if (_globals[kLlanieStatus] == 2) {
			_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('l', 1), 0);
			_anim3ActvFl = true;
			_twinklesStatus = 1;
		}
		break;

	case 38:
		_globals[kEndOfGame] = true;
		_scene->_nextSceneId = 106;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 81:
		switch (_action._activeAction._verbId) {
		case 0:
		case 2:
		case 4:
		case 5:
		case 7:
		case 8:
		case 11:
		case 17:
			if (_macStatus == 0)
				_macStatus = 1;

			_pidStatus = 0;
			if (_queenStatus != 2)
				_queenStatus = 0;

			_kingStatus = 0;
			break;

		case 1:
		case 3:
		case 6:
		case 20:
		case 22:
		case 24:
		case 26:
		case 28:
		case 32:
		case 34:
		case 39:
		case 41:
		case 43:
			_kingStatus = 1;
			_pidStatus = 0;
			if (_queenStatus != 2)
				_queenStatus = 0;

			if (_macStatus == 1)
				_macStatus = 0;

			break;

		case 21:
		case 23:
		case 25:
		case 27:
		case 35:
		case 40:
		case 42:
			if (_queenStatus != 2)
				_queenStatus = 1;

			_pidStatus = 0;
			if (_macStatus == 1)
				_macStatus = 0;

			_kingStatus = 0;
			break;

		case 31:
		case 33:
			_twinklesStatus = 1;
			_pidStatus = 0;
			if (_queenStatus != 2)
				_queenStatus = 0;

			if (_macStatus == 1)
				_macStatus = 0;

			_kingStatus = 0;
			break;

		default:
			break;
		}
		break;

	case 83:
		if ((_action._activeAction._verbId == 8) || (_action._activeAction._verbId == 11))
			_pidStatus = 2;
		else
			_pidStatus = 1;

		if (_macStatus == 1)
			_macStatus = 0;

		if (_queenStatus != 2)
			_queenStatus = 0;

		_kingStatus = 0;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(83);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(81);

	_kingCount = 0;
	_queenCount = 0;
	_macCount = 0;
	_twinklesCount = 0;
	_pidCount = 0;
}

void Scene104::handleKingAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _kingFrame)
		return;

	_kingFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_kingFrame) {
	case 14:
	case 21:
		switch (_kingStatus) {
		case 0:
			if (_kingCount > _vm->getRandomNumber(40, 50)) {
				_kingCount = 0;
				if (_vm->getRandomNumber(1,2) == 1)
					resetFrame = 13;
				else
					resetFrame = 14;
			} else {
				++_kingCount;
				resetFrame = 13;
			}
			break;

		case 1:
			resetFrame = 14;
			break;

		default:
			break;
		}
		break;

	case 16:
	case 17:
	case 18:
	case 19:
		switch (_kingStatus) {
		case 0:
			if (_kingCount > _vm->getRandomNumber(40, 50)) {
				_kingCount = 0;
				if (_vm->getRandomNumber(1,2) == 1)
					resetFrame = 15;
				else
					resetFrame = 19;
			} else {
				++_kingCount;
				resetFrame = 15;
			}
			break;

		case 1:
			resetFrame = _vm->getRandomNumber(16, 18);
			++_kingCount;
			if (_kingCount > 15) {
				_kingStatus = 0;
				_kingCount = 0;
				resetFrame = 15;
			}
			break;

		default:
			break;
		}
		break;

	case 22:
		if (_kingStatus == 2)
			resetFrame = 21;
		else
			resetFrame = 0;

		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_kingFrame = resetFrame;
	}
}


void Scene104::handleMacAnimation1() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _macFrame)
		return;

	_macFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_macFrame) {
	case 1:
	case 2:
	case 7:
		switch (_macStatus) {
		case 0:
			resetFrame = 0;
			break;

		case 1:
			if (_macCount == 0) {
				if (_vm->getRandomNumber(1,2) == 1)
					resetFrame = 2;
				else
					resetFrame = 1;

				++_macCount;
			} else {
				resetFrame = _vm->getRandomNumber(0, 1);
				++_macCount;
				if (_macCount > 15) {
					_macStatus = 0;
					_macCount = 0;
					resetFrame = 0;
				}
			}
			break;

		case 2:
			resetFrame = 90;
			break;

		default:
			break;
		}
		break;

	case 3:
	case 4:
	case 5:
		switch (_macStatus) {
		case 0:
		case 2:
		case 3:
		case 4:
			resetFrame = 5;
			break;

		case 1:
			if (_macCount == 0) {
				if (_vm->getRandomNumber(1,2) == 1)
					resetFrame = 5;
				else
					resetFrame = 3;

				++_macCount;

			} else {
				resetFrame = _vm->getRandomNumber(3, 4);
				++_macCount;
				if (_macCount > 15) {
					_macStatus = 0;
					_macCount = 0;
					resetFrame = 5;
				}
			}
			break;

		default:
			break;
		}
		break;

	case 25:
		_game._player._stepEnabled = true;
		break;

	case 26:
	case 90:
	case 174:
		switch (_macStatus) {
		case 3:
			resetFrame = 76;
			_macStatus = 0;
			_game._player._stepEnabled = false;
			break;

		case 4:
			break;

		case 5:
			resetFrame = 147;
			_macStatus = 0;
			break;

		case 6:
			_pidFrame = 105;
			resetFrame = 26;
			_scene->setAnimFrame(_globals._animationIndexes[4], 105);
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[4], SYNC_ANIM, _globals._animationIndexes[1]);
			break;

		default:
			resetFrame = 25;
			break;
		}
		break;

	case 36:
		_globals._sequenceIndexes[13] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[13], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 13);
		break;

	case 37:
		_scene->setAnimFrame(_globals._animationIndexes[4], 89);
		_pidFrame = 89;
		_pidStatus = 7;
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[4], SYNC_ANIM, _globals._animationIndexes[1]);
		break;

	case 76:
		_vm->_gameConv->reset(1);
		_vm->_dialogs->show(10467);
		_globals[kNoLoadWalker] = false;
		_scene->_nextSceneId = 119;
		break;

	case 82:
		_pidStatus = 4;
		break;

	case 97:
		_activateTimerFl = true;
		_scene->deleteSequence(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 10, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 85);
		break;

	case 112:
		resetFrame = 8;
		_macStatus = 0;
		break;

	case 162:
		_vm->_gameConv->release();
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_macFrame = resetFrame;
	}
}


void Scene104::handleMacAnimation2() {

	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _macFrame)
		return;

	_macFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_macFrame) {
	case 1:
		if (_macStatus != 4)
			resetFrame = 0;
		break;

	case 2:
		_scene->deleteSequence(_globals._sequenceIndexes[14]);
		_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[14], SYNC_ANIM, _globals._animationIndexes[1]);
		break;

	case 37:
		_scene->freeAnimation(_globals._animationIndexes[1]);
		_scene->_sprites.remove(_globals._spriteIndexes[15]);
		_anim5ActvFl = false;
		resetFrame = -1;

		_vm->_dialogs->show(10426);
		_vm->_sound->command(45);
		_vm->_gameConv->run(1);
		_vm->_gameConv->exportValue(0);
		_vm->_gameConv->exportValue(0);
		_vm->_gameConv->exportValue(1);
		if (_globals[kLlanieStatus] != 2)
			_vm->_gameConv->exportValue(1);
		else
			_vm->_gameConv->exportValue(0);
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_macFrame = resetFrame;
	}
}

void Scene104::handleQueenAnimation() {
	if (_scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame() == _queenFrame)
		return;

	_queenFrame = _scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_queenFrame) {
	case 1:
	case 8:
	case 15:
	case 24:
		switch (_queenStatus) {
		case 0:
			if (_queenCount > _vm->getRandomNumber(40, 50)) {
				_queenCount = 0;
				if (_vm->getRandomNumber(1,2) == 1)
					resetFrame = 0;
				else
					resetFrame = 8;
			} else {
				++_queenCount;
				resetFrame = 0;
			}
			break;

		case 1:
			if (_vm->getRandomNumber(1,2) == 1)
				resetFrame = 1;
			else
				resetFrame = 15;

			_queenStatus = 0;
			break;

		case 2:
			resetFrame = 8;
			break;

		default:
			break;
		}
		break;

	case 12:
	case 29:
		switch (_queenStatus) {
		case 0:
			if (_queenCount > _vm->getRandomNumber(40, 50)) {
				_queenCount = 0;
				if (_vm->getRandomNumber(1,2) == 1)
					resetFrame = 11;
				else
					resetFrame = 12;
			} else {
				++_queenCount;
				resetFrame = 11;
			}
			break;

		case 1:
			resetFrame = 12;
			break;

		case 2:
			resetFrame = 24;
			break;

		default:
			break;
		}
		break;

	case 27:
		switch (_queenStatus) {
		case 0:
		case 1:
			resetFrame = 27;
			break;

		case 2:
			resetFrame = 26;
			break;

		case 3:
			resetFrame = 27;
			_queenStatus = 0;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[2], resetFrame);
		_queenFrame = resetFrame;
	}
}

void Scene104::handleTwinklesAnimation() {
	if (_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == _twinklesFrame)
		return;

	_twinklesFrame = _scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_twinklesFrame) {
	case 1:
		if (_twinklesStatus == 2)
			resetFrame = 0;
		break;

	case 10:
		_queenStatus = 2;
		break;

	case 28:
		_vm->_gameConv->run(1);
		_vm->_gameConv->exportValue(0);
		_vm->_gameConv->exportValue(0);
		_vm->_gameConv->exportValue(1);
		if (_globals[kLlanieStatus] != 2)
			_vm->_gameConv->exportValue(1);
		else
			_vm->_gameConv->exportValue(0);
		break;

	case 36:
		_queenStatus = 3;
		break;

	case 37:
	case 38:
	case 47:
		switch (_twinklesStatus) {
		case 0:
			if (_twinklesFrame != 37 && _twinklesFrame != 38)
				_twinklesFrame = 38;

			if (_twinklesCount > _vm->getRandomNumber(40, 50)) {
				_twinklesCount = 0;
				if (_vm->getRandomNumber(1, 2) == 1)
					resetFrame = 36;
				else
					resetFrame = 37;
			} else {
				++_twinklesCount;
				resetFrame = _twinklesFrame - 1;
			}
			break;

		case 1:
			resetFrame = 38;
			_twinklesStatus = 0;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[3], resetFrame);
		_twinklesFrame = resetFrame;
	}
}

void Scene104::handleDeathAnimation() {
	if (_scene->_animation[_globals._animationIndexes[4]]->getCurrentFrame() == _deathFrame)
		return;

	_deathFrame = _scene->_animation[_globals._animationIndexes[4]]->getCurrentFrame();

	if (_deathFrame == 11)
		_scene->playSpeech(7);
	else if (_deathFrame == 17)
		_scene->playSpeech(6);
}

void Scene104::handlePidAnimation() {
	if (_scene->_animation[_globals._animationIndexes[4]]->getCurrentFrame() == _pidFrame)
		return;

	_pidFrame = _scene->_animation[_globals._animationIndexes[4]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_pidFrame) {
	case 1:
	case 2:
	case 3:
	case 9:
	case 47:
	case 81:
		switch (_pidStatus) {
		case 0:
			resetFrame = 0;
			break;

		case 1:
			resetFrame = _vm->getRandomNumber(0, 2);
			++_pidCount;
			if (_pidCount > 20) {
				_pidStatus = 0;
				_pidCount = 0;
				resetFrame = 0;
			}
			break;

		case 2:
			resetFrame = 3;
			break;

		case 3:
			_activateTimerFl = false;
			resetFrame = 10;
			break;

		case 5:
			_game._player._stepEnabled = false;
			_activateTimerFl = false;
			resetFrame = 47;
			_pidStatus = 0;
			break;

		case 6:
			resetFrame = 81;
			break;

		case 8:
			_scene->freeAnimation(_globals._animationIndexes[1]);
			_scene->freeAnimation(_globals._animationIndexes[4]);
			_anim1ActvFl = false;
			_anim4ActvFl = false;
			_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('d', 2), 95);
			_anim6ActvFl = true;
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[4], SYNC_CLOCK, 0);
			break;

		default:
			break;
		}
		break;

	case 4:
	case 5:
	case 6:
	case 7:
		if (_pidStatus == 2) {
			resetFrame = _vm->getRandomNumber(4, 6);
			++_pidCount;
			if (_pidCount > 20) {
				_pidStatus = 0;
				_pidCount = 0;
				resetFrame = 7;
			}
		} else
			resetFrame = 7;
		break;

	case 24:
		_vm->_gameConv->release();
		break;

	case 25:
		if (_pidStatus == 4) {
			resetFrame = 25;
			_pidStatus = 0;
		} else
			resetFrame = 24;
		break;

	case 46:
		_activateTimerFl = true;
		_clock = 0;
		_deathTimer = 0;
		_game._player._stepEnabled = true;
		break;

	case 60:
		resetFrame = 107;
		break;

	case 62:
		_macStatus = 4;
		break;

	case 88:
		if (_deathTimer < 1300)
			_macStatus = 5;
		break;

	case 89:
		if (_pidStatus == 5) {
			resetFrame = 55;
			_pidStatus = 0;
		} else if (_deathTimer >= 1300)
			_macStatus = 6;
		else {
			_pidDrawnSword = true;
			resetFrame = 88;
		}
		break;

	case 91:
		_scene->playSpeech(7);
		break;

	case 95:
		_scene->playSpeech(6);
		break;

	case 105:
		resetFrame = 104;
		break;

	case 106:
		if (_pidStatus == 7)
			resetFrame = 89;
		else
			resetFrame = 105;
		break;

	case 111:
		resetFrame = 60;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[4], resetFrame);
		_pidFrame = resetFrame;
	}
}
/*------------------------------------------------------------------------*/

Scene105::Scene105(MADSEngine *vm) : Scene1xx(vm) {
	_maidTalkingFl = false;
	_sitUpFl = false;
	_goodNumberFl = false;

	_maidFrame = -1;
	_newStatus = -1;
	_previousStatus = -1;
	_maidHotspotId1 = -1;
	_maidHotspotId2 = -1;
	_bucketHotspotId = -1;
	_boneHotspotId = -1;
	_gobletHotspotId = -1;
}

void Scene105::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_maidTalkingFl);
	s.syncAsByte(_sitUpFl);
	s.syncAsByte(_goodNumberFl);

	s.syncAsSint16LE(_maidFrame);
	s.syncAsSint16LE(_newStatus);
	s.syncAsSint16LE(_previousStatus);
	s.syncAsSint16LE(_maidHotspotId1);
	s.syncAsSint16LE(_maidHotspotId2);
	s.syncAsSint16LE(_bucketHotspotId);
	s.syncAsSint16LE(_boneHotspotId);
	s.syncAsSint16LE(_gobletHotspotId);
}

void Scene105::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_SCULLERY_MAID);
	_scene->addActiveVocab(NOUN_BUCKET);
	_scene->addActiveVocab(VERB_WALK_TO);
	_scene->addActiveVocab(NOUN_BONE);
	_scene->addActiveVocab(NOUN_GOBLET);
}

void Scene105::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*KGRD_8");

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 0, 3);

	_maidHotspotId1 = _scene->_dynamicHotspots.add(NOUN_SCULLERY_MAID, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(189, 123, 189 + 25, 123 + 11));
	_scene->_dynamicHotspots.setPosition(_maidHotspotId1, Common::Point(174, 151), FACING_NORTHEAST);
	_scene->_dynamicHotspots[_maidHotspotId1]._articleNumber = PREP_ON;

	_maidHotspotId2 = _scene->_dynamicHotspots.add(NOUN_SCULLERY_MAID, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(189, 134, 189 + 38, 134 + 9));
	_scene->_dynamicHotspots.setPosition(_maidHotspotId2, Common::Point(174, 151), FACING_NORTHEAST);
	_scene->_dynamicHotspots[_maidHotspotId2]._articleNumber = PREP_ON;

	_bucketHotspotId = _scene->_dynamicHotspots.add(NOUN_BUCKET, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(176, 137, 176 + 10, 137 + 8));
	_scene->_dynamicHotspots.setPosition(_bucketHotspotId, Common::Point(174, 151), FACING_NORTHEAST);

	if (_game._objects.isInRoom(OBJ_GOBLET)) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('p', 1));
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*KGRM_6");
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_gobletHotspotId = _scene->_dynamicHotspots.add(NOUN_GOBLET, VERB_WALK_TO, SYNTAX_SINGULAR, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 6);
		_scene->_dynamicHotspots.setPosition(_gobletHotspotId, Common::Point(63, 142), FACING_WEST);
	}

	if (_game._objects.isInRoom(OBJ_BONE)) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('p', 0));
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*KGRL_6");
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, -1);
		_boneHotspotId = _scene->_dynamicHotspots.add(NOUN_BONE, VERB_WALK_TO, SYNTAX_SINGULAR, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
		_scene->_dynamicHotspots.setPosition(_boneHotspotId, Common::Point(255, 145), FACING_EAST);
	}

	_vm->_gameConv->load(2);
	_newStatus = 1;
	_previousStatus = 0;
	_maidTalkingFl = false;
	_sitUpFl = false;
	_goodNumberFl = false;
	_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('A',-1), 1);


	if (_scene->_priorSceneId == 106) {
		_game._player._playerPos = Common::Point(104, 152);
		_game._player._facing = FACING_NORTHEAST;
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
	} else if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
		_game._player.firstWalk(Common::Point(120, 96), FACING_SOUTH, Common::Point(117, 108), FACING_SOUTH, false);
		_game._player.setWalkTrigger(70);
	} else {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);
	}

	sceneEntrySound();
}

void Scene105::step() {
	if (_scene->_animation[_globals._animationIndexes[0]]) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() != _maidFrame) {
			_maidFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
			int resetFrame = -1;

			if (_sitUpFl && !_maidTalkingFl) {
				_previousStatus = _newStatus;
				_newStatus = 4;
			}

			if (_maidTalkingFl) {
				_previousStatus = _newStatus;
				_newStatus = 5;
			}

			switch (_maidFrame) {
			case 16:
				if (_newStatus == 1) {
					_goodNumberFl = false;
					while (!_goodNumberFl)
						setRandomStatus();
				}

				if (_newStatus == 1)
					resetFrame = 1;
				else if ((_newStatus == 3 && !_sitUpFl) || _newStatus == 4)
					resetFrame = 53;
				else if ((_newStatus == 3) && (_sitUpFl))
					resetFrame = 24;
				break;

			case 24:
				if (_newStatus == 2) {
					_goodNumberFl = false;
					while (!_goodNumberFl)
						setRandomStatus();
				}

				if ((_newStatus == 3) || (_newStatus == 5))
					resetFrame = 53;
				else if (_newStatus == 1)
					resetFrame = 56;
				else if (_newStatus == 4)
					resetFrame = 53;

				break;

			case 32:
				if (_newStatus == 3) {
					_goodNumberFl = false;
					while (!_goodNumberFl)
						setRandomStatus();
				}

				if ((_newStatus == 1) || (_newStatus == 2))
					resetFrame = 57;
				else if (_newStatus == 4)
					resetFrame = 55;
				break;

			case 53:
				_maidTalkingFl = false;
				_goodNumberFl = false;
				while (!_goodNumberFl)
					setRandomStatus();

				if (_newStatus == 1)
					resetFrame = 60;
				else if (_newStatus == 2)
					resetFrame = 57;
				else if (_newStatus == 3)
					resetFrame = 63;
				else if (_newStatus == 4)
					resetFrame = 55;
				break;

			case 56:
				if (_newStatus == 4) {
					_goodNumberFl = false;
					while (!_goodNumberFl)
						setRandomWipebrow();
				}

				if (_newStatus == 4)
					resetFrame = 55;
				else if (_newStatus == 5)
					resetFrame = 32;
				else if (_newStatus == 3)
					resetFrame = 24;
				break;

			case 57:
				if (_newStatus == 1)
					resetFrame = 1;
				break;

			case 60:
				if (_newStatus == 1)
					resetFrame = 1;
				else if (_newStatus == 2)
					resetFrame = 16;
				break;

			case 63:
				if (_newStatus == 1)
					resetFrame = 1;
				break;

			case 64:
				resetFrame = 24;
				break;

			default:
				break;
			}

			if ((resetFrame >= 0) && (resetFrame != _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame())) {
				_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
				_maidFrame = resetFrame;
			}
		}
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_vm->_sound->command(25);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 6, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 71: {
			int seqIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_SEQ, seqIdx);
			_game._player._stepEnabled = true;
			}
			break;

		default:
			break;
		}
	}
}

void Scene105::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10501);
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 2) {
		handleConversation();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_HALLWAY) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_HALLWAY) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_HALLWAY)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 8, 2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[4],true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_vm->_sound->command(24);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2: {
			int seqIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_SEQ, seqIdx);
			}
			break;

		case 3:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[4]);
			_scene->_sequences.addTimer(1,4);
			break;

		case 4:
			_game._player.walk(Common::Point(120, 96), FACING_NORTH);
			_game._player.setWalkTrigger(5);
			break;

		case 5:
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_vm->_sound->command(25);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6: {
			int seqIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 1);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_SEQ, seqIdx);
			_scene->_sequences.addTimer(6, 7);
			}
			break;

		case 7:
			_scene->_nextSceneId = 103;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_THRONE_ROOM) || _action.isAction(VERB_OPEN, NOUN_DOOR_TO_THRONE_ROOM) || _action.isAction(VERB_PULL, NOUN_DOOR_TO_THRONE_ROOM)) {
		_scene->_nextSceneId = 106;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_SCULLERY_MAID)) {
		_vm->_gameConv->run(2);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_GOBLET)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_GOBLET)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 6, 2);
				_scene->_sequences.setAnimRange (_globals._sequenceIndexes[8], 1, 7);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 7, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_scene->deleteSequence (_globals._sequenceIndexes[5]);
				_scene->_dynamicHotspots.remove(_gobletHotspotId);
				_vm->_sound->command(26);
				_game._objects.addToInventory(OBJ_GOBLET);
				_vm->_dialogs->showItem(OBJ_GOBLET, 10519, 0);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
				_globals[kPlayerScore] += 1;
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

	if (_action.isAction(VERB_TAKE, NOUN_BONE) && _action._mainObjectSource == CAT_HOTSPOT) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_BONE)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], false, 6, 2);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[7], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_scene->deleteSequence (_globals._sequenceIndexes[6]);
				_vm->_sound->command(26);
				_scene->_dynamicHotspots.remove(_boneHotspotId);
				_game._objects.addToInventory(OBJ_BONE);
				_vm->_dialogs->showItem(OBJ_BONE, 10520, 0);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[7]);
				if (!(_globals[kPlayerScoreFlags] & 2048)) {
					_globals[kPlayerScoreFlags] |= 2048;
					_globals[kPlayerScore] += 1;
				}
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

	if (_action.isAction(VERB_TAKE, NOUN_BONE) && _game._objects.isInInventory(OBJ_BONE) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		_vm->_dialogs->show(40112);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(10502);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BRAZIER)) {
			_vm->_dialogs->show(10503);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DINING_TABLE)) {
			if (_scene->_customDest.x <= 98) {
				if (_game._objects[OBJ_GOBLET]._roomNumber == _scene->_currentSceneId)
					_vm->_dialogs->show(10505);
				else
					_vm->_dialogs->show(10522);
			} else
				_vm->_dialogs->show(10504);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHAIR)) {
			_vm->_dialogs->show(10507);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WINDOW)) {
			_vm->_dialogs->show(10508);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TAPESTRY)) {
			_vm->_dialogs->show(10509);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_THRONE_ROOM)) {
			_vm->_dialogs->show(10511);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10512);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_HALLWAY)) {
			_vm->_dialogs->show(10513);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DIVIDING_WALL)) {
			_vm->_dialogs->show(10514);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(10515);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SCULLERY_MAID)) {
			_vm->_dialogs->show(10516);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GOBLET) && _game._objects.isInRoom(OBJ_GOBLET)) {
			_vm->_dialogs->show(10517);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BONE) && _game._objects.isInRoom(OBJ_BONE)) {
			_vm->_dialogs->show(10518);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SCONCE)) {
			_vm->_dialogs->show(10524);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BUCKET)) {
			_vm->_dialogs->show(10521);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_SCULLERY_MAID)) {
		_vm->_dialogs->show(30);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_DINING_TABLE)) {
		_vm->_dialogs->show(10506);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_TAPESTRY)) {
		_vm->_dialogs->show(10510);
		_action._inProgress = false;
		return;
	}
}

void Scene105::preActions() {
	if (_action.isAction(VERB_TALK_TO, NOUN_SCULLERY_MAID))
		_sitUpFl = true;
}

void Scene105::setRandomStatus() {
	_previousStatus = _newStatus;
	int rndVal = _vm->getRandomNumber(1, 30);
	if (rndVal < 20) {
		_newStatus = 1;
		_goodNumberFl = true;
	} else if ((rndVal > 19) && (rndVal < 27) && (_previousStatus != 2)) {
		_newStatus = 2;
		_goodNumberFl = true;
	} else if (_previousStatus != 3) {
		_newStatus = 3;
		_goodNumberFl = true;
	}
}

void Scene105::setRandomWipebrow() {
	_previousStatus = _newStatus;

	if (_vm->getRandomNumber(1, 100) < 100) {
		_newStatus = 4;
		_goodNumberFl = true;
	} else if (_previousStatus != 3) {
		_newStatus = 3;
		_goodNumberFl = true;
	}
}

void Scene105::handleConversation() {
	if ((_action._activeAction._verbId == 0) && !_sitUpFl)
		_sitUpFl = true;

	if ((_action._activeAction._verbId == 1) || (_action._activeAction._verbId == 2)) {
		switch (_game._trigger) {
		case 0:
			_vm->_gameConv->setInterlocutorTrigger(1);
			break;

		case 1:
			_maidTalkingFl = true;
			_sitUpFl = false;
			_vm->_gameConv->setHeroTrigger(2);
			break;

		case 2:
			_maidTalkingFl = false;
			break;

		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Dragonsphere
} // End of namespace MADS
