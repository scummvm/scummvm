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

#include <engines/pink/objects/actors/actor.h>
#include <engines/pink/objects/actions/action_cel.h>
#include <engines/pink/cel_decoder.h>
#include "cursor_mgr.h"
#include "pink.h"
#include "objects/pages/game_page.h"

namespace Pink {

CursorMgr::CursorMgr(PinkEngine *game, GamePage *page)
        : _actor(nullptr), _action(nullptr), _page(page), _game(game),
          _isPlayingAnimation(0), _firstFrameIndex(0)
{}

CursorMgr::~CursorMgr() {}

void CursorMgr::setCursor(uint index, Common::Point point, const Common::String &itemName) {
    if (index == kClickableFirstFrameCursor) {
        if (!_isPlayingAnimation) {
            _isPlayingAnimation = 1;
            _time = _game->getTotalPlayTime();
            _firstFrameIndex = index;
            _isSecondFrame = 0;
            _game->setCursor(index);
        }
        return;
    }
    if (index != kHoldingItemCursor){
        _isPlayingAnimation = 0;
        _game->setCursor(index);
        return;
    }

    _game->setCursor(index);
    _actor = _actor ? _actor : _page->findActor(kCursor);
    assert(_actor);

    Action *action = _actor->findAction(itemName);
    assert(action);
    if (action != _action) {
        _action = action;
        _actor->setAction(action, 0);
    }

    assert(dynamic_cast<ActionCEL*>(action));
    CelDecoder *decoder = static_cast<ActionCEL*>(_action)->getDecoder();
    // this is buggy
    //decoder->setX(point.x);
    //decoder->setY(point.y);
}

void CursorMgr::update() {
    if (!_isPlayingAnimation)
        return;

    uint newTime = _game->getTotalPlayTime();
    if (newTime - _time > kCursorsUpdateTime){
        _time = newTime;
        _isSecondFrame = !_isSecondFrame;
        _game->setCursor(_firstFrameIndex + _isSecondFrame);
    }
}

void CursorMgr::setCursor(Common::String &cursorName, Common::Point point) {
    uint index;
    if (cursorName == kCursorNameExitLeft) {
        index = kExitLeftCursor;
    }
    else if (cursorName == kCursorNameExitRight){
        index = kExitRightCursor;
    }
    else //if (cursorName == kCursorNameExitForward || cursorName == kCursorNameExitUp)
        index = kExitForwardCursor;
    //else assert(0);


    setCursor(index, point, Common::String());
}

} // End of namespace Pink