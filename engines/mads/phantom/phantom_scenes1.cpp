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
#include "mads/phantom/phantom_scenes1.h"

namespace MADS {

namespace Phantom {

void Scene1xx::setAAName() {
	_game._aaName = Resources::formatAAName(_globals[kTempInterface]);
	_vm->_palette->setEntry(254, 43, 47, 51);
}

void Scene1xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	if (_globals[kDoneBrieConv203] == 3) {
		_vm->_sound->command(39);
	} else {
		switch (_scene->_nextSceneId) {
		case 101:
			if (!_game._visitedScenes._sceneRevisited)
				_vm->_sound->command(38);
			else
				_vm->_sound->command(16);
			break;

		case 102:
			if (_scene->_priorSceneId == 104)
				_vm->_sound->command(27);
			else
				_vm->_sound->command(16);
			break;

		case 103:
			if ((_globals[kJacquesStatus] == 2)) {
				_vm->_sound->command(32);
				_globals[kJacquesStatus] = 3;
			} else
				_vm->_sound->command(16);
			break;

		case 104:
			if ((_vm->_gameConv->_restoreRunning == 7) || (_scene->_priorSceneId == 301)) {
				_vm->_sound->command(33);
			} else if (!_globals[kRoom103104Transition] && !_globals[kObservedPhan104]) {
				_vm->_sound->command(37);
				_globals[kObservedPhan104] = true;
			} else
				_vm->_sound->command(16);
			break;

		case 111:
			if (_scene->_priorSceneId == 150)
				_vm->_sound->command(38);
			else if (_globals[kLeaveAngelMusicOn])
				_vm->_sound->command(34);
			else
				_vm->_sound->command(16);
			break;
		case 113:
			if (_globals[kLeaveAngelMusicOn])
				_vm->_sound->command(34);
			else if (_globals[kCurrentYear] == 1993)
				_vm->_sound->command(36);
			else
				_vm->_sound->command(35);
			break;

		default:
			if ((_scene->_priorSceneId != 204) && (_scene->_nextSceneId != 150))
				_vm->_sound->command(16);
			break;
		}
	}
}

void Scene1xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if (!_game._player._forcePrefix)
		_game._player._spritesPrefix = "RAL";

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;}

/*------------------------------------------------------------------------*/

Scene101::Scene101(MADSEngine *vm) : Scene1xx(vm) {
	_execute_chan = -1;
	_execute_wipe = -1;
	_brie_calling_position = -1;
	_brie_chandelier_position = -1;
	_brie_calling_frame = -1;
	_brie_chandelier_frame = -1;
	_talk_count = -1;
	_dynamic_brie = 0;
	_dynamic_brie_2 = 0;
	_start_walking = false;
	_start_walking_0 = false;
	_anim_0_running = false;
	_anim_1_running = false;
	_start_sitting_down = false;
}

void Scene101::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
	s.syncAsSint16LE(_execute_chan);
	s.syncAsSint16LE(_execute_wipe);
	s.syncAsSint16LE(_brie_calling_position);
	s.syncAsSint16LE(_brie_chandelier_position);
	s.syncAsSint16LE(_brie_calling_frame);
	s.syncAsSint16LE(_brie_chandelier_frame);
	s.syncAsSint16LE(_talk_count);
	s.syncAsSint16LE(_dynamic_brie);
	s.syncAsSint16LE(_dynamic_brie_2);
	s.syncAsByte(_start_walking);
	s.syncAsByte(_start_walking_0);
	s.syncAsByte(_anim_0_running);
	s.syncAsByte(_anim_1_running);
	s.syncAsByte(_start_sitting_down);
}

void Scene101::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_MONSIEUR_BRIE);
}

