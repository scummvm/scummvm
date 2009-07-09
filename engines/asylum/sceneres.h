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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#define SCENEMASK		  "scn.%03d"
#define Polygons_MAXSIZE  200
#define Commands_MAXSIZE  161

namespace Asylum {

class AsylumEngine;
class WorldStats;
class GamePolygons;
class ActionList;

#if 0
// TODO/FIXME: Are there REALLY any points which go beyond 32768???
// ScummVM common Point uses int16 and we need int32
typedef struct Point { 
	int32 x, y;
} Point;

// ScummVM common Rect uses int16 and we need int32
typedef struct Rect { 
	int32 top, left, bottom, right;
} Rect;
#endif

class SceneResource {
public:
	SceneResource();
	virtual ~SceneResource();

	bool load(uint8 sceneIdx);

	WorldStats*	  getWorldStats()	{ return _worldStats; }
	GamePolygons* getGamePolygons() { return _gamePolygons; }
	ActionList*	  getActionList()	{ return _actionList; }
	MainActor*	  getMainActor()	{ return _mainActor; }
	
	int getBarrierIndexById(uint32 id);

private:
	WorldStats	 *_worldStats;
	GamePolygons *_gamePolygons;
	ActionList	 *_actionList;
	MainActor	 *_mainActor;

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

typedef struct AmbientSoundItem {
	uint32 field_0;
	uint32 flags;
	uint32 resId;
	uint32 field_C;
	uint32 field_10;
	uint32 field_14;
	uint32 flagNum[6];
	uint32 x;
	uint32 y;

} AmbientSoundItem;

// FIXME figure out unknown fields
typedef struct BarrierItem {
	uint32		   id;
	uint32		   resId;
	uint32		   x;
	uint32		   y;
	Common::Rect   boundingRect;
	uint32		   field_20;
	uint32		   frameIdx;
	uint32		   frameCount;
	uint32		   field_2C;
	uint32		   field_30;
	uint32		   field_34;
	uint32		   flags;
	uint32		   field_3C;
	uint8		   name[52]; // field_40 till field_70;
	uint32		   field_74;
	uint32		   field_78;
	uint32		   field_7C;
	uint32		   field_80;
	uint32		   polyIdx;
	uint32		   flags2;
	uint32		   gameFlags[10];
	uint32		   field_B4;
	uint32		   tickCount;
	uint32		   tickCount2;
	uint32		   field_C0;
	uint32		   field_C4;
	uint32		   actionListIdx;
	SoundItem	   soundItems[16];
	FrameSoundItem frameSoundItems[50];
	uint32		   field_67C;
	uint32		   soundX;
	uint32		   soundY;
	uint32		   field_688;
	uint32		   field_68C[5];
	uint32		   soundResId;
	uint32		   field_6A4;

} BarrierItem;

// FIXME add unknown fields
typedef struct ActionItem {
	char   name[52];
	uint32 id;
	int32  actionListIdx1;
	int32  actionListIdx2;
	int32  actionType; // 0-none, 1-findwhat, 2-talk, 3-findwhat??, 4-grab
	int32  polyIdx;
	uint32 soundResId;
	uint32 palCorrection;
	int32  soundVolume;

} ActionItem;

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
	uint32 font1;
	uint32 font2;
	uint32 font3;
	uint32 palette;
	uint32 cellShadeMask1;
	uint32 cellShadeMask2;
	uint32 cellShadeMask3;
	uint32 unused;
	uint32 smallCurUp;
	uint32 smallCurDown;
	uint32 encounterFrameBg;

} CommonResources;


typedef struct ActorItem {
	uint32		 x0;
	uint32		 y0;
	uint32		 grResId;
	uint32		 field_C;
	uint32		 frameNum;
	uint32		 frameCount;
	uint32		 x1;
	uint32		 y1;
	uint32		 x2;
	uint32		 y2;
	Common::Rect boundingRect;
	uint32		 direction;
	uint32		 field_3C;
	uint32		 field_40;
	uint32		 field_44;
	uint32		 field_48;
	uint32		 flags;
	uint32		 field_50;
	uint32		 field_54;
	uint32		 field_58;
	uint32		 field_5C;
	uint32		 field_60;
	uint32		 actionIdx3;
	// TODO field_68 till field_617
	uint32		 reaction[8];
	uint32		 field_638;
	uint32		 walkingSound1;
	uint32		 walkingSound2;
	uint32		 walkingSound3;
	uint32		 walkingSound4;
	uint32		 field_64C;
	uint32		 field_650;
	uint32		 grResTable[55];
	char		 name[256];
	uint32		 field_830[20];
	uint32		 field_880[20];
	uint32		 field_8D0[20];
	uint32		 actionIdx2;
	uint32		 field_924;
	uint32		 tickValue1;
	uint32		 field_92C;
	uint32		 flags2;
	uint32		 field_934;
	uint32		 field_938;
	uint32		 soundResId;
	// TODO field_940 till field_978
	uint32		 actionIdx1;
	// TODO field_980 till field_9A0

} ActorItem;

class WorldStats {
public:
	WorldStats() {};
	virtual ~WorldStats() {
		barriers.clear();
		actors.clear();
		actions.clear();
	};

