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

#include "engines/asylum/resources/data.h"

#include "common/rect.h"
#include "common/serializer.h"
#include "common/textconsole.h"

namespace Asylum {

SharedData::SharedData() {
	cdNumber = 0;
	actorEnableForStatus7 = false;
	_flag1 = false;
	_flag2 = false;
	_flag3 = false;
	matteBarHeight = 0;
	matteVar2 = 0;
	sceneCounter = 0;
	point.x = -1;
	point.y = -1;
	sceneXLeft = 0;
	sceneYTop = 0;
	sceneOffset = 0;
	sceneOffsetAdd = 0;
	memset(&cursorResources, kResourceNone, sizeof(cursorResources));
	memset(&sceneFonts, kResourceNone, sizeof(sceneFonts));
	//_currentPaletteId = kResourceNone;
	//memset(&cellShadeMasks, 0, sizeof(cellShadeMasks));
	smallCurUp = 0;
	smallCurDown = 0;
	encounterFrameBg = 0;
	_flagSkipDrawScene = false;
	matteVar1 = 0;
	actorUpdateEnabledCheck = false;
	matteInitialized = false;
	mattePlaySound = false;
	currentScreenUpdatesCount = 0;
	memset(&_data1, 0, sizeof(_data1));
	memset(&_data2, 0, sizeof(_data2));
	actorUpdateStatusEnabledCounter = 0;
	memset(&_data3, 0, sizeof(_data3));
	_flagScene1 = false;
	//memset(&_movies, 0, sizeof(_movies));
	actorUpdateStatus15Check = false;
	_flag2 = false;
	globalDirection = kDirectionN;
	memset(&_ambientTicks, 0, sizeof(_ambientTicks));

	// Screen updates
	_flagRedraw = false;
	nextScreenUpdate = 0;
	movieIndex = 0;
}

void SharedData::setData(ActorIndex index, int32 val) {
	if (index < 50)
		_data1[index] = val;
	else if (index < 60)
		_data2[index - 50] = val;
	else
	error("[SharedData::setData] index is outside valid values (was: %d, valid: [0:60]", index);
}

int32 SharedData::getData(ActorIndex index) {
	if (index < 50)
		return _data1[index];
	else if (index < 60)
		return _data2[index - 50];
	else
		error("[SharedData::getData] index is outside valid values (was: %d, valid: [0:60]", index);
}

void SharedData::setData2(ActorIndex index, bool val) {
	if (index <= 12 || index > 20)
		error("[SharedData::setData2] index is outside valid values (was: %d, valid: [13:20]", index);

	_data3[index - 12] = val;
}

bool SharedData::getData2(ActorIndex index) {
	if (index <= 12 || index > 20)
		error("[SharedData::getData2] index is outside valid values (was: %d, valid: [13:20]", index);

	return _data3[index - 12];
}

void SharedData::resetActorData() {
	for (int i = 0; i < 9; i++) {
		_data1[i] = 160;
		_data1[i + 18] = 0;
		_data3[i] = 0;
	}
}

uint32 SharedData::getAmbientTick(uint32 index) {
	if (index >= ARRAYSIZE(_ambientTicks))
		error("[SharedData::getAmbientTick] index is outside valid values (was: %d, valid: [0:%d]", index, ARRAYSIZE(_ambientTicks));

	return _ambientTicks[index];
};

void SharedData::setAmbientTick(uint32 index, uint32 val) {
	if (index >= ARRAYSIZE(_ambientTicks))
		error("[SharedData::setAmbientTick] index is outside valid values (was: %d, valid: [0:%d]", index, ARRAYSIZE(_ambientTicks));

	_ambientTicks[index] = val;
};

uint32 SharedData::getAmbientFlag(uint32 index) {
	if (index >= ARRAYSIZE(_ambientFlags))
		error("[SharedData::getAmbientFlag] index is outside valid values (was: %d, valid: [0:%d]", index, ARRAYSIZE(_ambientFlags));

	return _ambientFlags[index];
};

void SharedData::setAmbientFlag(uint32 index, uint32 val) {
	if (index >= ARRAYSIZE(_ambientFlags))
		error("[SharedData::setAmbientFlag] index is outside valid values (was: %d, valid: [0:%d]", index, ARRAYSIZE(_ambientFlags));

	_ambientFlags[index] = val;
};

void SharedData::resetAmbientFlags() {
	memset(&_ambientFlags, 0, sizeof(_ambientFlags));
}

//////////////////////////////////////////////////////////////////////////
// Flags
//////////////////////////////////////////////////////////////////////////
bool SharedData::getFlag(GlobalFlag flag) const {
	switch (flag) {
	default:
		error("[SharedData::getFlag] Invalid flag type (%d)!", flag);

	case kFlag1:
		return _flag1;

	case kFlag2:
		return _flag2;

	case kFlag3:
		return _flag3;

	case kFlagRedraw:
		return _flagRedraw;
		break;

	case kFlagSkipDrawScene:
		return _flagSkipDrawScene;

	case kFlagSceneRectChanged:
		error("[SharedData::getFlag] Invalid flag type (kFlagSceneRectChanged)!");

	case kFlagScene1:
		return _flagScene1;
	}
}

void SharedData::setFlag(GlobalFlag flag, bool state) {
	switch (flag) {
	default:
		error("[SharedData::getFlag] Invalid flag type (%d)!", flag);

	case kFlag1:
		_flag1 = state;
		break;

	case kFlag2:
		_flag2 = state;
		break;

	case kFlag3:
		_flag3 = state;
		break;

	case kFlagRedraw:
		_flagRedraw = state;
		break;

	case kFlagSkipDrawScene:
		_flagSkipDrawScene = state;
		break;

	case kFlagSceneRectChanged:
		error("[SharedData::getFlag] Invalid flag type (kFlagSceneRectChanged)!");
		break;

	case kFlagScene1:
		_flagScene1 = state;
		break;
	}
}

void SharedData::saveLoadAmbientSoundData(Common::Serializer &s) {
	// Ambient sound flags/ticks
	for (uint32 i = 0; i < ARRAYSIZE(_ambientFlags); i++)
		s.syncAsUint32LE(_ambientFlags[i]);

	for (uint32 i = 0; i < ARRAYSIZE(_ambientTicks); i++)
		s.syncAsUint32LE(_ambientTicks[i]);
}

void SharedData::saveLoadWithSerializer(Common::Serializer &s) {
	// Original skips two elements (original has one unused, one used for debugging screen update counts)
	s.skip(8);

	// Script queue (part of ScriptManager)

	// Global coordinates (original uses int32 for coordinates)
	s.syncAsSint32LE(point.x);
	s.syncAsSint32LE(point.y);

	// Processing skipped (part of ScriptManager)
	// Encounter running (part of Encounter)

	// Player index
	//s.syncAsUint32LE(playerIndex);

	// Scene coordinates
	s.syncAsSint32LE(sceneXLeft);
	s.syncAsSint32LE(sceneYTop);
	s.syncAsSint32LE(sceneOffset);
	s.syncAsSint32LE(sceneOffsetAdd);

	// Original skips 4 bytes
	s.skip(4);

	// Cursor resources
	for (uint32 i = 0; i < ARRAYSIZE(cursorResources); i++)
		s.syncAsUint32LE(cursorResources[i]);

	// Fonts
	for (uint32 i = 0; i < ARRAYSIZE(sceneFonts); i++)
		s.syncAsUint32LE(sceneFonts[i]);

	// Chapter 2 actor data (Part 1)
	//for (uint32 i = 0; i < ARRAYSIZE(chapter2ActorData1); i++)
	//	s.syncAsUint32LE(chapter2ActorData1[i]);

	// Original skips 4 bytes
	s.skip(4);

	// Scene information
	s.syncAsSint32LE(smallCurUp);
	s.syncAsSint32LE(smallCurDown);
	s.syncAsSint32LE(encounterFrameBg);
	s.syncAsUint32LE(_flagSkipDrawScene);
	s.syncAsSint32LE(matteVar1);
	s.syncAsUint32LE(actorUpdateEnabledCheck);
	s.syncAsUint32LE(matteInitialized);
	s.syncAsUint32LE(mattePlaySound);
	s.syncAsSint32LE(currentScreenUpdatesCount);

	// Chapter 2 actor data (Part 2)
	//for (uint32 i = 0; i < ARRAYSIZE(chapter2ActorData2); i++)
	//	s.syncAsUint32LE(chapter2ActorData2[i]);

	// Chapter 2 counters (1-4)
	//s.syncAsSint32LE(chapter2Counter1);
	//s.syncAsSint32LE(chapter2Counter2);
	//s.syncAsSint32LE(chapter2Counter3);
	//s.syncAsSint32LE(chapter2Counter4);

	// Original skips 4 bytes
	s.skip(4);

	// Chapter 2 actor data (Part 3)
	//for (uint32 i = 0; i < ARRAYSIZE(chapter2ActorData3); i++)
	//	s.syncAsUint32LE(chapter2ActorData3[i]);

	// Original skips 4 bytes
	s.skip(4);

	// Chapter2 points
	//for (uint32 i = 0; i < ARRAYSIZE(chapter2Points); i++) {
	//	s.syncAsSint32LE(chapter2Points[i].x);
	//	s.syncAsSint32LE(chapter2Points[i].y);
	//}

	// Original skips 4 bytes
	s.skip(4);

	// Chapter 2 counter (5) and other data
	//s.syncAsSint32LE(chapter2Counter5);
	//s.syncAsSint32LE(chapter2FrameIndexOffset);
	//s.syncAsSint32LE(chapter2ActorIndex);

	//s.syncAsSint32LE(eventUpdateFlag);

	// Chapter 2 counters (6-8)
	//s.syncAsSint32LE(chapter2Counter6);
	//s.syncAsSint32LE(chapter2Counter7);
	//s.syncAsSint32LE(chapter2Counter8);

	// Original skips 7 * 4 bytes
	s.skip(7 * 4);

	// Chapter 2 actor data (Part 4)
	//for (uint32 i = 0; i < ARRAYSIZE(chapter2ActorData4); i++)
	//	s.syncAsUint32LE(chapter2ActorData4[i]);

	// Original skips 8 * 4 bytes
	s.skip(8 * 4);

	// Actor UpdateStatusEnabled Counter
	s.syncAsSint32LE(actorUpdateStatusEnabledCounter);

	// Chapter2 actor data (part 5)
	//for (uint32 i = 0; i < ARRAYSIZE(chapter2ActorData5); i++)
	//	s.syncAsUint32LE(chapter2ActorData5[i]);

	// Encounter disable player on exit
	//s.syncAsSint32LE(encounterDisablePlayerOnExit);

	// Scene flag 1
	s.syncAsUint32LE(_flagScene1);

	// Next screen update
	s.syncAsUint32LE(nextScreenUpdate);

	// Viewed movies (we also load it from an external file...)
	//s.syncBytes(&_moviesViewed, sizeof(_moviesViewed));

	// Actor update status 15 check
	s.syncAsUint32LE(actorUpdateStatus15Check);

	// TODO More?
}

} // End of namespace Asylum
