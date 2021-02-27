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

#include "engines/nancy/action/rotatinglockpuzzle.h"

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/state/scene.h"

#include "common/random.h"

namespace Nancy {
namespace Action {

void RotatingLockPuzzle::init() {
    _drawSurface.create(_screenPosition.width(), _screenPosition.height(), GraphicsManager::pixelFormat);
    _drawSurface.clear(GraphicsManager::transColor);

    Graphics::Surface surf;
    _engine->_res->loadImage("ciftree", imageName, surf);
    image.create(surf.w, surf.h, surf.format);
    image.blitFrom(surf);
    surf.free();
}

uint16 RotatingLockPuzzle::readData(Common::SeekableReadStream &stream) {
    char buf[10];
    stream.read(buf, 10);
    imageName = buf;

    uint numDials = stream.readUint16LE();

    for (uint i = 0; i < 10; ++i) {
        srcRects.push_back(Common::Rect());
        readRect(stream, srcRects.back());
    }

    for (uint i = 0; i < numDials; ++i) {
        destRects.push_back(Common::Rect());
        readRect(stream, destRects.back());

        if (i == 0) {
            _screenPosition = destRects.back();
        } else {
            _screenPosition.extend(destRects.back());
        }
    }

    stream.skip((8 - numDials) * 16);

    for (uint i = 0; i < numDials; ++i) {
        upHotspots.push_back(Common::Rect());
        readRect(stream, upHotspots.back());
    }

    stream.skip((8 - numDials) * 16);

    for (uint i = 0; i < numDials; ++i) {
        downHotspots.push_back(Common::Rect());
        readRect(stream, downHotspots.back());
    }

    stream.skip((8 - numDials) * 16);

    for (uint i = 0; i < numDials; ++i) {
        correctSequence.push_back(stream.readByte());
    }

    stream.skip(8 - numDials);

    clickSound.read(stream, SoundManager::SoundDescription::kNormal);
    solveExitScene.readData(stream);
    stream.skip(2); // shouldStopRendering, useless
    flagOnSolve.label = stream.readSint16LE();
    flagOnSolve.flag = (NancyFlag)stream.readByte();
    solveSoundDelay = stream.readUint16LE();
    solveSound.read(stream, SoundManager::SoundDescription::kNormal);
    exitScene.readData(stream);
    stream.skip(2); // shouldStopRendering, useless
    flagOnExit.label = stream.readSint16LE();
    flagOnExit.flag = (NancyFlag)stream.readByte();
    readRect(stream, exitHotspot);

    return 0x2A4;
}

void RotatingLockPuzzle::execute(Nancy::NancyEngine *engine) {
    switch (state) {
        case kBegin:
            init();
            registerGraphics();

            for (uint i = 0; i < correctSequence.size(); ++i) {
                currentSequence.push_back(_engine->_rnd->getRandomNumber(9));
                drawDial(i);
            }

            _engine->sound->loadSound(clickSound);
            _engine->sound->loadSound(solveSound);
            state = kRun;
            // fall through
        case kRun:
            switch (solveState) {
                case kNotSolved:
                    for (uint i = 0; i < correctSequence.size(); ++i) {
                        if (currentSequence[i] != (int16)correctSequence[i]) {
                            return;
                        }
                    }

                    _engine->scene->setEventFlag(flagOnSolve.label, flagOnSolve.flag);
                    solveSoundPlayTime = _engine->getTotalPlayTime() + solveSoundDelay * 1000;
                    solveState = kPlaySound;
                    // fall through
                case kPlaySound:
                    if (_engine->getTotalPlayTime() <= solveSoundPlayTime) {
                        break;
                    }

                    _engine->sound->playSound(solveSound.channelID);
                    solveState = kWaitForSound;
                    break;
                case kWaitForSound:
                    if (!_engine->sound->isSoundPlaying(solveSound.channelID)) {
                        state = kActionTrigger;
                    }

                    break;
            }
            break;
        case kActionTrigger:
            _engine->sound->stopSound(clickSound.channelID);
            _engine->sound->stopSound(solveSound.channelID);

            if (solveState == kNotSolved) {
                if (exitScene.sceneID != 9999) {
                    _engine->scene->changeScene(exitScene.sceneID, exitScene.frameID, exitScene.verticalOffset, exitScene.doNotStartSound);
                    _engine->scene->setEventFlag(flagOnExit.label, flagOnExit.flag);
                }
            } else {
                if (solveExitScene.sceneID != 9999) {
                    _engine->scene->changeScene(solveExitScene.sceneID, solveExitScene.frameID, solveExitScene.verticalOffset, solveExitScene.doNotStartSound);
                }
            }

            isDone = true;
    }
}

void RotatingLockPuzzle::handleInput(NancyInput &input) {
    if (solveState != kNotSolved) {
        return;
    }

    if (_engine->scene->getViewport().convertViewportToScreen(exitHotspot).contains(input.mousePos)) {
        _engine->cursorManager->setCursorType(CursorManager::kExitArrow);

        if (input.input & NancyInput::kLeftMouseButtonUp) {
            state = kActionTrigger;
        }
        return;
    }

    for (uint i = 0; i < upHotspots.size(); ++i) {
        if (_engine->scene->getViewport().convertViewportToScreen(upHotspots[i]).contains(input.mousePos)) {
            _engine->cursorManager->setCursorType(CursorManager::kHotspot);

            if (input.input & NancyInput::kLeftMouseButtonUp) {
                _engine->sound->playSound(clickSound.channelID);
                
                currentSequence[i] = ++currentSequence[i] > 9 ? 0 : currentSequence[i];
                drawDial(i);
            }
            return;
        }
    }

    for (uint i = 0; i < downHotspots.size(); ++i) {
        if (_engine->scene->getViewport().convertViewportToScreen(downHotspots[i]).contains(input.mousePos)) {
            _engine->cursorManager->setCursorType(CursorManager::kHotspot);

            if (input.input & NancyInput::kLeftMouseButtonUp) {
                _engine->sound->playSound(clickSound.channelID);

                int8 n = currentSequence[i];
                n = --n < 0 ? 9 : n;
                currentSequence[i] = n;
                drawDial(i);
            }
            return;
        }
    }
}

void RotatingLockPuzzle::drawDial(uint id) {
    Common::Point destPoint(destRects[id].left - _screenPosition.left, destRects[id].top - _screenPosition.top);
    _drawSurface.blitFrom(image, srcRects[currentSequence[id]], destPoint);

    _needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
