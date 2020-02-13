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
			if (_globals[kJacquesStatus] == 2) {
				_vm->_sound->command(32);
				_globals[kJacquesStatus] = 3;
			} else
				_vm->_sound->command(16);
			break;

		case 104:
			if ((_vm->_gameConv->restoreRunning() == 7) || (_scene->_priorSceneId == 301)) {
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

	_game._player._scalingVelocity = true;
}

/*------------------------------------------------------------------------*/

Scene101::Scene101(MADSEngine *vm) : Scene1xx(vm) {
	_chanStatus = -1;
	_wipeStatus = -1;
	_callingStatus = -1;
	_chandelierStatus = -1;
	_callingFrame = -1;
	_chandelierFrame = -1;
	_convCounter = 0;
	_talkCounter = -1;
	_brieAnimId = 0;
	_startWalkingFl = false;
	_startWalking0Fl = false;
	_anim0Running = false;
	_anim1Running = false;
	_startSittingFl = false;
}

void Scene101::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
	s.syncAsSint16LE(_chanStatus);
	s.syncAsSint16LE(_wipeStatus);
	s.syncAsSint16LE(_callingStatus);
	s.syncAsSint16LE(_chandelierStatus);
	s.syncAsSint16LE(_callingFrame);
	s.syncAsSint16LE(_chandelierFrame);
	s.syncAsSint16LE(_convCounter);
	s.syncAsSint16LE(_talkCounter);
	s.syncAsSint16LE(_brieAnimId);
	s.syncAsByte(_startWalkingFl);
	s.syncAsByte(_startWalking0Fl);
	s.syncAsByte(_anim0Running);
	s.syncAsByte(_anim1Running);
	s.syncAsByte(_startSittingFl);
}

void Scene101::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_MONSIEUR_BRIE);
}

void Scene101::enter() {
	_vm->_disableFastwalk = true;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_chanStatus = _wipeStatus = -1;
		_startWalkingFl = _startWalking0Fl = false;
		_anim0Running = _anim1Running = false;
		_startSittingFl = false;
	}

	// Load conversations
	_vm->_gameConv->load(0);
	_vm->_gameConv->load(1);

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else {
		_scene->_hotspots.activate(NOUN_CHANDELIER, false);
	}

	if (_globals[kBrieTalkStatus] == 0) {
		_game._player.firstWalk(Common::Point(-20, 75), FACING_EAST, Common::Point(18, 79), FACING_EAST, true);
		_callingStatus = 0;
		_chandelierStatus = 3;
		_game._player.setWalkTrigger(50);

		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('b', 9), 1);
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 8), 1);

		_anim0Running = true;
		_anim1Running = true;

		_brieAnimId = _scene->_dynamicHotspots.add(NOUN_MONSIEUR_BRIE, VERB_WALKTO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[_brieAnimId]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_brieAnimId, Common::Point(490, 119), FACING_NONE);
		_scene->setDynamicAnim(_brieAnimId, _globals._animationIndexes[0], 0);
		_scene->setDynamicAnim(_brieAnimId, _globals._animationIndexes[0], 1);
		_scene->setDynamicAnim(_brieAnimId, _globals._animationIndexes[0], 2);
		_scene->setDynamicAnim(_brieAnimId, _globals._animationIndexes[0], 3);
		_scene->setDynamicAnim(_brieAnimId, _globals._animationIndexes[0], 4);

		int tmpIdx = _scene->_dynamicHotspots.add(NOUN_MONSIEUR_BRIE, VERB_WALKTO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(25, 80), FACING_NONE);
		_scene->setDynamicAnim(tmpIdx, _globals._animationIndexes[1], 1);
		_scene->setDynamicAnim(tmpIdx, _globals._animationIndexes[1], 2);

		_talkCounter = 0;
	} else if (_globals[kBrieTalkStatus] == 1) {
		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('b', 9), 1);
		_brieAnimId = _scene->_dynamicHotspots.add(NOUN_MONSIEUR_BRIE, VERB_WALKTO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots[_brieAnimId]._articleNumber = PREP_ON;
		_scene->setDynamicAnim(_brieAnimId, _globals._animationIndexes[1], 1);
		_scene->setDynamicAnim(_brieAnimId, _globals._animationIndexes[1], 2);
		_anim1Running = true;
		_talkCounter = 0;
		_chandelierStatus = 3;

		if (_vm->_gameConv->restoreRunning() == 1) {
			_vm->_gameConv->run(1);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_chandelierStatus = 4;
			_scene->setAnimFrame(_globals._animationIndexes[1], 25);
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
	if (_anim0Running)
		handleAnimation0();

	if ((_globals[kWalkerConverse] == 2) || (_globals[kWalkerConverse] == 3)) {
		++_convCounter;
		if (_convCounter > 200)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	if (_anim1Running) {
		handleAnimation1();

		if (_scene->getAnimFrame(_globals._animationIndexes[1]) == 80) {
			_game._player._stepEnabled = true;
			_game._player.setWalkTrigger(55);
		}
	}

	if (_scene->_posAdjust.x > 200 && !_startSittingFl && (_globals[kBrieTalkStatus] != 2)) {
		_startSittingFl = true;
		_game._player.walk(Common::Point(490, 119), FACING_NORTHEAST);
		_game._player._stepEnabled = false;
		_game._player.setWalkTrigger(55);
		_chandelierStatus = 4;
	}

	if (_game._trigger == 55) {
		_game._player._stepEnabled = true;
		_vm->_gameConv->run(1);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_chandelierFrame = -1;
		_talkCounter = 0;
	}

	// Monsieur Brie beckons Raul
	if (_game._trigger == 50) {
		_vm->_gameConv->run(0);
		_callingStatus = 1;
	}
}

void Scene101::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_ORCHESTRA_PIT)) {
		if ((_globals[kBrieTalkStatus] == 2) || _startWalkingFl) {
			_game._player._walkOffScreenSceneId = 102;
			_globals[kBrieTalkStatus] = 2;
		} else {
			_vm->_gameConv->run(0);
			_game._player._needToWalk = false;
		}
	} else if (_action.isAction(VERB_EXIT_TO, NOUN_GRAND_FOYER)) {
		if ((_globals[kBrieTalkStatus] == 2) || _startWalkingFl)
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
	if (_vm->_gameConv->activeConvId() == 0)
		handleConversation0();
	else if (_vm->_gameConv->activeConvId() == 1)
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
			if ((_globals[kBrieTalkStatus] > 1) || _startWalkingFl)
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
		_startWalking0Fl = true;
	}
}

void Scene101::handleConversation1() {
	if ((_action._activeAction._verbId >= 0) && (_action._activeAction._verbId <= 27)) {
		bool interlocutorFl = false;

		if (_game._trigger == 60) {
			switch (_action._activeAction._verbId) {
			case 0:
				_chandelierStatus = 6;
				_wipeStatus = 2;
				break;

			case 1:
				_chandelierStatus = 2;
				_chanStatus = 9;
				break;

			case 4:
				_chandelierStatus = 0;
				_chanStatus = -1;
				_wipeStatus = -1;
				break;

			case 8:
			case 10:
			case 18:
			case 22:
			case 24:
				_startWalkingFl = true;
				_chanStatus = -1;
				_wipeStatus = -1;
				_globals[kWalkerConverse] = 0;
				_vm->_gameConv->setInterlocutorTrigger(105);
				interlocutorFl = true;
				break;

			case 12:
				_chandelierStatus = 5;
				_chanStatus = -1;
				_wipeStatus = -1;
				break;

			default:
				break;
			}
		}

		if (!interlocutorFl)
			_vm->_gameConv->setInterlocutorTrigger(60);

		_vm->_gameConv->setHeroTrigger(70);

		_talkCounter = 0;

		if (_game._trigger == 60) {
			if (!_startWalkingFl)
				_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);

			_chandelierStatus = 2;
		} else if ((_game._trigger == 70) && !_startWalkingFl) {
			_chandelierStatus = 4;
			_chanStatus = -1;
			_wipeStatus = -1;
			if (!_startWalkingFl)
				_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);

			_convCounter = 0;
		}
	}
}

void Scene101::handleAnimation0() {
	if (_scene->getAnimFrame(_globals._animationIndexes[0]) == _callingFrame)
		return;

	_callingFrame = _scene->getAnimFrame(_globals._animationIndexes[0]);
	int resetFrame = -1;
	switch (_callingFrame) {
	case 1:
	case 9:
	case 12:
		if (_callingStatus == 1) {
			if (_callingFrame == 9) {
				if (_startWalking0Fl) {
					resetFrame = 13;
					_callingStatus = 3;
				} else
					_callingStatus = 2;
			} else
				resetFrame = 1;
		}

		if (_callingStatus == 0) {
			if (_startWalking0Fl) {
				resetFrame = 60;
				_callingStatus = 3;
			} else
				resetFrame = 0;
		}

		if (_callingStatus == 2) {
			if (_startWalking0Fl) {
				resetFrame = 13;
				_callingStatus = 3;
			} else {
				++_talkCounter;

				if (_talkCounter < 18) {
					if (_vm->getRandomNumber(1, 2) == 1)
						resetFrame = 7;
					else
						resetFrame = 10;
				} else {
					resetFrame = 54;
					_callingStatus = 0;
				}
			}
		}
		break;

	case 53:
		_anim0Running = false;
		_scene->freeAnimation(0);
		break;

	case 59:
		if (_startWalking0Fl) {
			resetFrame = 60;
			_callingStatus = 3;
		} else {
			resetFrame = 0;
			_callingStatus = 0;
		}
		break;

	case 66:
		resetFrame = 24;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_callingFrame = resetFrame;
	}
}

void Scene101::handleAnimation1() {
	if (_scene->getAnimFrame(_globals._animationIndexes[1]) == _chandelierFrame)
		return;

	_chandelierFrame = _scene->getAnimFrame(_globals._animationIndexes[1]);
	int resetFrame = -1;
	switch (_chandelierFrame) {
	case 1:
		if (_chandelierStatus == 3)
			resetFrame = 0;
		break;

	case 11:
	case 14:
	case 17:
	case 19:
	case 26:
	case 44:
	case 333:
		if (_talkCounter == _chanStatus) {
			_chandelierStatus = 0;
			++_talkCounter;
			_chanStatus = -1;
		}

		if (_talkCounter == _wipeStatus) {
			_chandelierStatus = 6;
			++_talkCounter;
			_wipeStatus = -1;
		}

		if (_startWalkingFl) {
			if (_vm->_gameConv->activeConvId() == 1) {
				if (_talkCounter > 13)
					_chandelierStatus = 1;
			} else
				_chandelierStatus = 1;
		}

		switch (_chandelierStatus) {
		case 0:
			resetFrame = 27;
			_chandelierStatus = 2;
			break;

		case 1:
			_globals[kBrieTalkStatus] = 2;
			resetFrame = 45;
			if (_vm->_gameConv->activeConvId() == 1)
				_vm->_gameConv->stop();
			_scene->_dynamicHotspots.remove(_brieAnimId);
			_game._player._stepEnabled = false;
			break;

		case 2:
			++_talkCounter;
			if (_talkCounter < 15) {
				switch (_vm->getRandomNumber(1, 3)) {
				case 1:
					resetFrame = 12;
					break;

				case 2:
					resetFrame = 14;
					break;

				case 3:
					resetFrame = 17;
					break;

				default:
					break;
				}
			} else {
				_chandelierStatus = 4;
				resetFrame = 25;
			}
			break;

		case 4:
			resetFrame = 25;
			break;

		case 5:
			resetFrame = 21;
			_chandelierStatus = 2;
			break;

		case 6:
			resetFrame = 316;
			_chandelierStatus = 2;
			break;

		default:
			break;
		}
		break;

	case 315:
		_scene->freeAnimation(1);
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_chandelierFrame = resetFrame;
	}
}

/*------------------------------------------------------------------------*/

Scene102::Scene102(MADSEngine *vm) : Scene1xx(vm) {
	_anim0Running = false;
}

void Scene102::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_anim0Running);
}

void Scene102::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene102::enter() {
	_anim0Running = false;

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RAL86");

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
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
		_scene->_sequences.addTimer(120, 65);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if (_scene->_priorSceneId == 103 || _scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_game._player._playerPos = Common::Point(282, 145);
		_game._player._facing = FACING_WEST;
		_anim0Running = true;
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 60);
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
		_anim0Running = false;
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
	if (_action.isAction(VERB_OPEN, NOUN_ORCHESTRA_DOOR) || _action.isAction(VERB_PUSH, NOUN_ORCHESTRA_DOOR))
		_game._player.walk(Common::Point(282, 145), FACING_EAST);
}

void Scene102::actions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_AISLE)) {
		_scene->_nextSceneId = 101;
	} else if (_action.isAction(VERB_WALK_THROUGH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_PUSH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_OPEN, NOUN_ORCHESTRA_DOOR)) {
		if (_anim0Running) {
			_scene->_sequences.addTimer(15, 70);
			_game._player._stepEnabled = false;
		} else {
			switch (_game._trigger) {
			case 70:	// try again
			case 0:
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 0), 1);
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
	} else if (_action.isAction(VERB_CLOSE, NOUN_ORCHESTRA_DOOR))
		_vm->_dialogs->show(10228);

	_game._player._stepEnabled = false;
}

/*------------------------------------------------------------------------*/

Scene103::Scene103(MADSEngine *vm) : Scene1xx(vm) {
	_jacquesAction = -1;
	_lastRandom = -1;
	_standPosition = -1;
	_hotspotPrompt1 = -1;
	_hotspotPrompt2 = -1;
	_hotspotPrompt3 = -1;
	_hotspotPrompt4 = -1;
	_hotspotPrompt5 = -1;
	_hotspotRightFloor1 = -1;
	_hotspotRightFloor2 = -1;
	_hotspotLeftFloor1 = -1;
	_hotspotLeftFloor2 = -1;
	_hotspotGentleman = -1;
	_convCount = -1;
	_lastStairFrame = -1;
	_lastJacquesFrame = -1;
	_talkCount = -1;

	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;
	_anim4ActvFl = false;
	_anim5ActvFl = false;
	_anim6ActvFl = false;
	_climbThroughTrapFl = false;
	_guardFrameFl = false;
	_sitFl = false;
}

void Scene103::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsSint16LE(_jacquesAction);
	s.syncAsSint16LE(_lastRandom);
	s.syncAsSint16LE(_standPosition);
	s.syncAsSint16LE(_hotspotPrompt1);
	s.syncAsSint16LE(_hotspotPrompt2);
	s.syncAsSint16LE(_hotspotPrompt3);
	s.syncAsSint16LE(_hotspotPrompt4);
	s.syncAsSint16LE(_hotspotPrompt5);
	s.syncAsSint16LE(_hotspotRightFloor1);
	s.syncAsSint16LE(_hotspotRightFloor2);
	s.syncAsSint16LE(_hotspotLeftFloor1);
	s.syncAsSint16LE(_hotspotLeftFloor2);
	s.syncAsSint16LE(_convCount);
	s.syncAsSint16LE(_lastStairFrame);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_anim3ActvFl);
	s.syncAsByte(_anim4ActvFl);
	s.syncAsByte(_anim5ActvFl);
	s.syncAsByte(_anim6ActvFl);
	s.syncAsByte(_climbThroughTrapFl);
	s.syncAsByte(_guardFrameFl);
	s.syncAsByte(_sitFl);
}

void Scene103::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->_variant = 0;

	if ((_globals[kPrompterStandStatus] == 1) || (_globals[kCurrentYear] == 1881)) {
		_scene->_variant = 1;
		if ((_globals[kJacquesStatus] == 0) && (_globals[kCurrentYear] == 1881))
			_scene->_variant = 2;
		else if ((_globals[kJacquesStatus] >= 1) && (_globals[kCurrentYear] == 1881))
			_scene->_variant = 3;
	}

	_scene->addActiveVocab(NOUN_PROMPTERS_STAND);
	_scene->addActiveVocab(NOUN_JACQUES);
	_scene->addActiveVocab(NOUN_GENTLEMAN);
	_scene->addActiveVocab(VERB_CLIMB);
}

void Scene103::enter() {
	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim2ActvFl = false;
		_anim3ActvFl = false;
		_anim4ActvFl = false;
		_anim5ActvFl = false;
		_anim6ActvFl = false;
		_climbThroughTrapFl = false;
		_guardFrameFl = false;
		_sitFl = false;
		_jacquesAction = 1;
		_lastRandom = 0;
		_standPosition = 0;
	}

	if (_globals[kJacquesStatus] >= 1) {
		if (_game._objects.isInRoom(OBJ_KEY)) {
			_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 2));
			_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RRD_9");
		}
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 3));
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 1));
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('f', 1));
	} else {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 0));
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 3));
	}

	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 2));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RDR_6");

	adjustRails(_scene->_variant);

	_scene->_hotspots.activate(NOUN_JACQUES, false);
	_scene->_hotspots.activate(NOUN_KEY, false);

	_vm->_gameConv->load(12);

	if (_globals[kTrapDoorStatus] == 0) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 3);
	} else if (_globals[kTrapDoorStatus] == 1) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 3);
	}

	Common::Point promptPos;
	Facing promptFacing;

	if (_globals[kJacquesStatus] == 0) {
		promptPos = Common::Point(115, 142);
		promptFacing = FACING_NORTHEAST;
	} else {
		promptPos = Common::Point(171, 142);
		promptFacing = FACING_NORTHWEST;
	}

	if ((_globals[kPrompterStandStatus] == 1) || (_globals[kCurrentYear] == 1881)) {
		if (_globals[kJacquesStatus] >= 1) {
			_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);

			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(154, 139));

		} else {
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(154, 139));

			_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 1);
		}

		_hotspotPrompt1 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(121, 79, 121 + 40, 79 + 63));
		_scene->_dynamicHotspots[_hotspotPrompt1]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt1, promptPos, promptFacing);

		_hotspotPrompt2 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(161, 67, 161 + 16, 67 + 75));
		_scene->_dynamicHotspots[_hotspotPrompt2]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt2, promptPos, promptFacing);

		_hotspotPrompt3 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(177, 90, 177 + 18, 90 + 52));
		_scene->_dynamicHotspots[_hotspotPrompt3]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt3, promptPos, promptFacing);

		_hotspotPrompt4 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(114, 100, 114 + 7, 100 + 38));
		_scene->_dynamicHotspots[_hotspotPrompt4]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt4, promptPos, promptFacing);

		_hotspotPrompt5 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_CLIMB, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(121, 49, 121 + 40, 49 + 30));
		_scene->_dynamicHotspots[_hotspotPrompt5]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt5, Common::Point(196, 134), FACING_SOUTHWEST);
		_scene->_dynamicHotspots.setCursor(_hotspotPrompt5, CURSOR_GO_UP);

		_hotspotRightFloor1 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(154, 6, 154 + 41, 6 + 6));
		_scene->_dynamicHotspots[_hotspotRightFloor1]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotRightFloor1, Common::Point(171, 142), FACING_NONE);

		_hotspotRightFloor2 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(114, 136, 114 + 32, 136 + 6));
		_scene->_dynamicHotspots[_hotspotRightFloor2]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotRightFloor2, Common::Point(127, 140), FACING_NONE);

		if ((_globals[kJacquesStatus] == 0) && (_globals[kCurrentYear] == 1881)) {
			if (_globals[kJacquesNameIsKnown] >= 1) {
				_hotspotGentleman = _scene->_dynamicHotspots.add(NOUN_JACQUES, VERB_WALKTO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(156, 116, 156 + 33, 116 + 31));
				_scene->_dynamicHotspots[_hotspotGentleman]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(_hotspotGentleman, Common::Point(206, 148), FACING_NORTHWEST);
			} else {
				_hotspotGentleman = _scene->_dynamicHotspots.add(NOUN_GENTLEMAN, VERB_WALKTO, SYNTAX_MASC_NOT_PROPER, -1, Common::Rect(156, 116, 156 + 33, 116 + 31));
				_scene->_dynamicHotspots[_hotspotGentleman]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(_hotspotGentleman, Common::Point(206, 148), FACING_NORTHWEST);
			}
			int tmpIdx = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(149, 140, 149 + 13, 140 + 7));
			_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(155, 144), FACING_NONE);

			tmpIdx = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(187, 136, 187 + 8, 136 + 7));
			_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(195, 139), FACING_NONE);
		} else if ((_globals[kJacquesStatus] >= 1) && (_globals[kCurrentYear] == 1881)) {
			_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 3);
			if (_game._objects.isInRoom(OBJ_KEY)) {
				_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
				_scene->_hotspots.activate(NOUN_KEY, true);
			}
			_scene->_hotspots.activate(NOUN_JACQUES, true);
			_scene->_dynamicHotspots.remove(_hotspotRightFloor2);

			int tmpIdx = _scene->_dynamicHotspots.add(NOUN_JACQUES, VERB_WALKTO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(114, 132, 114 + 30, 132 + 10));
			_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(95, 144), FACING_NORTHEAST);

			tmpIdx = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(94, 129, 94 + 18, 129 + 4));
			_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(95, 133), FACING_NONE);

			tmpIdx = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(94, 132, 94 + 3, 132 + 9));
			_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(93, 135), FACING_NONE);

			tmpIdx = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(112, 150, 112 + 21, 150 + 3));
			_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(118, 154), FACING_NONE);

			tmpIdx = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(98, 146, 98 + 21, 146 + 4));
			_scene->_dynamicHotspots[tmpIdx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(104, 148), FACING_NONE);
		}
	} else if (_globals[kPrompterStandStatus] == 0) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(37, 139));

		_hotspotPrompt1 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(2, 79, 2 + 40, 79 + 63));
		_scene->_dynamicHotspots[_hotspotPrompt1]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt1, Common::Point(59, 140), FACING_NORTHWEST);

		_hotspotPrompt2 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(42, 67, 42 + 16, 67 + 75));
		_scene->_dynamicHotspots[_hotspotPrompt2]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt2, Common::Point(59, 140), FACING_NORTHWEST);

		_hotspotPrompt3 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(58, 90, 58 + 18, 90 + 52));
		_scene->_dynamicHotspots[_hotspotPrompt3]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt3, Common::Point(59, 140), FACING_NORTHWEST);

		_hotspotPrompt5 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_CLIMB, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(2, 49, 2 + 40, 49 + 30));
		_scene->_dynamicHotspots[_hotspotPrompt5]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotPrompt5, Common::Point(79, 132), FACING_SOUTHWEST);
		_scene->_dynamicHotspots.setCursor(_hotspotPrompt5, CURSOR_GO_UP);

		_hotspotLeftFloor1 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(35, 137, 35 + 40, 137 + 5));
		_scene->_dynamicHotspots[_hotspotLeftFloor1]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotLeftFloor1, Common::Point(59, 140), FACING_NONE);
		_hotspotLeftFloor2 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(76, 129, 76 + 6, 129 + 6));
		_scene->_dynamicHotspots[_hotspotLeftFloor2]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_hotspotLeftFloor2, Common::Point(80, 135), FACING_NONE);
	}

	if ((_globals[kJacquesStatus] == 0) && (_globals[kCurrentYear] == 1881)) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('j', 1), 1);
		_anim0ActvFl = true;
		_scene->setAnimFrame(_globals._animationIndexes[0], 9);
	}

	if (_scene->_priorSceneId == 104) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		if (_globals[kRoom103104Transition] == 0) {
			_globals[kRoom103104Transition] = 1;
			_globals._sequenceIndexes[10] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[10], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 4);
			_standPosition = 1;
			_game._player._playerPos = Common::Point(79, 132);
			_scene->_sequences.addTimer(1, 100);
		} else {
			_standPosition = 2;
			_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('w', 1), 0);
			_anim3ActvFl = true;
			_game._player._stepEnabled = true;
			_game._player._playerPos = Common::Point(196, 134);
			_scene->setAnimFrame(_globals._animationIndexes[3], 36);

			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(154, 139));
		}

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
	} else if (_scene->_priorSceneId == 102) {
		_game._player.firstWalk(Common::Point(-20, 140), FACING_EAST, Common::Point(15, 147), FACING_EAST, true);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
	} else if ((_scene->_priorSceneId == 105) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(287, 135);
		_game._player._facing = FACING_WEST;
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(252, 134), FACING_WEST);
		_game._player.setWalkTrigger(65);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
	} else if (_scene->_priorSceneId == -1) {
		if (_standPosition == 1) {
			_game._player._visible = false;
			_globals._animationIndexes[5] = _scene->loadAnimation(formAnimName('w', 3), 0);
			_anim5ActvFl = true;
			_scene->setAnimFrame(_globals._animationIndexes[5], 33);
		} else if (_standPosition == 2) {
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(154, 139));
			_game._player._visible = false;
			_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('w', 1), 0);
			_anim3ActvFl = true;
			_game._player._stepEnabled = true;
			_scene->setAnimFrame(_globals._animationIndexes[3], 36);
		} else if (_vm->_gameConv->restoreRunning() == 12) {
			_vm->_gameConv->run(12);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->exportValue(_globals[kMusicSelected]);
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		}
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
	}

	sceneEntrySound();
}

