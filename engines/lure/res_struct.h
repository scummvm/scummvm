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

#ifndef LURE_RESSTRUCT_H
#define LURE_RESSTRUCT_H

#include "lure/luredefs.h"
#include "common/list.h"
#include "common/file.h"

namespace Lure {

using namespace Common;

extern const Action sortedActions[];

/*-------------------------------------------------------------------------*/
/* Structure definitions                                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include "common/pack-start.h"	// START STRUCT PACKING

struct VersionStructure {
	uint16 id;
	byte vMajor;
	byte vMinor;
};

struct FileEntry {
	uint16 id;
	byte unused;
	byte sizeExtension;
	uint16 size;
	uint16 offset;
};

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
	uint16 startX;
	uint16 startY;
	uint16 width;
	uint16 height;
	uint16 widthCopy;
	uint16 heightCopy;
	uint16 yCorrection;
	int16 walkX;
	uint16 walkY;
	int8 talkX;
	int8 talkY;
	uint16 colourOffset;
	uint16 animRecordId;
	uint16 hotspotScriptOffset;
	uint16 talkScriptOffset;
	uint16 tickProcOffset;
	uint16 tickTimeout;
	uint16 tickScriptOffset;
	uint16 npcSchedule;
	uint16 characterMode;
	uint16 delayCtr;
	byte flags2;
	byte hdrFlags;
};

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
};

struct MovementResource {
	uint16 frameNumber;
	int16 xChange;
	int16 yChange;
};

struct RoomRect {
	int16 xs, xe;
	int16 ys, ye;
};

struct RoomResource {
	uint16 roomNumber;
	uint8 hdrFlags;
	uint8 unused;
	uint32 actions;
	uint16 descId;
	uint16 numLayers;
	uint16 layers[4];
	uint16 sequenceOffset;
	int16 clippingXStart;
	int16 clippingXEnd;
	RoomRect walkBounds;
	uint16 numExits;
};

struct RoomExitResource {
	int16 xs, xe, ys, ye;
	uint16 sequenceOffset;
	uint8 newRoom;
	uint8 direction;
	int16 newRoomX, newRoomY;
};

struct HotspotOverrideResource {
	uint16 hotspotId;
	int16 xs, xe, ys, ye;
};

struct RoomExitHotspotResource {
	uint16 hotspotId;
	int16 xs, xe;
	int16 ys, ye;
	uint16 cursorNum;
	uint16 destRoomNumber;
};

struct RoomExitJoinResource {
	uint16 hotspot1Id;
	byte h1CurrentFrame;
	byte h1DestFrame;
	uint8 h1OpenSound;
	uint8 h1CloseSound;
	uint16 hotspot2Id;
	byte h2CurrentFrame;
	byte h2DestFrame;
	uint8 h2OpenSound;
	uint8 h2CloseSound;
	byte blocked;
};

struct HotspotActionResource {
	byte action;
	uint16 sequenceOffset;
};

struct TalkHeaderResource {
	uint16 hotspotId;
	uint16 offset;
};

struct TalkDataHeaderResource {
	uint16 recordId;
	uint16 listOffset;
	uint16 responsesOffset;
};

struct TalkDataResource {
	uint16 preSequenceId;
	uint16 descId;
	uint16 postSequenceId;
};

struct TalkResponseResource {
	uint16 sequenceId1;
	uint16 sequenceId2;
	uint16 sequenceId3;
};

struct RoomExitCoordinateResource {
	int16 x;
	int16 y;
	uint16 roomNumber;
};

#define ROOM_EXIT_COORDINATES_NUM_ENTRIES 6
#define ROOM_EXIT_COORDINATES_NUM_ROOMS 52

struct RoomExitCoordinateEntryResource {
	RoomExitCoordinateResource entries[ROOM_EXIT_COORDINATES_NUM_ENTRIES];
	uint8 roomIndex[ROOM_EXIT_COORDINATES_NUM_ROOMS];
};

#define MAX_SCHEDULE_ENTRY_PARAMS 5

struct CharacterScheduleResource {
	uint16 action;
	uint16 params[MAX_SCHEDULE_ENTRY_PARAMS];
};

struct RoomExitIndexedHotspotResource {
	uint8 roomNumber;
	uint8 hotspotIndex;
	uint16 hotspotId;
};

#include "common/pack-end.h"	// END STRUCT PACKING

/**
 * Class template for a derived list that destroys the contained
 * object when the record containing it is destroyed. It's not
 * perfect, since the underlying list doesn't have virtual
 * methods, but it's sufficient for my usage.
 */
template <class T>
class ManagedList: public Common::List<T> {
	typedef typename Common::List<T> Common_List;
public:
	~ManagedList() {
		clear();
	}

	void clear() {
		typename Common::List<T>::iterator i;
		for (i = Common_List::begin(); i != Common_List::end(); ++i)
			delete *i;
		Common_List::clear();		
	}

	typename Common::List<T>::iterator erase(typename Common::List<T>::iterator pos) {
		delete *pos;
		return Common_List::erase(pos);
	}

	typename Common::List<T>::iterator erase(typename Common::List<T>::iterator first, 
			typename Common::List<T>::iterator last) {
		typename Common::List<T>::iterator i;
		for (i = first; i != last; ++i)
			delete *i;
		return Common_List::erase(first, last);
	}
};

/** Enumeration used for direction facings */
enum Direction {UP, DOWN, LEFT, RIGHT, NO_DIRECTION};

// Support classes to hold loaded resources

class RoomExitHotspotData {
public:
	RoomExitHotspotData(RoomExitHotspotResource *rec);

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

#define ROOM_PATHS_WIDTH 40
#define ROOM_PATHS_HEIGHT 24
#define ROOM_PATHS_SIZE (ROOM_PATHS_WIDTH / 8 * ROOM_PATHS_HEIGHT)
#define DECODED_PATHS_WIDTH 42
#define DECODED_PATHS_HEIGHT 26

typedef uint16 RoomPathsDecompressedData[DECODED_PATHS_WIDTH * DECODED_PATHS_HEIGHT];

class RoomPathsData {
private:
	byte _data[ROOM_PATHS_HEIGHT * ROOM_PATHS_WIDTH];
public:
	RoomPathsData() {}
	RoomPathsData(byte *srcData) { load(srcData); }

	void load(byte *srcData) {
		memcpy(_data, srcData, ROOM_PATHS_SIZE);
	}
	bool isOccupied(int x, int y);
	bool isOccupied(int x, int y, int width);
	void setOccupied(int x, int y, int width);
	void clearOccupied(int x, int y, int width);
	void decompress(RoomPathsDecompressedData &dataOut, int characterWidth);
};

#define MAX_NUM_LAYERS 4

class RoomData {
public:
	RoomData(RoomResource *rec, MemoryBlock *pathData);

	uint16 roomNumber;
	uint8 hdrFlags;
	uint8 flags;
	uint32 actions;
	uint16 descId;
	uint16 numLayers;
	uint16 layers[MAX_NUM_LAYERS];
	uint16 sequenceOffset;
	int16 clippingXStart;
	int16 clippingXEnd;
	Common::Rect walkBounds;
	RoomExitHotspotList exitHotspots;
	RoomExitList exits;
	RoomPathsData paths;
};

typedef ManagedList<RoomData *> RoomDataList;

class RoomExitJoinData {
public:
	RoomExitJoinData(RoomExitJoinResource *rec);

	uint16 hotspot1Id;
	byte h1CurrentFrame;
	byte h1DestFrame;
	uint8 h1OpenSound;
	uint8 h1CloseSound;
	uint16 hotspot2Id;
	byte h2CurrentFrame;
	byte h2DestFrame;
	uint8 h2OpenSound;
	uint8 h2CloseSound;
	byte blocked;
	uint32 unknown;
};

class RoomExitJoinList: public ManagedList<RoomExitJoinData *> {
public:
	void saveToStream(WriteStream *stream);
	void loadFromStream(ReadStream *stream);
};

class HotspotActionData {
public:
	HotspotActionData(HotspotActionResource *rec);

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

enum CharacterMode {CHARMODE_NONE, CHARMODE_HESITATE, CHARMODE_IDLE, CHARMODE_PAUSED,
	CHARMODE_WAIT_FOR_PLAYER, CHARMODE_CONVERSING, CHARMODE_PLAYER_WAIT, 
	CHARMODE_WAIT_FOR_INTERACT, CHARMODE_INTERACTING, CHARMODE_SPECIAL_PLAYER};

enum BlockedState {BS_NONE, BS_INITIAL, BS_FINAL};

enum VariantBool {VB_INITIAL, VB_FALSE, VB_TRUE};

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
	uint16 widthCopy;
	uint16 heightCopy;
	uint16 yCorrection;
	int16 walkX;
	uint16 walkY;
	int8 talkX;
	int8 talkY;
	uint16 colourOffset;
	uint16 animRecordId;
	uint16 hotspotScriptOffset;
	uint16 talkScriptOffset;
	uint16 tickProcOffset;
	uint16 tickTimeout;
	uint16 tickScriptOffset;
	uint16 npcSchedule;
	CharacterMode characterMode;
	uint16 delayCtr;
	uint8 flags2;
	uint8 headerFlags;

	// Runtime fields
	uint16 actionCtr;
	BlockedState blockedState;
	bool blockedFlag;
	VariantBool coveredFlag;
	uint16 talkMessageId;
	uint16 talkDestCharacterId;
	uint16 talkCountdown;
	uint16 pauseCtr;
	uint16 useHotspotId;	
	uint16 talkGate;
	uint16 actionHotspotId;
	uint16 talkOverride;

	uint16 use2HotspotId;

	void enable() { flags |= 0x80; }
	void disable() { flags &= 0x7F; }
	Direction nonVisualDirection() { return (Direction) scriptLoadFlag; }
	void saveToStream(WriteStream *stream);
	void loadFromStream(ReadStream *stream);
};

class HotspotDataList: public ManagedList<HotspotData *> {
public:
	void saveToStream(WriteStream *stream);
	void loadFromStream(ReadStream *stream);
};

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

// Talk header list

class TalkHeaderData {
private:
	uint16 *_data;
	int _numEntries;
public:
	TalkHeaderData(uint16 charId, uint16 *entries);
	~TalkHeaderData();

	uint16 characterId;
	uint16 getEntry(int index);
};

typedef ManagedList<TalkHeaderData *> TalkHeaderList;

class TalkEntryData {
public:
	TalkEntryData(TalkDataResource *rec);

	uint16 preSequenceId;
	uint16 descId;
	uint16 postSequenceId;
};

typedef ManagedList<TalkEntryData *> TalkEntryList;

class TalkData {
public:
	TalkData(uint16 id);
	~TalkData();

	uint16 recordId;
	TalkEntryList entries;
	TalkEntryList responses;

	TalkEntryData *getResponse(int index);
};

typedef ManagedList<TalkData *> TalkDataList;

struct RoomExitCoordinateData {
	int16 x;
	int16 y;
	uint16 roomNumber;
	byte hotspotIndexId;
};

class RoomExitCoordinates {
private:
	RoomExitCoordinateData _entries[ROOM_EXIT_COORDINATES_NUM_ENTRIES];
	uint8 _roomIndex[ROOM_EXIT_COORDINATES_NUM_ROOMS];
public:
	RoomExitCoordinates(RoomExitCoordinateEntryResource *rec);
	RoomExitCoordinateData &getData(uint16 destRoomNumber);
};

class RoomExitCoordinatesList: public ManagedList<RoomExitCoordinates *> {
public:
	RoomExitCoordinates &getEntry(uint16 roomNumber);
};

class RoomExitIndexedHotspotData {
public:
	RoomExitIndexedHotspotData(RoomExitIndexedHotspotResource *rec);

	uint16 roomNumber;
	uint8 hotspotIndex;
	uint16 hotspotId;
};

class RoomExitIndexedHotspotList: public ManagedList<RoomExitIndexedHotspotData *> {
public:
	uint16 getHotspot(uint16 roomNumber, uint8 hotspotIndexId);
};

// The following classes hold any sequence offsets that are being delayed

class SequenceDelayData {
public:
	SequenceDelayData(uint16 delay, uint16 seqOffset, bool canClearFlag);

	uint32 timeoutCtr;
	uint16 sequenceOffset;
	bool canClear;
};

class SequenceDelayList: public ManagedList<SequenceDelayData *> {
public:
	void add(uint16 delay, uint16 seqOffset, bool canClear);
	void tick();
	void clear();
};

// The following classes holds the data for NPC schedules

extern int actionNumParams[NPC_JUMP_ADDRESS+1];

class CharacterScheduleSet;

class CharacterScheduleEntry {
private:
	CharacterScheduleSet *_parent;
	Action _action;
	uint16 _params[MAX_TELL_COMMANDS * 3];
	int _numParams;
public:
	CharacterScheduleEntry() { _action = NONE; _parent = NULL; }
	CharacterScheduleEntry(Action theAction, ...);
	CharacterScheduleEntry(CharacterScheduleSet *parentSet, 
		CharacterScheduleResource *&rec);

	Action action() { return _action; }
	int numParams() { return _numParams; }
	uint16 param(int index);
	void setDetails(Action theAction, ...);
	void setDetails2(Action theAction, int numParamEntries, uint16 *paramList);
	CharacterScheduleEntry *next();
	CharacterScheduleSet *parent() { return _parent; }
	uint16 id();
};

class CharacterScheduleSet: public ManagedList<CharacterScheduleEntry *> {
private:
	uint16 _id;
public:
	CharacterScheduleSet(CharacterScheduleResource *rec, uint16 setId); 
	uint16 getId(CharacterScheduleEntry *rec);
	uint16 id() { return _id; }
};

class CharacterScheduleList: public ManagedList<CharacterScheduleSet *> {
public:
	CharacterScheduleEntry *getEntry(uint16 id, CharacterScheduleSet *currentSet = NULL);
};

typedef List<uint16> CharacterScheduleOffsets;

// The follow classes are used to store the NPC schedule Ids for the random actions a follower can do in each room

enum RandomActionType {REPEATABLE, REPEAT_ONCE, REPEAT_ONCE_DONE};

class RandomActionSet {
private:
	uint16 _roomNumber;
	int _numActions;
	RandomActionType *_types;
	uint16 *_ids;
public:
	RandomActionSet(uint16 *&offset);
	~RandomActionSet();

	uint16 roomNumber() { return _roomNumber; }
	int numActions() { return _numActions; }
	void getEntry(int index, RandomActionType &actionType, uint16 &id) {
		assert((index >= 0) && (index < _numActions));
		actionType = _types[index];
		id = _ids[index];
	}
	void setDone(int index) {
		assert((index >= 0) && (index < _numActions));
		assert(_types[index] == REPEAT_ONCE);
		_types[index] = REPEAT_ONCE_DONE;
	}
	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
};

class RandomActionList: public ManagedList<RandomActionSet *> {
public:
	RandomActionSet *getRoom(uint16 roomNumber);
	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
};

class PausedCharacter {
public:
	PausedCharacter(uint16 SrcCharId, uint16 DestCharId);

	uint16 srcCharId;
	uint16 destCharId;
	uint16 counter;
	HotspotData *charHotspot;
};

class Hotspot;

class PausedCharacterList: public ManagedList<PausedCharacter *> {
public:
	void reset(uint16 hotspotId);
	void countdown();
	void scan(Hotspot &h);
	int check(uint16 charId, int numImpinging, uint16 *impingingList);
};

struct ServeEntry {
	uint16 hotspotId;
	uint8 serveFlags;
};

#define NUM_SERVE_CUSTOMERS 4

enum BarmanGraphicType {BG_RANDOM = 0, BG_BEER = 1, BG_EXTRA1 = 2, BG_EXTRA2 = 3};

struct BarEntry {
	uint16 roomNumber;
	uint16 barmanId;
	ServeEntry customers[NUM_SERVE_CUSTOMERS];
	uint16 *graphics[4];
	uint16 gridLine;
	ServeEntry *currentCustomer;
};

class BarmanLists {
public:
	BarEntry &getDetails(uint16 roomNumber);
	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
};

enum BarmanAction {WALK_AROUND = 1, POLISH_BAR = 2, WAIT = 3, WAIT_DIALOG = 4, SERVE_BEER = 5};

enum StringEnum {S_CREDITS = 25, S_RESTART_GAME = 26, S_SAVE_GAME = 27, S_RESTORE_GAME = 28, 
	S_QUIT = 29, S_FAST_TEXT = 30, S_SLOW_TEXT = 31, S_SOUND_ON = 32, S_SOUND_OFF = 33, 
	S_NOTHING = 34, S_FOR = 35, S_TO = 36, S_ON = 37, S_AND_THEN = 38,
	S_FINISH = 39};

class StringList {
private:
	MemoryBlock *_data;
	int _numEntries;
	char **_entries;
public:
	StringList() { _numEntries = 0; }
	~StringList() { clear(); }

	void load(MemoryBlock *data);
	void clear();
	int count() { return _numEntries; }
	const char *getString(int index) {
		if ((index < 0) || (index >= _numEntries)) error("Invalid index specified to String List");
		return _entries[index];
	}
	const char *getString(Action action) { return getString((int) action - 1); }
	const char *getString(StringEnum sEnum) { return getString((int) sEnum); }
};

// The following class holds the field list used by the script engine as 
// well as miscellaneous fields used by the game.                          

#define NUM_VALUE_FIELDS 90

enum FieldName {
	ROOM_NUMBER = 0, 
	CHARACTER_HOTSPOT_ID = 1, 
	USE_HOTSPOT_ID = 2,
	ACTIVE_HOTSPOT_ID = 3,
	SEQUENCE_RESULT = 4,
	GENERAL = 5,
	GIVE_TALK_INDEX = 6,
	NEW_ROOM_NUMBER = 7,
	OLD_ROOM_NUMBER = 8,
	CELL_DOOR_STATE = 9,
	TORCH_HIDE = 10,
	PRISONER_DEAD = 15,
	BOTTLE_FILLED = 18,
	TALK_INDEX = 19,
	SACK_CUT = 20,
	ROOM_EXIT_ANIMATION = 76
};

enum GameFlags {
	GAMEFLAG_1 = 1,
	GAMEFLAG_2 = 2,
	GAMEFLAG_4 = 4,
	GAMEFLAG_8 = 8,
	GAMEFLAG_10 = 0x10,
	GAMEFLAG_20 = 0x20,
	GAMEFLAG_40 = 0x40,
	GAMEFLAG_FAST_TEXTSPEED = 0x80
};

struct PlayerNewPosition {
	Point position;
	uint16 roomNumber;
};

struct PlayerPendingPosition {
	Point pos;
	bool isSet;
};

class ValueTableData {
private:
	uint16 _numGroats;
	PlayerNewPosition _playerNewPos;
	PlayerPendingPosition _playerPendingPos;
	uint8 _flags;
	uint8 _hdrFlagMask;

	uint16 _fieldList[NUM_VALUE_FIELDS];
	bool isKnownField(uint16 fieldIndex);
public:
	ValueTableData();
	uint16 getField(uint16 fieldIndex);
	uint16 getField(FieldName fieldName);

	void setField(uint16 fieldIndex, uint16 value);
	void setField(FieldName fieldName, uint16 value);
	int size() { return NUM_VALUE_FIELDS; }

	uint16 &numGroats() { return _numGroats; }
	uint8 &flags() { return _flags; }
	uint8 &hdrFlagMask() { return _hdrFlagMask; }
	PlayerNewPosition &playerNewPos() { return _playerNewPos; }
	PlayerPendingPosition &playerPendingPos() { return _playerPendingPos; }

	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
};

} // End of namespace Lure

#endif
