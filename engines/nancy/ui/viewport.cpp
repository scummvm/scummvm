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

#include "common/system.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/viewport.h"

namespace Nancy {
namespace UI {

// does NOT put the object in a valid state until loadVideo is called
void Viewport::init() {
	Common::SeekableReadStream *viewChunk = g_nancy->getBootChunkStream("VIEW");
	viewChunk->seek(0);

	Common::Rect dest;
	readRect(*viewChunk, dest);
	viewChunk->skip(16); // skip viewport source rect
	readRect(*viewChunk, _format1Bounds);
	readRect(*viewChunk, _format2Bounds);

	_screenPosition = dest;

	setEdgesSize(g_nancy->_verticalEdgesSize, g_nancy->_verticalEdgesSize, g_nancy->_horizontalEdgesSize, g_nancy->_horizontalEdgesSize);

	RenderObject::init();
}

void Viewport::handleInput(NancyInput &input) {
	Time playTime = g_nancy->getTotalPlayTime();
	byte direction = 0;

	// Make cursor sticky when scrolling the viewport
	if (	g_nancy->getGameType() != kGameTypeVampire &&
			input.input & (NancyInput::kLeftMouseButton | NancyInput::kRightMouseButton)
			&& _stickyCursorPos.x > -1) {
		g_system->warpMouse(_stickyCursorPos.x, _stickyCursorPos.y);
		input.mousePos = _stickyCursorPos;
	}

	Common::Rect viewportActiveZone;

	if (g_nancy->getGameType() == kGameTypeVampire) {
		viewportActiveZone = g_nancy->_graphicsManager->getScreen()->getBounds();
		viewportActiveZone.bottom = _screenPosition.bottom;
	} else {
		viewportActiveZone = _screenPosition;
	}

	if (viewportActiveZone.contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kNormal);

		if (input.mousePos.x < _nonScrollZone.left) {
			direction |= kLeft;
		}

		if (input.mousePos.x > _nonScrollZone.right) {
			direction |= kRight;
		}

		if (input.mousePos.y < _nonScrollZone.top) {
			direction |= kUp;
		}

		if (input.mousePos.y > _nonScrollZone.bottom) {
			// Handle TVD's weird behavior
			if (_screenPosition.contains(input.mousePos)) {
				direction |= kDown;
			} else {
				direction &= ~(kLeft | kRight);
			}
		}

		// Handle diagonals
		if (direction & (kLeft | kUp) ||
			direction & (kLeft | kDown) ||
			direction & (kRight | kUp) ||
			direction & (kRight | kDown)) {
				if (direction & _edgesMask) {
					direction = 0;
				}
		}

		direction &= ~_edgesMask;
	}

	// Set sticky cursor
	if (input.input & (NancyInput::kLeftMouseButton | NancyInput::kRightMouseButton) && direction) {
		if (_stickyCursorPos.x <= -1) {
			_stickyCursorPos = input.mousePos;
		}
	} else {
		_stickyCursorPos.x = -1;
	}

	if (direction) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kMove);

		if (input.input & NancyInput::kRightMouseButton) {
			direction |= kMoveFast;
		} else if ((input.input & NancyInput::kLeftMouseButton) == 0) {
			direction = 0;
		}

		// If we hover over an edge we don't want to click an element in the viewport underneath
		// or to change the cursor, so we make the mouse input invalid
		input.eatMouseInput();
	}

	if (!direction) {
		if (input.input & NancyInput::kMoveUp) {
			direction |= kUp;
		}
		if (input.input & NancyInput::kMoveDown) {
			direction |= kDown;
		}
		if (input.input & NancyInput::kMoveLeft) {
			direction |= kLeft;
		}
		if (input.input & NancyInput::kMoveRight) {
			direction |= kRight;
		}
		if (input.input & NancyInput::kMoveFastModifier) {
			direction |= kMoveFast;
		}
	}

	// Perform the movement
	if (direction) {
		const Nancy::State::Scene::SceneSummary &summary = NancySceneState.getSceneSummary();
		Time movementDelta = NancySceneState.getMovementTimeDelta(direction & kMoveFast);

		if (playTime > _nextMovementTime) {
			if (direction & kLeft) {
				setNextFrame();
			}

			if (direction & kRight) {
				setPreviousFrame();
			}

			if (direction & kUp) {
				scrollUp(summary.verticalScrollDelta);
			}

			if (direction & kDown) {
				scrollDown(summary.verticalScrollDelta);
			}

			_nextMovementTime = playTime + movementDelta;
		}
	}

	_movementLastFrame = direction;
}

