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
#include "mads/phantom/phantom_scenes2.h"

namespace MADS {

namespace Phantom {

void Scene2xx::setAAName() {
	_game._aaName = Resources::formatAAName(_globals[kTempInterface]);
	_vm->_palette->setEntry(254, 43, 47, 51);
}

void Scene2xx::sceneEntrySound() {
	if (!_vm->_musicFlag)
		return;

	switch (_scene->_nextSceneId) {
	case 206:
		if (!_globals[kKnockedOverHead])
			_vm->_sound->command(16);
		break;

	case 208:
		_vm->_sound->command(34);
		break;

	default:
		if (_scene->_nextSceneId != 250)
			_vm->_sound->command(16);
		break;
	}
}

void Scene2xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	if (_scene->_nextSceneId == 208)
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

Scene201::Scene201(MADSEngine *vm) : Scene2xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_needHoldFl = false;
	_sellerCount = -1;
	_sellerStatus = -1;
	_sellerFrame = -1;
	_raoulFrame = -1;
	_raoulStatus = -1;
}

void Scene201::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_needHoldFl);
	s.syncAsSint16LE(_sellerCount);
	s.syncAsSint16LE(_sellerStatus);
	s.syncAsSint16LE(_sellerFrame);
	s.syncAsSint16LE(_raoulFrame);
	s.syncAsSint16LE(_raoulStatus);
}

void Scene201::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene201::enter() {
	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_anim1ActvFl = false;
	}

	_sellerCount = 0;
	_needHoldFl = false;

	_vm->_gameConv->get(16);
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0), false);
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 0), false);

	if (_globals[kTicketPeoplePresent]) {
		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('s', 1), 0);
		_anim1ActvFl = true;
		_sellerStatus = 2;
		_scene->setAnimFrame(_globals._animationIndexes[1], 5);
	} else {
		_scene->_hotspots.activate(NOUN_TICKET_SELLER, false);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	}

	if (_vm->_gameConv->_restoreRunning == 16) {
		_game._player._playerPos = Common::Point(72, 101);
		_game._player._facing = FACING_NORTHWEST;
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('r', 1), 0);
		_anim0ActvFl = true;
		_raoulStatus = 1;
		_game._player._visible = false;

		_vm->_gameConv->run(16);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_vm->_gameConv->exportPointer(&_globals[kChristineToldEnvelope]);
	}

	if ((_scene->_priorSceneId == 202) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(314, 86);
		_game._player.walk(Common::Point(266, 98), FACING_SOUTHWEST);
	}

	sceneEntrySound();
}

void Scene201::step() {
	if (_anim0ActvFl)
		handleRaoulAnimation();

	if (_anim1ActvFl)
		handleSellerAnimation();

	if ((_needHoldFl) && (_vm->_gameConv->_running != 16)) {
		_game._player._stepEnabled = false;
		_needHoldFl = false;
	}
}

void Scene201::actions() {
	if (_vm->_gameConv->_running == 16) {
		handleConversation();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ARCHWAY)) {
		_scene->_nextSceneId = 202;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_TICKET_SELLER)) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('r', 1), 0);
		_anim0ActvFl = true;
		_raoulStatus = 1;
		_vm->_gameConv->run(16);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_vm->_gameConv->exportPointer(&_globals[kChristineToldEnvelope]);
		_game._player._visible = false;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if (_globals[kCurrentYear] == 1993)
			_vm->_dialogs->show(20110);
		else
			_vm->_dialogs->show(20111);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(20112);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(20113);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BUST)) {
			_vm->_dialogs->show(20114);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PEDESTAL)) {
			_vm->_dialogs->show(20115);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BARRIER)) {
			_vm->_dialogs->show(20116);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PLACARD)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(20117);
			else
				_vm->_dialogs->show(20118);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TICKET_WINDOW)) {
			if (_globals[kCurrentYear] == 1993)
				_vm->_dialogs->show(20119);
			else
				_vm->_dialogs->show(20120);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_ARCHWAY)) {
			_vm->_dialogs->show(20121);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TICKET_SELLER)) {
			_vm->_dialogs->show(20123);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_TICKET_SELLER)) {
		_vm->_dialogs->show(20124);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_BUST)) {
		_vm->_dialogs->show(20122);
		_action._inProgress = false;
		return;
	}
}

void Scene201::preActions() {
	if ((_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) && _action.isObject(NOUN_PLACARD))
		_game._player.walk(Common::Point(147, 104), FACING_NORTHWEST);
}

