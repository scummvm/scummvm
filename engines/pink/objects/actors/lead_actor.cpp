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

#include "pink/archive.h"
#include "pink/cursor_mgr.h"
#include "pink/pink.h"
#include "pink/objects/actions/action.h"
#include "pink/objects/actors/supporting_actor.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequence_context.h"
#include "pink/objects/sequences/sequencer.h"
#include "pink/objects/walk/walk_mgr.h"

namespace Pink {

LeadActor::LeadActor()
		: _state(kReady), _nextState(kReady), _isHaveItem(false),
		  _recipient(nullptr), _cursorMgr(nullptr), _walkMgr(nullptr),
		  _sequencer(nullptr), _audioInfoMgr(this) {}

void LeadActor::deserialize(Archive &archive) {
	_state = kReady;
	Actor::deserialize(archive);
	_cursorMgr = static_cast<CursorMgr*>(archive.readObject());
	_walkMgr = static_cast<WalkMgr*>(archive.readObject());
	_sequencer = static_cast<Sequencer*>(archive.readObject());
}

void LeadActor::toConsole() {
	debug("LeadActor: _name = %s", _name.c_str());
	for (uint i = 0; i < _actions.size(); ++i) {
		_actions[i]->toConsole();
	}
}

void LeadActor::loadState(Archive &archive) {
	_state = (State) archive.readByte();
	_nextState = (State) archive.readByte();
	_stateBeforeInventory = (State) archive.readByte();
	_stateBeforePDA = (State) archive.readByte();
	_isHaveItem = archive.readByte();
	Common::String recepient = archive.readString();
	if (!recepient.empty())
		_recipient = _page->findActor(recepient);
	else
		_recipient = nullptr;
	_sequencer->loadState(archive);
	_walkMgr->loadState(archive);
	_page->getGame()->getPdaMgr().loadState(archive);
	// load audioInfoMgr
}

void LeadActor::saveState(Archive &archive) {
	archive.writeByte(_state);
	archive.writeByte(_nextState);
	archive.writeByte(_stateBeforeInventory);
	archive.writeByte(_stateBeforePDA);
	archive.writeByte(_isHaveItem);
	if (_recipient)
		archive.writeString(_recipient->getName());
	else
		archive.writeString(Common::String());
	_sequencer->saveState(archive);
	_walkMgr->saveState(archive);
	_page->getGame()->getPdaMgr().saveState(archive);
}

void LeadActor::init(bool unk) {
	if (_state == kUndefined)
		_state = kReady;

	getInventoryMgr()->setLeadActor(this);
	_page->getGame()->setLeadActor(this);
	Actor::init(unk);
}

void LeadActor::start(bool isHandler) {
	if (isHandler && _state != kPlayingExitSequence) {
		_state = kPlayingSequence;
		_nextState = kReady;
	}

	switch (_state) {
	case kInventory:
		startInventory(1);
		break;
	case kPDA:
		if (_stateBeforePDA == kInventory)
			startInventory(1);
		loadPDA(_page->getGame()->getPdaMgr().getSavedPageName());
		break;
	default:
		forceUpdateCursor();
	}
}

void LeadActor::update() {
	switch (_state) {
	case kReady:
		_sequencer->update();
		_cursorMgr->update();
		break;
	case kMoving:
		_walkMgr->update();
		_cursorMgr->update();
		break;
	case kPlayingSequence:
		_sequencer->update();
		if (!_sequencer->isPlaying()) {
			_state = _nextState;
			_nextState = kUndefined;
			forceUpdateCursor();
		}
		break;
	case kInventory:
		getInventoryMgr()->update();
		break;
	case kPDA:
		getPage()->getGame()->getPdaMgr().update();
		break;
	case kPlayingExitSequence:
		_sequencer->update();
		if (!_sequencer->isPlaying()) {
			_state = kUndefined;
			_page->getGame()->changeScene();
		}
		break;
	default:
		break;
	}
}

void LeadActor::loadPDA(const Common::String &pageName) {
	if (_state != kPDA) {
		if (_state == kMoving)
			cancelInteraction();
		if (_state != kInventory)
			_page->pause(true);

		_stateBeforePDA = _state;
		_state = kPDA;
	}
	_page->getGame()->getDirector()->saveStage();
	_page->getGame()->getPdaMgr().setLead(this);
	_page->getGame()->getPdaMgr().goToPage(pageName);
}

void LeadActor::onKeyboardButtonClick(Common::KeyCode code) {
	if (code == Common::KEYCODE_g) {
		loadPDA("TOC");
		return;
	}
	switch (_state) {
	case kMoving:
		switch (code) {
		case Common::KEYCODE_ESCAPE:
			cancelInteraction();
			// Fall Through intended
		case Common::KEYCODE_SPACE:
			_walkMgr->skip();
		default:
			break;
		}
		break;
	case kPlayingSequence:
	case kPlayingExitSequence:
		switch (code) {
		case Common::KEYCODE_SPACE:
		case Common::KEYCODE_RIGHT:
			_sequencer->skipSubSequence();
			break;
		case Common::KEYCODE_ESCAPE:
			_sequencer->skipSequence();
			break;
		case Common::KEYCODE_LEFT:
			_sequencer->restartSequence();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void LeadActor::onLeftButtonClick(const Common::Point point) {
	switch (_state) {
	case kReady:
	case kMoving: {
		Actor *clickedActor = getActorByPoint(point);

		if (this == clickedActor) {
			_audioInfoMgr.stop();
			onClick();
		} else if (isInteractingWith(clickedActor)) {
			_recipient = clickedActor;
			if (!startWalk()) {
				_audioInfoMgr.stop();
				if (_isHaveItem)
					sendUseClickMessage(clickedActor);
				else
					sendLeftClickMessage(clickedActor);
			}
		} else
			clickedActor->onLeftClickMessage();

		break;
	}
	case kPDA:
		_page->getGame()->getPdaMgr().onLeftButtonClick(point);
		break;
	case kInventory:
		getInventoryMgr()->onClick(point);
		break;
	default:
		break;
	}
}

void LeadActor::onRightButtonClick(const Common::Point point) {
	if (_state == kReady || _state == kMoving) {
		Actor *clickedActor = getActorByPoint(point);
		if (isInteractingWith(clickedActor)) {
			_audioInfoMgr.start(clickedActor);
		}

		if (_state == kMoving)
			cancelInteraction();
	}
}


void LeadActor::onMouseMove(Common::Point point) {
	if (_state != kPDA)
		updateCursor(point);
	else _page->getGame()->getPdaMgr().onMouseMove(point);
}

void LeadActor::onMouseOver(const Common::Point point, CursorMgr *mgr) {
	if (getInventoryMgr()->isPinkOwnsAnyItems())
		_cursorMgr->setCursor(kClickableFirstFrameCursor, point, Common::String());
	else
		Actor::onMouseOver(point, mgr);
}

void LeadActor::onClick() {
	if (_isHaveItem) {
		assert(_state != kMoving);
		_isHaveItem = false;
		_nextState = kUndefined;
	} else {
		if (_state == kMoving)
			cancelInteraction();
		startInventory(0);
	}
}

void LeadActor::onInventoryClosed(bool isItemClicked) {
	_isHaveItem = isItemClicked;
	_state = _stateBeforeInventory;
	_stateBeforeInventory = kUndefined;
	_page->pause(false);
	forceUpdateCursor();
}

void LeadActor::onWalkEnd(const Common::String &stopName) {
	State oldNextState = _nextState;
	_state = kReady;
	_nextState = kUndefined;
	if (_recipient && oldNextState == kPlayingSequence) {
		if (_isHaveItem)
			sendUseClickMessage(_recipient);
		else
			sendLeftClickMessage(_recipient);
	} else {  // on ESC button
		Action *action = findAction(stopName);
		assert(action);
		setAction(action);
	}
}

void LeadActor::onPDAClose() {
	_page->initPalette();
	_page->getGame()->getDirector()->loadStage();

	_state = _stateBeforePDA;
	if (_state != kInventory)
		_page->pause(0);
}

bool LeadActor::isInteractingWith(Actor *actor) {
	if (!_isHaveItem)
		return actor->isLeftClickHandlers();

	return actor->isUseClickHandlers(getInventoryMgr()->getCurrentItem());
}

void LeadActor::setNextExecutors(const Common::String &nextModule, const Common::String &nextPage) {
	if (_state == kReady || _state == kMoving || _state == kPlayingSequence || _state == kInventory || _state == kPDA) {
		_state = kPlayingExitSequence;
		_page->getGame()->setNextExecutors(nextModule, nextPage);
	}
}

void LeadActor::forceUpdateCursor() {
	const Common::Point point = _page->getGame()->getEventManager()->getMousePos();
	updateCursor(point);
}

void LeadActor::updateCursor(const Common::Point point) {
	switch (_state) {
	case kReady:
	case kMoving: {
		Actor *actor = getActorByPoint(point);
		InventoryItem *item = getInventoryMgr()->getCurrentItem();
		if (_isHaveItem) {
			if (actor) {
				actor->onHover(point, item->getName(), _cursorMgr);
			} else
				_cursorMgr->setCursor(kHoldingItemCursor, point, item->getName());
		} else if (actor)
			actor->onMouseOver(point, _cursorMgr);
		else
			_cursorMgr->setCursor(kDefaultCursor, point, Common::String());
		break;
	}
	case kPlayingSequence:
	case kPlayingExitSequence:
		_cursorMgr->setCursor(kNotClickableCursor, point, Common::String());
		break;
	case kPDA:
	case kInventory:
		_cursorMgr->setCursor(kDefaultCursor, point, Common::String());
		break;
	default:
		break;
	}
}

bool LeadActor::sendUseClickMessage(Actor *actor) {
	InventoryMgr *mgr = getInventoryMgr();
	assert(_state != kPlayingExitSequence);
	_nextState = kReady;
	_state = kPlayingSequence;
	InventoryItem *item = mgr->getCurrentItem();
	actor->onUseClickMessage(mgr->getCurrentItem(), mgr);
	if (item->getCurrentOwner() != this->_name)
		_isHaveItem = false;
	 forceUpdateCursor();
	 return true;
}

bool LeadActor::sendLeftClickMessage(Actor *actor) {
	assert(_state != kPlayingExitSequence);
	_nextState = kReady;
	_state = kPlayingSequence;
	actor->onLeftClickMessage();
	forceUpdateCursor();
	return true;
}

WalkLocation *LeadActor::getWalkDestination() {
	return _walkMgr->findLocation(_recipient->getLocation());
}

Actor *LeadActor::getActorByPoint(const Common::Point point) {
	return _page->getGame()->getDirector()->getActorByPoint(point);
}

void LeadActor::startInventory(bool paused) {
	if (!getInventoryMgr()->start(paused))
		return;

	if (!paused) {
		_isHaveItem = false;
		_stateBeforeInventory = _state;
		_state = kInventory;
		forceUpdateCursor();
	}
	_page->pause(true);
}

bool LeadActor::startWalk() {
	WalkLocation *location = getWalkDestination();
	if (location) {
		_state = kMoving;
		_nextState = kPlayingSequence;
		_walkMgr->start(location);
		return true;
	}

	return false;
}

void LeadActor::cancelInteraction() {
	_recipient = nullptr;
	_nextState = kReady;
}

Actor *LeadActor::findActor(const Common::String &name) {
	return _page->findActor(name);
}

void ParlSqPink::toConsole() {
	debug("ParlSqPink: _name = %s", _name.c_str());
	for (uint i = 0; i < _actions.size(); ++i) {
		_actions[i]->toConsole();
	}
}

WalkLocation *ParlSqPink::getWalkDestination() {
	if (_recipient->getName() == kBoy && _page->checkValueOfVariable(kBoyBlocked, "UNDEFINED"))
		return _walkMgr->findLocation(kSirBaldley);

	return LeadActor::getWalkDestination();
}

PubPink::PubPink() :
		LeadActor(), _round(0) {}

void PubPink::toConsole() {
	debug("PubPink: _name = %s", _name.c_str());
	for (uint i = 0; i < _actions.size(); ++i) {
		_actions[i]->toConsole();
	}
}

void PubPink::onClick() {
	if (!playingMiniGame())
		LeadActor::onClick();
}

void PubPink::onVariableSet() {
	if (playingMiniGame())
		_isHaveItem = true;
}

void PubPink::updateCursor(const Common::Point point) {
	if (playingMiniGame()) {
		Actor *actor = getActorByPoint(point);
		assert(actor);
		if (_state == kReady && actor->isUseClickHandlers(getInventoryMgr()->getCurrentItem())) {
			_cursorMgr->setCursor(kClickableFirstFrameCursor, point, Common::String());
		} else
			_cursorMgr->setCursor(kDefaultCursor, point, Common::String());
	}
	else LeadActor::updateCursor(point);
}

bool PubPink::sendUseClickMessage(Actor *actor) {
   if (!LeadActor::sendUseClickMessage(actor) && playingMiniGame()) {
	   _nextState = _state;
	   _state = kPlayingSequence;

	   const char *roundName;
	   switch (_round++ % 3) {
	   case 0:
		   roundName = kFirstRound;
		   break;
	   case 1:
		   roundName = kSecondRound;
		   break;
	   case 2:
		   roundName = kThirdRound;
		   break;
	   default:
		   roundName = nullptr;
		   assert(0);
	   }
	   _sequencer->authorSequence(_sequencer->findSequence(roundName), 0);
   }

   if (playingMiniGame())
	   _isHaveItem = true;

   return true;
}

WalkLocation *PubPink::getWalkDestination() {
	if (playingMiniGame())
		return nullptr;

	if (_recipient->getName() == kJackson && !_page->checkValueOfVariable(kDrunkLocation, kBolted))
		return _walkMgr->findLocation(_page->findActor(kDrunk)->getName());

	return LeadActor::getWalkDestination();
}

bool PubPink::playingMiniGame() {
	return !(_page->checkValueOfVariable(kFoodPuzzle, "TRUE") ||
		   _page->checkValueOfVariable(kFoodPuzzle, "UNDEFINED"));
}

} // End of namespace Pink
