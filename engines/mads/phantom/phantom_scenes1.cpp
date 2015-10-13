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
	_converse_counter = 0;
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
	s.syncAsSint16LE(_converse_counter);
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
		_execute_chan = _execute_wipe = -1;
		_start_walking = _start_walking_0 = false;
		_anim_0_running = _anim_1_running = false;
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
	if (_anim_0_running)
		handleAnimation0();

	if ((_globals[kWalkerConverse] == 2) || (_globals[kWalkerConverse] == 3)) {
		++_converse_counter;
		if (_converse_counter > 200)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	if (_anim_1_running) {
		handleAnimation1();

		if (_scene->_animation[1]->getCurrentFrame() == 80) {
			_game._player._stepEnabled = true;
			_game._player.setWalkTrigger(55);
		}
	}

	if (!_start_sitting_down && (_globals[kBrieTalkStatus] != 2)) {
		warning("TODO: Add a check on view port x > 200");
		_start_sitting_down = true;
		_game._player.walk(Common::Point(490, 119), FACING_NORTHEAST);
		_game._player._stepEnabled = false;
		_game._player.setWalkTrigger(55);
		_brie_chandelier_position = 4;
	}

	if (_game._trigger == 55) {
		_game._player._stepEnabled = true;
		_vm->_gameConv->run(1);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_brie_chandelier_frame = -1;
		_talk_count = 0;
	}

	if (_game._trigger == 50) {
		_vm->_gameConv->run(0);
		_brie_calling_position = 1;
	}
}

void Scene101::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_ORCHESTRA_PIT)) {
		if ((_globals[kBrieTalkStatus] == 2) || _start_walking) {
			_game._player._walkOffScreenSceneId = 102;
			_globals[kBrieTalkStatus] = 2;
		} else {
			_vm->_gameConv->run(0);
			_game._player._needToWalk = false;
		}
	} else if (_action.isAction(VERB_EXIT_TO, NOUN_GRAND_FOYER)) {
		if ((_globals[kBrieTalkStatus] == 2) || _start_walking)
			_game._player._walkOffScreenSceneId = 202;
		else {
			_vm->_gameConv->run(0);
			_game._player._needToWalk = false;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_MONSIEUR_BRIE))
		_vm->_dialogs->show(10121);
	else if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE) && (_globals[kBrieTalkStatus] == 2))
		_game._player._needToWalk = false;
}

void Scene101::actions() {
	if (_vm->_gameConv->_running == 0)
		handleConversation0();
	else if (_vm->_gameConv->_running == 1)
		handleConversation1();
	else if (_action._lookFlag) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(10110);
		else
			_vm->_dialogs->show(10111);
	} else if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_AISLE)) {
			_vm->_dialogs->show(10112);
		} else if (_action.isObject(NOUN_CHANDELIER)) {
			_vm->_dialogs->show(10113);
		} else if (_action.isObject(NOUN_BACK_WALL)) {
			_vm->_dialogs->show(10114);
		} else if (_action.isObject(NOUN_SIDE_WALL)) {
			_vm->_dialogs->show(10115);
		} else if (_action.isObject(NOUN_SEATS)) {
			if ((_globals[kBrieTalkStatus] > 1) || _start_walking)
				_vm->_dialogs->show(10119);
			else
				_vm->_dialogs->show(10116);
		} else if (_action.isObject(NOUN_GRAND_FOYER)) {
			_vm->_dialogs->show(10117);
		} else if (_action.isObject(NOUN_ORCHESTRA_PIT)) {
			_vm->_dialogs->show(10118);
		} else if (_action.isObject(NOUN_MONSIEUR_BRIE)) {
			_vm->_dialogs->show(10120);
		}
	} else if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE)) {
		if (_globals[kBrieTalkStatus] == 2)
			_vm->_dialogs->show(10122);
	} else if (!_action.isAction(VERB_TAKE, NOUN_MONSIEUR_BRIE))
		return;

	_action._inProgress = false;
}

void Scene101::handleConversation0() {
	_vm->_gameConv->setHeroTrigger(90);

	if (_game._trigger == 90) {
		_globals[kBrieTalkStatus] = 1;
		_start_walking_0 = true;
	}  
}

