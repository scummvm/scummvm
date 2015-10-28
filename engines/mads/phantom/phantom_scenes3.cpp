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
#include "mads/phantom/phantom_scenes3.h"

namespace MADS {

namespace Phantom {

void Scene3xx::setAAName() {
	_game._aaName = Resources::formatAAName(_globals[kTempInterface]);
	_vm->_palette->setEntry(254, 43, 47, 51);
}

void Scene3xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	switch (_scene->_nextSceneId) {
	case 303:
	case 304:
	case 305:
	case 307:
	case 308:
		if (_globals[kKnockedOverHead])
			_vm->_sound->command(33);
		else
			_vm->_sound->command(16);
		break;

	case 310:
	case 320:
	case 330:
	case 340:
		_vm->_sound->command(36);
		break;

	default:
		if (_scene->_nextSceneId != 306)
			_vm->_sound->command(16);
		break;
	}
}

void Scene3xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	if ((_scene->_nextSceneId == 304) || (_scene->_nextSceneId == 305) || (_scene->_nextSceneId == 306) || (_scene->_nextSceneId == 310))
		_game._player._spritesPrefix = "";
	else {
		Common::String oldName = _game._player._spritesPrefix;
		if (!_game._player._forcePrefix)
			_game._player._spritesPrefix = "RAL";
		if (oldName != _game._player._spritesPrefix)
			_game._player._spritesChanged = true;
	}

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

Scene301::Scene301(MADSEngine *vm) : Scene3xx(vm) {
	_anim0ActvFl = false;
	_skip1Fl = false;
	_skip2Fl = false;

	_lightingHotspotId = -1;
	_sandbagHotspotId = -1;
}

void Scene301::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_skip1Fl);
	s.syncAsByte(_skip2Fl);

	s.syncAsSint16LE(_lightingHotspotId);
	s.syncAsSint16LE(_sandbagHotspotId);
}

void Scene301::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene301::enter() {
	_vm->_disableFastwalk = true;

	_anim0ActvFl = false;
	_skip2Fl = false;
	_skip1Fl = false;

	_scene->loadSpeech(6);

	_scene->_hotspots.activate(NOUN_CABLE, false);
	_scene->_hotspots.activate(NOUN_STOOL, false);

	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 0), false);

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 1), false);
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', 0), false);
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('z', 1), false);
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('z', 2), false);
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('z', 3), false);
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('z', 4), false);
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('z', 5), false);
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('z', 6), false);

		_lightingHotspotId = _scene->_dynamicHotspots.add(NOUN_LIGHTING_INSTRUMENT, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(50, 116, 50 + 19, 116 + 10));
		_scene->_dynamicHotspots.setPosition(_lightingHotspotId, Common::Point(67, 129), FACING_NORTHWEST);
		_lightingHotspotId = _scene->_dynamicHotspots.add(NOUN_LIGHTING_INSTRUMENT, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(126, 118, 126 + 29, 118 + 8));
		_scene->_dynamicHotspots.setPosition(_lightingHotspotId, Common::Point(152, 129), FACING_NORTHWEST);

		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);

		_scene->_hotspots.activate(NOUN_CABLE, true);
	} else {
		_sandbagHotspotId = _scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(464, 114, 464 + 11, 114 + 9));
		_scene->_dynamicHotspots.setPosition(_sandbagHotspotId, Common::Point(475, 125), FACING_NORTHWEST);
		_scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_LOOK_AT, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(468, 42, 468 + 6, 42 + 13));
		_scene->_hotspots.activate(NOUN_STOOL, true);
		_scene->_hotspots.activate(NOUN_BIG_PROP, false);
	}

	if (_scene->_priorSceneId == 302) {
		_game._player.firstWalk(Common::Point(-20, 132), FACING_WEST, Common::Point(19, 132), FACING_EAST, true);
		_scene->setCamera(Common::Point(0, 0));
	} else if ((_scene->_priorSceneId == 106) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(568, 133);
		_game._player._facing = FACING_WEST;
		_scene->setCamera(Common::Point(320, 0));
	}

	if (!_game._visitedScenes._sceneRevisited) {
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('x', 0), false);
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 7, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], -1, -2);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[9], 0, 0, 64);
	}

	sceneEntrySound();
}

