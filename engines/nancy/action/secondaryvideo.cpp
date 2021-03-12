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
#include "engines/nancy/sound.h"

#include "common/system.h"
#include "common/events.h"

namespace Nancy {
namespace Action {

void PlaySecondaryVideo::init() {
    if(_decoder.isVideoLoaded()) {
        _decoder.close();
    }

    _decoder.loadFile(filename + ".avf");
    // Every secondary video frame (in nancy1) plays exactly 12ms slower than what its metadata says.
    // I'm still not sure how/why that happens so for now I'm using this hack to fix the timings
    _decoder.addFrameTime(12);
    _drawSurface.create(_decoder.getWidth(), _decoder.getHeight(), GraphicsManager::getInputPixelFormat());

    if (paletteFilename.size()) {
        GraphicsManager::loadSurfacePalette(_drawSurface, paletteFilename);
    }

    setVisible(false);
    setTransparent(true);

    RenderObject::init();
}

void PlaySecondaryVideo::updateGraphics() {
    if (!_decoder.isVideoLoaded()) {
        return;
    }

    if (_isPlaying) {
        if (!_decoder.isPlaying()) {
            _decoder.start();
        }
        
        switch (hoverState) {
        case kNoHover:
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

    if (pause) {
        registerGraphics();
    }
}

void PlaySecondaryVideo::handleInput(NancyInput &input) {
    if (hasHotspot && NancySceneState.getViewport().convertViewportToScreen(hotspot).contains(input.mousePos)) {
        _isHovered = true;
    } else {
        _isHovered = false;
    }
}

void PlaySecondaryVideo::readData(Common::SeekableReadStream &stream) {
    char buf[10];
    stream.read(buf, 10);
    filename = buf;
    stream.read(buf, 10);
    paletteFilename = buf;
    stream.skip(10);
    
    if (paletteFilename.size()) {
        stream.skip(14); // unknown data
    }

    loopFirstFrame = stream.readUint16LE();
    loopLastFrame = stream.readUint16LE();
    onHoverFirstFrame = stream.readUint16LE();
    onHoverLastFrame = stream.readUint16LE();
    onHoverEndFirstFrame = stream.readUint16LE();
    onHoverEndLastFrame = stream.readUint16LE();

    sceneChange.readData(stream);

    if (paletteFilename.size()) {
        stream.skip(3);
    } else {
        stream.skip(1);
    }

    uint16 numVideoDescs = stream.readUint16LE();
    for (uint i = 0; i < numVideoDescs; ++i) {
        videoDescs.push_back(SecondaryVideoDescription());
        videoDescs[i].readData(stream);
    }
}

void PlaySecondaryVideo::execute() {
    switch (state) {
    case kBegin:
        init();
        registerGraphics();
        state = kRun;
        // fall through
    case kRun: {
        // Set correct position according to viewport frame
        if (_currentViewportFrame != NancySceneState.getSceneInfo().frameID) {
            _currentViewportFrame = NancySceneState.getSceneInfo().frameID;

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
        NancySceneState.pushScene();
        NancySceneState.changeScene(sceneChange);
        finishExecution();
        break;
    }
}

} // End of namespace Action
} // End of namespace Nancy
