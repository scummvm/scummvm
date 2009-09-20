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

#include "worldstats.h"

namespace Asylum {

WorldStats::WorldStats(Common::SeekableReadStream *stream, Scene *scene)
	: _scene(scene) {
	load(stream);
}

WorldStats::~WorldStats() {
	barriers.clear();
	actors.clear();
	actions.clear();
}

int WorldStats::getActionAreaIndexById(uint32 id) {
	for (uint32 i = 0; i < numActions; i++) {
		if (actions[i].id == id)
			return i;
	}

	return -1;
}

ActionArea* WorldStats::getActionAreaById(uint32 id) {
	return &actions[getActionAreaIndexById(id)];
}

int WorldStats::getBarrierIndexById(uint32 id) {
	for (uint32 i = 0; i < numBarriers; i++) {
		if (barriers[i].id == id)
			return i;
	}

	return -1;
}

Barrier* WorldStats::getBarrierById(uint32 id) {
	return &barriers[getBarrierIndexById(id)];
}

Barrier* WorldStats::getBarrierByIndex(uint32 idx) {
	return &barriers[idx];
}

bool WorldStats::isBarrierOnScreen(uint32 idx) {
	Barrier *b = getBarrierByIndex(idx);

	Common::Rect screenRect  = Common::Rect(xLeft, yTop, xLeft + 640, yTop + 480);
	Common::Rect barrierRect = b->boundingRect;
	barrierRect.translate(b->x, b->y);
	return isBarrierVisible(idx) && (b->flags & 1) && screenRect.intersects(barrierRect);
}

bool WorldStats::isBarrierVisible(uint32 idx) {
	Barrier *b = getBarrierByIndex(idx);

	if ((b->flags & 0xFF) & 1) {
		for (int f = 0; f < 10; f++) {
			bool   isSet = false;
			uint32 flag  = b->gameFlags[f];

			if (flag <= 0)
				isSet = _scene->vm()->isGameFlagNotSet(flag); // -flag
			else
				isSet = _scene->vm()->isGameFlagSet(flag);

			if(!isSet)
				return false;
		}
		return true;
	}
	return false;
}

// FIXME: load necessary World Stats content
void WorldStats::load(Common::SeekableReadStream *stream) {
	size       = stream->readUint32LE();
	numEntries = stream->readUint32LE();

	numChapter = stream->readUint32LE();
	xLeft      = stream->readUint32LE();
	yTop       = stream->readUint32LE();

	boundingRect.left   = stream->readUint32LE() & 0xFFFF;
	boundingRect.top    = stream->readUint32LE() & 0xFFFF;
	boundingRect.right  = stream->readUint32LE() & 0xFFFF;
	boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

	// read common graphic resources
	commonRes.backgroundImage    = stream->readUint32LE();
	commonRes.curScrollUp        = stream->readUint32LE();
	commonRes.curScrollUpLeft    = stream->readUint32LE();
	commonRes.curScrollLeft      = stream->readUint32LE();
	commonRes.curScrollDownLeft  = stream->readUint32LE();
	commonRes.curScrollDown      = stream->readUint32LE();
	commonRes.curScrollDownRight = stream->readUint32LE();
	commonRes.curScrollRight     = stream->readUint32LE();
	commonRes.curScrollUpRight   = stream->readUint32LE();
	commonRes.curHand            = stream->readUint32LE();
	commonRes.curMagnifyingGlass = stream->readUint32LE();
	commonRes.curTalkNCP         = stream->readUint32LE();
	commonRes.curGrabPointer     = stream->readUint32LE();
	commonRes.curTalkNCP2        = stream->readUint32LE();
	commonRes.font1              = stream->readUint32LE();
	commonRes.font2              = stream->readUint32LE();
	commonRes.font3	             = stream->readUint32LE();
	commonRes.palette            = stream->readUint32LE();
	commonRes.cellShadeMask1     = stream->readUint32LE();
	commonRes.cellShadeMask2     = stream->readUint32LE();
	commonRes.cellShadeMask3     = stream->readUint32LE();
	commonRes.unused             = stream->readUint32LE();
	commonRes.smallCurUp         = stream->readUint32LE();
	commonRes.smallCurDown       = stream->readUint32LE();
	commonRes.encounterFrameBg   = stream->readUint32LE();

	width        = stream->readUint32LE();
	height       = stream->readUint32LE();
	motionStatus = stream->readUint32LE();
	field_8C     = stream->readUint32LE();
	numActions   = stream->readUint32LE();
	numBarriers  = stream->readUint32LE();
	targetX      = stream->readUint32LE();
	targetY      = stream->readUint32LE();
	field_A0     = stream->readUint32LE();
	field_A4     = stream->readUint32LE();
	field_A8     = stream->readUint32LE();
	field_AC     = stream->readUint32LE();
	field_B0     = stream->readUint32LE();
	numActors    = stream->readUint32LE();

	stereoReversedFlag = stream->readUint32LE();

	for (int r=0; r < 6; r++) {
		sceneRects[r].left   = stream->readUint32LE() & 0xFFFF;
		sceneRects[r].top    = stream->readUint32LE() & 0xFFFF;
		sceneRects[r].right  = stream->readUint32LE() & 0xFFFF;
		sceneRects[r].bottom = stream->readUint32LE() & 0xFFFF;
	}

	sceneRectIdx  = stream->readByte();
	field_11D[0]  = stream->readByte();
	field_11D[1]  = stream->readByte();
	field_11D[2]  = stream->readByte();
	field_120     = stream->readUint32LE();
	actionListIdx = stream->readUint32LE();

	for (int gr = 0; gr < 100; gr++)
		grResId[gr] = stream->readUint32LE();

	sceneTitleGrResId  = stream->readUint32LE();
	sceneTitlePalResId = stream->readUint32LE();
	actorType          = stream->readUint32LE();

	for(int s = 0; s < 50; s++)
		soundResId[s] = stream->readUint32LE();

	for(int s=0; s < 15; s++) {
		ambientSounds[s].field_0  = stream->readUint32LE();
		ambientSounds[s].flags    = stream->readUint32LE();
		ambientSounds[s].resId    = stream->readUint32LE();
		ambientSounds[s].field_C  = stream->readUint32LE();
		ambientSounds[s].field_10 = stream->readUint32LE();
		ambientSounds[s].field_14 = stream->readUint32LE();

		for(int i = 0; i < 6; i++)
			ambientSounds[s].flagNum[i] = stream->readUint32LE();

		ambientSounds[s].x = stream->readUint32LE();
		ambientSounds[s].y = stream->readUint32LE();
	}

	numAmbientSound   = stream->readUint32LE();
	musicStatus       = stream->readUint32LE();
	musicCurrentResId = stream->readUint32LE();
	musicFlag         = stream->readUint32LE();
	musicResId        = stream->readUint32LE();
	musicStatusExt    = stream->readUint32LE();

	for (uint32 a = 0; a < numBarriers; a++) {
		int i;
		Barrier barrier;

		barrier.id	  = stream->readUint32LE();
		barrier.resId = stream->readUint32LE();
		barrier.x	  = stream->readUint32LE();
		barrier.y	  = stream->readUint32LE();

		barrier.boundingRect.left	= stream->readUint32LE() & 0xFFFF;
		barrier.boundingRect.top	= stream->readUint32LE() & 0xFFFF;
		barrier.boundingRect.right	= stream->readUint32LE() & 0xFFFF;
		barrier.boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

		barrier.field_20   = stream->readUint32LE();
		barrier.frameIdx   = stream->readUint32LE();
		barrier.frameCount = stream->readUint32LE();
		barrier.field_2C   = stream->readUint32LE();
		barrier.field_30   = stream->readUint32LE();
		barrier.field_34   = stream->readUint32LE();
		barrier.flags	   = stream->readUint32LE();
		barrier.field_3C   = stream->readUint32LE();

		stream->read(barrier.name, sizeof(barrier.name));

		barrier.field_74 = stream->readUint32LE();
		barrier.field_78 = stream->readUint32LE();
		barrier.field_7C = stream->readUint32LE();
		barrier.field_80 = stream->readUint32LE();
		barrier.polyIdx	 = stream->readUint32LE();
		barrier.flags2	 = stream->readUint32LE();

		for (i = 0; i < 10; i++)
			barrier.gameFlags[i] = stream->readUint32LE();

		barrier.field_B4	  = stream->readUint32LE();
		barrier.tickCount	  = stream->readUint32LE();
		barrier.tickCount2	  = stream->readUint32LE();
		barrier.field_C0	  = stream->readUint32LE();
		barrier.priority	  = stream->readUint32LE();
		barrier.actionListIdx = stream->readUint32LE();

		for (i = 0; i < 16; i++) {
			barrier.soundItems[i].resId	  = stream->readUint32LE();
			barrier.soundItems[i].field_4 = stream->readUint32LE();
			barrier.soundItems[i].field_8 = stream->readUint32LE();
			barrier.soundItems[i].field_C = stream->readUint32LE();

		}

		for (i = 0; i < 50; i++) {
			barrier.frameSoundItems[i].resId	= stream->readUint32LE();
			barrier.frameSoundItems[i].frameIdx = stream->readUint32LE();
			barrier.frameSoundItems[i].index	= stream->readUint32LE();
			barrier.frameSoundItems[i].field_C	= stream->readUint32LE();
			barrier.frameSoundItems[i].field_10 = stream->readUint32LE();
			barrier.frameSoundItems[i].field_14 = stream->readUint32LE();
		}

		barrier.field_67C = stream->readUint32LE();
		barrier.soundX	  = stream->readUint32LE();
		barrier.soundY	  = stream->readUint32LE();
		barrier.field_688 = stream->readUint32LE();

		for (i = 0; i < 5; i++)
			barrier.field_68C[i] = stream->readUint32LE();

		barrier.soundResId = stream->readUint32LE();
		barrier.field_6A4  = stream->readUint32LE();

		barriers.push_back(barrier);
	}

	// need to jump all unused barriers data to where actors data start
    stream->seek(0xA6D7A);

	for (uint32 a = 0; a < numActors; a++) {
		int i;
		Actor actor;

		actor.x          = stream->readUint32LE();
		actor.y          = stream->readUint32LE();
		actor.grResId    = stream->readUint32LE();
		actor.field_C    = stream->readUint32LE();
		actor.frameNum   = stream->readUint32LE();
		actor.frameCount = stream->readUint32LE();
		actor.x1         = stream->readUint32LE();
		actor.y1         = stream->readUint32LE();
		actor.x2         = stream->readUint32LE();
		actor.y2         = stream->readUint32LE();

		actor.boundingRect.left   = stream->readUint32LE() & 0xFFFF;
		actor.boundingRect.top    = stream->readUint32LE() & 0xFFFF;
		actor.boundingRect.right  = stream->readUint32LE() & 0xFFFF;
		actor.boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

		actor.direction  = stream->readUint32LE();
		actor.field_3C   = stream->readUint32LE();
		actor.updateType = stream->readUint32LE();
		actor.field_44	 = stream->readUint32LE();
		actor.field_48	 = stream->readUint32LE();
		actor.flags      = stream->readUint32LE();
		actor.field_50   = stream->readUint32LE();
		actor.field_54	 = stream->readUint32LE();
		actor.field_58	 = stream->readUint32LE();
		actor.field_5C	 = stream->readUint32LE();
		actor.field_60	 = stream->readUint32LE();
		actor.actionIdx3 = stream->readUint32LE();

		// TODO skip field_68 till field_617
		stream->skip(0x5B0);

		for (i = 0; i < 8; i++)
			actor.reaction[i] = stream->readUint32LE();

		actor.field_638     = stream->readUint32LE();
		actor.walkingSound1 = stream->readUint32LE();
		actor.walkingSound2 = stream->readUint32LE();
		actor.walkingSound3 = stream->readUint32LE();
		actor.walkingSound4 = stream->readUint32LE();
		actor.field_64C     = stream->readUint32LE();
		actor.field_650     = stream->readUint32LE();

		for (i = 0; i < 55; i++)
			actor.grResTable[i] = stream->readUint32LE();

		stream->read(actor.name, sizeof(actor.name));

		for (i = 0; i < 20; i++)
			actor.field_830[i] = stream->readUint32LE();

		for (i = 0; i < 20; i++)
			actor.field_880[i] = stream->readUint32LE();

		for (i = 0; i < 20; i++)
			actor.field_8D0[i] = stream->readUint32LE();

		actor.actionIdx2 = stream->readUint32LE();
		actor.field_924  = stream->readUint32LE();
		actor.tickValue1 = stream->readUint32LE();
		actor.field_92C  = stream->readUint32LE();
		actor.flags2     = stream->readUint32LE();
		actor.field_934  = stream->readUint32LE();
		actor.field_938  = stream->readUint32LE();
		actor.soundResId = stream->readUint32LE();

		// TODO skip field_940 till field_978
		stream->skip(0x3C);

		actor.actionIdx1 = stream->readUint32LE();

		// TODO skip field_980 till field_9A0
		stream->skip(0x24);

		actors.push_back(actor);
	}

	// TODO Take this out, it shouldn't be here (TEST ONLY)
	stream->seek(0xA73B6);

	uint8 mainActorData[500];
	stream->read(mainActorData, 500);

	// FIXME
	// This is ONLY ever going to work for scenes where there's only
	// one actor in the worldStats->actors[] collection
	actors[0].setRawResources(mainActorData);

	stream->seek(0xD6B5A); // where action items start

	// FIXME Figure out all the actions items
	for (uint32 a = 0; a < numActions; a++) {
		ActionArea action;
		memset(&action, 0, sizeof(ActionArea));

		stream->read(action.name,52);
		action.id             = stream->readUint32LE();
		action.field01        = stream->readUint32LE();
		action.field02        = stream->readUint32LE();
		action.field_40       = stream->readUint32LE();
		action.field_44       = stream->readUint32LE();
		action.flags          = stream->readUint32LE();
		action.actionListIdx1 = stream->readUint32LE();
		action.actionListIdx2 = stream->readUint32LE();
		action.actionType     = stream->readUint32LE();

		for (int aa1 = 0; aa1 < 10; aa1++)
			action.flagNums[aa1] = stream->readUint32LE();

		action.field_7C     = stream->readUint32LE();
		action.polyIdx      = stream->readUint32LE();
		action.field_84     = stream->readUint32LE();
		action.field_88     = stream->readUint32LE();
		action.soundResId   = stream->readUint32LE();
		action.field_90     = stream->readUint32LE();
		action.paletteValue = stream->readUint32LE();

		for (int aa2 = 0; aa2 < 5; aa2++)
			action.array[aa2] = stream->readUint32LE();

		action.volume = stream->readUint32LE();

		actions.push_back(action);
	}
}

} // end of namespace Asylum
