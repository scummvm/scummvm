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

#ifndef NANCY_ACTION_SECONDARYMOVIE_H
#define NANCY_ACTION_SECONDARYMOVIE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/video.h"
#include "engines/nancy/commontypes.h"

#include "common/str.h"
#include "common/array.h"

namespace Nancy {
namespace Action {

class PlaySecondaryMovie : public ActionRecord, public RenderObject {
public:
    struct FlagAtFrame {
        int16 frameID;
        EventFlagDescription flagDesc;
    };

    PlaySecondaryMovie(RenderObject &redrawFrom) :
        RenderObject(redrawFrom),
        _curViewportFrame(-1),
        isFinished(false) {}
    virtual ~PlaySecondaryMovie();

    virtual void init() override;
    virtual void updateGraphics() override;
    virtual void onPause(bool pause) override;

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    Common::String videoName; // 0x00

    uint16 unknown; // 0x1C
    NancyFlag hideMouse; // 0x1E
    NancyFlag isReverse; // 0x20
    uint16 firstFrame; // 0x22
    uint16 lastFrame; // 0x24
    FlagAtFrame frameFlags[15]; // 0x26
    MultiEventFlagDescription triggerFlags; // 0x80

    SoundDescription sound; // 0xA8

    SceneChangeDescription sceneChange; // 0xCA
    Common::Array<SecondaryVideoDescription> videoDescs; // 0xD4

protected:
    virtual Common::String getRecordTypeName() const override { return "PlaySecondaryMovie"; }

    virtual uint16 getZOrder() const override { return 8; }
    virtual BlitType getBlitType() const override { return kNoTrans; }
    virtual bool isViewportRelative() const override { return true; }

    AVFDecoder _decoder;
    int _curViewportFrame;
    bool isFinished;
};
    
} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SECONDARYMOVIE_H
