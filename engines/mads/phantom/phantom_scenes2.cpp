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

	_vm->_gameConv->load(16);
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 0));

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

	if (_vm->_gameConv->restoreRunning() == 16) {
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

	if ((_needHoldFl) && (_vm->_gameConv->activeConvId() != 16)) {
		_game._player._stepEnabled = false;
		_needHoldFl = false;
	}
}

void Scene201::actions() {
	if (_vm->_gameConv->activeConvId() == 16) {
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

		default:
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
	bool interlocutorFl = false;
	bool heroFl = false;

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
		_scene->_variant = 1;

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
	_vm->_gameConv->load(17);
	_vm->_gameConv->load(9);

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RDR_9");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('f', 0));

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

	if (_vm->_gameConv->restoreRunning() == 17) {
		_vm->_gameConv->run(17);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
		_vm->_gameConv->exportValue(0);
		_game._player._playerPos = Common::Point(569, 147);
		_game._player._facing = FACING_NORTHEAST;
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	if (_vm->_gameConv->restoreRunning() == 9) {
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
	if (_game._camX._currentFrameFl)
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
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
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
	if (_vm->_gameConv->activeConvId() == 17) {
		handleConversation1();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 9) {
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

	if (_action.isAction(VERB_TALK_TO, NOUN_GENTLEMAN) || _action.isAction(VERB_TALK_TO, NOUN_EDGAR_DEGAS)) {
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
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 4, 80);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
			break;

		case 80:
			_scene->deleteSequence(_globals._sequenceIndexes[0]);
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 1, 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			_vm->_sound->command(24);
			break;

		case 81: {
			int idx = _globals._sequenceIndexes[0];
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 5);
			_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[0], SYNC_SEQ, idx);
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
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 84);
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

	if ((_globals[kTicketPeoplePresent] == 2) && _action.isAction(VERB_WALK_THROUGH, NOUN_RIGHT_ARCHWAY))
		_game._player.walk(Common::Point(569, 147), FACING_NORTHEAST);

	if (_action.isAction(VERB_TAKE, NOUN_GENTLEMAN) || _action.isAction(VERB_TAKE, NOUN_EDGAR_DEGAS))
		_game._player._needToWalk = false;
}

void Scene202::handleConversation1() {
	bool interlocutorFl = false;
	bool heroFl = false;

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
	bool interlocutorFl = false;
	bool heroFl = false;

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

	default:
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

Scene203::Scene203(MADSEngine *vm) : Scene2xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;
	_showNoteFl = false;

	_brieStatus = -1;
	_brieFrame = -1;
	_brieCount = -1;
	_raoulStatus = -1;
	_raoulFrame = -1;
	_raoulCount = -1;
	_richardStatus = -1;
	_richardFrame = -1;
	_daaeStatus = -1;
	_daaeFrame = -1;
	_conversationCount = -1;
}

void Scene203::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_anim3ActvFl);
	s.syncAsByte(_showNoteFl);

	s.syncAsSint16LE(_brieStatus);
	s.syncAsSint16LE(_brieFrame);
	s.syncAsSint16LE(_brieCount);
	s.syncAsSint16LE(_raoulStatus);
	s.syncAsSint16LE(_raoulFrame);
	s.syncAsSint16LE(_raoulCount);
	s.syncAsSint16LE(_richardStatus);
	s.syncAsSint16LE(_richardFrame);
	s.syncAsSint16LE(_daaeStatus);
	s.syncAsSint16LE(_daaeFrame);
	s.syncAsSint16LE(_conversationCount);
}

void Scene203::setup() {
	if (_globals[kCurrentYear] == 1993)
		_scene->_variant = 1;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene203::enter() {
	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim2ActvFl = false;
		_anim3ActvFl = false;
		_showNoteFl = false;
	}

	_conversationCount = 0;
	_scene->_hotspots.activate(NOUN_LETTER, false);
	_scene->_hotspots.activate(NOUN_PARCHMENT, false);
	_scene->_hotspots.activate(NOUN_NOTICE, false);

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('p', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RDR_6");

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_vm->_gameConv->load(5);
	} else {
		_vm->_gameConv->load(8);
		_vm->_gameConv->load(15);
	}

	if (_globals[kCurrentYear] == 1993) {
		if (_game._objects.isInRoom(OBJ_PARCHMENT)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 3);
			_scene->_hotspots.activate(NOUN_PARCHMENT, true);
		}

		_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, false);
		_scene->_hotspots.activate(NOUN_MANAGERS_CHAIR, false);

		if (!_globals[kMakeBrieLeave203]) {
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 9), 1);
			_anim0ActvFl = true;
			_brieStatus = 4;
		} else {
			_scene->_hotspots.activate(NOUN_MONSIEUR_BRIE, false);
			_scene->_hotspots.activate(NOUN_MANAGERS_CHAIR, true);
		}

		if ((_scene->_priorSceneId == RETURNING_FROM_LOADING) && (_vm->_gameConv->restoreRunning() == 5)) {
			_brieStatus = 4;
			_raoulStatus = 0;
			_anim1ActvFl = true;
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('c', 1), 0);
			_scene->setAnimFrame(_globals._animationIndexes[1], 9);
			_vm->_gameConv->run(5);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_SMALL_NOTE));
			_vm->_gameConv->exportValue(_globals[kReadBook]);
			_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_LARGE_NOTE));
			_vm->_gameConv->exportValue(_globals[kLookedAtCase]);
			_vm->_gameConv->exportValue(_globals[kCharlesNameIsKnown]);
			_vm->_gameConv->exportValue(_globals[kCanFindBookInLibrary]);
			_vm->_gameConv->exportValue(_globals[kFlorentNameIsKnown]);
			_vm->_gameConv->exportValue(_globals[kSandbagStatus]);
			_vm->_gameConv->exportValue(_globals[kObservedPhan104]);
		}

		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_hotspots.activate(NOUN_CANDLE, false);
	} else if (_globals[kJacquesStatus] == 0) {
		_scene->_hotspots.activate(NOUN_DESK_LAMP, false);
		_scene->_hotspots.activate(NOUN_MONSIEUR_BRIE, false);
		_scene->_hotspots.activate(NOUN_MANAGERS_CHAIR, false);

		if (!_globals[kMakeRichLeave203]) {
			_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('r', 1), 1);
			_anim2ActvFl = true;
			_richardStatus = 4;
		} else {
			_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, false);
			_scene->_hotspots.activate(NOUN_MANAGERS_CHAIR, true);
		}

		if ((_scene->_priorSceneId == RETURNING_FROM_LOADING) && (_vm->_gameConv->restoreRunning() == 8)) {
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('c', 1), 0);
			_scene->setAnimFrame(_globals._animationIndexes[1], 9);
			_anim1ActvFl = true;
			_game._player._visible = false;
			_raoulStatus = 0;
			_vm->_gameConv->run(8);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		}

		if (_game._objects.isInRoom(OBJ_LETTER)) {
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
			_scene->_hotspots.activate(NOUN_LETTER, true);
		}

		if (_game._objects.isInRoom(OBJ_NOTICE)) {
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 3);
			_scene->_hotspots.activate(NOUN_NOTICE, true);
		}
	} else {
		_scene->_hotspots.activate(NOUN_MONSIEUR_BRIE, false);
		_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, false);
		_scene->_hotspots.activate(NOUN_DESK_LAMP, false);
	}

	if (_vm->_gameConv->restoreRunning() == 15) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
		_game._player._playerPos = Common::Point(98, 137);
		_game._player._facing = FACING_NORTHEAST;
		_vm->_gameConv->run(15);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_vm->_gameConv->exportPointer(&_globals[kChristineToldEnvelope]);
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
	} else if (_scene->_priorSceneId == 202) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
		_game._player._playerPos = Common::Point(195, 147);
		_game._player._facing = FACING_NORTH;
	} else if (_scene->_priorSceneId == 150) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
		_game._player._playerPos = Common::Point(98, 137);
		_game._player._facing = FACING_NORTHEAST;
		_vm->_gameConv->run(15);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_vm->_gameConv->exportPointer(&_globals[kChristineToldEnvelope]);
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	} else if ((_scene->_priorSceneId == 204) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_game._player._playerPos = Common::Point(319, 123);
		_game._player._facing = FACING_SOUTHWEST;
		_game._player.walk(Common::Point(276, 123), FACING_WEST);
		_game._player.setWalkTrigger(95);
		_game._player._stepEnabled = false;
	}

	sceneEntrySound();
}

void Scene203::step() {
	if (_anim0ActvFl)
		handleBrieAnimation();

	if (_anim1ActvFl)
		handleRaoulAnimation();

	if (_anim2ActvFl)
		handleRichardAnimation();

	if (_anim3ActvFl)
		handleDaaeAnimation();

	if ((_globals[kWalkerConverse] == 2) || (_globals[kWalkerConverse] == 3)) {
		++_conversationCount;
		if (_conversationCount > 200)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	switch (_game._trigger) {
	case 95:
		_scene->deleteSequence(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[5], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 10);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 96);
		break;

	case 96:
		_vm->_sound->command(25);
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
		if (_vm->_gameConv->activeConvId() != 15)
			_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene203::actions() {
	if (_vm->_gameConv->activeConvId() == 5) {
		handleBrieConversation();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 8) {
		handleRichardConversation();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 15) {
		handleRichardAndDaaeConversation();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR) || _game._trigger) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[4], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 4, 90);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 92);
			break;

		case 90:
			_vm->_sound->command(24);
			_scene->deleteSequence(_globals._sequenceIndexes[5]);
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
			break;

		case 91:
			_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
			break;

		case 92:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[4]);
			_game._player.walk(Common::Point(319, 123), FACING_WEST);
			_game._player.setWalkTrigger(93);
			break;

		case 93:
			_scene->_nextSceneId = 204;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE)) {
		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('c', 1), 0);
		_anim1ActvFl = true;
		_game._player._visible = false;
		_raoulStatus = 0;
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[1], SYNC_PLAYER, 0);

		_vm->_gameConv->run(5);
		_vm->_gameConv->hold();
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_SMALL_NOTE));
		_vm->_gameConv->exportValue(_globals[kReadBook]);
		_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_LARGE_NOTE));
		_vm->_gameConv->exportValue(_globals[kLookedAtCase]);
		if (_globals[kCharlesNameIsKnown] == 2)
			_vm->_gameConv->exportValue(1);
		else
			_vm->_gameConv->exportValue(0);

		_vm->_gameConv->exportValue(_globals[kCanFindBookInLibrary]);
		_vm->_gameConv->exportValue(_globals[kFlorentNameIsKnown]);
		_vm->_gameConv->exportValue(_globals[kSandbagStatus]);
		_vm->_gameConv->exportValue(_globals[kObservedPhan104]);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_RICHARD)) {
		if (_globals[kCameFromFade]) {
			_vm->_gameConv->run(15);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->exportPointer(&_globals[kChristineToldEnvelope]);
		} else {
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('c', 1), 0);
			_anim1ActvFl = true;
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_raoulStatus = 0;
			_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[1], SYNC_PLAYER, 0);

			_vm->_gameConv->run(8);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->hold();
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_GRAND_FOYER)) {
		_scene->_nextSceneId = 202;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		if (_globals[kMakeBrieLeave203])
			_vm->_dialogs->show(20337);
		else
			_vm->_dialogs->show(20310);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(20311);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(20312);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOOKCASE)) {
			_vm->_dialogs->show(20313);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOORWAY)) {
			_vm->_dialogs->show(20314);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COMFY_CHAIR)) {
			_vm->_dialogs->show(20315);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DESK)) {
			_vm->_dialogs->show(20316);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MANAGERS_CHAIR)) {
			_vm->_dialogs->show(20317);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DESK_LAMP)) {
			_vm->_dialogs->show(20318);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LAMP)) {
			_vm->_dialogs->show(20319);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LIGHT_FIXTURE)) {
			_vm->_dialogs->show(20320);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WINDOW)) {
			_vm->_dialogs->show(20321);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SHEERS)) {
			_vm->_dialogs->show(20322);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TAPESTRY)) {
			_vm->_dialogs->show(20323);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRAND_FOYER)) {
			_vm->_dialogs->show(20324);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_TABLE)) {
			_vm->_dialogs->show(20325);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CANDLE)) {
			_vm->_dialogs->show(20326);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MONSIEUR_BRIE)) {
			_vm->_dialogs->show(20327);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MONSIEUR_RICHARD)) {
			_vm->_dialogs->show(20328);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PARCHMENT) && _game._objects.isInRoom(OBJ_PARCHMENT)) {
			_vm->_dialogs->show(20329);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LETTER) && _game._objects.isInRoom(OBJ_LETTER)) {
			_vm->_dialogs->show(20331);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_NOTICE) && _game._objects.isInRoom(OBJ_NOTICE)) {
			_vm->_dialogs->show(20333);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TAKE, NOUN_PARCHMENT)) {
		_vm->_dialogs->show(20330);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_LETTER)) {
		_vm->_dialogs->show(20332);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_NOTICE)) {
		_vm->_dialogs->show(20334);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_MONSIEUR_BRIE)) {
		_vm->_dialogs->show(20335);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_MONSIEUR_RICHARD)) {
		_vm->_dialogs->show(20336);
		_action._inProgress = false;
		return;
	}
}

void Scene203::preActions() {
	if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_RICHARD)) {
		if (_globals[kCameFromFade])
			_game._player.walk(Common::Point(98, 137), FACING_NORTHEAST);
		else
			_game._player.walk(Common::Point(154, 131), FACING_NORTHWEST);
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE))
		_game._player.walk(Common::Point(154, 131), FACING_NORTHWEST);

	if (_action.isAction(VERB_OPEN, NOUN_DOOR))
		_game._player.walk(Common::Point(276, 123), FACING_EAST);
}

