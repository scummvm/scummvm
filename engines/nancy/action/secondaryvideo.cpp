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

#include "engines/nancy/action/secondaryvideo.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/viewport.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/graphics.h"

#include "common/system.h"

#include "common/events.h"

namespace Nancy {
namespace Action {

void SecondaryVideoDesc::readData(Common::SeekableReadStream &stream) {
    frameID = stream.readUint16LE();
    readRect(stream, srcRect);
    readRect(stream, destRect);
    stream.skip(0x20);
}

void PlaySecondaryVideo::init() {
    if(_decoder.isVideoLoaded()) {
        _decoder.close();
    }

    _decoder.loadFile(filename + ".avf");
    // TODO add 12 ms to the frame time, not sure why this happens in the engine yet
    _drawSurface.create(_decoder.getWidth(), _decoder.getHeight(), GraphicsManager::pixelFormat);

    setVisible(false);

    RenderObject::init();
}

void PlaySecondaryVideo::updateGraphics() {
    if (!_decoder.isVideoLoaded()) {
        return;
    }

    if (_isPlaying) {
        switch (hoverState) {
            case kNoHover:
                if (!_decoder.isPlaying()) {
                    _decoder.start();
                }
                if (_isHovered) {
                    _decoder.seekToFrame(onHoverFirstFrame);
                    
                    hoverState = kHover;
                } else {
                    if (_decoder.getCurFrame() == loopLastFrame) {
                        // loop back to beginning
                        _decoder.seekToFrame(loopFirstFrame);
                    }
                    break;
                }
                // fall through
            case kHover:
                if (!_isHovered) {
                    // Stopped hovering, reverse playback
                    _decoder.seekToFrame(onHoverEndLastFrame);
                    _decoder.setRate(-_decoder.getRate());
                    if (!_decoder.isPlaying()) {
                        _decoder.start();
                    }
                    hoverState = kEndHover;
                } else {
                    break;
                }
                // fall through
            case kEndHover:
                if (_decoder.getCurFrame() == onHoverEndFirstFrame) {
                    // reversed playback has ended, go back to no hover state
                    _decoder.seekToFrame(loopFirstFrame);
                    _decoder.setRate(-_decoder.getRate());
                    hoverState = kNoHover;
                }
                break;
        }

        if (_decoder.needsUpdate() && !_screenPosition.isEmpty()) {
            for (uint i = 0; i < videoDescs.size(); ++i) {
                if ((uint16)videoDescs[i].frameID == _currentViewportFrame) {
                    _drawSurface.blitFrom(*_decoder.decodeNextFrame(), videoDescs[i].srcRect, Common::Point());
                    break;
                }
            }
            _needsRedraw = true;
        }
    } else {
        _decoder.seekToFrame(0);
    }

    RenderObject::updateGraphics();
}

void PlaySecondaryVideo::onPause(bool pause) {
    _decoder.pauseVideo(pause);
}

void PlaySecondaryVideo::handleInput(NancyInput &input) {
    if (hasHotspot && _engine->scene->getViewport().convertViewportToScreen(hotspot).contains(input.mousePos)) {
        _isHovered = true;
    } else {
        _isHovered = false;
    }
}

uint16 PlaySecondaryVideo::readData(Common::SeekableReadStream &stream) {
    char buf[10];
    stream.read(buf, 10);
    filename = buf;
    stream.skip(0x14);
    loopFirstFrame = stream.readUint16LE();
    loopLastFrame = stream.readUint16LE();
    onHoverFirstFrame = stream.readUint16LE();
    onHoverLastFrame = stream.readUint16LE();
    onHoverEndFirstFrame = stream.readUint16LE();
    onHoverEndLastFrame = stream.readUint16LE();
    SceneChange::readData(stream);
    stream.skip(1);

    uint16 numVideoDescs = stream.readUint16LE();
    for (uint i = 0; i < numVideoDescs; ++i) {
        videoDescs.push_back(SecondaryVideoDesc());
        videoDescs[i].readData(stream);
    }

    return 0x35 + (numVideoDescs * 0x42);
}

void PlaySecondaryVideo::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            state = kRun;
            // fall through
        case kRun: {
            // Set correct position according to viewport frame
            if (_currentViewportFrame != engine->scene->getSceneInfo().frameID) {
                _currentViewportFrame = engine->scene->getSceneInfo().frameID;

                int activeFrame = -1;

                for (uint i = 0; i < videoDescs.size(); ++i) {
                    if ((uint16)videoDescs[i].frameID == _currentViewportFrame) {
                        activeFrame = i;
                    }
                }

                if (activeFrame != -1) {
                    // Make the drawing destination rectangle valid
                    _screenPosition = videoDescs[activeFrame].destRect;

                    // Activate the hotspot
                    hotspot = videoDescs[activeFrame].destRect;
                    hasHotspot = true;
                    _isPlaying = true;
                    setVisible(true);
                } else {
                    setVisible(false);
                    hasHotspot = false;
                    _isPlaying = false;
                }
            }

            break;
        }
        case kActionTrigger:
            engine->scene->pushScene();
            SceneChange::execute(engine);
            break;
    }
}