void Scene201::handleRaoulAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _raoulFrame)
		return;

	_raoulFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int random = -1;
	int resetFrame = -1;

	switch (_raoulFrame) {
	case 1:
	case 19:
	case 49:
		random = _vm->getRandomNumber(4, 50);

		switch (_raoulStatus) {
		case 0:
			random = 1;
			break;

		case 2:
			random = 2;
			_game._player._stepEnabled = false;
			break;

		case 3:
			random = 3;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 9;
			_raoulStatus = 1;
			break;

		case 2:
			resetFrame = 1;
			break;

		case 3:
			_game._player._visible = true;
			_sellerStatus = 2;
			_anim0ActvFl = false;
			resetFrame = 49;
			_game._player._stepEnabled = true;
			_needHoldFl = false;
			break;

		case 4:
			resetFrame = 19;
			break;

		default:
			resetFrame = 0;
			break;

		}
		break;

	case 5:
		_scene->deleteSequence(_globals._sequenceIndexes[1]);
		_game._objects.addToInventory(OBJ_ENVELOPE);
		_vm->_sound->command(26);
		_vm->_dialogs->showItem(OBJ_ENVELOPE, 834, 0);
		break;

	case 9:
		_game._player._visible = true;
		_anim0ActvFl = false;
		_game._player._stepEnabled = true;
		resetFrame = 49;
		break;

	case 23:
	case 35:
	case 45:
		random = _vm->getRandomNumber(3, 70);

		switch (_raoulStatus) {
		case 0:
			random = 2;
			break;

		case 2:
		case 3:
			random = 1;
			break;

		default:
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 45;
			break;

		case 2:
			resetFrame = 23;
			_raoulStatus = 1;
			break;

		case 3:
			resetFrame = 35;
			break;

		default:
			resetFrame = 22;
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_raoulFrame = resetFrame;
	}
}

void Scene201::handleSellerAnimation() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _sellerFrame )
		return;

	int random = -1;
	int resetFrame = -1;
	_sellerFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();

	switch (_sellerFrame ) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 10:
		if (_sellerFrame == 10)
			_raoulStatus = 2;

		switch (_sellerStatus) {
		case 0:
			random = 1;
			break;

		case 1:
			random = _vm->getRandomNumber(1, 5);
			++_sellerCount;
			if (_sellerCount > 30) {
				_sellerStatus = 0;
				random = 6;
			}
			break;

		case 2:
			if (_sellerFrame == 6)
				random = 6;
			else if (_sellerFrame == 7)
				random = 7;
			else
				random = _vm->getRandomNumber(6, 7);

			++_sellerCount;
			if (_sellerCount > 30) {
				_sellerCount = 0;
				random = _vm->getRandomNumber(6, 7);
			}
			break;

		case 3:
			random = 8;
			break;

		default:
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
			resetFrame = 3;
			break;

		case 5:
			resetFrame = 4;
			break;

		case 6:
			resetFrame = 5;
			break;

		case 7:
			resetFrame = 6;
			break;

		case 8:
			resetFrame = 7;
			break;

		default:
			break;
		}
		break;

	case 9:
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_sellerStatus = 2;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_sellerFrame = resetFrame;
	}
}

void Scene201::handleConversation() {
	int interlocutorFl = false;
	int heroFl = false;

	switch (_action._activeAction._verbId) {
	case 4:
	case 12:
	case 13:
	case 14:
		_vm->_gameConv->setInterlocutorTrigger(90);
		_needHoldFl = true;
		interlocutorFl = true;
		break;

	case 6:
		_sellerStatus = 3;
		_needHoldFl = true;
		interlocutorFl = true;
		heroFl = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 65:
		if (_sellerStatus != 3)
			_sellerStatus = 1;
		break;

	case 70:
		if (_sellerStatus != 3) {
			_sellerStatus = 0;
			_raoulStatus = 0;
		}
		break;

	case 90:
		_vm->_gameConv->setHeroTrigger(91);
		heroFl = true;
		break;

	case 91:
		_raoulStatus = 3;
		heroFl = true;
		interlocutorFl = true;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(70);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(65);

	_sellerCount = 0;
}

/*------------------------------------------------------------------------*/

Scene202::Scene202(MADSEngine *vm) : Scene2xx(vm) {
	_ticketGivenFl = false;
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_skipWalkFl = false;

	for (int i = 0; i < 5; i++) {
		_chandeliersPosX[i] = -1;
		_chandeliersHotspotId[i] = -1;
	}

	_conversationCount = -1;
	_usherStatus = -1;
	_usherFrame = -1;
	_usherCount = -1;
	_degasStatus = -1;
	_degasFrame = -1;
}

void Scene202::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_ticketGivenFl);
	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_skipWalkFl);

	for (int i = 0; i < 5; i++) {
		s.syncAsSint16LE(_chandeliersPosX[i]);
		s.syncAsSint16LE(_chandeliersHotspotId[i]);
	}

	s.syncAsSint16LE(_conversationCount);
	s.syncAsSint16LE(_usherStatus);
	s.syncAsSint16LE(_usherFrame);
	s.syncAsSint16LE(_usherCount);
	s.syncAsSint16LE(_degasStatus);
	s.syncAsSint16LE(_degasFrame);
}