void Scene203::handleBrieConversation() {
	bool interlocutorFl = false;
	bool heroFl = false;

	switch (_action._activeAction._verbId) {
	case 9:
	case 12:
	case 35:
	case 42:
		_vm->_gameConv->setInterlocutorTrigger(70);
		interlocutorFl = true;
		heroFl = true;
		break;

	case 14:
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_raoulStatus = 5;
		}
		break;

	case 17:
		_raoulStatus = 0;
		_vm->_gameConv->hold();
		break;

	case 24:
		_vm->_gameConv->setInterlocutorTrigger(78);
		interlocutorFl = true;
		break;

	case 20:
	case 25:
		_vm->_gameConv->setInterlocutorTrigger(76);
		interlocutorFl = true;
		break;

	case 37:
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			if (_vm->_sound->_preferRoland)
				_vm->_sound->command(65);
			else
				_scene->playSpeech(1);

			_scene->_sequences.addTimer(60, 110);
		}
		break;

	case 41:
		_globals[kDoneBrieConv203] = 1;
		_globals[kChrisFStatus] = 0;
		break;

	case 44:
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_raoulStatus = 3;
		}
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 60:
		_brieStatus = 0;
		break;

	case 65:
		_brieStatus = 4;
		break;

	case 70:
		_vm->_gameConv->setHeroTrigger(71);
		interlocutorFl = true;
		heroFl = true;
		break;

	case 71:
		_raoulStatus = 2;
		break;

	case 74:
		_vm->_gameConv->hold();
		_raoulStatus = 3;
		break;

	case 76:
		_brieStatus = 1;
		break;

	case 78:
		_brieStatus = 2;
		break;

	case 110:
		_vm->_gameConv->release();
		break;

	default:
		break;
	}

	if (!heroFl && (_raoulStatus != 5))
		_vm->_gameConv->setHeroTrigger(65);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(60);

	_brieCount = 0;
	_raoulCount = 0;
}

void Scene203::handleRichardConversation() {
	bool interlocutorFl = false;
	bool heroFl = false;

	switch (_action._activeAction._verbId) {
	case 4:
		_vm->_gameConv->setInterlocutorTrigger(83);
		interlocutorFl = true;
		break;

	case 5:
	case 15:
	case 20:
	case 21:
	case 27:
		_vm->_gameConv->setInterlocutorTrigger(70);
		interlocutorFl = true;
		heroFl = true;
		break;

	case 7:
		if (_game._objects.isInRoom(OBJ_NOTICE) && !_game._trigger) {
			_vm->_gameConv->hold();
			_raoulStatus = 3;
		}
		break;

	case 9:
		if (_game._objects.isInRoom(OBJ_LETTER) && !_game._trigger) {
			_vm->_gameConv->hold();
			_raoulStatus = 4;
		}
		break;

	case 17:
		_vm->_gameConv->setInterlocutorTrigger(85);
		interlocutorFl = true;
		break;

	case 19:
		_vm->_gameConv->setInterlocutorTrigger(81);
		interlocutorFl = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 65:
		_richardStatus = 4;
		break;

	case 70:
		_vm->_gameConv->setHeroTrigger(71);
		interlocutorFl = true;
		heroFl = true;
		break;

	case 71: {
		_raoulStatus = 2;
		int *val1 = _vm->_gameConv->getVariable(24);
		int *val2 = _vm->_gameConv->getVariable(26);
		if ((*val1) && (*val2)) {
			_globals[kDoneRichConv203] = true;
			_globals[kMadameGiryShowsUp] = true;
		}
		}
		break;

	case 74:
		_vm->_gameConv->hold();
		_raoulStatus = 3;
		break;

	case 81:
		_richardStatus = 2;
		break;

	case 83:
		_richardStatus = 1;
		break;

	case 85:
		_richardStatus = 3;
		break;

	case 100:
		_richardStatus = 0;
		break;

	default:
		break;
	}

	if (!heroFl && (_raoulStatus != 5))
		_vm->_gameConv->setHeroTrigger(65);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(100);

	_brieCount = 0;
	_raoulCount = 0;
}

void Scene203::handleRichardAndDaaeConversation() {
	bool interlocutorFl = false;
	bool heroFl = false;

	switch (_action._activeAction._verbId) {
	case 5:
		if (!_game._trigger) {
			_vm->_gameConv->hold();

			_vm->_sound->command(24);
			_scene->deleteSequence(_globals._sequenceIndexes[5]);
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
			interlocutorFl = true;
			heroFl = true;
		}
		break;

	case 11:
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_daaeStatus = 3;
		}
		break;

	case 13:
	case 14:
	case 15:
		_globals[kChristineDoorStatus] = 1;
		_globals[kTicketPeoplePresent] = 2;
		heroFl = true;
		interlocutorFl = true;
		_vm->_gameConv->setInterlocutorTrigger(115);
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 65:
		if (_globals[kWalkerConverse] != 0)
			_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);

		if ((_richardStatus != 5) && (_richardStatus != 7))
			_richardStatus = 4;

		_conversationCount = 0;
		break;

	case 90:
		_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('d', 1), 0);
		_anim3ActvFl = true;
		_daaeStatus = 0;
		break;

	case 100:
		if (_globals[kWalkerConverse] != 0)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);

		if ((_action._activeAction._verbId == 7) || (_action._activeAction._verbId == 9))
			_daaeStatus = 1;
		else if (_richardStatus == 7)
			_richardStatus = 5;
		else if (_richardStatus != 5)
			_richardStatus = 0;

		break;

	case 115:
		_globals[kWalkerConverse] = 0;
		heroFl = true;
		interlocutorFl = true;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(65);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(100);

	_brieCount = 0;
	_raoulCount = 0;
}

void Scene203::handleBrieAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _brieFrame)
		return;

	_brieFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int resetFrame = -1;
	int random = -1;

	switch (_brieFrame) {
	case 1:
	case 2:
	case 4:
	case 11:
	case 22:
	case 31:
	case 35:
	case 29:
		random = _vm->getRandomNumber(6, 45);

		switch (_brieStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 2);
			++_brieCount;
			if (_brieCount > 20) {
				_brieStatus = 4;
				random = 45;
			}
			break;

		case 1:
			random = 5;
			break;

		case 2:
			random = 4;
			break;

		case 3:
			random = 3;
			_brieStatus = 0;
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
			resetFrame = 22;
			_brieStatus = 0;
			break;

		case 5:
			resetFrame = 13;
			break;

		case 6:
			resetFrame = 4;
			break;

		case 7:
			resetFrame = 29;
			break;

		case 8:
			resetFrame = 31;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 30:
		random = _vm->getRandomNumber(1, 45);

		switch (_brieStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 30;
		else
			resetFrame = 29;

		break;

	case 8:
		random = _vm->getRandomNumber(1, 45);

		switch (_brieStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
			random = 3;
			break;
		default:
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 11;
			break;

		case 2:
			resetFrame = 12;
			break;

		case 3:
			resetFrame = 8;
			break;

		default:
			resetFrame = 7;
			break;
		}
		break;

	case 12:
		random = _vm->getRandomNumber(1, 45);

		switch (_brieStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 7;
		else
			resetFrame = 11;

		break;

	case 13:
		random = _vm->getRandomNumber(1, 45);

		switch (_brieStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 7;
		else
			resetFrame = 12;

		break;

	case 17:
	case 18:
		switch (_brieStatus) {
		case 0:
		case 2:
		case 3:
			random = 3;
			break;
		case 1:
			random = _vm->getRandomNumber(1, 2);
			++_brieCount;
			if (_brieCount > 20) {
				_brieStatus = 4;
				random = 3;
			}
			break;

		default:
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 16;
			break;

		case 2:
			resetFrame = 17;
			break;

		case 3:
			resetFrame = 18;
			break;

		default:
			break;
		}
		break;

	case 33:
	case 40:
		random = _vm->getRandomNumber(1, 45);

		switch (_brieStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
			random = 1;
			break;
		default:
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 33;
			break;

		case 2:
			resetFrame = 35;
			break;

		default:
			resetFrame = 32;
			break;
		}
		break;

	case 38:
		random = _vm->getRandomNumber(1, 45);

		switch (_brieStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 38;
		else
			resetFrame = 37;

		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_brieFrame = resetFrame;
	}
}

void Scene203::handleRichardAnimation() {
	if (_scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame() == _richardFrame)
		return;

	_richardFrame = _scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame();
	int resetFrame = -1;
	int random = -1;

	switch (_richardFrame) {
	case 1:
	case 2:
	case 3:
	case 7:
	case 15:
	case 25:
	case 37:
	case 53:
	case 59:
	case 63:
	case 67:
	case 69:
	case 79:
	case 87:
	case 108:
		random = _vm->getRandomNumber(8, 45);

		switch (_richardStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 3);
			++_brieCount;
			if (_brieCount > 20) {
				_richardStatus = 4;
				random = 45;
			}
			break;

		case 1:
			random = 4;
			_richardStatus = 0;
			break;

		case 2:
			random = 5;
			_richardStatus = 0;
			break;

		case 3:
			random = 6;
			_richardStatus = 0;
			break;

		case 5:
			random = 7;
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
			resetFrame = 69;
			break;

		case 5:
			resetFrame = 53;
			_richardStatus = 0;
			break;

		case 6:
			resetFrame = 42;
			break;

		case 7:
			resetFrame = 87;
			break;

		case 8:
			resetFrame = 25;
			break;

		case 9:
			resetFrame = 15;
			break;

		case 10:
			resetFrame = 3;
			break;

		case 11:
			resetFrame = 7;
			break;

		case 12:
			resetFrame = 59;
			break;

		case 13:
			resetFrame = 63;
			break;

		case 14:
			resetFrame = 67;
			break;

		case 15:
			resetFrame = 79;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 5:
		random = _vm->getRandomNumber(1, 45);

		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 5;
		else
			resetFrame = 4;

		break;

	case 11:
		random = _vm->getRandomNumber(1, 45);

		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 11;
		else
			resetFrame = 10;

		break;

	case 61:
		random = _vm->getRandomNumber(1, 45);

		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 61;
		else
			resetFrame = 60;

		break;

	case 65:
		random = _vm->getRandomNumber(1, 45);

		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 65;
		else
			resetFrame = 64;

		break;

	case 68:
		random = _vm->getRandomNumber(1, 45);

		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 68;
		else
			resetFrame = 67;

		break;

	case 83:
		random = _vm->getRandomNumber(1, 45);

		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 83;
		else
			resetFrame = 82;

		break;

	case 92:
		_vm->_gameConv->release();
		break;

	case 93:
	case 94:
	case 95:
		if (_richardStatus == 5) {
			random = _vm->getRandomNumber(1, 3);
			++_brieCount;
			if (_brieCount > 20) {
				_richardStatus = 7;
				random = 4;
			}
		} else
			random = 4;

		switch (random) {
		case 1:
			resetFrame = 92;
			break;

		case 2:
			resetFrame = 93;
			break;

		case 3:
			resetFrame = 94;
			break;

		case 4:
			resetFrame = 95;
			break;

		default:
			break;
		}
		break;

	case 100:
	case 101:
	case 102:
		switch (_richardStatus) {
		case 5:
			random = _vm->getRandomNumber(1, 3);
			++_brieCount;
			if (_brieCount > 20) {
				_richardStatus = 7;
				random = 1;
			}
			break;

		case 7:
			random = 1;
			break;

		default:
			random = 4;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 99;
			break;

		case 2:
			resetFrame = 100;
			break;

		case 3:
			resetFrame = 101;
			break;

		case 4:
			resetFrame = 102;
			break;

		default:
			break;
		}
		break;

	case 31:
		random = _vm->getRandomNumber(1, 45);

		switch (_richardStatus) {
		case 0 :
		case 1:
		case 2:
		case 3:
		case 5:
			random = 1;
			break;
		default:
			break;
		}

		if (random == 1)
			resetFrame = 31;
		else
			resetFrame = 30;

		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[2], resetFrame);
		_richardFrame = resetFrame;
	}
}

void Scene203::handleRaoulAnimation() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _raoulFrame)
		return;

	_raoulFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;
	int random = -1;

	switch (_raoulFrame) {
	case 9:
	case 21:
	case 57:
		_vm->_gameConv->release();
		break;

	case 10:
	case 22:
	case 30:
	case 34:
	case 58:
		if (_raoulFrame == 22)
			_raoulStatus = 0;

		random = _vm->getRandomNumber(5, 45);

		switch (_raoulStatus) {
		case 1:
			random = 1;
			break;

		case 2:
			_game._player._stepEnabled = false;
			random = 2;
			break;

		case 3:
		case 5:
			random = 3;
			break;

		case 4:
			random = 4;
			break;

		default:
			random = _vm->getRandomNumber(5, 100);
			break;
			}

		switch (random) {
		case 1:
			resetFrame = 22;
			break;

		case 2:
			resetFrame = 34;
			break;

		case 3:
			resetFrame = 10;
			break;

		case 4:
			resetFrame = 45;
			break;

		case 5:
			resetFrame = 30;
			break;

		default:
			resetFrame = 9;
			break;
		}
		break;

	case 16:
		switch (_raoulStatus) {
		case 3:
			random = 1;
			break;

		case 0:
		case 2:
			random = 2;
			break;

		case 5:
			random = 3;
			break;

		default:
			break;
		}

		switch (random) {
		case 1:
			if (_globals[kCurrentYear] == 1881) {
				resetFrame = 16;
				_scene->deleteSequence(_globals._sequenceIndexes[1]);
				_scene->_hotspots.activate(NOUN_NOTICE, false);
				_game._objects.addToInventory(OBJ_NOTICE);
				_vm->_sound->command(26);
				_scene->_speechReady = -1;
				_vm->_dialogs->showItem(OBJ_NOTICE, 814, 5);
				_raoulStatus = 0;
			} else {
				resetFrame = 16;
				_scene->deleteSequence(_globals._sequenceIndexes[1]);
				_scene->_hotspots.activate(NOUN_PARCHMENT, false);
				_game._objects.addToInventory(OBJ_PARCHMENT);
				_vm->_sound->command(26);
				_scene->_speechReady = -1;
				_vm->_dialogs->showItem(OBJ_PARCHMENT, 812, 3);
				_raoulStatus = 0;
			}
			break;

		case 2:
			if (_showNoteFl) {
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_showNoteFl = false;
			}
			break;

		default:
			if (!_showNoteFl) {
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], true, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
				_vm->_gameConv->release();
				_showNoteFl = true;
			}
			resetFrame = 15;
			break;
		}
		break;

	case 26:
		random = _vm->getRandomNumber(1, 45);

		switch (_raoulStatus) {
		case 0:
		case 2:
		case 3:
		case 4:
		case 5:
			random = 1;
			break;

		case 1:
			random = 2;
			break;

		default:
			break;
		}

		if (random == 1)
			resetFrame = 26;
		else
			resetFrame = 25;

		break;

	case 45:
		_anim1ActvFl = false;
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[1]);
		resetFrame = 58;
		if (_globals[kDoneBrieConv203] && (_globals[kCurrentYear] == 1993)) {
			_globals[kPrompterStandStatus] = 1;
			_scene->_nextSceneId = 150;
		}
		break;

	case 52:
		_scene->deleteSequence(_globals._sequenceIndexes[3]);
		_scene->_hotspots.activate(NOUN_LETTER, false);
		_game._objects.addToInventory(OBJ_LETTER);
		_vm->_sound->command(26);
		_scene->_speechReady = -1;
		_vm->_dialogs->showItem(OBJ_LETTER, 813, 4);
		_raoulStatus = 0;
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_raoulFrame = resetFrame;
	}
}

