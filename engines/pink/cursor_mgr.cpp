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

#include "pink/cursor_mgr.h"
#include "pink/pink.h"
#include "pink/objects/pages/page.h"
#include "pink/objects/actors/cursor_actor.h"

namespace Pink {

CursorMgr::CursorMgr(PinkEngine *game, Page *page)
	: _actor(nullptr), _page(page), _game(game),
	_time(0), _isPlayingAnimation(false),
	_isSecondFrame(false), _firstFrameIndex(0)  {}

void CursorMgr::setCursor(byte index, Common::Point point, const Common::String &itemName) {
	switch (index) {
	case kClickableFirstFrameCursor:
	case kPDAClickableFirstFrameCursor:
		startAnimation(index);
		hideItem();
		break;
	case kHoldingItemCursor:
		_game->setCursor(index);
		_isPlayingAnimation = false;
		showItem(itemName, point);
		break;
	default:
		_game->setCursor(index);
		_isPlayingAnimation = false;
		hideItem();
		break;
	}
}

void CursorMgr::update() {
	if (!_isPlayingAnimation)
		return;

	uint newTime = _game->getTotalPlayTime();
	if (newTime - _time > kCursorsUpdateTime) {
		_time = newTime;
		_isSecondFrame = !_isSecondFrame;
		_game->setCursor(_firstFrameIndex + _isSecondFrame);
	}
}

void CursorMgr::setCursor(const Common::String &cursorName, Common::Point point) {
	byte index;
	if (cursorName == kCursorNameExitLeft)
		index = kExitLeftCursor;
	else if (cursorName == kCursorNameExitRight)
		index = kExitRightCursor;
	else if (cursorName == kCursorNameExitForward || cursorName == kCursorNameExitUp || cursorName == kCursorNameExit)
		index = kExitForwardCursor;
	else if (cursorName == kCursorNameExitBackWards)
		index = kExitDownCursor;
	else {
		warning("%s UNKNOWN CURSOR", cursorName.c_str());
		index = kExitForwardCursor;
	}

	setCursor(index, point, "");
}

void CursorMgr::hideItem() {
	if (_actor)
		_actor->setAction(kHideAction);
}

void CursorMgr::startAnimation(byte index) {
	if (_isPlayingAnimation)
		return;

	_game->setCursor(index);
	_time = _game->getTotalPlayTime();
	_firstFrameIndex = index;
	_isPlayingAnimation = true;
	_isSecondFrame = false;
}

void CursorMgr::showItem(const Common::String &itemName, Common::Point point) {
	if (!_actor)
		_actor = static_cast<CursorActor *>(_page->findActor(kCursor));
	_actor->setCursorItem(itemName, point);
}

} // End of namespace Pink