void Scene101::handleConversation1() {
	if ((_action._activeAction._verbId >= 0) && (_action._activeAction._verbId <= 27)) {
		int interlocutorFl = false;

		if (_game._trigger == 60) {
			switch (_action._activeAction._verbId) {
			case 0:
				_brie_chandelier_position = 6;
				_execute_wipe = 2;
				break;

			case 1:
				_brie_chandelier_position = 2;
				_execute_chan = 9;
				break;

			case 4:
				_brie_chandelier_position = 0;
				_execute_chan = -1;
				_execute_wipe = -1;
				break;

			case 8:
			case 10:
			case 18:
			case 22:
			case 24:
				_start_walking = true;
				_execute_chan = -1;
				_execute_wipe = -1;
				_globals[kWalkerConverse] = 0;
				_vm->_gameConv->setInterlocutorTrigger(105);
				interlocutorFl = true;
				break;

			case 12:
				_brie_chandelier_position = 5;
				_execute_chan = -1;
				_execute_wipe = -1;
				break;

			default:
				break;
			}
		}

		if (!interlocutorFl)
			_vm->_gameConv->setInterlocutorTrigger(60);

		_vm->_gameConv->setHeroTrigger(70);

		_talk_count = 0;

		if (_game._trigger == 60) {
			if (!_start_walking)
				_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);

			_brie_chandelier_position = 2;
		} else if ((_game._trigger == 70) && !_start_walking) {
			_brie_chandelier_position = 4;
			_execute_chan = -1;
			_execute_wipe = -1;
			if (!_start_walking)
				_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);

			_converse_counter = 0;
		}
	}
}

void Scene101::handleAnimation0() {
	int random;
	int reset_frame;

	if (_scene->_animation[0]->getCurrentFrame() != _brie_calling_frame) {
		_brie_calling_frame = _scene->_animation[0]->getCurrentFrame();
		reset_frame = -1;
		switch (_brie_calling_frame) {
		case 1:
		case 9:
		case 12:
			if (_brie_calling_position == 1) {
				if (_brie_calling_frame == 9) {
					if (_start_walking_0) {
						reset_frame = 13;
						_brie_calling_position = 3;
					} else
						_brie_calling_position = 2;
				} else
					reset_frame = 1;
			}

			if (_brie_calling_position == 0) {
				if (_start_walking_0) {
					reset_frame = 60;
					_brie_calling_position = 3;
				} else
					reset_frame = 0;
			}

			if (_brie_calling_position == 2) {
				if (_start_walking_0) {
					reset_frame = 13;
					_brie_calling_position = 3;
				} else {
					random = _vm->getRandomNumber(1, 2);
					++_talk_count;

					if (_talk_count < 18) {
						if (random == 1)
							reset_frame = 7;
						else
							reset_frame = 10;
					} else {
						reset_frame = 54;
						_brie_calling_position = 0;
					}
				}
			}
			break;

		case 53:
			_anim_0_running    = false;
			_scene->freeAnimation(0);
			break;

		case 59:  
			if (_start_walking_0) {
				reset_frame = 60;
				_brie_calling_position = 3;
			} else {
				reset_frame = 0;
				_brie_calling_position = 0;
			}
			break;

		case 66:
			reset_frame = 24;
			break;
		}

		if (reset_frame >= 0) {
			if (_scene->_animation[0])
				_scene->_animation[0]->setCurrentFrame(reset_frame);
			_brie_calling_frame = reset_frame;
		}
	}
} 