void Scene103::step() {
	if (_anim0ActvFl)
		handleJacquesAnim();

	if (_anim3ActvFl)
		climbRightStairs();

	if (_anim5ActvFl)
		climbLeftStairs();

	if (_anim4ActvFl)
		descendRightStairs();

	if (_anim6ActvFl)
		descendLeftStairs();

	if ((_globals[kWalkerConverse] == 2) || (_globals[kWalkerConverse] == 3)) {
		++_convCount;
		if (_convCount > 200)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	switch (_game._trigger) {
	case 65:
		_scene->deleteSequence(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 6);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
		_vm->_sound->command(66);
		break;

	case 66: {
		int syncIdx = _globals._sequenceIndexes[1];
		_vm->_sound->command(25);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_SEQ, syncIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_game._player._stepEnabled = true;
		}
		break;

	case 80:
		_scene->_nextSceneId = 104;
		_scene->_reloadSceneFlag = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 100:
		_scene->deleteSequence(_globals._sequenceIndexes[10]);
		_globals._sequenceIndexes[10] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[10], false, 6, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 101);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], -1, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 4);
		break;

	case 101:
		_globals._animationIndexes[5] = _scene->loadAnimation(formAnimName('w', 3), 0);
		_anim5ActvFl = true;
		_scene->setAnimFrame(_globals._animationIndexes[5], 33);
		_game._player._stepEnabled = true;
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[5], SYNC_SEQ, _globals._sequenceIndexes[10]);
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 0:
		if (_anim1ActvFl) {
			if ((_scene->getAnimFrame(_globals._animationIndexes[1]) == 10) && !_guardFrameFl) {
				_vm->_sound->command(64);
				_guardFrameFl = true;
				_scene->deleteSequence(_globals._sequenceIndexes[0]);
				_scene->deleteSequence(_globals._sequenceIndexes[9]);
				_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 105);
			}
		} else if (_anim2ActvFl) {
			if ((_scene->getAnimFrame(_globals._animationIndexes[2]) == 7) && !_guardFrameFl) {
				_vm->_sound->command(64);
				_guardFrameFl = true;
				_scene->deleteSequence(_globals._sequenceIndexes[0]);
				_scene->deleteSequence(_globals._sequenceIndexes[9]);
				_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
			}
		}
		break;

	case 105:
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 2);
		_guardFrameFl = false;
		break;

	case 110:
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_globals._sequenceIndexes[9] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[9], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 2);
		_guardFrameFl = false;
		break;

	default:
		break;
	}

	if (_globals[kJacquesStatus] == 3) {
		warning("TODO: add a check on the return value of _vm->_sound->command ???");
		_vm->_sound->command(38);
	}
}

void Scene103::preActions() {
	if (_action.isAction(VERB_OPEN, NOUN_DOOR))
		_game._player._needToWalk = true;

	if (_action.isAction(VERB_OPEN, NOUN_DOOR_TO_PIT)) {
		_game._player.walk(Common::Point(0, 150), FACING_WEST);
		_game._player._walkOffScreenSceneId = 102;
	}

	if ((_standPosition != 0) && !_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR) &&
		!_action.isAction(VERB_OPEN, NOUN_TRAP_DOOR) &&
		!_action.isAction(VERB_CLIMB, NOUN_PROMPTERS_STAND) &&
		!_action.isAction(VERB_PUSH, NOUN_TRAP_DOOR) &&
		!_action.isAction(VERB_LOOK_THROUGH, NOUN_PROMPTERS_BOX) &&
		!_action.isAction(VERB_PULL, NOUN_TRAP_DOOR)) {

		if (_action.isAction(VERB_PULL) || _action.isAction(VERB_PUSH)) {
			if (!_action.isObject(NOUN_LEVER) && !_game._trigger)
				_game._player._needToWalk = false;
		}

		if ((_action.isAction(VERB_PUSH, NOUN_PROMPTERS_STAND)) || (_action.isAction(VERB_PULL, NOUN_PROMPTERS_STAND))) {
			if (!_game._trigger)
				_game._player._needToWalk = true;
		}

		if (_game._player._needToWalk) {
			if (_globals[kPrompterStandStatus] == 0) {
				switch (_game._trigger) {
				case 0:
					_scene->freeAnimation(5);
					_game._player._readyToWalk = false;
					_game._player._visible = false;
					_game._player._stepEnabled = false;
					_anim5ActvFl = false;
					_anim6ActvFl = true;
					_globals._animationIndexes[6] = _scene->loadAnimation(formAnimName('w', 4), 1);
					break;

				case 1:
					_game._player._playerPos = Common::Point(79, 132);
					_game._player._stepEnabled = true;
					_game._player._visible = true;
					_standPosition = 0;
					_game._player._readyToWalk = true;
					if (_action.isAction(VERB_PUSH, NOUN_PROMPTERS_STAND))
						_game._player._needToWalk = false;
					_anim6ActvFl = false;
					_game._player.resetFacing(FACING_NORTHEAST);
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[6]);
					break;

				default:
					break;
				}
			} else {
				switch (_game._trigger) {
				case 0:
					_scene->freeAnimation(3);
					_game._player._readyToWalk = false;
					if (_action.isAction(VERB_PUSH, NOUN_PROMPTERS_STAND))
						_game._player._needToWalk = true;
					_game._player._visible = false;
					_game._player._stepEnabled = false;
					_anim3ActvFl = false;
					_anim4ActvFl = true;
					_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('w', 2), 1);
					break;

				case 1:
					_game._player._playerPos = Common::Point(196, 134);
					_game._player._stepEnabled = true;
					_game._player._readyToWalk = true;
					if (_action.isAction(VERB_PUSH, NOUN_PROMPTERS_STAND))
						_game._player._needToWalk = false;
					_game._player._visible = true;
					_standPosition = 0;
					_anim4ActvFl = false;
					_game._player.resetFacing(FACING_NORTHEAST);
					_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[4]);
					break;

				default:
					break;
				}
			}
		}
	}

	if (_standPosition == 0) {
		if ((_action.isAction(VERB_PUSH, NOUN_PROMPTERS_STAND)) || (_action.isAction(VERB_PULL, NOUN_PROMPTERS_STAND))) {
			if (_globals[kCurrentYear] == 1993) {
				if (_globals[kPrompterStandStatus] == 0)
					_game._player.walk(Common::Point(2, 138), FACING_WEST);
				else
					_game._player.walk(Common::Point(176, 142), FACING_WEST);
			}
		}
	}

	if (_action.isAction(VERB_LOOK_THROUGH, NOUN_PROMPTERS_BOX) && (_standPosition == 0) && (_globals[kPrompterStandStatus] == 0) && (_globals[kCurrentYear] == 1993))
		_game._player.walk(Common::Point(79, 132), FACING_SOUTHWEST);

	if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR) && (_standPosition == 0) && (_globals[kPrompterStandStatus] == 1) && (_globals[kTrapDoorStatus] == 0))
		_game._player.walk(Common::Point(196, 134), FACING_SOUTHWEST);

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_PIT))
		_game._player._walkOffScreenSceneId = 102;

	if ((_action.isAction(VERB_OPEN, NOUN_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_DOOR) || _action.isAction(VERB_LOCK, NOUN_DOOR)) && (_standPosition == 0))
		_game._player.walk(Common::Point(252, 134), FACING_EAST);
}

void Scene103::actions() {
	if (_action.isAction(VERB_OPEN, NOUN_DOOR_TO_PIT)) {
		_scene->_nextSceneId = 102;
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 12) {
		process_conv_jacques();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)) {
		if (_globals[kTrapDoorStatus] == 1) {
			_vm->_dialogs->show(10333);
			_action._inProgress = false;
			return;
		} else if (_globals[kPrompterStandStatus] == 0) {
			_vm->_dialogs->show(10341);
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isAction(VERB_LOOK_THROUGH, NOUN_PROMPTERS_BOX)) && (_globals[kPrompterStandStatus] == 1)) {
		_vm->_dialogs->show(10342);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_CLIMB, NOUN_PROMPTERS_STAND) && _standPosition == 0) ||
		(_action.isAction(VERB_LOOK_THROUGH, NOUN_PROMPTERS_BOX) && (_standPosition == 0)) ||
		(_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR) && (_standPosition == 0))) {
		if (_globals[kPrompterStandStatus] == 0) {
			if (_game._trigger == 0) {
				if (_action.isAction(VERB_LOOK_THROUGH, NOUN_PROMPTERS_BOX)) {
					_sitFl = true;
					_globals._animationIndexes[5] = _scene->loadAnimation(formAnimName('w', 3), 115);
				} else
					_globals._animationIndexes[5] = _scene->loadAnimation(formAnimName('w', 3), 0);

				_game._player._visible = false;
				_game._player._stepEnabled = false;
				_anim5ActvFl = true;
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[5], SYNC_PLAYER, 0);
				_action._inProgress = false;
				return;
			}
		} else if (_game._trigger == 0) {
			if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)) {
				_climbThroughTrapFl = true;
				_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('w', 1), 120);
			} else
				_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('w', 1), 0);

			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_anim3ActvFl = true;
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[3], SYNC_PLAYER, 0);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_CLIMB, NOUN_PROMPTERS_STAND) && (_standPosition != 0)) {
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH, NOUN_PROMPTERS_STAND)) || (_action.isAction(VERB_PULL, NOUN_PROMPTERS_STAND))) {
		if (_globals[kCurrentYear] == 1993) {
			if (_globals[kPrompterStandStatus] == 0) {
				switch (_game._trigger) {
				case 0:
					if (_globals[kPrompterStandStatus] == 0) {
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('s', 1), 75);
						_game._player._priorTimer = _scene->_frameStartTime;
						_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
						_scene->deleteSequence(_globals._sequenceIndexes[3]);
					}
					break;

				case 75:
					if (_globals[kPrompterStandStatus] == 0) {
						_globals._sequenceIndexes[12] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[12], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 1);
						_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
						_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(154, 139));
						_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_ANIM, _globals._animationIndexes[0]);
						_globals[kPrompterStandStatus] = 1;
						_game._player._stepEnabled = true;
						_game._player._visible = true;
						_game._player._playerPos = Common::Point(117, 139);
						_game._player.resetFacing(FACING_EAST);
						_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);

						_scene->_dynamicHotspots.remove(_hotspotPrompt1);
						_scene->_dynamicHotspots.remove(_hotspotPrompt2);
						_scene->_dynamicHotspots.remove(_hotspotPrompt3);
						_scene->_dynamicHotspots.remove(_hotspotPrompt5);
						_scene->_dynamicHotspots.remove(_hotspotLeftFloor1);
						_scene->_dynamicHotspots.remove(_hotspotLeftFloor2);

						_hotspotPrompt1 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(121, 79, 121 + 40, 79 + 63));
						_scene->_dynamicHotspots[_hotspotPrompt1]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt1, Common::Point(171, 142), FACING_NORTHWEST);

						_hotspotPrompt2 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(161, 67, 161 + 16, 67 + 75));
						_scene->_dynamicHotspots[_hotspotPrompt2]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt2, Common::Point(171, 142), FACING_NORTHWEST);

						_hotspotPrompt3 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(177, 90, 177 + 18, 90 + 52));
						_scene->_dynamicHotspots[_hotspotPrompt3]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt3, Common::Point(171, 142), FACING_NORTHWEST);

						_hotspotPrompt4 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(114, 100, 114 + 7, 100 + 38));
						_scene->_dynamicHotspots[_hotspotPrompt4]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt4, Common::Point(171, 142), FACING_NORTHWEST);

						_hotspotPrompt5 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_CLIMB, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(121, 49, 121 + 40, 49 + 30));
						_scene->_dynamicHotspots[_hotspotPrompt5]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt5, Common::Point(196, 134), FACING_SOUTHWEST);
						_scene->_dynamicHotspots.setCursor(_hotspotPrompt5, CURSOR_GO_UP);

						_hotspotRightFloor1 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(154, 6, 154 + 41, 6 + 6));
						_scene->_dynamicHotspots[_hotspotRightFloor1]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotRightFloor1, Common::Point(171, 142), FACING_NONE);

						_hotspotRightFloor2 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(114, 136, 114 + 32, 132 + 6));
						_scene->_dynamicHotspots[_hotspotRightFloor2]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotRightFloor2, Common::Point(127, 140), FACING_NONE);

						_scene->changeVariant(1);
						adjustRails(1);
					}
					break;

				default:
					break;
				}
			} else {
				switch (_game._trigger) {
				case 0:
					if (_globals[kPrompterStandStatus] == 1) {
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('s', 2), 75);
						_game._player._priorTimer = _scene->_frameStartTime;
						_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
						_scene->deleteSequence(_globals._sequenceIndexes[12]);
						_scene->deleteSequence(_globals._sequenceIndexes[3]);
					}
					break;

				case 75:
					if (_globals[kPrompterStandStatus] == 1) {
						_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
						_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(37, 139));
						_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_ANIM, _globals._animationIndexes[0]);
						_globals[kPrompterStandStatus] = 0;
						_game._player._stepEnabled = true;
						_game._player._visible = true;
						_game._player._playerPos = Common::Point(62, 142);
						_game._player.resetFacing(FACING_WEST);
						_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
						_scene->_dynamicHotspots.remove(_hotspotPrompt1);
						_scene->_dynamicHotspots.remove(_hotspotPrompt2);
						_scene->_dynamicHotspots.remove(_hotspotPrompt3);
						_scene->_dynamicHotspots.remove(_hotspotPrompt4);
						_scene->_dynamicHotspots.remove(_hotspotPrompt5);
						_scene->_dynamicHotspots.remove(_hotspotRightFloor1);
						_scene->_dynamicHotspots.remove(_hotspotRightFloor2);

						_hotspotPrompt1 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(2, 79, 2 + 40, 79 + 63));
						_scene->_dynamicHotspots[_hotspotPrompt1]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt1, Common::Point(59, 140), FACING_NORTHWEST);

						_hotspotPrompt2 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(42, 67, 42 + 16, 67 + 75));
						_scene->_dynamicHotspots[_hotspotPrompt2]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt2, Common::Point(59, 140), FACING_NORTHWEST);

						_hotspotPrompt3 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(58, 90, 58 + 18, 90 + 52));
						_scene->_dynamicHotspots[_hotspotPrompt3]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt3, Common::Point(59, 140), FACING_NORTHWEST);

						_hotspotLeftFloor1 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(35, 137, 35 + 40, 137 + 5));
						_scene->_dynamicHotspots[_hotspotLeftFloor1]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotLeftFloor1, Common::Point(59, 140), FACING_NONE);

						_hotspotLeftFloor2 = _scene->_dynamicHotspots.add(NOUN_FLOOR, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(76, 129, 76 + 6, 129 + 6));
						_scene->_dynamicHotspots[_hotspotLeftFloor2]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotLeftFloor2, Common::Point(80, 135), FACING_NONE);

						_hotspotPrompt5 = _scene->_dynamicHotspots.add(NOUN_PROMPTERS_STAND, VERB_CLIMB, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(2, 49, 2 + 40, 49 + 30));
						_scene->_dynamicHotspots[_hotspotPrompt5]._articleNumber = PREP_ON;
						_scene->_dynamicHotspots.setPosition(_hotspotPrompt5, Common::Point(79, 132), FACING_SOUTHWEST);
						_scene->_dynamicHotspots.setCursor(_hotspotPrompt5, CURSOR_GO_UP);

						_scene->changeVariant(0);
						adjustRails(0);
					}
					break;

				default:
					break;
				}
			}
		} else if (_globals[kJacquesNameIsKnown])
			_vm->_dialogs->show(10340);
		else
			_vm->_dialogs->show(10350);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_DOOR) || _action.isAction(VERB_LOCK, NOUN_DOOR)) {
		if ((_globals[kCurrentYear] == 1881) && !_action.isAction(VERB_LOCK, NOUN_DOOR) && !_action.isAction(VERB_UNLOCK, NOUN_DOOR)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 3, 70);
				break;

			case 2:
				_game._player._visible = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[2]);
				_game._player.walk(Common::Point(295, 132), FACING_WEST);
				_scene->_sequences.addTimer(180, 3);
				break;

			case 3:
				_scene->_nextSceneId = 105;
				break;

			case 70:
				_vm->_sound->command(24);
				_scene->deleteSequence(_globals._sequenceIndexes[1]);
				_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
				_vm->_sound->command(66);
				break;

			case 71: {
				int oldIdx = _globals._sequenceIndexes[1];
				_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -2);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_SEQ, oldIdx);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
				}
				break;

			default:
				break;
			}
		} else {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1: {
				int oldIdx = _globals._sequenceIndexes[2];
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 4);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[2], SYNC_SEQ, oldIdx);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], false);
				_scene->_sequences.addTimer(15, 2);
				_vm->_sound->command(73);
				}
				break;

			case 2:
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				if (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK))
					_vm->_dialogs->show(00032);
				else
					_vm->_dialogs->show(10335);

				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_KEY) && (_game._objects.isInRoom(OBJ_KEY) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 5);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 5, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_action._inProgress = false;
			return;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[5]);
			_scene->_hotspots.activate(NOUN_KEY, false);
			_game._objects.addToInventory(OBJ_KEY);
			_vm->_sound->command(26);
			_action._inProgress = false;
			return;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			_action._inProgress = false;
			return;

		case 3:
			_globals[kPlayerScore] += 5;
			_vm->_dialogs->showItem(OBJ_KEY, 800, 0);
			_game._player._stepEnabled = true;
			_action._inProgress = false;
			return;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_GENTLEMAN) || _action.isAction(VERB_TALK_TO, NOUN_JACQUES)) {
		if (_globals[kJacquesStatus] == 0) {
			_vm->_gameConv->run(12);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->exportValue(_globals[kMusicSelected]);
		} else
			_vm->_dialogs->show(10343);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR) || _climbThroughTrapFl) {
		if ((_standPosition == 2) && (_globals[kTrapDoorStatus] == 0)) {
			switch (_game._trigger) {
			case 0:
			case 120:
				if (!(_globals[kPlayerScoreFlags] & 1)) {
					_globals[kPlayerScoreFlags] |= 1;
					_globals[kPlayerScore] += 3;
				}

				_scene->freeAnimation(3);
				_anim3ActvFl = false;
				_climbThroughTrapFl = false;
				_game._player._stepEnabled = false;
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 5);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals[kRoom103104Transition] = 1;
				_scene->_nextSceneId = 104;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK_THROUGH, NOUN_PROMPTERS_BOX) || _sitFl) {
		if (_standPosition == 1) {
			switch (_game._trigger) {
			case 0:
			case 115:
				_scene->freeAnimation(5);
				_anim5ActvFl = false;
				_game._player._stepEnabled = false;
				_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 4);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				if (!_sitFl)
					_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[10], SYNC_ANIM, _globals._animationIndexes[5]);
				_sitFl = false;
				break;

			case 1:
				_globals[kRoom103104Transition] = 0;
				_scene->_nextSceneId = 104;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUSH, NOUN_LEVER) || _action.isAction(VERB_PULL, NOUN_LEVER)) {
		if (_globals[kTrapDoorStatus] == 1) {
			switch (_game._trigger) {
			case 0:
				_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('l', 1), 1);
				_anim1ActvFl = true;
				_game._player._visible = false;
				_game._player._stepEnabled = false;
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[1], SYNC_PLAYER, 0);
				break;

			case 1:
				_anim1ActvFl = false;
				_game._player._visible = true;
				_globals[kTrapDoorStatus] = 0;
				_game._player._stepEnabled = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
				break;

			default:
				break;
			}
		} else {
			switch (_game._trigger) {
			case 0:
				_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('l', 2), 1);
				_anim2ActvFl = true;
				_game._player._visible = false;
				_game._player._stepEnabled = false;
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[2], SYNC_PLAYER, 0);
				break;

			case 1:
				_anim2ActvFl = false;
				_game._player._visible = true;
				_globals[kTrapDoorStatus] = 1;
				_game._player._stepEnabled = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[2]);
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if ((_globals[kCurrentYear] == 1881) && (_globals[kJacquesStatus] >= 1))
			_vm->_dialogs->show(10311);
		else
			_vm->_dialogs->show(10310);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_PROMPTERS_STAND)) {
			if (_globals[kJacquesStatus] >= 1)
				_vm->_dialogs->show(10349);
			else if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10312);
			else
				_vm->_dialogs->show(10345);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(10313);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TRAP_CEILING)) {
			_vm->_dialogs->show(10314);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(10315);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_TO_PIT)) {
			_vm->_dialogs->show(10316);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10317);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROMPTERS_BOX)) {
			_vm->_dialogs->show(10318);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TRAP_DOOR)) {
			_vm->_dialogs->show(10319);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_JUNK)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10320);
			else
				_vm->_dialogs->show(10346);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CARTON)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10321);
			else
				_vm->_dialogs->show(10347);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GARBAGE_CAN)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10322);
			else
				_vm->_dialogs->show(10348);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CABLE)) {
			_vm->_dialogs->show(10323);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_JACQUES) || _action.isObject(NOUN_GENTLEMAN)) {
			if (_globals[kJacquesStatus] == 0)
				_vm->_dialogs->show(10324);
			else
				_vm->_dialogs->show(10325);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_KEY) && _game._objects.isInRoom(OBJ_KEY)) {
			_vm->_dialogs->show(10326);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAIR_UNIT)) {
			_vm->_dialogs->show(10327);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXPOSED_BRICK)) {
			_vm->_dialogs->show(10328);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WATER_PIPE)) {
			_vm->_dialogs->show(10329);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROMPTERS_SEAT)) {
			_vm->_dialogs->show(10338);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEVER)) {
			_vm->_dialogs->show(10339);
			_action._inProgress = false;
			return;
		}

	}

	if (_action.isAction(VERB_CLOSE, NOUN_DOOR_TO_PIT)) {
		_vm->_dialogs->show(10331);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_DOOR)) {
		_vm->_dialogs->show(10331);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_OPEN, NOUN_TRAP_DOOR)) || (_action.isAction(VERB_CLOSE, NOUN_TRAP_DOOR))) {
		_vm->_dialogs->show(10344);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_JACQUES) || _action.isAction(VERB_TAKE, NOUN_GENTLEMAN)) {
		if (_globals[kJacquesStatus] == 0) {
			if (_globals[kJacquesNameIsKnown])
				_vm->_dialogs->show(10336);
			else
				_vm->_dialogs->show(10351);
		} else
			_vm->_dialogs->show(10337);
		_action._inProgress = false;
		return;
	}
}

void Scene103::adjustRails(int variant) {
	switch (variant) {
	case 0:
		_scene->_rails.disableNode(3);
		_scene->_rails.disableNode(4);
		_scene->_rails.disableNode(5);
		_scene->_rails.disableNode(6);
		_scene->_rails.disableNode(12);
		_scene->_rails.disableNode(13);
		_scene->_rails.disableNode(14);
		break;

	case 1:
		_scene->_rails.disableNode(1);
		_scene->_rails.disableNode(2);
		_scene->_rails.disableNode(3);
		_scene->_rails.disableNode(4);
		_scene->_rails.disableNode(5);
		_scene->_rails.disableNode(6);
		_scene->_rails.disableNode(7);
		_scene->_rails.disableNode(9);
		_scene->_rails.disableNode(10);
		_scene->_rails.disableNode(11);
		break;

	case 2:
		_scene->_rails.disableNode(1);
		_scene->_rails.disableNode(2);
		_scene->_rails.disableNode(5);
		_scene->_rails.disableNode(6);
		_scene->_rails.disableNode(7);
		_scene->_rails.disableNode(9);
		_scene->_rails.disableNode(10);
		_scene->_rails.disableNode(11);
		break;

	case 3:
		_scene->_rails.disableNode(1);
		_scene->_rails.disableNode(2);
		_scene->_rails.disableNode(3);
		_scene->_rails.disableNode(4);
		_scene->_rails.disableNode(10);
		_scene->_rails.disableNode(11);
		break;

	default:
		break;
	}
}