void Scene301::step() {
	if ((_globals[kDoneBrieConv203] == 3) && (_scene->_posAdjust.x < 320) && (_game._player._playerPos.x < 350)) {
		_game._player.cancelCommand();
		_game._player.walk(Common::Point(256, 130), FACING_NORTHWEST);
		_game._player.setWalkTrigger(60);
		_globals[kDoneBrieConv203] = 0;
		_game._player._stepEnabled = false;
	}

	if (_game._trigger == 60) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('p', 1), 0);
		_anim0ActvFl = true;
		_game._player._visible = false;
		_globals[kPlayerScore] += 10;
		_game.syncTimers(3, _globals._animationIndexes[0], 2, 0);
	}

	if (_anim0ActvFl && !_skip1Fl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 50) {
			_scene->playSpeech(6);
			_skip1Fl = true;
		}
	}

	if (_anim0ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 61) {
			_scene->setAnimFrame(_globals._animationIndexes[0], 60);
			if (!_skip2Fl) {
				_vm->_sound->command(1);
				_scene->_sequences.setTimingTrigger(420, 70);
				_skip2Fl = true;
			}
		}
	}

	if (_game._trigger == 70) {
		_scene->_userInterface.noInventoryAnim();
		// CHECKME: Not sure about the next function call
		_scene->_userInterface.refresh();
		_scene->_nextSceneId = 104;
	}

	if (_game._trigger == 64)
		_scene->_sequences.setTimingTrigger(60, 65);

	if (_game._trigger == 65)
		_vm->_dialogs->show(30137);
}

void Scene301::actions() {
	if (_action.isAction(VERB_CLIMB_DOWN, NOUN_CIRCULAR_STAIRCASE)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 1);
			_game.syncTimers(3, _globals._animationIndexes[0], 2, 0);
			break;

		case 1:
			_scene->_nextSceneId = 106;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(30110);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_GRID)) {
			_vm->_dialogs->show(30111);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CATWALK)) {
			_vm->_dialogs->show(30112);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SIDE_WALL)) {
			_vm->_dialogs->show(30113);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BACK_WALL)) {
			_vm->_dialogs->show(30114);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SUPPORT)) {
			_vm->_dialogs->show(30115);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ACT_CURTAIN)) {
			_vm->_dialogs->show(30116);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOUSE)) {
			_vm->_dialogs->show(30117);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_OTHER_CATWALK)) {
			_vm->_dialogs->show(30118);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRIDWORK)) {
			_vm->_dialogs->show(30119);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BEAM_POSITION)) {
			_vm->_dialogs->show(30120);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHTING_INSTRUMENT)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30121);
			else
				_vm->_dialogs->show(30122);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TARP)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30123);
			else
				_vm->_dialogs->show(30140);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COUNTERWEIGHT_SYSTEM)) {
			_vm->_dialogs->show(30124);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SANDBAG) && (_action._mainObjectSource == CAT_HOTSPOT)) {
			_vm->_dialogs->show(30125);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BATTEN)) {
			_vm->_dialogs->show(30126);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STOOL)) {
			_vm->_dialogs->show(30127);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HEMP)) {
			_vm->_dialogs->show(30128);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CIRCULAR_STAIRCASE)) {
			_vm->_dialogs->show(30129);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CATWALK_OVER_HOUSE)) {
			_vm->_dialogs->show(30130);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAIRCASE_POST)) {
			_vm->_dialogs->show(30131);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RAILING)) {
			_vm->_dialogs->show(30132);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CYCLORAMA)) {
			_vm->_dialogs->show(30133);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BIG_PROP)) {
			_vm->_dialogs->show(30134);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROSCENIUM_ARCH)) {
			_vm->_dialogs->show(30135);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CABLE)) {
			_vm->_dialogs->show(30136);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_HEMP)) {
		_vm->_dialogs->show(30138);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PULL, NOUN_HEMP)) {
		_vm->_dialogs->show(30141);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SANDBAG) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		_vm->_dialogs->show(30139);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_TARP)) {
		_vm->_dialogs->show(30142);
		_action._inProgress = false;
		return;
	}
}

