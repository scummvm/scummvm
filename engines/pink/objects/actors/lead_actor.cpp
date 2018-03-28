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

namespace Pink {

void LeadActor::deserialize(Archive &archive) {
    _state = kReady;
    Actor::deserialize(archive);
    _state = kReady;
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
        case kPlayingVideo:
            _sequencer->update();
            if (!_sequencer->_context){
                _state = kUnk_Loading;
                _page->getGame()->changeScene(_page);
            }
        default:
            break;
    }
}

void LeadActor::OnKeyboardButtonClick(Common::KeyCode code) {
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
