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

#ifndef ASYLUM_WORLDSTATS_H_
#define ASYLUM_WORLDSTATS_H_

#include "common/rect.h"
#include "common/array.h"

#include "asylum/actor.h"
#include "asylum/barrier.h"
#include "asylum/scene.h"
#include "asylum/sound.h"

namespace Asylum {

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

typedef struct ActionArea {
	char   name[52];
	uint32 id;
	uint32 field01;
	uint32 field02;
	uint32 field_40;
	uint32 field_44;
	uint32 flags;
	uint32 actionListIdx1;
	uint32 actionListIdx2;
	uint32 actionType; // aka flags2: 0-none, 1-findwhat, 2-talk, 3-findwhat??, 4-grab
	uint32 flagNums[10];
	uint32 field_7C;
	uint32 polyIdx;
	uint32 field_84;
	uint32 field_88;
	uint32 soundResId;
	uint32 field_90;
	uint32 paletteValue;
	uint32 array[5];
	uint32 volume;

} ActionArea;

class WorldStats {
public:
	WorldStats(Common::SeekableReadStream *stream, Scene *scene);
	virtual ~WorldStats();

	uint32			 size;
	uint32			 numEntries;
	uint32			 numChapter;
	uint32			 xLeft;			 // scene start x position
	uint32			 yTop;			 // scene start y position
	Common::Rect	 boundingRect;
	CommonResources	 commonRes;		 // field_1C till field_7C
	uint32			 width;			 // field_80
	uint32			 height;
	uint32			 motionStatus;
	uint32			 field_8C;
	uint32			 numActions;	 // field_90
	uint32			 numBarriers;
	uint32			 targetX;
	uint32			 targetY;
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
	Common::Array<Barrier>    barriers; // maxsize 400
	Common::Array<Actor>      actors;   // maxsize 50
	// TODO add rest fields
	Common::Array<ActionArea> actions;  // maxsize 400
	// TODO add rest fields
	uint32 field_E860C;
	// TODO add rest fields

	int         getActionAreaIndexById(uint32 id);
	ActionArea* getActionAreaById(uint32 id);

	int      getBarrierIndexById(uint32 id);
	Barrier* getBarrierById(uint32 id);
	Barrier* getBarrierByIndex(uint32 idx);

	bool isBarrierOnScreen(uint32 idx);
	bool isBarrierVisible(uint32 idx);
	// TODO this needs a better name
	bool checkBarrierFlagsCondition(uint32 idx);
private:
	Scene *_scene;

	void load(Common::SeekableReadStream *stream);

};

} // end of namespace Asylum

#endif