void Scene103::handleJacquesAnim() {
	if (_scene->getAnimFrame(_globals._animationIndexes[0]) == _lastJacquesFrame)
		return;

	_lastJacquesFrame = _scene->getAnimFrame(_globals._animationIndexes[0]);
	int resetFrame = -1;
	int random;

	switch (_lastJacquesFrame) {
	case 1:
	case 2:
	case 3:
	case 9:
	case 17:
	case 23:
	case 33:
	case 51:
		switch (_jacquesAction) {
		case 2:
			random = 4;
			_jacquesAction = 0;
			break;

		case 3:
			random = 5;
			_jacquesAction = 0;
			break;

		case 4:
			random = 6;
			break;

		case 0:
			random = _vm->getRandomNumber(1, 3);
			++_talkCount;
			if (_talkCount > 22) {
				_jacquesAction = 1;
				random = 9;
			}
			break;

		default:
			random = _vm->getRandomNumber(6, 50);
			while (_lastRandom == random)
				random = _vm->getRandomNumber(6, 50);

			_lastRandom = random;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 0;
			break;

		case 2:
			resetFrame = 1;
			break;

		case 3:
			resetFrame = 2;
			break;

		case 4:
			resetFrame = 4;
			break;

		case 5:
			resetFrame = 10;
			break;

		case 6:
			resetFrame = 34;
			break;

		case 7:
			resetFrame = 24;
			break;

		case 8:
			resetFrame = 18;
			break;

		default:
			resetFrame = 8;
			break;
		}
		break;

	case 36:
	case 40:
	case 48:
		switch (_jacquesAction) {
		case 0:
		case 2:
		case 3:
			random = 2;
			break;

		case 4:
			random = 1;
			_jacquesAction = 0;
			break;

		default:
			random = _vm->getRandomNumber(2, 50);
			while (_lastRandom == random)
				random = _vm->getRandomNumber(2, 50);
			_lastRandom = random;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 37;
			break;

		case 2:
			resetFrame = 49;
			break;

		case 3:
			resetFrame = 41;
			break;

		default:
			resetFrame = 35;
			break;
		}
		break;

	case 44:
		random = _vm->getRandomNumber(1, 50);
		while (_lastRandom == random)
			random = _vm->getRandomNumber(1, 50);

		_lastRandom = random;

		switch (_jacquesAction) {
		case 0:
		case 2:
		case 3:
		case 4:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			while (_lastRandom == random)
				random = _vm->getRandomNumber(1, 50);
			_lastRandom = random;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 45;
			break;

		default:
			resetFrame = 43;
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_lastJacquesFrame = resetFrame;
	}
}

void Scene103::climbRightStairs() {
	if (_scene->getAnimFrame(_globals._animationIndexes[3]) == _lastStairFrame)
		return;

	_lastStairFrame = _scene->getAnimFrame(_globals._animationIndexes[3]);
	int stairsResetFrame = -1;

	if (_lastStairFrame == 37) {
		stairsResetFrame = 36;
		_standPosition = 2;
		_game._player._stepEnabled = true;
	}

	if (_lastStairFrame == 2) {
		_scene->deleteSequence(3);
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(154, 139));
	}

	if (stairsResetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[3], stairsResetFrame);
		_lastStairFrame = stairsResetFrame;
	}
}

void Scene103::climbLeftStairs() {
	if (_scene->getAnimFrame(_globals._animationIndexes[5]) == _lastStairFrame)
		return;

	_lastStairFrame = _scene->getAnimFrame(_globals._animationIndexes[5]);
	int stairsResetFrame = -1;

	if (_lastStairFrame == 34) {
		stairsResetFrame = 33;
		_standPosition = 1;
		_game._player._stepEnabled = true;
	}

	if (_lastStairFrame == 2) {
		_scene->deleteSequence(3);
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(37, 139));
	}

	if (stairsResetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[5], stairsResetFrame);
		_lastStairFrame = stairsResetFrame;
	}
}

void Scene103::descendRightStairs() {
	if (_scene->getAnimFrame(_globals._animationIndexes[4]) == _lastStairFrame)
		return;

	_lastStairFrame = _scene->getAnimFrame(_globals._animationIndexes[4]);

	if (_lastStairFrame == 2) {
		_scene->deleteSequence(3);
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(154, 139));
	}
}

void Scene103::descendLeftStairs() {
	if (_scene->getAnimFrame(_globals._animationIndexes[6]) == _lastStairFrame)
		return;

	_lastStairFrame = _scene->getAnimFrame(_globals._animationIndexes[6]);

	if (_lastStairFrame == 2) {
		_scene->deleteSequence(3);
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(37, 139));
	}
}

void Scene103::process_conv_jacques() {
	bool quitConversationFl = false;

	switch (_action._activeAction._verbId) {
	case 1:
		_vm->_gameConv->setInterlocutorTrigger(96);
		if (_globals[kJacquesNameIsKnown] == 0) {
			_globals[kJacquesNameIsKnown] = 1;
			_scene->_dynamicHotspots.remove(_hotspotGentleman);
			_hotspotGentleman = _scene->_dynamicHotspots.add(NOUN_JACQUES, VERB_WALKTO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(156, 116, 156 + 33, 116 + 31));
			_scene->_dynamicHotspots[_hotspotGentleman]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(_hotspotGentleman, Common::Point(206, 148), FACING_NORTHWEST);
		}
		break;

	case 4:
	case 6:
	case 7:
	case 10:
	case 19:
	case 30: {
		_globals[kWalkerConverse] = 0;
		int *val = _vm->_gameConv->getVariable(26);
		if (*val)
			_globals[kJacquesNameIsKnown] = 2;

		quitConversationFl = true;
		}
		break;

	case 8:
		_vm->_gameConv->setInterlocutorTrigger(94);
		break;

	case 12:
		_vm->_gameConv->setInterlocutorTrigger(96);
		break;

	case 29:
		_vm->_gameConv->setInterlocutorTrigger(98);
		break;

	default:
		break;
	}

	if ((_action._activeAction._verbId != 1) && (_action._activeAction._verbId != 8)
	 && (_action._activeAction._verbId != 12) && (_action._activeAction._verbId != 29))
		_vm->_gameConv->setInterlocutorTrigger(90);

	_vm->_gameConv->setHeroTrigger(92);

	switch (_game._trigger) {
	case 90:
		if (!quitConversationFl)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_jacquesAction = 0;
		break;

	case 92:
		if (!quitConversationFl)
			_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);
		_convCount = 0;
		_jacquesAction = 1;
		break;

	case 94:
		if (!quitConversationFl)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_jacquesAction = 2;
		break;

	case 96:
		if (!quitConversationFl)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_jacquesAction = 3;
		break;

	case 98:
		if (!quitConversationFl)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_jacquesAction = 4;
		break;

	default:
		break;
	}

	_talkCount = 0;
}

/*------------------------------------------------------------------------*/

Scene104::Scene104(MADSEngine *vm) : Scene1xx(vm) {
	_anim0ActvFl = _anim1ActvFl = _anim2ActvFl = false;
	_needToTalk = false;
	_needToGetUp = false;
	_sittingUp = false;
	_beforeHeLeaves = false;
	_beforeSheLeaves = false;
	_needToStandUp = false;

	_walkStatus = -1;
	_walkFrame = -1;
	_coupleStatus = -1;
	_richStatus = -1;
	_richTalkCount = -1;
	_manTalkCount = -1;
	_womanTalkCount = -1;
	_lookCount = -1;
	_coupleFrame = -1;
	_lastPlayerFrame = -1;
	_richFrame = -1;
}

void Scene104::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_needToTalk);
	s.syncAsByte(_needToGetUp);
	s.syncAsByte(_sittingUp);
	s.syncAsByte(_beforeHeLeaves);
	s.syncAsByte(_beforeSheLeaves);
	s.syncAsByte(_needToStandUp);

	s.syncAsSint16LE(_walkStatus);
	s.syncAsSint16LE(_walkFrame);
	s.syncAsSint16LE(_coupleStatus);
	s.syncAsSint16LE(_richStatus);
	s.syncAsSint16LE(_richTalkCount);
	s.syncAsSint16LE(_manTalkCount);
	s.syncAsSint16LE(_womanTalkCount);
	s.syncAsSint16LE(_lookCount);
	s.syncAsSint16LE(_coupleFrame);
	s.syncAsSint16LE(_lastPlayerFrame);
}

void Scene104::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kTrapDoorStatus] == 1)
		_scene->_variant = 1;

	_scene->addActiveVocab(NOUN_MONSIEUR_RICHARD);
}

void Scene104::enter() {
	_vm->_disableFastwalk = true;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim2ActvFl = false;
		_needToTalk = false;
		_needToGetUp = false;
		_sittingUp = false;
		_beforeSheLeaves = false;
		_needToStandUp = false;
	}

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 6));
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));

	if (_globals[kCurrentYear] == 1993)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('z', 0));

	_vm->_gameConv->load(7);

	if (_globals[kTrapDoorStatus] == 1) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 15);
	} else {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 15);
	}

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_vm->_gameConv->restoreRunning() == 7) {
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('r', 1), 1);
			_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('d', 1), 1);
			_walkStatus = 0;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('m', 1), 1);
			if (_coupleStatus < 11) {
				_coupleStatus = 1;
				_richStatus = 0;
				_scene->setAnimFrame(_globals._animationIndexes[1], 14);
			} else {
				_coupleStatus = 17;
				_richStatus = 4;
				_scene->setAnimFrame(_globals._animationIndexes[1], 105);
				_scene->setAnimFrame(_globals._animationIndexes[0], 216);
			}

			_vm->_gameConv->run(7);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_game._player._visible = false;
		}

	} else if (_scene->_priorSceneId == 301) {
		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('r', 1), 1);
		_anim1ActvFl = true;
		_coupleStatus = 11;

		_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('d', 1), 1);
		_anim2ActvFl = true;
		_walkStatus = 0;

		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('m', 1), 1);
		_anim0ActvFl = true;
		_richStatus = 0;

		cleanInventory();

		_game._player._visible = false;
		_game._visitedScenes.pop_back();
		_globals[kTrapDoorStatus] = 1;
		_globals[kCurrentYear] = 1881;
		_globals[kPrompterStandStatus] = 1;
		_globals[kTicketPeoplePresent] = 1;
		_globals[kMakeBrieLeave203] = false;
		_game._player._playerPos.x = 161;

		_game._visitedScenes.add(301);
		_game._visitedScenes.add(101);

		_scene->setCamera(Common::Point(60, 0));
		_scene->_sequences.addTimer(1, 91);

	} else if (_scene->_priorSceneId == 103) {
		if (_globals[kRoom103104Transition] == 0) {
			_scene->_userInterface.emptyConversationList();
			_scene->_userInterface.setup(kInputConversation);

			if (!_globals[kObservedPhan104]) {
				_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('p', 1), 93);
				_game._player._playerPos.x = 319;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_scene->setCamera(Common::Point(158, 0));
			} else {
				_globals._animationIndexes[5] = _scene->loadAnimation(formAnimName('p', 2), 94);
				_game._player._playerPos.x = 319;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_scene->setCamera(Common::Point(158, 0));
			}
		} else {
			_game._player._playerPos = Common::Point(319, 96);
			_game._player._facing = FACING_SOUTH;
			_scene->setCamera(Common::Point(158, 0));
		}
	} else if (_scene->_priorSceneId == 102) {
		switch (_globals[kDeathLocation]) {
		case 0:
			_game._player._playerPos = Common::Point(496, 79);
			_scene->setCamera(Common::Point(320, 0));
			break;

		case 1:
			_game._player._playerPos = Common::Point(346, 71);
			_scene->setCamera(Common::Point(158, 0));
			break;

		case 2:
			_game._player._playerPos = Common::Point(172, 73);
			break;

		default:
			break;
		}
	} else if (_scene->_priorSceneId == 108) {
		if (_game._player._playerPos.x > 213)
			_game._player._playerPos.y = 97;
		else if (_game._player._playerPos.x > 110)
			_game._player._playerPos.y = 128;
		else
			_game._player._playerPos.y = 148;

		_game._player.firstWalk(Common::Point(-20, _game._player._playerPos.y), FACING_EAST, Common::Point(12, _game._player._playerPos.y), FACING_EAST, true);
	} else if ((_scene->_priorSceneId == 107) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		if (_game._player._playerPos.x > 191)
			_game._player._playerPos.y = 142;
		else if (_game._player._playerPos.x > 104)
			_game._player._playerPos.y = 120;
		else
			_game._player._playerPos.y = 95;

		_game._player.firstWalk(Common::Point(655, _game._player._playerPos.y), FACING_WEST, Common::Point(627, _game._player._playerPos.y), FACING_WEST, true);
		_scene->setCamera(Common::Point(320, 0));
	}

	if (_globals[kCurrentYear] == 1993) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else
		_scene->_hotspots.activate(NOUN_CHANDELIER, false);

	if (_globals[kTrapDoorStatus] == 1) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 15);
	} else {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 15);
	}

	sceneEntrySound();
}

void Scene104::step() {
	if (_anim0ActvFl)
		handleRichAnimations();

	if (_anim1ActvFl)
		handleCoupleAnimations();

	if (_anim2ActvFl)
		handleWalkAnimation();

	if (_game._player._moving)
		handlePlayerWalk();

	if (_game._trigger == 91) {
		_vm->_dialogs->show(10434);
		_vm->_gameConv->run(7);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
	}

	if (_game._trigger == 93) {
		_scene->_nextSceneId = 103;
		_game._player._playerPos.x = 400;
		_globals[kRoom103104Transition] = 0;
	}

	if (_game._trigger == 94) {
		_scene->_nextSceneId = 103;
		_globals[kRoom103104Transition] = 0;
	}
}

void Scene104::preActions() {
	if (_action.isAction(VERB_EXIT, NOUN_STAGE_LEFT))
		_game._player._walkOffScreenSceneId = 108;

	if (_action.isAction(VERB_EXIT, NOUN_STAGE_RIGHT))
		_game._player._walkOffScreenSceneId = 107;

	if (_action.isAction(VERB_OPEN, NOUN_TRAP_DOOR) || _action.isAction(VERB_CLOSE, NOUN_TRAP_DOOR))
		_game._player.walk(Common::Point(320, 92), FACING_NORTH);
}

void Scene104::actions() {
	if (_vm->_gameConv->activeConvId() == 7) {
		processConversations();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_THROUGH, NOUN_TRAP_DOOR)) {
		if (_globals[kTrapDoorStatus] == 0) {
			switch (_game._trigger) {
			case 0:
				_game._player._visible = false;
				_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 13);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 16);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
				break;

			case 60:
				_scene->_nextSceneId = 103;
				_globals[kRoom103104Transition] = 1;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else {
			_vm->_dialogs->show(10429);
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_JUMP_INTO, NOUN_ORCHESTRA_PIT)) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._playerPos.x > 400)
				_globals[kDeathLocation] = 0;
			else if (_game._player._playerPos.x > 200)
				_globals[kDeathLocation] = 1;
			else
				_globals[kDeathLocation] = 2;

			_scene->changeVariant(2);

			if (_globals[kTrapDoorStatus] == 1)
				_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
			else
				_scene->drawToBackground(_globals._spriteIndexes[0], 2, Common::Point(-32000, -32000), 0, 100);

			_vm->_dialogs->show(10426);
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, 4);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], _game._player._playerPos);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 1, 10);
			_scene->_sequences.addTimer(60, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 4);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[3], true);
			_scene->_sequences.setMotion(_globals._sequenceIndexes[3], 0, 0, 200);
			break;

		case 2:
			_vm->_sound->command(1);
			_vm->_sound->command(67);
			_scene->_nextSceneId = 102;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(10410);
		else
			_vm->_dialogs->show(10411);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(10412);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_APRON)) {
			_vm->_dialogs->show(10413);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROSCENIUM_ARCH)) {
			_vm->_dialogs->show(10414);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ACT_CURTAIN)) {
			_vm->_dialogs->show(10415);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ORCHESTRA_PIT)) {
			_vm->_dialogs->show(10416);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CONDUCTORS_STAND)) {
			_vm->_dialogs->show(10417);
			_action._inProgress = false;
			return;
		}

		if ((_action.isObject(NOUN_MUSIC_STAND)) || (_action.isObject(NOUN_MUSIC_STANDS))) {
			_vm->_dialogs->show(10418);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROMPTERS_BOX)) {
			_vm->_dialogs->show(10419);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TRAP_DOOR)) {
			_vm->_dialogs->show(10420);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOUSE)) {
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->show(10421);
			else
				_vm->_dialogs->show(10427);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE_LEFT)) {
			_vm->_dialogs->show(10422);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE_RIGHT)) {
			_vm->_dialogs->show(10423);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHANDELIER)) {
			_vm->_dialogs->show(10428);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MONSIEUR_RICHARD)) {
			_vm->_dialogs->show(10433);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_JUMP_INTO, NOUN_ORCHESTRA_PIT)) {
		_vm->_dialogs->show(10426);
		_scene->_nextSceneId = 102;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_PROMPTERS_BOX) || _action.isAction(VERB_CLOSE, NOUN_PROMPTERS_BOX)) {
		_vm->_dialogs->show(10430);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_TRAP_DOOR)) {
		if (_globals[kTrapDoorStatus] == 0)
			_vm->_dialogs->show(10424);
		else
			_vm->_dialogs->show(10432);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_TRAP_DOOR)) {
		if (_globals[kTrapDoorStatus] == 1)
			_vm->_dialogs->show(10425);
		else
			_vm->_dialogs->show(10433);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHANDELIER)) {
		_vm->_dialogs->show(10435);
		_action._inProgress = false;
		return;
	}
}

void Scene104::cleanInventory() {
	if (_game._objects.isInInventory(OBJ_LARGE_NOTE))
		_game._objects.setRoom(OBJ_LARGE_NOTE, NOWHERE);

	if (_game._objects.isInInventory(OBJ_SANDBAG))
		_game._objects.setRoom(OBJ_SANDBAG, NOWHERE);

	if (_game._objects.isInInventory(OBJ_SMALL_NOTE))
		_game._objects.setRoom(OBJ_SMALL_NOTE, NOWHERE);

	if (_game._objects.isInInventory(OBJ_PARCHMENT))
		_game._objects.setRoom(OBJ_PARCHMENT, NOWHERE);

	if (_game._objects.isInInventory(OBJ_BOOK))
		_game._objects.setRoom(OBJ_BOOK, NOWHERE);

	if (_game._objects.isInInventory(OBJ_RED_FRAME))
		_game._objects.setRoom(OBJ_RED_FRAME, 105);

	if (_game._objects.isInInventory(OBJ_YELLOW_FRAME))
		_game._objects.setRoom(OBJ_YELLOW_FRAME, 107);

	if (_game._objects.isInInventory(OBJ_BLUE_FRAME))
		_game._objects.setRoom(OBJ_BLUE_FRAME, 302);

	if (_game._objects.isInInventory(OBJ_GREEN_FRAME))
		_game._objects.setRoom(OBJ_GREEN_FRAME, 307);
}

void Scene104::processConversations() {
	bool interlocutorTriggerFl = false;
	bool heroTriggerFl = false;

	switch (_action._activeAction._verbId) {
	case 2:
	case 10:
	case 12:
	case 16:
	case 20:
	case 21:
	case 24:
		_vm->_gameConv->setInterlocutorTrigger(75);
		interlocutorTriggerFl = true;
		break;

	case 3:
		if (!_needToGetUp) {
			_vm->_gameConv->setInterlocutorTrigger(67);
			interlocutorTriggerFl = true;
			_needToGetUp = true;
		}
		break;

	case 8:
		_vm->_gameConv->setInterlocutorTrigger(87);
		interlocutorTriggerFl = true;
		break;

	case 11:
		_vm->_gameConv->setInterlocutorTrigger(77);
		interlocutorTriggerFl = true;
		break;

	case 14:
	case 25:
		if (!_game._trigger) {
			_richStatus = 0;
			_coupleStatus = 5;
			_vm->_gameConv->hold();
		}
		break;

	case 22:
		_vm->_gameConv->setInterlocutorTrigger(75);
		_vm->_gameConv->setHeroTrigger(79);
		interlocutorTriggerFl = true;
		heroTriggerFl = true;
		break;

	case 23:
		_vm->_gameConv->setInterlocutorTrigger(89);
		interlocutorTriggerFl = true;
		break;

	case 28:
		_vm->_gameConv->setInterlocutorTrigger(81);
		interlocutorTriggerFl = true;
		break;

	case 30:
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_richStatus = 4;
		}
		break;

	case 32:
		_coupleStatus = 14;
		heroTriggerFl = true;
		interlocutorTriggerFl = true;
		_vm->_gameConv->hold();
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 67:
		_vm->_gameConv->hold();
		_coupleStatus = 12;
		break;

	case 69:
		if (!_beforeSheLeaves && !_beforeHeLeaves && (_coupleStatus != 14) && !_needToStandUp) {
			_richStatus = 0;
			if (_sittingUp)
				_coupleStatus = 4;
			else
				_coupleStatus = 12;
		}
		break;

	case 71:
		if (!_beforeSheLeaves && !_beforeHeLeaves && (_coupleStatus != 14) && !_needToStandUp) {
			_richStatus = 0;
			if (_sittingUp && !_beforeSheLeaves) {
				_coupleStatus = 3;
				_richStatus = 0;
			}
		}
		break;

	case 75:
		_richStatus = 1;

		if (_sittingUp) {
			if (_action._activeAction._verbId == 20) {
				_lookCount = 0;
				_coupleStatus = 9;
			} else if ((_action._activeAction._verbId == 21) || (_action._activeAction._verbId == 22)) {
				_lookCount = 0;
				_coupleStatus = 10;
			} else {
				_coupleStatus = 1;
			}
		} else {
			_coupleStatus = 11;
		}
		break;

	case 77:
		_richStatus = 0;
		_coupleStatus = 8;
		break;

	case 79:
		_richStatus = 0;
		_coupleStatus = 7;
		break;

	case 81:
		_richStatus = 1;
		_beforeHeLeaves = true;
		_coupleStatus = 15;
		break;

	case 83:
		_vm->_gameConv->release();
		if (_coupleStatus != 17)
			_game._player._stepEnabled = false;
		break;

	case 87:
		_richStatus = 3;
		break;

	case 89:
		_richStatus = 2;
		break;

	default:
		break;
	}

	if (!heroTriggerFl && !_beforeSheLeaves)
		_vm->_gameConv->setHeroTrigger(71);

	if (!interlocutorTriggerFl)
		_vm->_gameConv->setInterlocutorTrigger(69);

	_richTalkCount = 0;
	_manTalkCount = 0;
	_womanTalkCount = 0;
}

void Scene104::handleWalkAnimation() {
	if (_scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame() == _walkFrame)
		return;

	_walkFrame = _scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame();
	int daaeResetFrame = -1;

	switch (_walkFrame) {
	case 1:
		if (_walkStatus == 0) {
			daaeResetFrame = 0;
		} else {
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[2], SYNC_ANIM, _globals._animationIndexes[1]);
			daaeResetFrame = 1;
		}
		break;

	case 138:
		_walkStatus = 0;
		daaeResetFrame = 0;
		break;

	default:
		break;
	}

	if (daaeResetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[2], daaeResetFrame);
		_walkFrame = daaeResetFrame;
	}
}

void Scene104::handleRichAnimations() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _richFrame)
		return;

	_richFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int random;
	int resetFrame = -1;

	switch (_richFrame) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 8:
	case 14:
	case 22:
	case 34:
	case 40:
	case 44:
	case 48:
		random = -1;
		if (_richStatus == 1) {
			random = _vm->getRandomNumber(1, 3);
			++_richTalkCount;
			if (_richTalkCount > 15) {
				_richStatus = 0;
				random = 40;
			}
		}

		if (_richStatus == 0)
			random = _vm->getRandomNumber(7, 80);

		if (_richStatus == 2) {
			random = 4;
			_richStatus = 1;
			_richTalkCount = 8;
		}

		if (_richStatus == 3) {
			random = 5;
			_richStatus = 1;
			_richTalkCount = 8;
		}

		if (_richStatus == 4)
			random = 6;

		if (_richStatus == 5)
			random = 7;

		switch (random) {
		case 1:
			resetFrame = 1;
			break;

		case 2:
			resetFrame = 2;
			break;

		case 3:
			resetFrame = 3;
			break;

		case 4:
			resetFrame = 23;
			break;

		case 5:
			resetFrame = 35;
			break;

		case 6:
			resetFrame = 49;
			break;

		case 7:
			resetFrame = 41;
			break;

		case 8:
			resetFrame = 45;
			break;

		case 9:
			resetFrame = 9;
			break;

		case 10:
			resetFrame = 5;
			break;

		case 11:
			resetFrame = 15;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 117:
		_coupleStatus = 13;
		break;

	case 125:
		resetFrame = 124;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_richFrame = resetFrame;
	}
}

