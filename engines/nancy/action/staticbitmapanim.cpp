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

#include "engines/nancy/action/staticbitmapanim.h"

#include "engines/nancy/sound.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/resource.h"

#include "common/rational.h"

namespace Nancy {
namespace Action {

void PlayIntStaticBitmapAnimation::init() {
    Graphics::Surface surf;
    _engine->_res->loadImage("ciftree", imageName, surf);

    _fullSurface.create(surf.w, surf.h, surf.format);
    _fullSurface.blitFrom(surf);
    surf.free();
    setFrame(0);

    RenderObject::init();
}

uint16 PlayIntStaticBitmapAnimation::readData(Common::SeekableReadStream &stream) {
    char name[10];
    stream.read(name, 10);
    imageName = Common::String(name);

    stream.skip(0x2);
    isTransparent = (NancyFlag)(stream.readUint16LE());
    doNotChangeScene = (NancyFlag)(stream.readUint16LE());
    isReverse = (NancyFlag)(stream.readUint16LE());
    isLooping = (NancyFlag)(stream.readUint16LE());
    firstFrame = stream.readUint16LE();
    loopFirstFrame = stream.readUint16LE();
    loopLastFrame = stream.readUint16LE();
    frameTime = Common::Rational(1000, stream.readUint16LE()).toInt();
    zOrder = stream.readUint16LE();
    updateCondition.label = stream.readSint16LE();
    updateCondition.flag = (NancyFlag)stream.readUint16LE();
    sceneChange.readData(stream);
    triggerFlags.readData(stream);
    sound.read(stream, SoundDescription::kNormal);
    uint numFrames = stream.readUint16LE();

    for (uint i = firstFrame; i <= loopLastFrame; ++i) {
        srcRects.push_back(Common::Rect());
        readRect(stream, srcRects[i]);
    }

    for (uint i = 0; i < numFrames; ++i) {
        bitmaps.push_back(BitmapDescription());
        BitmapDescription &rects = bitmaps[i];
        rects.frameID = stream.readUint16LE();
        readRect(stream, rects.src);
        readRect(stream, rects.dest);
    }

    return 0x76 + numFrames * 0x22 + (loopLastFrame - firstFrame + 1) * 16;
}

void PlayIntStaticBitmapAnimation::execute(NancyEngine *engine) {
    uint32 currentFrameTime = engine->getTotalPlayTime();
    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            _engine->sound->loadSound(sound);
            _engine->sound->playSound(sound.channelID);
            state = kRun;
            // fall through
        case kRun: {
            // Check the timer to see if we need to draw the next animation frame
            if (nextFrameTime <= currentFrameTime) {
                // World's worst if statement
                if (engine->scene->getEventFlag(updateCondition.label, updateCondition.flag) ||
                    (   (((currentFrame == loopLastFrame) && (isReverse == kFalse) && (isLooping == kFalse)) ||
                        ((currentFrame == loopFirstFrame) && (isReverse == kTrue) && (isLooping == kFalse))) &&
                            !engine->sound->isSoundPlaying(sound.channelID))   ) {
                    
                    state = kActionTrigger;

                    // Not sure if hiding when triggered is a hack or the intended behavior, but it's here to fix
                    // nancy1's safe lock light not turning off.
                    setVisible(false);
        
                    if (!engine->sound->isSoundPlaying(sound.channelID)) {
                        engine->sound->stopSound(sound.channelID);
                    }
                } else {
                    // Check if we've moved the viewport
                    uint16 newFrame = engine->scene->getSceneInfo().frameID;
                    if (currentViewportFrame != newFrame) {
                        currentViewportFrame = newFrame;
                        for (uint i = 0; i < bitmaps.size(); ++i) {
                            if (currentViewportFrame == bitmaps[i].frameID) {
                                _screenPosition = bitmaps[i].dest;
                                break;
                            }
                        }
                    }
                    
                    nextFrameTime = currentFrameTime + frameTime;
                    setFrame(currentFrame);
                    if (isReverse == kTrue) {
                        --currentFrame;
                        currentFrame = currentFrame < loopFirstFrame ? loopLastFrame : currentFrame;
                        return;
                    } else {
                        ++currentFrame;
                        currentFrame = currentFrame > loopLastFrame ? loopFirstFrame : currentFrame;
                        return;
                    }
                }                
            } else {
                // Check if we've moved the viewport
                uint16 newFrame = engine->scene->getSceneInfo().frameID;
                if (currentViewportFrame != newFrame) {
                    currentViewportFrame = newFrame;
                    for (uint i = 0; i < bitmaps.size(); ++i) {
                        if (currentViewportFrame == bitmaps[i].frameID) {
                            _screenPosition = bitmaps[i].dest;
                            break;
                        }
                    }
                }
            }      
            
            break;
        }
        case kActionTrigger:
            triggerFlags.execute(engine);
            if (doNotChangeScene == kFalse) {
                engine->scene->changeScene(sceneChange);
                finishExecution();
            }
            break;
    }
}

void PlayIntStaticBitmapAnimation::setFrame(uint frame) {
    currentFrame = frame;
    _drawSurface.create(_fullSurface, srcRects[frame]);
    _needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