void Scene301::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_CATWALK_OVER_HOUSE))
		_game._player._walkOffScreenSceneId = 302;
}

/*------------------------------------------------------------------------*/

Scene302::Scene302(MADSEngine *vm) : Scene3xx(vm) {
}

void Scene302::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);
}

void Scene302::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene302::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', 0), false);
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RRD_9", false);

	if (_game._objects.isInRoom(OBJ_BLUE_FRAME)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else
		_scene->_hotspots.activate(NOUN_BLUE_FRAME, false);

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1), false);
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
	}

	if (_scene->_priorSceneId == 303)
		_game._player.firstWalk(Common::Point(-20, 134), FACING_WEST, Common::Point(15, 134), FACING_EAST, true);
	else if ((_scene->_priorSceneId == 301) || (_scene->_priorSceneId != RETURNING_FROM_LOADING))
		_game._player.firstWalk(Common::Point(340, 134), FACING_WEST, Common::Point(297, 134), FACING_WEST, true);

	sceneEntrySound();
}

void Scene302::step() {
}

void Scene302::actions() {
	if (_action.isAction(VERB_TAKE, NOUN_BLUE_FRAME) && (_game._objects.isInRoom(OBJ_BLUE_FRAME) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			if (_globals[kCurrentYear] == 1881) {
				int count = 0;
				if (_game._objects.isInInventory(OBJ_YELLOW_FRAME))
					++count;
				if (_game._objects.isInInventory(OBJ_RED_FRAME))
					++count;
				if (_game._objects.isInInventory(OBJ_GREEN_FRAME))
					++count;
				if (count < 3)
					_globals[kPlayerScore] += 5;
			}

			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], true, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 5);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 2, 5, 1);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 2);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[2]);
			_scene->_hotspots.activate(NOUN_BLUE_FRAME, false);
			_game._objects.addToInventory(OBJ_BLUE_FRAME);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(2, 0, 1, _globals._sequenceIndexes[1]);
			_game._player._visible = true;
			_scene->_sequences.setTimingTrigger(20, 3);
			break;

		case 3:
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 844, 0);
			else
				_vm->_dialogs->showItem(OBJ_BLUE_FRAME, 817, 0);

			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(30210);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_CATWALK)) {
			_vm->_dialogs->show(30211);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRID)) {
			_vm->_dialogs->show(30212);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HEMP)) {
			_vm->_dialogs->show(30213);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SIDE_WALL)) {
			_vm->_dialogs->show(30214);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CATWALK)) {
			_vm->_dialogs->show(30215);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RAILING)) {
			_vm->_dialogs->show(30216);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BEAM_POSITION)) {
			_vm->_dialogs->show(30217);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHTING_INSTRUMENT)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30218);
			else
				_vm->_dialogs->show(30219);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BLUE_FRAME) && _game._objects.isInRoom(OBJ_BLUE_FRAME)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30220);
			else
				_vm->_dialogs->show(30221);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SUPPORT)) {
			_vm->_dialogs->show(30222);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_OTHER_CATWALK)) {
			_vm->_dialogs->show(30223);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_HEMP)) {
		_vm->_dialogs->show(30224);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PULL, NOUN_HEMP)) {
		_vm->_dialogs->show(30141);
		_action._inProgress = false;
		return;
	}
}

void Scene302::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_CATWALK)) {
		if (_scene->_customDest.x > 160)
			_game._player._walkOffScreenSceneId = 301;
		else
			_game._player._walkOffScreenSceneId = 303;
	}
}

/*------------------------------------------------------------------------*/

Scene303::Scene303(MADSEngine *vm) : Scene3xx(vm) {
	_anim0ActvFl = false;
	_hempHotspotId = -1;
	_skipFrameCheckFl = -1;
}

void Scene303::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsSint16LE(_hempHotspotId);
	s.syncAsSint16LE(_skipFrameCheckFl);
}

void Scene303::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_CHANDELIER_CABLE);
	_scene->addActiveVocab(VERB_CLIMB_DOWN);
}

