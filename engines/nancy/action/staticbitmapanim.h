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

#ifndef NANCY_ACTION_STATICBITMAPANIM_H
#define NANCY_ACTION_STATICBITMAPANIM_H

#include "engines/nancy/action/recordtypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/sound.h"

#include "common/str.h"
#include "common/array.h"

namespace Nancy {
namespace Action {

// ActionRecord subclass describing a short "flipbook" animation from a single bitmap
// Can also play sound, but this has not yet been implemented
class PlayIntStaticBitmapAnimation : public SceneChange, public RenderObject {
public:
    PlayIntStaticBitmapAnimation(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~PlayIntStaticBitmapAnimation() { _fullSurface.free(); }

    virtual void init()override;

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    Common::String imageName;
    uint16 firstFrame;
    uint16 lastFrame;
    FlagDesc soundFlagDesc;
    EventFlagsDesc triggerFlags;
    Time frameTime;

    Nancy::SoundManager::SoundDescription sound;

    // Describes a single frame in this animation
    Common::Array<Common::Rect> srcRects;
    // Describes how the animation will be displayed on a single
    // frame of the viewport
    Common::Array<BitmapDesc> bitmaps;

    int16 currentFrame = -1;
    int16 currentViewportFrame = -1;
    Time nextFrameTime;
    
protected:
    virtual uint16 getZOrder() const override { return 7; }
    virtual BlitType getBlitType() const override { return kNoTrans; }
    virtual bool isViewportRelative() const override { return true; }

    void setFrame(uint frame);
    
    Graphics::ManagedSurface _fullSurface;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_STATICBITMAPANIM_H
