/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "player.h"
#include "alcachofa.h"

using namespace Common;

namespace Alcachofa {

Player::Player()
	: _activeCharacter(&g_engine->world().mortadelo()) {
	const auto &cursorPath = g_engine->world().getGlobalAnimationName(GlobalAnimationKind::Cursor);
	_cursorAnimation.reset(new Animation(cursorPath));
	_cursorAnimation->load();
}

void Player::preUpdate() {
	_selectedObject = nullptr;
	_cursorFrameI = 0;
}

void Player::postUpdate() {
	if (g_engine->input().wasAnyMouseReleased())
		_pressedObject = nullptr;
}

void Player::updateCursor() {
	if (_isOptionsMenuOpen || !_isGameLoaded)
		_cursorFrameI = 0;
	else if (_selectedObject == nullptr)
		_cursorFrameI = !g_engine->input().isMouseLeftDown() || _pressedObject != nullptr ? 6 : 7;
	else {
		auto type = _selectedObject->cursorType();
		switch (type) {
		case CursorType::Point: _cursorFrameI = 0; break;
		case CursorType::LeaveUp: _cursorFrameI = 8; break;
		case CursorType::LeaveRight: _cursorFrameI = 10; break;
		case CursorType::LeaveDown: _cursorFrameI = 12; break;
		case CursorType::LeaveLeft: _cursorFrameI = 14; break;
		case CursorType::WalkTo: _cursorFrameI = 6; break;
		default: error("Invalid cursor type %u", (uint)type); break;
		}

		if (_cursorFrameI != 0) {
			if (g_engine->input().isAnyMouseDown() && _pressedObject == _selectedObject)
				_cursorFrameI++;
		}
		else if (g_engine->input().isMouseLeftDown())
			_cursorFrameI = 2;
		else if (g_engine->input().isMouseRightDown())
			_cursorFrameI = 4;
	}

	Point cursorPos = g_engine->input().mousePos2D();
	if (_heldItem == nullptr)
		g_engine->drawQueue().add<AnimationDrawRequest>(_cursorAnimation.get(), _cursorFrameI, as2D(cursorPos), -10);
	else {
		auto itemGraphic = _heldItem->graphic();
		assert(itemGraphic != nullptr);
		auto &animation = itemGraphic->animation();
		auto frameOffset = animation.totalFrameOffset(0);
		auto imageSize = animation.imageSize(animation.imageIndex(0, 0));
		cursorPos -= frameOffset + imageSize / 2;
		g_engine->drawQueue().add<AnimationDrawRequest>(&animation, 0, as2D(cursorPos), -10);
	}
}

}