void Scene203::handleDaaeAnimation() {
	if (_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == _daaeFrame)
		return;

	_daaeFrame = _scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame();
	int resetFrame = -1;
	int random = -1;

	switch (_daaeFrame) {
	case 20:
		_vm->_gameConv->release();
		_richardStatus = 5;
		break;

	case 175:
		_richardStatus = 4;
		break;

	case 198:
		_vm->_gameConv->release();
		break;

	case 201:
		_scene->_sequences.addTimer(1, 95);
		break;

	case 76:
	case 92:
	case 102:
	case 123:
		switch (_daaeStatus) {
		case 1:
			random = _vm->getRandomNumber(1, 2);
			_daaeStatus = 2;
			break;

		case 2:
			random = 3;
			_daaeStatus = 0;
			break;

		case 3:
			random = 4;
			break;

		default:
			random = 5;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 79;
			break;

		case 2:
			resetFrame = 92;
			break;

		case 3:
			resetFrame = 102;
			break;

		case 4:
			resetFrame = 123;
			break;

		case 5:
			resetFrame = 75;
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
		_daaeFrame = resetFrame;
	}
}

/*------------------------------------------------------------------------*/

Scene204::Scene204(MADSEngine *vm) : Scene2xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_anim2ActvFl = false;
	_anim3ActvFl = false;
	_raoulDown = false;
	_florentGone = false;
	_skip1Fl = false;
	_skip2Fl = false;
	_skip3Fl = false;
	_endGameFl = false;

	_brieStatus = -1;
	_brieFrame = -1;
	_florStatus = -1;
	_florFrame = -1;
	_raoulStatus = -1;
	_raoulFrame = -1;
	_raoulCount = -1;
}

void Scene204::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_anim2ActvFl);
	s.syncAsByte(_anim3ActvFl);
	s.syncAsByte(_raoulDown);
	s.syncAsByte(_florentGone);
	s.syncAsByte(_skip1Fl);
	s.syncAsByte(_skip2Fl);
	s.syncAsByte(_skip3Fl);
	s.syncAsByte(_endGameFl);

	s.syncAsSint16LE(_brieStatus);
	s.syncAsSint16LE(_brieFrame);
	s.syncAsSint16LE(_florStatus);
	s.syncAsSint16LE(_florFrame);
	s.syncAsSint16LE(_raoulStatus);
	s.syncAsSint16LE(_raoulFrame);
	s.syncAsSint16LE(_raoulCount);
}

void Scene204::setup() {
	if ((_globals[kCurrentYear] == 1993) || _globals[kRightDoorIsOpen504])
		_scene->_variant = 1;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene204::enter() {
	_skip3Fl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim2ActvFl = false;
		_anim3ActvFl = false;
		_florentGone = false;
		_skip1Fl = false;
		_skip2Fl = false;
		_endGameFl = false;
		_raoulDown = true;
	}

	if (_globals[kTicketPeoplePresent] == 2)
		_globals[kMakeRichLeave203] = true;

	if (_globals[kRightDoorIsOpen504])
		_endGameFl = true;

	warning("TODO: If end of game, remove the walking areas");

	_scene->_hotspots.activate(NOUN_BOOK, false);
	_vm->_gameConv->load(22);

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 6));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RALRH_9");
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RDRR_6");

	if (_game._objects.isInRoom(OBJ_BOOK) || (_globals[kCurrentYear] == 1881) || _endGameFl) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 5);
		if (_globals[kScannedBookcase] && (_globals[kCurrentYear] == 1993))
			_scene->_hotspots.activate(NOUN_BOOK, true);
	}

	if ((_globals[kCurrentYear] == 1993) || _endGameFl) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		_scene->drawToBackground(_globals._spriteIndexes[0], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_sprites.remove(_globals._spriteIndexes[0]);
		_scene->_hotspots.activate(NOUN_CANDLE, false);
		_scene->_hotspots.activate(NOUN_BUST, false);
		_scene->_hotspots.activate(NOUN_COFFEE_TABLE, false);

		int idx = _scene->_dynamicHotspots.add(NOUN_COFFEE_TABLE, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(83, 140, 83 + 45, 140 + 12));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(84, 150), FACING_SOUTHEAST);

		idx = _scene->_dynamicHotspots.add(NOUN_GRAND_FOYER, VERB_EXIT_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(199, 147, 199 + 52, 147 + 8));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(224, 152), FACING_SOUTH);
		_scene->_dynamicHotspots.setCursor(idx, CURSOR_GO_DOWN);

		idx = _scene->_dynamicHotspots.add(NOUN_GRAND_FOYER, VERB_EXIT_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(145, 147, 145 + 54, 147 + 8));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(175, 152), FACING_SOUTH);
		_scene->_dynamicHotspots.setCursor(idx, CURSOR_GO_DOWN);
	} else {
		_scene->_hotspots.activate(NOUN_LIGHT, false);
		_scene->_hotspots.activate(NOUN_GLASS_CASE, false);

		int idx = _scene->_dynamicHotspots.add(NOUN_COMFY_CHAIR, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(220, 147, 220 + 6, 147 + 8));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(220, 150), FACING_SOUTHEAST);

		idx = _scene->_dynamicHotspots.add(NOUN_COMFY_CHAIR, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(226, 134, 226 + 12, 134 + 21));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(220, 150), FACING_SOUTHEAST);

		idx = _scene->_dynamicHotspots.add(NOUN_COMFY_CHAIR, VERB_WALK_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(238, 128, 238 + 13, 128 + 27));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(220, 150), FACING_SOUTHEAST);

		idx = _scene->_dynamicHotspots.add(NOUN_GRAND_FOYER, VERB_EXIT_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(199, 147, 199 + 19, 147 + 8));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(209, 152), FACING_SOUTH);
		_scene->_dynamicHotspots.setCursor(idx, CURSOR_GO_DOWN);

		idx = _scene->_dynamicHotspots.add(NOUN_GRAND_FOYER, VERB_EXIT_TO, SYNTAX_SINGULAR, EXT_NONE, Common::Rect(84, 147, 84 + 61, 147 + 8));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(115, 152), FACING_SOUTH);
		_scene->_dynamicHotspots.setCursor(idx, CURSOR_GO_DOWN);
	}

	if ((_scene->_priorSceneId == 306) || (_endGameFl)) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('f', 0));
		_scene->drawToBackground(_globals._spriteIndexes[6], 1, Common::Point(-32000, -32000), 0, 100);
	} else if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 1));
		_scene->drawToBackground(_globals._spriteIndexes[1], 1, Common::Point(-32000, -32000), 0, 100);
		_scene->_sprites.remove(_globals._spriteIndexes[1]);
	}

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		if (_endGameFl) {
			_game.loadQuoteSet(0x75, 0);

			_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('r', 1), 0);
			_anim2ActvFl = true;

			if (_florentGone) {
				_scene->setAnimFrame(_globals._animationIndexes[2], 9);
				_raoulStatus = 1;
			} else {
				_scene->setAnimFrame(_globals._animationIndexes[2], 32);
				_raoulStatus = 4;
			}

			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('m', 1), 0);
			_anim0ActvFl = true;
			_brieStatus = 2;

			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('f', 1), 0);
			_anim1ActvFl = true;
			_florStatus = 2;

			if (_florentGone)
				_scene->setAnimFrame(_globals._animationIndexes[1], 172);
			else if (!_raoulDown)
				_scene->setAnimFrame(_globals._animationIndexes[1], 21);

			_game._player._visible = false;
			_vm->_gameConv->run(22);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
		} else {
			_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
		}
	} else if (_scene->_priorSceneId == 202) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
		if (_globals[kCurrentYear] == 1993)
			_game._player._playerPos = Common::Point(175, 145);
		else
			_game._player._playerPos = Common::Point(115, 147);

		_game._player._facing = FACING_NORTHWEST;
	} else if (_scene->_priorSceneId == 150) {
		int size = _game._objects.size();
		for (int i = 0; i < size; i++)
			_game._objects.setRoom(i, 1);

		_game.loadQuoteSet(0x75, 0);

		_globals._animationIndexes[2] = _scene->loadAnimation(formAnimName('r', 1), 0);
		_anim2ActvFl = true;
		_raoulStatus = 4;
		_scene->setAnimFrame(_globals._animationIndexes[2], 32);

		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('m', 1), 0);
		_anim0ActvFl = true;
		_brieStatus = 2;

		_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('f', 1), 0);
		_anim1ActvFl = true;
		_raoulDown = true;
		_florStatus = 2;

		_game._player._visible = false;
		_endGameFl = true;

		_vm->_gameConv->run(22);
		_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
	} else if ((_scene->_priorSceneId == 203) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player.firstWalk(Common::Point(-10, 136), FACING_EAST, Common::Point(30, 140), FACING_EAST, true);
		_game._player.setWalkTrigger(70);
		_game._player._stepEnabled = false;
	}

	sceneEntrySound();
}

void Scene204::step() {
	if (_anim0ActvFl)
		handleBrieAnimation();

	if (_anim1ActvFl)
		handleFlorAnimation();

	if (_anim2ActvFl)
		handleRaoulAnimation();

	if (_anim3ActvFl)
		handleEndAnimation();

	if (_game._trigger == 85)
		_scene->_nextSceneId = 250;

	if ((_vm->_gameConv->activeConvId() != 22) && !_skip1Fl && _endGameFl) {
		_game._player._stepEnabled = false;
		_skip1Fl = true;
	}

	if (_game._trigger == 70) {
		 _globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 8, 1);
		 _scene->_sequences.setDepth(_globals._sequenceIndexes[3], 10);
		 _scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
		 _scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	} else if (_game._trigger == 71) {
		 _vm->_sound->command(25);
		 _globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, 1);
		 _scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
		 _game._player._stepEnabled = true;
	}
}

void Scene204::actions() {
	if (_vm->_gameConv->activeConvId() == 22) {
		handleConversation();

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR) || _action.isAction(VERB_OPEN, NOUN_DOOR)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 4);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 4, 60);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
			break;

		case 60:
			_vm->_sound->command(24);
			_scene->deleteSequence(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
			break;

		case 61:
			_game._player._visible = true;
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[5]);
			_game._player.walk(Common::Point(0, 136), FACING_WEST);
			_game._player.setWalkTrigger(62);
			break;

		case 62:
			_scene->_nextSceneId = 203;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BOOK) && (_game._objects.isInRoom(OBJ_BOOK) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[4], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 8, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->deleteSequence(_globals._sequenceIndexes[2]);
			_scene->_hotspots.activate(NOUN_BOOK, false);
			_game._objects.addToInventory(OBJ_BOOK);
			_vm->_sound->command(26);
			break;

		case 2:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[4]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_vm->_dialogs->showItem(OBJ_BOOK, 815, 0);
			_globals[kReadBook] = true;
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_GRAND_FOYER)) {
		_scene->_nextSceneId = 202;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(20410);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(20411);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(20412);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RUG)) {
			_vm->_dialogs->show(20413);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GLASS_CASE)) {
			_vm->_dialogs->show(20414);
			_globals[kLookedAtCase] = true;
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			if (_globals[kSandbagStatus] == 0)
				_vm->_dialogs->show(20429);
			else
				_vm->_dialogs->show(20416);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOOKCASE)) {
			if (_globals[kCanFindBookInLibrary] && (_globals[kCurrentYear] == 1993)) {
				if ((_scene->_customDest.x < 46) && !_game._objects.isInInventory(OBJ_BOOK)) {
					if (!_globals[kScannedBookcase]) {
						_vm->_dialogs->show(20433);
						_scene->_hotspots.activate(NOUN_BOOK, true);
						_globals[kScannedBookcase] = true;
					} else {
						_vm->_dialogs->show(20437);
					}
				} else {
					_vm->_dialogs->show(20417);
				}
			} else {
				_vm->_dialogs->show(20417);
			}
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SOFA)) {
			_vm->_dialogs->show(20418);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_END_TABLE)) {
			_vm->_dialogs->show(20419);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LAMP)) {
			_vm->_dialogs->show(20420);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BUST)) {
			_vm->_dialogs->show(20421);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COFFEE_TABLE)) {
			_vm->_dialogs->show(20422);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_COMFY_CHAIR)) {
			_vm->_dialogs->show(20423);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DECORATIVE_VASE )) {
			_vm->_dialogs->show(20424);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PAINTING)) {
			_vm->_dialogs->show(20425);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRAND_FOYER)) {
			_vm->_dialogs->show(20426);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_DOOR)) {
			_vm->_dialogs->show(20427);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WINDOW)) {
			_vm->_dialogs->show(20428);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOOK) && _game._objects.isInRoom(OBJ_BOOK)) {
			_vm->_dialogs->show(20434);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_BUST)) {
		_vm->_dialogs->show(20436);
		_action._inProgress = false;
		return;
	}
}

