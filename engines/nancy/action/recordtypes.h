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
#include "engines/nancy/commontypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/sound.h"

#include "common/stream.h"
#include "common/array.h"
#include "common/str.h"

namespace Nancy {

class NancyEngine;

namespace Action {

// Describes a hotspot
struct HotspotDesc {
    uint16 frameID = 0;
    Common::Rect coords;

    void readData(Common::SeekableReadStream &stream);
};

// Describes a single bitmap draw
struct BitmapDesc {
    uint16 frameID = 0;
    Common::Rect src;
    Common::Rect dest;

    void readData(Common::SeekableReadStream &stream);
};

// Describes a single event flag change or comparison
struct FlagDesc {
    int16 label;
    NancyFlag flag;
};

// Describes 10 event flag changes to be executed when an action is triggered
struct EventFlagsDesc {
    FlagDesc descs[10];

    void readData(Common::SeekableReadStream &stream);
    void execute(Nancy::NancyEngine *engine);
};

// Describes a scene transition
struct SceneChangeDesc {
    uint16 sceneID = 0;
    uint16 frameID = 0;
    uint16 verticalOffset = 0;
    bool doNotStartSound = false;

    void readData(Common::SeekableReadStream &stream);
};

class SceneChange : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    SceneChangeDesc sceneChange;
};

class HotMultiframeSceneChange : public SceneChange {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    Common::Array<HotspotDesc> hotspots;
};

class Hot1FrSceneChange : public SceneChange {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    HotspotDesc hotspotDesc;
};

class Hot1FrExitSceneChange : public Hot1FrSceneChange {
    virtual CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExitArrow; }
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

class PlayStaticBitmapAnimation : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;

    byte bitmapData[0xA88];
};

class MapCall : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    virtual CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExitArrow; }
};

class MapCallHot1Fr : public MapCall {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    HotspotDesc hotspotDesc;
};

class MapCallHotMultiframe : public MapCall {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    Common::Array<HotspotDesc> hotspots;
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
    virtual void execute(Nancy::NancyEngine *engine) override;
};

class StopTimer : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;
};

class EventFlags : public ActionRecord {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    EventFlagsDesc flags;
};

class EventFlagsMultiHS : public EventFlags {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    Common::Array<HotspotDesc> hotspots;
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
    virtual void execute(Nancy::NancyEngine *engine) override;

    uint16 difficulty = 0;
    FlagDesc flag;
};

class ShowInventoryItem : public ActionRecord, public RenderObject {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;

    ShowInventoryItem(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~ShowInventoryItem() { _fullSurface.free(); }

    virtual void init()override;
 
    uint16 objectID = 0;
    Common::String imageName;
    Common::Array<BitmapDesc> bitmaps;

    int16 drawnFrameID = -1;
    Graphics::ManagedSurface _fullSurface;
    
protected:
    virtual uint16 getZOrder() const override { return 9; }
    virtual BlitType getBlitType() const override { return kNoTrans; }
    virtual bool isViewportRelative() const override { return true; }
};

class PlayDigiSoundAndDie : public SceneChange {
public:
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;
    // TODO subclass into Play and Stop (?)

    SoundManager::SoundDescription sound;
    // SceneChange 0x1E
    FlagDesc flagOnTrigger;
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
    virtual void execute(Nancy::NancyEngine *engine) override;

    byte characterID; // 0x00
    SoundManager::SoundDescription genericSound; // 0x01

    Common::String text;
    SceneChangeDesc sceneChange;
    uint16 hintID;
    int16 hintWeight;

    void selectHint(Nancy::NancyEngine *engine);
    void getHint(uint hint, uint difficulty);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RECORDTYPES_H
