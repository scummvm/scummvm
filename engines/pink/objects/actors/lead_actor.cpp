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

#include "lead_actor.h"
#include <engines/pink/objects/actions/action.h>
#include <engines/pink/objects/sequences/sequence_context.h>
#include "engines/pink/objects/walk/walk_mgr.h"
#include "engines/pink/cursor_mgr.h"
#include "engines/pink/objects/sequences/sequencer.h"
#include "engines/pink/archive.h"
#include "engines/pink/objects/pages/game_page.h"
#include "engines/pink/pink.h"
#include "supporting_actor.h"

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
            if (actor)
                actor->onMouseOver(point, _cursorMgr);
            else _cursorMgr->setCursor(kDefaultCursor, point);
            break;
        }
        case kInDialog1:
        case kInDialog2:
        case kPlayingVideo:
            _cursorMgr->setCursor(kNotClickableCursor, point);
            break;
        case kPDA:
        case kInventory:
            _cursorMgr->setCursor(kDefaultCursor, point);
            break;
        default:
            break;
    }
}

void LeadActor::onLeftButtonClick(Common::Point point) {
    switch (_state) {
        case kReady:
        case kMoving: {
        Actor *actor = _page->getGame()->getDirector()->getActorByPoint(point);

        if (this == actor) {
            onClick();
            return;
        }

        _recipient = (SupportingActor*) actor;
        if (actor->isClickable() &&
            _recipient->isLeftClickHandlers()) {
            WalkLocation *location = _walkMgr->findLocation(_recipient->getLocation());
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
            _page->getModule()->getInventoryMgr()->onClick(point);
            break;
        default:
            break;
    }
}

void LeadActor::onMouseOver(Common::Point point, CursorMgr *mgr) {
    if (_page->getModule()->getInventoryMgr()->isPinkOwnsAnyItems())
        _cursorMgr->setCursor(kClickableFirstFrameCursor, point);
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
    return false;
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

void ParlSqPink::toConsole() {
    debug("ParlSqPink: _name = %s", _name.c_str());
    for (int i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
}

void PubPink::toConsole() {
    debug("PubPink: _name = %s", _name.c_str());
    for (int i = 0; i < _actions.size(); ++i) {
        _actions[i]->toConsole();
    }
}

} // End of namespace Pink
