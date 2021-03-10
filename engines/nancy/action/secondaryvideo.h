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

#ifndef NANCY_ACTION_SECONDARYVIDEO_H
#define NANCY_ACTION_SECONDARYVIDEO_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/video.h"
#include "engines/nancy/commontypes.h"

#include "common/str.h"
#include "common/array.h"

namespace Nancy {
namespace Action {

// ActionRecord that shows NPC animations outside of dialogue. Supports
// different animations depending on whether the NPC is hovered by the mouse
class PlaySecondaryVideo : public ActionRecord, public RenderObject {
public:
    enum HoverState { kNoHover, kHover, kEndHover };

    PlaySecondaryVideo(char chan, RenderObject &redrawFrom) : RenderObject(redrawFrom), channel(chan) {}
    virtual ~PlaySecondaryVideo() { _decoder.close(); }

    virtual void init() override;
    virtual void updateGraphics() override;
    virtual void onPause(bool pause) override;
    virtual void handleInput(NancyInput &input) override;

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    Common::String filename;
    //...
    uint16 loopFirstFrame = 0; // 0x1E
    uint16 loopLastFrame = 0; // 0x20
    uint16 onHoverFirstFrame = 0; // 0x22
    uint16 onHoverLastFrame = 0; // 0x24
    uint16 onHoverEndFirstFrame = 0; // 0x26
    uint16 onHoverEndLastFrame = 0; // 0x28
    SceneChangeDescription sceneChange; // 0x2A
    // unknown byte
    Common::Array<SecondaryVideoDescription> videoDescs; // 0x35

protected:
    virtual Common::String getRecordTypeName() const override { return Common::String("PlaySecondaryVideoChan" + channel); }

    virtual uint16 getZOrder() const override { return 8; }
    virtual BlitType getBlitType() const override { return kTrans; }
    virtual bool isViewportRelative() const override { return true; }

    HoverState hoverState = kNoHover;
    AVFDecoder _decoder;
    int _currentViewportFrame = -1;
    bool _isPlaying = false;
    bool _isHovered = false;

    char channel;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SECONDARYVIDEO_H