void Scene202::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kTicketPeoplePresent] == 2)
		_scene->_initialVariant = 1;

	_scene->addActiveVocab(NOUN_CHANDELIER);
	_scene->addActiveVocab(NOUN_EDGAR_DEGAS);
}

void Scene202::enter() {
	_vm->_disableFastwalk = true;
	_ticketGivenFl = false;
	_chandeliersPosX[0] = 77;
	_chandeliersPosX[1] = 192;
	_chandeliersPosX[2] = 319;
	_chandeliersPosX[3] = 445;
	_chandeliersPosX[4] = 560;

	if (_globals[kTicketPeoplePresent] == 2)
		_globals[kMakeRichLeave203] = true;

	if ((_globals[kDegasNameIsKnown]) || (_globals[kCurrentYear] == 1993))
		_scene->_hotspots.activate(NOUN_GENTLEMAN, false);

	for (int i = 0; i < 5; i++) {
		_globals._sequenceIndexes[2 + i] = -1;
		_chandeliersHotspotId[i] = -1;
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_skipWalkFl = false;
	}

	_conversationCount = 0;
	_vm->_gameConv->get(17);
	_vm->_gameConv->get(9);

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RDR_9", false);
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0), false);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 0), false);

	if (_globals[kTicketPeoplePresent] == 2) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 0), 0);
		_anim0ActvFl = true;
		_usherStatus = 2;
	} else
		_scene->_hotspots.activate(NOUN_USHER, false);

	if (_globals[kDegasNameIsKnown])
		_anim1ActvFl = false;

	if ((_globals[kCurrentYear] == 1881) && (!_globals[kDegasNameIsKnown])) {
		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('d', 1), 100);
		_anim1ActvFl = true;
		_degasStatus = 4;
	}

	if (_vm->_gameConv->_restoreRunning == 17) {
		_vm->_gameConv->run(17);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
		_vm->_gameConv->exportValue(0);
		_game._player._playerPos = Common::Point(569, 147);
		_game._player._facing = FACING_NORTHEAST;
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	if (_vm->_gameConv->_restoreRunning == 9) {
		_vm->_gameConv->run(9);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_game._player._playerPos = Common::Point(400, 141);
		_game._player._facing = FACING_NORTHWEST;
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	if (_scene->_priorSceneId == 201) {
		_game._player._playerPos = Common::Point(3, 141);
		_game._player.walk(Common::Point(40, 141), FACING_EAST);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else if (_scene->_priorSceneId == 203) {
		_game._player._playerPos = Common::Point(134, 112);
		_game._player._facing = FACING_SOUTH;
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(126, 123), FACING_SOUTH);
		_game._player.setWalkTrigger(60);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else if (_scene->_priorSceneId == 204) {
		_game._player._playerPos = Common::Point(253, 117);
		_game._player.walk(Common::Point(255, 133), FACING_SOUTH);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_scene->setCamera(Common::Point(70, 0));
	} else if (_scene->_priorSceneId == 205) {
		_game._player._playerPos = Common::Point(510, 117);
		_game._player.walk(Common::Point(512, 133), FACING_SOUTH);
		_scene->setCamera(Common::Point(320, 0));
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else if ((_scene->_priorSceneId == 101) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(636, 143);
		_game._player.walk(Common::Point(598, 143), FACING_WEST);
		_scene->setCamera(Common::Point(320, 0));
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	}

	handleChandeliersPositions();
	sceneEntrySound();
}

void Scene202::step() {
	if (_game._camX._panFrame)
		handleChandeliersPositions();

	if (_anim0ActvFl)
		handleUsherAnimation();

	if (_game._trigger == 100)
		_anim1ActvFl = false;

	if (_anim1ActvFl)
		handleDegasAnimation();

	if ((_globals[kCurrentYear] == 1881) && !_globals[kDegasNameIsKnown] && (_game._player._playerPos.x < 405) && !_skipWalkFl) {
		_game._player.walk(Common::Point(400, 141), FACING_NORTHWEST);
		_game._player.setWalkTrigger(90);
		_game._player._stepEnabled = false;
		_skipWalkFl = true;
	}

	if (_game._trigger == 90) {
		_game._player._stepEnabled = true;
		_vm->_gameConv->run(9);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	switch (_game._trigger) {
	case 60:
		_scene->deleteSequence(_globals._sequenceIndexes[0]);
		_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
		_scene->_sequences.setTrigger(_globals._sequenceIndexes[0], 0, 0, 61);
		break;

	case 61:
		_vm->_sound->command(25);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	if ((_globals[kWalkerConverse] == 2) || (_globals[kWalkerConverse] == 3)) {
		++_conversationCount;
		if (_conversationCount > 200)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}
}

void Scene202::actions() {
	if (_vm->_gameConv->_running == 17) {
		handleConversation1();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->_running == 9) {
		handleConversation2();
		_action._inProgress = false;
		return;
	}

	if ((_globals[kTicketPeoplePresent] == 2) && (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_ARCHWAY) || _action.isAction(VERB_TALK_TO, NOUN_USHER))) {
		_vm->_gameConv->run(17);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
		_vm->_gameConv->exportValue(0);
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_GIVE, NOUN_TICKET, NOUN_USHER)) {
		_ticketGivenFl = true;
		_vm->_gameConv->run(17);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
		_vm->_gameConv->exportValue(1);
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_TALK_TO, NOUN_GENTLEMAN)) || (_action.isAction(VERB_TALK_TO, NOUN_EDGAR_DEGAS))) {
		if (!_globals[kDegasNameIsKnown] ) {
			_vm->_gameConv->run(9);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		} else
			_vm->_dialogs->show(20224);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_LEFT_DOOR) || _action.isAction(VERB_OPEN, NOUN_LEFT_DOOR)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[1], true);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 2, 4, 80);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[1], 0, 0, 82);
			break;

		case 80:
			_scene->deleteSequence(_globals._sequenceIndexes[0]);
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[0], 0, 0, 81);
			_vm->_sound->command(24);
			break;

		case 81: {
			int idx = _globals._sequenceIndexes[0];
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 5);
			_game.syncTimers(1, _globals._sequenceIndexes[0], 1, idx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_game._player.walk(Common::Point(134, 112), FACING_NORTH);
			_game._player.setWalkTrigger(83);
			}
			break;

		case 82:
			_game._player._visible = true;
			break;

		case 83:
			_scene->deleteSequence(_globals._sequenceIndexes[0]);
			_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
			_scene->_sequences.setTrigger(_globals._sequenceIndexes[0], 0, 0, 84);
			_vm->_sound->command(25);
			break;

		case 84:
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
			_scene->_nextSceneId = 203;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_LEFT_ARCHWAY)) {
		_scene->_nextSceneId = 201;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_ARCHWAY)) {
		_scene->_nextSceneId = 101;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_MIDDLE_DOOR)) {
		_scene->_nextSceneId = 204;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_DOOR)) {
		_scene->_nextSceneId = 205;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(20210);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(20211);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(20212);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEFT_ARCHWAY)) {
			_vm->_dialogs->show(20213);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RIGHT_ARCHWAY)) {
			_vm->_dialogs->show(20214);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEFT_DOOR)) {
			_vm->_dialogs->show(20215);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MIDDLE_DOOR)) {
			_vm->_dialogs->show(20216);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RIGHT_DOOR)) {
			_vm->_dialogs->show(20217);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT_FIXTURE)) {
			_vm->_dialogs->show(20218);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_OVERDOOR_MEDALLION)) {
			_vm->_dialogs->show(20219);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DECORATIVE_MOLDING)) {
			_vm->_dialogs->show(20220);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PAINTING)) {
			_vm->_dialogs->show(20221);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_EDGAR_DEGAS) || _action.isObject(NOUN_GENTLEMAN)) {
			_vm->_dialogs->show(20223);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_USHER)) {
			_vm->_dialogs->show(20225);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CHANDELIER)) {
			_vm->_dialogs->show(20218);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE) && (_action.isObject(NOUN_GENTLEMAN) || _action.isObject(NOUN_EDGAR_DEGAS))) {
		_vm->_dialogs->show(20226);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_USHER)) {
		_vm->_dialogs->show(20227);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_DOOR)) {
		_vm->_dialogs->show(20222);
		_action._inProgress = false;
		return;
	}
}

