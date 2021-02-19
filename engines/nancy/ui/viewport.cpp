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

#include "engines/nancy/ui/viewport.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

namespace Nancy {
namespace UI {

// does NOT put the object in a valid state until loadVideo is called
void Viewport::init() {
    Common::SeekableReadStream *viewChunk = _engine->getBootChunkStream("VIEW");
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
    Time playTime = _engine->getTotalPlayTime();
    byte direction = 0;

    if (_screenPosition.contains(input.mousePos)) {
        _engine->cursorManager->setCursorType(CursorManager::kNormal);
    }

    if (_upHotspot.contains(input.mousePos)) {
        direction |= kUp;
    } else if (_downHotspot.contains(input.mousePos)) {
        direction |= kDown;
    }

    if (_leftHotspot.contains(input.mousePos)) {
        direction |= kLeft;
    } else if (_rightHotspot.contains(input.mousePos)) {
        direction |= kRight;
    }

    if (direction) {
        _engine->cursorManager->setCursorType(CursorManager::kMove);

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
        const Nancy::State::Scene::SceneSummary &summary = _engine->scene->getSceneSummary();
        Time movementDelta = _engine->scene->getMovementTimeDelta(direction & kMoveFast);

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
        } else if (direction != _movementLastFrame) {
            _nextMovementTime = playTime + movementDelta;
        }
    }

    _movementLastFrame = direction;
}

void Viewport::loadVideo(const Common::String &filename, uint frameNr, uint verticalScroll) {
    if (_decoder.isVideoLoaded()) {
        _decoder.close();
    }
    _decoder.loadFile(filename + ".avf");
    
    setFrame(frameNr);
    setVerticalScroll(verticalScroll); 
}

void Viewport::setFrame(uint frameNr) {
    assert(frameNr < _decoder.getFrameCount());

    const Graphics::Surface *newFrame = _decoder.decodeFrame(frameNr);

    if (_fullFrame.w != newFrame->w || _fullFrame.h != newFrame->h) {
        _fullFrame.create(newFrame->w, newFrame->h, GraphicsManager::pixelFormat);
    }

    _fullFrame.blitFrom(*newFrame);
    _needsRedraw = true;

    _engine->scene->getSceneInfo().frameID = frameNr;
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
    
    _engine->scene->getSceneInfo().verticalOffset = scroll;
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
    return ret;
}

// Convert a screen-space coordinate to viewport coordinates
Common::Rect Viewport::convertScreenToViewport(const Common::Rect &viewportRect) const {
    Common::Rect ret = convertToLocal(viewportRect);
    ret.translate(0, getCurVerticalScroll());
    return ret;
}

void Viewport::setEdges(int16 upSize, int16 downSize, int16 leftSize, int16 rightSize) {
    if (upSize > -1) {
        _upHotspot.setHeight(upSize);
    }

    if (leftSize > -1) {
        _leftHotspot.setWidth(leftSize);
    }

    if (downSize > -1) {
        _downHotspot.top = _screenPosition.bottom;
        _downHotspot.setHeight(downSize);
        _downHotspot.translate(0, -downSize);
    }

    if (rightSize > -1) {
        _rightHotspot.left = _screenPosition.right;
        _rightHotspot.setWidth(rightSize);
        _rightHotspot.translate(-rightSize, 0);
    }
}

} // End of namespace UI
} // End of namespace Nancy