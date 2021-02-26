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

#include "engines/nancy/action/sliderpuzzle.h"

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

Common::Array<Common::Array<int16>> SliderPuzzle::playerTileOrder = Common::Array<Common::Array<int16>>();
bool SliderPuzzle::playerHasTriedPuzzle = false;

void SliderPuzzle::init() {
    _drawSurface.create(_screenPosition.width(), _screenPosition.height(), GraphicsManager::pixelFormat);
    _drawSurface.clear(GraphicsManager::transColor);

    Graphics::Surface surf;
    _engine->_res->loadImage("ciftree", imageName, surf);
    image.create(surf.w, surf.h, surf.format);
    image.blitFrom(surf);
    surf.free();
}

uint16 SliderPuzzle::readData(Common::SeekableReadStream &stream) {
    char buf[10];
    stream.read(buf, 10);
    imageName = buf;

    width = stream.readUint16LE();
    height = stream.readUint16LE();

    for (uint y = 0; y < height; ++y) {
        srcRects.push_back(Common::Array<Common::Rect>());
        for (uint x = 0; x < width; ++x) {
            srcRects.back().push_back(Common::Rect());
            readRect(stream, srcRects.back().back());
        }
        stream.skip((6 - width) * 16);
    }

    stream.skip((6 - height) * 6 * 16);

    for (uint y = 0; y < height; ++y) {
        destRects.push_back(Common::Array<Common::Rect>());
        for (uint x = 0; x < width; ++x) {
            destRects.back().push_back(Common::Rect());
            readRect(stream, destRects.back().back());

            if (x == 0 && y == 0) {
                _screenPosition = destRects.back().back();
            } else {
                _screenPosition.extend(destRects.back().back());
            }
        }
        stream.skip((6 - width) * 16);
    }

    stream.skip((6 - height) * 6 * 16);

    for (uint y = 0; y < height; ++y) {
        correctTileOrder.push_back(Common::Array<int16>());
        for (uint x = 0; x < width; ++x) {
            correctTileOrder.back().push_back(stream.readSint16LE());
        }
        stream.skip((6 - width) * 2);
    }

    stream.skip((6 - height) * 6 * 2);

    clickSound.read(stream, SoundManager::SoundDescription::kNormal);
    solveExitScene.readData(stream);
    stream.skip(2);
    flagOnSolve.label = stream.readUint16LE();
    flagOnSolve.flag = (NancyFlag)stream.readByte();
    solveSound.read(stream, SoundManager::SoundDescription::kNormal);
    exitScene.readData(stream);
    stream.skip(2);
    flagOnExit.label = stream.readUint16LE();
    flagOnExit.flag = (NancyFlag)stream.readByte();
    readRect(stream, exitHotspot);

    return 0x544;
}

void SliderPuzzle::execute(Nancy::NancyEngine *engine) {
    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            if (!playerHasTriedPuzzle) {
                Common::SeekableReadStream *spuz = engine->getBootChunkStream("SPUZ");
                playerTileOrder.clear();
                spuz->seek(engine->scene->getDifficulty() * 0x48);
                for (uint y = 0; y < height; ++y) {
                    playerTileOrder.push_back(Common::Array<int16>());
                    for (uint x = 0; x < width; ++x) {
                        playerTileOrder.back().push_back(spuz->readSint16LE());
                    }
                    spuz->skip((6 - width) * 2);
                }
                playerHasTriedPuzzle = true;
            }

            for (uint y = 0; y < height; ++y) {
                for (uint x = 0; x < width; ++x) {
                    if (!srcRects[y][x].isEmpty()) {
                        drawTile(playerTileOrder[y][x], x, y);
                    }
                }
            }

            engine->sound->loadSound(clickSound);
            state = kRun;
            // fall through
        case kRun:
            switch (solveState) {
                case kNotSolved:
                    for (uint y = 0; y < height; ++y) {
                        for (uint x = 0; x < width; ++x) {
                            if (playerTileOrder[y][x] != correctTileOrder[y][x]) {
                                return;
                            }
                        }
                    }

                    engine->sound->loadSound(solveSound);
                    engine->sound->playSound(solveSound.channelID);
                    solveState = kWaitForSound;
                    break;
                case kWaitForSound:
                    if (!engine->sound->isSoundPlaying(solveSound.channelID)) {
                        engine->sound->stopSound(solveSound.channelID);
                        state = kActionTrigger;
                    }

                    break;
            }

            break;
        case kActionTrigger:
            switch (solveState) {
                case kNotSolved:
                    if (exitScene.sceneID != 9999) {
                        engine->scene->changeScene(exitScene.sceneID, exitScene.frameID, exitScene.verticalOffset, exitScene.doNotStartSound);
                    }

                    engine->scene->setEventFlag(flagOnExit.label, flagOnExit.flag);
                    break;
                case kWaitForSound:
                    if (exitScene.sceneID != 9999) {
                        engine->scene->changeScene(solveExitScene.sceneID, solveExitScene.frameID, solveExitScene.verticalOffset, solveExitScene.doNotStartSound);
                    }

                    engine->scene->setEventFlag(flagOnSolve.label, flagOnSolve.flag);
                    playerHasTriedPuzzle = false;
                    break;
            }

            engine->sound->stopSound(clickSound.channelID);
            isDone = true;
    }
}

