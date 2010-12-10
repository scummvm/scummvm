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

#ifndef ASYLUM_DATA_H
#define ASYLUM_DATA_H

#include "engines/asylum/shared.h"

#include "common/rational.h"
#include "common/rect.h"

namespace Asylum {

//////////////////////////////////////////////////////////////////////////
// Flags
enum GlobalFlag {
	kFlag1,
	kFlag2,
	kFlag3,
	kFlagRedraw,
	kFlagSkipDrawScene,
	kFlagSceneRectChanged,
	kFlagScene1
};

/**
 * Shared data
 * -----------
 *  uint32 {1}      - actorEnableForStatus7
 *
 *  uint32 {1}      - global direction
 *  uint32 {1}      - lastScreenUpdate
 *
 *  uint32 {1}      - actor update counter
 *
 *  uint32 {1}      - matte bar height
 *
 *  Lots of data
 *
 *  -- Scene data (reset on scene load)
 *  uint32 {16}     - ambient sound panning array
 *  uint32 {1}      - scene counter
 *  -- Script queue (stored in ScriptManager - reset on scene change)
 *  uint32 {1}      - global Object X
 *  uint32 {1}      - global Object Y
 *  -- Skip processing flag (stored in ScriptManager)
 *  uint32 {1}      - Encounter flag 2
 *  -- Player ActorIndex (reset on scene enter)
 *  uint32 {1}      -  scene xLeft
 *  uint32 {1}      -  scene yTop
 *  uint32 {1}      -  scene offset
 *  uint32 {1}      -  scene offsetAdd
 *  -- UNUSED ??
 *  uint32 {13}    - cursor resources
 *  uint32 {3}     - scene fonts (3)
 *  uint32 {1}     - current palette Id
 *  uint32 {3}     - cellshade masks (3)
 *  uint32 {1}     - small cursor Up
 *  uint32 {1}     - small cursor Down
 *  uint32 {1}     - Encounter frame background
 *  uint32 {1}     - Flag skip draw scene
 *  uint32 {1}     - matte var 1
 *  uint32 {1}     - actorUpdateEnabledCheck
 *  uint32 {1}     - matte Initialized
 *  uint32 {1}     - matte playSound
 *  uint32 {1}     - currentScreenUpdatesCount
 *  uint32 {50}    - Actor data
 *  uint32 {11}    - UNUSED ?? (Actor data 2)
 *  uint32 {1}     - actorUpdateEnabledCounter
 *  uint32 {9}     - Actor data 3
 *  uint32 {1}     - Encounter flag 5
 *  uint32 {1}     - Scene flag 1
 *  uint32 {1}     - nextScreenUpdate
 *	uint32 {49}    - Viewed movies
 *  uint32 {1}     - actorUpdateStatus15Check
 *  -- Skip opening movie command line flag (not used)
 *  uint32 {1}     - Encounter flag 3
 *  uint32 {1}     - Flag 2
 *  uint32 {1}     - Flag 5
 *  -- scripts (reset on scene load)
 *  -- polygons (reset on scene load)
 */
struct SharedData {
public:
	SharedData() {
		cdNumber = 0;
		actorEnableForStatus7 = false;
		_flag1 = false;
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
		memset(&_ambientTick, 0, sizeof(_ambientTick));

		// Screen updates
		_flagRedraw = false;
		nextScreenUpdate = 0;
	}

	// Public variables
	int32 cdNumber;

	// Saved scene data
	ResourceId      cursorResources[11];
	ResourceId      sceneFonts[3];
	int32           smallCurUp;
	int32           smallCurDown;
	int32           encounterFrameBg;

	Common::Point   point; // global point
	uint32          sceneCounter;

	// Global scene coordinates and offset
	int32           sceneXLeft;
	int32           sceneYTop;
	int32           sceneOffset;
	int32           sceneOffsetAdd;

	// Actor
	Common::Point   vector1;
	Common::Point   vector2;
	bool            actorEnableForStatus7;
	bool            actorUpdateEnabledCheck;
	int32           actorUpdateStatusEnabledCounter;
	bool            actorUpdateStatus15Check;
	ActorDirection  globalDirection;

	// Matte bars
	uint32          matteBarHeight;
	int32           matteVar1;
	uint32          matteVar2;
	bool            matteInitialized;
	bool            mattePlaySound;

	// Screen updates
	uint32          nextScreenUpdate;
	int32           currentScreenUpdatesCount;


	// Accessors
	void  setActorUpdateFlag(int32 val)  { _data1[40] = 2; }
	void  setActorUpdateFlag2(int32 val) { _data1[36] = val; }

	int32 getActorUpdateFlag2() { return _data1[36]; }

	void setData(ActorIndex index, int32 val) {
		if (index < 50)
			_data1[index] = val;
		else if (index < 60)
			_data2[index - 50] = val;
		else
		error("[SharedData::setData] index is outside valid values (was: %d, valid: [0:60]", index);
	}

	int32 getData(ActorIndex index) {
		if (index < 50)
			return _data1[index];
		else if (index < 60)
			return _data2[index - 50];
		else
			error("[SharedData::getData] index is outside valid values (was: %d, valid: [0:60]", index);
	}

	void setData2(ActorIndex index, bool val) {
		if (index <= 12 || index > 20)
			error("[SharedData::setData2] index is outside valid values (was: %d, valid: [13:20]", index);

		_data3[index - 12] = val;
	}

	bool getData2(ActorIndex index) {
		if (index <= 12 || index > 20)
			error("[SharedData::getData2] index is outside valid values (was: %d, valid: [13:20]", index);

		return _data3[index - 12];
	}

	// Used by Actor::enableActorsChapter2 (and maybe others)
	void resetActorData() {
		for (int i = 0; i < 9; i++) {
			_data1[i] = 160;
			_data1[i + 18] = 0;
			_data3[i] = 0;
		}
	}

	uint32 *getAmbientTick(uint32 index) {
		if (index >= ARRAYSIZE(_ambientTick))
			error("[SharedData::getAmbientTick] index is outside valid values (was: %d, valid: [0:15]", index);

		return &_ambientTick[index];
	};

	//////////////////////////////////////////////////////////////////////////
	// Flags
	//////////////////////////////////////////////////////////////////////////
	bool getFlag(GlobalFlag flag) const {
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

	void setFlag(GlobalFlag flag, bool state) {
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

private:
	uint32          _ambientTick[16];

	// Flags
	bool            _flag1;
	bool            _flag2;
	bool            _flag3;
	bool            _flagSkipDrawScene;
	bool            _flagScene1;
	bool            _flagRedraw;

	// Shared data
	int32           _data1[50];
	int32           _data2[11];
	bool            _data3[9];
};

} // End of namespace Asylum

#endif // ASYLUM_DATA_H
