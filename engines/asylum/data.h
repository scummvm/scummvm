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
	kFlagSkipDraw,
	kFlagSceneRectChanged,
	kFlagScene1,
	kFlagEncounter2,
	kFlagEncounter3,
	kFlagEncounter5
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
 *  -- Script queue (stored in ActionList - reset on scene change)
 *  uint32 {1}      - global Object X
 *  uint32 {1}      - global Object Y
 *  -- Skip processing flag (stored in ActionList)
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
		_globalX = 0;
		_globalY = 0;
		_flagEncouter2 = false;
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
		_flagEncouter5 = false;
		_flagScene1 = false;
		_nextScreenUpdate = 0;
		memset(&_movies[49], 0, sizeof(_movies));
		_actorUpdateStatus15Check = false;
		_flagEncouter3 = false;
		_flag2 = false;
		_flag5 = false;
	}
	// Accessors
	int32 getActorUpdateEnabledCounter() { return _actorUpdateStatusEnabledCounter; }
	void  setActorUpdateEnabledCounter(int32 val) { _actorUpdateStatusEnabledCounter = val; }

	int32 *nextScreenUpdate() { return &_nextScreenUpdate; }

	bool getActorEnableForStatus7() { return _actorEnableForStatus7; }
	void setActorEnableForStatus7(bool state) { _actorEnableForStatus7 = state; }

	bool getActorUpdateEnabledCheck() { return _actorUpdateEnabledCheck; }

	void setActorUpdateFlag(int32 val) { _data1[40] = 2; }

	int32 getUpdateCounter() { return _actorUpdateCounter; }
	void  setUpdateCounter(int32 val) { _actorUpdateCounter = val; }

	int32 getActorUpdateFlag2() { return _data1[36]; }
	void setActorUpdateFlag2(int32 val) { _data1[36] = val; }

	bool getSkipDrawScene() { return _flagSkipDrawScene; }
	void setSkipDrawScene(bool skip) { _flagSkipDrawScene = skip; }

	// Shared global Data
	int32 getGlobalX() const   { return _globalX; }
	void setGlobalX(int32 val) { _globalX = val; }
	int32 getGlobalY() const   { return _globalY; }
	void setGlobalY(int32 val) { _globalY = val; }

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

	Common::Rect *getActorRect() { return &_actorRect; }

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

		case kFlagSkipDraw:
			return _flagSkipDrawScene;

		case kFlagSceneRectChanged:
			error("[SharedData::getFlag] Invalid flag type (kFlagSceneRectChanged)!");

		case kFlagScene1:
			return _flagScene1;

		case kFlagEncounter2:
			return _flagEncouter2;

		case kFlagEncounter3:
			return _flagEncouter3;

		case kFlagEncounter5:
			return _flagEncouter5;
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

		case kFlagSkipDraw:
			_flagSkipDrawScene = state;
			break;

		case kFlagSceneRectChanged:
			error("[SharedData::getFlag] Invalid flag type (kFlagSceneRectChanged)!");
			break;

		case kFlagScene1:
			_flagScene1 = state;
			break;

		case kFlagEncounter2:
			_flagEncouter2 = state;
			break;

		case kFlagEncounter3:
			_flagEncouter3 = state;
			break;

		case kFlagEncounter5:
			_flagEncouter5 = state;
			break;
		}
	}

private:
	Common::Rect _actorRect;

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
	int32            _globalX;
	int32            _globalY;
	bool             _flagEncouter2;
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
	int32           _data3[9];
	bool            _flagEncouter5;
	bool            _flagScene1;
	int32           _nextScreenUpdate;
	int32           _movies[49];
	bool            _actorUpdateStatus15Check;
	// Skip opening flag (not used)
	bool            _flagEncouter3;
	bool            _flag2;
	bool            _flag5;
};

} // End of namespace Asylum

#endif // ASYLUM_DATA_H
