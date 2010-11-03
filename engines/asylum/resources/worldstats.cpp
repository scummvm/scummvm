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
	CLEAR_ARRAY(Barrier, barriers);
	CLEAR_ARRAY(Actor, actors);
	CLEAR_ARRAY(ActionArea, actions);
}

int32 WorldStats::getActionAreaIndexById(int32 id) {
	for (int32 i = 0; i < numActions; i++) {
		if (actions[i]->id == id)
			return i;
	}

	return -1;
}

ActionArea* WorldStats::getActionAreaById(int32 id) {
	int index = getActionAreaIndexById(id);
	if (index == -1)
		error("[WorldStats::getActionAreaById] action id is invalid");

	return actions[index];
}

int32 WorldStats::getBarrierIndexById(int32 id) {
	for (int32 i = 0; i < numBarriers; i++) {
		if (barriers[i]->id == id)
			return i;
	}

	return -1;
}

Barrier* WorldStats::getBarrierById(int32 id) {
	return barriers[getBarrierIndexById(id)];
}

Barrier* WorldStats::getBarrierByIndex(int32 idx) {
	return barriers[idx];
}

bool WorldStats::isBarrierOnScreen(int32 idx) {
	Barrier *b = getBarrierByIndex(idx);

	Common::Rect screenRect  = Common::Rect(xLeft, yTop, xLeft + 640, yTop + 480);
	Common::Rect barrierRect = b->boundingRect;
	barrierRect.translate(b->x, b->y);
	return isBarrierVisible(idx) && (b->flags & 1) && screenRect.intersects(barrierRect);
}

bool WorldStats::isBarrierVisible(int32 idx) {
	Barrier *b = getBarrierByIndex(idx);

	if ((b->flags & 0xFF) & 1) {
		for (int32 f = 0; f < 10; f++) {
			bool   isSet = false;
			GameFlag flag  = b->gameFlags[f];

			if (flag <= 0)
				isSet = _scene->vm()->isGameFlagNotSet(flag); // -flag
			else
				isSet = _scene->vm()->isGameFlagSet(flag);

			if (!isSet)
				return false;
		}
		return true;
	}
	return false;
}

bool WorldStats::checkBarrierFlagsCondition(int32 idx) {
	Barrier *b = getBarrierByIndex(idx);
	bool result;

	if (LOBYTE(b->flags) & 1) {
		for (int32 i = 0; i < 10; i++) {
			result = _scene->vm()->isGameFlagSet(b->gameFlags[i]);
			if (result)
				return result;
		}
		result = true;
	} else {
		result = false;
	}

	return result;
}

