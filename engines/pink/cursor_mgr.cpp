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

#include "cursor_mgr.h"
#include "pink.h"

namespace Pink {

CursorMgr::CursorMgr(PinkEngine *game, GamePage *page)
        : _actor(nullptr), _page(page), _game(game),
          _isPlayingAnimation(0), _firstFrameIndex(0)
{}

CursorMgr::~CursorMgr() {}

void CursorMgr::setCursor(uint index, Common::Point point) {
    if (index == kClickableFirstFrameCursor) {
        if (!_isPlayingAnimation) {
            _isPlayingAnimation = 1;
            _time = _game->getTotalPlayTime();
            _firstFrameIndex = index;
            _isSecondFrame = 0;
            _game->setCursor(index);
        }
    }
    else {
        _isPlayingAnimation = 0;
        _game->setCursor(index);
    }
}

void CursorMgr::update() {
    if (!_isPlayingAnimation)
        return;

    uint newTime = _game->getTotalPlayTime();
    if (newTime - _time > 0xC8){
        _time = newTime;
        _isSecondFrame = !_isSecondFrame;
        _game->setCursor(_firstFrameIndex + _isSecondFrame);
    }
}

void CursorMgr::setCursor(Common::String &cursorName, Common::Point point) {
    uint index;
    if (cursorName == "ExitLeft") {
        index = kExitLeftCursor;
    }
    else if (cursorName == "ExitRight"){
        index = kExitRightCursor;
    }
    else if (cursorName == "ExitForward" || cursorName == "ExitUp")
        index = kExitForwardCursor;
    else assert(0);
    setCursor(index, point);
}

} // End of namespace Pink