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

#include "engines/nancy/commontypes.h"

#include "common/stream.h"
#include "common/array.h"
#include "common/str.h"

namespace Nancy {

class NancyEngine;

namespace Action {

class SceneChange : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    SceneChangeDescription sceneChange;

protected:
    virtual Common::String getRecordTypeName() const override { return "SceneChange"; }
};

class HotMultiframeSceneChange : public SceneChange {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    Common::Array<HotspotDescription> hotspots;

protected:
    virtual Common::String getRecordTypeName() const override { return "HotMultiframeSceneChange"; }
};

class Hot1FrSceneChange : public SceneChange {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    HotspotDescription hotspotDesc;

protected:
    virtual Common::String getRecordTypeName() const override { return "Hot1FrSceneChange"; }
};

class Hot1FrExitSceneChange : public Hot1FrSceneChange {
    virtual CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExitArrow; }

protected:
    virtual Common::String getRecordTypeName() const override { return "Hot1FrExitSceneChange"; }
};

class HotMultiframeMultisceneChange : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;

protected:
    virtual Common::String getRecordTypeName() const override { return "HotMultiframeMultisceneChange"; }
};

class StartFrameNextScene : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;

protected:
    virtual Common::String getRecordTypeName() const override { return "StartFrameNextScene"; }
};

class StartStopPlayerScrolling : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    // TODO add a Start and Stop subclass

    byte type = 0;

protected:
    virtual Common::String getRecordTypeName() const override { return "StartStopPlayerScrolling"; }
};

class MapCall : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    virtual CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExitArrow; }

protected:
    virtual Common::String getRecordTypeName() const override { return "MapCall"; }
};

class MapCallHot1Fr : public MapCall {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    HotspotDescription hotspotDesc;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "MapCallHot1Fr"; }
};

class MapCallHotMultiframe : public MapCall {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    Common::Array<HotspotDescription> hotspots;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "MapCallHotMultiframe"; }
};

class MapLocationAccess : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "MapLocationAccess"; }
};

class MapSound : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "MapSound"; }
};

class MapAviOverride : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "MapAviOverride"; }
};

class MapAviOverrideOff : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "MapAviOverrideOff"; }
};

class TextBoxWrite : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "TextBoxWrite"; }
};

class TextBoxClear : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "TextBoxClear"; }
};

class BumpPlayerClock : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "BumpPlayerClock"; }
};

class SaveContinueGame : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "SaveContinueGame"; }
};

class TurnOffMainRendering : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "TurnOffMainRendering"; }
};

class TurnOnMainRendering : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "TurnOnMainRendering"; }
};

class ResetAndStartTimer : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "ResetAndStartTimer"; }
};

class StopTimer : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "StopTimer"; }
};

class EventFlags : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    MultiEventFlagDescription flags;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "EventFlags"; }
};

class EventFlagsMultiHS : public EventFlags {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    Common::Array<HotspotDescription> hotspots;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "EventFlagsMultiHS"; }
};

class LoseGame : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "LoseGame"; }
};

class PushScene : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "PushScene"; }
};

class PopScene : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "PopScene"; }
};

class WinGame : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "WinGame"; }
};

class AddInventoryNoHS : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    
    uint itemID;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "AddInventoryNoHS"; }
};

class RemoveInventoryNoHS : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "RemoveInventoryNoHS"; }
};

class DifficultyLevel : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    uint16 difficulty = 0;
    EventFlagDescription flag;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "DifficultyLevel"; }
};

class ShowInventoryItem : public ActionRecord, public RenderObject {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    ShowInventoryItem(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~ShowInventoryItem() { _fullSurface.free(); }

    virtual void init() override;
    virtual void onPause(bool pause) override;
 
    uint16 objectID = 0;
    Common::String imageName;
    Common::Array<BitmapDescription> bitmaps;

    int16 drawnFrameID = -1;
    Graphics::ManagedSurface _fullSurface;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "ShowInventoryItem"; }

    virtual uint16 getZOrder() const override { return 9; }
    virtual bool isViewportRelative() const override { return true; }
};

class PlayDigiSoundAndDie : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    // TODO subclass into Play and Stop (?)

    SoundDescription sound;
    SceneChangeDescription sceneChange;
    EventFlagDescription flagOnTrigger;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "PlayDigiSoundAndDie"; }
};

class PlaySoundPanFrameAnchorAndDie : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    
protected:
    virtual Common::String getRecordTypeName() const override { return "PlaySoundPanFrameAnchorAndDie"; }
};

class PlaySoundMultiHS : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    SoundDescription sound; // 0x0
    SceneChangeDescription sceneChange; // 0x22
    EventFlagDescription flag; // 0x2A
    Common::Array<HotspotDescription> hotspots; // 0x31
    
protected:
    virtual Common::String getRecordTypeName() const override { return "PlaySoundMultiHS"; }
};

class HintSystem : public ActionRecord {
public:
    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;

    byte characterID; // 0x00
    SoundDescription genericSound; // 0x01

    Common::String text;
    SceneChangeDescription sceneChange;
    uint16 hintID;
    int16 hintWeight;

    void selectHint();
    void getHint(uint hint, uint difficulty);
    
protected:
    virtual Common::String getRecordTypeName() const override { return "HintSystem"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RECORDTYPES_H