void Scene101::enter() {
	_vm->_disableFastwalk = true;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_execute_chan         = -1;
		_execute_wipe         = -1;
		_start_walking        = false;
		_start_walking_0      = false;
		_anim_0_running       = true;
		_anim_1_running       = false;
		_start_sitting_down = false;
	}

	// Load Dialogs
	_vm->_gameConv->get(0);
	_vm->_gameConv->get(1);

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else {
		_scene->_hotspots.activate(NOUN_CHANDELIER, false);
	}

	if (_globals[kBrieTalkStatus] == 0) {
		_game._player.firstWalk(Common::Point(-20, 75), FACING_EAST, Common::Point(18, 79), FACING_EAST, true);
		_brie_calling_position = 0;
		_brie_chandelier_position = 3;
		_game._player.setWalkTrigger(50);

		_scene->loadAnimation(formAnimName('b', 9), 1, 1);
		_scene->loadAnimation(formAnimName('b', 8), 1, 0);

		_anim_0_running  = true;
		_anim_1_running  = true;

		_dynamic_brie = _scene->_dynamicHotspots.add(NOUN_MONSIEUR_BRIE, VERB_WALKTO, SYNTAX_SINGULAR_MASC, -1, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[_dynamic_brie]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_dynamic_brie, Common::Point(490, 119), FACING_NONE);
		_scene->setDynamicAnim(_dynamic_brie, 0, 0);
		_scene->setDynamicAnim(_dynamic_brie, 0, 1);
		_scene->setDynamicAnim(_dynamic_brie, 0, 2);
		_scene->setDynamicAnim(_dynamic_brie, 0, 3);
		_scene->setDynamicAnim(_dynamic_brie, 0, 4);

		_dynamic_brie_2 = _scene->_dynamicHotspots.add(NOUN_MONSIEUR_BRIE, VERB_WALKTO, SYNTAX_SINGULAR_MASC, -1, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[_dynamic_brie_2]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_dynamic_brie_2, Common::Point(25, 80), FACING_NONE);
		_scene->setDynamicAnim(_dynamic_brie_2, 1, 1);
		_scene->setDynamicAnim(_dynamic_brie_2, 1, 2);

		_talk_count = 0;
	} else if (_globals[kBrieTalkStatus] == 1) {
		_scene->loadAnimation(formAnimName('b', 9), 1, 1);
		_dynamic_brie = _scene->_dynamicHotspots.add(NOUN_MONSIEUR_BRIE, VERB_WALKTO, SYNTAX_SINGULAR_MASC, -1, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[_dynamic_brie]._articleNumber = PREP_ON;
		_scene->setDynamicAnim(_dynamic_brie, 1, 1);
		_scene->setDynamicAnim(_dynamic_brie, 1, 2);
		_anim_1_running = true;
		_talk_count = 0;
		_brie_chandelier_position = 3;

		if (_vm->_gameConv->_restoreRunning == 1) {
			_vm->_gameConv->run(1);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_brie_chandelier_position = 4;
			if (_scene->_animation[1])
				_scene->_animation[1]->setCurrentFrame(25);
		}
	} else if (_scene->_priorSceneId == 202) {
		if (_globals[kJacquesStatus] == 1)
			_globals[kJacquesStatus] = 2;
		_game._player.firstWalk(Common::Point(-20, 75), FACING_EAST, Common::Point(18, 79), FACING_EAST, true);
	} else if ((_scene->_priorSceneId == 102) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player.firstWalk(Common::Point(655, 130), FACING_WEST, Common::Point(625, 127), FACING_WEST, true);
		_scene->setCamera(Common::Point(320, 0));
	}

	sceneEntrySound();
}

void Scene101::step() {
	// TODO
}

void Scene101::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_ORCHESTRA_PIT)) {
		// TODO: Handle Brie
		_game._player._walkOffScreenSceneId = 102;
	} else if (_action.isAction(VERB_EXIT_TO, NOUN_GRAND_FOYER)) {
		// TODO: Handle Brie
		_game._player._walkOffScreenSceneId = 202;
	} else if (_action.isAction(VERB_TAKE, NOUN_MONSIEUR_BRIE)) {
		_vm->_dialogs->show(10121);
	} else if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE)) {
		if (_globals[kBrieTalkStatus] == 2)
			_game._player._needToWalk = false;
	}

	// TODO
}

void Scene101::actions() {
	// TODO: Brie conversation

	// TODO: Look around

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_AISLE)) {
			_vm->_dialogs->show(10112);
		} else if (_action.isObject(NOUN_CHANDELIER)) {
			_vm->_dialogs->show(10113);
		} else if (_action.isObject(NOUN_BACK_WALL)) {
			_vm->_dialogs->show(10114);
		} else if (_action.isObject(NOUN_SIDE_WALL)) {
			_vm->_dialogs->show(10115);
		} else if (_action.isObject(NOUN_SEATS)) {
			// TODO: Finish this
			_vm->_dialogs->show(10116);
		} else if (_action.isObject(NOUN_GRAND_FOYER)) {
			_vm->_dialogs->show(10117);
		} else if (_action.isObject(NOUN_ORCHESTRA_PIT)) {
			_vm->_dialogs->show(10118);
		} else if (_action.isObject(NOUN_MONSIEUR_BRIE)) {
			_vm->_dialogs->show(10120);
		}

		_game._player._stepEnabled = true;
	} else if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE)) {
		if (_globals[kBrieTalkStatus] == 2)
			_vm->_dialogs->show(10122);
		_game._player._stepEnabled = true;
	} else if (_action.isAction(VERB_TAKE, NOUN_MONSIEUR_BRIE)) {
		_game._player._stepEnabled = true;
	}
}

