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

#include "engines/nancy/action/orderingpuzzle.h"

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/viewport.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/sound.h"

#include "graphics/surface.h"

namespace Nancy {
namespace Action {

void OrderingPuzzle::init() {
    // Screen position is initialized in readData and fits exactly the bounds of all elements on screen.
    // This is a hacky way to make this particular action record work with this implementation's graphics manager
    _drawSurface.create(_screenPosition.width(), _screenPosition.height(), GraphicsManager::getInputPixelFormat());
    clearAllElements();
    
    setTransparent(true);

    NanEngine.resource->loadImage(imageName, image);

    setVisible(false);

    RenderObject::init();
}

void OrderingPuzzle::readData(Common::SeekableReadStream &stream) {
    char buf[10];

    stream.read(buf, 10);
    imageName = buf;
    uint16 numElements = stream.readUint16LE();

    for (uint i = 0; i < numElements; ++i) {
        srcRects.push_back(Common::Rect());
        readRect(stream, srcRects.back());
    }
    
    stream.skip(16 * (15 - numElements));

    for (uint i = 0; i < numElements; ++i) {
        destRects.push_back(Common::Rect());
        readRect(stream, destRects.back());

        if (i == 0) {
            _screenPosition = destRects[i];
        } else {
            _screenPosition.extend(destRects[i]);
        }

        drawnElements.push_back(false);
    }

    stream.skip(16 * (15 - numElements));

    sequenceLength = stream.readUint16LE();

    for (uint i = 0; i < 15; ++i) {
        correctSequence.push_back(stream.readByte());
    }

    clickSound.read(stream, SoundDescription::kNormal);
    solveExitScene.readData(stream);
    stream.skip(2); // shouldStopRendering, useless
    flagOnSolve.label = stream.readSint16LE();
    flagOnSolve.flag = (NancyFlag)stream.readByte();
    solveSoundDelay = stream.readUint16LE();
    solveSound.read(stream, SoundDescription::kNormal);
    exitScene.readData(stream);
    stream.skip(2); // shouldStopRendering, useless
    flagOnExit.label = stream.readSint16LE();
    flagOnExit.flag = (NancyFlag)stream.readByte();
    readRect(stream, exitHotspot);
}

void OrderingPuzzle::execute() {
    switch (state) {
    case kBegin:
        init();
        registerGraphics();
        NanEngine.sound->loadSound(clickSound);
        NanEngine.sound->loadSound(solveSound);
        state = kRun;
        // fall through
    case kRun:
        switch (solveState) {
        case kNotSolved:
            if (clickedSequence.size() != sequenceLength) {
                return;
            }

            for (uint i = 0; i < sequenceLength; ++i) {
                if (clickedSequence[i] != (int16)correctSequence[i]) {
                    return;
                }
            }

            NancySceneState.setEventFlag(flagOnSolve);
            solveSoundPlayTime = NanEngine.getTotalPlayTime() + solveSoundDelay * 1000;
            solveState = kPlaySound;
            // fall through
        case kPlaySound:
            if (NanEngine.getTotalPlayTime() <= solveSoundPlayTime) {
                break;
            }

            NanEngine.sound->playSound(solveSound);
            solveState = kWaitForSound;
            break;
        case kWaitForSound:
            if (!NanEngine.sound->isSoundPlaying(solveSound)) {
                state = kActionTrigger;
            }

            break;
        }
        break;
    case kActionTrigger:
        NanEngine.sound->stopSound(clickSound);
        NanEngine.sound->stopSound(solveSound);

        if (solveState == kNotSolved) {
            NancySceneState.changeScene(exitScene);
            NancySceneState.setEventFlag(flagOnExit);
        } else {
            NancySceneState.changeScene(solveExitScene);
        }

        finishExecution();
        break;
    }
}

void OrderingPuzzle::handleInput(NancyInput &input) {
    if (solveState != kNotSolved) {
        return;
    }

    if (NancySceneState.getViewport().convertViewportToScreen(exitHotspot).contains(input.mousePos)) {
        NanEngine.cursorManager->setCursorType(CursorManager::kExitArrow);

        if (input.input & NancyInput::kLeftMouseButtonUp) {
            state = kActionTrigger;
        }
        return;
    }

    for (int i = 0; i < (int)destRects.size(); ++i) {
        if (NancySceneState.getViewport().convertViewportToScreen(destRects[i]).contains(input.mousePos)) {
            NanEngine.cursorManager->setCursorType(CursorManager::kHotspot);

            if (input.input & NancyInput::kLeftMouseButtonUp) {
                NanEngine.sound->playSound(clickSound);
                
                for (uint j = 0; j < clickedSequence.size(); ++j) {
                    if (clickedSequence[j] == i && drawnElements[i] == true) {
                        undrawElement(i);
                        if (clickedSequence.back() == i) {
                            clickedSequence.pop_back();
                        }
                        
                        return;
                    }
                }

                clickedSequence.push_back(i);

                if (clickedSequence.size() > (uint)sequenceLength + 1) {
                    clearAllElements();
                } else {
                    drawElement(i);
                }
            }
            return;
        }
    }
}

void OrderingPuzzle::onPause(bool pause) {
    if (pause) {
        registerGraphics();
    }
}

void OrderingPuzzle::drawElement(uint id) {
    drawnElements[id] = true;
    Common::Point destPoint(destRects[id].left - _screenPosition.left, destRects[id].top - _screenPosition.top);
    _drawSurface.blitFrom(image, srcRects[id], destPoint);
    setVisible(true);
}

void OrderingPuzzle::undrawElement(uint id) {
    drawnElements[id] = false;
    Common::Rect bounds = destRects[id];
    bounds.translate(-_screenPosition.left, -_screenPosition.top);

    _drawSurface.fillRect(bounds, GraphicsManager::getTransColor());
    _needsRedraw = true;
}

void OrderingPuzzle::clearAllElements() {
    _drawSurface.clear(GraphicsManager::getTransColor());
    setVisible(false);
    clickedSequence.clear();
    return;
}

} // End of namespace Action
} // End of namespace Nancy
