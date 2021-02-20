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

#include "engines/nancy/action/recordtypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/video.h"

#include "common/str.h"
#include "common/array.h"

namespace Nancy {

class NancyEngine;

namespace Action {

// ActionRecord subclass that handles all NPC dialog and nancy1's intro video
class PlayPrimaryVideoChan0 : public SceneChange, public RenderObject {

struct ConditionFlags {
    byte unknown[5];
};

struct ResponseStruct {
    Common::Array<ConditionFlags> conditionFlags; // 0x02
    Common::String text; // 0x06
    Common::String soundName; // 0x196
    SceneChangeDesc sceneChange; // 0x1A0
    FlagDesc flagDesc; // 0x1A8
};

struct FlagsStruct {
    enum ConditionType : byte { kNone = 0, kEventFlags = 1, kInventory = 2 };
    Common::Array<ConditionFlags> conditionFlags;

    ConditionType type;
    int16 label;
    NancyFlag flag;
};

public:
    PlayPrimaryVideoChan0(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~PlayPrimaryVideoChan0();

    virtual void init() override;
    virtual void updateGraphics() override;

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;
    
    // Functions for handling the built-in dialogue responses found in the executable
    void addConditionalResponses(NancyEngine *engine);
    void addGoodbye(NancyEngine *engine);

    Common::String videoName; // 0x00
    Common::Rect src; // 0x1D
    // _screenPosition 0x2D
    Common::String text; // 0x3D

    SoundManager::SoundDescription sound; // 0x619

    byte conditionalResponseCharacterID; // 0x65E
    byte goodbyeResponseCharacterID; // 0x65F
    byte numSceneChanges; // 0x660, not sure
    bool shouldPopScene; // 0x661
    // SceneChange data is at 0x662

    Common::Array<ResponseStruct> responses;
    Common::Array<FlagsStruct> flagsStructs;

    AVFDecoder _decoder;

    bool hasDrawnTextbox = false;
    int16 pickedResponse = -1;

    // Used to avoid showing first frame of unrelated primary video between scenes
    static bool isExitingScene;

protected:
    virtual uint16 getZOrder() const override { return 8; }
    virtual BlitType getBlitType() const override { return kNoTrans; }
    virtual bool isViewportRelative() const override { return true; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PRIMARYVIDEO_H