void Scene204::preActions() {
	if (_action.isAction(VERB_LOOK, NOUN_BOOKCASE))
		_game._player._needToWalk = true;

	if (_action.isAction(VERB_LOOK, NOUN_BOOK) && _game._objects.isInRoom(OBJ_BOOK))
		_game._player._needToWalk = true;

	if (_action.isAction(VERB_OPEN, NOUN_DOOR))
		_game._player.walk(Common::Point(27, 139), FACING_WEST);
}

void Scene204::handleConversation() {
	bool interlocutorFl = false;
	bool heroFl = false;

	switch (_action._activeAction._verbId) {
	case 6:
	case 7:
	case 8:
		if (_raoulDown) {
			_vm->_gameConv->hold();
			_raoulDown = false;
		}
		break;

	case 17:
		if (!_game._trigger) {
			_florStatus = 3;
			_florentGone = true;
			interlocutorFl = true;
			heroFl = true;
			_vm->_gameConv->hold();
		}
		break;

	case 25:
		if (!_game._trigger) {
			_raoulStatus = 5;
			_florStatus = 5;
			interlocutorFl = true;
			heroFl = true;
			_vm->_gameConv->hold();
		}
		break;

	case 29:
		interlocutorFl = true;
		heroFl = true;
		if (!_game._trigger) {
			_brieStatus = 3;
			_vm->_gameConv->hold();
		}
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 75:
		if (_florentGone) {
			if (_raoulStatus != 2)
				_raoulStatus = 0;
		} else
			_florStatus = 4;

		break;

	case 80:
		if (_florentGone) {
			if ((_action._activeAction._verbId != 18) && (_action._activeAction._verbId != 23))
				_brieStatus = 0;
		} else {
			switch (_action._activeAction._verbId) {
			case 1:
			case 7:
			case 8:
			case 9:
			case 13:
			case 15:
			case 19:
			case 20:
			case 21:
			case 22:
				_brieStatus = 0;
				break;

			default:
				_florStatus = 0;
				break;
			}
		}
		break;

	default:
		break;
	}

	if (!heroFl && !_raoulDown)
		_vm->_gameConv->setHeroTrigger(75);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(80);

	_raoulCount = 0;
}

void Scene204::handleBrieAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _brieFrame)
		return;

	_brieFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int resetFrame = -1;
	int random;

	switch (_brieFrame) {
	case 80:
		_vm->_gameConv->release();
		_raoulStatus = 2;
		break;

	case 173:
		_game._player._stepEnabled = true;
		_vm->_dialogs->show(20430);
		_game._player._stepEnabled = false;
		break;

	case 174:
		_raoulStatus = 3;
		resetFrame = 173;
		break;

	case 1:
	case 22:
	case 49:
	case 7:
	case 13:
	case 33:
	case 61:
		switch (_brieStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 4);
			_brieStatus = 2;
			break;

		case 1:
			random = 5;
			break;

		case 3:
			random = 6;
			break;

		case 4:
			random = 7;
			break;

		default:
			random = 8;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 1;
			_brieStatus = 2;
			break;

		case 2:
			resetFrame = 7;
			_brieStatus = 2;
			break;

		case 3:
			resetFrame = 22;
			_brieStatus = 2;
			break;

		case 4:
			resetFrame = 49;
			_brieStatus = 2;
			break;

		case 5:
			resetFrame = 13;
			_brieStatus = 2;
			break;

		case 6:
			resetFrame = 61;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_brieFrame = resetFrame;
	}
}

void Scene204::handleFlorAnimation() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _florFrame)
		return;

	_florFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int resetFrame = -1;
	int random;

	switch (_florFrame) {
	case 80:
		_scene->setAnimFrame(_globals._animationIndexes[2], 1);
		_game.syncTimers(SYNC_ANIM, _globals._animationIndexes[2], SYNC_ANIM, _globals._animationIndexes[1]);
		_raoulStatus = 1;
		break;

	case 86:
		_vm->_gameConv->release();
		break;

	case 173:
		resetFrame = 172;
		break;

	case 1:
	case 2:
	case 3:
		if (_raoulDown) {
			random = _vm->getRandomNumber(1, 1000);
			if (random < 300)
				resetFrame = 0;
			else if (random < 600)
				resetFrame = 1;
			else
				resetFrame = 2;
		}
		break;

	case 21:
	case 180:
		_vm->_gameConv->release();
		break;

	case 22:
	case 50:
	case 30:
	case 174:
	case 175:
	case 176:
	case 181:
		switch (_florStatus) {
		case 0:
			random = 1;
			_florStatus = 2;
			break;

		case 1:
			random = 2;
			_florStatus = 2;
			break;

		case 3:
			random = 3;
			break;

		case 5:
			random = 4;
			_florStatus = 2;
			break;

		case 4:
			random = _vm->getRandomNumber(5, 7);
			++_raoulCount;
			if (_raoulCount > 17) {
				_florStatus = 2;
				random = 8;
			}
			break;

		default:
			random = 7;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 22;
			break;

		case 2:
			resetFrame = 30;
			break;

		case 3:
			resetFrame = 53;
			break;

		case 4:
			resetFrame = 176;
			break;

		case 5:
			resetFrame = 173;
			break;

		case 6:
			resetFrame = 174;
			break;

		case 7:
			resetFrame = 175;
			break;

		default:
			resetFrame = 21;
			break;
		}
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_florFrame = resetFrame;
	}
}

void Scene204::handleRaoulAnimation() {
	if (_scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame() == _raoulFrame)
		return;

	_raoulFrame = _scene->_animation[_globals._animationIndexes[2]]->getCurrentFrame();
	int resetFrame = -1;
	int random;

	switch (_raoulFrame) {
	case 1:
		if (_raoulStatus == 4)
			resetFrame = 0;

		break;

	case 10:
	case 14:
	case 20:
	case 258:
		switch (_raoulStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 2);
			_raoulStatus = 1;
			break;

		case 2:
			random = 3;
			break;

		case 5:
			random = 4;
			break;

		default:
			random = 5;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 10;
			_raoulStatus = 1;
			break;

		case 2:
			resetFrame = 14;
			_raoulStatus = 1;
			break;

		case 3:
			resetFrame = 20;
			break;

		case 4:
			resetFrame = 253;
			_raoulStatus = 1;
			break;

		default:
			resetFrame = 9;
			break;
		}
		break;

	case 31:
		if (_raoulStatus == 3)
			resetFrame = 33;
		else
			resetFrame = 30;

		break;

	case 33:
		resetFrame = 32;
		break;

	case 114:
		_scene->deleteSequence(_globals._sequenceIndexes[2]);
		_game._objects.addToInventory(OBJ_BOOK);
		break;

	case 213:
		_game._player._stepEnabled = true;
		_vm->_dialogs->showItem(OBJ_BOOK, 20431, 0);
		_game._player._stepEnabled = false;
		break;

	case 229:
		_game._player._stepEnabled = true;
		_vm->_dialogs->showItem(OBJ_BOOK, 20432, 0);
		_game._player._stepEnabled = false;
		break;

	case 237:
		_scene->freeAnimation(_globals._animationIndexes[1]);
		_scene->freeAnimation(_globals._animationIndexes[0]);
		_anim0ActvFl = false;
		_anim1ActvFl = false;
		_anim3ActvFl = true;
		_globals._animationIndexes[3] = _scene->loadAnimation(formAnimName('e', 1), 0);
		_scene->loadSpeech(9);
		break;

	case 253:
		resetFrame = 244;
		break;

	case 257:
		_vm->_gameConv->release();
		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[2], resetFrame);
		_raoulFrame = resetFrame;
	}
}

void Scene204::handleEndAnimation() {
	if ((_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == 15) && !_skip3Fl) {
		_scene->playSpeech(9);
		_skip3Fl = true;
	}

	if ((_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == 26) && !_skip2Fl) {
		_scene->_sequences.addTimer(300, 85);
		_scene->_kernelMessages.add(Common::Point(123, 137), 0x1110, 0, 0, 360, _game.getQuote(0x75));
		_skip2Fl = true;
	}

	if (_scene->_animation[_globals._animationIndexes[3]]->getCurrentFrame() == 27)
		_scene->setAnimFrame(_globals._animationIndexes[3], 12);
}

/*------------------------------------------------------------------------*/

Scene205::Scene205(MADSEngine *vm) : Scene2xx(vm) {
	_anim0ActvFl = false;
	_anim1ActvFl = false;
	_noConversationHold = false;
	_giveTicketFl = false;

	_richardFrame = -1;
	_richardStatus = -1;
	_richardCount = -1;
	_giryFrame = -1;
	_giryStatus = -1;
	_giryCount = -1;
	_conversationCounter = -1;
	_lastRandom = -1;
}

void Scene205::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_anim1ActvFl);
	s.syncAsByte(_noConversationHold);
	s.syncAsByte(_giveTicketFl);

	s.syncAsSint16LE(_richardFrame);
	s.syncAsSint16LE(_richardStatus);
	s.syncAsSint16LE(_richardCount);
	s.syncAsSint16LE(_giryFrame);
	s.syncAsSint16LE(_giryStatus);
	s.syncAsSint16LE(_giryCount);
	s.syncAsSint16LE(_conversationCounter);
	s.syncAsSint16LE(_lastRandom);
}

void Scene205::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	if (_globals[kCurrentYear] != 1881)
		return;

	if (_globals[kJacquesStatus] == 1)
		_scene->_variant = 3;
	else if (_globals[kJacquesStatus] == 0) {
		if (_globals[kMadameGiryLocation] == 0)
			_scene->_variant = 2;
		else if (_globals[kMadameGiryLocation] == 1)
			_scene->_variant = 1;
	}
}

