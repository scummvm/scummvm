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

#include "engines/nancy/action/telephone.h"

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/cursor.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/textbox.h"

namespace Nancy {
namespace Action {

void Telephone::init() {
    _drawSurface.create(_screenPosition.width(), _screenPosition.height(), GraphicsManager::pixelFormat);
    _drawSurface.clear(GraphicsManager::transColor);

    Graphics::Surface surf;
    _engine->_res->loadImage("ciftree", imageName, surf);
    image.create(surf.w, surf.h, surf.format);
    image.blitFrom(surf);
    surf.free();
}

uint16 Telephone::readData(Common::SeekableReadStream &stream) {
    char buf[10];
    stream.read(buf, 10);
    imageName = buf;

    for (uint i = 0; i < 12; ++i) {
        srcRects.push_back(Common::Rect());
        readRect(stream, srcRects.back());
    }

    for (uint i = 0; i < 12; ++i) {
        destRects.push_back(Common::Rect());
        readRect(stream, destRects.back());

        if (i == 0) {
            _screenPosition = destRects.back();
        } else {
            _screenPosition.extend(destRects.back());
        }
    }

    genericDialogueSound.read(stream, SoundDescription::kNormal);
    genericButtonSound.read(stream, SoundDescription::kNormal);
    ringSound.read(stream, SoundDescription::kNormal);
    dialToneSound.read(stream, SoundDescription::kNormal);
    dialAgainSound.read(stream, SoundDescription::kNormal);
    hangUpSound.read(stream, SoundDescription::kNormal);

    for (uint i = 0; i < 12; ++i) {
        stream.read(buf, 10);
        buttonSoundNames.push_back(buf);
    }

    char buf2[200];
    stream.read(buf2, 200);
    addressBookString = buf2;
    stream.read(buf2, 200);
    dialAgainString = buf2;
    reloadScene.readData(stream);
    stream.skip(2);
    flagOnReload.label = stream.readSint16LE();
    flagOnReload.flag = (NancyFlag)stream.readUint16LE();
    exitScene.readData(stream);
    stream.skip(2);
    flagOnExit.label = stream.readSint16LE();
    flagOnExit.flag = (NancyFlag)stream.readUint16LE();
    readRect(stream, exitHotspot);

    uint numCalls = stream.readUint16LE();

    for (uint i = 0; i < numCalls; ++i) {
        calls.push_back(PhoneCall());
        PhoneCall &call = calls.back();
        for (uint j = 0; j < 11; ++j) {
            call.phoneNumber.push_back(stream.readByte());
        }
        
        stream.read(buf, 10);
        call.soundName = buf;
        stream.read(buf2, 200);
        call.text = buf2;
        call.sceneChange.readData(stream);
        stream.skip(2);
        call.flag.label = stream.readSint16LE();
        call.flag.flag = (NancyFlag)stream.readUint16LE();
    }

    return numCalls * 0xEB + 0x48C;
}

void Telephone::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            _engine->sound->loadSound(dialToneSound);
            _engine->sound->playSound(dialToneSound.channelID);
            _engine->scene->getTextbox().clear();
            _engine->scene->getTextbox().addTextLine(addressBookString);
            state = kRun;
            // fall through
        case kRun:
            switch (callState) {
                case kWaiting:
                    // Long phone numbers start with 1
                    if (calledNumber.size() >= 11 || (calledNumber.size() >= 7 && (calledNumber[0] != 1))) {
                        _engine->scene->getTextbox().clear();
                        _engine->scene->getTextbox().addTextLine("ringing...<n><e>"); // Hardcoded in the original engine
                        _engine->sound->loadSound(ringSound);
                        _engine->sound->playSound(ringSound.channelID);
                        callState = kRinging;
                    }
                    break;
                case kButtonPress:
                    if (!_engine->sound->isSoundPlaying(genericButtonSound.channelID)) {
                        _engine->sound->stopSound(genericButtonSound.channelID);
                        undrawButton(selected);
                        callState = kWaiting;
                    }

                    break;
                case kRinging:
                    if (!_engine->sound->isSoundPlaying(ringSound.channelID)) {
                        _engine->sound->stopSound(ringSound.channelID);

                        uint numberLength = calledNumber[0] == 1 ? 11 : 7;
                        for (uint i = 0; i < calls.size(); ++i) {
                            bool invalid = false;
                            for (uint j = 0; j < numberLength; ++j) {
                                if (calledNumber[j] != calls[i].phoneNumber[j]) {
                                    // Invalid number, move onto next
                                    invalid = true;
                                    break;
                                }
                            }

                            if (invalid) {
                                continue;
                            }

                            _engine->scene->getTextbox().clear();
                            _engine->scene->getTextbox().addTextLine(calls[i].text);

                            genericDialogueSound.name = calls[i].soundName;
                            _engine->sound->loadSound(genericDialogueSound);
                            _engine->sound->playSound(genericDialogueSound.channelID);
                            selected = i;
                            callState = kCall;

                            return;
                        }
                        
                        _engine->scene->getTextbox().clear();
                        _engine->scene->getTextbox().addTextLine(dialAgainString);

                        _engine->sound->loadSound(dialAgainSound);
                        _engine->sound->playSound(dialAgainSound.channelID);
                        callState = kBadNumber;
                        return;
                    }

                    break;
                case kBadNumber:
                    if (!_engine->sound->isSoundPlaying(dialAgainSound.channelID)) {
                        _engine->sound->stopSound(dialAgainSound.channelID);

                        state = kActionTrigger;
                    }

                    break;
                case kCall:
                    if (!_engine->sound->isSoundPlaying(genericDialogueSound.channelID)) {
                        _engine->sound->stopSound(genericDialogueSound.channelID);

                        state = kActionTrigger;
                    }

                    break;
                case kHangUp:
                    if (!_engine->sound->isSoundPlaying(hangUpSound.channelID)) {
                        _engine->sound->stopSound(hangUpSound.channelID);

                        state = kActionTrigger;
                    }

                    break;
            }