void Scene104::handleCoupleAnimations() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _coupleFrame)
		return;

	_coupleFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;
	int random;

	switch (_coupleFrame) {
	case 1:
	case 103:
	case 104:
	case 105:
		if (_coupleStatus == 11)
			resetFrame = 0;
		else {
			resetFrame = _vm->getRandomNumber(102, 104);
			++_womanTalkCount;
			if (_womanTalkCount > 15) {
				if (_needToGetUp) {
					_coupleStatus = 6;
					resetFrame = 1;
				} else {
					_coupleStatus = 11;
					resetFrame = 0;
				}
			}
		}
		break;

	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 25:
	case 33:
	case 41:
		switch (_coupleFrame) {
		case 9:
			_coupleStatus = 6;
			break;

		case 33:
			_vm->_gameConv->release();
			if (_action._activeAction._verbId == 13)
				_coupleStatus = 4;

			break;

		case 41:
			_vm->_gameConv->release();
			_sittingUp = true;
			if (_needToTalk)
				_coupleStatus = 3;
			else
				_coupleStatus = 1;
			break;

		default:
			break;
		}

		random = -1;

		switch (_coupleStatus) {
		case 1:
			random = 12;
			break;

		case 2:
		case 7:
		case 8:
			random = 11;
			break;

		case 3:
			random = _vm->getRandomNumber(4, 6);
			++_manTalkCount;
			if (_manTalkCount > 15) {
				_coupleStatus = 1;
				random = 12;
			}
			break;

		case 4:
			if (_beforeSheLeaves) {
				random = 10;
			} else {
				random = _vm->getRandomNumber(1, 3);
				++_womanTalkCount;
				if (_womanTalkCount > 15) {
					_coupleStatus = 1;
					random = 12;
				}
			}
			break;

		case 5:
			_coupleStatus = 1;
			random = 8;
			break;

		case 6:
			_coupleStatus = 1;
			random = 7;
			break;

		case 13:
			random = 9;
			break;

		case 15:
			random = 10;
			break;

		default:
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 12;
			break;

		case 2:
			resetFrame = 13;
			break;

		case 3:
			resetFrame = 14;
			break;

		case 4:
			resetFrame = 9;
			break;

		case 5:
			resetFrame = 10;
			break;

		case 6:
			resetFrame = 11;
			break;

		case 7:
			resetFrame = 33;
			break;

		case 8:
			resetFrame = 25;
			break;

		case 9:
			resetFrame = 54;
			break;

		case 10:
			resetFrame = 41;
			break;

		case 11:
			resetFrame = 15;
			break;

		case 12:
			resetFrame = 14;
			break;

		default:
			break;
		}
		break;

	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
		random = -1;
		switch (_coupleStatus) {
		case 1:
		case 3:
		case 4:
		case 5:
		case 6:
		case 15:
		case 16:
			random = 7;
			break;

		case 2:
			random = 8;
			break;

		case 7:
			random = _vm->getRandomNumber(4, 6);
			++_manTalkCount;
			if (_manTalkCount > 15) {
				_coupleStatus = 2;
				random = 8;
			}
			break;

		case 8:
			random = _vm->getRandomNumber(1, 3);
			++_womanTalkCount;
			if (_womanTalkCount > 15) {
				_coupleStatus = 1;
				random = 7;
			}
			break;

		case 9:
			random = 1;
			++_lookCount;
			if (_lookCount > 6) {
				_coupleStatus = 1;
				random = 7;
			}
			break;

		case 10:
			random = 1;
			break;

		default:
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 20;
			break;

		case 2:
			resetFrame = 21;
			break;

		case 3:
			resetFrame = 22;
			break;

		case 4:
			resetFrame = 17;
			break;

		case 5:
			resetFrame = 18;
			break;

		case 6:
			resetFrame = 19;
			break;

		case 7:
			resetFrame = 23;
			break;

		case 8:
			resetFrame = 20;
			break;

		default:
			break;
		}
		break;

	case 52:
		_walkStatus = 1;
		resetFrame = 54;
		break;

	case 55:
		if (_coupleStatus != 13)
			resetFrame = 54;

		break;

	case 89:
		_vm->_gameConv->release();
		break;

	case 90:
		if (_coupleStatus != 14) {
			resetFrame = 89;
		} else {
			resetFrame = 90;
			_globals[kTempVar] = 200;
		}
		break;

	case 102:
		_vm->_gameConv->release();
		_game._player._playerPos = Common::Point(166, 126);
		_game._player.resetFacing(FACING_SOUTH);
		resetFrame = 105;
		_game._player._visible = true;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		break;

	case 106:
		_coupleStatus = 17;
		resetFrame = 105;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_coupleFrame = resetFrame;
	}
}

void Scene104::handlePlayerWalk() {
	if (_game._player._frameNumber == _lastPlayerFrame)
		return;

	_lastPlayerFrame = _game._player._frameNumber;
	switch (_game._player._facing) {
	case FACING_NORTH:
	case FACING_SOUTH:
		if ((_game._player._frameNumber == 5) || (_game._player._frameNumber == 11))
			_vm->_sound->command(68);
		break;

	case FACING_NORTHEAST:
	case FACING_NORTHWEST:
	case FACING_SOUTHEAST:
	case FACING_SOUTHWEST:
		if ((_game._player._frameNumber == 7) || (_game._player._frameNumber == 14))
			_vm->_sound->command(68);
		break;

	case FACING_EAST:
	case FACING_WEST:
		if ((_game._player._frameNumber == 8) || (_game._player._frameNumber == 16))
			_vm->_sound->command(68);
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

Scene105::Scene105(MADSEngine *vm) : Scene1xx(vm) {
}

void Scene105::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

}

void Scene105::setup() {
	if (_globals[kCurrentYear] == 1993)
		_scene->_variant = 1;

	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_LIGHT_FIXTURE);
}

void Scene105::enter() {
	_scene->loadSpeech(8);

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RRD_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RDR_9");
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 1));
	if (_globals[kCurrentYear] == 1993)
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('z', -1));

	if ((_globals[kCurrentYear] == 1881) && (!_globals[kHintThatDaaeIsHome2])) {
		if ((_globals[kJacquesNameIsKnown] == 2) && (_globals[kMadameNameIsKnown] == 2) &&
			_globals[kPanelIn206] && _globals[kDoneRichConv203] && _game._objects.isInInventory(OBJ_LANTERN) &&
			((_game._objects.isInInventory(OBJ_CABLE_HOOK) && _game._objects.isInInventory(OBJ_ROPE)) || _game._objects.isInInventory(OBJ_ROPE_WITH_HOOK))) {
			_globals[kHintThatDaaeIsHome2] = true;
			_scene->_sequences.addTimer(300, 75);
		}
	}

	if ((_game._objects.isInRoom(OBJ_LANTERN)) && (_globals[kCurrentYear] == 1881)) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else {
		_scene->_hotspots.activate(NOUN_LANTERN, false);
	}

	if (_game._objects.isInRoom(OBJ_RED_FRAME)) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
	} else {
		_scene->_hotspots.activate(NOUN_RED_FRAME, false);
	}

	if (_globals[kCurrentYear] == 1993) {
		_scene->drawToBackground(_globals._spriteIndexes[3], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_PROPS, false);
		_scene->_hotspots.activate(NOUN_BEAR_PROP, false);
		_scene->_hotspots.activate(NOUN_STAIR_UNIT, false);
		_scene->_hotspots.activate(NOUN_PROP, false);
		_scene->_hotspots.activate(NOUN_ELEPHANT_PROP, false);
		_scene->_hotspots.activate(NOUN_COLUMN_PROP, false);

		int tmpIdx = _scene->_dynamicHotspots.add(NOUN_COLUMN_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(132, 24, 132 + 21, 24 + 105));
		_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(159, 133), FACING_NORTHWEST);

		tmpIdx = _scene->_dynamicHotspots.add(NOUN_COLUMN_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(56, 45, 56 + 11, 45 + 77));
		_scene->_dynamicHotspots.setPosition(tmpIdx, Common::Point(72, 126), FACING_NORTHWEST);

		_scene->_dynamicHotspots.add(NOUN_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(0, 125, 29, 125 + 31));
		_scene->_dynamicHotspots.add(NOUN_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(29, 136, 29 + 50, 136 + 20));
		_scene->_dynamicHotspots.add(NOUN_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(79, 141, 79 + 53, 141 + 15));

		_scene->_dynamicHotspots.add(NOUN_BEAR_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(278, 132, 278 + 21, 132 + 24));
		_scene->_dynamicHotspots.add(NOUN_BEAR_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(299, 146, 299 + 8, 146 + 10));
		_scene->_dynamicHotspots.add(NOUN_BEAR_PROP, VERB_WALKTO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(269, 142, 269 + 8, 142 + 8));

		_scene->_dynamicHotspots.add(NOUN_LIGHT_FIXTURE, VERB_LOOK_AT, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(102, 14, 102 + 24, 102 + 10));
	}

	_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

	if ((_scene->_priorSceneId == 106) || (_scene->_priorSceneId == 114)) {
		_game._player._playerPos = Common::Point(198, 132);
		_game._player._facing = FACING_WEST;
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
	} else if ((_scene->_priorSceneId == 103) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(3, 112);
		_game._player._facing = FACING_SOUTHEAST;
		_game._player.walk(Common::Point(45, 131), FACING_SOUTHEAST);
		_game._player.setWalkTrigger(60);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 8);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
	} else if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
	}

	sceneEntrySound();
}

void Scene105::step() {
	switch (_game._trigger) {
	case 60:
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		_vm->_sound->command(66);
		break;

	case 61:
		_vm->_sound->command(25);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_game._player._stepEnabled = true;
		break;

	case 75:
		_scene->playSpeech(8);
		_scene->_sequences.addTimer(120, 76);
		break;

	case 76:
		_vm->_dialogs->show(10537);
		break;

	default:
		break;
	}
}

void Scene105::actions() {
	if ((_action.isAction(VERB_PUSH, NOUN_THUNDER_MACHINE)) || (_action.isAction(VERB_PULL, NOUN_THUNDER_MACHINE))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('t', 1), 70);
			_scene->deleteSequence(_globals._sequenceIndexes[4]);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[4], SYNC_ANIM, _globals._animationIndexes[0]);
			_scene->_sequences.setSeqPlayer(_globals._animationIndexes[0], false);
			break;

		case 70:
			_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[4], SYNC_ANIM, _globals._animationIndexes[0]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_UP, NOUN_CIRCULAR_STAIRCASE)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('u', 1), 1);
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
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

	if (_action.isAction(VERB_CLIMB_DOWN, NOUN_CIRCULAR_STAIRCASE)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 1);
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
			break;

		case 1:
			_scene->_nextSceneId = 114;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_RED_FRAME) && (_game._objects.isInRoom(OBJ_RED_FRAME) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			if (_globals[kCurrentYear] == 1881) {
				int count = 0;

				if (_game._objects.isInInventory(NOUN_YELLOW_FRAME))
					++count;

				if (_game._objects.isInInventory(NOUN_GREEN_FRAME))
					++count;

				if (_game._objects.isInInventory(NOUN_BLUE_FRAME))
					++count;

				if (count < 3)
					_globals[kPlayerScore] += 5;
			}

			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 5);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 5, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[1]);
			_scene->_hotspots.activate(NOUN_RED_FRAME, false);
			_game._objects.addToInventory(OBJ_RED_FRAME);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[5]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->showItem(OBJ_RED_FRAME, 842, 0);
			else
				_vm->_dialogs->showItem(OBJ_RED_FRAME, 802, 0);

			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_LANTERN) && (_game._objects.isInRoom(OBJ_LANTERN) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], true, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[6], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[0]);
			_scene->_hotspots.activate(NOUN_LANTERN, false);
			_game._objects.addToInventory(OBJ_LANTERN);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[6]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_vm->_dialogs->showItem(OBJ_LANTERN, 801, 0);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR) || (_game._trigger) ||
		_action.isAction(VERB_UNLOCK, NOUN_DOOR) || _action.isAction(VERB_LOCK, NOUN_DOOR)) {
		if ((_globals[kCurrentYear] == 1881) && !_action.isAction(VERB_UNLOCK) && !_action.isAction(VERB_LOCK)){
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[6], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 4, 65);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_game._player._visible = true;
				_scene->_sequences.addTimer(180, 3);
				break;

			case 3:
				_scene->_nextSceneId = 103;
				break;

			case 65:
				_vm->_sound->command(24);
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
				_vm->_sound->command(66);
				break;

			case 66: {
				int tmpIdx = _globals._sequenceIndexes[2];
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 8);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[2], SYNC_SEQ, tmpIdx);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
				_game._player.walk(Common::Point(0, 111), FACING_NORTHWEST);
				}
				break;

			default:
				break;
			}
		} else {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], true, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[6], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1: {
				int tmpIdx = _globals._sequenceIndexes[6];
				_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], true, 4);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[6], SYNC_SEQ, tmpIdx);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[6], false);
				_vm->_sound->command(73);
				_scene->_sequences.addTimer(15, 2);
				}
				break;

			case 2:
				_scene->deleteSequence(_globals._sequenceIndexes[6]);
				_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], true, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[6], false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[6]);
				_game._player._visible = true;
				if (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK))
					_vm->_dialogs->show(32);
				else
					_vm->_dialogs->show(10536);

				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(10510);
		else
			_vm->_dialogs->show(10511);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(10512);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CIRCULAR_STAIRCASE)) {
			_vm->_dialogs->show(10513);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LANTERN) && _game._objects.isInRoom(OBJ_LANTERN)) {
			_vm->_dialogs->show(10514);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RED_FRAME) && _game._objects.isInRoom(OBJ_RED_FRAME)){
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->show(10530);
			else
				_vm->_dialogs->show(10515);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(10516);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10517);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROP_TABLE)) {
			_vm->_dialogs->show(10518);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BEAR_PROP)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10519);
			else
				_vm->_dialogs->show(10538);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ELEPHANT_PROP)) {
			_vm->_dialogs->show(10520);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COLUMN_PROP)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10521);
			else
				_vm->_dialogs->show(10539);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_THUNDER_MACHINE)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10522);
			else
				_vm->_dialogs->show(10540);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAIR_UNIT)) {
			_vm->_dialogs->show(10523);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROP)) {
			_vm->_dialogs->show(10524);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROPS)) {
			_vm->_dialogs->show(10525);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EXIT_SIGN)) {
			_vm->_dialogs->show(10526);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLATS)) {
			_vm->_dialogs->show(10527);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HEMP)) {
			_vm->_dialogs->show(10528);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PIPE)) {
			_vm->_dialogs->show(10529);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRAFFITI)) {
			_vm->_dialogs->show(10531);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT_FIXTURE)) {
			_vm->_dialogs->show(10535);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_BEAR_PROP)) {
		_vm->_dialogs->show(10532);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BEAR_PROP)) {
		_vm->_dialogs->show(10533);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_DOOR)) {
		_vm->_dialogs->show(10534);
		_action._inProgress = false;
		return;
	}
}

void Scene105::preActions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR_TO_PIT))
		_game._player._walkOffScreenSceneId = 102;

	if (_action.isAction(VERB_OPEN, NOUN_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_DOOR) || _action.isAction(VERB_LOCK, NOUN_DOOR))
		_game._player.walk(Common::Point(33, 128), FACING_NORTHWEST);
}

/*------------------------------------------------------------------------*/

Scene106::Scene106(MADSEngine *vm) : Scene1xx(vm) {
	_sandbagHostpotId = -1;
}

void Scene106::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsSint16LE(_sandbagHostpotId);
}

void Scene106::setup() {
	if (_globals[kCurrentYear] == 1881)
		_scene->_variant = 1;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene106::enter() {
	_scene->loadSpeech(8);

	if (_globals[kCurrentYear] == 1993) {
		if (!_game._objects.isInInventory(OBJ_SANDBAG)) {
			_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RRD_9");
			_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
		}
	} else {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('z', -1));
		if (_game._objects.isInRoom(OBJ_CABLE_HOOK) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)) {
			_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RRD_9");
			_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('p', 0));
		}
	}

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RDR_9");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 2));

	if ((_globals[kCurrentYear] == 1881) && (!_globals[kHintThatDaaeIsHome1])) {
		if ((_globals[kJacquesNameIsKnown] == 2) && (_globals[kMadameNameIsKnown] == 2)
		 && (_globals[kPanelIn206]) && (_globals[kDoneRichConv203]) && (_game._objects.isInInventory(OBJ_LANTERN))
		 && ((_game._objects.isInInventory(OBJ_CABLE_HOOK) && _game._objects.isInInventory(OBJ_ROPE))
		   || _game._objects.isInInventory(OBJ_ROPE_WITH_HOOK))) {
			_globals[kHintThatDaaeIsHome1] = true;
			_scene->_sequences.addTimer(300, 85);
		}
	}

	if ((_globals[kSandbagStatus] == 1) && (_globals[kCurrentYear] == 1993) && _game._objects.isInRoom(OBJ_SANDBAG)) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
		_sandbagHostpotId = _scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(227, 140, 227 + 18, 140 + 11));
		_scene->_dynamicHotspots.setPosition(_sandbagHostpotId, Common::Point(224, 152), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
	}

	if (_globals[kCurrentYear] == 1881) {
		_scene->drawToBackground(_globals._spriteIndexes[7], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_BIG_PROP, false);
		_scene->_hotspots.activate(NOUN_STATUE, false);
		_scene->_hotspots.activate(NOUN_PLANT_PROP, false);
		_scene->_hotspots.activate(NOUN_PEDESTAL, false);
		_scene->_hotspots.activate(NOUN_SANDBAG, false);
		_scene->_hotspots.activate(NOUN_CRATE, false);

		_scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_LOOK_AT, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(40, 47, 40 + 11, 47 + 17));
		_scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_LOOK_AT, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(98, 14, 98 + 5, 14 + 10));
		_scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_LOOK_AT, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(111, 23, 111 + 6, 23 + 9));
		_scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_LOOK_AT, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(119, 12, 119 + 5, 12 + 8));

		int idx = _scene->_dynamicHotspots.add(NOUN_STAGE, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(80, 114, 80 + 24, 114 + 4));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(93, 121), FACING_NONE);

		idx = _scene->_dynamicHotspots.add(NOUN_STAGE, VERB_WALK_ACROSS, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(106, 102, 106 + 5, 102 + 10));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(108, 109), FACING_NONE);
	} else {
		_scene->_hotspots.activate(NOUN_BOXES, false);
		_scene->_hotspots.activate(NOUN_CASE, false);
	}

	if ((_game._objects.isInRoom(OBJ_CABLE_HOOK)) && (_globals[kCurrentYear] == 1881) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)){
		_globals._sequenceIndexes[8] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[8], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 3);
	} else {
		_scene->_hotspots.activate(NOUN_CABLE_HOOK, false);
	}

	if (_scene->_priorSceneId == 109) {
		_game._player._playerPos = Common::Point(180, 58);
		_game._player._facing = FACING_SOUTH;
		_game._player.walk(Common::Point(179, 63), FACING_SOUTH);
		_game._player.setWalkTrigger(60);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if ((_scene->_priorSceneId == 105) || (_scene->_priorSceneId == 301)) {
		_game._player._playerPos = Common::Point(235, 142);
		_game._player._facing = FACING_WEST;
		_game._player.walk(Common::Point(227, 143), FACING_WEST);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if (_scene->_priorSceneId == 107) {
		_game._player._playerPos = Common::Point(143, 68);
		_game._player._facing = FACING_WEST;
		_game._player.walk(Common::Point(163, 68), FACING_SOUTHEAST);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if ((_scene->_priorSceneId == 108) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player.firstWalk(Common::Point(-20, 130), FACING_SOUTHEAST, Common::Point(19, 147), FACING_NORTHEAST, true);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	}

	sceneEntrySound();
}

void Scene106::step() {
	switch (_game._trigger) {
	case 85:
		_scene->playSpeech(8);
		_scene->_sequences.addTimer(120, 86);
		break;

	case 86:
		_vm->_dialogs->show(10637);
		break;

	case 60:
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		_vm->_sound->command(25);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene106::actions() {
	if ((_action.isAction(VERB_TAKE, NOUN_SANDBAG)) && (_scene->_customDest.y < 61)) {
		_vm->_dialogs->show(10635);
		_action._inProgress = false;
		return;
	} else if (_action.isAction(VERB_TAKE, NOUN_SANDBAG)) {
		if (_game._objects.isInRoom(OBJ_SANDBAG)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_scene->deleteSequence(_globals._sequenceIndexes[0]);
				_scene->_dynamicHotspots.remove(_sandbagHostpotId);
				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[5]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(20, 3);
				break;

			case 3:
				_game._objects.addToInventory(OBJ_SANDBAG);
				_vm->_dialogs->showItem(OBJ_SANDBAG, 803, 0);
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_CABLE_HOOK)) {
		if (_game._objects.isInRoom(OBJ_CABLE_HOOK)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 5);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				_globals[kPlayerScore] += 5;
				break;

			case 1:
				_scene->deleteSequence(_globals._sequenceIndexes[8]);
				_scene->_hotspots.activate(NOUN_CABLE_HOOK, false);
				_vm->_sound->command(26);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[5]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(20, 3);
				break;

			case 3:
				_game._objects.addToInventory(OBJ_CABLE_HOOK);
				_vm->_dialogs->showItem(OBJ_CABLE_HOOK, 822, 0);
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	switch (_game._trigger) {
	case 75:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 76);
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
		_action._inProgress = false;
		return;

	case 76:
		_scene->_nextSceneId = 105;
		_action._inProgress = false;
		return;

	case 80:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('u', 1), 81);
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
		_action._inProgress = false;
		return;

	case 81:
		_scene->_sequences.addTimer(120, 82);
		_action._inProgress = false;
		return;

	case 82:
		_scene->_nextSceneId = 301;
		_action._inProgress = false;
		return;

	default:
		break;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_STAGE_RIGHT_WING)) {
		_scene->_nextSceneId = 107;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_DOWN) || _action.isAction(VERB_CLIMB_UP)) {
		if (_globals[kSandbagStatus] == 1) {
			switch (_game._trigger) {
			case 0:
				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('s',-1), 1);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				break;

			case 1:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
				_game._player._visible = true;
				_game._player._playerPos = Common::Point(225, 143);
				_game._player.resetFacing(FACING_EAST);
				_game._player.walk(Common::Point(236, 142), FACING_EAST);

				if (_action.isAction(VERB_CLIMB_DOWN))
					_game._player.setWalkTrigger(75);
				else
					_game._player.setWalkTrigger(80);
				break;

			default:
				break;
			}
		} else {
			switch (_game._trigger) {
			case 0:
				_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 4, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 9);
				_scene->_sequences.addTimer(6, 2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_vm->_sound->command(70);
				_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 2);
				_sandbagHostpotId = _scene->_dynamicHotspots.add(NOUN_SANDBAG, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(227, 140, 227 + 18, 140 + 11));
				_scene->_dynamicHotspots.setPosition(_sandbagHostpotId, Common::Point(224, 152), FACING_NORTHEAST);
				break;

			case 2:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('s', 1), 3);
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
				break;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_game._player._playerPos = Common::Point(228, 140);
				_game._player.resetFacing(FACING_SOUTHEAST);
				_globals[kSandbagStatus] = 1;
				_vm->_dialogs->show(10632);
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) {
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
			_vm->_sound->command(24);
			_scene->deleteSequence(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
			break;

		case 66: {
			int idx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 5);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[2], SYNC_SEQ, idx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
			}
			break;

		case 67:
			_game._player._visible = true;
			_game._player.walk(Common::Point(180, 60), FACING_NORTH);
			_game._player.setWalkTrigger(68);
			break;

		case 68:
			_scene->deleteSequence(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 69);
			break;

		case 69:
			_vm->_sound->command(25);
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_nextSceneId = 109;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(10610);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_STAGE_RIGHT_WING)) {
			_vm->_dialogs->show(10611);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE_LEFT_WING)) {
			_vm->_dialogs->show(10612);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_IN_TWO)) {
			_vm->_dialogs->show(10613);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CYCLORAMA)) {
			_vm->_dialogs->show(10614);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(10615);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PEDESTAL)) {
			_vm->_dialogs->show(10616);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PLANT_PROP)) {
			_vm->_dialogs->show(10617);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SANDBAG)) {
			if (_scene->_customDest.y < 60) {
				_vm->_dialogs->show(10618);
				_action._inProgress = false;
				return;
			} else if (_game._objects.isInRoom(OBJ_SANDBAG)) {
				_vm->_dialogs->show(10633);
				_action._inProgress = false;
				return;
			}
		}

		if (_action.isObject(NOUN_STATUE)) {
			_vm->_dialogs->show(10619);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CIRCULAR_STAIRCASE)) {
			_vm->_dialogs->show(10620);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BATTEN)) {
			_vm->_dialogs->show(10621);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(10622);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOXES) || _action.isObject(NOUN_BOX)) {
			_vm->_dialogs->show(10623);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BIG_PROP)) {
			_vm->_dialogs->show(10624);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CRATE)) {
			_vm->_dialogs->show(10625);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CASE)) {
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->show(10638);
			else
				_vm->_dialogs->show(10636);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_VENTILATION_DUCT)) {
			_vm->_dialogs->show(10626);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_JUNK)) {
			_vm->_dialogs->show(10627);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLATS)) {
			_vm->_dialogs->show(10628);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10629);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CABLE_HOOK) && _game._objects.isInRoom(OBJ_CABLE_HOOK)) {
			_vm->_dialogs->show(10639);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_STATUE)) {
		_vm->_dialogs->show(10630);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_DOOR)) {
		_vm->_dialogs->show(10634);
		_action._inProgress = false;
		return;
	}
}