void SliderPuzzle::handleInput(NancyInput &input) {
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

    int currentTileX = -1;
    int currentTileY = -1;
    uint direction = 0;
    for (uint y = 0; y < height; ++y) {
        bool shouldBreak = false;
        for (uint x = 0; x < width; ++x) {
            if (x > 0 && playerTileOrder[y][x - 1] < 0) {
                if (_engine->scene->getViewport().convertViewportToScreen(destRects[y][x]).contains(input.mousePos)) {
                    currentTileX = x;
                    currentTileY = y;
                    direction = kLeft;
                    shouldBreak = true;
                    break;
                }
            } else if ((int)x < width - 1 && playerTileOrder[y][x + 1] < 0) {
                if (_engine->scene->getViewport().convertViewportToScreen(destRects[y][x]).contains(input.mousePos)) {
                    currentTileX = x;
                    currentTileY = y;
                    direction = kRight;
                    shouldBreak = true;
                    break;
                }
            } else if (y > 0 && playerTileOrder[y - 1][x] < 0) {
                if (_engine->scene->getViewport().convertViewportToScreen(destRects[y][x]).contains(input.mousePos)) {
                    currentTileX = x;
                    currentTileY = y;
                    direction = kUp;
                    shouldBreak = true;
                    break;
                }
            } else if ((int)y < height - 1 && playerTileOrder[y + 1][x] < 0) {
                if (_engine->scene->getViewport().convertViewportToScreen(destRects[y][x]).contains(input.mousePos)) {
                    currentTileX = x;
                    currentTileY = y;
                    direction = kDown;
                    shouldBreak = true;
                    break;
                }
            }
        }

        if (shouldBreak) {
            break;
        }
    }

    if (currentTileX != -1) {
        _engine->cursorManager->setCursorType(CursorManager::kHotspot);

        if (input.input & NancyInput::kLeftMouseButtonUp) {
            _engine->sound->playSound(clickSound.channelID);
            switch (direction) {
                case kUp: {
                    uint curTileID = playerTileOrder[currentTileY][currentTileX];
                    drawTile(curTileID, currentTileX, currentTileY - 1);
                    undrawTile(currentTileX, currentTileY);
                    playerTileOrder[currentTileY - 1][currentTileX] = curTileID;
                    playerTileOrder[currentTileY][currentTileX] = -10;
                    break;
                }
                case kDown: {
                    uint curTileID = playerTileOrder[currentTileY][currentTileX];
                    drawTile(curTileID, currentTileX, currentTileY + 1);
                    undrawTile(currentTileX, currentTileY);
                    playerTileOrder[currentTileY + 1][currentTileX] = curTileID;
                    playerTileOrder[currentTileY][currentTileX] = -10;
                    break;
                }
                case kLeft: {
                    uint curTileID = playerTileOrder[currentTileY][currentTileX];
                    drawTile(curTileID, currentTileX - 1, currentTileY);
                    undrawTile(currentTileX, currentTileY);
                    playerTileOrder[currentTileY][currentTileX - 1] = curTileID;
                    playerTileOrder[currentTileY][currentTileX] = -10;
                    break;
                }
                case kRight: {
                    uint curTileID = playerTileOrder[currentTileY][currentTileX];
                    drawTile(curTileID, currentTileX + 1, currentTileY);
                    undrawTile(currentTileX, currentTileY);
                    playerTileOrder[currentTileY][currentTileX + 1] = curTileID;
                    playerTileOrder[currentTileY][currentTileX] = -10;
                    break;
                }
            }
        }
    }
}

void SliderPuzzle::drawTile(uint tileID, uint posX, uint posY) {
    Common::Point destPoint(destRects[posY][posX].left - _screenPosition.left, destRects[posY][posX].top - _screenPosition.top);
    _drawSurface.blitFrom(image, srcRects[tileID / height][tileID % width], destPoint);

    _needsRedraw = true;
}

void SliderPuzzle::undrawTile(uint posX, uint posY) {
    Common::Rect bounds = destRects[posY][posX];
    bounds.translate(-_screenPosition.left, -_screenPosition.top);
    _drawSurface.fillRect(bounds, GraphicsManager::transColor);

    _needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