void Scene205::enter() {
	_vm->_disableFastwalk = true;

	if (_globals[kJacquesStatus] != 1) {
		_scene->_rails.disableNode(6);
		_scene->_rails.disableNode(7);
		_scene->_rails.disableNode(8);
		_scene->_rails.disableNode(9);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING) {
		_lastRandom = -1;
		_anim0ActvFl = false;
		_anim1ActvFl = false;
	}

	_conversationCounter = 0;
	_noConversationHold = false;
	_giveTicketFl = false;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RDR_9");

	_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, false);
	_scene->_hotspots.activate(NOUN_MADAME_GIRY, false);
	_scene->_hotspots.activate(NOUN_WOMAN, false);

	_vm->_gameConv->load(18);
	_vm->_gameConv->load(10);
	_vm->_gameConv->load(11);

	if (_globals[kCurrentYear] == 1881) {
		if ((_globals[kMadameGiryShowsUp]) && (_globals[kJacquesStatus] == 0)) {
			_globals._animationIndexes[1] = _scene->loadAnimation(formAnimName('g', 1), 1);
			_anim1ActvFl = true;
			_giryStatus = 2;

			int idx = _scene->_dynamicHotspots.add(NOUN_MADAME_GIRY, VERB_WALK_TO, SYNTAX_SINGULAR_FEM, EXT_NONE, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots[idx]._articleNumber = PREP_ON;
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(75, 84), FACING_NORTHWEST);
			_scene->setDynamicAnim(idx, _globals._animationIndexes[1], 1);
			_scene->setDynamicAnim(idx, _globals._animationIndexes[1], 2);

			switch (_globals[kMadameGiryLocation]) {
			case 0:
				_scene->setAnimFrame(_globals._animationIndexes[1], 138);
				_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(62, 54));
				_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(62, 66));
				break;

			case 1:
				if (_globals[kMadameNameIsKnown] >= 1) {
					_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(113, 44));
					_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(107, 66));
				} else
					_scene->_hotspots.activate(NOUN_WOMAN, true);

				break;

			case 2:
				_scene->setAnimFrame(_globals._animationIndexes[1], 273);
				_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(283, 51));
				_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(289, 62));
				break;

			default:
				break;
			}
		}

		if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
			if (_vm->_gameConv->restoreRunning() == 10) {
				int count = 0;

				if (_game._objects.isInInventory(OBJ_RED_FRAME))
					++count;
				if (_game._objects.isInInventory(OBJ_GREEN_FRAME))
					++count;
				if (_game._objects.isInInventory(OBJ_YELLOW_FRAME))
					++count;
				if (_game._objects.isInInventory(OBJ_BLUE_FRAME))
					++count;

				_vm->_gameConv->run(10);
				_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
				_vm->_gameConv->exportValue(_game._difficulty);

				if (count > 2)
					_vm->_gameConv->exportValue(1);
				else
					_vm->_gameConv->exportValue(0);

				_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
				_noConversationHold = true;

				if (_giryStatus == 4)
					_scene->setAnimFrame(_globals._animationIndexes[1], 66);
				else
					_giryStatus = 2;
			} else if (_vm->_gameConv->restoreRunning() == 11) {
				_vm->_gameConv->run(11);
				_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
				_vm->_gameConv->exportValue(0);
				_vm->_gameConv->exportValue(0);
				_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
			}
		}
	}

	if (_scene->_priorSceneId == RETURNING_FROM_LOADING) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

		if (_globals[kJacquesStatus] == 1) {
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 9), 1);
			_anim0ActvFl = true;
			_richardStatus = 3;
			_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, true);

			if (_vm->_gameConv->restoreRunning() == 18) {
				_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
				_richardStatus = 3;
				_vm->_gameConv->run(18);
				_scene->setAnimFrame(_globals._animationIndexes[0], 1);
			}
		}
	}

	if (_scene->_priorSceneId == 206) {
		_game._player._playerPos = Common::Point(37, 64);
		_game._player._facing = FACING_SOUTH;
		_game._player.walk(Common::Point(41, 67), FACING_SOUTH);
		_game._player.setWalkTrigger(90);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		if (_globals[kJacquesStatus] == 1) {
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 9), 1);
			_anim0ActvFl = true;
			_richardStatus = 3;
			_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, true);
		}
	} else if (_scene->_priorSceneId == 207) {
		_game._player._playerPos = Common::Point(263, 59);
		_game._player._facing = FACING_SOUTH;
		_game._player.walk(Common::Point(262, 63), FACING_SOUTH);
		_game._player.setWalkTrigger(95);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else if (_scene->_priorSceneId == 150) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 9), 1);
		_anim0ActvFl = true;
		_richardStatus = 3;
		_game._player._playerPos = Common::Point(132, 112);
		_game._player._facing = FACING_NORTHWEST;
		_globals[kDoorsIn205] = 1;
		_globals[kTicketPeoplePresent] = 0;
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, true);
		_vm->_gameConv->run(18);
	} else if ((_scene->_priorSceneId == 202) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		if (_globals[kJacquesStatus] == 1) {
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('b', 9), 1);
			_anim0ActvFl = true;
			_richardStatus = 3;
			_scene->_hotspots.activate(NOUN_MONSIEUR_RICHARD, true);
		}

		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);

		_game._player.firstWalk(Common::Point(-20, 144), FACING_EAST, Common::Point(19, 144), FACING_NORTHEAST, true);
	}

	sceneEntrySound();
}

void Scene205::step() {
	if (_anim0ActvFl)
		handleRichardAnimation();

	if (_anim1ActvFl)
		handleGiryAnimation();

	if ((_globals[kWalkerConverse] == 2) || (_globals[kWalkerConverse] == 3)) {
			++_conversationCounter;
			if (_conversationCounter > 200)
				_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
	}

	if (_giveTicketFl && !_action.isAction(VERB_GIVE)) {
		_globals[kWalkerConverse] = 0;
		_game._player.walk(Common::Point(_game._player._playerPos.x + 5, _game._player._playerPos.y - 10), FACING_NORTHWEST);
		_game._player.setWalkTrigger(100);
		_giveTicketFl = false;
	}

	switch (_game._trigger) {
	case 100:
		_game._player._visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 5, 2);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
		_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], true);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 102);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 4, 101);
		break;

	case 101:
		_game._objects.setRoom(OBJ_TICKET, NOWHERE);
		_giryStatus = 2;
		break;

	case 102:
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[2]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_vm->_gameConv->release();
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 90:
		_scene->deleteSequence(_globals._sequenceIndexes[0]);
		_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
		break;

	case 91:
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
		_vm->_sound->command(25);
		_game._player._stepEnabled = true;
		break;

	case 95:
		_scene->deleteSequence(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 96);
		break;

	case 96:
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_vm->_sound->command(25);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene205::actions() {
	if (_vm->_gameConv->activeConvId() == 18) {
		handleConversation18();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 10) {
		handleConversation10();
		_action._inProgress = false;
		return;
	}

	if (_vm->_gameConv->activeConvId() == 11) {
		handleConversation11();
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_OPEN, NOUN_BOX_FIVE)) || (_action.isAction(VERB_ENTER, NOUN_BOX_FIVE))) {
		if (_globals[kTicketPeoplePresent] == 2) {
			if (_globals[kMadameGiryLocation] == 2) {
				_vm->_gameConv->run(11);
				_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
				_vm->_gameConv->exportValue(3);
				_vm->_gameConv->exportValue(0);
			} else {
				_vm->_gameConv->run(11);
				_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
				_vm->_gameConv->exportValue(4);
				_vm->_gameConv->exportValue(0);
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_WALK_DOWN_STAIRS_TO, NOUN_GRAND_FOYER)) {
		if (_globals[kTicketPeoplePresent] == 2) {
			if (_globals[kMadameGiryLocation] == 2) {
				_vm->_gameConv->run(11);
				_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
				_vm->_gameConv->exportValue(1);
				_vm->_gameConv->exportValue(0);
			} else {
				_vm->_gameConv->run(11);
				_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
				_vm->_gameConv->exportValue(2);
				_vm->_gameConv->exportValue(0);
			}
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isAction(VERB_ENTER)) || (_action.isAction(VERB_OPEN)) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_LOCK)) {
		if (((_action.isObject(NOUN_BOX_FIVE) && ((_globals[kDoorsIn205] == 0) || (_globals[kDoorsIn205] == 2)))
			|| _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_LOCK))
			|| ((_action.isObject(NOUN_BOX_NINE) && ((_globals[kDoorsIn205] == 0) || (_globals[kDoorsIn205] == 1))))
			|| (_action.isObject(NOUN_BOX_SIX)) || (_action.isObject(NOUN_BOX_SEVEN)) || (_action.isObject(NOUN_BOX_EIGHT))) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_action._inProgress = false;
				return;

			case 1: {
				int idx = _globals._sequenceIndexes[2];
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 4);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[2], SYNC_SEQ, idx);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], false);
				_vm->_sound->command(72);
				_scene->_sequences.addTimer(15, 2);
				_action._inProgress = false;
				return;
				}

			case 2:
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 5, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 4);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				_action._inProgress = false;
				return;

			case 3:
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				if (_action.isAction(VERB_UNLOCK) || _action.isAction(VERB_LOCK))
					_vm->_dialogs->show(20528);
				else
					_vm->_dialogs->show(20527);

				_action._inProgress = false;
				return;

			default:
				break;
			}
		} else if (((_action.isObject(NOUN_BOX_FIVE)) && ((_globals[kDoorsIn205] == 3) || (_globals[kDoorsIn205] == 1)))
			|| ((_action.isObject(NOUN_BOX_NINE)) && ((_globals[kDoorsIn205] == 3) || (_globals[kDoorsIn205] == 2)))) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 5, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[2], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 4, 80);
				_action._inProgress = false;
				return;

			case 80:
				_vm->_sound->command(24);
				if (_action.isObject(NOUN_BOX_FIVE)) {
					_scene->deleteSequence(_globals._sequenceIndexes[0]);
					_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 8, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
				} else if (_action.isObject(NOUN_BOX_NINE)) {
					_scene->deleteSequence(_globals._sequenceIndexes[1]);
					_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, -2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
				}
				_action._inProgress = false;
				return;

			case 81:
				if (_action.isObject(NOUN_BOX_FIVE)) {
					int idx = _globals._sequenceIndexes[0];
					_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
					_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[0], SYNC_SEQ, idx);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
				} else if (_action.isObject(NOUN_BOX_NINE)) {
					int idx = _globals._sequenceIndexes[1];
					_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, -2);
					_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[1], SYNC_SEQ, idx);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
				}
				_action._inProgress = false;
				return;

			case 2:
				_game._player._visible = true;
				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[2]);
				if (_action.isObject(NOUN_BOX_FIVE)) {
					_game._player.walk(Common::Point(37, 64), FACING_NORTH);
					_game._player.setWalkTrigger(3);

				} else if (_action.isObject(NOUN_BOX_NINE)) {
					_game._player.walk(Common::Point(263, 59), FACING_NORTH);
					_game._player.setWalkTrigger(3);
				}
				_action._inProgress = false;
				return;

			case 3:
				if (_action.isObject(NOUN_BOX_FIVE)) {
					_scene->_nextSceneId = 206;
					_globals[kMadameGiryLocation] = 1;
				} else if (_action.isObject(NOUN_BOX_NINE)) {
					_scene->_nextSceneId = 207;
					_globals[kMadameGiryLocation] = 1;
				}
				_action._inProgress = false;
				return;

			default:
				break;
			}
		}
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_RICHARD)) {
		_vm->_gameConv->run(18);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_MADAME_GIRY) || _action.isAction(VERB_TALK_TO, NOUN_WOMAN) || _action.isAction(VERB_GIVE, NOUN_TICKET, NOUN_MADAME_GIRY)) {
		if (_globals[kTicketPeoplePresent] == 2) {
			if ((_globals[kDoorsIn205] == 2) || (_globals[kDoorsIn205] == 3)) {
				if (_globals[kMadameGiryLocation] == 2) {
					_vm->_gameConv->run(11);
					_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
					_vm->_gameConv->exportValue(1);

					if (_action.isAction(VERB_GIVE))
						_vm->_gameConv->exportValue(1);
					else
						_vm->_gameConv->exportValue(0);
				} else {
					_vm->_gameConv->run(11);
					_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
					_vm->_gameConv->exportValue(2);

					if (_action.isAction(VERB_GIVE))
						_vm->_gameConv->exportValue(1);
					else
						_vm->_gameConv->exportValue(0);
				}
			} else {
				_vm->_gameConv->run(11);
				_vm->_gameConv->exportValue(_game._objects.isInInventory(OBJ_TICKET));
				_vm->_gameConv->exportValue(0);

				if (_action.isAction(VERB_GIVE))
					_vm->_gameConv->exportValue(1);
				else
					_vm->_gameConv->exportValue(0);
			}
		} else {
			int count = 0;

			if (_game._objects.isInInventory(OBJ_RED_FRAME))
				++count;
			if (_game._objects.isInInventory(OBJ_GREEN_FRAME))
				++count;
			if (_game._objects.isInInventory(OBJ_YELLOW_FRAME))
				++count;
			if (_game._objects.isInInventory(OBJ_BLUE_FRAME))
				++count;

			_vm->_gameConv->run(10);
			_vm->_gameConv->exportPointer(&_globals[kPlayerScore]);
			_vm->_gameConv->exportValue(_game._difficulty);

			if (count > 2)
				_vm->_gameConv->exportValue(1);
			else
				_vm->_gameConv->exportValue(0);

			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		}
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(20510);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_BOX_SIX)) {
			_vm->_dialogs->show(20511);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOX_SEVEN)) {
			_vm->_dialogs->show(20512);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOX_EIGHT)) {
			_vm->_dialogs->show(20513);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOX_NINE)) {
			if ((_globals[kDoorsIn205] == 0) || (_globals[kDoorsIn205] == 1))
				_vm->_dialogs->show(20516);
			else
				_vm->_dialogs->show(20517);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BOX_FIVE)) {
			if ((_globals[kDoorsIn205] == 0) || (_globals[kDoorsIn205] == 2))
				_vm->_dialogs->show(20514);
			else
				_vm->_dialogs->show(20515);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(20518);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MARBLE_COLUMN)) {
			_vm->_dialogs->show(20519);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(20520);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(20521);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_BUST)) {
			_vm->_dialogs->show(20522);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CARPET)) {
			_vm->_dialogs->show(20523);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_GRAND_FOYER)) {
			_vm->_dialogs->show(20524);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_WOMAN) || _action.isObject(NOUN_MADAME_GIRY)) {
			_vm->_dialogs->show(20525);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_MONSIEUR_RICHARD)) {
			_vm->_dialogs->show(20526);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_TALK_TO, NOUN_BUST)) {
		_vm->_dialogs->show(20529);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_BOX_TEN) || _action.isAction(VERB_ENTER, NOUN_BOX_TEN) || _action.isAction(VERB_LOOK, NOUN_BOX_TEN)) {
		_vm->_dialogs->show(20513);
		_action._inProgress = false;
		return;
	}

	// FIX: the original was doing a | between the two 'objects'
	if (_action.isAction(VERB_TAKE) && (_action.isObject(NOUN_WOMAN) || _action.isObject(NOUN_MADAME_GIRY))) {
		_vm->_dialogs->show(20530);
		_action._inProgress = false;
		return;
	}
}

