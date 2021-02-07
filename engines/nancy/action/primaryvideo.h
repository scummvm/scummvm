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

namespace Nancy {

class NancyEngine;

class PlayPrimaryVideoChan0 : public SceneChange {

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
    PlayState::Flag flag;
};

public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;
    
    // Functions for handling the built-in dialogue responses found in the executable
    void addConditionalResponses(NancyEngine *engine);
    void addGoodbye(NancyEngine *engine);

    Common::String videoName; // 0x00
    Common::Rect src; // 0x1D
    Common::Rect dest; // 0x2D
    Common::String text; // 0x3D

    Common::String soundName; // 0x619, TODO make a proper soundDesc struct
    uint16 soundChannelID; // 0x623
    uint16 numRepeats; // 0x62D
    uint16 volume; // 0x633

    byte conditionalResponseCharacterID; // 0x65E
    byte goodbyeResponseCharacterID; // 0x65F
    byte numSceneChanges; // 0x660, not sure
    bool shouldPopScene; // 0x661
    // SceneChange data is at 0x662

    Common::Array<ResponseStruct> responses;
    Common::Array<FlagsStruct> flagsStructs;

    bool hasDrawnTextbox = false;
    int16 pickedResponse = -1;

private:
    void assembleText(char *rawCaption, Common::String &output, uint size);

};

}

#endif // NANCY_ACTION_PRIMARYVIDEO_H