void Scene303::enter() {
	_anim0ActvFl = false;
	_skipFrameCheckFl = false;

	if (_globals[kRightDoorIsOpen504])
		_vm->_gameConv->get(26);

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1), false);
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', 0), false);
	}

	if ((_game._objects.isInRoom(OBJ_LARGE_NOTE)) && (_globals[kCurrentYear] == 1993)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
	} else
		_scene->_hotspots.activate(NOUN_LARGE_NOTE, false);

	if (_globals[kCurrentYear] == 1993)
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
	else {
		_hempHotspotId = _scene->_dynamicHotspots.add(NOUN_CHANDELIER_CABLE, VERB_CLIMB_DOWN, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(74, 92, 74 + 7, 92 + 12));
		_scene->_dynamicHotspots.setPosition(_hempHotspotId, Common::Point(95, 107), FACING_NORTHWEST);
		_scene->_dynamicHotspots[_hempHotspotId]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setCursor(_hempHotspotId, CURSOR_GO_DOWN);
	}

	if (_scene->_priorSceneId == 307)
		_game._player.firstWalk(Common::Point(-20, 135), FACING_EAST, Common::Point(16, 135), FACING_EAST, true);
	else if (_scene->_priorSceneId == 304) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('u', 1), 60);
		_game._player._stepEnabled = false;
		_game._player._visible = false;
	} else if (_scene->_priorSceneId == 305) {
		_game._objects.addToInventory(OBJ_SWORD);
		_game._player._playerPos = Common::Point(117, 92);
		_game._player._facing = FACING_SOUTHWEST;
	} else if ((_scene->_priorSceneId == 302) || (_scene->_priorSceneId != RETURNING_FROM_LOADING))
		_game._player.firstWalk(Common::Point(340, 136), FACING_WEST, Common::Point(303, 136), FACING_WEST, true);

	_scene->_rails.disableLine(5, 9);
	_scene->_rails.disableLine(5, 12);
	_scene->_rails.disableLine(5, 8);
	_scene->_rails.disableLine(6, 3);
	_scene->_rails.disableLine(6, 2);
	_scene->_rails.disableLine(11, 3);
	_scene->_rails.disableLine(11, 4);
	_scene->_rails.disableLine(10, 2);
	_scene->_rails.disableLine(4, 9);
	_scene->_rails.disableLine(8, 0);

	sceneEntrySound();
}

void Scene303::step() {
	if (_game._trigger == 60) {
		_game._player._playerPos = Common::Point(110, 95);
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game.syncTimers(2, 0, 3, _globals._animationIndexes[0]);
		_game._player.resetFacing(FACING_SOUTHWEST);
	}

	if (_anim0ActvFl) {
		if ((_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() >= 6) && !_skipFrameCheckFl) {
			_skipFrameCheckFl = true;
			_scene->deleteSequence(_globals._sequenceIndexes[2]);
			_scene->_hotspots.activate(NOUN_LARGE_NOTE, false);
			_game._objects.addToInventory(OBJ_LARGE_NOTE);
			_vm->_sound->command(26);
		}
	}
}

