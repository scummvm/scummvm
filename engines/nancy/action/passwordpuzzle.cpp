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

#include "engines/nancy/action/passwordpuzzle.h"

#include "engines/nancy/util.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/state/scene.h"

#include "graphics/font.h"

namespace Nancy {
namespace Action {

void PasswordPuzzle::init() {
    _drawSurface.create(_screenPosition.width(), _screenPosition.height(), GraphicsManager::getInputPixelFormat());
    _drawSurface.clear(GraphicsManager::getTransColor());
    
    setTransparent(true);

    RenderObject::init();
}

void PasswordPuzzle::readData(Common::SeekableReadStream &stream) {
    fontID = stream.readUint16LE();
    cursorBlinkTime = stream.readUint16LE();
    readRect(stream, nameBounds);
    readRect(stream, passwordBounds);
    readRect(stream, _screenPosition);

    char buf[20];
    stream.read(buf, 20);
    name = buf;
    stream.read(buf, 20);
    password = buf;
    solveExitScene.readData(stream);
    stream.skip(2);
    flagOnSolve.label = stream.readSint16LE();
    flagOnSolve.flag = (NancyFlag)stream.readByte();
    solveSound.read(stream, SoundDescription::kNormal);
    failExitScene.readData(stream);
    stream.skip(2);
    flagOnFail.label = stream.readSint16LE();
    flagOnFail.flag = (NancyFlag)stream.readByte();
    failSound.read(stream, SoundDescription::kNormal);
    exitScene.readData(stream);
    stream.skip(2);
    flagOnExit.label = stream.readSint16LE();
    flagOnExit.flag = (NancyFlag)stream.readByte();
    readRect(stream, exitHotspot);
}

void PasswordPuzzle::execute() {
    switch (state) {
    case kBegin:
        init();
        registerGraphics();
        nextBlinkTime = g_nancy->getTotalPlayTime() + cursorBlinkTime;
        state = kRun;
        // fall through
    case kRun:
        switch (solveState) {
        case kNotSolved: {
            Common::String &activeField = passwordFieldIsActive ? playerPasswordInput : playerNameInput;
            Common::String &correctField = passwordFieldIsActive ? password : name;
            Time currentTime = g_nancy->getTotalPlayTime();

            if (playerHasHitReturn) {
                playerHasHitReturn = false;

                if (activeField.lastChar() == '-') {
                    activeField.deleteLastChar();
                    drawText();
                }

                if (activeField.equalsIgnoreCase(correctField)) {
                    if (!passwordFieldIsActive) {
                        passwordFieldIsActive = true;
                    } else {
                        g_nancy->sound->loadSound(solveSound);
                        g_nancy->sound->playSound(solveSound);
                        solveState = kSolved;
                    }
                } else {
                    g_nancy->sound->loadSound(failSound);
                    g_nancy->sound->playSound(failSound);
                    solveState = kFailed;
                }
                
                
            } else if (currentTime >= nextBlinkTime) {
                nextBlinkTime = currentTime + cursorBlinkTime;

                if (activeField.size() && activeField.lastChar() == '-') {
                    activeField.deleteLastChar();
                } else {
                    activeField += '-';
                }

                drawText();
            }

            break;
        }
        case kFailed:
            if (!g_nancy->sound->isSoundPlaying(failSound)) {
                g_nancy->sound->stopSound(failSound);
                state = kActionTrigger;
            }

            break;
        case kSolved:
            if (!g_nancy->sound->isSoundPlaying(solveSound)) {
                g_nancy->sound->stopSound(solveSound);
                state = kActionTrigger;
            }

            break;
        }

        break;
    case kActionTrigger:
        switch (solveState) {
        case kNotSolved:
            NancySceneState.changeScene(exitScene);
            NancySceneState.setEventFlag(flagOnExit);
            break;
        case kFailed:
            NancySceneState.changeScene(failExitScene);
            NancySceneState.setEventFlag(flagOnFail.label);
            break;
        case kSolved:
            NancySceneState.changeScene(solveExitScene);
            NancySceneState.setEventFlag(flagOnSolve.label);
            break;
        }

        finishExecution();
    }
}

void PasswordPuzzle::handleInput(NancyInput &input) {
    if (solveState != kNotSolved) {
        return;
    }

    if (NancySceneState.getViewport().convertViewportToScreen(exitHotspot).contains(input.mousePos)) {
        g_nancy->cursorManager->setCursorType(CursorManager::kExitArrow);

        if (input.input & NancyInput::kLeftMouseButtonUp) {
            state = kActionTrigger;
        }
        
        return;
    }

    for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
        Common::KeyState &key = input.otherKbdInput[i];
        Common::String &activeField = passwordFieldIsActive ? playerPasswordInput : playerNameInput;
        Common::String &correctField = passwordFieldIsActive ? password : name;
        if (key.keycode == Common::KEYCODE_BACKSPACE) {
            if (activeField.size() && activeField.lastChar() == '-' ? activeField.size() > 1 : true) {
                if (activeField.lastChar() == '-') {
                    activeField.deleteChar(activeField.size() -2);
                } else {
                    activeField.deleteLastChar();
                }

                drawText();
            }
        } else if (key.keycode == Common::KEYCODE_RETURN) {
            playerHasHitReturn = true;
        } else if (Common::isAlnum(key.ascii) || Common::isSpace(key.ascii)) {
            if (activeField.size() && activeField.lastChar() == '-') {
                if (activeField.size() <= correctField.size() + 2) {
                    activeField.deleteLastChar();
                    activeField += key.ascii;
                    activeField += '-';
                }
            } else {
                if (activeField.size() <= correctField.size() + 1) {
                    activeField += key.ascii;
                }
            }

            drawText();
        }
    }
}

void PasswordPuzzle::onPause(bool pause) {
    if (pause) {
        registerGraphics();
    }
}

void PasswordPuzzle::drawText() {
    _drawSurface.clear(GraphicsManager::getTransColor());
    Graphics::Font *font = g_nancy->graphicsManager->getFont(fontID);

    Common::Rect bounds = nameBounds;
    bounds = NancySceneState.getViewport().convertViewportToScreen(bounds);
    bounds = convertToLocal(bounds);
    Common::Point destPoint(bounds.left, bounds.bottom + 1 - font->getFontHeight());
    font->drawString(&_drawSurface, playerNameInput, destPoint.x, destPoint.y, bounds.width(), 0);

    bounds = passwordBounds;
    bounds = NancySceneState.getViewport().convertViewportToScreen(bounds);
    bounds = convertToLocal(bounds);
    destPoint.x = bounds.left;
    destPoint.y = bounds.bottom + 1 - font->getFontHeight();
    font->drawString(&_drawSurface, playerPasswordInput, destPoint.x, destPoint.y, bounds.width(), 0);

    _needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