void Scene101::handleAnimation1() {
	int random;
	int reset_frame;

	if (_scene->_animation[1]->getCurrentFrame() != _brie_chandelier_frame) {
		_brie_chandelier_frame = _scene->_animation[1]->getCurrentFrame();
		reset_frame = -1;
		switch (_brie_chandelier_frame) {
		case 1:
			if (_brie_chandelier_position == 3)
				reset_frame = 0;
			break;

		case 11:
		case 14:
		case 17:
		case 19:
		case 26:
		case 44:
		case 333:
			if (_talk_count == _execute_chan) {
				_brie_chandelier_position = 0;
				++_talk_count;
				_execute_chan = -1;
			} 

			if (_talk_count == _execute_wipe) {
				_brie_chandelier_position = 6;
				++_talk_count;
				_execute_wipe = -1;
			}

			if (_start_walking) {
				if (_vm->_gameConv->_running == 1) {
					if (_talk_count > 13)
						_brie_chandelier_position = 1;
				} else
					_brie_chandelier_position = 1;
			}

			switch (_brie_chandelier_position) {
			case 0:
				reset_frame = 27;
				_brie_chandelier_position = 2;
				break;

			case 1:
				_globals[kBrieTalkStatus] = 2;
				reset_frame = 45;
				if (_vm->_gameConv->_running == 1)
					_vm->_gameConv->stop();
				_scene->_dynamicHotspots.remove(_dynamic_brie);
				_game._player._stepEnabled = false;
				break;

			case 2:
				random = _vm->getRandomNumber(1, 3);
				++_talk_count;
				if (_talk_count < 15) {
					if (random == 1)
						reset_frame = 12;
					else if (random == 2)
						reset_frame = 14;
					else if (random == 3)
						reset_frame = 17;
				} else {
					_brie_chandelier_position = 4;
					reset_frame = 25;
				}
				break;

			case 4:
				reset_frame = 25;
				break;

			case 5:
				reset_frame = 21;
				_brie_chandelier_position = 2;
				break;

			case 6:
				reset_frame = 316;
				_brie_chandelier_position = 2;
				break;

			default:
				break;
			}
			break;

		case 315:
			_scene->freeAnimation(1);
			break;
		}

		if (reset_frame >= 0) {
			if (_scene->_animation[1])
				_scene->_animation[1]->setCurrentFrame(reset_frame);
			_brie_chandelier_frame = reset_frame;
		}
	}
}

/*------------------------------------------------------------------------*/

Scene102::Scene102(MADSEngine *vm) : Scene1xx(vm) {
	_anim0RunningFl = false;
}

void Scene102::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_anim0RunningFl);
}

void Scene102::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene102::enter() {
	_anim0RunningFl = false;

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RAL86");

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_scene->drawToBackground(_globals._sequenceIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else {
		_scene->_hotspots.activate(NOUN_CHANDELIER, false);
	}

	if (_scene->_priorSceneId == 101) {
		_game._player._playerPos = Common::Point(97, 79);
		_game._player._facing = FACING_SOUTHEAST;
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 4);
		_game._player.walk(Common::Point(83, 87), FACING_SOUTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if (_scene->_priorSceneId == 104) {
		// Player fell from pit -> death
		Common::Point deathPos = Common::Point(0, 0);
		int deathScale = 0;
		int deathDepth = 0;
		_game._player._stepEnabled = false;
		_game._player._visible = false;

		switch (_globals[36]) {
		case 0:
			deathPos = Common::Point(221, 57);
			deathScale = 50;
			deathDepth = 14;
			break;

		case 1:
			deathPos = Common::Point(219, 85);
			deathScale = 60;
			deathDepth = 6;
			break;

		case 2:
			deathPos = Common::Point(257, 138);
			deathScale = 76;
			deathDepth = 1;
			break;

		default:
			break;
		}
		_scene->_userInterface.emptyConversationList();
		_scene->_userInterface.setup(kInputConversation);
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], deathDepth);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], deathPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[3], deathScale);
		_scene->_sequences.setTimingTrigger(120, 65);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if (_scene->_priorSceneId == 103 || _scene->_priorSceneId != -1) {
		_game._player._playerPos = Common::Point(282, 145);
		_game._player._facing = FACING_WEST;
		_anim0RunningFl = true;
		_scene->loadAnimation(formAnimName('d', 1), 60, 0);
	} else if (_scene->_priorSceneId == -1) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	}

	sceneEntrySound();
}

void Scene102::step() {
	if (_game._trigger == 60) {
		// Door closes
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		_anim0RunningFl = false;
	} else if (_game._trigger == 65) {
		// Death
		if (_globals[kDeathLocation] == 0)
			_vm->_dialogs->show(10232);
		else
			_vm->_dialogs->show(10229);

		_vm->_sound->command(16);
		_scene->_nextSceneId = 104;
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
	} else if (_action.isAction(VERB_WALK_THROUGH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_PUSH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_OPEN, NOUN_ORCHESTRA_DOOR)) {
		if (_anim0RunningFl) {
			_scene->_sequences.setTimingTrigger(15, 70);
			_game._player._stepEnabled = false;
		} else {
			switch (_game._trigger) {
			case 70:	// try again
			case 0:
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_scene->loadAnimation(formAnimName('d', 0), 1, 0);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				break;
			case 1:
				_scene->_nextSceneId = 103;
				break;
			default:
				break;
			}
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(10210);
	else if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
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
	} else if (_action.isAction(VERB_CLOSE, NOUN_ORCHESTRA_DOOR)) {
		_vm->_dialogs->show(10228);
	}

	_game._player._stepEnabled = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
