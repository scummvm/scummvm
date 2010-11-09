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

#include "asylum/resources/worldstats.h"

#include "asylum/resources/actionlist.h"
#include "asylum/resources/actor.h"
#include "asylum/resources/object.h"

#include "asylum/views/scene.h"

namespace Asylum {

WorldStats::WorldStats(Common::SeekableReadStream *stream, Scene *scene) : _scene(scene) {
	load(stream);
}

WorldStats::~WorldStats() {
	CLEAR_ARRAY(Object, objects);
	CLEAR_ARRAY(Actor, actors);
	CLEAR_ARRAY(ActionArea, actions);
}

// FIXME: load necessary World Stats content
void WorldStats::load(Common::SeekableReadStream *stream) {
	size       = stream->readSint32LE();
	numEntries = stream->readSint32LE();

	chapter = (ChapterIndex)stream->readSint32LE();
	xLeft      = stream->readSint32LE();
	yTop       = stream->readSint32LE();

	boundingRect.left   = stream->readSint32LE() & 0xFFFF;
	boundingRect.top    = stream->readSint32LE() & 0xFFFF;
	boundingRect.right  = stream->readSint32LE() & 0xFFFF;
	boundingRect.bottom = stream->readSint32LE() & 0xFFFF;

	// read common graphic resources
	backgroundImage    = (ResourceId)stream->readSint32LE();
	curScrollUp        = (ResourceId)stream->readSint32LE();
	curScrollUpLeft    = (ResourceId)stream->readSint32LE();
	curScrollLeft      = (ResourceId)stream->readSint32LE();
	curScrollDownLeft  = (ResourceId)stream->readSint32LE();
	curScrollDown      = (ResourceId)stream->readSint32LE();
	curScrollDownRight = (ResourceId)stream->readSint32LE();
	curScrollRight     = (ResourceId)stream->readSint32LE();
	curScrollUpRight   = (ResourceId)stream->readSint32LE();
	curHand            = (ResourceId)stream->readSint32LE();
	curMagnifyingGlass = (ResourceId)stream->readSint32LE();
	curTalkNPC         = (ResourceId)stream->readSint32LE();
	curGrabPointer     = (ResourceId)stream->readSint32LE();
	curTalkNPC2        = (ResourceId)stream->readSint32LE();
	font1              = (ResourceId)stream->readSint32LE();
	font2              = (ResourceId)stream->readSint32LE();
	font3              = (ResourceId)stream->readSint32LE();
	currentPaletteId   = (ResourceId)stream->readSint32LE();
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
	uint32 numActions   = stream->readUint32LE();
	uint32 numObjects  = stream->readUint32LE();

	for (int32 c = 0; c < 7; c++)
		coordinates[c] = stream->readSint32LE();

	uint32 numActors    = stream->readUint32LE();

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
	scriptIndex = stream->readSint32LE();

	for (int32 gr = 0; gr < 100; gr++)
		graphicResourceIds[gr] = (ResourceId)stream->readSint32LE();

	sceneTitleGraphicResourceId  = (ResourceId)stream->readSint32LE();
	sceneTitlePaletteResourceId = (ResourceId)stream->readSint32LE();
	actorType          = stream->readUint32LE();

	for (int32 s = 0; s < 50; s++)
		soundResourceIds[s] = (ResourceId)stream->readSint32LE();

	for (int32 s = 0; s < 15; s++) {
		ambientSounds[s].field_0  = stream->readSint32LE();
		ambientSounds[s].flags    = stream->readSint32LE();
		ambientSounds[s].resourceId    = (ResourceId)stream->readSint32LE();
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
	musicCurrentResourceIndex = stream->readSint32LE();
	musicFlag         = stream->readSint32LE();
	musicResourceId        = (ResourceId)stream->readSint32LE();
	musicStatusExt    = stream->readSint32LE();

	for (uint32 a = 0; a < numObjects; a++) {
		Object *object = new Object(_scene->vm());
		object->load(stream);

		objects.push_back(object);
	}

	// Jump over unused objects
	stream->seek((OBJECTS_MAX_COUNT - numObjects) * OBJECTS_SIZE, SEEK_CUR);

	for (ActorIndex index = 0; index < (int)numActors; index++) {
		Actor *actor = new Actor(_scene->vm(), index);
		actor->load(stream);

		actors.push_back(actor);
	}

	// Jump over unused actors
	stream->seek((ACTORS_MAX_COUNT - numActors) * ACTORS_SIZE, SEEK_CUR);

	uint8 mainActorData[500];
	stream->read(mainActorData, 500);

	// FIXME
	// This is ONLY ever going to work for scenes where there's only
	// one actor in the worldStats->actors[] collection
	actors[0]->setRawResources(mainActorData);

	stream->seek(0xD6B5A); // where action items start

	// FIXME Figure out all the actions items
	for (uint32 a = 0; a < numActions; a++) {
		ActionArea *action = new ActionArea();

		stream->read(action->name, 52);
		action->id             = stream->readSint32LE();
		action->field01        = stream->readSint32LE();
		action->field02        = stream->readSint32LE();
		action->field_40       = stream->readSint32LE();
		action->field_44       = stream->readSint32LE();
		action->flags          = stream->readSint32LE();
		action->scriptIndex = stream->readSint32LE();
		action->scriptIndex2 = stream->readSint32LE();
		action->actionType     = stream->readSint32LE();

		for (int32 aa1 = 0; aa1 < 10; aa1++)
			action->flagNums[aa1] = stream->readSint32LE();

		action->field_7C     = stream->readSint32LE();
		action->polyIdx      = stream->readSint32LE();
		action->field_84     = stream->readSint32LE();
		action->field_88     = stream->readSint32LE();
		action->soundResourceId   = (ResourceId)stream->readSint32LE();
		action->field_90     = stream->readSint32LE();
		action->paletteResourceId = (ResourceId)stream->readSint32LE();

		for (int32 aa2 = 0; aa2 < 5; aa2++)
			action->array[aa2] = stream->readSint32LE();

		action->volume = stream->readSint32LE();

		actions.push_back(action);
	}

	// Jump over unused actions
	stream->seek((ACTIONS_MAX_COUNT - numActions) * ACTIONS_SIZE, SEEK_CUR);
}

//////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////
int32 WorldStats::getActionAreaIndexById(int32 id) {
	for (uint32 i = 0; i < actions.size(); i++) {
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

Object* WorldStats::getObjectById(ObjectId id) {
	for (uint32 i = 0; i < objects.size(); i++)
		if (objects[i]->getId() == id)
			return objects[i];

	return NULL;
}

void WorldStats::setWheelObjects() {
	wheels[0] = getObjectById(kObjectWheel1);
	wheels[1] = getObjectById(kObjectWheel2);
	wheels[2] = getObjectById(kObjectWheel3);
	wheels[3] = getObjectById(kObjectWheel4);
	wheels[4] = getObjectById(kObjectWheel5);
	wheels[6] = getObjectById(kObjectWheel6);
	wheels[7] = getObjectById(kObjectWheel7);
}

Common::String WorldStats::toString() {
	Common::String output;

	output += Common::String::format("xLeft:          %d\n", xLeft);
	output += Common::String::format("yTop:           %d\n", yTop);
	output += Common::String::format("boundingRect:   top[%d] left[%d] right[%d] bottom[%d]: \n", boundingRect.top, boundingRect.left, boundingRect.right, boundingRect.bottom);
	output += Common::String::format("width:          %d\n", width);
	output += Common::String::format("height:         %d\n", height);
	output += Common::String::format("motionStatus:   %d\n", motionStatus);
	output += "coordinates:    ";
	for (int i = 0; i < 7; i++)
		output += Common::String::format("%d[%d] ", i, coordinates[i]);
	output += "\n";
	output += Common::String::format("sceneRectIndex: %d\n", sceneRectIdx);
	output += Common::String::format("sceneRects:     0: top[%d] left[%d] right[%d] bottom[%d]\n", sceneRects[0].top, sceneRects[0].left, sceneRects[0].right, sceneRects[0].bottom);
	for (int i = 1; i < 6; i++)
		output += Common::String::format("                %d: top[%d] left[%d] right[%d] bottom[%d]\n", i, sceneRects[i].top, sceneRects[i].left, sceneRects[i].right, sceneRects[i].bottom);
	output += Common::String::format("scriptInex:     %d\n", scriptIndex);
	output += Common::String::format("actorType:      %d\n", actorType);
	output += Common::String::format("musicStatus:    %d\n", musicStatus);

	return output;
}
} // end of namespace Asylum