void Scene106::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_STAGE_LEFT_WING))
		_game._player._walkOffScreenSceneId = 108;

	if (_action.isAction(VERB_OPEN, NOUN_DOOR))
		_game._player.walk(Common::Point(179, 63), FACING_NORTHWEST);
}

/*------------------------------------------------------------------------*/

Scene107::Scene107(MADSEngine *vm) : Scene1xx(vm) {
}

void Scene107::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
}

void Scene107::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene107::enter() {
	if (_globals[kCurrentYear] == 1993)
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));

	if (_game._objects.isInRoom(OBJ_YELLOW_FRAME)) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 0));
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RRD_9");
	}

	if (_game._objects.isInRoom(OBJ_YELLOW_FRAME)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else {
		_scene->_hotspots.activate(NOUN_YELLOW_FRAME, false);
	}

	if (_globals[kCurrentYear] == 1993) {
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_PROP_TABLE, false);
	} else {
		_scene->_hotspots.activate(NOUN_HEADSET, false);
	}

	if (_scene->_priorSceneId == 106) {
		_game._player._playerPos = Common::Point(276, 73);
		_game._player._facing = FACING_SOUTHWEST;
		_game._player.walk(Common::Point(248, 75), FACING_SOUTHWEST);
	} else if ((_scene->_priorSceneId == 104) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		if (_game._player._playerPos.y > 128) {
			_game._player._playerPos.x = 216;
			_game._player._facing = FACING_NORTHWEST;
		} else if (_game._player._playerPos.y > 99) {
			_game._player._playerPos.x = 127;
			_game._player._facing = FACING_NORTHWEST;
		} else {
			_game._player._playerPos.x = 44;
			_game._player._facing = FACING_NORTHEAST;
		}
		_game._player._playerPos.y = 143;
	}

	sceneEntrySound();
}

void Scene107::step() {
}

void Scene107::actions() {
	if (_action.isAction(VERB_WALK_ONTO, NOUN_STAGE)) {
		_scene->_nextSceneId = 104;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK, NOUN_BACKSTAGE)) {
		_scene->_nextSceneId = 106;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_YELLOW_FRAME) && (_game._objects.isInRoom(OBJ_YELLOW_FRAME) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			if (_globals[kCurrentYear] == 1881) {
				int count = 0;
				if (_game._objects.isInInventory(OBJ_GREEN_FRAME))
					++count;
				if (_game._objects.isInInventory(OBJ_RED_FRAME))
					++count;
				if (_game._objects.isInInventory(OBJ_BLUE_FRAME))
					++count;

				if (count < 3)
					_globals[kPlayerScore] += 5;
			}

			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 5);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 5, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[2]);
			_scene->_hotspots.activate(NOUN_YELLOW_FRAME, false);
			_game._objects.addToInventory(OBJ_YELLOW_FRAME);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 843, 0);
			else
				_vm->_dialogs->showItem(OBJ_YELLOW_FRAME, 804, 0);

			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(10710);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(10711);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_IN_TWO)) {
			_vm->_dialogs->show(10712);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_IN_ONE)) {
			_vm->_dialogs->show(10713);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CYCLORAMA)) {
			_vm->_dialogs->show(10714);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COUNTERWEIGHT_SYSTEM)) {
			_vm->_dialogs->show(10715);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PURCHASE_LINES)) {
			_vm->_dialogs->show(10716);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LOCKRAIL)) {
			_vm->_dialogs->show(10717);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(10718);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROP_TABLE)) {
			_vm->_dialogs->show(10719);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ACT_CURTAIN)) {
			_vm->_dialogs->show(10720);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEG)) {
			_vm->_dialogs->show(10721);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_APRON)) {
			_vm->_dialogs->show(10722);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROSCENIUM_ARCH)) {
			_vm->_dialogs->show(10723);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(10724);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BACKSTAGE)) {
			_vm->_dialogs->show(10725);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_YELLOW_FRAME) && _game._objects.isInRoom(OBJ_YELLOW_FRAME)) {
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->show(10727);
			else
				_vm->_dialogs->show(10726);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HEADSET)) {
			_vm->_dialogs->show(10728);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10730);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_HEADSET)) {
		_vm->_dialogs->show(10729);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_HEADSET)) {
		_vm->_dialogs->show(10732);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PULL, NOUN_PURCHASE_LINES)) {
		_vm->_dialogs->show(10731);
		_action._inProgress = false;
		return;
	}
}

void Scene107::preActions() {
}

/*------------------------------------------------------------------------*/

Scene108::Scene108(MADSEngine *vm) : Scene1xx(vm) {
	_anim0ActvFl = false;
	_handRaisedFl = false;
	_shutUpCount = -1;
	_maxTalkCount = -1;
	_charAction = -1;
	_charFrame = -1;
	_charHotspotId = -1;
	_charTalkCount = -1;
	_conversationCount = -1;
	_prevShutUpFrame = -1;
}

void Scene108::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_handRaisedFl);
	s.syncAsSint16LE(_shutUpCount);
	s.syncAsSint16LE(_maxTalkCount);
	s.syncAsSint16LE(_charAction);
	s.syncAsSint16LE(_charFrame);
	s.syncAsSint16LE(_charHotspotId);
	s.syncAsSint16LE(_charTalkCount);
	s.syncAsSint16LE(_conversationCount);
	s.syncAsSint16LE(_prevShutUpFrame);
}

void Scene108::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kCurrentYear] == 1993)
		_scene->_variant = 1;

	_scene->addActiveVocab(NOUN_GENTLEMAN);
	_scene->addActiveVocab(NOUN_CHARLES);
}

void Scene108::enter() {
	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_handRaisedFl = false;
		_charTalkCount = 0;
		_shutUpCount = 40;
		_maxTalkCount = 15;
	}

	_vm->_gameConv->load(2);

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_scene->_hotspots.activate(NOUN_STOOL, false);
		int idx = _scene->_dynamicHotspots.add(NOUN_STOOL, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(250, 68, 250 + 8, 68 + 21));
		_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(253, 75), FACING_SOUTHEAST);
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
	} else {
		_scene->_hotspots.activate(NOUN_HEADSET, false);
		int idx = _scene->_dynamicHotspots.add(NOUN_WALL, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(258, 58, 258 + 6, 58 + 10));
		_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(236, 69), FACING_NORTHEAST);
	}

	if ((_globals[kCurrentYear] == 1993) && (_globals[kDoneBrieConv203] == 0)) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('c', 1), 1);
		_anim0ActvFl = true;

		if (_vm->_gameConv->activeConvId() == 2) {
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
			_charAction = 0;
			_vm->_gameConv->run(2);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->exportValue(_globals[kJulieNameIsKnown]);
			_vm->_gameConv->exportValue(_globals[kObservedPhan104]);
			_scene->setAnimFrame(_globals._animationIndexes[0], 55);
			_shutUpCount = 40;
		} else {
			_charAction = 2;
		}

		if (_globals[kCharlesNameIsKnown]) {
			_charHotspotId = _scene->_dynamicHotspots.add(NOUN_CHARLES, VERB_WALK_TO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(253, 52, 253 + 15, 52 + 34));
			_scene->_dynamicHotspots[_charHotspotId]._articleNumber = PREP_ON;
		} else {
			_charHotspotId = _scene->_dynamicHotspots.add(NOUN_GENTLEMAN, VERB_WALK_TO, SYNTAX_MASC_NOT_PROPER, EXT_NONE, Common::Rect(253, 52, 253 + 15, 52 + 34));
			_scene->_dynamicHotspots[_charHotspotId]._articleNumber = PREP_ON;
		}
		_scene->_dynamicHotspots.setPosition(_charHotspotId, Common::Point(235, 102), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId == 106) {
		_game._player._playerPos = Common::Point(48, 81);
		_game._player._facing = FACING_SOUTHEAST;
		_game._player.walk(Common::Point(71, 76), FACING_SOUTHEAST);
	} else if ((_scene->_priorSceneId == 104) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		if (_game._player._playerPos.y > 128) {
			_game._player._playerPos.x = 124;
			_game._player._facing = FACING_NORTHEAST;
		} else if (_game._player._playerPos.y > 99) {
			_game._player._playerPos.x = 185;
			_game._player._facing = FACING_NORTHEAST;
		} else {
			_game._player._playerPos.x = 243;
			_game._player._facing = FACING_NORTHWEST;
		}

		_game._player._playerPos.y = 143;
	}

	sceneEntrySound();
}

void Scene108::step() {
	if (_anim0ActvFl)
		handleCharAnimation();

	if ((_globals[kWalkerConverse] == 2) || (_globals[kWalkerConverse] == 3)) {
		++_conversationCount;
		if (_conversationCount > 200)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}
}

void Scene108::actions() {
	if (_vm->_gameConv->activeConvId() == 2) {
		handleCharlesConversation();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_GENTLEMAN) || _action.isAction(VERB_TALK_TO, NOUN_CHARLES)) {
		_charAction = 6;
		_game._player._stepEnabled = false;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_ONTO, NOUN_STAGE)) {
		_scene->_nextSceneId = 104;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK, NOUN_BACKSTAGE)) {
		_scene->_nextSceneId = 106;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(10810);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10730);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(10811);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_IN_TWO)) {
			_vm->_dialogs->show(10812);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_IN_ONE)) {
			_vm->_dialogs->show(10813);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROSCENIUM_ARCH)) {
			_vm->_dialogs->show(10814);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ACT_CURTAIN)) {
			_vm->_dialogs->show(10815);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEG)) {
			_vm->_dialogs->show(10816);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CYCLORAMA)) {
			_vm->_dialogs->show(10817);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLATS)) {
			_vm->_dialogs->show(10818);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGEMANAGERS_POST)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(10819);
			else
				_vm->_dialogs->show(10820);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STOOL)) {
			_vm->_dialogs->show(10821);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BACKSTAGE)) {
			_vm->_dialogs->show(10822);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(10823);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HEADSET)) {
			_vm->_dialogs->show(10824);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10826);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHARLES) || _action.isObject(NOUN_GENTLEMAN)) {
			_vm->_dialogs->show(10827);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHARLES) || _action.isAction(VERB_TAKE, NOUN_GENTLEMAN)) {
		_vm->_dialogs->show(10828);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_HEADSET)) {
		_vm->_dialogs->show(10825);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_STOOL) && (_globals[kCurrentYear] == 1993)) {
		_vm->_dialogs->show(10829);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_HEADSET)) {
		_vm->_dialogs->show(10830);
		_action._inProgress = false;
		return;
	}
}

void Scene108::preActions() {
}

void Scene108::handleCharAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _charFrame)
		return;

	_charFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int resetFrame = -1;
	int random = -1;

	switch (_charFrame) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 92:
		if (_charAction == 2)
			random = _vm->getRandomNumber(2, 15);

		if (_charAction == 6) {
			_charTalkCount = 0;
			_charAction = 1;
			random = 1;
		}

		switch (random) {
		case 1:
			resetFrame = 5;
			break;

		case 2:
			resetFrame = 0;
			break;

		case 3:
			resetFrame = 1;
			break;

		case 4:
			resetFrame = 2;
			break;

		default:
			resetFrame = 3;
			break;
		}
		break;

	case 18:
	case 19:
	case 20:
	case 21:
	case 27:
	case 33:
	case 41:
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
	case 62:
	case 73:
		if (_charFrame == 18) {
			_game._player._stepEnabled = true;
			_vm->_gameConv->run(2);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->exportValue(_globals[kJulieNameIsKnown]);
			_vm->_gameConv->exportValue(_globals[kObservedPhan104]);
		}

		if (_charAction == 1) {
			if (_handRaisedFl) {
				random = _vm->getRandomNumber(1, 3);
			} else {
				random = _vm->getRandomNumber(1, 4);
				if (random == 4)
					random = _vm->getRandomNumber(1, 4);
			}

			if (random == 4)
				_handRaisedFl = true;

			++_charTalkCount;
			if (_charTalkCount > _maxTalkCount) {
				_charAction = 0;
				_shutUpCount = 0;
				_prevShutUpFrame = 10;
				random = 12;
			}
		} else if (_charAction == 0) {
			int delay = _vm->getRandomNumber(10, 15);
			++_shutUpCount;
			if (_shutUpCount > delay) {
				random = _vm->getRandomNumber(10, 16);
				_prevShutUpFrame = random;
				if (random == 15)
					_shutUpCount = 16;
				else
					_shutUpCount = 0;
			} else {
				random = _prevShutUpFrame;
			}
		} else if (_charAction == 7) {
			_charAction = 1;
			random = 5;
		} else if (_charAction == 3) {
			_charAction = 1;
			random = 6;
		} else if (_charAction == 5) {
			_charAction = 1;
			random = 7;
		} else if (_charAction == 4) {
			_charAction = 1;
			random = 8;
		} else if (_charAction == 2) {
			random = 9;
		}

		switch (random) {
		case 1:
			resetFrame = 18;
			break;

		case 2:
			resetFrame = 19;
			break;

		case 3:
			resetFrame = 20;
			break;

		case 4:
			resetFrame = 62;
			_charTalkCount += 5;
			break;

		case 5:
			resetFrame = 21;
			break;

		case 6:
			resetFrame = 41;
			break;

		case 7:
			resetFrame = 33;
			break;

		case 8:
			resetFrame = 27;
			break;

		case 9:
			resetFrame = 74;
			break;

		case 10:
			resetFrame = 51;
			break;

		case 11:
			resetFrame = 53;
			break;

		case 12:
			resetFrame = 54;
			break;

		case 13:
			resetFrame = 55;
			break;

		case 14:
			resetFrame = 56;
			break;

		case 15:
			resetFrame = 57;
			break;

		case 16:
			resetFrame = 52;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_charFrame = resetFrame;
	}
}

void Scene108::handleCharlesConversation() {
	switch (_action._activeAction._verbId) {
	case 1:
	case 25:
	case 26:
		_globals[kWalkerConverse] = 0;
		_vm->_gameConv->setHeroTrigger(64);
		if (_action._activeAction._verbId == 26)
			_globals[kCharlesNameIsKnown] = 2;
		break;

	case 2:
		if (!_globals[kCharlesNameIsKnown]) {
			_scene->_dynamicHotspots.remove(_charHotspotId);
			_charHotspotId = _scene->_dynamicHotspots.add(NOUN_CHARLES, VERB_WALK_TO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(253, 52, 253 + 15, 52 + 34));
			_scene->_dynamicHotspots[_charHotspotId]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(_charHotspotId, Common::Point(235, 102), FACING_NORTHEAST);
			_globals[kCharlesNameIsKnown] = true;
		}
		break;

	case 5:
		_vm->_gameConv->setInterlocutorTrigger(66);
		_maxTalkCount = 35;
		break;

	case 6:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 15:
		_maxTalkCount = 35;
		break;

	case 16:
		_vm->_gameConv->setInterlocutorTrigger(68);
		_maxTalkCount = 35;
		break;

	case 19:
		_vm->_gameConv->setInterlocutorTrigger(72);
		break;

	case 22:
		_vm->_gameConv->setInterlocutorTrigger(70);
		break;

	default:
		_maxTalkCount = 15;
		break;
	}

	switch (_game._trigger) {
	case 60:
		_charAction = 1;
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		break;

	case 62:
		_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);
		_conversationCount = 0;
		_charAction = 0;
		break;

	case 64:
		_charAction = 2;
		break;

	case 66:
		_charAction = 5;
		break;

	case 68:
		_charAction = 4;
		break;

	case 70:
		_charAction = 7;
		break;

	case 72:
		_charAction = 3;
		break;

	default:
		break;
	}

	if ((_action._activeAction._verbId != 1) && (_action._activeAction._verbId != 5) &&
		(_action._activeAction._verbId != 16) && (_action._activeAction._verbId != 19) &&
		(_action._activeAction._verbId != 22) && (_action._activeAction._verbId != 25) &&
		(_action._activeAction._verbId != 26) && (_charAction != 2)) {
		_vm->_gameConv->setInterlocutorTrigger(60);
		_vm->_gameConv->setHeroTrigger(62);
	}

	_charTalkCount = 0;
	_shutUpCount = 40;
	_handRaisedFl = false;
}

/*------------------------------------------------------------------------*/

Scene109::Scene109(MADSEngine *vm) : Scene1xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;

	_currentFloor = -1;
}

void Scene109::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_anim3ActvFl);

	s.syncAsSint16LE(_currentFloor);
}

void Scene109::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene109::enter() {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RDR_6");

	if (_globals[kCurrentYear] == 1881) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));

		_scene->_hotspots.activate(NOUN_LIGHT_FIXTURE, false);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);
		_scene->_sequences.addTimer(1, 70);
	} else {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('z', 0));
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('z', 1));
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('z', 2));

		_scene->_hotspots.activate(NOUN_LAMP, false);
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 1);
		_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, 1);
		_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, 1);
	}

	if (_scene->_priorSceneId == 106) {
		_game._player._playerPos = Common::Point(31, 459);
		_game._player._facing = FACING_NORTHEAST;
		_scene->setCamera(Common::Point(0, 312));
		_scene->sceneScale(467, 95, 442, 78);
		_currentFloor = 1;
	} else if (_scene->_priorSceneId == 111) {
		if (_globals[kLeaveAngelMusicOn]) {
			_globals[kLeaveAngelMusicOn] = false;
			sceneEntrySound();
		}
		_game._player._playerPos = Common::Point(4, 136);
		_game._player._facing = FACING_EAST;
		_game._player.walk(Common::Point(32, 138), FACING_EAST);
		_game._player.setWalkTrigger(60);
		_game._player._stepEnabled = false;
		_scene->setCamera(Common::Point(0, 0));
		_scene->sceneScale(155, 95, 130, 78);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_currentFloor = 3;
	} else if ((_scene->_priorSceneId == 110) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(3, 292);
		_game._player._facing = FACING_EAST;
		_game._player.walk(Common::Point(31, 295), FACING_EAST);
		_game._player.setWalkTrigger(65);
		_game._player._stepEnabled = false;
		_scene->setCamera(Common::Point(0, 156));
		_scene->sceneScale(311, 95, 286, 78);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_currentFloor = 2;
	} else if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_currentFloor == 2) {
			_scene->setCamera(Common::Point(0, 156));
			_scene->sceneScale(311, 95, 286, 78);
		} else if (_currentFloor == 3) {
			_scene->setCamera(Common::Point(0, 0));
			_scene->sceneScale(155, 95, 130, 78);
		} else {
			_scene->setCamera(Common::Point(0, 312));
			_scene->sceneScale(467, 95, 442, 78);
		}
	}

	sceneEntrySound();
}

void Scene109::step() {
	if (_anim0ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 80)
			_game._camY.camPanTo(156);
	}

	if (_anim1ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == 80)
			_game._camY.camPanTo(0);
	}

	if (_anim2ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame() == 7)
			_game._camY.camPanTo(312);
	}

	if (_anim3ActvFl) {
		if (_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == 14)
			_game._camY.camPanTo(156);
	}

	switch (_game._trigger) {
	case 60:
		_scene->deleteSequence(_globals._sequenceIndexes[0]);
		_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_vm->_sound->command(25);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 65:
		_scene->deleteSequence(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
		break;

	case 66:
		_vm->_sound->command(25);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	if (_game._trigger == 70) {
		int rndFrame = _vm->getRandomNumber(1, 3);
		int rndDelay = _vm->getRandomNumber(4, 7);
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_scene->deleteSequence(_globals._sequenceIndexes[3]);
		_scene->deleteSequence(_globals._sequenceIndexes[4]);

		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, rndFrame);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);

		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, rndFrame);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, rndFrame);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

		_scene->_sequences.addTimer(rndDelay, 70);
	}
}

void Scene109::actions() {
	if (_action.isAction(VERB_WALK, NOUN_BACKSTAGE)) {
		_scene->_nextSceneId = 106;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_UP, NOUN_STAIRCASE)) {
		if (_currentFloor == 2) {
			switch (_game._trigger) {
			case 0:
				_game._player.walk(Common::Point(58, 295), FACING_EAST);
				_game._player.setWalkTrigger(1);
				break;

			case 1:
				_anim1ActvFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('w', 2), 2);
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[1], SYNC_PLAYER, 0);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
				_scene->sceneScale(155, 95, 130, 78);
				_anim1ActvFl = false;
				_game._player._visible = true;
				_game._player._playerPos = Common::Point(261, 137);
				_game._player.walk(Common::Point(281, 143), FACING_SOUTHWEST);
				_game._player.setWalkTrigger(3);
				break;

			case 3:
				_game._player._stepEnabled = true;
				_currentFloor = 3;
				break;

			default:
				break;
			}
		} else if (_currentFloor == 1) {
			switch (_game._trigger) {
			case 0:
				_game._player.walk(Common::Point(58, 452), FACING_EAST);
				_game._player.setWalkTrigger(1);
				break;

			case 1:
				_anim0ActvFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('w', 1), 2);
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
				break;

			case 2:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
				_scene->sceneScale(311, 95, 286, 78);
				_anim0ActvFl = false;
				_game._player._visible = true;
				_game._player._playerPos = Common::Point(264, 295);
				_game._player.walk(Common::Point(289, 299), FACING_SOUTHWEST);
				_game._player.setWalkTrigger(3);
				break;

			case 3:
				_game._player._stepEnabled = true;
				_currentFloor = 2;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_DOWN, NOUN_STAIRCASE)) {
		if (_currentFloor == 2) {
			switch (_game._trigger) {
			case 0:
				_game._player.walk(Common::Point(287, 291), FACING_WEST);
				_game._player.setWalkTrigger(1);
				break;

			case 1:
				_game._player.walk(Common::Point(269, 292), FACING_WEST);
				_game._player.setWalkTrigger(2);
				break;

			case 2:
				_anim2ActvFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('w', 3), 3);
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[2], SYNC_PLAYER, 0);
				break;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[2]);
				_scene->sceneScale(467, 95, 442, 78);
				_anim2ActvFl = false;
				_game._player._visible = true;
				_game._player._playerPos = Common::Point(61, 450);
				_game._player.walk(Common::Point(36, 450), FACING_SOUTHEAST);
				_game._player.setWalkTrigger(4);
				break;

			case 4:
				_game._player._stepEnabled = true;
				_currentFloor = 1;
				break;

			default:
				break;
			}
		} else if (_currentFloor == 3) {
			switch (_game._trigger) {
			case 0:
				_game._player.walk(Common::Point(287, 137), FACING_WEST);
				_game._player.setWalkTrigger(1);
				break;

			case 1:
				_game._player.walk(Common::Point(269, 138), FACING_WEST);
				_game._player.setWalkTrigger(2);
				break;

			case 2:
				_anim3ActvFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('w', 4), 3);
				_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[3], SYNC_PLAYER, 0);
				break;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[3]);
				_scene->sceneScale(311, 95, 286, 78);
				_anim3ActvFl = false;
				_game._player._visible = true;
				_game._player._playerPos = Common::Point(59, 296);
				_game._player.walk(Common::Point(40, 294), FACING_SOUTHEAST);
				_game._player.setWalkTrigger(4);
				break;

			case 4:
				_game._player._stepEnabled = true;
				_currentFloor = 2;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR) || _game._trigger) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], true, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[8], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 4, 75);
			break;

		case 2:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[8]);
			if (_currentFloor == 3)
				_game._player.walk(Common::Point(2, 131), FACING_WEST);
			else if (_currentFloor == 2)
				_game._player.walk(Common::Point(2, 281), FACING_WEST);

			_scene->_sequences.addTimer(180, 3);
			break;

		case 3:
			if (_currentFloor == 3)
				_scene->_nextSceneId = 111;
			else if (_currentFloor == 2)
				_scene->_nextSceneId = 110;
			break;

		case 75:
			_vm->_sound->command(24);
			if (_currentFloor == 3) {
				_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 3);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
			} else if (_currentFloor == 2) {
				_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 3);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
			}
			break;

		case 76:
			if (_currentFloor == 3) {
				int idx = _globals._sequenceIndexes[0];
				_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 3);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[0], SYNC_SEQ, idx);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			} else if (_currentFloor == 2) {
				int idx = _globals._sequenceIndexes[1];
				_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 3);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_SEQ, idx);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
			}
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(10910);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_STAIRCASE)) {
			if (_currentFloor == 1)
				_vm->_dialogs->show(10911);
			else if (_currentFloor == 2)
				_vm->_dialogs->show(10921);
			else if (_currentFloor == 3)
				_vm->_dialogs->show(10922);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(10912);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BACKSTAGE)) {
			_vm->_dialogs->show(10913);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(10914);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RAILING)) {
			_vm->_dialogs->show(10915);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(10916);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT_FIXTURE)) {
			_vm->_dialogs->show(10917);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LAMP)) {
			_vm->_dialogs->show(10918);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOLE)) {
			_vm->_dialogs->show(10919);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(10920);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_LAMP)) {
		_vm->_dialogs->show(10924);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_DOOR)) {
		_vm->_dialogs->show(10923);
		_action._inProgress = false;
		return;
	}
}

