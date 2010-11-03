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

#ifndef ASYLUM_WORLDSTATS_H
#define ASYLUM_WORLDSTATS_H

#include "common/rect.h"
#include "common/array.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/barrier.h"
#include "asylum/resources/actionarea.h"
#include "asylum/views/scene.h"
#include "asylum/system/sound.h"

namespace Asylum {

class WorldStats {
public:
	WorldStats(Common::SeekableReadStream *stream, Scene *scene);
	virtual ~WorldStats();

	int32 size;
	int32 numEntries;
	int32 numChapter;
	int32 xLeft; // scene start x position
	int32 yTop;  // scene start y position
	Common::Rect boundingRect;
	int32 backgroundImage;
	int32 curScrollUp;
	int32 curScrollUpLeft;
	int32 curScrollLeft;
	int32 curScrollDownLeft;
	int32 curScrollDown;
	int32 curScrollDownRight;
	int32 curScrollRight;
	int32 curScrollUpRight;
	int32 curHand;
	int32 curMagnifyingGlass;
	int32 curTalkNPC;
	int32 curGrabPointer;
	int32 curTalkNPC2;
	int32 font1;
	int32 font2;
	int32 font3;
	int32 palette;
	int32 cellShadeMask1;
	int32 cellShadeMask2;
	int32 cellShadeMask3;
	int32 unused;
	int32 smallCurUp;
	int32 smallCurDown;
	int32 encounterFrameBg;
	int32 width; // field_80
	int32 height;
	int32 motionStatus;
	int32 field_8C;
	int32 numActions; // field_90
	int32 numBarriers;
	int32 targetX;
	int32 targetY;
	int32 field_A0;
	int32 field_A4;
	int32 field_A8;
	int32 field_AC;
	int32 field_B0;
	int32 numActors;
	int32 stereoReversedFlag;
	Common::Rect sceneRects[6]; // including scene size rect
	uint8 sceneRectIdx;
	uint8 field_11D[3];
	int32 field_120;
	int32 actionListIdx;	 // actionList start index
	int32 grResId[100];
	int32 sceneTitleGrResId;
	int32 sceneTitlePalResId;
	int32 actorType;
	int32 soundResId[50];
	AmbientSoundItem ambientSounds[15];
	int32 numAmbientSound;
	int32 musicStatus;
	int32 musicCurrentResId;
	int32 musicFlag;
	int32 musicResId;
	int32 musicStatusExt;

	// FIXME: Investigate if we need to actually reserve maxsize for this arrays.
	// It always have that size under scene file and they are always save in savegames.
	Common::Array<Barrier>    barriers; // maxsize 400
	Common::Array<Actor>      actors;   // maxsize 50
	// TODO add rest fields
	Common::Array<ActionArea> actions;  // maxsize 400
	// TODO add rest fields
	int32 field_E860C;
	// TODO add rest fields

	int32 getActionAreaIndexById(int32 id);
	ActionArea* getActionAreaById(int32 id);

	int32 getBarrierIndexById(int32 id);
	Barrier* getBarrierById(int32 id);
	Barrier* getBarrierByIndex(int32 idx);

	bool isBarrierOnScreen(int32 idx);
	bool isBarrierVisible(int32 idx);
	// TODO this needs a better name
	bool checkBarrierFlagsCondition(int32 idx);

private:
	Scene *_scene;

	void load(Common::SeekableReadStream *stream);
};

} // end of namespace Asylum

#endif