void Scene205::preActions() {
	if (_action.isObject(NOUN_BOX_FIVE) && (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_OPEN)))
		_game._player.walk(Common::Point(37, 67), FACING_NORTHEAST);

	if (_action.isObject(NOUN_BOX_SIX) && (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_OPEN)))
		_game._player.walk(Common::Point(80, 68), FACING_NORTHEAST);

	if (_action.isObject(NOUN_BOX_SEVEN) && (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_OPEN)))
		_game._player.walk(Common::Point(167, 65), FACING_NORTHEAST);

	if (_action.isObject(NOUN_BOX_EIGHT) && (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_OPEN)))
		_game._player.walk(Common::Point(212, 64), FACING_NORTHEAST);

	if (_action.isObject(NOUN_BOX_NINE) && (_action.isAction(VERB_LOCK) || _action.isAction(VERB_UNLOCK) || _action.isAction(VERB_OPEN)))
		_game._player.walk(Common::Point(258, 63), FACING_NORTHEAST);

	if (_action.isAction(VERB_WALK_DOWN_STAIRS_TO, NOUN_GRAND_FOYER) && (_globals[kDoorsIn205] != 2) && (_globals[kDoorsIn205] != 3)) {
		_game._player._walkOffScreenSceneId = 202;
		_globals[kMadameGiryLocation] = 1;
	}
}

void Scene205::handleConversation18() {
	int interlocutorFl = false;
	int heroFl = false;

	switch (_action._activeAction._verbId) {
	case 0:
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_richardStatus = 0;
		break;

	case 3:
	case 4:
		_vm->_gameConv->setHeroTrigger(64);
		_globals[kRanConvIn205] = true;
		heroFl = true;
		interlocutorFl = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 64:
		_globals[kWalkerConverse] = 0;
		heroFl = true;
		interlocutorFl = true;
		break;

	case 62:
		_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);
		_richardStatus = 0;
		break;

	case 60:
		_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);
		_richardStatus = 3;
		_conversationCounter = 0;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(60);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(62);

	_richardCount = 0;
}

void Scene205::handleConversation10() {
	int interlocutorFl = false;
	int heroFl = false;

	switch (_action._activeAction._verbId) {
	case 9:
		_vm->_gameConv->setInterlocutorTrigger(68);
		interlocutorFl = true;
		_globals[kMadameNameIsKnown] = 1;
		_scene->_hotspots.activate(NOUN_WOMAN, false);
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(113, 44));
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(107, 66));
		break;

	case 10:
		_globals[kMadameNameIsKnown] = 1;
		_scene->_hotspots.activate(NOUN_WOMAN, false);
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(113, 44));
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(107, 66));
		break;

	case 7:
		if (!_game._trigger) {
			_giryStatus = 6;
			_globals[kMadameNameIsKnown] = 2;
			_globals[kMadameGiryLocation] = 0;
			_scene->changeVariant(2);
			_scene->_rails.disableNode(6);
			_scene->_rails.disableNode(7);
			_scene->_rails.disableNode(8);
			_scene->_rails.disableNode(9);
			_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, false, Common::Point(113, 44));
			_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, false, Common::Point(107, 66));
			interlocutorFl = true;
			heroFl = true;
			if (_globals[kDoorsIn205] == 0)
				_globals[kDoorsIn205] = 1;
			else if (_globals[kDoorsIn205] == 2)
				_globals[kDoorsIn205] = 3;
		}
		break;

	case 13:
	case 45:
		_vm->_gameConv->setInterlocutorTrigger(70);
		interlocutorFl = true;
		break;

	case 21:
		if (!_game._trigger && !_noConversationHold) {
			_vm->_gameConv->hold();
			_giryStatus = 4;
		} else
			_noConversationHold = false;

		break;

	case 17:
		_vm->_gameConv->setInterlocutorTrigger(72);
		interlocutorFl = true;
		break;

	case 11:
		if (!_game._trigger) {
			_vm->_gameConv->setInterlocutorTrigger(74);
		}
		heroFl = true;
		interlocutorFl = true;
		break;

	case 23:
	case 25:
		if ((!_game._trigger) && (_giryStatus == 4)) {
			_vm->_gameConv->hold();
			_giryStatus = 0;
		}
		break;

	case 4:
	case 5:
	case 8:
	case 14:
	case 16:
	case 19:
	case 40:
	case 46:
		_vm->_gameConv->setInterlocutorTrigger(64);
		interlocutorFl = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 64:
		switch (_action._activeAction._verbId) {
		case 5:
		case 14:
		case 16:
		case 19:
		case 40:
		case 46:
			_giryStatus = 0;
			break;
		default:
			break;
		}
		_globals[kWalkerConverse] = 0;
		heroFl = true;
		interlocutorFl = true;
		break;

	case 68:
		_giryStatus = 5;
		break;

	case 74:
		_giryStatus = 8;
		_globals[kWalkerConverse] = 0;
		heroFl = true;
		interlocutorFl = true;
		break;

	case 72:
		_giryStatus = 3;
		break;

	case 70:
		if (_action._activeAction._verbId == 13) {
			_globals[kWalkerConverse] = 0;
			heroFl = true;
			interlocutorFl = true;
		}
		_giryStatus = 1;
		break;

	case 66:
		if (_globals[kWalkerConverse] != 0)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);

		if ((_giryStatus != 4) && (_giryStatus != 6) && (_giryStatus != 7))
			_giryStatus = 0;
		break;

	case 60:
		if (_globals[kWalkerConverse] != 0)
			_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);

		if ((_giryStatus != 4) && (_giryStatus != 6) && (_giryStatus != 7))
			_giryStatus = 2;

		_conversationCounter = 0;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(60);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(66);

	_giryCount = 0;
}

void Scene205::handleConversation11() {
	int interlocutorFl = false;
	int heroFl = false;

	switch (_action._activeAction._verbId) {
	case 5:
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_giryStatus = 9;
		}
		break;

	case 8:
		if (!_game._trigger) {
			_vm->_gameConv->hold();
			_giryStatus = 7;
			_vm->_gameConv->setInterlocutorTrigger(64);
			_game._player.walk(Common::Point(225,79), FACING_NORTHEAST);
			interlocutorFl = true;
			_globals[kMadameGiryLocation] = 2;
			_scene->changeVariant(4);
			_scene->_rails.disableNode(6);
			_scene->_rails.disableNode(7);
			_scene->_rails.disableNode(8);
			_scene->_rails.disableNode(9);
			_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, false, Common::Point(113, 44));
			_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, false, Common::Point(107, 66));
			if (_globals[kDoorsIn205] == 0)
				_globals[kDoorsIn205] = 2;
			else if (_globals[kDoorsIn205] == 1)
				_globals[kDoorsIn205] = 3;
		}
		break;

	case 9:
	case 10:
	case 12:
	case 13:
	case 14:
		_vm->_gameConv->setInterlocutorTrigger(64);
		interlocutorFl = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 64:
		switch (_action._activeAction._verbId) {
		case 6:
		case 10:
		case 12:
		case 13:
		case 14:
			_giryStatus = 0;
			break;
		default:
			break;
		}
		_globals[kWalkerConverse] = 0;
		heroFl = true;
		interlocutorFl = true;
		break;

	case 110:
		_vm->_gameConv->hold();
		_giryStatus = 7;
		break;

	case 66:
		if (_globals[kWalkerConverse] != 0)
			_globals[kWalkerConverse] = _vm->getRandomNumber(1, 4);

		if (_giryStatus != 9)
			_giryStatus = 0;
		break;

	case 60:
		if (_globals[kWalkerConverse] != 0)
			_globals[kWalkerConverse] = _vm->getRandomNumber(2, 3);

		if (_giryStatus != 9)
			_giryStatus = 2;

		_conversationCounter = 0;
		break;

	default:
		break;
	}

	if (!heroFl)
		_vm->_gameConv->setHeroTrigger(60);

	if (!interlocutorFl)
		_vm->_gameConv->setInterlocutorTrigger(66);

	_giryCount = 0;
}

void Scene205::handleRichardAnimation() {
	if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == _richardFrame)
		return;

	_richardFrame = _scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame();
	int random;
	int resetFrame = -1;

	switch (_richardFrame) {
	case 1:
	case 2:
	case 3:
	case 11:
	case 19:
	case 35:
	case 47:
	case 57:
	case 69:
		switch (_richardStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 3);
			++_richardCount;
			if (_richardCount > 30) {
				_richardStatus = 3;
				random = 9;
			}
			break;

		case 1:
			random = 4;
			_richardStatus = 0;
			break;

		case 2:
			random = 6;
			break;

		case 4:
			random = 5;
			_richardStatus = 0;
			break;

		default:
			random = _vm->getRandomNumber(7, 50);
			while (_lastRandom == random)
				random = _vm->getRandomNumber(7, 50);

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
			resetFrame = 11;
			break;

		case 5:
			resetFrame = 3;
			break;

		case 6:
			resetFrame = 57;
			break;

		case 7:
			resetFrame = 23;
			break;

		case 8:
			resetFrame = 19;
			break;

		case 9:
			resetFrame = 21;
			break;

		case 10:
			resetFrame = 25;
			break;

		case 11:
			resetFrame = 35;
			break;

		case 12:
			resetFrame = 47;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 30:
		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 4:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			break;
		}

		if (random == 1)
			resetFrame = 30;
		else
			resetFrame = 29;

		break;

	case 24:
		switch (_richardStatus) {
		case 1:
		case 2:
		case 4:
		case 0:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 30);
			break;
		}

		if (random == 1)
			resetFrame = 0;
		else
			resetFrame = 23;

		break;

	case 20:
		switch (_richardStatus) {
		case 1:
		case 2:
		case 4:
		case 0:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			break;
		}

		if (random == 1)
			resetFrame = 0;
		else
			resetFrame = 19;

		break;

	case 22:
		switch (_richardStatus) {
		case 1:
		case 2:
		case 4:
		case 0:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			break;
		}

		if (random == 1)
			resetFrame = 0;
		else
			resetFrame = 21;

		break;

	case 41:
		switch (_richardStatus) {
		case 1:
		case 2:
		case 4:
		case 0:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			break;
		}

		if (random == 1)
			resetFrame = 41;
		else
			resetFrame = 40;

		break;

	case 52:
		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 4:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			break;
		}

		if (random == 1)
			resetFrame = 52;
		else
			resetFrame = 51;

		break;

	case 65:
		switch (_richardStatus) {
		case 0:
		case 1:
		case 2:
		case 4:
			random = 1;
			break;

		default:
			random = _vm->getRandomNumber(1, 50);
			break;
		}

		if (random == 1)
			resetFrame = 65;
		else
			resetFrame = 64;

		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[0], resetFrame);
		_richardFrame = resetFrame;
	}
}

void Scene205::handleGiryAnimation() {
	if (_scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame() == _giryFrame)
		return;

	_giryFrame = _scene->_animation[_globals._animationIndexes[1]]->getCurrentFrame();
	int random;
	int resetFrame = -1;

	switch (_giryFrame) {
	case 77:
		_vm->_gameConv->release();
		break;

	case 1:
	case 2:
	case 3:
	case 44:
	case 14:
	case 21:
	case 35:
	case 56:
	case 78:
	case 284:
		switch (_giryStatus) {
		case 0:
			random = _vm->getRandomNumber(1, 3);
			++_giryCount;
			if (_giryCount > 30) {
				_giryStatus = 2;
				random = 100;
			}
			break;

		case 8:
			random = 4;
			_giryStatus = 0;
			break;

		case 3:
			random = 5;
			_giryStatus = 0;
			break;

		case 1:
			random = 6;
			break;

		case 5:
			_giryStatus = 0;
			random = 7;
			break;

		case 4:
			random = 8;
			break;

		case 6:
			random = 9;
			_giryStatus = 2;
			break;

		case 7:
			random = 10;
			_giryStatus = 2;
			break;

		case 9:
			random = 11;
			break;

		default:
			random = _vm->getRandomNumber(12, 100);
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
			resetFrame = 16;
			break;

		case 6:
			resetFrame = 21;
			break;

		case 7:
			resetFrame = 44;
			break;

		case 8:
			resetFrame = 56;
			break;

		case 9:
			resetFrame = 78;
			_vm->_gameConv->hold();
			break;

		case 10:
			resetFrame = 140;
			break;

		case 11:
			resetFrame = 276;
			break;

		case 12:
			resetFrame = 35;
			break;

		default:
			resetFrame = 0;
			break;
		}
		break;

	case 27:
	case 28:
	case 29:
		switch (_giryStatus) {
		case 0:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			random = 4;
			break;

		default:
			random = _vm->getRandomNumber(1, 3);
			++_giryCount;
			if (_giryCount > 30) {
				_giryStatus = 2;
				random = 100;
			}
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 26;
			break;

		case 2:
			resetFrame = 27;
			break;

		case 3:
			resetFrame = 28;
			break;

		default:
			resetFrame = 29;
			break;
		}
		break;

	case 265:
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(283, 51));
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(289, 62));
		_vm->_gameConv->release();
		break;

	case 274:
	case 275:
	case 276:
		if (_giryStatus == 0) {
			random = _vm->getRandomNumber(1, 3);
			++_giryCount;
			if (_giryCount > 30) {
				_giryStatus = 2;
				random = 100;
			}
		} else
			random = 100;

		switch (random) {
		case 1:
			resetFrame = 273;
			break;

		case 2:
			resetFrame = 274;
			break;

		case 3:
			resetFrame = 275;
			break;

		default:
			resetFrame = 273;
			break;
		}
		break;

	case 85:
		_vm->_gameConv->release();
		break;

	case 110:
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(62, 54));
		_scene->_hotspots.activateAtPos(NOUN_MADAME_GIRY, true, Common::Point(62, 66));
		break;

	case 138:
	case 139:
	case 140:
		switch (_giryStatus) {
		case 0:
		case 1:
			random = _vm->getRandomNumber(1, 3);
			++_giryCount;
			if (_giryCount > 30) {
				_giryStatus = 2;
				random = 100;
			}
			break;

		default:
			random = 100;
			break;
		}

		switch (random) {
		case 1:
			resetFrame = 137;
			break;

		case 2:
			resetFrame = 138;
			break;

		case 3:
			resetFrame = 139;
			break;

		default:
			resetFrame = 137;
			break;
		}
		break;

	case 66:
		_vm->_gameConv->release();
		break;

	case 67:
		switch (_giryStatus) {
		case 0:
		case 9:
		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
			random = 1;
			break;

		default:
			random = 100;
			break;
		}

		if (random == 1)
			resetFrame = 67;
		else
			resetFrame = 66;

		break;

	case 8:
	case 9:
		switch (_giryStatus) {
		case 0:
		case 9:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			random = 1;
			break;

		default:
			random = 100;
			break;
		}

		if (random == 1)
			resetFrame = 9;
		else
			resetFrame = 8;

		break;

	case 280:
		_giveTicketFl = true;
		break;

	case 281:
		switch (_giryStatus) {
		case 0:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			random = 1;
			break;

		default:
			random = 100;
			break;
		}

		if (random == 1)
			resetFrame = 281;
		else
			resetFrame = 280;

		break;

	default:
		break;
	}

	if (resetFrame >= 0) {
		_scene->setAnimFrame(_globals._animationIndexes[1], resetFrame);
		_giryFrame = resetFrame;
	}
}

