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

void LeadActor::deserialize(Archive &archive) {
    _state = kReady;
    Actor::deserialize(archive);
    _cursorMgr = static_cast<CursorMgr*>(archive.readObject());
    _walkMgr = static_cast<WalkMgr*>(archive.readObject());
    _sequencer = static_cast<Sequencer*>(archive.readObject());
}

void LeadActor::setNextExecutors(Common::String &nextModule, Common::String &nextPage) {
    if (_state == kReady || _state == kMoving || _state == kInDialog1 || _state == kInventory || _state == kPDA) {
        _state = kPlayingVideo;
        _page->getGame()->setNextExecutors(nextModule, nextPage);
    }
}

void LeadActor::init(bool unk) {
    if (_state == kUnk_Loading){
        _state = kReady;
    }
    _page->getModule()->getInventoryMgr()->setLeadActor(this);
    _page->getGame()->setLeadActor(this);
    Actor::init(unk);
}

void LeadActor::toConsole() {
    debug("LeadActor: _name = %s", _name.c_str());
    for (int i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
}

LeadActor::State LeadActor::getState() const {
    return _state;
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
        case kInDialog1:
        case kInDialog2:
            _sequencer->update();
            if (!_sequencer->_context){
                _state = _nextState;
                _nextState = kUnk_Loading;
            }
            break;

        case kInventory:
            getPage()->getModule()->getInventoryMgr()->update();
            break;
        case kPDA:
            break;

        case kPlayingVideo:
            _sequencer->update();
            if (!_sequencer->_context){
                _state = kUnk_Loading;
                _page->getGame()->changeScene(_page);
            }
            break;
        case kUnk_Loading:
            break;
    }
}

