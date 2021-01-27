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

#ifndef NANCY_ACTION_RECORDTYPES_H
#define NANCY_ACTION_RECORDTYPES_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/playstate.h"

#include "common/stream.h"
#include "common/array.h"

namespace Nancy {

class NancyEngine;

// Describes a hotspot
struct HotspotDesc {
    uint16 frameID = 0;
    Common::Rect coords;

    void readData(Common::SeekableReadStream &stream);
};

// Describes a secondary video/movie's source and destination
struct SecondaryVideoDesc {
    int16 frameID;
    Common::Rect srcRect;
    Common::Rect destRect;
    // 2 unknown/empty rects

    void readData(Common::SeekableReadStream &stream);
};

// Describes a single event flag change or comparison
struct FlagDesc {
    int16 label;
    PlayState::Flag flag;
};

// Describes 10 event flag changes to be executed when an action is triggered
struct EventFlagsDesc {
    FlagDesc descs[10];

    void readData(Common::SeekableReadStream &stream);
    void execute(NancyEngine *engine);
};

class SceneChange : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    uint16 sceneID = 0;
    uint16 frameID = 0;
    uint16 verticalOffset = 0;
    bool doNotStartSound = false;
};

class HotMultiframeSceneChange : public SceneChange {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    Common::Array<HotspotDesc> hotspots;
};

// The exact same logic as Hot1FrExitSceneChange
class Hot1FrSceneChange : public SceneChange {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    HotspotDesc hotspotDesc;
};

class HotMultiframeMultisceneChange : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class StartFrameNextScene : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class StartStopPlayerScrolling : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    // TODO add a Start and Stop subclass

    byte type = 0;
};

class PlayPrimaryVideoChan0 : public ActionRecord {

struct ConditionFlags {
    byte unknown[5];
};

struct ResponseStruct {
    Common::Array<ConditionFlags> conditionFlags;
    byte unknown[0x1d8]; // TODO
};

struct FlagsStruct {
    Common::Array<ConditionFlags> conditionFlags;
    uint32 unknown; // TODO
};

public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte videoData[0x69C]; // TODO this is its own class
    Common::Array<ResponseStruct> responses;
    Common::Array<FlagsStruct> flagsStructs;
};

// Base class for PlaySecondaryVideoChan0 and PlaySecondaryVideoChan1
class PlaySecondaryVideo : public SceneChange {
public:

    enum HoverState { kNoHover, kHover, kEndHover, kEndHoverDone };

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    virtual uint channelID() =0;

    Common::String filename;
    //...
    uint16 loopFirstFrame = 0; // 0x1E
    uint16 loopLastFrame = 0; // 0x20
    uint16 onHoverFirstFrame = 0; // 0x22
    uint16 onHoverLastFrame = 0; // 0x24
    uint16 onHoverEndFirstFrame = 0; // 0x26
    uint16 onHoverEndLastFrame = 0; // 0x28
    // SceneChange data is at 0x2A
    // unknown byte
    Common::Array<SecondaryVideoDesc> videoDescs; // 0x35

    // not present in original data
    HoverState hoverState = kNoHover;
};

class PlaySecondaryVideoChan0 : public PlaySecondaryVideo {
    virtual uint channelID() override { return 0; }
};

class PlaySecondaryVideoChan1 : public PlaySecondaryVideo {
    virtual uint channelID() override { return 1; }
};

class PlaySecondaryMovie : public SceneChange {
public:
    struct FlagAtFrame {
        int16 frameID;
        FlagDesc flagDesc;
    };

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    Common::String videoName; // 0x00

    FlagAtFrame frameFlags[15]; // 0x26
    EventFlagsDesc triggerFlags; // 0x80

    Common::String soundName; // 0xA8
    uint16 soundChannel = 0; // 0xB2
    uint16 soundVolume = 0; // 0xC2

    // SceneChange data at 0xCA
    Common::Array<SecondaryVideoDesc> videoDescs; // 0xD4


};

class PlayStaticBitmapAnimation : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte bitmapData[0xA88];
};

class PlayIntStaticBitmapAnimation : public SceneChange {
// TODO this effectively also contains an EventFlags, consider multiple inheritance
// or maybe splitting EventFlags into a separate struct
public:
    struct SrcDestDesc {
        uint16 frameId = 0;
        Common::Rect src;
        Common::Rect dest;
    };

    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    Common::String imageName;
    uint16 firstFrame;
    uint16 lastFrame;
    FlagDesc soundFlagDesc;
    EventFlagsDesc triggerFlags;
    Time frameTime;

    // Todo
    Common::String soundName;
    uint16 channelID;

    // Describes a single frame in this animation
    Common::Array<Common::Rect> frameRects;
    // Describes how the animation will be displayed on a single
    // frame of the viewport
    Common::Array<SrcDestDesc> srcDestRects;

    uint16 currentFrame = 0;
    uint16 lastViewFrame = 0;
    int16 currentViewFrameID = -1;
    Time nextFrameTime;
};

class MapCall : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte mapData = 0;
};

class MapCallHot1Fr : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class MapCallHotMultiframe : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class MapLocationAccess : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class MapSound : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class MapAviOverride : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class MapAviOverrideOff : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte overrideOffData = 0;
};

class TextBoxWrite : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class TextBoxClear : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte clearData = 0;
};

class BumpPlayerClock : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class SaveContinueGame : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte saveContinueData = 0;
};

class TurnOffMainRendering : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte turnOffData = 0;
};

class TurnOnMainRendering : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte turnOnData = 0;
};

class ResetAndStartTimer : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;
};

class StopTimer : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;
};

class EventFlags : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    EventFlagsDesc flags;
};

class EventFlagsMultiHS : public EventFlags {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    Common::Array<HotspotDesc> hotspots;
};

class OrderingPuzzle : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class LoseGame : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte loseData = 0;
};

class PushScene : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte pushData = 0;
};

class PopScene : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte popData = 0;
};

class WinGame : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte winData = 0;
};

class LeverPuzzle : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class Telephone : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class SliderPuzzle : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class PasswordPuzzle : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class AddInventoryNoHS : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class RemoveInventoryNoHS : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class DifficultyLevel : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;

    uint16 difficulty = 0;
    int16 flagLabel = 0;
    uint16 flagCondition = 0;
};

class RotatingLockPuzzle : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class ShowInventoryItem : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class PlayDigiSoundAndDie : public SceneChange {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(NancyEngine *engine) override;
    // TODO subclass into Play and Stop (?)

    Common::String filename;
    int16 id = -1; // 0xA
    uint16 numLoops = 0; // 0x10
    uint16 volume = 0; // 0x16, maximum is 65?
    // ...
    // SceneChange elements at 0x1E
};

class PlaySoundPanFrameAnchorAndDie : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class PlaySoundMultiHS : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

class HintSystem : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
};

} // End of namespace Nancy

#endif // NANCY_ACTION_RECORDTYPES_H