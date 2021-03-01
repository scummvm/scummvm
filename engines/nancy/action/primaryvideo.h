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

#ifndef NANCY_ACTION_PRIMARYVIDEO_H
#define NANCY_ACTION_PRIMARYVIDEO_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/video.h"
#include "engines/nancy/commontypes.h"

#include "common/str.h"
#include "common/array.h"

namespace Nancy {

class NancyEngine;

namespace Action {

// ActionRecord subclass that handles all NPC dialog and nancy1's intro video
class PlayPrimaryVideoChan0 : public ActionRecord, public RenderObject {

struct ConditionFlag {
enum ConditionType : byte { kNone = 0, kEventFlags = 1, kInventory = 2 };

    ConditionType type;
    EventFlagDescription flag;
    bool orFlag;

    void read(Common::SeekableReadStream &stream);
    bool isSatisfied(NancyEngine *engine) const;
    void set(NancyEngine *engine) const;
};

struct ConditionFlags {
    Common::Array<ConditionFlag> conditionFlags;

    void read(Common::SeekableReadStream &stream);
    bool isSatisfied(NancyEngine *engine) const;
};

struct ResponseStruct {
    ConditionFlags conditionFlags; // 0x01
    Common::String text; // 0x06
    Common::String soundName; // 0x196
    SceneChangeDescription sceneChange; // 0x1A0
    EventFlagDescription flagDesc; // 0x1A8
};

struct FlagsStruct {
    ConditionFlags conditions;
    ConditionFlag flagToSet;
};

public:
    PlayPrimaryVideoChan0(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~PlayPrimaryVideoChan0();

    virtual void init() override;
    virtual void updateGraphics() override;
    virtual void onPause(bool pause) override;

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;
    
    // Functions for handling the built-in dialogue responses found in the executable
    void addConditionalResponses(NancyEngine *engine);
    void addGoodbye(NancyEngine *engine);

    Common::String videoName; // 0x00
    Common::Rect src; // 0x1D
    // _screenPosition 0x2D
    Common::String text; // 0x3D

    SoundDescription sound; // 0x619
    SoundDescription responseGenericSound; // 0x63B

    byte conditionalResponseCharacterID; // 0x65E
    byte goodbyeResponseCharacterID; // 0x65F
    NancyFlag isDialogueExitScene; // 0x660
    NancyFlag doNotPop; // 0x661
    SceneChangeDescription sceneChange; // 0x662

    Common::Array<ResponseStruct> responses; // 0x69E
    Common::Array<FlagsStruct> flagsStructs; // 0x6AA

    AVFDecoder _decoder;

    bool hasDrawnTextbox = false;
    int16 pickedResponse = -1;

    // Used to avoid clashes between multiple instances in the same scene
	static PlayPrimaryVideoChan0 *activePrimaryVideo;

protected:
    virtual Common::String getRecordTypeName() const override { return "PlayPrimaryVideoChan0"; }
    
    virtual uint16 getZOrder() const override { return 8; }
    virtual BlitType getBlitType() const override { return kNoTrans; }
    virtual bool isViewportRelative() const override { return true; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PRIMARYVIDEO_H
