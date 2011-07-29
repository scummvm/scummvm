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

#ifndef ASYLUM_DATA_H
#define ASYLUM_DATA_H

#include "engines/asylum/shared.h"

#include "common/rect.h"
#include "common/serializer.h"

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
 *  uint32 {15}    - ambient flags
 *  uint32 {15}    - ambient ticks
 *  uint32 {1}     - UNUSED
 *  uint32 {1}     - scene updateScreen calls count
 *  -- Script queue (stored in ScriptManager - reset on scene change)
 *  uint32 {1}     - global Object X
 *  uint32 {1}     - global Object Y
 *  -- Skip processing flag (stored in ScriptManager)
 *  -- Encounter running flag (stored in Encounter)
 *  uint32 {1}     - Player ActorIndex
 *  uint32 {1}     - scene xLeft
 *  uint32 {1}     - scene yTop
 *  uint32 {1}     - scene offset
 *  uint32 {1}     - scene offsetAdd
 *  uint32 {1}     - UNUSED
 *  uint32 {13}    - cursor resources
 *  uint32 {3}     - scene fonts (3)
 *  uint32 {3}     - Chapter2 actor data (part 1)
 *  uint32 {1}     - UNUSED
 *  uint32 {1}     - small cursor Up
 *  uint32 {1}     - small cursor Down
 *  uint32 {1}     - Encounter frame background
 *  uint32 {1}     - Flag skip draw scene
 *  uint32 {1}     - matte var 1
 *  uint32 {1}     - actorUpdateEnabledCheck
 *  uint32 {1}     - matte Initialized
 *  uint32 {1}     - matte playSound
 *  uint32 {1}     - currentScreenUpdatesCount
 *  uint32 {3}     - Chapter2 actor data (part 2)
 *  uint32 {1}     - Chapter 2 counter 1
 *  uint32 {1}     - Chapter 2 counter 2
 *  uint32 {1}     - Chapter 2 counter 3
 *  uint32 {1}     - Chapter 2 counter 4
 *  uint32 {1}     - UNUSED
 *  uint32 {3}     - Chapter2 actor data (part 3)
 *  uint32 {1}     - UNUSED
 *  uint32 {8}     - Chapter 2 points
 *  uint32 {9}     - UNUSED
 *  uint32 {1}     - Special 2 counter 5
 *  uint32 {1}     - Chapter 2 frameIndex Offset
 *  uint32 {1}     - Chapter 2 Actor index
 *  uint32 {1}     - Event update flag
 *  uint32 {1}     - Chapter 2 counter 6
 *  uint32 {1}     - Chapter 2 counter 7
 *  uint32 {1}     - Chapter 2 counter 8
 *  uint32 {7}     - UNUSED
 *  uint32 {3}     - Chapter2 actor data (part 4)
 *  uint32 {8}     - UNUSED
 *  uint32 {1}     - actorUpdateStatusEnabledCounter
 *  uint32 {9}     - Chapter2 actor data (part 5)
 *  uint32 {1}     - Encounter disablePlayerOnExit
 *  uint32 {1}     - Scene flag 1
 *  uint32 {1}     - nextScreenUpdate
 *  uint32 {49}    - Viewed movies
 *  uint32 {1}     - actorUpdateStatus15Check
 *  -- Skip opening movie command line flag (not used)
 *  uint32 {1}     - Encounter flag 3
 *  uint32 {1}     - Flag 2
 *  uint32 {1}     - Flag 5
 *  -- scripts (reset on scene load)
 *  -- polygons (reset on scene load)
 */
class SharedData : public Common::Serializable {
public:
	SharedData();

	// Public variables
	int32 cdNumber;
	int32 movieIndex;

	// Saved scene data
	ResourceId      cursorResources[13];
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

	// TODO Remove and replace by accessors
	void setData(ActorIndex index, int32 val);
	int32 getData(ActorIndex index);
	void setData2(ActorIndex index, bool val);
	bool getData2(ActorIndex index);

	// Used by Actor::enableActorsChapter2 (and maybe others)
	void resetActorData();

	// Ambient sound data
	uint32 getAmbientTick(uint32 index);
	void setAmbientTick(uint32 index, uint32 val);
	uint32 getAmbientFlag(uint32 index);
	void setAmbientFlag(uint32 index, uint32 val);
	void resetAmbientFlags();

	// Flags
	bool getFlag(GlobalFlag flag) const;
	void setFlag(GlobalFlag flag, bool state);

	// Serializer
	void saveLoadAmbientSoundData(Common::Serializer &s);
	void saveLoadWithSerializer(Common::Serializer &s);

private:
	uint32          _ambientFlags[15];
	uint32          _ambientTicks[15];

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