/*------------------------------------------------------------------------*/

Scene206::Scene206(MADSEngine *vm) : Scene2xx(vm) {
	_anim0ActvFl = false;
	_skip1Fl = false;
	_skip2Fl = false;
}

void Scene206::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_anim0ActvFl);
	s.syncAsByte(_skip1Fl);
	s.syncAsByte(_skip2Fl);
}

void Scene206::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene206::enter() {
	_anim0ActvFl = false;
	_skip1Fl = false;
	_skip2Fl = false;

	_scene->loadSpeech(1);
	_vm->_gameConv->load(26);

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RDR_9");

	if (_scene->_priorSceneId != 308) {
		if (_globals[kPanelIn206] == 0) {
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		} else {
			_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 3);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		}
	}

	if (_globals[kPanelIn206] == 0) {
		_scene->_hotspots.activate(NOUN_PANEL, false);
		_scene->_hotspots.activate(NOUN_KEYHOLE, false);
	}

	if (_game._objects.isInRoom(OBJ_CRUMPLED_NOTE)) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('p', 0));
		_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 10);
	} else
		_scene->_hotspots.activate(NOUN_CRUMPLED_NOTE, false);

	if (_globals[kTrapDoorStatus] == 0) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	}

	if (_globals[kRightDoorIsOpen504] && !_globals[kKnockedOverHead]) {
		_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('h', 1), 88);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_anim0ActvFl = true;
	} else if (_scene->_priorSceneId == 308) {
		_game._player._playerPos = Common::Point(67, 127);
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(108, 137), FACING_EAST);
		_game._player.setWalkTrigger(82);
	} else if ((_scene->_priorSceneId == 205) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(153, 148);
		_game._player._facing = FACING_NORTH;
	}

	sceneEntrySound();
}

void Scene206::step() {
	switch (_game._trigger) {
	case 82:
		_vm->_sound->command(24);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
		break;

	case 83:
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		_game._player._stepEnabled = true;
		break;

	case 88:
		_game._player._playerPos = Common::Point(168, 138);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_anim0ActvFl = false;
		_game._player.resetFacing(FACING_WEST);
		break;

	default:
		break;
	}

	if (_anim0ActvFl && !_skip1Fl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 118) {
			_globals[kKnockedOverHead] = true;
			_skip1Fl = true;
			_scene->_sequences.addTimer(1, 84);
		}
	}

	if (_anim0ActvFl && !_skip2Fl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 61) {
			if (_vm->_sound->_preferRoland)
				_vm->_sound->command(65);
			else
				_scene->playSpeech(1);

			_skip2Fl = true;
		}
	}

	switch (_game._trigger) {
	case 84:
		_vm->_sound->command(24);
		_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 8, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 85);
		break;

	case 85:
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		break;

	default:
		break;
	}
}

void Scene206::actions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_LOGE_CORRIDOR)) {
		if (_globals[kRightDoorIsOpen504]) {
			if (_vm->_sound->_preferRoland)
				_vm->_sound->command(65);
			else
				_scene->playSpeech(1);

			_vm->_gameConv->run(26);
			_vm->_gameConv->exportValue(4);
		} else
			_scene->_nextSceneId = 205;

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_CRUMPLED_NOTE) && (_game._objects.isInRoom(OBJ_CRUMPLED_NOTE) || _game._trigger)) {
		switch (_game._trigger) {
		case (0):
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 5, 2);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, -2);
			_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 4, 77);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 78);
			_action._inProgress = false;
			return;

		case 77:
			_scene->deleteSequence(_globals._sequenceIndexes[4]);
			_scene->_hotspots.activate(NOUN_CRUMPLED_NOTE, false);
			_game._objects.addToInventory(OBJ_CRUMPLED_NOTE);
			_vm->_sound->command(26);
			_action._inProgress = false;
			return;

		case 78:
			_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[5]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 79);
			_action._inProgress = false;
			return;

		case 79:
			_vm->_dialogs->showItem(OBJ_CRUMPLED_NOTE, 816, 6);
			_game._player._stepEnabled = true;
			_action._inProgress = false;
			return;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_WALK_BEHIND, NOUN_PANEL) || _action.isAction(VERB_OPEN, NOUN_PANEL)
	 || ((_game._trigger >= 70) && (_game._trigger < 77))) {
		if (_globals[kPanelIn206] == 3) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 7, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 70);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
				break;

			case 70:
				_vm->_sound->command(24);
				_scene->deleteSequence(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
				break;

			case 71:
				_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
				_game._player._stepEnabled = true;
				_game._player._visible = true;
				_game._player.walk(Common::Point(67, 127), FACING_NORTHWEST);
				_game._player.setWalkTrigger(72);
				break;

			case 72:
				_vm->_sound->command(24);
				_scene->deleteSequence(_globals._sequenceIndexes[3]);
				_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 8, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
				break;

			case 73:
				_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
				_scene->_nextSceneId = 308;
				break;

			default:
				break;
			}
		} else {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 7, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 75);
				break;

			case 74:
				_game._player._stepEnabled = true;
				_game._player._visible = true;
				_vm->_dialogs->show(20625);
				break;

			case 75:
				_vm->_sound->command(72);
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_UNLOCK, NOUN_PANEL) || _action.isAction(VERB_LOCK, NOUN_PANEL) || _action.isAction(VERB_PUT, NOUN_KEY, NOUN_KEYHOLE)
	 || _action.isAction(VERB_UNLOCK, NOUN_KEYHOLE) || _action.isAction(VERB_LOCK, NOUN_KEYHOLE) || (_game._trigger >= 64)) {
		if (_globals[kPanelIn206] >= 1) {
			switch (_game._trigger) {
			case (0):
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], true, 7, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], true);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 64);
				break;

			case 64: {
				_vm->_sound->command(71);
				int idx = _globals._sequenceIndexes[5];
				_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], true, -2);
				_game.syncTimers(SYNC_SEQ, _globals._sequenceIndexes[5], SYNC_SEQ, idx);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], false);
				_scene->_sequences.addTimer(30, 65);
				}
				break;

			case 65:
				_scene->deleteSequence(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[5] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[5], true, 7, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
				_scene->_sequences.setSeqPlayer(_globals._sequenceIndexes[5], false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
				break;

			case 66:
				if (_action.isAction(VERB_LOCK)) {
					_globals[kPanelIn206] = 2;
					_vm->_dialogs->show(20629);
				} else if (_action.isAction(VERB_UNLOCK)) {
					_globals[kPanelIn206] = 3;
					_vm->_dialogs->show(20628);
				}

				if (_action.isAction(VERB_PUT)) {
					if (_globals[kPanelIn206] <= 2) {
						_globals[kPanelIn206] = 3;
						_vm->_dialogs->show(20628);
					} else {
						_globals[kPanelIn206] = 2;
						_vm->_dialogs->show(20629);
					}
				}

				_game.syncTimers(SYNC_PLAYER, 0, SYNC_SEQ, _globals._sequenceIndexes[5]);
				_game._player._stepEnabled = true;
				_game._player._visible = true;
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(20610);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(20611);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(20612);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RIGHT_COLUMN)) {
			_vm->_dialogs->show(20614);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_KEYHOLE)) {
			_vm->_dialogs->show(20615);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RAIL)) {
			_vm->_dialogs->show(20616);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SEAT)) {
			_vm->_dialogs->show(20617);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LOGE_CORRIDOR)) {
			_vm->_dialogs->show(20618);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE)) {
			if (_globals[kJacquesStatus])
				_vm->_dialogs->show(20630);
			else
				_vm->_dialogs->show(20619);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOUSE)) {
			_vm->_dialogs->show(20620);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(20621);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOUSE_LIGHT)) {
			_vm->_dialogs->show(20622);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_PANEL)) {
			if (_globals[kPanelIn206] == 3)
				_vm->_dialogs->show(20624);
			else
				_vm->_dialogs->show(20626);

			_action._inProgress = false;
			return;
		}

		if ((_action.isObject(NOUN_LEFT_COLUMN)) && (!_game._trigger)) {
			if (_globals[kPanelIn206] == 0) {
				_vm->_dialogs->show(20613);
				_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('k', 1), 95);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
			} else
				_vm->_dialogs->show(20623);

			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CRUMPLED_NOTE) && _game._objects.isInRoom(OBJ_CRUMPLED_NOTE)) {
			_vm->_dialogs->show(20627);
			_action._inProgress = false;
			return;
		}
	}

	switch (_game._trigger) {
	case 95:
		_game._player._visible = true;
		_game.syncTimers(SYNC_PLAYER, 0, SYNC_ANIM, _globals._animationIndexes[0]);
		_scene->_hotspots.activate(NOUN_PANEL, true);
		_scene->_hotspots.activate(NOUN_KEYHOLE, true);
		_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		if (!(_globals[kPlayerScoreFlags] & 8)) {
			_globals[kPlayerScoreFlags] |= 8;
			_globals[kPlayerScore] += 5;
		}
		_scene->_sequences.addTimer(15, 96);

		_action._inProgress = false;
		return;

	case 96:
		_game._player._stepEnabled = true;
		_globals[kPanelIn206] = 1;
		_vm->_dialogs->show(20623);
		_action._inProgress = false;
		return;

	default:
		break;
	}
}

void Scene206::preActions() {
	if (_action.isAction(VERB_LOOK, NOUN_LEFT_COLUMN))
		_game._player._needToWalk = true;

	if (_action.isAction(VERB_UNLOCK, NOUN_PANEL) || _action.isAction(VERB_WALK_BEHIND, NOUN_PANEL)
	 || _action.isAction(VERB_LOCK, NOUN_PANEL) || _action.isAction(VERB_OPEN, NOUN_PANEL))
		_game._player.walk(Common::Point(108, 137), FACING_NORTHWEST);

	if (_action.isObject(NOUN_LEFT_COLUMN) && (_globals[kPanelIn206] == 0))
		_game._player.walk(Common::Point(103, 137), FACING_NORTHWEST);
}

/*------------------------------------------------------------------------*/

Scene207::Scene207(MADSEngine *vm) : Scene2xx(vm) {
	_skip1Fl = false;
	_anim0ActvFl = false;
}

void Scene207::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_skip1Fl);
	s.syncAsByte(_anim0ActvFl);
}

void Scene207::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene207::enter() {
	_scene->loadSpeech(2);
	_skip1Fl = false;
	_anim0ActvFl = false;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));

	_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);

	if ((_scene->_priorSceneId == 205) || (_scene->_priorSceneId != RETURNING_FROM_LOADING)) {
		_game._player._playerPos = Common::Point(159, 147);
		_game._player._facing = FACING_NORTH;
	}

	sceneEntrySound();
}

void Scene207::step() {
	if (_anim0ActvFl && !_skip1Fl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 6) {
			if (_vm->_sound->_preferRoland)
				_vm->_sound->command(69);
			else
				_scene->playSpeech(2);

			_skip1Fl = true;
		}
	}
}

void Scene207::actions() {
	if (_action.isAction(VERB_TAKE, NOUN_SEAT)) {
		switch (_game._trigger) {
		case 0:
			_globals[kPlayerScore] += 5;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_vm->_sound->command(3);
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 7, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 10);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -1, -2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 10);
			_scene->_sequences.addTimer(120, 2);
			_scene->_sequences.addTimer(240, 3);
			break;

		case 2:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 3:
			_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('s', 1), 5);
			_anim0ActvFl = true;
			_scene->deleteSequence(_globals._sequenceIndexes[1]);
			break;

		case 4:
			_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			break;

		case 5:
			_scene->_nextSceneId = 208;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_EXIT_TO, NOUN_LOGE_CORRIDOR)) {
		_scene->_nextSceneId = 205;
		_action._inProgress = false;
		return;
	}

	if (_action._lookFlag) {
		_vm->_dialogs->show(20710);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_WALL)) {
			_vm->_dialogs->show(20711);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_FLOOR)) {
			_vm->_dialogs->show(20712);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LEFT_COLUMN)) {
			_vm->_dialogs->show(20713);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RIGHT_COLUMN)) {
			_vm->_dialogs->show(20714);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_RAIL)) {
			_vm->_dialogs->show(20715);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_SEAT)) {
			_vm->_dialogs->show(20716);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_LOGE_CORRIDOR)) {
			_vm->_dialogs->show(20717);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_STAGE)) {
			_vm->_dialogs->show(20718);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOUSE)) {
			_vm->_dialogs->show(20719);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_CEILING)) {
			_vm->_dialogs->show(20720);
			_action._inProgress = false;
			return;
		}

		if (_action.isObject(NOUN_HOUSE_LIGHT)) {
			_vm->_dialogs->show(20721);
			_action._inProgress = false;
			return;
		}
	}
}