void Scene202::preActions() {
	if (_action.isAction(VERB_OPEN, NOUN_LEFT_DOOR))
		_game._player.walk(Common::Point(126, 123), FACING_NORTHEAST);

	if ((_globals[kTicketPeoplePresent] == 2) && (_action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_ARCHWAY)))
		_game._player.walk(Common::Point(569, 147), FACING_NORTHEAST);

	if (_action.isAction(VERB_TAKE, NOUN_GENTLEMAN) || _action.isAction(VERB_TAKE, NOUN_EDGAR_DEGAS))
		_game._player._needToWalk = false;
}

void Scene202::handleConversation1() {
	int interlocutorFl = false;
	int heroFl = false;

	switch (_action._activeAction._verbId) {
	case 0:
		if (!_ticketGivenFl)
			_usherStatus = 4;

		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		interlocutorFl = true;
		heroFl = true;
		break;

	case 3:
		_vm->_gameConv->setInterlocutorTrigger(72);
		_vm->_gameConv->setHeroTrigger(76);
		interlocutorFl = true;
		heroFl = true;
		break;

	case 4:
		_vm->_gameConv->setHeroTrigger(76);
		heroFl = true;
		interlocutorFl = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 70:
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_usherStatus = 0;
		break;

	case 72:
		_usherStatus = 17;
		break;

	case 74:
		_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);
		_usherStatus = 2;
		_conversationCount = 0;
		break;

	case 76:
		_globals[kWalkerConverse] = 0;
		_ticketGivenFl = false;
		heroFl = true;
		interlocutorFl = true;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(74);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(70);

	_usherCount = 0;
}

