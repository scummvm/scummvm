/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_RESOURCES_DATA_H
#define ASYLUM_RESOURCES_DATA_H

#include "common/rect.h"
#include "common/serializer.h"

#include "engines/asylum/shared.h"

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
	kFlagScene1,
	kFlagSkipScriptProcessing,
	kFlagIsEncounterRunning,
	kFlagEncounterDisablePlayerOnExit,
	kFlagActorUpdateEnabledCheck,
	kFlagActorUpdateStatus15Check
};

class SharedData;

class CrowsData {
public:
	CrowsData(SharedData *sharedData) : /* _sharedData(sharedData), */_data() { memset(_data, 0, sizeof(_data)); }
	int32 &operator [](uint32 index);
private:
	int32 _data[84];
	//SharedData *_sharedData;
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
 *  uint32 {15}    - ambient flags
 *  uint32 {15}    - ambient ticks
 *  uint32 {1}     - UNUSED
 *  uint32 {1}     - UNUSED (scene updateScreen calls count)
 *  -- Script queue (stored in ScriptManager)
 *  uint32 {1}     - global Object X
 *  uint32 {1}     - global Object Y
 *  uint32 {1}     - Skip script processing flag
 *  uint32 {1}     - Encounter running flag
 *  uint32 {1}     - Player ActorIndex
 *  uint32 {1}     - scene xLeft
 *  uint32 {1}     - scene yTop
 *  uint32 {1}     - scene offset
 *  uint32 {1}     - scene offsetAdd
 *  uint32 {1}     - UNUSED
 *  uint32 {13}    - cursor resources
 *  uint32 {3}     - scene fonts (3)
 *  uint32 {5}     - Chapter2 actor data (part 1)
 *  uint32 {1}     - small cursor Up
 *  uint32 {1}     - small cursor Down
 *  uint32 {1}     - Encounter frame background
 *  uint32 {1}     - Flag skip draw scene
 *  uint32 {1}     - matte var 1
 *  uint32 {1}     - actorUpdateEnabledCheck
 *  uint32 {1}     - matte Initialized
 *  uint32 {1}     - matte playSound
 *  uint32 {1}     - currentScreenUpdatesCount
 *  uint32 {9}     - Chapter2 actor data (part 2)
 *  uint32 {1}     - Chapter 2 counter 1
 *  uint32 {1}     - Chapter 2 counter 2
 *  uint32 {1}     - Chapter 2 counter 3
 *  uint32 {1}     - Chapter 2 counter 4
 *  uint32 {23}    - Chapter2 actor data (part 3)
 *  uint32 {1}     - Special 2 counter 5
 *  uint32 {1}     - Chapter 2 frameIndex Offset
 *  uint32 {1}     - Chapter 2 Actor index
 *  uint32 {1}     - Event update flag
 *  uint32 {1}     - Chapter 2 counter 6
 *  uint32 {1}     - Chapter 2 counter 7
 *  uint32 {1}     - Chapter 2 counter 8
 *  uint32 {18}    - Chapter2 actor data (part 4)
 *  uint32 {1}     - actorUpdateStatusEnabledCounter
 *  uint32 {9}     - Chapter2 actor data (part 5)
 *  uint32 {1}     - Encounter disablePlayerOnExit
 *  uint32 {1}     - Scene flag 1
 *  uint32 {1}     - nextScreenUpdate
 *  uint32 {49}    - Viewed movies
 *  uint32 {1}     - actorUpdateStatus15Check
 *  -- Skip opening movie command line flag (not used)
 *  -- Encounter flag 3
 *  uint32 {1}     - Flag 2
 *  uint32 {1}     - Flag Redraw
 *  -- scripts (reset on scene load)
 *  -- polygons (reset on scene load)
 */
class SharedData : public Common::Serializable {
public:
	SharedData();
	virtual ~SharedData() {};

	// Public variables
	int32           cdNumber;
	uint32          movieIndex;
	uint32          sceneCounter;
	Common::Point   vector1;
	Common::Point   vector2;
	bool            actorEnableForStatus7;
	ActorDirection  globalDirection;
	CrowsData       crowsData;

	// Used by Actor::enableActorsChapter2 (and maybe others)
	void resetChapter2Data();
	void reset();  // called during game reset

	// Flags
	bool getFlag(GlobalFlag flag) const;
	void setFlag(GlobalFlag flag, bool state);

	// Serializer
	void saveLoadAmbientSoundData(Common::Serializer &s);
	void saveLoadWithSerializer(Common::Serializer &s);

	//////////////////////////////////////////////////////////////////////////
	// Accessors
	//////////////////////////////////////////////////////////////////////////

	// Ambient sound data
	uint32 getAmbientTick(uint32 index) const;
	void setAmbientTick(uint32 index, uint32 val);
	uint32 getAmbientFlag(uint32 index) const;
	void setAmbientFlag(uint32 index, uint32 val);
	void resetAmbientFlags();

	/**
	 * Return the index of the player actor
	 */
	ActorIndex getPlayerIndex() { return _playerIndex; }

	/**
	 * Sets the player actor index.
	 *
	 * @param index index of the player actor
	 */
	void setPlayerIndex(ActorIndex index) { _playerIndex = index; }