	uint32			 size;
	uint32			 numEntries;
	uint32			 numChapter;
	uint32			 xLeft;			 // scene start x position
	uint32			 yTop;			 // scene start y position
	Common::Rect	 boundingRect;
	CommonResources	 commonRes;		 // field_1C till field_7C
	uint32			 width;			 // field_80
	uint32			 height;
	uint32			 field_88;
	uint32			 field_8C;
	uint32			 numActions;	 // field_90
	uint32			 numBarriers;
	uint32			 field_98;
	uint32			 field_9C;
	uint32			 field_A0;
	uint32			 field_A4;
	uint32			 field_A8;
	uint32			 field_AC;
	uint32			 field_B0;
	uint32			 numActors;		 // Max and all other characters that have
									 // own interactions
	uint32			 stereoReversedFlag;
	Common::Rect	 sceneRects[6];	 // including scene size rect
	uint8			 sceneRectIdx;
	uint8			 field_11D[3];
	uint32			 field_120;
	uint32			 actionListIdx;	 // actionList start index
	uint32			 grResId[100];
	uint32			 sceneTitleGrResId;
	uint32			 sceneTitlePalResId;
	uint32			 actorType;
	uint32			 soundResId[50];
	AmbientSoundItem ambientSounds[15];
	uint32			 numAmbientSound;
	uint32			 musicStatus;
	uint32			 musicCurrentResId;
	uint32			 musicFlag;
	uint32			 musicResId;
	uint32			 musicStatusExt;
	
	// FIXME: Investigate if we need to actually reserve maxsize for this arrays. 
	// It always have that size under scene file and they are always save in savegames.
	Common::Array<BarrierItem> barriers; // maxsize 400
	Common::Array<ActorItem>   actors;	 // maxsize 50
	// TODO add rest fields
	Common::Array<ActionItem>  actions;	 // maxsize 400

}; // end of class WorldStats

typedef struct PolyDefinitions{
	uint32		  numPoints;
	Common::Point *points; //[Polygons_MAXSIZE];
	Common::Rect  boundingRect;

} PolyDefinitions;

class GamePolygons {
public:
	GamePolygons() {};
	virtual ~GamePolygons() {
		polygons.clear();
	};

	uint32 size;
	uint32 numEntries;

	Common::Array<PolyDefinitions> polygons;

}; // end of class GamePolygons

typedef struct ActionCommand {
	uint32 numLines;	//	Only set on the first line of each script
	uint32 opcode;
	uint32 param1;
	uint32 param2;
	uint32 param3;
	uint32 param4;
	uint32 param5;
	uint32 param6;
	uint32 param7;
	uint32 param8;
	uint32 param9;

} ActionCommand;

typedef struct ActionDefinitions {
	ActionCommand commands[161];
	uint32 field_1BAC;
	uint32 field_1BB0;
	uint32 counter;

} ActionDefinitions;

class ActionList {
public:
	ActionList() {};
	virtual ~ActionList() {
		actions.clear();
	};

	uint32 size;
	uint32 numEntries;

	Common::Array<ActionDefinitions> actions;

}; // end of class ActionList

} // end of namespace Asylum

#endif