/*------------------------------------------------------------------------*/

Scene102::Scene102(MADSEngine *vm) : Scene1xx(vm) {
	_animRunningFl = false;
}

void Scene102::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_animRunningFl);
}

void Scene102::setup() {
	//setPlayerSpritesPrefix();
	setAAName();
}

void Scene102::enter() {
	_animRunningFl = false;

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RAL86");

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		// TODO
		//_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else {
		// TODO
	}

	if (_scene->_priorSceneId == 101) {
		_game._player._playerPos = Common::Point(97, 79);
		_game._player._facing = FACING_SOUTHEAST;
		// TODO
		_game._player.walk(Common::Point(83, 87), FACING_SOUTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if (_scene->_priorSceneId == 104) {
		// Player fell from pit -> death
		// TODO
	} else if (_scene->_priorSceneId == 103 || _scene->_priorSceneId != -1) {
		_game._player._playerPos = Common::Point(282, 145);
		_game._player._facing = FACING_WEST;
		_animRunningFl = true;
		// TODO: Door closing animation
	} else if (_scene->_priorSceneId == -1) {
		// TODO
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	}

	sceneEntrySound();
}

void Scene102::step() {
	if (_game._trigger == 60) {		// Door closes
		// TODO
		_animRunningFl = false;
	} else if (_game._trigger == 65) {		// Death
		// TODO
		_scene->_currentSceneId = 104;
	}
}

void Scene102::preActions() {
	if (_action.isAction(VERB_OPEN, NOUN_ORCHESTRA_DOOR) || _action.isAction(VERB_PUSH, NOUN_ORCHESTRA_DOOR)) {
		_game._player.walk(Common::Point(282, 145), FACING_EAST);
	}
}

void Scene102::actions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_AISLE)) {
		_scene->_nextSceneId = 101;
		_game._player._stepEnabled = true;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_PUSH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_OPEN, NOUN_ORCHESTRA_DOOR)) {
		if (_animRunningFl) {
			// TODO
		} else {
			_scene->_nextSceneId = 103;		// FIXME: temporary HACK - remove!

			switch (_game._trigger) {
			case 70:	// try again
			case 0:
				// TODO
				break;
			case 1:
				_scene->_nextSceneId = 103;
				break;
			}
		}

		_game._player._stepEnabled = true;
	}

	// TODO: Look around

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_PIT))
			_vm->_dialogs->show(10211);
		else if (_action.isObject(NOUN_SEATS))
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->show(10212);
			else
				_vm->_dialogs->show(10230);
		else if (_action.isObject(NOUN_ORCHESTRA_DOOR))
			_vm->_dialogs->show(10213);
		else if (_action.isObject(NOUN_CONDUCTORS_STAND))
			_vm->_dialogs->show(10214);
		else if (_action.isObject(NOUN_MUSIC_STAND) || _action.isObject(NOUN_MUSIC_STANDS))
			_vm->_dialogs->show(10215);
		else if (_action.isObject(NOUN_PROMPTERS_BOX))
			_vm->_dialogs->show(10217);
		else if (_action.isObject(NOUN_STAGE))
			_vm->_dialogs->show(10218);
		else if (_action.isObject(NOUN_APRON))
			_vm->_dialogs->show(10219);
		else if (_action.isObject(NOUN_SIDE_WALL))
			_vm->_dialogs->show(10220);
		else if (_action.isObject(NOUN_FOLDING_CHAIRS))
			_vm->_dialogs->show(10221);
		else if (_action.isObject(NOUN_AISLE))
			_vm->_dialogs->show(10222);
		else if (_action.isObject(NOUN_PROSCENIUM_ARCH))
			_vm->_dialogs->show(10223);
		else if (_action.isObject(NOUN_ACT_CURTAIN))
			_vm->_dialogs->show(10224);
		else if (_action.isObject(NOUN_IN_ONE))
			_vm->_dialogs->show(10225);
		else if (_action.isObject(NOUN_IN_TWO))
			_vm->_dialogs->show(10226);
		else if (_action.isObject(NOUN_LEG))
			_vm->_dialogs->show(10227);
		else if (_action.isObject(NOUN_CHANDELIER))
			_vm->_dialogs->show(10231);
		else
			return;

		_game._player._stepEnabled = true;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_ORCHESTRA_DOOR)) {
		_vm->_dialogs->show(10228);
		_game._player._stepEnabled = true;
	}
}


/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
