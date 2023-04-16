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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/animatedbutton.h"

namespace Nancy {
namespace UI {

AnimatedButton::AnimatedButton(uint zOrder) :
	RenderObject(zOrder),
	_frameTime(0),
	_currentFrame(-1),
	_nextFrameTime(0),
	_isOpen(false),
	_alwaysHighlightCursor(false) {}

void AnimatedButton::init() {
	setTransparent(true);
}

void AnimatedButton::updateGraphics() {
	uint32 currentTime = g_nancy->getTotalPlayTime();
	if (currentTime > _nextFrameTime) {
		if (_isOpen && _currentFrame < (int)_srcRects.size()) {
			setFrame(++_currentFrame);
			_nextFrameTime = currentTime + _frameTime;
			setVisible(true);
			if (_currentFrame == (int)_srcRects.size()) {
				onTrigger();
			}
		} else if (!_isOpen && _currentFrame > -1) {
			setFrame(--_currentFrame);
			_nextFrameTime = currentTime + _frameTime;
			if (_currentFrame == -1) {
				onTrigger();
				setVisible(false);
			}
		}
	}
}

void AnimatedButton::handleInput(NancyInput &input) {
	if (_hotspot.contains(input.mousePos)) {
		if (_alwaysHighlightCursor || _currentFrame == -1 || _currentFrame == (int)_srcRects.size()) {
			g_nancy->_cursorManager->setCursorType(g_nancy->getGameType() == kGameTypeVampire ? CursorManager::kHotspot : CursorManager::kHotspotArrow);
		}

		if (isPlaying()) {
			return;
		}

		if (!_highlightSrcRect.isEmpty() && !isVisible()) {
			_drawSurface.create(g_nancy->_graphicsManager->_object0, _highlightSrcRect);
			moveTo(_highlightDestRect);
			setVisible(true);
		}

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (_currentFrame == -1) {
				onClick();
				setOpen(true);
			} else if (_currentFrame == (int)_srcRects.size()) {
				onClick();
				setOpen(false);
			}
		}

		// This breaks TowerPuzzle in nancy2, so we only enable it for TVD
		if (g_nancy->getGameType() == kGameTypeVampire) {
			input.eatMouseInput();
		}
	} else if (!_highlightSrcRect.isEmpty() && isVisible() && !(isPlaying() || _isOpen)) {
		setVisible(false);
	}
}

void AnimatedButton::setFrame(int frame) {
	if (frame > -1 && frame < (int)_srcRects.size()) {
		_drawSurface.create(g_nancy->_graphicsManager->_object0, _srcRects[frame]);
		setTransparent(true);

		if (_destRects.size()) {
			moveTo(_destRects[frame]);
		}

		_needsRedraw = true;
	}
}

bool AnimatedButton::isPlaying() const {
	return _isOpen ? _currentFrame < (int)_srcRects.size() : _currentFrame > -1;
}

} // End of namespace UI
} // End of namespace Nancy
