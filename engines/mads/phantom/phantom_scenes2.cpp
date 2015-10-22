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

} // End of namespace Phantom
} // End of namespace MADS