void LeadActor::onKeyboardButtonClick(Common::KeyCode code) {
    switch(_state) {
        case kMoving:
            switch (code){
                case Common::KEYCODE_ESCAPE:
                    // set unk variables
                    // Fall Through intended
                case Common::KEYCODE_SPACE:
                    //skip walking animation
                default:
                    break;
            }
            break;
        case kInDialog1:
        case kInDialog2:
        case kPlayingVideo:
            switch (code) {
                case Common::KEYCODE_SPACE:
                case Common::KEYCODE_RIGHT:
                    _sequencer->skipSubSequence();
                    break;
                case Common::KEYCODE_ESCAPE:
                    _sequencer->skipToLastSubSequence();
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

void LeadActor::start(bool isHandler) {
    if (isHandler && _state != kPlayingVideo){
        _state = kInDialog1;
        _nextState = kReady;
    }
    forceUpdateCursor();
}

void LeadActor::onMouseMove(Common::Point point) {
    if (_state != kPDA)
        updateCursor(point);
    else error("pda is not supported");
}

void LeadActor::updateCursor(Common::Point point) {
    switch (_state) {
        case kReady:
        case kMoving: {
            Director *director = _page->getGame()->getDirector();
            Actor *actor = director->getActorByPoint(point);
            InventoryItem *item = _page->getModule()->getInventoryMgr()->getCurrentItem();
            if (_isHaveItem) {
                if (actor) {
                    actor->onHover(point, item->getName(), _cursorMgr);
                }
                else _cursorMgr->setCursor(kHoldingItemCursor, point, item->getName());
            }
            else if (actor)
                actor->onMouseOver(point, _cursorMgr);
            else _cursorMgr->setCursor(kDefaultCursor, point, Common::String());
            break;
        }
        case kInDialog1:
        case kInDialog2:
        case kPlayingVideo:
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

void LeadActor::onLeftButtonClick(Common::Point point) {
    InventoryMgr *invMgr = _page->getModule()->getInventoryMgr();

    switch (_state) {
        case kReady:
        case kMoving: {
        Actor *actor = _page->getGame()->getDirector()->getActorByPoint(point);

        if (this == actor) {
            onClick();
            return;
        }

        _recipient = dynamic_cast<SupportingActor*>(actor);
        if (actor->isClickable() && isInteractingWith(_recipient)) {
            WalkLocation *location = getWalkDestination();
            if (location) {
                _state = kMoving;
                _nextState = kInDialog1;
                _walkMgr->start(location);
            }
            else if (_state == kReady){
                if (_isHaveItem)
                    sendUseClickMessage(_recipient);
                else sendLeftClickMessage(_recipient);
            }
        }

        break;
        }
        case kPDA:

            break;
        case kInventory:
            invMgr->onClick(point);
            break;
        default:
            break;
    }
}

void LeadActor::onMouseOver(Common::Point point, CursorMgr *mgr) {
    if (_page->getModule()->getInventoryMgr()->isPinkOwnsAnyItems())
        _cursorMgr->setCursor(kClickableFirstFrameCursor, point, Common::String());
    else Actor::onMouseOver(point, mgr);
}

void LeadActor::onWalkEnd() {
    State oldNextState = _nextState;
    _state = kReady;
    _nextState = kUnk_Loading;
    if (_recipient && oldNextState == kInDialog1){
        if (_isHaveItem)
            sendUseClickMessage(_recipient);
        else sendLeftClickMessage(_recipient);
    }
}

bool LeadActor::sendUseClickMessage(SupportingActor *actor) {
    InventoryMgr *mgr = _page->getModule()->getInventoryMgr();
    _nextState = _state != kPlayingVideo ? kReady : kPlayingVideo;
    _state = kInDialog1;
    InventoryItem *item = mgr->getCurrentItem();
    actor->onUseClickMessage(mgr->getCurrentItem(), mgr);
    if (item->getCurrentOwner() != this->_name)
        _isHaveItem = false;
     return true;
}

bool LeadActor::sendLeftClickMessage(SupportingActor *actor) {
    _nextState = _state != kPlayingVideo ? kReady : kPlayingVideo;
    _state = kInDialog1;
    return actor->onLeftClickMessage();
}

void LeadActor::onClick() {
    if (_isHaveItem) {
        _isHaveItem = false;
        _nextState = (_state != kMoving) ?
                     kUnk_Loading : kReady;
    }
    else {
        if (_state == kMoving) {
            _recipient = nullptr;
            _nextState = kReady;
        }
        if (_page->getModule()->getInventoryMgr()->start(1)){
            _stateCopy = _state;
            _state = kInventory;
            _page->pause();
        }
    }
}

LeadActor::LeadActor()
    : _state(kReady), _nextState(kReady),
      _isHaveItem(false), _recipient(nullptr),
      _cursorMgr(nullptr), _walkMgr(nullptr),
      _sequencer(nullptr)
{}

void LeadActor::onInventoryClosed(bool isItemClicked) {
    _isHaveItem = isItemClicked;
    _state = _stateCopy;
    _stateCopy = kUnk_Loading;
    _page->unpause();
    forceUpdateCursor();
}

void LeadActor::forceUpdateCursor() {
    Common::Point point = _page->getGame()->getEventManager()->getMousePos();
    updateCursor(point);
}

WalkLocation *LeadActor::getWalkDestination() {
    return _walkMgr->findLocation(_recipient->getLocation());
}

bool LeadActor::isInteractingWith(SupportingActor *actor) {
    if (!_isHaveItem)
        return actor->isLeftClickHandlers();

    return actor->isUseClickHandlers(_page->getModule()->getInventoryMgr()->getCurrentItem());
}

void ParlSqPink::toConsole() {
    debug("ParlSqPink: _name = %s", _name.c_str());
    for (int i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
}

WalkLocation *ParlSqPink::getWalkDestination() {
    if (_recipient->getName() == kBoy &&
        _page->checkValueOfVariable(kBoyBlocked, kUndefined))
    {
        return _walkMgr->findLocation(kSirBaldley);
    }
    return LeadActor::getWalkDestination();
}

PubPink::PubPink() :
        LeadActor(), _round(0)
{}

void PubPink::toConsole() {
    debug("PubPink: _name = %s", _name.c_str());
    for (int i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
}

bool PubPink::playingMiniGame() {
    return !(_page->checkValueOfVariable(kFoodPuzzle, "TRUE") ||
              _page->checkValueOfVariable(kFoodPuzzle, kUndefined));
}

void PubPink::onClick() {
    if (!playingMiniGame())
        LeadActor::onClick();
}

void PubPink::updateCursor(Common::Point point) {
    if (playingMiniGame()) {
        SupportingActor *actor = dynamic_cast<SupportingActor*>(_page->getGame()->getDirector()->getActorByPoint(point));
        if (_state == kReady &&
            actor &&
            actor->isUseClickHandlers(_page->getModule()->getInventoryMgr()->getCurrentItem()))
        {
                   _cursorMgr->setCursor(kClickableFirstFrameCursor, point, Common::String());
        }
        else _cursorMgr->setCursor(kDefaultCursor, point, Common::String());
    }
    else LeadActor::updateCursor(point);
}

WalkLocation *PubPink::getWalkDestination() {
    if (playingMiniGame())
        return nullptr;

    if (_recipient->getName() == kJackson &&
        !_page->checkValueOfVariable(kDrunkLocation, kBolted))
    {
        return _walkMgr->findLocation(_page->findActor(kDrunk)->getName());
    }

    return LeadActor::getWalkDestination();
}

bool PubPink::sendUseClickMessage(SupportingActor *actor) {
   if (!LeadActor::sendUseClickMessage(actor) &&
           playingMiniGame()) {
       _nextState = _state;
       _state = kInDialog1;

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
               break;
       }
       _sequencer->authorSequence(_sequencer->findSequence(roundName), 0);
   }

   if (playingMiniGame())
       _isHaveItem = true;

   return true;
}

void PubPink::onVariableSet() {
    if (playingMiniGame())
        _isHaveItem = true;
}

} // End of namespace Pink