void Scene109::preActions() {
	if (_action.isAction(VERB_OPEN, NOUN_DOOR)) {
		if (_currentFloor == 3)
			_game._player.walk(Common::Point(32, 138), FACING_WEST);
		else if (_currentFloor == 2)
			_game._player.walk(Common::Point(31, 295), FACING_WEST);
	}
}

/*------------------------------------------------------------------------*/

Scene110::Scene110(MADSEngine *vm) : Scene1xx(vm) {
}

void Scene110::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
}

void Scene110::setup() {
	if (_globals[kCurrentYear] == 1993)
		_scene->_variant = 1;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene110::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RDR_9");

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_POSTER, false);
		_scene->_hotspots.activate(NOUN_WASTE_BASKET, false);

		if (_globals[kDoneBrieConv203] == 0) {
			if (_globals[kJuliesDoor] == 0) {
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 3);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
			}
		} else {
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		}
	} else {
		_scene->_hotspots.activate(NOUN_BULLETIN_BOARD, false);
		_scene->_hotspots.activate(NOUN_PAPER, false);
		_scene->_hotspots.activate(NOUN_TRASH_BUCKET, false);
	}

	if (_scene->_priorSceneId == 112) {
		_game._player._playerPos = Common::Point(261, 121);
		_game._player._facing = FACING_SOUTH;
		_game._player.walk(Common::Point(221, 131), FACING_SOUTH);
	} else if ((_scene->_priorSceneId == 109) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player.firstWalk(Common::Point(335, 150), FACING_WEST, Common::Point(310, 150), FACING_WEST, true);
	}

	sceneEntrySound();
}

void Scene110::step() {
}

void Scene110::actions() {
	if ((_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_RIGHT_DOOR) || _action.isAction(VERB_LOCK, NOUN_RIGHT_DOOR))
	 && (_globals[kDoneBrieConv203] == 0) && (_globals[kCurrentYear] == 1993) && (_globals[kJuliesDoor] == 0)) {
		_scene->_nextSceneId = 112;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_LEFT_DOOR) || _action.isAction(VERB_OPEN, NOUN_LEFT_DOOR)
	 || _action.isAction(VERB_UNLOCK, NOUN_LEFT_DOOR) || _action.isAction(VERB_LOCK, NOUN_LEFT_DOOR)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int idx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 4);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_SEQ, idx);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], false);
			_scene->_sequences.addTimer(30, 2);
			_vm->_sound->command(73);
			}
			break;

		case 2:
			_scene->deleteSequence(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], false);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			if (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK))
				_vm->_dialogs->show(32);
			else
				_vm->_dialogs->show(11022);

			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_DOOR) || _action.isAction(VERB_OPEN, NOUN_RIGHT_DOOR)
	 || _action.isAction(VERB_UNLOCK, NOUN_RIGHT_DOOR) || _action.isAction(VERB_LOCK, NOUN_RIGHT_DOOR)) {
		if (((_globals[kCurrentYear] == 1881) || (_globals[kDoneBrieConv203] >= 1))
		 && !_action.isAction(VERB_UNLOCK, NOUN_RIGHT_DOOR) && !_action.isAction(VERB_LOCK, NOUN_RIGHT_DOOR)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1: {
				int idx = _globals._sequenceIndexes[1];
				_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 4);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_SEQ, idx);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], false);
				_scene->_sequences.addTimer(30, 2);
				_vm->_sound->command(73);
				}
				break;

			case 2:
				_scene->deleteSequence(_globals._sequenceIndexes[1]);
				_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				if (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK))
					_vm->_dialogs->show(32);
				else
					_vm->_dialogs->show(11023);
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		} else if (_globals[kJuliesDoor] == 1) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 7, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 4, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 1:
				_vm->_sound->command(24);
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 10, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 3);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
				break;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
				_game._player._visible = true;
				_game._player.walk(Common::Point(261, 121), FACING_NORTHEAST);
				_game._player.setWalkTrigger(4);
				break;

			case 4:
				_scene->_nextSceneId = 112;
				_globals[kJuliesDoor] = 0;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(11010);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(11011);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(11012);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(11013);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAIRWELL)) {
			_vm->_dialogs->show(11014);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RIGHT_DOOR)) {
			if (_globals[kDoneBrieConv203] >= 1)
				_vm->_dialogs->show(11016);
			else if (_globals[kChrisFStatus] == 1)
				_vm->_dialogs->show(11015);
			else
				_vm->_dialogs->show(11016);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEFT_DOOR)) {
			_vm->_dialogs->show(11016);
			_action._inProgress = false;
			return;
		}

		if ((_action.isObject(NOUN_WASTE_BASKET)) || (_action.isObject(NOUN_TRASH_BUCKET))) {
			_vm->_dialogs->show(11017);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_POSTER)) {
			_vm->_dialogs->show(11018);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BULLETIN_BOARD)) {
			_vm->_dialogs->show(11019);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PAPER)) {
			_vm->_dialogs->show(11029);
			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_TAKE, NOUN_WASTE_BASKET) || _action.isAction(VERB_TAKE, NOUN_TRASH_BUCKET)) {
			_vm->_dialogs->show(11020);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT_FIXTURE)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11021);
			else
				_vm->_dialogs->show(11028);

			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_PAPER)) {
		_vm->_dialogs->show(11030);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_RIGHT_DOOR)) {
		if (_globals[kDoneBrieConv203] >= 1)
			_vm->_dialogs->show(11026);
		else if (_globals[kChrisFStatus] == 1)
			_vm->_dialogs->show(11024);
		else
			_vm->_dialogs->show(11026);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_LEFT_DOOR)) {
		_vm->_dialogs->show(11025);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_RIGHT_DOOR)) {
		if ((_globals[kChrisFStatus] == 1) && (_globals[kDoneBrieConv203] == 0))
			_vm->_dialogs->show(11027);

		_action._inProgress = false;
		return;
	}
}

void Scene110::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_STAIRWELL))
		_game._player._walkOffScreenSceneId = 109;

	if (_action.isAction(VERB_OPEN, NOUN_LEFT_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_LEFT_DOOR) || _action.isAction(VERB_LOCK, NOUN_LEFT_DOOR))
		_game._player.walk(Common::Point(111, 126), FACING_NORTHEAST);

	if (_action.isAction(VERB_OPEN, NOUN_RIGHT_DOOR) || _action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_DOOR)
	 || _action.isAction(VERB_UNLOCK, NOUN_RIGHT_DOOR) || _action.isAction(VERB_LOCK, NOUN_RIGHT_DOOR)) {
		if ((_globals[kCurrentYear] == 1881) || (_globals[kDoneBrieConv203] >= 1))
			_game._player.walk(Common::Point(221, 131), FACING_NORTHEAST);
		else if ((_globals[kJuliesDoor] == 1) || _action.isAction(VERB_OPEN))
			_game._player.walk(Common::Point(223, 128), FACING_NORTHEAST);
		else if (_globals[kJuliesDoor] == 0)
			_game._player.walk(Common::Point(261, 120), FACING_NORTHEAST);
	}

	if (_action.isAction(VERB_LOOK, NOUN_PAPER))
		_game._player._needToWalk = true;
}

/*------------------------------------------------------------------------*/

Scene111::Scene111(MADSEngine *vm) : Scene1xx(vm) {
	_removeAxe = false;
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_closedFl = false;
	_listenFrame = -1;
	_listenStatus = -1;

}

void Scene111::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_removeAxe);
	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_closedFl);
	s.syncAsSint16LE(_listenFrame);
	s.syncAsSint16LE(_listenStatus);
}

void Scene111::setup() {
	if (_globals[kCurrentYear] == 1993)
		_scene->_variant = 1;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene111::enter() {
	_scene->_hotspots.activate(NOUN_HOOK, false);

	_removeAxe = false;
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_closedFl = true;

	if (_globals[kCurrentYear] == 1881) {
		if ((_globals[kJacquesNameIsKnown] == 2) && (_globals[kMadameNameIsKnown] == 2) && (_globals[kPanelIn206]) &&
			(_globals[kDoneRichConv203]) && (_game._objects.isInInventory(OBJ_LANTERN)) &&
			((_game._objects.isInInventory(OBJ_CABLE_HOOK) && _game._objects.isInInventory(OBJ_ROPE)) || _game._objects.isInInventory(OBJ_ROPE_WITH_HOOK))) {
				_closedFl = false;
		} else
			_closedFl = true;
	} else
		_closedFl = false;

	if (_globals[kJacquesStatus]) {
		_scene->_hotspots.activate(NOUN_HOOK, true);
		_scene->_hotspots.activate(NOUN_FIRE_AXE, false);
	}

	_vm->_gameConv->load(14);

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*RDR_9");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 0));

	if (_globals[kCurrentYear] == 1881)
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 2));

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_scene->drawToBackground(_globals._spriteIndexes[1], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_LAMP, false);
		int idx = _scene->_dynamicHotspots.add(NOUN_WALL, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(35, 82, 35 + 13, 82 + 11));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(78, 122), FACING_NORTHEAST);
	} else {
		_scene->_hotspots.activate(NOUN_LIGHT, false);
		if (_globals[kChristineDoorStatus] == 2) {
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 10);
			_scene->drawToBackground(_globals._spriteIndexes[4], 1, Common::Point(-32000, -32000), 0, 100);
			_scene->_hotspots.activate(NOUN_FIRE_AXE, false);
			_scene->_hotspots.activate(NOUN_DOOR_CHUNKS, true);
			_scene->_hotspots.activate(NOUN_HANDLE, true);
			_scene->_hotspots.activate(NOUN_AXE, true);
		}
	}

	if ((_globals[kChristineDoorStatus] == 0) && (_scene->_priorSceneId != 113) && !_closedFl) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
	}

	if ((_globals[kCurrentYear] == 1993) || (_globals[kChristineDoorStatus] <= 1)) {
		if (!_globals[kJacquesStatus]) {
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 5);
			_scene->_hotspots.activate(NOUN_AXE, false);
		}
		_scene->_hotspots.activate(NOUN_DOOR_CHUNKS, false);
		_scene->_hotspots.activate(NOUN_HANDLE, false);
	}

	if (_scene->_priorSceneId == 113) {
		_game._player._playerPos = Common::Point(146, 108);
		_game._player.walk(Common::Point(112, 126), FACING_SOUTH);

		if (!_globals[kLeaveAngelMusicOn])
			sceneEntrySound();

		if (_globals[kChristineDoorStatus] != 2) {
			_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, 3);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);

			if ((_globals[kCurrentYear] == 1881) && (_globals[kLeaveAngelMusicOn])) {
				_game._player.setWalkTrigger(60);
				_game._player._stepEnabled = false;
				_globals[kChristineDoorStatus] = 1;
				_globals[kChrisKickedRaoulOut] = true;
				_globals[kHintThatDaaeIsHome1] = true;
				_globals[kHintThatDaaeIsHome2] = true;
			}
		}
	} else if ((_scene->_priorSceneId == 109) || (_scene->_priorSceneId != RETURNING_FROM_LOADING))
		_game._player.firstWalk(Common::Point(335, 150), FACING_WEST, Common::Point(311, 150), FACING_WEST, true);

	sceneEntrySound();
}

void Scene111::step() {
	if (_anim0ActvFl)
		handleListenAnimation();

	if (!_removeAxe && _anim1ActvFl && (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == 5)) {
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_game._objects.addToInventory(OBJ_FIRE_AXE);
		_removeAxe = true;
	}

	if (_game._objects.isInInventory(OBJ_FIRE_AXE) && _anim1ActvFl && (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == 36))
		_game._objects.setRoom(OBJ_FIRE_AXE, NOWHERE);

	if (_game._player._playerPos == Common::Point(145, 108))
		_scene->_nextSceneId = 113;

	switch (_game._trigger) {
	case 60:
		_scene->deleteSequence(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[5], false, 7, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 62);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		break;

	case 62:
		_vm->_sound->command(25);
		_game._player._stepEnabled = true;
		_vm->_gameConv->run(14);
		if (_game._difficulty == DIFFICULTY_EASY)
			_vm->_gameConv->exportValue(1);
		else
			_vm->_gameConv->exportValue(0);

		_game._player.walk(Common::Point(119, 124), FACING_EAST);
		_game._player.setWalkTrigger(64);
		break;

	case 64:
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('l', 1), 70);
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
		_anim0ActvFl = true;
		_listenStatus = 0;
		_game._player._visible = false;
		break;

	case 70:
		_game._player._visible = true;
		_anim0ActvFl = false;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
		_scene->_sequences.addTimer(30, 71);
		break;

	case 71:
		_vm->_gameConv->release();
		break;

	default:
		break;
	}
}

void Scene111::actions() {
	if (_vm->_gameConv->activeConvId() == 14) {
		handleListenConversation();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_DOOR) || _action.isAction(VERB_OPEN, NOUN_RIGHT_DOOR) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_LOCK)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_action._inProgress = false;
			return;

		case 1: {
			int idx = _globals._sequenceIndexes[0];
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 4);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[0], SYNC_SEQ, idx);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], false);
			_scene->_sequences.addTimer(30, 2);
			_vm->_sound->command(73);
			_action._inProgress = false;
			return;
			}

		case 2:
			_scene->deleteSequence(_globals._sequenceIndexes[0]);
			_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], false);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			_action._inProgress = false;
			return;

		case 3:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
			_game._player._visible = true;
			if (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK))
				_vm->_dialogs->show(32);
			else
				_vm->_dialogs->show(11123);

			_game._player._stepEnabled = true;
			_action._inProgress = false;
			return;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_LEFT_DOOR) || _action.isAction(VERB_OPEN, NOUN_LEFT_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_LEFT_DOOR) || _action.isAction(VERB_LOCK, NOUN_LEFT_DOOR)) {
		if ((_globals[kChristineDoorStatus] == 1) || _closedFl || _action.isAction(VERB_UNLOCK, NOUN_LEFT_DOOR) || _action.isAction(VERB_LOCK, NOUN_LEFT_DOOR)) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_action._inProgress = false;
				return;

			case 1: {
				int idx = _globals._sequenceIndexes[0];
				_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 4);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[0], SYNC_SEQ, idx);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], false);
				_scene->_sequences.addTimer(30, 2);
				_vm->_sound->command(73);
				_action._inProgress = false;
				return;
				}

			case 2:
				_scene->deleteSequence(_globals._sequenceIndexes[0]);
				_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[0], false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				_action._inProgress = false;
				return;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[0]);
				_game._player._visible = true;

				if (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK))
					_vm->_dialogs->show(32);
				else if ((_globals[kChrisKickedRaoulOut]) && (_globals[kTicketPeoplePresent] == 1) && (_globals[kJacquesStatus] == 0))
					_vm->_dialogs->show(11135);
				else
					_vm->_dialogs->show(11124);

				_game._player._stepEnabled = true;
				_action._inProgress = false;
				return;

			default:
				break;
			}
		} else if (_globals[kChristineDoorStatus] == 2) {
			_vm->_dialogs->show(11137);
			_action._inProgress = false;
			return;
		} else {
			_vm->_dialogs->show(11126);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_FIRE_AXE)) {
		if ((_globals[kCurrentYear] == 1881) && (_globals[kJacquesStatus] == 0) && (_globals[kChristineDoorStatus] == 1) && (_globals[kChrisKickedRaoulOut])) {
			switch (_game._trigger) {
			case 0:
				_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('q', -1), 1);
				_scene->_hotspots.activate(NOUN_FIRE_AXE, false);
				_scene->_hotspots.activate(NOUN_DOOR_CHUNKS, true);
				_scene->_hotspots.activate(NOUN_HANDLE, true);
				_scene->_hotspots.activate(NOUN_AXE, true);
				_anim1ActvFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				break;

			case 1:
				_globals[kChristineDoorStatus] = 2;
				_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[3], SYNC_ANIM, _globals._animationIndexes[1]);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 10);
				_scene->drawToBackground(_globals._spriteIndexes[4], 1, Common::Point(-32000, -32000), 0, 100);
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
				_game._player._visible = true;
				_game._player._playerPos = Common::Point(126, 126);
				_anim1ActvFl = false;
				_game._player.walk(Common::Point(145, 108), FACING_NORTHEAST);
				_game._player.resetFacing(FACING_NORTHWEST);
				break;

			default:
				break;
			}
		} else if (_globals[kCurrentYear] == 1993 && _globals[kMakeBrieLeave203])
			_vm->_dialogs->show(11142);
		else
			_vm->_dialogs->show(11134);
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(11110);
		else
			_vm->_dialogs->show(11111);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(11112);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(11113);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(11114);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAIRWELL)) {
			_vm->_dialogs->show(11115);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TABLE)) {
			_vm->_dialogs->show(11118);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PLANT)) {
			_vm->_dialogs->show(11119);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FIRE_AXE)) {
			_vm->_dialogs->show(11120);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HANDLE)) {
			_vm->_dialogs->show(11121);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_AXE)) {
			_vm->_dialogs->show(11122);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR_CHUNKS)) {
			_vm->_dialogs->show(11129);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT)) {
			_vm->_dialogs->show(11131);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LAMP)) {
			_vm->_dialogs->show(11132);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOOK)) {
			_vm->_dialogs->show(11141);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEFT_DOOR)) {
			if (_globals[kJacquesStatus])
				_vm->_dialogs->show(11140);
			else if (_globals[kChristineDoorStatus] == 2)
				_vm->_dialogs->show(11130);
			else if ((_globals[kChrisKickedRaoulOut]) && (_globals[kTicketPeoplePresent] == 1))
				_vm->_dialogs->show(11136);
			else if ((_globals[kChristineDoorStatus] == 1) || _closedFl)
				_vm->_dialogs->show(11117);
			else if (_globals[kChristineDoorStatus] == 0)
				_vm->_dialogs->show(11116);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RIGHT_DOOR)) {
			_vm->_dialogs->show(11117);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_CLOSE, NOUN_RIGHT_DOOR)) {
		_vm->_dialogs->show(11128);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_LEFT_DOOR)) {
		if (_closedFl)
			_vm->_dialogs->show(11128);
		else if ((_globals[kChristineDoorStatus] == 2) && (!_globals[kTicketPeoplePresent]))
			_vm->_dialogs->show(11138);
		else if (_globals[kChristineDoorStatus] == 1)
			_vm->_dialogs->show(11127);
		else if (_globals[kChristineDoorStatus] == 0)
			_vm->_dialogs->show(11125);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_LEFT_DOOR)) {
		if (_globals[kChristineDoorStatus] == 0)
			_vm->_dialogs->show(11126);
		_action._inProgress = false;
		return;
	}
}

void Scene111::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_STAIRWELL)) {
		if (_globals[kLeaveAngelMusicOn]) {
			_vm->_dialogs->show(11139);
			_game._player._needToWalk = false;
			_game._player.cancelCommand();
		} else
			_game._player._walkOffScreenSceneId = 109;
	}

	if (_action.isAction(VERB_TAKE, NOUN_FIRE_AXE) && _game._objects.isInRoom(OBJ_FIRE_AXE) && (_globals[kCurrentYear] == 1881)
	 && (_globals[kChristineDoorStatus] == 1) && (_globals[kChrisKickedRaoulOut]) && (_globals[kJacquesStatus] == 0))
		_game._player.walk(Common::Point(119, 124), FACING_EAST);

	if (_action.isAction(VERB_OPEN, NOUN_RIGHT_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_RIGHT_DOOR) || _action.isAction(VERB_LOCK, NOUN_RIGHT_DOOR))
		_game._player.walk(Common::Point(219, 131), FACING_NORTHEAST);

	if (_action.isAction(VERB_OPEN, NOUN_LEFT_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_LEFT_DOOR) || _action.isAction(VERB_LOCK, NOUN_LEFT_DOOR))
		_game._player.walk(Common::Point(109, 124), FACING_NORTHEAST);

	if ((_action.isAction(VERB_WALK_THROUGH, NOUN_LEFT_DOOR) || _action.isAction(VERB_UNLOCK, NOUN_LEFT_DOOR) || _action.isAction(VERB_LOCK, NOUN_LEFT_DOOR))
	 && !_closedFl && ((_globals[kChristineDoorStatus] == 2) || (_globals[kChristineDoorStatus] == 0)))
		_game._player.walk(Common::Point(145, 108), FACING_NORTHEAST);
}

void Scene111::handleListenAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _listenFrame)
		return;

	int nextFrame = -1;
	_listenFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();

	if (_listenFrame == 6) {
		if (_listenStatus == 0)
			nextFrame = 5;
		else if (_listenStatus == 1)
			nextFrame = 7;
	}

	if (nextFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], nextFrame);
		_listenFrame = nextFrame;
	}
}

void Scene111::handleListenConversation() {
	if (_action._activeAction._verbId == 1)
		_vm->_gameConv->setInterlocutorTrigger(66);

	if (_action._activeAction._verbId == 7)
		_vm->_gameConv->setInterlocutorTrigger(68);

	if (_game._trigger == 66)
		_listenStatus = 0;

	if (_game._trigger == 68) {
		_listenStatus = 1;
		_vm->_gameConv->hold();
	}

	if ((_action._activeAction._verbId != 1) && (_action._activeAction._verbId != 7))
		_vm->_gameConv->setInterlocutorTrigger(72);
}

/*------------------------------------------------------------------------*/

Scene112::Scene112(MADSEngine *vm) : Scene1xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_raoulAction = -1;
	_raoulFrame = -1;
	_didOptionFl = -1;
	_julieFrame = -1;
	_julieAction = -1;
	_julieHotspotId = -1;
	_julieCounter = -1;
}

void Scene112::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsSint16LE(_raoulAction);
	s.syncAsSint16LE(_raoulFrame);
	s.syncAsSint16LE(_didOptionFl);
	s.syncAsSint16LE(_julieFrame);
	s.syncAsSint16LE(_julieAction);
	s.syncAsSint16LE(_julieCounter);
}

void Scene112::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_WOMAN);
	_scene->addActiveVocab(NOUN_JULIE);
}

void Scene112::enter() {
	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_raoulAction = 2;
		_didOptionFl = 0;
	}

	_vm->_gameConv->load(3);

	_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('j', 1), 1);
	_anim0ActvFl = true;
	_julieAction = 2;
	_scene->setAnimFrame(_globals._animationIndexes[0], 3);
	if (_globals[kJulieNameIsKnown] == 2) {
		_julieHotspotId = _scene->_dynamicHotspots.add(NOUN_JULIE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(255, 82, 255 + 30, 82 + 44));
		_scene->_dynamicHotspots[_julieHotspotId]._articleNumber = PREP_ON;
		_scene->_dynamicHotspots.setPosition(_julieHotspotId, Common::Point(216, 137), FACING_NORTHEAST);
		_scene->_hotspots.activate(NOUN_WOMAN, false);
	}
	_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('r', 1), 1);
	_anim1ActvFl = true;
	_scene->setAnimFrame(_globals._animationIndexes[1], 82);
	_raoulAction = 2;

	if (_vm->_gameConv->restoreRunning() == 3) {
		_vm->_gameConv->run(3);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_scene->setAnimFrame(_globals._animationIndexes[1], 17);
		_raoulAction = 1;
		_game._player._playerPos = Common::Point(53, 128);
		_game._player._facing = FACING_EAST;
		_game._player._visible = false;
	} else if ((_scene->_priorSceneId == 110) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(128, 145);
		_game._player._facing = FACING_NORTH;
	}

	sceneEntrySound();
}

void Scene112::step() {
	if (_anim0ActvFl)
		handleJulieAnimation();

	if (_anim1ActvFl)
		handleRaoulChair();
}