uint16 PlaySecondaryMovie::readData(Common::SeekableReadStream &stream) {  
    char name[10];
    stream.read(name, 10);
    videoName = name;

    stream.skip(0x1C);
    for (uint i = 0; i < 15; ++i) {
        frameFlags[i].frameID = stream.readSint16LE();
        frameFlags[i].flagDesc.label = stream.readSint16LE();
        frameFlags[i].flagDesc.flag = (NancyFlag)stream.readUint16LE();
    }

    triggerFlags.readData(stream);
    sound.read(stream, SoundManager::SoundDescription::kNormal);
    SceneChange::readData(stream);

    uint16 numVideoDescs = stream.readUint16LE();
    for (uint i = 0; i < numVideoDescs; ++i) {
        videoDescs.push_back(SecondaryVideoDesc());
        videoDescs[i].readData(stream);
    }

    return 0xD4 + numVideoDescs * 0x42; // TODO
}

void PlaySecondaryMovie::init() {
    if(_decoder.isVideoLoaded()) {
        _decoder.close();
    }
    _decoder.loadFile(videoName + ".avf");
    _drawSurface.create(_decoder.getWidth(), _decoder.getHeight(), GraphicsManager::pixelFormat);
    _screenPosition = _drawSurface.getBounds();

    RenderObject::init();
}

void PlaySecondaryMovie::updateGraphics() {
    if (!_decoder.isVideoLoaded()) {
        return;
    }

    if (!_decoder.isPlaying() && _isVisible) {
        _decoder.start();
    }

    if (_decoder.needsUpdate()) {
        uint descID = 0;
        for (uint i = 0; i < videoDescs.size(); ++i) {
            if (videoDescs[i].frameID == _curViewportFrame) {
                descID = i;
            }
        }
        _drawSurface.blitFrom(*_decoder.decodeNextFrame(), videoDescs[descID].srcRect, Common::Point());
        _needsRedraw = true;
    } else {
        // Set flag if not drawing new frame
        for (auto f : frameFlags) {
            if (_decoder.getCurFrame() == f.frameID) {
                _engine->scene->setEventFlag(f.flagDesc.label, f.flagDesc.flag);
            }
        }
    }

    RenderObject::updateGraphics();
}

void PlaySecondaryMovie::onPause(bool pause) {
    _decoder.pauseVideo(pause);
}

void PlaySecondaryMovie::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            engine->sound->loadSound(sound);
            state = kRun;
            // fall through
        case kRun: {
            engine->cursorManager->showCursor(false);
            
            int newFrame = _engine->scene->getSceneInfo().frameID;

            if (newFrame != _curViewportFrame) {
                _curViewportFrame = newFrame;
                int activeFrame = -1;
                for (uint i = 0; i < videoDescs.size(); ++i) {
                    if (newFrame == videoDescs[i].frameID) {
                        activeFrame = i;
                        break;
                    }
                }

                if (activeFrame != -1) {
                    _screenPosition = videoDescs[activeFrame].destRect;
                    engine->sound->playSound(sound.channelID);
                    setVisible(true);
                } else {
                    setVisible(false);
                }
            }

            break;
        }
        case kActionTrigger:
            triggerFlags.execute(engine);
            SceneChange::execute(engine);
            break;
    }
}

} // End of namespace Action
} // End of namespace Nancy