void Scene202::handleConversation2() {
	int interlocutorFl = false;
	int heroFl = false;

	switch (_action._activeAction._verbId) {
	case 1:
		_globals[kDegasNameIsKnown] = 1;
		break;

	case 10:
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_vm->_gameConv->setHeroTrigger(96);
		interlocutorFl = true;
		heroFl = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 74:
		_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);
		_degasStatus = 4;
		_conversationCount = 0;
		break;

	case 93:
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		switch (_action._activeAction._verbId) {
		case 1:
			_degasStatus = 1;
			break;

		case 2:
			_degasStatus = 2;
			break;

		default:
			if ((_action._activeAction._verbId != 11) && (_action._activeAction._verbId != 12))
				_degasStatus = 0;
			break;
		}
		break;

	case 96:
		_vm->_gameConv->setInterlocutorTrigger(97);
		interlocutorFl = true;
		heroFl = true;
		break;

	case 97:
		_vm->_gameConv->setHeroTrigger(98);
		_degasStatus = 0;
		interlocutorFl = true;
		heroFl = true;
		break;

	case 98:
		_globals[kWalkerConverse] = 0;
		_degasStatus = 3;

		if (_globals[kDegasNameIsKnown] == 1) {
			int idx = _scene->_dynamicHotspots.add(NOUN_EDGAR_DEGAS, VERB_WALK_TO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(596, 144), FACING_EAST);
			_scene->setDynamicAnim(idx, _globals._animationIndexes[1], 1);
		} else {
			int idx = _scene->_dynamicHotspots.add(NOUN_GENTLEMAN, VERB_WALK_TO, SYNTAX_SINGULAR_MASC, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(596, 144), FACING_EAST);
			_scene->setDynamicAnim(idx, _globals._animationIndexes[1], 1);
		}

		_globals[kDegasNameIsKnown] = 2;
		interlocutorFl = true;
		heroFl = true;
		_scene->_hotspots.activate(NOUN_GENTLEMAN, false);
		_game._player._stepEnabled = false;
		_vm->_gameConv->hold();
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(74);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(93);
}