	// Coordinates
	Common::Point getGlobalPoint() { return _globalPoint; }
	void setGlobalPoint(const Common::Point &point) { _globalPoint = point; }
	Common::Point getSceneCoords() { return _sceneCoords; }
	void setSceneCoords(const Common::Point &point) { _sceneCoords = point; }
	int16 getSceneOffset() { return _sceneOffset; }
	void setSceneOffset(int16 sceneOffset) { _sceneOffset = sceneOffset; }
	int16 getSceneOffsetAdd() { return _sceneOffsetAdd; }
	void setSceneOffsetAdd(int16 sceneOffsetAdd) { _sceneOffsetAdd = sceneOffsetAdd; }

	// Saved scene data
	void saveCursorResources(ResourceId *resources, uint32 size);
	void loadCursorResources(ResourceId *resources, uint32 size) const;
	void saveSceneFonts(ResourceId font1, ResourceId font2, ResourceId font3);
	void loadSceneFonts(ResourceId *font1, ResourceId *font2, ResourceId *font3) const;
	void saveSmallCursor(int32 smallCurUp, int32 smallCurDown);
	void loadSmallCursor(int32 *smallCurUp, int32 *smallCurDown) const;
	void saveEncounterFrameBackground(ResourceId encounterFrameBg) { _encounterFrameBg = encounterFrameBg; }
	void loadEncounterFrameBackground(ResourceId *encounterFrameBg) { *encounterFrameBg = _encounterFrameBg; }

	// Matte data
	int32 getMatteVar1() const { return _matteVar1; }
	void setMatteVar1(int32 val) { _matteVar1 = val; }
	uint32 getMatteVar2() const { return _matteVar2; }
	void setMatteVar2(uint32 val) { _matteVar2 = val; }
	int16 getMatteBarHeight() const { return _matteBarHeight; }
	void setMatteBarHeight(int16 val) { _matteBarHeight = val; }
	bool getMatteInitialized() const { return _matteInitialized; }
	void setMatteInitialized(bool val) { _matteInitialized = val; }
	bool getMattePlaySound() const { return _mattePlaySound; }
	void setMattePlaySound(bool val) { _mattePlaySound = val; }

	// Chapter 2 data
	void setChapter2Counter(uint32 index, int32 val);
	int32 getChapter2Counter(uint32 index) const;
	int32 getChapter2FrameIndexOffset() const { return _chapter2FrameIndexOffset; }
	void setChapter2FrameIndexOffset(int32 val) { _chapter2FrameIndexOffset = val; }
	ActorIndex getChapter2ActorIndex() const { return _chapter2ActorIndex; }
	void setChapter2ActorIndex(ActorIndex val) { _chapter2ActorIndex = val; }

	// Misc
	int32 getActorUpdateStatusEnabledCounter() { return _actorUpdateStatusEnabledCounter; }
	void setActorUpdateStatusEnabledCounter(int32 val) { _actorUpdateStatusEnabledCounter = val; }

	// Screen updates
	int32 getEventUpdate() { return _eventUpdate; }
	void setEventUpdate(int32 val) { _eventUpdate = val; }
	uint32 getNextScreenUpdate() { return _nextScreenUpdate; }
	void setNextScreenUpdate(uint32 nextScreenUpdate) { _nextScreenUpdate = nextScreenUpdate; }

private:
	uint32          _ambientFlags[15];
	uint32          _ambientTicks[15];
	Common::Point   _globalPoint; // global point
	bool            _flagSkipScriptProcessing;
	bool            _flagIsEncounterRunning;
	int32           _playerIndex;
	Common::Point   _sceneCoords;
	int16           _sceneOffset;
	int16           _sceneOffsetAdd;
	ResourceId      _cursorResources[13];
	ResourceId      _sceneFonts[3];
	uint32          _chapter2Data1[5];
	int32           _smallCurUp;
	int32           _smallCurDown;
	ResourceId      _encounterFrameBg;
	bool            _flagSkipDrawScene;
	int32           _matteVar1;
	bool            _flagActorUpdateEnabledCheck;
	bool            _matteInitialized;
	bool            _mattePlaySound;
	int32           _currentScreenUpdatesCount;
	uint32          _chapter2Data2[9];
	int32           _chapter2Counters[8];
	int32           _chapter2Data3[23];
	int32           _chapter2FrameIndexOffset;
	ActorIndex      _chapter2ActorIndex;
	int32           _eventUpdate;
	uint32          _chapter2Data4[18];
	int32           _actorUpdateStatusEnabledCounter;
	uint32          _chapter2Data5[9];
	bool            _flagEncounterDisablePlayerOnExit;
	bool            _flag1;
	uint32          _nextScreenUpdate;
	//byte            _moviesViewed[196];
	bool            _flagActorUpdateStatus15Check;

	// Non-saved data
	bool            _flag2;
	bool            _flag3;
	bool            _flagScene1;
	bool            _flagRedraw;

	int16           _matteBarHeight;
	uint32          _matteVar2;

	friend class CrowsData;
};

} // End of namespace Asylum

#endif // ASYLUM_RESOURCES_DATA_H