void Scene303::actions() {
	if ((_action.isAction(VERB_TAKE, NOUN_LARGE_NOTE) && _game._objects.isInRoom(OBJ_LARGE_NOTE)) || ((_game._trigger > 0) && _game._trigger < 3)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('n', 1), 1);
			_anim0ActvFl = true;
			_game.syncTimers(3, _globals._animationIndexes[0], 2, 0);
			_globals[kPlayerScore] += 5;
			break;

		case 1:
			_anim0ActvFl = false;
			_game._player._visible = true;
			_game.syncTimers(2, 0, 3, _globals._animationIndexes[0]);
			_scene->_sequences.setTimingTrigger(20, 2);
			break;

		case 2:
			_vm->_dialogs->showItem(OBJ_LARGE_NOTE, 818, 7);
			_game._player._stepEnabled = true;
			break;
		}
		goto handled;
	}

	if (_action.isAction(VERB_CLIMB_INTO, NOUN_HOLE) || _action.isAction(VERB_CLIMB_DOWN, NOUN_CHANDELIER_CABLE)) {
		if (_globals[kCurrentYear] == 1881) {
			switch (_game._trigger) {
			case 0:
				if (_globals[kRightDoorIsOpen504])
					_vm->_dialogs->show(30331);

				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 3);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_game.syncTimers(3, _globals._animationIndexes[0], 2, 0);
				break;

			case 3:
				_scene->_nextSceneId = 304;
				break;

			default:
				break;
			}
		} else
			_vm->_dialogs->show(30325);

		goto handled;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_CATWALK)) {
		if (_globals[kRightDoorIsOpen504]) {
			if (_vm->_sound->_preferRoland)
				_vm->_sound->command(74);
			else
				_scene->playSpeech(1);

			_vm->_gameConv->run(26);
			_vm->_gameConv->exportValue(4);
			goto handled;
		}
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(30310);
		goto handled;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		// CHECKME: That's illogical, the check is always false... Should be out of the big 'look' check
		// It looks to me like an original bug
		if (_action.isAction(VERB_EXIT_TO, NOUN_CATWALK)) {
			_vm->_dialogs->show(30316);
			goto handled;
		}

		if (_action.isObject(NOUN_CATWALK)) {
			_vm->_dialogs->show(30311);
			goto handled;
		}

		if (_action.isObject(NOUN_GRID)) {
			_vm->_dialogs->show(30312);
			goto handled;
		}

		if (_action.isObject(NOUN_CHANDELIER_CABLE)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30317);
			else if (_globals[kRightDoorIsOpen504])
				_vm->_dialogs->show(30330);
			else
				_vm->_dialogs->show(30329);

			goto handled;
		}

		if (_action.isObject(NOUN_HEMP)) {
			_vm->_dialogs->show(30313);
			goto handled;
		}

		if (_action.isObject(NOUN_BACK_WALL)) {
			_vm->_dialogs->show(30314);
			goto handled;
		}

		if (_action.isObject(NOUN_DUCTWORK)) {
			_vm->_dialogs->show(30315);
			goto handled;
		}

		if (_action.isObject(NOUN_CRATE)) {
			_vm->_dialogs->show(30318);
			goto handled;
		}

		if (_action.isObject(NOUN_SUPPORT)) {
			_vm->_dialogs->show(30319);
			goto handled;
		}

		if (_action.isObject(NOUN_PIECE_OF_WOOD)) {
			_vm->_dialogs->show(30320);
			goto handled;
		}

		if (_action.isObject(NOUN_RAILING)) {
			_vm->_dialogs->show(30321);
			goto handled;
		}

		if (_action.isObject(NOUN_CHANDELIER_TRAP)) {
			_vm->_dialogs->show(30322);
			goto handled;
		}

		if (_action.isObject(NOUN_HOLE)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30326);
			else
				_vm->_dialogs->show(30323);

			goto handled;
		}

		if (_action.isObject(NOUN_LARGE_NOTE) && _game._objects.isInRoom(OBJ_LARGE_NOTE)) {
			_vm->_dialogs->show(30324);
			goto handled;
		}
	}

	if (_action.isAction(VERB_WALK_TO, NOUN_HOLE)) {
		_vm->_dialogs->show(30325);
		goto handled;
	}

	if (_action.isAction(VERB_TAKE, NOUN_HEMP)) {
		_vm->_dialogs->show(30327);
		goto handled;
	}

	if (_action.isAction(VERB_PULL, NOUN_HEMP)) {
		_vm->_dialogs->show(30141);
		goto handled;
	}

	goto done;

handled:
	_action._inProgress = false;

done:
	;
}

void Scene303::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_CATWALK) && !_globals[kRightDoorIsOpen504]) {
		if (_scene->_customDest.x > 160)
			_game._player._walkOffScreenSceneId = 302;
		else
			_game._player._walkOffScreenSceneId = 307;
	}

	if (_action.isAction(VERB_CLIMB_INTO, NOUN_HOLE) || _action.isAction(VERB_CLIMB_DOWN, NOUN_CHANDELIER_CABLE))
		_game._player.walk(Common::Point(110, 95), FACING_SOUTHWEST);
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
