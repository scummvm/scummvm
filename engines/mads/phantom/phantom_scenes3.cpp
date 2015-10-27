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
		goto handled;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(30110);
		goto handled;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_GRID)) {
			_vm->_dialogs->show(30111);
			goto handled;
		}

		if (_action.isObject(NOUN_CATWALK)) {
			_vm->_dialogs->show(30112);
			goto handled;
		}

		if (_action.isObject(NOUN_SIDE_WALL)) {
			_vm->_dialogs->show(30113);
			goto handled;
		}

		if (_action.isObject(NOUN_BACK_WALL)) {
			_vm->_dialogs->show(30114);
			goto handled;
		}

		if (_action.isObject(NOUN_SUPPORT)) {
			_vm->_dialogs->show(30115);
			goto handled;
		}

		if (_action.isObject(NOUN_ACT_CURTAIN)) {
			_vm->_dialogs->show(30116);
			goto handled;
		}

		if (_action.isObject(NOUN_HOUSE)) {
			_vm->_dialogs->show(30117);
			goto handled;
		}

		if (_action.isObject(NOUN_OTHER_CATWALK)) {
			_vm->_dialogs->show(30118);
			goto handled;
		}

		if (_action.isObject(NOUN_GRIDWORK)) {
			_vm->_dialogs->show(30119);
			goto handled;
		}

		if (_action.isObject(NOUN_BEAM_POSITION)) {
			_vm->_dialogs->show(30120);
			goto handled;
		}

		if (_action.isObject(NOUN_LIGHTING_INSTRUMENT)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30121);
			else
				_vm->_dialogs->show(30122);

			goto handled;
		}

		if (_action.isObject(NOUN_TARP)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(30123);
			else
				_vm->_dialogs->show(30140);

			goto handled;
		}

		if (_action.isObject(NOUN_COUNTERWEIGHT_SYSTEM)) {
			_vm->_dialogs->show(30124);
			goto handled;
		}

		if (_action.isObject(NOUN_SANDBAG) && (_action._mainObjectSource == CAT_HOTSPOT)) {
			_vm->_dialogs->show(30125);
			goto handled;
		}

		if (_action.isObject(NOUN_BATTEN)) {
			_vm->_dialogs->show(30126);
			goto handled;
		}

		if (_action.isObject(NOUN_STOOL)) {
			_vm->_dialogs->show(30127);
			goto handled;
		}

		if (_action.isObject(NOUN_HEMP)) {
			_vm->_dialogs->show(30128);
			goto handled;
		}

		if (_action.isObject(NOUN_CIRCULAR_STAIRCASE)) {
			_vm->_dialogs->show(30129);
			goto handled;
		}

		if (_action.isObject(NOUN_CATWALK_OVER_HOUSE)) {
			_vm->_dialogs->show(30130);
			goto handled;
		}

		if (_action.isObject(NOUN_STAIRCASE_POST)) {
			_vm->_dialogs->show(30131);
			goto handled;
		}

		if (_action.isObject(NOUN_RAILING)) {
			_vm->_dialogs->show(30132);
			goto handled;
		}

		if (_action.isObject(NOUN_CYCLORAMA)) {
			_vm->_dialogs->show(30133);
			goto handled;
		}

		if (_action.isObject(NOUN_BIG_PROP)) {
			_vm->_dialogs->show(30134);
			goto handled;
		}

		if (_action.isObject(NOUN_PROSCENIUM_ARCH)) {
			_vm->_dialogs->show(30135);
			goto handled;
		}

		if (_action.isObject(NOUN_CABLE)) {
			_vm->_dialogs->show(30136);
			goto handled;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_HEMP)) {
		_vm->_dialogs->show(30138);
		goto handled;
	}

	if (_action.isAction(VERB_PULL, NOUN_HEMP)) {
		_vm->_dialogs->show(30141);
		goto handled;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SANDBAG) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		_vm->_dialogs->show(30139);
		goto handled;
	}

	if (_action.isAction(VERB_TAKE, NOUN_TARP)) {
		_vm->_dialogs->show(30142);
		goto handled;
	}

	goto done;

handled:
	_action._inProgress = false;

done:
	;
}

void Scene301::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_CATWALK_OVER_HOUSE))
		_game._player._walkOffScreenSceneId = 302;
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
