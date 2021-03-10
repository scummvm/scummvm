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

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/commontypes.h"

#include "common/str.h"
#include "common/array.h"

namespace Nancy {
namespace Action {

// ActionRecord subclass describing a short "flipbook" animation from a single bitmap
// Also supports sound and getting interrupted by an event flag.
// This class covers both the PlayStaticBitmapAnimation and PlayIntStaticBitmapAnimation
// action record types, whose functionality is nearly identical
class PlayStaticBitmapAnimation : public ActionRecord, public RenderObject {
public:
    PlayStaticBitmapAnimation(bool interruptible, RenderObject &redrawFrom) : RenderObject(redrawFrom), isInterruptible(interruptible) {}
    virtual ~PlayStaticBitmapAnimation() { _fullSurface.free(); }

    virtual void init() override;

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    virtual void onPause(bool pause) override;

    Common::String imageName;

    NancyFlag isTransparent; // 0xC
    NancyFlag doNotChangeScene; // 0xE
    NancyFlag isReverse; // 0x10
    NancyFlag isLooping; // 0x12
    uint16 firstFrame; // 0x14
    uint16 loopFirstFrame; // 0x16
    uint16 loopLastFrame; // 0x18
    Time frameTime;
    uint16 zOrder; // 0x1C
    EventFlagDescription interruptCondition; // 0x1E
    SceneChangeDescription sceneChange;
    MultiEventFlagDescription triggerFlags; // 0x2A

    Nancy::SoundDescription sound; // 0x52

    // Describes a single frame in this animation
    Common::Array<Common::Rect> srcRects;
    // Describes how the animation will be displayed on a single
    // frame of the viewport
    Common::Array<BitmapDescription> bitmaps;

    int16 currentFrame = -1;
    int16 currentViewportFrame = -1;
    Time nextFrameTime;
    bool isInterruptible;
    
protected:
    virtual Common::String getRecordTypeName() const override { return isInterruptible ? "PlayIntStaticBitmapAnimation" : "PlayStaticBitmapAnimation"; }

    virtual uint16 getZOrder() const override { return zOrder; }
    virtual BlitType getBlitType() const override { return isTransparent == kTrue ? kTrans : kNoTrans; }
    virtual bool isViewportRelative() const override { return true; }

    void setFrame(uint frame);
    
    Graphics::ManagedSurface _fullSurface;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_STATICBITMAPANIM_H
