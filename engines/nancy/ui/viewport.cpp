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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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

	_upHotspot = _downHotspot = _leftHotspot = _rightHotspot = _screenPosition;
	_downHotspot.translate(0, _screenPosition.height());
	_rightHotspot.translate(_screenPosition.width(), 0);
	_upHotspot.setHeight(0);
	_downHotspot.setHeight(0);
	_leftHotspot.setWidth(0);
	_rightHotspot.setWidth(0);

	RenderObject::init();
}

void Viewport::handleInput(NancyInput &input) {
	Time playTime = g_nancy->getTotalPlayTime();
	byte direction = 0;

	if (_screenPosition.contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kNormal);
	}

	// Do not handle hotspots marked as incative and ignore diagonals if intersecting hotspots are not active
	if (_upHotspot.contains(input.mousePos) && (_edgesMask & kUp) == 0) {
		if (_upHotspot.findIntersectingRect(_leftHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kLeft) == 0) {
				direction |= kUp;
			}
		} else if (_upHotspot.findIntersectingRect(_rightHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kRight) == 0) {
				direction |= kUp;
			}
		} else {
			direction |= kUp;
		}
	} else if (_downHotspot.contains(input.mousePos) && (_edgesMask & kDown) == 0) {
		if (_downHotspot.findIntersectingRect(_leftHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kLeft) == 0) {
				direction |= kDown;
			}
		} else if (_downHotspot.findIntersectingRect(_rightHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kRight) == 0) {
				direction |= kDown;
			}
		} else {
			direction |= kDown;
		}
	}

	if (_leftHotspot.contains(input.mousePos) && (_edgesMask & kLeft) == 0) {
		if (_leftHotspot.findIntersectingRect(_upHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kUp) == 0) {
				direction |= kLeft;
			}
		} else if (_leftHotspot.findIntersectingRect(_downHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kDown) == 0) {
				direction |= kLeft;
			}
		} else {
			direction |= kLeft;
		}
	} else if (_rightHotspot.contains(input.mousePos) && (_edgesMask & kRight) == 0) {
		if (_rightHotspot.findIntersectingRect(_upHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kUp) == 0) {
				direction |= kRight;
			}
		} else if (_rightHotspot.findIntersectingRect(_downHotspot).contains(input.mousePos)) {
			if ((_edgesMask & kDown) == 0) {
				direction |= kRight;
			}
		} else {
			direction |= kRight;
		}
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

void Viewport::loadVideo(const Common::String &filename, uint frameNr, uint verticalScroll, uint16 format, const Common::String &palette) {
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
		GraphicsManager::loadSurfacePalette(_drawSurface, palette);
		GraphicsManager::loadSurfacePalette(_fullFrame, palette);
	}

	_movementLastFrame = 0;
	_nextMovementTime = 0;
}

void Viewport::setFrame(uint frameNr) {
	assert(frameNr < _decoder.getFrameCount());

	const Graphics::Surface *newFrame = _decoder.decodeFrame(frameNr);

	if (_videoFormat == 2) {
		// Format 2 uses full-size images
		GraphicsManager::copyToManaged(*newFrame, _fullFrame);
	} else if (_videoFormat == 1) {
		// Format 1 uses quarter-size, upside-down images
		GraphicsManager::copyToManaged(*newFrame, _fullFrame, true, true);
	}

	_needsRedraw = true;

	_currentFrame = frameNr;
}

void Viewport::setNextFrame() {
	setFrame(getCurFrame() + 1 >= getFrameCount() ? 0 : getCurFrame() + 1);
}

void Viewport::setPreviousFrame() {
	setFrame((int)getCurFrame() - 1 < 0 ? getFrameCount() - 1 : getCurFrame() - 1);
}

void Viewport::setVerticalScroll(uint scroll) {
	assert(scroll + _drawSurface.h + 1 <= _fullFrame.h);

	Common::Rect sourceBounds = _screenPosition;
	sourceBounds.moveTo(0, scroll + 1);
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
	setVerticalScroll(_drawSurface.getOffsetFromOwner().y < (int16)delta ? 0 : _drawSurface.getOffsetFromOwner().y - delta);
}

void Viewport::scrollDown(uint delta) {
	setVerticalScroll(_drawSurface.getOffsetFromOwner().y + delta > getMaxScroll() ? getMaxScroll() : _drawSurface.getOffsetFromOwner().y + delta);
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
	_upHotspot.setHeight(upSize);
	_leftHotspot.setWidth(leftSize);

	_downHotspot.top = _screenPosition.bottom;
	_downHotspot.setHeight(downSize);
	_downHotspot.translate(0, -downSize);

	_rightHotspot.left = _screenPosition.right;
	_rightHotspot.setWidth(rightSize);
	_rightHotspot.translate(-rightSize, 0);
}

void Viewport::disableEdges(byte edges) {
	_edgesMask |= edges;
}

void Viewport::enableEdges(byte edges) {
	_edgesMask &= ~edges;
}

} // End of namespace UI
} // End of namespace Nancy