// FIXME: load necessary World Stats content
void WorldStats::load(Common::SeekableReadStream *stream) {
	size       = stream->readSint32LE();
	numEntries = stream->readSint32LE();

	numChapter = stream->readSint32LE();
	xLeft      = stream->readSint32LE();
	yTop       = stream->readSint32LE();

	boundingRect.left   = stream->readSint32LE() & 0xFFFF;
	boundingRect.top    = stream->readSint32LE() & 0xFFFF;
	boundingRect.right  = stream->readSint32LE() & 0xFFFF;
	boundingRect.bottom = stream->readSint32LE() & 0xFFFF;

	// read common graphic resources
	backgroundImage    = stream->readSint32LE();
	curScrollUp        = stream->readSint32LE();
	curScrollUpLeft    = stream->readSint32LE();
	curScrollLeft      = stream->readSint32LE();
	curScrollDownLeft  = stream->readSint32LE();
	curScrollDown      = stream->readSint32LE();
	curScrollDownRight = stream->readSint32LE();
	curScrollRight     = stream->readSint32LE();
	curScrollUpRight   = stream->readSint32LE();
	curHand            = stream->readSint32LE();
	curMagnifyingGlass = stream->readSint32LE();
	curTalkNPC         = stream->readSint32LE();
	curGrabPointer     = stream->readSint32LE();
	curTalkNPC2        = stream->readSint32LE();
	font1              = stream->readSint32LE();
	font2              = stream->readSint32LE();
	font3              = stream->readSint32LE();
	currentPaletteId   = stream->readSint32LE();
	cellShadeMask1     = stream->readSint32LE();
	cellShadeMask2     = stream->readSint32LE();
	cellShadeMask3     = stream->readSint32LE();
	unused             = stream->readSint32LE();
	smallCurUp         = stream->readSint32LE();
	smallCurDown       = stream->readSint32LE();
	encounterFrameBg   = stream->readSint32LE();

	width        = stream->readSint32LE();
	height       = stream->readSint32LE();
	motionStatus = stream->readSint32LE();
	field_8C     = stream->readSint32LE();
	numActions   = stream->readSint32LE();
	numBarriers  = stream->readSint32LE();
	targetX      = stream->readSint32LE();
	targetY      = stream->readSint32LE();
	field_A0     = stream->readSint32LE();
	field_A4     = stream->readSint32LE();
	field_A8     = stream->readSint32LE();
	field_AC     = stream->readSint32LE();
	field_B0     = stream->readSint32LE();
	numActors    = stream->readSint32LE();

	stereoReversedFlag = stream->readSint32LE();

	for (int32 r = 0; r < 6; r++) {
		sceneRects[r].left   = stream->readSint32LE() & 0xFFFF;
		sceneRects[r].top    = stream->readSint32LE() & 0xFFFF;
		sceneRects[r].right  = stream->readSint32LE() & 0xFFFF;
		sceneRects[r].bottom = stream->readSint32LE() & 0xFFFF;
	}

	sceneRectIdx  = stream->readByte();
	field_11D[0]  = stream->readByte();
	field_11D[1]  = stream->readByte();
	field_11D[2]  = stream->readByte();
	field_120     = stream->readSint32LE();
	actionListIdx = stream->readSint32LE();

	for (int32 gr = 0; gr < 100; gr++)
		graphicResourceIds[gr] = stream->readSint32LE();

	sceneTitleGraphicResourceId  = stream->readSint32LE();
	sceneTitlePaletteResourceId = stream->readSint32LE();
	actorType          = stream->readSint32LE();

	for (int32 s = 0; s < 50; s++)
		soundResourceIds[s] = stream->readSint32LE();

	for (int32 s = 0; s < 15; s++) {
		ambientSounds[s].field_0  = stream->readSint32LE();
		ambientSounds[s].flags    = stream->readSint32LE();
		ambientSounds[s].resourceId    = stream->readSint32LE();
		ambientSounds[s].field_C  = stream->readSint32LE();
		ambientSounds[s].field_10 = stream->readSint32LE();
		ambientSounds[s].field_14 = stream->readSint32LE();

		for (int32 i = 0; i < 6; i++)
			ambientSounds[s].flagNum[i] = (GameFlag)stream->readSint32LE();

		ambientSounds[s].x = stream->readSint32LE();
		ambientSounds[s].y = stream->readSint32LE();
	}

	numAmbientSound   = stream->readSint32LE();
	musicStatus       = stream->readSint32LE();
	musicCurrentResourceId = stream->readSint32LE();
	musicFlag         = stream->readSint32LE();
	musicResourceId        = stream->readSint32LE();
	musicStatusExt    = stream->readSint32LE();

	for (int32 a = 0; a < numBarriers; a++) {
		int32 i;
		Barrier *barrier = new Barrier(_scene);

		barrier->id	  = stream->readSint32LE();
		barrier->resourceId = stream->readSint32LE();
		barrier->x	  = stream->readSint32LE();
		barrier->y	  = stream->readSint32LE();

		barrier->boundingRect.left	= stream->readSint32LE() & 0xFFFF;
		barrier->boundingRect.top	= stream->readSint32LE() & 0xFFFF;
		barrier->boundingRect.right	= stream->readSint32LE() & 0xFFFF;
		barrier->boundingRect.bottom = stream->readSint32LE() & 0xFFFF;

		barrier->field_20   = stream->readSint32LE();
		barrier->frameIdx   = stream->readSint32LE();
		barrier->frameCount = stream->readSint32LE();
		barrier->field_2C   = stream->readSint32LE();
		barrier->field_30   = stream->readSint32LE();
		barrier->field_34   = stream->readSint32LE();
		barrier->flags	   = stream->readSint32LE();
		barrier->field_3C   = stream->readSint32LE();

		stream->read(barrier->name, sizeof(barrier->name));

		barrier->field_74 = stream->readSint32LE();
		barrier->field_78 = stream->readSint32LE();
		barrier->field_7C = stream->readSint32LE();
		barrier->field_80 = stream->readSint32LE();
		barrier->polyIdx	 = stream->readSint32LE();
		barrier->actionType	 = stream->readSint32LE();

		for (i = 0; i < 10; i++)
			barrier->gameFlags[i] = (GameFlag)stream->readSint32LE();

		barrier->field_B4	  = stream->readSint32LE();
		barrier->tickCount	  = stream->readSint32LE();
		barrier->tickCount2	  = stream->readSint32LE();
		barrier->field_C0	  = stream->readSint32LE();
		barrier->priority	  = stream->readSint32LE();
		barrier->actionListIdx = stream->readSint32LE();

		for (i = 0; i < 16; i++) {
			barrier->soundItems[i].resourceId	  = stream->readSint32LE();
			barrier->soundItems[i].field_4 = stream->readSint32LE();
			barrier->soundItems[i].field_8 = stream->readSint32LE();
			barrier->soundItems[i].field_C = stream->readSint32LE();

		}

		for (i = 0; i < 50; i++) {
			barrier->frameSoundItems[i].resourceId	= stream->readSint32LE();
			barrier->frameSoundItems[i].frameIdx = stream->readSint32LE();
			barrier->frameSoundItems[i].index	= stream->readSint32LE();
			barrier->frameSoundItems[i].field_C	= stream->readSint32LE();
			barrier->frameSoundItems[i].field_10 = stream->readSint32LE();
			barrier->frameSoundItems[i].field_14 = stream->readSint32LE();
		}

		barrier->field_67C = stream->readSint32LE();
		barrier->soundX	  = stream->readSint32LE();
		barrier->soundY	  = stream->readSint32LE();
		barrier->field_688 = stream->readSint32LE();

		for (i = 0; i < 5; i++)
			barrier->field_68C[i] = stream->readSint32LE();

		barrier->soundResourceId = stream->readSint32LE();
		barrier->field_6A4  = stream->readSint32LE();

		barriers.push_back(barrier);
	}

	// need to jump all unused barriers data to where actors data start
	stream->seek(0xA6D7A);

	for (ActorIndex index = 0; index < numActors; index++) {
		Actor *actor = new Actor(_scene, index);
		actor->load(stream);

		actors.push_back(actor);
	}

	// TODO Take this out, it shouldn't be here (TEST ONLY)
	stream->seek(0xA73B6);

	uint8 mainActorData[500];
	stream->read(mainActorData, 500);

	// FIXME
	// This is ONLY ever going to work for scenes where there's only
	// one actor in the worldStats->actors[] collection
	actors[0]->setRawResources(mainActorData);

	stream->seek(0xD6B5A); // where action items start

	// FIXME Figure out all the actions items
	for (int32 a = 0; a < numActions; a++) {
		ActionArea *action = new ActionArea();

		stream->read(action->name, 52);
		action->id             = stream->readSint32LE();
		action->field01        = stream->readSint32LE();
		action->field02        = stream->readSint32LE();
		action->field_40       = stream->readSint32LE();
		action->field_44       = stream->readSint32LE();
		action->flags          = stream->readSint32LE();
		action->scriptIndex = stream->readSint32LE();
		action->actionListIdx2 = stream->readSint32LE();
		action->actionType     = stream->readSint32LE();

		for (int32 aa1 = 0; aa1 < 10; aa1++)
			action->flagNums[aa1] = stream->readSint32LE();

		action->field_7C     = stream->readSint32LE();
		action->polyIdx      = stream->readSint32LE();
		action->field_84     = stream->readSint32LE();
		action->field_88     = stream->readSint32LE();
		action->soundResourceId   = stream->readSint32LE();
		action->field_90     = stream->readSint32LE();
		action->paletteValue = stream->readSint32LE();

		for (int32 aa2 = 0; aa2 < 5; aa2++)
			action->array[aa2] = stream->readSint32LE();

		action->volume = stream->readSint32LE();

		actions.push_back(action);
	}
}

} // end of namespace Asylum