void Scene207::preActions() {
	if (_action.isAction(VERB_TAKE, NOUN_SEAT))
		_game._player.walk(Common::Point(139, 124), FACING_NORTH);
}

/*------------------------------------------------------------------------*/

Scene208::Scene208(MADSEngine *vm) : Scene2xx(vm) {
	_skip1Fl = false;
	_skip2Fl = false;

	_topLeftPeopleFrame = -1;
	_topRightPeopleFrame = -1;
	_middleLeftPeopleFrame = -1;
	_centerPeopleFrame = -1;
	_middleRightPeopleFrame = -1;
	_bottomLeftPeopleFrame = -1;
	_bottomMiddlePeopleFrame = -1;
	_bottomRightPeopleFrame = -1;
	_direction = -1;
}

void Scene208::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);

	s.syncAsByte(_skip1Fl);
	s.syncAsByte(_skip2Fl);

	s.syncAsSint16LE(_topLeftPeopleFrame);
	s.syncAsSint16LE(_topRightPeopleFrame);
	s.syncAsSint16LE(_middleLeftPeopleFrame);
	s.syncAsSint16LE(_centerPeopleFrame);
	s.syncAsSint16LE(_middleRightPeopleFrame);
	s.syncAsSint16LE(_bottomLeftPeopleFrame);
	s.syncAsSint16LE(_bottomMiddlePeopleFrame);
	s.syncAsSint16LE(_bottomRightPeopleFrame);
	s.syncAsSint16LE(_direction);
}

void Scene208::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene208::enter() {
	_scene->loadSpeech(1);

	_skip1Fl = false;
	_skip2Fl = false;

	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 6));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('x', 7));

	_scene->_userInterface.emptyConversationList();
	_scene->_userInterface.setup(kInputConversation);

	_scene->loadSpeech(1);

	_game._player._stepEnabled = false;
	_game._player._visible = false;

	_topLeftPeopleFrame = 2;
	_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, _topLeftPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
	_scene->_sequences.addTimer(120, 60);

	_topRightPeopleFrame = 2;
	_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, _topRightPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	_scene->_sequences.addTimer(30, 62);

	_middleLeftPeopleFrame = 2;
	_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, _middleLeftPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
	_scene->_sequences.addTimer(30, 64);

	_centerPeopleFrame = 1;
	_direction = 1;
	_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, _centerPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
	_scene->_sequences.addTimer(300, 66);

	_middleRightPeopleFrame = 3;
	_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, _middleRightPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
	_scene->_sequences.addTimer(60, 68);

	_bottomLeftPeopleFrame = 4;
	_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, _bottomLeftPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
	_scene->_sequences.addTimer(60, 70);

	_bottomMiddlePeopleFrame = 4;
	_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, _bottomMiddlePeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
	_scene->_sequences.addTimer(30, 72);

	_bottomRightPeopleFrame = 3;
	_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, _bottomRightPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
	_scene->_sequences.addTimer(15, 74);

	_globals._animationIndexes[0] = _scene->loadAnimation(formAnimName('p', 1), 80);

	sceneEntrySound();
}

void Scene208::step() {
	animateTopLeftPeople();
	animateTopRightPeople();
	animateMiddleLeftPeople();
	animateCenterPeople();
	animateMiddleRightPeople();
	animateBottomLeftPeople();
	animateBottomMiddlePeople();
	animateBottomRightPeople();

	if (!_skip1Fl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 49) {
			if (_vm->_sound->_preferRoland)
				_vm->_sound->command(65);
			else
				_scene->playSpeech(1);

			_skip1Fl = true;
		}
	}

	if (!_skip2Fl) {
		if (_scene->_animation[_globals._animationIndexes[0]]->getCurrentFrame() == 68) {
			if (_vm->_sound->_preferRoland)
				_vm->_sound->command(65);
			else
				_scene->playSpeech(1);

			_skip2Fl = true;
		}
	}

	if (_game._trigger == 80)
		_scene->_nextSceneId = 150;
}

void Scene208::actions() {
}

void Scene208::preActions() {
}

void Scene208::animateTopLeftPeople() {

	if (_game._trigger != 60)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[0]);

	int triggerVal;
	int rndVal = _vm->getRandomNumber(1, 2);
	if (_topLeftPeopleFrame == 3)
		triggerVal = 1;
	else
		triggerVal = rndVal;

	if (rndVal == triggerVal) {
		_topLeftPeopleFrame += _vm->getRandomNumber(-1, 1);
		if (_topLeftPeopleFrame == 0)
			_topLeftPeopleFrame = 1;
		else if (_topLeftPeopleFrame == 4)
			_topLeftPeopleFrame = 3;
	}

	_globals._sequenceIndexes[0] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[0], false, _topLeftPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
	int delay = _vm->getRandomNumber(15, 60);
	_scene->_sequences.addTimer(delay, 60);
}

void Scene208::animateTopRightPeople() {
	if (_game._trigger != 62)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[1]);

	int triggerVal;
	int rndVal = _vm->getRandomNumber(1, 2);
	if (_topRightPeopleFrame == 4)
		triggerVal = 1;
	else
		triggerVal = rndVal;

	if (rndVal == triggerVal) {
		_topRightPeopleFrame += _vm->getRandomNumber(-1, 1);
		if (_topRightPeopleFrame == 0)
			_topRightPeopleFrame = 1;
		else if (_topRightPeopleFrame == 5)
			_topRightPeopleFrame = 4;
	}

	_globals._sequenceIndexes[1] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[1], false, _topRightPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	int delay = _vm->getRandomNumber(15, 60);
	_scene->_sequences.addTimer(delay, 62);
}

void Scene208::animateMiddleLeftPeople() {
	if (_game._trigger != 64)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[2]);
	int delay = _vm->getRandomNumber(60, 120);

	int triggerVal;
	int rndVal = _vm->getRandomNumber(1, 2);
	if (_middleLeftPeopleFrame == 2)
		triggerVal = 1;
	else
		triggerVal = rndVal;

	if (rndVal == triggerVal) {
		_middleLeftPeopleFrame += _vm->getRandomNumber(-1, 1);
		if (_middleLeftPeopleFrame == 0)
			_middleLeftPeopleFrame = 1;
		else if (_middleLeftPeopleFrame == 5)
			_middleLeftPeopleFrame = 4;
	}

	if ((_centerPeopleFrame == 3) && (_middleLeftPeopleFrame < 4)) {
		++_middleLeftPeopleFrame;
		delay = 10;
	}

	_globals._sequenceIndexes[2] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[2], false, _middleLeftPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
	_scene->_sequences.addTimer(delay, 64);
}


void Scene208::animateCenterPeople() {
	if (_game._trigger != 66)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[3]);

	int delay;

	if (_direction) {
		++_centerPeopleFrame;
		delay = 15;
		if (_centerPeopleFrame == 4) {
			delay = _vm->getRandomNumber(300, 420);
			--_centerPeopleFrame;
			_direction = 0;
		}
	} else {
		--_centerPeopleFrame;
		delay = 15;
		if (_centerPeopleFrame == 0) {
			delay = _vm->getRandomNumber(600, 900);
			++_centerPeopleFrame;
			_direction = 1;
		}
	}

	_globals._sequenceIndexes[3] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[3], false, _centerPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);
	_scene->_sequences.addTimer(delay, 66);
}

void Scene208::animateMiddleRightPeople() {
	if (_game._trigger != 68)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[4]);

	int triggerVal;
	int rndVal = _vm->getRandomNumber(1, 2);
	if (_middleRightPeopleFrame == 1)
		triggerVal = 1;
	else
		triggerVal = rndVal;

	if (rndVal == triggerVal) {
		_middleRightPeopleFrame += _vm->getRandomNumber(-1, 1);
		if (_middleRightPeopleFrame == 0)
			_middleRightPeopleFrame = 1;
		else if (_middleRightPeopleFrame == 4)
			_middleRightPeopleFrame = 3;

		if (_centerPeopleFrame == 3)
			_middleRightPeopleFrame = 3;
	}

	_globals._sequenceIndexes[4] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[4], false, _middleRightPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
	int delay = _vm->getRandomNumber(15, 60);
	_scene->_sequences.addTimer(delay, 68);
}

void Scene208::animateBottomLeftPeople() {
	if (_game._trigger != 70)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[5]);

	int triggerVal;
	int rndVal = _vm->getRandomNumber(1, 2);
	if (_bottomLeftPeopleFrame == 4)
		triggerVal = 1;
	else
		triggerVal = rndVal;

	if (rndVal == triggerVal) {
		_bottomLeftPeopleFrame += _vm->getRandomNumber(-1, 1);
		if (_bottomLeftPeopleFrame == 0)
			_bottomLeftPeopleFrame = 1;
		else if (_bottomLeftPeopleFrame == 5)
			_bottomLeftPeopleFrame = 4;
	}

	_globals._sequenceIndexes[5] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[5], false, _bottomLeftPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
	int delay = _vm->getRandomNumber(15, 60);
	_scene->_sequences.addTimer(delay, 70);
}

void Scene208::animateBottomMiddlePeople() {
	if (_game._trigger != 72)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[6]);

	int triggerVal;
	int rndVal = _vm->getRandomNumber(1, 2);
	if (_bottomMiddlePeopleFrame == 4)
		triggerVal = 1;
	else
		triggerVal = rndVal;

	if (rndVal == triggerVal) {
		_bottomMiddlePeopleFrame += _vm->getRandomNumber(-1, 1);
		if (_bottomMiddlePeopleFrame == 0)
			_bottomMiddlePeopleFrame = 1;
		else if (_bottomMiddlePeopleFrame == 5)
			_bottomMiddlePeopleFrame = 4;
	}

	_globals._sequenceIndexes[6] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[6], false, _bottomMiddlePeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
	int delay = _vm->getRandomNumber(15, 60);
	_scene->_sequences.addTimer(delay, 72);
}

void Scene208::animateBottomRightPeople() {
	if (_game._trigger != 74)
		return;

	_scene->deleteSequence(_globals._sequenceIndexes[7]);

	int triggerVal;
	int rndVal = _vm->getRandomNumber(1, 2);
	if ((_bottomRightPeopleFrame == 3) || (_bottomRightPeopleFrame == 1))
		triggerVal = 1;
	else
		triggerVal = rndVal;

	if (rndVal == triggerVal) {
		_bottomRightPeopleFrame += _vm->getRandomNumber(-1, 1);
		if (_bottomRightPeopleFrame == 0)
			_bottomRightPeopleFrame = 1;
		else if (_bottomRightPeopleFrame == 4)
			_bottomRightPeopleFrame = 3;
	}

	_globals._sequenceIndexes[7] = _scene->_sequences.addStampCycle(_globals._spriteIndexes[7], false, _bottomRightPeopleFrame);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
	int delay = _vm->getRandomNumber(15, 60);
	_scene->_sequences.addTimer(delay, 74);
}

/*------------------------------------------------------------------------*/

Scene250::Scene250(MADSEngine *vm) : Scene2xx(vm) {
}

void Scene250::synchronize(Common::Serializer &s) {
	Scene2xx::synchronize(s);
}

void Scene250::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene250::enter() {
	warning("TODO: Switch to letter box view. See definition of MADS_MENU_Y");

	_game._player._stepEnabled = false;
	_game._player._visible    = false;

	_game.loadQuoteSet(0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0);
	_globals._animationIndexes[0] = _scene->loadAnimation("*RM150Q1", 1);
}

void Scene250::step() {
	if (_game._trigger == 1)
		_scene->_sequences.addTimer(12, 2);

	if (_game._trigger == 2) {
		int y = 68;

		_scene->_kernelMessages.add(Common::Point(160, y), 0x1110, 0x20, 0, 900, _game.getQuote(0x36));
		y += 16;

		if (_globals[kPlayerScore] > 250)
			_globals[kPlayerScore] = 250;

		Common::String message = Common::String::format("%d", _globals[kPlayerScore]);
		message += " ";
		message += _game.getQuote(0x37);
		message += " 250 ";
		message += _game.getQuote(0x38);

		_scene->_kernelMessages.add(Common::Point(160, y), 0x1110, 0x20, 3, 900, message);
		y += 16;

		_scene->_kernelMessages.add(Common::Point(160, y), 0x1110, 0x20, 0, 900, _game.getQuote(0x39));
		y += 16;

		int score = _globals[kPlayerScore];
		int messageId;

		if (score <= 25)
			// Score level: Stage sweeper
			messageId = 0x3A;
		else if (score <= 50)
			// Score level: Dresser
			messageId = 0x3B;
		else if (score <= 75)
			// Score level: Usher
			messageId = 0x3C;
		else if (score <= 100)
			// Score level: Stagehand
			messageId = 0x3D;
		else if (score <= 150)
			// Score level: Chorus member
			messageId = 0x3E;
		else if (score <= 200)
			// Score level: Supporting player
			messageId = 0x3F;
		else if (score <= 249)
			// Score level: Star player
			messageId = 0x40;
		else
			// Score level: Director
			messageId = 0x41;

		_scene->_kernelMessages.add(Common::Point(160, y), 0x1110, 0x20, 0, 900, _game.getQuote(messageId));
		_scene->_sequences.addTimer(930, 3);
	}

	if (_game._trigger == 3)
		_game._winStatus = 1;
}

void Scene250::actions() {
}

void Scene250::preActions() {
}

/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