void Scene112::actions() {
	if (_vm->_gameConv->activeConvId() == 3) {
		handleConversation();
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_TALK_TO, NOUN_JULIE)) || (_action.isAction(VERB_TALK_TO, NOUN_WOMAN))) {
		if (_globals[kJulieNameIsKnown] > 0) {
			_game._player._visible = false;
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[1], SYNC_PLAYER, 0);
			_scene->setAnimFrame(_globals._animationIndexes[1], 1);
			_raoulAction = 1;
		}
		_vm->_gameConv->run(3);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_CORRIDOR)) {
		_scene->_nextSceneId = 110;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(11210);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(11211);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(11212);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MIRROR)) {
			_vm->_dialogs->show(11213);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BALLET_BAR)) {
			_vm->_dialogs->show(11214);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CORRIDOR)) {
			_vm->_dialogs->show(11215);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_THROW_RUGS)) {
			_vm->_dialogs->show(11216);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DRESSING_SCREEN)) {
			_vm->_dialogs->show(11217);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DRESSING_TABLE)) {
			_vm->_dialogs->show(11218);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHAIR)) {
			_vm->_dialogs->show(11219);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PLANT)) {
			_vm->_dialogs->show(11220);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COAT_RACK)) {
			_vm->_dialogs->show(11221);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_UMBRELLA)) {
			_vm->_dialogs->show(11222);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PAINTINGS)) {
			_vm->_dialogs->show(11223);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TRASH_BUCKET)) {
			_vm->_dialogs->show(11224);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SHELF)) {
			_vm->_dialogs->show(11225);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CONTAINER)) {
			_vm->_dialogs->show(11226);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TORN_POSTER)) {
			_vm->_dialogs->show(11227);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_POSTER)) {
			_vm->_dialogs->show(11228);
			_action._inProgress = false;
			return;
		}

		if ((_action.isObject(NOUN_REVIEW)) || (_action.isObject(NOUN_REVIEWS))) {
			_vm->_dialogs->show(11229);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_JULIE)) {
			_vm->_dialogs->show(11231);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COSTUME_RACK)) {
			_vm->_dialogs->show(11232);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT_FIXTURE)) {
			_vm->_dialogs->show(11233);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WARDROBE)) {
			_vm->_dialogs->show(11234);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WOMAN)) {
			_vm->_dialogs->show(11237);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE)) {
		if (_action.isObject(NOUN_WOMAN) || _action.isObject(NOUN_JULIE)) {
			_vm->_dialogs->show(11238);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_WARDROBE)) {
		_vm->_dialogs->show(11235);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_CONTAINER)) {
		_vm->_dialogs->show(11236);
		_action._inProgress = false;
		return;
	}
}

void Scene112::preActions() {
	if ((_action.isAction(VERB_TALK_TO, NOUN_JULIE) || _action.isAction(VERB_TALK_TO, NOUN_WOMAN)) && (_globals[kJulieNameIsKnown] > 0))
		_game._player.walk(Common::Point(53, 128), FACING_EAST);
}

void Scene112::handleConversation() {
	switch (_action._activeAction._verbId) {
	case 1:
		if (!_game._trigger && (_raoulAction == 2))
			_vm->_gameConv->setInterlocutorTrigger(68);
		break;

	case 3:
	case 4:
		if (_globals[kJulieNameIsKnown] < 2) {
			_globals[kJulieNameIsKnown] = 2;
			_scene->_hotspots.activate(NOUN_WOMAN, false);
			_julieHotspotId = _scene->_dynamicHotspots.add(NOUN_JULIE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(255, 82, 255 + 30, 82 + 44));
			_scene->_dynamicHotspots[_julieHotspotId]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(_julieHotspotId, Common::Point(216, 137), FACING_NORTHEAST);
		}
		break;

	case 5:
		_vm->_gameConv->setInterlocutorTrigger(74);
		_globals[kJulieNameIsKnown] = 1;
		break;

	case 6:
	case 7:
	case 10:
	case 23:
	case 27:
		_vm->_gameConv->setInterlocutorTrigger(74);
		_globals[kJulieNameIsKnown] = 2;
		break;

	case 13:
		_julieAction = 1;
		_raoulAction = 1;
		break;

	case 14:
	case 15:
	case 16:
		_julieAction = 0;
		_raoulAction = 1;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 60:
		_julieAction = 0;
		_raoulAction = 1;
		break;

	case 62:
		_julieAction = 2;
		if (_raoulAction != 2)
			_raoulAction = 0;
		break;

	case 68:
		_scene->_sequences.addTimer(120, 70);
		_vm->_gameConv->hold();
		break;

	case 70:
		_game._player.walk(Common::Point(53, 128), FACING_EAST);
		_game._player.setWalkTrigger(72);
		break;

	case 72:
		_vm->_gameConv->release();
		_game._player._visible = false;
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[1], SYNC_PLAYER, 0);
		_scene->setAnimFrame(_globals._animationIndexes[1], 1);
		_raoulAction = 1;
		break;

	case 74:
		_raoulAction = 3;
		_vm->_gameConv->hold();
		break;

	default:
		break;
	}

	if ((_action._activeAction._verbId != 0) && (_action._activeAction._verbId != 1) &&
		(_action._activeAction._verbId != 2) && (_action._activeAction._verbId != 5) &&
		(_action._activeAction._verbId != 6) && (_action._activeAction._verbId != 7) &&
		(_action._activeAction._verbId != 10) && (_action._activeAction._verbId != 23) &&
		(_action._activeAction._verbId != 27)) {
		_vm->_gameConv->setInterlocutorTrigger(60);
		_vm->_gameConv->setHeroTrigger(62);
	}

	if (_action._activeAction._verbId == 18)
		_globals[kCanFindBookInLibrary] = true;

	_julieCounter = 0;
}

void Scene112::handleJulieAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _julieFrame)
		return;

	_julieFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int random = -1;
	int resetFrame = -1;

	switch (_julieFrame) {
	case 1:
	case 2:
	case 3:
	case 22:
	case 39:
	case 71:
	case 107:
		switch (_julieAction) {
		case 0:
			random = _vm->getRandomNumber(4, 6);
			++_julieCounter;
			if (_julieCounter > 25)
				_julieAction = 2;
			break;

		case 1:
			random = 3;
			break;

		case 2:
			random = _vm->getRandomNumber(1, 3);
			while (_didOptionFl == random)
				random = _vm->getRandomNumber(1, 3);
			_didOptionFl = random;
			break;

		default:
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 22;
			break;

		case 2:
			resetFrame = 39;
			break;

		case 3:
			resetFrame = 83;
			break;

		case 4:
			resetFrame = 0;
			break;

		case 5:
			resetFrame = 1;
			break;

		case 6:
			resetFrame = 2;
			break;

		default:
			break;
		}
		break;
	case 56:
	case 77:
	case 83:
		random = _vm->getRandomNumber(1, 3);
		while (_didOptionFl == random)
			random = _vm->getRandomNumber(1, 3);
		_didOptionFl = random;

		if ((_julieAction == 0) || (_julieAction == 1))
			random = 3;

		switch (random) {
		case 1:
			resetFrame = 71;
			break;

		case 2:
			resetFrame = 77;
			break;

		case 3:
			resetFrame = 56;
			break;

		default:
			break;
		}

		break;

	case 95:
		if (_julieAction == 1)
			random = 2;
		else
			random = 1;

		switch (random) {
		case 1:
			resetFrame = 95;
			break;

		case 2:
			resetFrame = 107;
			break;

		default:
			break;
		}
		break;

	case 130:
		resetFrame = 95;
		_julieAction = 2;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_julieFrame = resetFrame;
	}
}

void Scene112::handleRaoulChair() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _raoulFrame)
		return;

	_raoulFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;
	int random = -1;

	switch (_raoulFrame) {
	case 18:
	case 30:
	case 41:
	case 49:
	case 56:
	case 65:
		switch (_raoulAction) {
		case 0:
			random = _vm->getRandomNumber(1, 4);
			_raoulAction = 1;
			break;

		case 2:
			random = 6;
			break;

		case 3:
			random = 5;
			_raoulAction = 2;
			break;

		default:
			random = _vm->getRandomNumber(7, 50);
			while (_didOptionFl == random)
				random = _vm->getRandomNumber(7, 50);
			_didOptionFl = random;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 49;
			break;

		case 2:
			resetFrame = 41;
			break;

		case 3:
			resetFrame = 18;
			break;

		case 4:
			resetFrame = 30;
			break;

		case 5:
			resetFrame = 65;
			break;

		case 6:
			resetFrame = 82;
			break;

		case 7:
			resetFrame = 56;
			break;

		default:
			resetFrame = 17;
			break;
		}
		break;

	case 61:
		if (_raoulAction == 1)
			random = 1;
		else
			random = 2;

		switch (random) {
		case 1:
			resetFrame = 60;
			break;

		case 2:
			resetFrame = 61;
			break;

		default:
			break;
		}
		break;

	case 82:
		_raoulAction = 2;
		_game._player._visible = true;
		_vm->_gameConv->release();
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		break;

	case 83:
		if (_raoulAction == 2)
			random = 1;
		else
			random = 2;

		switch (random) {
		case 1:
			resetFrame = 82;
			break;

		case 2:
			resetFrame = 0;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_raoulFrame = resetFrame;
	}
}

/*------------------------------------------------------------------------*/

Scene113::Scene113(MADSEngine *vm) : Scene1xx(vm) {
	_standingAndTalking = false;
	_dayWantsToTalk = false;
	_musicPlaying = false;
	_afterKissFl = false;
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;
	_anim4ActvFl = false;
	_prevent1 = false;
	_prevent2 = false;
	_raoulStandingFl = false;
	_armsOutFl = false;

	_dayStatus = -1;
	_raoulAction = -1;
	_christineHotspotId1 = -1;
	_christineHotspotId2 = -1;
	_raoulCount = -1;
	_dayCount = -1;
	_standCount = -1;
	_julieStatus = -1;
	_florentStatus = -1;
	_florentFrame = -1;
	_florentCount = -1;
	_dayFrame = -1;
	_lastDayResetFrame = -1;
	_raoulFrame = -1;
	_julieFrame = -1;
	_julieCount = -1;
}

void Scene113::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_standingAndTalking);
	s.syncAsByte(_dayWantsToTalk);
	s.syncAsByte(_musicPlaying);
	s.syncAsByte(_afterKissFl);
	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_anim3ActvFl);
	s.syncAsByte(_anim4ActvFl);
	s.syncAsByte(_prevent1);
	s.syncAsByte(_prevent2);
	s.syncAsByte(_raoulStandingFl);
	s.syncAsByte(_armsOutFl);

	s.syncAsSint16LE(_dayStatus);
	s.syncAsSint16LE(_raoulAction);
	s.syncAsSint16LE(_christineHotspotId1);
	s.syncAsSint16LE(_christineHotspotId2);
	s.syncAsSint16LE(_raoulCount);
	s.syncAsSint16LE(_dayCount);
	s.syncAsSint16LE(_standCount);
	s.syncAsSint16LE(_julieStatus);
	s.syncAsSint16LE(_florentStatus);
	s.syncAsSint16LE(_florentFrame);
	s.syncAsSint16LE(_florentCount);
	s.syncAsSint16LE(_dayFrame);
	s.syncAsSint16LE(_lastDayResetFrame);
	s.syncAsSint16LE(_raoulFrame);
	s.syncAsSint16LE(_julieFrame);
	s.syncAsSint16LE(_julieCount);
}

void Scene113::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(NOUN_CHRISTINE);
	_scene->addActiveVocab(NOUN_WOMAN);
	_scene->addActiveVocab(NOUN_JULIE);
}

void Scene113::enter() {
	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_standingAndTalking = false;
		_dayWantsToTalk = false;
		_musicPlaying = false;
		_afterKissFl = false;
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim2ActvFl = false;
		_anim3ActvFl = false;
		_anim4ActvFl = false;
		_prevent1 = false;
		_prevent2 = false;
		_raoulStandingFl = false;
		_armsOutFl = false;
	}

	_scene->_hotspots.activate(NOUN_SMALL_NOTE, false);
	_scene->_hotspots.activate(NOUN_DRESSING_GOWN, false);
	_scene->_hotspots.activate(NOUN_CHRISTINE, false);
	_scene->_hotspots.activate(NOUN_JULIE, false);
	_scene->_hotspots.activate(NOUN_LIGHT_FIXTURE, false);

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('f', 0));

		if (_game._objects.isInRoom(OBJ_SMALL_NOTE))
			_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 1));

		if ((_globals[kDoneBrieConv203] == 1) || (_globals[kDoneBrieConv203] == 3))
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 3));
	}

	if (_globals[kCurrentYear] == 1993) {
		_vm->_gameConv->load(4);
		if ((_globals[kDoneBrieConv203] == 1) || (_globals[kDoneBrieConv203] == 3))
			_vm->_gameConv->load(6);
	} else {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*faceral", false);
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*facecrsd", false);
		_vm->_gameConv->load(13);
	}

	if (_globals[kCurrentYear] == 1993) {
		_scene->drawToBackground(_globals._spriteIndexes[3], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_VASE, false);
		_scene->_hotspots.activate(NOUN_FAN, false);
		_scene->_hotspots.activateAtPos(NOUN_LIGHT_FIXTURE, true, Common::Point(155, 17));
	} else {
		_scene->_hotspots.activateAtPos(NOUN_LIGHT_FIXTURE, true, Common::Point(150, 46));
		_scene->_hotspots.activate(NOUN_NOTICE, false);
	}

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_globals[kCurrentYear] == 1881) {
			if (_dayStatus <= 3) {
				_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('r', 1), 1);
				_anim1ActvFl = true;
			}

			if (_raoulAction == 3)
				_scene->setAnimFrame(_globals._animationIndexes[1], 33);
			else {
				_raoulAction = 1;
				_game._player._visible = false;
				_scene->setAnimFrame(_globals._animationIndexes[1], 8);
			}

			if (_musicPlaying) {
				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 2), 0);
				_anim0ActvFl = true;
				_christineHotspotId1 = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots[_christineHotspotId1]._articleNumber = PREP_ON;
				_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 13);
				_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 14);
				_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 16);
				_scene->setAnimFrame(_globals._animationIndexes[0], 212);
				_game._player._visible = true;
				_anim1ActvFl = false;
			} else {
				switch (_dayStatus) {
				case 0:
				case 1:
				case 2:
				case 3:
					_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 0);
					_anim0ActvFl = true;
					_dayStatus = 2;
					_scene->setAnimFrame(_globals._animationIndexes[0], 208);
					_scene->_hotspots.activate(NOUN_CHRISTINE, true);
					_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, false, Common::Point(220, 130));
					break;

				default:
					_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 2), 0);
					_anim0ActvFl = true;
					_dayStatus = 7;
					_game._player._visible = false;
					_scene->setAnimFrame(_globals._animationIndexes[0], 165);
					break;
				}
				_christineHotspotId1 = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots[_christineHotspotId1]._articleNumber = PREP_ON;
				_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 13);
				_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 14);
				_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 16);
			}

			if (_dayStatus == 2) {
				_scene->_hotspots.activate(NOUN_CHRISTINE, true);
				_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, false, Common::Point(220, 130));
			} else {
				_game._player._playerPos = Common::Point(175, 148);
				_game._player._facing = FACING_NORTHEAST;
			}

		} else if ((_globals[kDoneBrieConv203] == 1) || (_globals[kDoneBrieConv203] == 3)) {
			_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('x', 1), 1);
			_julieStatus = 0;
			_anim4ActvFl = true;
			_globals[kMakeBrieLeave203] = true;
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
			_scene->_hotspots.activate(NOUN_DRESSING_GOWN, true);
			_scene->_hotspots.activate(NOUN_JULIE, true);
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(220, 130));
		} else if (_globals[kCurrentYear] == 1993) {
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('f', 1), 1);
			_anim2ActvFl = true;

			if (_florentStatus != 3)
				_florentStatus = 1;

			_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('r', 1), 1);
			_anim3ActvFl = true;

			if (_raoulAction == 3)
				_scene->setAnimFrame(_globals._animationIndexes[3], 33);
			else {
				_scene->setAnimFrame(_globals._animationIndexes[3], 8);
				_raoulAction = 1;
				_game._player._playerPos = Common::Point(201, 120);
				_game._player._facing = FACING_SOUTH;
				_game._player._visible = false;
			}

			if (_florentStatus == 3)
				_scene->setAnimFrame(_globals._animationIndexes[2], 41);

			if (_globals[kFlorentNameIsKnown] >= 1) {
				_christineHotspotId2 = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(43, 118, 43 + 15, 118 + 29));
				_scene->_dynamicHotspots[_christineHotspotId2]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(_christineHotspotId2, Common::Point(106, 127), FACING_SOUTHWEST);
			} else {
				_christineHotspotId2 = _scene->_dynamicHotspots.add(NOUN_WOMAN, VERB_WALK_TO, SYNTAX_FEM_NOT_PROPER, EXT_NONE, Common::Rect(43, 118, 43 + 15, 118 + 29));
				_scene->_dynamicHotspots[_christineHotspotId2]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(_christineHotspotId2, Common::Point(106, 127), FACING_SOUTHWEST);
			}

			_scene->_hotspots.activate(NOUN_DRESSING_GOWN, true);

			if (_game._objects.isInRoom(OBJ_SMALL_NOTE)) {
				_scene->_hotspots.activate(NOUN_SMALL_NOTE, true);
				_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 13);
			}
		}
	} else if (_globals[kCurrentYear] == 1993) {
		if ((_globals[kDoneBrieConv203] == 1) || (_globals[kDoneBrieConv203] == 3)) {
			if (!(_globals[kPlayerScoreFlags] & 4)) {
				_globals[kPlayerScoreFlags] = _globals[kPlayerScoreFlags] | 4;
				_globals[kPlayerScore] += 5;
			}

			_globals._animationIndexes[4] = _scene->loadAnimation(formAnimName('x', 1), 1);
			_julieStatus = 0;
			_anim4ActvFl = true;
			_globals[kMakeBrieLeave203] = true;
			_game._player._playerPos = Common::Point(190, 148);
			_game._player._facing = FACING_NORTH;
			_game._player._stepEnabled = false;
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
			_scene->_hotspots.activate(NOUN_DRESSING_GOWN, true);
			_scene->_hotspots.activate(NOUN_JULIE, true);
			_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, true, Common::Point(220, 130));
			_game._player.walk(Common::Point(175, 128), FACING_NORTHEAST);
			_game._player.setWalkTrigger(102);
		} else {
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

			_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('f', 1), 1);
			_florentStatus = 1;
			_anim2ActvFl = true;

			_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('r', 1), 1);
			_raoulAction = 3;
			_anim3ActvFl = true;
			_scene->setAnimFrame(_globals._animationIndexes[3], 33);

			if (_globals[kFlorentNameIsKnown] >= 1) {
				_christineHotspotId2 = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(43, 118, 43 + 15, 118 + 29));
				_scene->_dynamicHotspots[_christineHotspotId2]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(_christineHotspotId2, Common::Point(106, 127), FACING_SOUTHWEST);
			} else {
				_christineHotspotId2 = _scene->_dynamicHotspots.add(NOUN_WOMAN, VERB_WALK_TO, SYNTAX_FEM_NOT_PROPER, EXT_NONE, Common::Rect(43, 118, 43 + 15, 118 + 29));
				_scene->_dynamicHotspots[_christineHotspotId2]._articleNumber = PREP_ON;
				_scene->_dynamicHotspots.setPosition(_christineHotspotId2, Common::Point(106, 127), FACING_SOUTHWEST);
			}

			_game._player._playerPos = Common::Point(190, 148);
			_game._player._facing = FACING_NORTH;

			_scene->_hotspots.activate(NOUN_DRESSING_GOWN, true);
			if (_game._objects.isInRoom(OBJ_SMALL_NOTE)) {
				_scene->_hotspots.activate(NOUN_SMALL_NOTE, true);
				_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 13);
			}
		}
	} else if ((_globals[kCurrentYear] == 1881) && (! _globals[kChrisKickedRaoulOut])) {
		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('r', 1), 1);
		_raoulAction = 3;
		_anim1ActvFl = true;
		_scene->setAnimFrame(_globals._animationIndexes[1], 33);
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 1), 1);
		_dayStatus = 2;
		_anim0ActvFl = true;

		_scene->_hotspots.activate(NOUN_CHRISTINE, true);
		_scene->_hotspots.activateAtPos(NOUN_CHRISTINE, false, Common::Point(220, 130));
		_scene->_sequences.addTimer(60, 60);
		_game._player._playerPos = Common::Point(190, 148);
		_game._player._facing = FACING_NORTH;
	} else if ((_globals[kCurrentYear] == 1881) && _globals[kChrisKickedRaoulOut]) {
		_globals[kChrisKickedRaoulOut] = 2;
		_game._player._playerPos = Common::Point(190, 148);
		_game._player._facing = FACING_NORTH;
		_globals[kCameFromFade] = true;
		_game._player._stepEnabled = false;
		_globals[kPlayerScore] += 8;
		_scene->_sequences.addTimer(60, 110);
	} else if (_scene->_priorSceneId == 111) {
		_game._player._playerPos = Common::Point(190, 148);
		_game._player._facing = FACING_NORTH;
	}

	switch (_vm->_gameConv->restoreRunning()) {
	case 4:
		_vm->_gameConv->run(4);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		break;

	case 6:
		_vm->_gameConv->run(6);
		break;

	case 13:
		_vm->_gameConv->run(13);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		break;

	default:
		break;
	}

	sceneEntrySound();
}

void Scene113::step() {
	if (_anim1ActvFl)
		handleRaoulAnimation2();

	if (_anim4ActvFl)
		handleJulieAnimation();

	if (_anim2ActvFl)
		handleFlorentAnimation();

	if (_anim3ActvFl) {
		handleRaoulAnimation();

		if ((!_game._objects.isInInventory(OBJ_SMALL_NOTE)) && (_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == 26)) {
			_scene->_hotspots.activate(NOUN_SMALL_NOTE, false);
			_game._objects.addToInventory(OBJ_SMALL_NOTE);
			_scene->deleteSequence(_globals._sequenceIndexes[1]);
			_vm->_sound->command(26);
			_vm->_dialogs->showItem(OBJ_SMALL_NOTE, 806, 2);
		}
	}

	if ((_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 86) && (!_prevent2)) {
		_scene->freeAnimation(_globals._animationIndexes[0]);
		_prevent2 = true;
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('d', 2), 0);
		_scene->setAnimFrame (_globals._animationIndexes[0], 86);
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_CLOCK, 0);
		_scene->animations_tick();
	}

	if (_anim0ActvFl) {
		handleDayAnimation();

		if ((_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 95) && (!_armsOutFl)) {
			_standingAndTalking = true;
			_dayStatus = 7;
			_raoulAction = 3;
			_armsOutFl = true;
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_ANIM, _globals._animationIndexes[1]);
		}

		if ((_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 165) && !_afterKissFl)
			_afterKissFl = true;
	}

	switch (_game._trigger) {
	case 60:
		if (!_game._visitedScenes._sceneRevisited)
			_vm->_dialogs->show(11342);
		break;

	case 102:
		_game._player._stepEnabled = true;
		_vm->_gameConv->run(6);
		break;

	case 108:
		_vm->_dialogs->show(11332);
		_vm->_dialogs->show(11333);
		_scene->_nextSceneId = 150;
		_globals[kLeaveAngelMusicOn] = false;
		break;

	case 110:
		_vm->_dialogs->show(11331);
		_game._player.walk(Common::Point(272, 138), FACING_EAST);
		_game._player.setWalkTrigger(108);
		break;

	default:
		break;
	}
}

void Scene113::actions() {
	if (_vm->_gameConv->activeConvId() == 13) {
		handleLoveConversation();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 4) {
		handleFlorentConversation();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 6) {
		handleDeadConversation();
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_TALK_TO, NOUN_WOMAN)) || (_action.isAction(VERB_TALK_TO, NOUN_CHRISTINE))) {
		if (_globals[kCurrentYear] == 1881) {
			_game._player._stepEnabled = true;
			_vm->_gameConv->run(13);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		} else if (_globals[kDoneBrieConv203]) {
			_vm->_dialogs->show(11348);
		} else {
			_game._player._stepEnabled = true;
			_vm->_gameConv->run(4);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_CORRIDOR)) {
		if (_globals[kDoneBrieConv203] == 1)
			_globals[kDoneBrieConv203] = 3;

		_scene->_nextSceneId = 111;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if (_globals[kCurrentYear] == 1993) {
			if (_globals[kDoneBrieConv203])
				_vm->_dialogs->show(11340);
			else
				_vm->_dialogs->show(11310);
		} else {
			_vm->_dialogs->show(11311);
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(11312);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RUG)) {
			_vm->_dialogs->show(11313);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(11314);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COUCH)) {
			_vm->_dialogs->show(11315);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MURAL)) {
			_vm->_dialogs->show(11316);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PAINTING)) {
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->show(11317);
			else
				_vm->_dialogs->show(11343);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PAINTING)) {
			_vm->_dialogs->show(11317);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DRESSING_TABLE)) {
			_vm->_dialogs->show(11318);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHAIR)) {
			_vm->_dialogs->show(11319);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MIRROR)) {
			if (_globals[kCurrentYear] == 1993) {
				_vm->_dialogs->show(11344);
			} else {
				_vm->_dialogs->show(11320);
			}
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FAN)) {
			_vm->_dialogs->show(11321);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_VASE)) {
			_vm->_dialogs->show(11322);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HAT_RACK)) {
			_vm->_dialogs->show(11323);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT_FIXTURE)) {
			_vm->_dialogs->show(11324);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(11325);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WARDROBE)) {
			_vm->_dialogs->show(11326);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DRESSING_SCREEN)) {
			_vm->_dialogs->show(11327);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CORRIDOR)) {
			_vm->_dialogs->show(11328);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DRESSING_GOWN)) {
			_vm->_dialogs->show(11330);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SMALL_NOTE) && _game._objects.isInRoom(OBJ_SMALL_NOTE)) {
			_vm->_dialogs->show(11349);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHRISTINE) && _globals[kDoneBrieConv203]) {
			_vm->_dialogs->show(11338);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_JULIE)) {
			_vm->_dialogs->show(11339);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_NOTICE)) {
			_vm->_dialogs->show(11347);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CLOTHES_DUMMY)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11345);
			else
				_vm->_dialogs->show(11346);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WOMAN) || _action.isObject(NOUN_CHRISTINE)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11341);
			else {
				if (_musicPlaying)
					_vm->_dialogs->show(11336);
				else
					_vm->_dialogs->show(11342);
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_WARDROBE)) {
		_vm->_dialogs->show(11329);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SMALL_NOTE) && _game._objects.isInRoom(OBJ_SMALL_NOTE)) {
		_vm->_dialogs->show(11334);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_DRESSING_GOWN)) {
		_vm->_dialogs->show(11335);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CHRISTINE) || _action.isAction(VERB_TAKE, NOUN_WOMAN)) {
		_vm->_dialogs->show(11337);
		_action._inProgress = false;
		return;
	}
}

