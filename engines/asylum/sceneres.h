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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SCENERESOURCE_H_
#define ASYLUM_SCENERESOURCE_H_

#include "common/file.h"
#include "common/array.h"
#include "common/rect.h"

#include "asylum/actor.h"
#include "asylum/screen.h"
#include "asylum/respack.h"

#define SCENEMASK   "scn.%03d"
#define Polygons_MAXSIZE  100

namespace Asylum {

class AsylumEngine;
class WorldStats;
class GamePolygons;
class ActionList;

// ScummVM common Point uses int16 and we need int32
typedef struct Point { 
    int32 x; 
    int32 y; 
} Point;

// ScummVM common Rect uses int16 and we need int32
typedef struct Rect { 
    int32 top, left;
    int32 bottom, right; 
} Rect;

class SceneResource {
public:
	SceneResource();
	virtual ~SceneResource();

    bool load(uint8 sceneIdx);

    WorldStats* getWorldStats() { return _worldStats; }
    GamePolygons* getGamePolygons() { return _gamePolygons; }
    ActionList* getActionList() { return _actionList; }
    MainActor* getMainActor() { return _mainActor; }
private:
    WorldStats   *_worldStats;
    GamePolygons *_gamePolygons;
    ActionList   *_actionList;
    MainActor    *_mainActor;

    void loadWorldStats(Common::SeekableReadStream *stream);
    void loadGamePolygons(Common::SeekableReadStream *stream);
    void loadActionList(Common::SeekableReadStream *stream);
    Common::String parseFilename(uint8 sceneIdx);

}; // end of class Scene


typedef struct SoundItem {
    uint32 resId;
    uint32 field_4;
    uint32 field_8;
    uint32 field_C;
} SoundItem;

typedef struct FrameSoundItem {
    uint32 resId;
    uint32 frameIdx;
    uint32 index;
    uint32 field_C;
    uint32 field_10;
    uint32 field_14;
} FrameSoundItem;

// FIXME figure out unknown fields
typedef struct ActorDefinitions {
    uint32 id;
    uint32 resId;
    uint32 x;
    uint32 y;
    Rect boundingBox;
    uint32 field_20;
    uint32 frameIdx;
    uint32 frameCount;   
    uint32 field_2C;
    uint32 field_30;
    uint32 field_34;
    uint32 flags;
    uint32 field_3C;
    uint8  name[52]; // field_40 till field_70;
    uint32 field_74;
    uint32 field_78;
    uint32 field_7C;
    uint32 field_80;
    uint32 polyIdx;
    uint32 flags2;
    uint32 gameFlags[10];
    uint32 field_B4;
    uint32 tickCount;
    uint32 tickCount2;
    uint32 field_C0;
    uint32 field_C4;
    uint32 actionListIdx;
    SoundItem soundItems[16];
    FrameSoundItem frameSoundItems[50];
    uint32 field_67C;
    uint32 soundX;
    uint32 soundY;
    uint32 field_688;
    uint32 field_68C[5];
    uint32 soundResId;
    uint32 field_6A4;
} ActorDefinitions;


// FIXME add unknown fields
typedef struct ActorActionDefinitions {
    char   name[52];
    uint32 id;
    int32  actionListIdx1;
    int32  actionListIdx2;
    int32  actionType; // 0-none, 1-findwhat, 2-talk, 3-findwhat??, 4-grab
    int32  polyIdx;
    uint32 soundResId;
    uint32 palCorrection;
    int32  soundVolume;
} ActorActionDefinitions;

typedef struct CommonResources {
    uint32 backgroundImage;
    uint32 curScrollUp;
    uint32 curScrollUpLeft;
    uint32 curScrollLeft;
    uint32 curScrollDownLeft;
    uint32 curScrollDown;
    uint32 curScrollDownRight;
    uint32 curScrollRight;
    uint32 curScrollUpRight;
    uint32 curHand;
    uint32 curMagnifyingGlass;
    uint32 curTalkNCP;
    uint32 curGrabPointer;
    uint32 curTalkNCP2;
    uint32 unknown1;
    uint32 unknown2;
    uint32 unknown3;
    uint32 palette;
    uint32 cellShadeMask1;
    uint32 cellShadeMask2;
    uint32 cellShadeMask3;
    uint32 unused;
    uint32 smallCurUp;
    uint32 smallCurDown;
    uint32 unknown4;
} CommonResources;


class WorldStats {
public:
    WorldStats() {};
    virtual ~WorldStats() {};

    uint32 _size;
    uint32 _numEntries;
    uint32 _numChapter;
    uint32 _width;
    uint32 _height;
    uint32 _numActions;
    uint32 _numActors;
    uint32 _loadingScreenGrResId;
    uint32 _loadingScreenPalResId;

    CommonResources _commonRes;

    Common::Array<ActorDefinitions> _actorsDef;
    Common::Array<ActorActionDefinitions> _actorsActionDef;
}; // end of class WorldStats


typedef struct PolyDefinitions{
    uint32  numPoints;
    Point   points[Polygons_MAXSIZE];
    Rect    boundingRect;
} PolyDefinitions;

class GamePolygons {
public:
    GamePolygons() {};
    virtual ~GamePolygons() {};

    uint32 _size;
    uint32 _numEntries;

    Common::Array<PolyDefinitions> _Polygons;
}; // end of class GamePolygons


class ActionList {
public:
    ActionList() {};
    virtual ~ActionList() {};
}; // end of class ActionList

} // end of namespace Asylum

#endif
