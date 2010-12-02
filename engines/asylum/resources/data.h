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
		_actorEnableForStatus7 = false;
		_flag1 = false;
		_actorUpdateCounter = 0;
		_matteBarHeight = 0;
		_matteVar2 = 0;
		_sceneCounter = 0;
		_point.x = -1;
		_point.y = -1;
		_sceneXLeft = 0;
		_sceneYTop = 0;
		_sceneOffset = 0;
		_sceneOffsetAdd = 0;
		memset(&_cursorResources[13], kResourceNone, sizeof(_cursorResources));
		memset(&_sceneFonts[3], kResourceNone, sizeof(_sceneFonts));
		_currentPaletteId = kResourceNone;
		memset(&_cellShadeMasks[3], 0, sizeof(_cellShadeMasks));
		_smallCurUp = 0;
		_smallCurDown = 0;
		_encounterFrameBg = 0;
		_flagSkipDrawScene = false;
		_matteVar1 = 0;
		_actorUpdateEnabledCheck = false;
		_matteInitialized = false;
		_mattePlaySound = false;
		_currentScreenUpdatesCount = 0;
		memset(&_data1[50], 0, sizeof(_data1));
		memset(&_data2[11], 0, sizeof(_data2));
		_actorUpdateStatusEnabledCounter = 0;
		memset(&_data3[9], 0, sizeof(_data3));
		_flagScene1 = false;
		memset(&_movies[49], 0, sizeof(_movies));
		_actorUpdateStatus15Check = false;
		_flag2 = false;
		_globalDirection = kDirectionN;

		// Screen updates
		_flagRedraw = false;
		_nextScreenUpdate = 0;
	}

	// Accessors
	int32 getActorUpdateEnabledCounter() { return _actorUpdateStatusEnabledCounter; }
	void  setActorUpdateEnabledCounter(int32 val) { _actorUpdateStatusEnabledCounter = val; }

	bool getActorEnableForStatus7() { return _actorEnableForStatus7; }
	void setActorEnableForStatus7(bool state) { _actorEnableForStatus7 = state; }

	bool getActorUpdateEnabledCheck() { return _actorUpdateEnabledCheck; }

	ActorDirection getGlobalDirection() { return _globalDirection; }

	void setActorUpdateFlag(int32 val) { _data1[40] = 2; }

	int32 getUpdateCounter() { return _actorUpdateCounter; }
	void  setUpdateCounter(int32 val) { _actorUpdateCounter = val; }

	int32 getActorUpdateFlag2() { return _data1[36]; }
	void setActorUpdateFlag2(int32 val) { _data1[36] = val; }

	// Shared global Data
	Common::Point *getPoint() { return &_point; }

	int32 getMatteBarHeight() { return _matteBarHeight;}
	void setMatteBarHeight(int32 val) { _matteBarHeight = val; }

	int32 getMatteVar1() { return _matteVar1; }
	void setMatteVar1(int32 val) { _matteVar1 = val; }

	int32 getMatteVar2() { return _matteVar2; }
	void setMatteVar2(int32 val) { _matteVar2 = val; }

	bool getMatteInitialized() { return _matteInitialized; }
	void setMatteInitialized(bool state) { _matteInitialized = state; }

	bool getMattePlaySound() { return _mattePlaySound; }
	void setMattePlaySound(bool state) { _mattePlaySound = state; }

	// Scene
	void setSceneXLeft(int32 val) { _sceneXLeft = val; }
	void setSceneYTop(int32 val) { _sceneYTop = val; }
	void setSceneOffset(int32 val) { _sceneOffset = val; }
	void setSceneOffsetAdd(const Common::Rational &offset) { _sceneOffsetAdd = offset; }

	Common::Point *getVector1() { return &_vector1; }
	Common::Point *getVector2() { return &_vector2; }

	void setNextScreenUpdate(uint32 ticks) { _nextScreenUpdate = ticks; }
	uint32 getNextScreenUpdate() { return _nextScreenUpdate; }

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
	Common::Point   _vector1;
	Common::Point   _vector2;

	bool            _actorEnableForStatus7;

	bool            _flag1;

	uint32          _actorUpdateCounter;
	uint32          _matteBarHeight;
	uint32          _matteVar2;

	// Lots of other data

	//////////////////////////////////////////////////////////////////////////
	// Shared data
	// (Some functions access those by offset, so until we figure out what
	//  exact data they need, we keep all of them here)
	//////////////////////////////////////////////////////////////////////////

	// TODO Add ambient sound panning array
	uint32           _sceneCounter;
	Common::Point    _point; // global point
	//bool             _flagEncouter2;
	int32            _sceneXLeft;
	int32            _sceneYTop;
	int32            _sceneOffset;
	Common::Rational _sceneOffsetAdd;

	// Saved scene data
	ResourceId      _cursorResources[13];
	ResourceId      _sceneFonts[3];
	ResourceId      _currentPaletteId;
	int32           _cellShadeMasks[3];
	// unused
	int32           _smallCurUp;
	int32           _smallCurDown;
	int32           _encounterFrameBg;


	bool            _flagSkipDrawScene;
	int32           _matteVar1;
	bool            _actorUpdateEnabledCheck;
	bool            _matteInitialized;
	bool            _mattePlaySound;
	int32           _currentScreenUpdatesCount;
	int32           _data1[50];
	int32           _data2[11];
	bool            _actorUpdateStatusEnabledCounter;
	bool            _data3[9];
	//bool            _flagEncouter5;
	bool            _flagScene1;
	int32           _movies[49];
	bool            _actorUpdateStatus15Check;
	// Skip opening flag (not used)
	bool            _flag3;
	bool            _flag2;

	ActorDirection  _globalDirection;


	// Screen updates
	bool            _flagRedraw;
	uint32          _nextScreenUpdate;
};

} // End of namespace Asylum

#endif // ASYLUM_DATA_H
