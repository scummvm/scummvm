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

#include "pink/pink.h"
#include "pink/cel_decoder.h"
#include "pink/cursor_mgr.h"
#include "pink/objects/actions/action_cel.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

CursorMgr::CursorMgr(PinkEngine *game, GamePage *page)
		: _actor(nullptr), _page(page), _game(game),
		  _isPlayingAnimation(0), _firstFrameIndex(0) {}

CursorMgr::~CursorMgr() {}

void CursorMgr::setCursor(uint index, Common::Point point, const Common::String &itemName) {
	if (index == kClickableFirstFrameCursor) {
		startAnimation(index);
		return hideItem();
	} else if (index != kHoldingItemCursor) {

		if (index != kPDAClickableFirstFrameCursor) {
			_game->setCursor(index);
			_isPlayingAnimation = 0;
			return hideItem();
		}

		hideItem();
		return startAnimation(index);
	}

	_game->setCursor(index);
	_isPlayingAnimation = 0;

	_actor = _actor ? _actor : _page->findActor(kCursor);
	assert(_actor);

	Action *action = _actor->findAction(itemName);
	assert(dynamic_cast<ActionCEL*>(action));

	if (action != _actor->getAction()) {
		_actor->setAction(action, 0);
		CelDecoder *decoder = static_cast<ActionCEL*>(action)->getDecoder();
		decoder->setX(point.x);
		decoder->setY(point.y);
	} else {
		CelDecoder *decoder = static_cast<ActionCEL*>(action)->getDecoder();
		decoder->setX(point.x);
		decoder->setY(point.y);
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

void CursorMgr::setCursor(Common::String &cursorName, Common::Point point) {
	uint index;
	if (cursorName == kCursorNameExitLeft)
		index = kExitLeftCursor;
	else if (cursorName == kCursorNameExitRight)
		index = kExitRightCursor;
	else //if (cursorName == kCursorNameExitForward || cursorName == kCursorNameExitUp)
		index = kExitForwardCursor;
	//else
		//assert(0);

	setCursor(index, point, Common::String());
}

void CursorMgr::hideItem() {
	if (_actor)
		_actor->hide();
}

void CursorMgr::startAnimation(int index) {
	if (!_isPlayingAnimation) {
		_isPlayingAnimation = 1;
		_time = _game->getTotalPlayTime();
		_firstFrameIndex = index;
		_isSecondFrame = 0;
		_game->setCursor(index);
	}
}

} // End of namespace Pink
