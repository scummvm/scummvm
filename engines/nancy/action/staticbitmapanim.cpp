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
    setFrame(0);

    RenderObject::init();
}

uint16 PlayIntStaticBitmapAnimation::readData(Common::SeekableReadStream &stream) {
    uint beginOffset = stream.pos();
    char name[10];
    stream.read(name, 10);
    imageName = Common::String(name);

    stream.skip(0xA);
    firstFrame = stream.readUint16LE();
    stream.skip(2);
    lastFrame = stream.readUint16LE();
    frameTime = Common::Rational(1000, stream.readUint16LE()).toInt();
    stream.skip(2);
    soundFlagDesc.label = stream.readSint16LE();
    soundFlagDesc.flag = (NancyFlag)stream.readUint16LE();

    SceneChange::readData(stream);

    triggerFlags.readData(stream);

    stream.read(name, 10);
    soundName = name;
    channelID = stream.readUint16LE();

    stream.seek(beginOffset + 0x74, SEEK_SET);
    uint numFrames = stream.readUint16LE();

    for (uint i = firstFrame; i <= lastFrame; ++i) {
        srcRects.push_back(Common::Rect());
        readRect(stream, srcRects[i]);
    }

    for (uint i = 0; i < numFrames; ++i) {
        bitmaps.push_back(BitmapDesc());
        BitmapDesc &rects = bitmaps[i];
        rects.frameID = stream.readUint16LE();
        readRect(stream, rects.src);
        readRect(stream, rects.dest);
    }

    return 0x76 + numFrames * 0x22 + (lastFrame - firstFrame + 1) * 16;
}

void PlayIntStaticBitmapAnimation::execute(NancyEngine *engine) {
    // TODO handle sound, event flags
    uint32 currentFrameTime = engine->getTotalPlayTime();
    switch (state) {
        case kBegin:
            init();
            registerGraphics();

            if (soundName != "NO SOUND") {
                warning("PlayIntStaticBitmapAnimation has a sound, please implement it!");
            }

            state = kRun;
            // fall through
        case kRun: {
            // Check if we've moved the viewport
            uint16 newFrame = engine->scene->getSceneInfo().frameID;
            if (currentViewportFrame != newFrame) {
                currentViewportFrame = newFrame;
                for (uint i = 0; i < bitmaps.size(); ++i) {
                    if (currentViewportFrame == bitmaps[i].frameID) {
                        nextFrameTime = 0;
                        _screenPosition = bitmaps[i].dest;
                        break;
                    }
                }
            }

            // Check the timer to see if we need to draw the next animation frame
            if (nextFrameTime <= currentFrameTime) {
                nextFrameTime = currentFrameTime + frameTime;
                currentFrame = ++currentFrame > lastFrame ? firstFrame : currentFrame;
                setFrame(currentFrame);
            }
            
            break;
        }
        case kActionTrigger:
            triggerFlags.execute(engine);

            SceneChange::execute(engine);
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