            break;
        case kActionTrigger:
            switch (callState) {
                case kBadNumber:
                    engine->scene->changeScene(reloadScene);
                    calledNumber.clear();
                    _engine->scene->setEventFlag(flagOnReload);
                    state = kRun;
                    callState = kWaiting;

                    break;
                case kCall: {
                    PhoneCall &call = calls[selected];
                    _engine->scene->changeScene(call.sceneChange);
                    _engine->scene->setEventFlag(call.flag);

                    break;
                }
                case kHangUp:
                    _engine->scene->changeScene(exitScene);
                    _engine->scene->setEventFlag(flagOnExit);
                    
                    break;
                default:
                    break;

            }

            finishExecution();
            _engine->scene->getTextbox().clear();
    }
}

void Telephone::handleInput(NancyInput &input) {
    int buttonNr = -1;
    // Cursor gets changed regardless of state
    for (uint i = 0; i < 12; ++i) {
        if (_engine->scene->getViewport().convertViewportToScreen(destRects[i]).contains(input.mousePos)) {
            _engine->cursorManager->setCursorType(CursorManager::kHotspot);
            buttonNr = i;
            break;
        }
    }

    if (callState != kWaiting) {
        return;
    }

    if (_engine->scene->getViewport().convertViewportToScreen(exitHotspot).contains(input.mousePos)) {
        _engine->cursorManager->setCursorType(CursorManager::kExitArrow);

        if (input.input & NancyInput::kLeftMouseButtonUp) {
            _engine->sound->loadSound(hangUpSound);
            _engine->sound->playSound(hangUpSound.channelID);

            callState = kHangUp;
        }
        return;
    }

    if (buttonNr != -1) {
        if (input.input & NancyInput::kLeftMouseButtonUp) {
            if (_engine->sound->isSoundPlaying(dialToneSound.channelID)) {
                _engine->sound->stopSound(dialToneSound.channelID);
            }

            calledNumber.push_back(buttonNr);
            genericButtonSound.name = buttonSoundNames[buttonNr];
            _engine->sound->loadSound(genericButtonSound);
            _engine->sound->playSound(genericButtonSound.channelID);

            drawButton(buttonNr);

            selected = buttonNr;

            callState = kButtonPress;
        }
    }
}

void Telephone::drawButton(uint id) {
    Common::Point destPoint(destRects[id].left - _screenPosition.left, destRects[id].top - _screenPosition.top);
    _drawSurface.blitFrom(image, srcRects[id], destPoint);

    _needsRedraw = true;
}

void Telephone::undrawButton(uint id) {
    Common::Rect bounds = destRects[id];
    bounds.translate(-_screenPosition.left, -_screenPosition.top);

    _drawSurface.fillRect(bounds, GraphicsManager::transColor);
    _needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy