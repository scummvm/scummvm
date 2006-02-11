/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#ifndef __lure_resstruct_h__
#define __lure_resstruct_h__

#include "lure/luredefs.h"
#include "common/list.h"

namespace Lure {

extern const char *actionList[];

/*-------------------------------------------------------------------------*/
/* Structure definitions                                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct HotspotResource {
	uint16 hotspotId;
	uint16 nameId;
	uint16 descId;
	uint16 descId2;
	uint32 actions;
	uint16 actionsOffset;
	uint16 roomNumber;
	byte layer;
	byte scriptLoadFlag;
	uint16 loadOffset;
	int16 startX;
	int16 startY;
	uint16 width;
	uint16 height;
	uint16 colourOffset;
	uint16 animRecordId;
	uint16 sequenceOffset;
	uint16 tickProcOffset;
	uint16 tickTimeout;
} GCC_PACK;

struct HotspotAnimResource {
	uint16 animRecordId;
	uint16 animId;
	uint16 flags;
	uint16 upOffset;
	uint16 downOffset;
	uint16 leftOffset;
	uint16 rightOffset;
	uint8 upFrame;
	uint8 downFrame;
	uint8 leftFrame;
	uint8 rightFrame;
} GCC_PACK;

struct MovementResource {
	uint16 frameNumber;
	int16 xChange;
	int16 yChange;
} GCC_PACK;


struct RoomResource {
	uint16 roomNumber;
	uint16 descId;
	uint16 numLayers;
	uint16 layers[4];
	uint16 sequenceOffset;
	uint16 numExits;
} GCC_PACK;

struct RoomExitResource {
	int16 xs, xe, ys, ye;
	uint16 sequenceOffset;
	uint8 newRoom;
	uint8 direction;
	int16 newRoomX, newRoomY;
} GCC_PACK;

struct HotspotOverrideResource {
	uint16 hotspotId;
	int16 xs, xe, ys, ye;
} GCC_PACK;

struct RoomExitHotspotRecord {
	uint16 hotspotId;
	int16 xs, xe;
	int16 ys, ye;
	uint16 cursorNum;
	uint16 destRoomNumber;
} GCC_PACK;

struct RoomExitJoinRecord {
	uint16 hotspot1Id;
	byte h1CurrentFrame;
	byte h1DestFrame;
	uint16 h1Unknown;
	uint16 hotspot2Id;
	byte h2CurrentFrame;
	byte h2DestFrame;
	uint16 h2Unknown;
	byte blocked;
	uint32 unknown;
} GCC_PACK;

struct HotspotActionRecord {
	byte action;
	uint16 sequenceOffset;
} GCC_PACK;

struct FileEntry {
	uint16 id;
	byte unused;
	byte sizeExtension;
	uint16 size;
	uint16 offset;
} GCC_PACK;

struct VersionStructure {
	uint16 id;
	byte vMajor;
	byte vMinor;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif

// Class template for a derived list that destroys the contained
// object when the record containing it is destroyed. It's not
// perfect, since the underlying list doesn't have virtual
// methods, but it's sufficient for my usage

template <class T>
class ManagedList: public Common::List<T> {
public:
	~ManagedList() {
		clear();
	}

	void clear() {
		typename Common::List<T>::iterator i;
		for (i = Common::List<T>::begin(); i != Common::List<T>::end(); ++i)
			delete *i;
		Common::List<T>::clear();		
	}

	typename Common::List<T>::iterator erase(typename Common::List<T>::iterator pos) {
		delete *pos;
		return Common::List<T>::erase(pos);
	}

	typename Common::List<T>::iterator erase(typename Common::List<T>::iterator first, 
			typename Common::List<T>::iterator last) {
		typename Common::List<T>::iterator i;
		for (i = first; i != last; ++i)
			delete *i;
		return Common::List<T>::erase(first, last);
	}
};

// Enumeration used for direction facings

enum Direction {UP, DOWN, LEFT, RIGHT, NO_DIRECTION};

// Support classes to hold loaded resources

class RoomExitHotspotData {
public:
	RoomExitHotspotData(RoomExitHotspotRecord *rec);

	uint16 hotspotId;
	int16 xs, xe;
	int16 ys, ye;
	uint16 cursorNum;
	uint16 destRoomNumber;
};

typedef ManagedList<RoomExitHotspotData *> RoomExitHotspotList;

class RoomExitData {
public:
	RoomExitData(RoomExitResource *rec);
	bool insideRect(int16 xp, int16 yp);

	int16 xs, xe, ys, ye;
	uint16 sequenceOffset;
	Direction direction;
	uint8 roomNumber;
	uint16 x, y;
};

class RoomExitList: public ManagedList<RoomExitData *> {
public:
	RoomExitData *checkExits(int16 xp, int16 yp);
};

#define MAX_NUM_LAYERS 4

class RoomData {
public:
	RoomData(RoomResource *rec);

	uint16 roomNumber;
	uint16 descId;
	uint16 numLayers;
	uint16 layers[MAX_NUM_LAYERS];
	uint16 sequenceOffset;
	RoomExitHotspotList exitHotspots;
	RoomExitList exits;
};

typedef ManagedList<RoomData *> RoomDataList;

class RoomExitJoinData {
public:
	RoomExitJoinData(RoomExitJoinRecord *rec);

	uint16 hotspot1Id;
	byte h1CurrentFrame;
	byte h1DestFrame;
	uint16 h1Unknown;
	uint16 hotspot2Id;
	byte h2CurrentFrame;
	byte h2DestFrame;
	uint16 h2Unknown;
	byte blocked;
	uint32 unknown;
};

typedef ManagedList<RoomExitJoinData *> RoomExitJoinList;

class HotspotActionData {
public:
	HotspotActionData(HotspotActionRecord *rec);

	Action action;
	uint16 sequenceOffset;
};

class HotspotActionList: public ManagedList<HotspotActionData *> {
public:
	uint16 recordId;

	HotspotActionList(uint16 id, byte *data);
	uint16 getActionOffset(Action action);
};

class HotspotActionSet: public ManagedList<HotspotActionList *> {
public:
	HotspotActionList *getActions(uint16 recordId);
};

class HotspotData {
public:
	HotspotData(HotspotResource *rec);
	
	uint16 hotspotId;
	uint16 nameId;
	uint16 descId;
	uint16 descId2;
	uint32 actions;
	uint16 actionsOffset;
	byte flags;
	uint16 roomNumber;
	byte layer;
	byte scriptLoadFlag;
	uint16 loadOffset;
	int16 startX;
	int16 startY;
	uint16 width;
	uint16 height;
	uint16 colourOffset;
	uint16 animRecordId;
	uint16 sequenceOffset;
	uint16 tickProcOffset;
	uint16 tickTimeout;
};

typedef ManagedList<HotspotData *> HotspotDataList;

class HotspotOverrideData {
public:
	HotspotOverrideData(HotspotOverrideResource *rec);

	uint16 hotspotId;
	int16 xs, xe, ys, ye;
};

typedef ManagedList<HotspotOverrideData *> HotspotOverrideList;

class MovementData {
public:
	MovementData(MovementResource *);

	uint16 frameNumber;
	int16 xChange;
	int16 yChange;
};

class MovementDataList: public ManagedList<MovementData *> {
public:
	bool getFrame(uint16 currentFrame, int16 &xChange, int16 &yChange, 
		uint16 &nextFrame);
};

class HotspotAnimData {
public:
	HotspotAnimData(HotspotAnimResource *rec);

	uint16 animRecordId;
	uint16 animId;
	uint16 flags;
	uint8 upFrame;
	uint8 downFrame;
	uint8 leftFrame;
	uint8 rightFrame;

	MovementDataList leftFrames, rightFrames;
	MovementDataList upFrames, downFrames;
};

typedef ManagedList<HotspotAnimData *> HotspotAnimList;

// The following classes hold any sequence offsets that are being delayed

class SequenceDelayData {
	friend class SequenceDelayList;
private:
	uint32 _timeoutCtr;
	uint16 _sequenceOffset;
public:
	SequenceDelayData(uint16 delay, uint16 seqOffset);
};

class SequenceDelayList: public ManagedList<SequenceDelayData *> {
public:
	void addSequence(uint16 delay, uint16 seqOffset);
	void tick();
};

// The following class holds the field list used by the script engine as 
// well as miscellaneous fields used by the game.                          

#define NUM_VALUE_FIELDS 85

enum FieldName {
	ROOM_NUMBER = 0, 
	CHARACTER_HOTSPOT_ID = 1, 
	USE_HOTSPOT_ID = 2,
	ACTIVE_HOTSPOT_ID = 3,
	SEQUENCE_RESULT = 4,
	GENERAL = 5,
	NEW_ROOM_NUMBER = 7,
	GENERAL_STATUS = 8,
	TORCH_HIDE = 10,
	PRISONER_DEAD = 15,
	BOTTLE_FILLED = 18,
	SACK_CUT = 20
};

class ValueTableData {
private:
	uint16 _numGroats;
	uint16 _fieldList[NUM_VALUE_FIELDS];
	bool isKnownField(uint16 fieldIndex);
public:
	ValueTableData();
	uint16 getField(uint16 fieldIndex);
	uint16 getField(FieldName fieldName);

	void setField(uint16 fieldIndex, uint16 value);
	void setField(FieldName fieldName, uint16 value);
	uint16 &numGroats() { return _numGroats; }
};

} // End of namespace Lure

#endif
