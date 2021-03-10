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

#include "engines/nancy/action/secondarymovie.h"

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

PlaySecondaryMovie::~PlaySecondaryMovie() {
    _decoder.close();
    
    if (hideMouse == kTrue && unknown == 5) {
        NanEngine.setMouseEnabled(true);
    }
}

uint16 PlaySecondaryMovie::readData(Common::SeekableReadStream &stream) {  
    char name[10];
    stream.read(name, 10);
    videoName = name;

    stream.skip(0x12);
    unknown = stream.readUint16LE();
	hideMouse = (NancyFlag)stream.readUint16LE();
    isReverse = (NancyFlag)stream.readUint16LE();
    firstFrame = (NancyFlag)stream.readUint16LE();
    lastFrame = (NancyFlag)stream.readUint16LE();

    for (uint i = 0; i < 15; ++i) {
        frameFlags[i].frameID = stream.readSint16LE();
        frameFlags[i].flagDesc.label = stream.readSint16LE();
        frameFlags[i].flagDesc.flag = (NancyFlag)stream.readUint16LE();
    }

    triggerFlags.readData(stream);
    sound.read(stream, SoundDescription::kNormal);
    sceneChange.readData(stream);

    uint16 numVideoDescs = stream.readUint16LE();
    for (uint i = 0; i < numVideoDescs; ++i) {
        videoDescs.push_back(SecondaryVideoDescription());
        videoDescs[i].readData(stream);
    }

    return 0xD4 + numVideoDescs * 0x42; // TODO
}

void PlaySecondaryMovie::init() {
    if (_decoder.isVideoLoaded()) {
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

    if (!_decoder.isPlaying() && _isVisible && !isFinished) {
        _decoder.start();

		if (isReverse == kTrue) {
			_decoder.setRate(-_decoder.getRate());
			_decoder.seekToFrame(lastFrame);
		} else {
			_decoder.seekToFrame(firstFrame);
		}
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
        
        for (auto f : frameFlags) {
            if (_decoder.getCurFrame() == f.frameID) {
                NancySceneState.setEventFlag(f.flagDesc);
            }
        }
    }

	if ((_decoder.getCurFrame() == lastFrame && isReverse == kFalse) ||
	    (_decoder.getCurFrame() == firstFrame && isReverse == kTrue)) {
		if (!NanEngine.sound->isSoundPlaying(sound)) {
			NanEngine.sound->stopSound(sound);
			_decoder.stop();
            isFinished = true;
			state = kActionTrigger;
		}
	}

    RenderObject::updateGraphics();
}

void PlaySecondaryMovie::onPause(bool pause) {
    _decoder.pauseVideo(pause);

    if (pause) {
        registerGraphics();
    }
}

void PlaySecondaryMovie::execute() {
    switch (state) {
    case kBegin:
        init();
        registerGraphics();
        NanEngine.sound->loadSound(sound);
        NanEngine.sound->playSound(sound);

        if (hideMouse == kTrue) {
            NanEngine.setMouseEnabled(false);
        }

        state = kRun;
        // fall through
    case kRun: {
        int newFrame = NancySceneState.getSceneInfo().frameID;

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
                setVisible(true);
            } else {
                setVisible(false);
            }
        }

        break;
    }
    case kActionTrigger:
        triggerFlags.execute();
        if (unknown == 5) {
            NancySceneState.changeScene(sceneChange);
        } else {
            // Not changing the scene so enable the mouse now
            if (hideMouse == kTrue) {
                NanEngine.setMouseEnabled(true);
            }
        }

        finishExecution();
        break;
    }
}

} // End of namespace Action
} // End of namespace Nancy