void Scene202::handleChandeliersPositions() {
	int center = _scene->_posAdjust.x + 160;

	for (int chandelier = 0; chandelier < 5; chandelier++) {
		if (_globals._sequenceIndexes[chandelier + 2] >= 0)
			_scene->deleteSequence(_globals._sequenceIndexes[chandelier + 2]);

		int diff = center - _chandeliersPosX[chandelier];
		int dir = 0;

		if (diff < 0)
			dir = 1;
		else if (diff > 0)
			dir = -1;

		int shiftBase = (int)(abs(diff) / 5);
		if (dir < 0)
			shiftBase = -shiftBase;

		int posX = _chandeliersPosX[chandelier] + shiftBase - 1;
		int posY = _scene->_sprites[_globals._spriteIndexes[2]]->getFrameHeight(0) - 1;
		int frameWidth = _scene->_sprites[_globals._spriteIndexes[2]]->getFrameWidth(0);

		if (((posX - ((frameWidth >> 1) + 1)) >= (_scene->_posAdjust.x + 320)) || ((posX + ((frameWidth >> 1) + 1)) < _scene->_posAdjust.x))
			_globals._sequenceIndexes[chandelier + 2] = -1;
		else {
			if (_chandeliersHotspotId[chandelier] != -1)
				_scene->_dynamicHotspots.remove(_chandeliersHotspotId[chandelier]);

			_chandeliersHotspotId[chandelier] = _scene->_dynamicHotspots.add(NOUN_CHANDELIER, VERB_LOOK_AT, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(posX - 8, posY - 12, posX + 8, posY + 1));

			_globals._sequenceIndexes[chandelier + 2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[chandelier + 2], Common::Point(posX, posY));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[chandelier + 2], 1);
		}
	}
}

void Scene202::handleUsherAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _usherFrame)
		return;

	_usherFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int resetFrame = -1;
	int random;
	switch (_usherFrame) {
	case 1:
	case 13:
	case 35:
	case 51:
	case 52:
	case 53:
	case 54:
		switch (_usherStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 3);
			++_usherCount;
			if (_usherCount > 15) {
				if (_action._activeAction._verbId == 0) {
					_usherStatus = 3;
					random = 5;
				} else {
					_usherStatus = 2;
					random = 7;
				}
			}
			break;

		case 3:
			random = 5;
			break;

		case 4:
			random = 6;
			break;

		case 17:
			random = 4;
			break;

		default:
			random = 7;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 51;
			break;

		case 2:
			resetFrame = 52;
			break;

		case 3:
			resetFrame = 53;
			break;

		case 4:
			resetFrame = 21;
			_usherStatus = 17;
			break;

		case 5:
			resetFrame = 1;
			break;

		case 6:
			resetFrame = 35;
			_usherStatus = 0;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 7:
		if (_usherStatus == 3)
			random = 1;
		else
			random = 2;

		if (random == 1)
			resetFrame = 6;
		else
			resetFrame = 7;

		break;

	case 28:
		if (_usherStatus == 17) {
			random = 1;
			++_usherCount;
			if (_usherCount > 15) {
				_usherStatus = 2;
				random = 2;
			}
		} else
			random = 2;

		if (random == 1)
			resetFrame = 27;
		else
			resetFrame = 28;
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_usherFrame = resetFrame;
	}
}


void Scene202::handleDegasAnimation() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _degasFrame)
		return;

	_degasFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;
	int random;

	switch (_degasFrame) {
	case 1:
	case 17:
	case 58:
		switch (_degasStatus) {
		case 0:
			_degasStatus = 4;
			random = 1;
			break;

		case 1:
		case 2:
		case 3:
			random = 2;
			break;

		default:
			random = _vm->getRandomNumber(3, 50);
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 1;
			break;

		case 2:
			resetFrame = 58;
			break;

		case 3:
			resetFrame = 58;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 33:
	case 40:
	case 55:
	case 62:
		switch (_degasStatus) {
		case 0:
			_degasStatus = 4;
			random = 1;
			break;

		case 1:
			_degasStatus = 4;
			random = 2;
			break;

		case 2:
			_degasStatus = 4;
			random = 3;
			break;

		case 3:
			random = 4;
			break;

		default:
			random = _vm->getRandomNumber(5, 50);
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 33;
			break;

		case 2:
			resetFrame = 17;
			break;

		case 3:
			resetFrame = 42;
			break;

		case 4:
			resetFrame = 62;
			break;

		case 5:
			resetFrame = 41;
			break;

		case 6:
			resetFrame = 55;
			break;

		default:
			resetFrame = 39;
			break;
		}
		break;

	case 42:
		switch (_degasStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			break;
		}

		if (random == 1)
			resetFrame = 39;
		else
			resetFrame = 41;

		break;

	case 110:
		_vm->_gameConv->release();
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_degasFrame = resetFrame;
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
