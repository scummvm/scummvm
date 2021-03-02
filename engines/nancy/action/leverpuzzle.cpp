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

#include "engines/nancy/action/leverpuzzle.h"

#include "engines/nancy/util.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void LeverPuzzle::init() {
    _drawSurface.create(_screenPosition.width(), _screenPosition.height(), GraphicsManager::pixelFormat);
    _drawSurface.clear(GraphicsManager::transColor);

    Graphics::Surface surf;
    _engine->_res->loadImage("ciftree", imageName, surf);
    image.create(surf.w, surf.h, surf.format);
    image.blitFrom(surf);
    surf.free();
}

uint16 LeverPuzzle::readData(Common::SeekableReadStream &stream) {
    char buf[10];
    stream.read(buf, 10);
    imageName = buf;

    for (uint leverID = 0; leverID < 3; ++leverID) {
        srcRects.push_back(Common::Array<Common::Rect>());
        for (uint i = 0; i < 4; ++i) {
            srcRects.back().push_back(Common::Rect());
            readRect(stream, srcRects.back().back());
        }
    }

    for (uint leverID = 0; leverID < 3; ++leverID) {
        destRects.push_back(Common::Rect());
        readRect(stream, destRects.back());

        if (leverID == 0) {
            _screenPosition = destRects.back();
        } else {
            _screenPosition.extend(destRects.back());
        }
    }

    for (uint leverID = 0; leverID < 3; ++leverID) {
        playerSequence.push_back(stream.readByte());
        leverDirection.push_back(true);
    }

    for (uint leverID = 0; leverID < 3; ++leverID) {
        correctSequence.push_back(stream.readByte());
    }

    moveSound.read(stream, SoundDescription::kNormal);
    noMoveSound.read(stream, SoundDescription::kNormal);
    solveExitScene.readData(stream);
    stream.skip(2);
    flagOnSolve.label = stream.readSint16LE();
    flagOnSolve.flag = (NancyFlag)stream.readByte();
    solveSoundDelay = stream.readUint16LE();
    solveSound.read(stream, SoundDescription::kNormal);
    exitScene.readData(stream);
    stream.skip(2);
    flagOnExit.label = stream.readSint16LE();
    flagOnExit.flag = (NancyFlag)stream.readByte();
    readRect(stream, exitHotspot);

    return 0x192;
}

void LeverPuzzle::execute(Nancy::NancyEngine *engine) {
    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            engine->sound->loadSound(moveSound);
            engine->sound->loadSound(noMoveSound);

            for (uint i = 0; i < 3; ++i) {
                drawLever(i);
            }

            state = kRun;
            // fall through
        case kRun:
            switch (solveState) {
                case kNotSolved:
                    for (uint i = 0; i < 3; ++i) {
                        if (playerSequence[i] != correctSequence[i]) {
                            return;
                        }
                    }
                    
                    engine->scene->setEventFlag(flagOnSolve);
                    solveSoundPlayTime = _engine->getTotalPlayTime() + solveSoundDelay * 1000;
                    solveState = kPlaySound;
                    break;
                case kPlaySound:
                    if (_engine->getTotalPlayTime() <= solveSoundPlayTime) {
                        break;
                    }

                    engine->sound->loadSound(solveSound);
                    _engine->sound->playSound(solveSound);
                    solveState = kWaitForSound;
                    break;
                case kWaitForSound:
                    if (!_engine->sound->isSoundPlaying(solveSound)) {
                        _engine->sound->stopSound(solveSound);
                        state = kActionTrigger;
                    }

                    break;
            }

            break;
        case kActionTrigger:
            _engine->sound->stopSound(moveSound);
            _engine->sound->stopSound(noMoveSound);
            
            if (solveState == kNotSolved) {
                _engine->scene->changeScene(exitScene);
                _engine->scene->setEventFlag(flagOnExit);
            } else {
                _engine->scene->changeScene(solveExitScene);
            }

            finishExecution();
    }
}

void LeverPuzzle::handleInput(NancyInput &input) {
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

    for (uint i = 0; i < 3; ++i) {
        if (_engine->scene->getViewport().convertViewportToScreen(destRects[i]).contains(input.mousePos)) {
            _engine->cursorManager->setCursorType(CursorManager::kHotspot);
            
            if (input.input & NancyInput::kLeftMouseButtonUp) {
                bool isMoving = false;
                // Hardcoded by the original engine
                switch (i) {
                    case 0:
                        isMoving = true;
                        break;
                    case 1:
                        if (playerSequence[0] == 1) {
                            isMoving = true;
                        }
                        break;
                    case 2:
                        if (playerSequence[0] == 2) {
                            isMoving = true;
                        }
                        break;
                }

                if (isMoving) {
                    _engine->sound->playSound(moveSound);

                    if (leverDirection[i]) {
                        // Moving down
                        if (playerSequence[i] == 3) {
                            --playerSequence[i];
                            leverDirection[i] = false;
                        } else {
                            ++playerSequence[i];
                        }
                    } else {
                        // Moving up
                        if (playerSequence[i] == 0) {
                            ++playerSequence[i];
                            leverDirection[i] = true;
                        } else {
                            --playerSequence[i];
                        }
                    }

                    drawLever(i);
                } else {
                    _engine->sound->playSound(noMoveSound);
                    return;
                }
            }
        }
    }
}

void LeverPuzzle::drawLever(uint id) {
    Common::Point destPoint(destRects[id].left - _screenPosition.left, destRects[id].top - _screenPosition.top);
    _drawSurface.blitFrom(image, srcRects[id][playerSequence[id]], destPoint);
    
    _needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