void Scene113::preActions() {
	if (_action.isAction(VERB_WALK_UP, NOUN_AISLE))
		_game._player._walkOffScreenSceneId = 101;

	if (_action.isAction(VERB_LOOK, NOUN_MIRROR))
		_game._player.walk(Common::Point(272, 138), FACING_EAST);

	if (! _action.isAction(VERB_EXIT_TO, NOUN_CORRIDOR) && !_globals[kChrisKickedRaoulOut]
	 && !_action.isAction(VERB_LOOK, NOUN_CHRISTINE) && !_action.isAction(VERB_TAKE, NOUN_CHRISTINE) && _musicPlaying) {
		_vm->_gameConv->run(13);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_game._player.cancelCommand();
	} else if (_action.isAction(VERB_TALK_TO, NOUN_WOMAN) || _action.isAction(VERB_TALK_TO, NOUN_CHRISTINE)) {
		_game._player.walk(Common::Point(106, 127), FACING_SOUTHWEST);
	}

	if (!_action.isAction(VERB_EXIT_TO, NOUN_CORRIDOR) && !_action.isAction(VERB_LOOK, NOUN_JULIE)
	 && !_action.isAction(VERB_LOOK, NOUN_CHRISTINE) && !_action._lookFlag && (_globals[kDoneBrieConv203] != 0)
	 && (_globals[kCurrentYear] == 1993)) {
		if (_action.isAction(VERB_TALK_TO, NOUN_CHRISTINE)) {
			_game._player._needToWalk = false;

		} else if (_action.isAction(VERB_LOOK)) {
			_game._player._needToWalk = false;

		} else if (_action.isAction(VERB_TALK_TO, NOUN_JULIE)) {
			_game._player._needToWalk = false;
			_vm->_gameConv->run(6);
			_game._player.cancelCommand();

		} else {
			_vm->_dialogs->show(11350);
			_game._player._needToWalk = false;
			_game._player.cancelCommand();
		}
	}
}

void Scene113::handleFlorentAnimation() {
	if (_scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame() == _florentFrame)
		return;

	int random;
	_florentFrame = _scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_florentFrame) {
	case 1:
	case 11:
	case 23:
	case 38:
	case 45:
		random = _vm->getRandomNumber(4, 30);

		if (_florentStatus == 2) {
			random = 1;
		}

		if (_florentStatus == 0) {
			random = 2;
		}

		if (_florentStatus == 3) {
			random = 3;
		}

		switch (random) {
		case 1:
			resetFrame = 2;
			_florentStatus = 1;
			break;

		case 2:
			resetFrame = 28;
			break;

		case 3:
			resetFrame = 39;
			break;

		case 4:
			resetFrame = 12;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 24:
	case 25:
	case 26:
	case 27:
	case 33:
		random = _vm->getRandomNumber(5, 20);

		if (_florentStatus == 0) {
			random = _vm->getRandomNumber(1, 4);
			++_florentCount;
			if (_florentCount > 5) {
				_florentStatus = 1;
				random = 5;
			}
		}

		if ((_florentStatus == 2) ||
			(_florentStatus == 3)) {
				random = 5;
		}

		switch (random) {
		case 1:
			resetFrame = 23;
			break;

		case 2:
			resetFrame = 24;
			break;

		case 3:
			resetFrame = 25;
			break;

		case 4:
			resetFrame = 26;
			break;

		case 5:
			resetFrame = 34;
			break;

		default:
			resetFrame = 32;
			break;
		}
		break;

	case 42:
		if (_florentStatus == 3)
			resetFrame = 41;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[2], resetFrame);
		_florentFrame = resetFrame;
	}
}

void Scene113::handleDayAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _dayFrame)
		return;

	_dayFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int random;
	int resetFrame = -1;
	switch (_dayFrame) {
	case 1:
	case 4:
	case 14:
	case 17:
	case 20:
	case 26:
	case 166:
	case 206:
	case 207:
	case 208:
	case 209:
	case 210:
	case 211:
	case 219:
	case 231:
	case 253:
		switch (_dayStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 3);
			++_dayCount;
			if (_dayCount < 6) {
				if (random == 1) {
					resetFrame = 1;
				} else if (random == 2) {
					resetFrame = 15;
				} else {
					resetFrame = 18;
				}
			} else {
				_dayStatus = 1;
				resetFrame = 25;
				_dayWantsToTalk = false;
			}
			break;

		case 1:
			if (_dayWantsToTalk) {
				_dayStatus = 0;
				_dayWantsToTalk = false;
				resetFrame = 1;
			} else {
				resetFrame = 25;
			}
			break;

		case 2:
			random = _vm->getRandomNumber(1, 50);
			switch (random) {
			case 1:
				resetFrame = 212;
				break;

			case 2:
				resetFrame = 219;
				break;

			case 3:
				resetFrame = 231;
				break;

			default:
				resetFrame = 0;
				break;
			}
			break;

		case 3:
			resetFrame = 5;
			_dayStatus = 0;
			break;

		case 4:
			resetFrame = 31;
			break;

		case 5:
			resetFrame = _vm->getRandomNumber(205, 207);
			++_standCount;
			if (_standCount > 18) {
				_dayStatus = 7;
				resetFrame = 165;
			}
			if (_musicPlaying)
				resetFrame = 167;
			break;

		case 6:
			resetFrame = _vm->getRandomNumber(208, 210);
			while (_lastDayResetFrame == resetFrame)
				resetFrame = _vm->getRandomNumber(208, 210);

			_lastDayResetFrame = resetFrame;

			++_standCount;
			if (_standCount > 18) {
				_dayStatus = 7;
				resetFrame = 165;
			}

			if (_musicPlaying)
				resetFrame = 167;
			break;

		case 7:
			resetFrame = 165;
			if (_musicPlaying)
				resetFrame = 167;
			break;

		default:
			break;
		}
		break;

	case 182:
		_game._player._visible = true;
		_vm->_gameConv->release();
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		_game._player._playerPos = Common::Point(175, 148);
		_game._player._turnToFacing = FACING_NORTHEAST;
		_game._player.resetFacing(FACING_SOUTHEAST);
		_game._player._turnToFacing = FACING_NORTHEAST;
		break;

	case 205:
	case 212:
		resetFrame = 211;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_dayFrame = resetFrame;
	}
}

void Scene113::handleRaoulAnimation() {
	if (_globals[kFlorentNameIsKnown] == 2)
		_raoulAction = 3;

	if (_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == _raoulFrame)
		return;

	_raoulFrame = _scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame();
	int resetFrame = -1;

	switch (_raoulFrame) {
	case 9:
	case 10:
	case 11:
	case 34:
	case 33:
		if (_raoulFrame == 33)
			_vm->_gameConv->release();

		switch (_raoulAction) {
		case 0:
			resetFrame = _vm->getRandomNumber(7, 9);
			++_raoulCount;

			if (_raoulCount > 17) {
				_raoulAction = 1;
				resetFrame = 8;
			}
			break;

		case 1:
			resetFrame = 8;
			break;

		case 2:
			resetFrame = 11;
			break;

		case 3:
			resetFrame = 33;
			break;

		case 4:
			resetFrame = 38;
			_raoulStandingFl = true;
			_vm->_gameConv->hold();
			break;

		case 5:
			resetFrame = 20;
			_raoulAction = 1;
			break;

		default:
			break;
		}
		break;

	case 19:
		if ((_raoulAction == 5) && (!_game._objects.isInInventory(OBJ_SMALL_NOTE))) {
			resetFrame = 20;
			_raoulAction = 1;
		} else if (_raoulAction == 4) {
			resetFrame = 38;
			_raoulStandingFl = true;
		} else if (_raoulAction == 0) {
			resetFrame = 9;
		} else {
			resetFrame = 8;
			_raoulAction = 1;
		}
		break;

	case 42:
		if (_raoulAction == 4) {
			if (!_prevent2) {
				_vm->_gameConv->release();
			}
			resetFrame = 33;
			_raoulAction = 3;
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[3]);
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[3], resetFrame);
		_raoulFrame = resetFrame;
	}
}

void Scene113::handleRaoulAnimation2() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _raoulFrame)
		return;

	_raoulFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;
	switch (_raoulFrame) {
	case 9:
	case 10:
	case 11:
	case 34:
		switch (_raoulAction) {
		case 0:
			resetFrame = _vm->getRandomNumber(7, 9);
			++_raoulCount;

			if (_raoulCount > 17) {
				_raoulAction = 1;
				resetFrame = 8;
			}
			break;

		case 1:
			resetFrame = 8;
			break;

		case 2:
			resetFrame = 11;
			break;

		case 3:
			resetFrame = 33;
			break;

		default:
			break;
		}
		break;

	case 19:
		if (_raoulAction == 0)
			resetFrame = 9;
		else {
			resetFrame = 8;
			_raoulAction = 1;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_raoulFrame = resetFrame;
	}
}

void Scene113::handleJulieAnimation() {
	if (_scene->_animation[_globals._animationIndexes[4]]->getCurrentFrame() == _julieFrame)
		return;

	_julieFrame = _scene->_animation[_globals._animationIndexes[4]]->getCurrentFrame();
	int resetFrame = -1;
	int random;

	switch (_julieFrame) {
	case 1:
	case 2:
	case 3:
	case 11:
	case 16:
	case 17:
		switch (_julieStatus) {
		case 0:
			random = _vm->getRandomNumber(4, 20);
			break;

		case 1:
			random = _vm->getRandomNumber(1, 3);
			++_julieCount;
			if (_julieCount > 20) {
				_julieStatus = 0;
				random = 6;
			}
			break;

		default:
			random = -1;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 0;
			break;

		case 2:
			resetFrame = 1;
			break;

		case 3:
			resetFrame = 2;
			break;

		case 4:
			resetFrame = 12;
			break;

		case 5:
			resetFrame = 4;
			break;

		default:
			resetFrame = 16;
			break;
		}
		break;
	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[4], resetFrame);
		_julieFrame = resetFrame;
	}
}

void Scene113::handleDeadConversation() {
	switch (_game._trigger) {
	case 106:
		_julieStatus = 1;
		break;

	case 104:
		_julieStatus = 0;
		break;

	default:
		break;
	}

	_vm->_gameConv->setInterlocutorTrigger(106);
	_vm->_gameConv->setHeroTrigger(104);

	_julieCount = 0;
}

void Scene113::handleFlorentConversation() {
	switch (_action._activeAction._verbId) {
	case 3:
	case 4:
		if (!_prevent1) {
			_globals[kFlorentNameIsKnown] = 1;
			_vm->_gameConv->setInterlocutorTrigger(82);
			_scene->_dynamicHotspots.remove(_christineHotspotId2);
			_christineHotspotId2 = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(43, 118, 43 + 15, 118 + 29));
			_scene->_dynamicHotspots[_christineHotspotId2]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(_christineHotspotId2, Common::Point(106, 127), FACING_SOUTHWEST);
			_prevent1 = true;
		}
		break;

	case 19:
		_florentStatus = 2;
		_vm->_gameConv->setInterlocutorTrigger(86);
		break;

	case 25:
		if (_raoulAction != 3)
			_raoulAction = 4;
		break;

	case 26:
		if (!_prevent2) {
			_scene->_sequences.addTimer(120, 92);
			_vm->_gameConv->setInterlocutorTrigger(96);
			_florentStatus = 0;
			_prevent2 = true;
		}
		break;

	case 27:
	case 30:
		_vm->_gameConv->setInterlocutorTrigger(98);
		break;

	case 28:
	case 29:
		_vm->_gameConv->setInterlocutorTrigger(100);
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 62:
		if ((_action._activeAction._verbId != 3) && (_action._activeAction._verbId != 4) && (_action._activeAction._verbId != 19) &&
			(_action._activeAction._verbId != 27) && (_action._activeAction._verbId != 28) && (_action._activeAction._verbId != 29)) {
				if ((_raoulAction != 5) && (_raoulAction != 4) && !_raoulStandingFl)
					_raoulAction = 0;

				if ((_florentStatus != 3) && (_florentStatus != 2))
					_florentStatus = 1;
		}
		break;

	case 66:
		if ((_florentStatus != 3) &&
			(_florentStatus != 2)) {
				if (_vm->getRandomNumber(1, 5) == 1)
					_florentStatus = 0;
		}

		if ((_raoulAction != 3) && (_raoulAction != 5) && (_raoulAction != 4) && (!_raoulStandingFl)) {
			_raoulAction = 1;
			if (_vm->getRandomNumber(1, 2) == 1)
				_raoulAction = 2;
		}
		break;

	case 80:
		_vm->_gameConv->release();
		_game._player._visible = false;
		_raoulAction = 1;
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[3], SYNC_PLAYER, 0);
		_scene->setAnimFrame(_globals._animationIndexes[3], 1);
		break;

	case 82:
		_florentStatus = 0;
		_scene->_sequences.addTimer(120, 84);
		_vm->_gameConv->hold();
		break;

	case 84:
		_game._player.walk(Common::Point(201, 120), FACING_SOUTH);
		_game._player.setWalkTrigger(80);
		break;

	case 86:
		_raoulAction = 5;
		_vm->_gameConv->hold();
		break;

	case 92:
		_game._player.walk(Common::Point(68, 142), FACING_SOUTHWEST);
		_game._player.setWalkTrigger(94);
		break;

	case 94:
		_globals[kStopWalkerDisabled] = true;
		_globals[kFlorentNameIsKnown] = 2;
		_vm->_gameConv->release();
		break;

	case 96:
		_vm->_gameConv->hold();
		break;

	case 98:
		_florentStatus = 3;
		break;

	case 100:
		_florentStatus = 1;
		_globals[kStopWalkerDisabled] = false;
		break;

	default:
		break;
	}

	if ((_action._activeAction._verbId != 3) && (_action._activeAction._verbId != 4) && (_action._activeAction._verbId != 19) &&
		(_action._activeAction._verbId != 28) && (_action._activeAction._verbId != 29) && (_action._activeAction._verbId != 27) &&
		(_action._activeAction._verbId != 30) && (_action._activeAction._verbId != 26))
		_vm->_gameConv->setInterlocutorTrigger(66);

	_vm->_gameConv->setHeroTrigger(62);
	_raoulCount = 0;
	_florentCount = 0;
}

void Scene113::handleLoveConversation() {
	switch (_action._activeAction._verbId) {
	case 1:
	case 2:
		if (!_prevent1) {
			_vm->_gameConv->setInterlocutorTrigger(82);
			_prevent1 = true;
		}
		break;

	case 21:
		if (!_armsOutFl) {
			_dayStatus = 4;
			_scene->_sequences.addTimer(1, 70);
			_scene->_userInterface.emptyConversationList();
			_scene->_userInterface.setup(kInputConversation);
			_scene->_hotspots.activate(NOUN_CHRISTINE, false);
			_vm->_gameConv->hold();
		}
		break;

	case 27:
		_vm->_sound->command(34);
		_globals[kLeaveAngelMusicOn] = true;
		break;

	case 31:
		if (!_musicPlaying) {
			_vm->_gameConv->hold();
			_dayStatus = 7;
			_musicPlaying = true;
			_christineHotspotId1 = _scene->_dynamicHotspots.add(NOUN_CHRISTINE, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[_christineHotspotId1]._articleNumber = PREP_ON;
			_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 14);
			_scene->setDynamicAnim(_christineHotspotId1, _globals._animationIndexes[0], 16);
		}
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 62:
		if (_action._activeAction._verbId != 0) {
			if (_standingAndTalking) {
				_dayStatus = 6;
			} else if (_dayStatus != 4) {
				_dayStatus = 1;
				_raoulAction = 0;
				_dayWantsToTalk = false;
			}
		}
		break;

	case 66:
		if (_standingAndTalking) {
			_dayStatus = 5;

		} else {
			if (_action._activeAction._verbId == 19) {
				_dayStatus = 3;
			} else {
				_dayStatus = 1;
				_dayWantsToTalk = true;
			}

			if (_raoulAction != 3) {
				_raoulAction = 1;
				if (_vm->getRandomNumber(1, 2) == 1)
					_raoulAction = 2;
			}
		}
		break;

	case 70:
		if (_armsOutFl) {
			_vm->_gameConv->release();
			_vm->_gameConv->setInterlocutorTrigger(76);
		}
		break;

	case 72:
		if (_afterKissFl)
			_vm->_gameConv->release();
		else
			_scene->_sequences.addTimer(1, 72);
		break;

	case 76:
		_vm->_gameConv->hold();
		_scene->_sequences.addTimer(1, 72);
		break;

	case 78:
		if (_armsOutFl) {
			_vm->_gameConv->release();
			_vm->_gameConv->setInterlocutorTrigger(76);
		}
		break;

	case 80:
		_vm->_gameConv->release();
		_game._player._visible = false;
		_raoulAction = 1;
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[1], SYNC_PLAYER, 0);
		_scene->setAnimFrame(_globals._animationIndexes[1], 1);
		break;

	case 82:
		_scene->_sequences.addTimer(120, 84);
		_vm->_gameConv->hold();
		_dayStatus = 1;
		_dayWantsToTalk = true;
		break;

	case 84:
		_game._player.walk(Common::Point(201, 120), FACING_SOUTH);
		_game._player.setWalkTrigger(80);
		break;

	default:
		break;
	}

	if ((_action._activeAction._verbId != 21) && (_action._activeAction._verbId != 1) && (_action._activeAction._verbId != 2))
		_vm->_gameConv->setInterlocutorTrigger(66);

	_vm->_gameConv->setHeroTrigger(62);

	_raoulCount = 0;
	_dayCount = 0;
	_standCount = 0;
}

/*------------------------------------------------------------------------*/

Scene114::Scene114(MADSEngine *vm) : Scene1xx(vm) {
}

void Scene114::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
}

void Scene114::setup() {
	if (_globals[kCurrentYear] == 1993)
		_scene->_variant = 1;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene114::enter() {
	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
	} else if (_game._objects.isInRoom(OBJ_ROPE) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 0));
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RDR_9");
	}

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 0));

	if ((_game._objects.isInRoom(OBJ_ROPE)) && (_globals[kCurrentYear] == 1881) && !_game._objects.isInInventory(OBJ_ROPE_WITH_HOOK)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
	} else
		_scene->_hotspots.activate(NOUN_ROPE, false);

	if (_globals[kCurrentYear] == 1993) {
		_scene->_hotspots.activate(NOUN_CYLINDER, false);
		_scene->_hotspots.activate(NOUN_MANNEQUINS, false);
		_scene->_hotspots.activate(NOUN_PROP, false);
		_scene->_hotspots.activate(NOUN_BUST, false);
		_scene->_hotspots.activate(NOUN_SCAFFOLDING, false);
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
	} else {
		_scene->_hotspots.activate(NOUN_BOXES, false);
		_scene->_hotspots.activate(NOUN_DINETTE_SET, false);
		_scene->_hotspots.activate(NOUN_CRATE, false);
		_scene->_hotspots.activate(NOUN_CASES, false);
	}

	if ((_scene->_priorSceneId == 105) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(218, 123);
		_game._player._facing = FACING_WEST;
	}

	sceneEntrySound();
}

void Scene114::step() {
}

void Scene114::actions() {
	if (_action.isAction(VERB_CLIMB_UP, NOUN_CIRCULAR_STAIRCASE)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('u', 1), 1);
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[0], SYNC_PLAYER, 0);
			break;

		case 1:
			_scene->_nextSceneId = 105;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLIMB_UP, NOUN_CIRCULAR_STAIRCASE)) {
		_scene->_nextSceneId = 105;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_ROPE) && (_game._objects.isInRoom(OBJ_ROPE) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 6, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 3);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 3, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[2]);
			_scene->_hotspots.activate(NOUN_ROPE, false);
			_game._objects.addToInventory(OBJ_ROPE);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[1]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_vm->_dialogs->showItem(OBJ_ROPE, 807, 0);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(11410);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(11411);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CIRCULAR_STAIRCASE)) {
			_vm->_dialogs->show(11412);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ROPE) && _game._objects.isInRoom(OBJ_ROPE)) {
			_vm->_dialogs->show(11413);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(11414);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MANNEQUINS)) {
			_vm->_dialogs->show(11415);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLAT)) {
			_vm->_dialogs->show(11416);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SCAFFOLDING)) {
			_vm->_dialogs->show(11417);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MUMMY_PROP)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11418);
			else
				_vm->_dialogs->show(11433);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CRATES)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11419);
			else
				_vm->_dialogs->show(11434);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CRATE)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11420);
			else
				_vm->_dialogs->show(11435);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CARTONS)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11421);
			else
				_vm->_dialogs->show(11436);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CARTON)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11422);
			else
				_vm->_dialogs->show(11437);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PROP)) {
			_vm->_dialogs->show(11423);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BUST)) {
			_vm->_dialogs->show(11424);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MUG)) {
			_vm->_dialogs->show(11425);
			_action._inProgress = false;
			return;
		}

		if ((_action.isObject(NOUN_CASES)) || (_action.isObject(NOUN_CASE))) {
			_vm->_dialogs->show(11426);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOXES) || _action.isObject(NOUN_BOX)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(11427);
			else
				_vm->_dialogs->show(11439);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOX)) {
			if (_globals[kCurrentYear] == 1881) {
				_vm->_dialogs->show(11439);
				_action._inProgress = false;
				return;
			}
		}

		if (_action.isObject(NOUN_DINETTE_SET)) {
			_vm->_dialogs->show(11428);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CYLINDER)) {
			_vm->_dialogs->show(11429);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_OPEN, NOUN_CARTONS)) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(11421);
		else
			_vm->_dialogs->show(11436);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_CARTON)) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(11422);
		else
			_vm->_dialogs->show(11437);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_CRATES)) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(11419);
		else
			_vm->_dialogs->show(11434);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_CRATE)) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(11419);
		else
			_vm->_dialogs->show(11435);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_CASES)) {
		_vm->_dialogs->show(11426);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_BOXES) || _action.isAction(VERB_OPEN, NOUN_BOX)) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(11427);
		else
			_vm->_dialogs->show(11439);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_MUG)) {
		_vm->_dialogs->show(11430);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_MUMMY_PROP)) {
		_vm->_dialogs->show(11431);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_MUMMY_PROP)) {
		_vm->_dialogs->show(11432);
		_action._inProgress = false;
		return;
	}
}

void Scene114::preActions() {
}

/*------------------------------------------------------------------------*/

Scene150::Scene150(MADSEngine *vm) : Scene1xx(vm) {
}

void Scene150::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);
}

void Scene150::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene150::enter() {
	warning("TODO: Switch to letter box view. See definition of MADS_MENU_Y");

	_game._player._stepEnabled = false;
	_game._player._visible = false;

	if (_scene->_priorSceneId == 113)
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('l', 1), 1);
	else if (_scene->_priorSceneId == 203) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('f', 1), 2);
		sceneEntrySound();
	} else if (_scene->_priorSceneId == 306)
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('e', 1), 4);
	else if (_scene->_priorSceneId == 208)
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('h', 1), 3);
	else
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('q', 1), 5);
}

void Scene150::step() {
	if (_game._trigger == 1)
		_scene->_nextSceneId = 203;

	if (_game._trigger == 2)
		_scene->_nextSceneId = 111;

	if (_game._trigger == 4)
		_scene->_nextSceneId = 204;

	if (_game._trigger == 3) {
		_globals[kJacquesStatus] = 1;
		_scene->_nextSceneId = 205;
	}

	if (_game._trigger == 5)
		_game._winStatus = 1;

	if (_scene->_nextSceneId != _scene->_currentSceneId) {
		byte pal[768];
		_vm->_palette->getFullPalette(pal);
		Common::fill(&pal[12], &pal[756], 0);
		_vm->_palette->setFullPalette(pal);
	}
}

void Scene150::actions() {
}

void Scene150::preActions() {
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