void Viewport::loadVideo(const Common::String &filename, uint frameNr, uint verticalScroll, byte panningType, uint16 format, const Common::String &palette) {
	if (_decoder.isVideoLoaded()) {
		_decoder.close();
	}

	if (!_decoder.loadFile(filename + ".avf")) {
		error("Couldn't load video file %s", filename.c_str());
	}

	_videoFormat = format;

	enableEdges(kUp | kDown | kLeft | kRight);

	setFrame(frameNr);
	setVerticalScroll(verticalScroll);

	if (palette.size()) {
		setPalette(palette);
	}

	_movementLastFrame = 0;
	_nextMovementTime = 0;
	_panningType = panningType;
}

void Viewport::setFrame(uint frameNr) {
	assert(frameNr < _decoder.getFrameCount());

	const Graphics::Surface *newFrame = _decoder.decodeFrame(frameNr);

	// Format 1 uses quarter-size images, while format 2 uses full-size ones
	// Videos in TVD are always upside-down
	GraphicsManager::copyToManaged(*newFrame, _fullFrame, g_nancy->getGameType() == kGameTypeVampire, _videoFormat == 1);

	_needsRedraw = true;
	_currentFrame = frameNr;

	if (_panningType == kPanLeftRight && !((_edgesMask & kLeft) && (_edgesMask & kRight))) {
		if (_currentFrame == 0) {
			disableEdges(kRight);
		} else if (_currentFrame == getFrameCount() - 1) {
			disableEdges(kLeft);
		} else {
			enableEdges(kLeft | kRight);
		}
	}
}

void Viewport::setNextFrame() {
	uint newFrame = getCurFrame() + 1 >= getFrameCount() ? 0 : getCurFrame() + 1;
	if (newFrame != _currentFrame) {
		setFrame(newFrame);
	}
}

void Viewport::setPreviousFrame() {
	uint newFrame = (int)getCurFrame() - 1 < 0 ? getFrameCount() - 1 : getCurFrame() - 1;
	if (newFrame != _currentFrame) {
		setFrame(newFrame);
	}
}

void Viewport::setVerticalScroll(uint scroll) {
	assert((int)scroll + _drawSurface.h <= _fullFrame.h);

	Common::Rect sourceBounds = _screenPosition;
	sourceBounds.moveTo(0, scroll);
	_drawSurface.create(_fullFrame, sourceBounds);
	_needsRedraw = true;

	if (scroll == getMaxScroll()) {
		disableEdges(kDown);
		enableEdges(kUp);
	} else if (scroll == 0) {
		disableEdges(kUp);
		enableEdges(kDown);
	} else {
		enableEdges(kUp | kDown);
	}
}

void Viewport::scrollUp(uint delta) {
	if (_drawSurface.getOffsetFromOwner().y != 0) {
		setVerticalScroll(_drawSurface.getOffsetFromOwner().y < (int16)delta ? 0 : _drawSurface.getOffsetFromOwner().y - delta);
	}
}

void Viewport::scrollDown(uint delta) {
	if (_drawSurface.getOffsetFromOwner().y != getMaxScroll()) {
		setVerticalScroll(_drawSurface.getOffsetFromOwner().y + delta > getMaxScroll() ? getMaxScroll() : _drawSurface.getOffsetFromOwner().y + delta);
	}
}

uint16 Viewport::getMaxScroll() const {
	return _fullFrame.h - _drawSurface.h - (g_nancy->getGameType() == kGameTypeVampire ? 1 : 0);
}

Common::Rect Viewport::getBoundsByFormat(uint format) const {
	if (format == 1) {
		return _format1Bounds;
	} else if (format == 2) {
		return _format2Bounds;
	} else {
		return Common::Rect();
	}
}

// Convert a viewport-space rectangle to screen coordinates
Common::Rect Viewport::convertViewportToScreen(const Common::Rect &viewportRect) const {
	Common::Rect ret = convertToScreen(viewportRect);
	ret.translate(0, -getCurVerticalScroll());
	ret.clip(_screenPosition);
	return ret;
}

// Convert a screen-space coordinate to viewport coordinates
Common::Rect Viewport::convertScreenToViewport(const Common::Rect &viewportRect) const {
	Common::Rect ret = convertToLocal(viewportRect);
	ret.translate(0, getCurVerticalScroll());
	return ret;
}

void Viewport::setEdgesSize(uint16 upSize, uint16 downSize, uint16 leftSize, uint16 rightSize) {
	_nonScrollZone = _screenPosition;
	uint offset = g_nancy->getGameType() == kGameTypeVampire ? 0 : 1;
	_nonScrollZone.top += upSize;
	_nonScrollZone.left += leftSize;
	_nonScrollZone.bottom -= downSize + offset;
	_nonScrollZone.right -= rightSize + offset;
}

void Viewport::disableEdges(byte edges) {
	_edgesMask |= edges;
}

void Viewport::enableEdges(byte edges) {
	_edgesMask &= ~edges;
}

} // End of namespace UI
} // End of namespace Nancy
