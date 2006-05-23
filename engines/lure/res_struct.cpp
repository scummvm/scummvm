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

#include "lure/res.h"
#include "lure/disk.h"
#include "lure/scripts.h"
#include "lure/system.h"
#include "common/endian.h"

namespace Lure {

const char *actionList[] = {NULL, "Get", NULL, "Push", "Pull", "Operate", "Open",
	"Close", "Lock", "Unlock", "Use", "Give", "Talk to", "Tell", "Buy",
	"Look", "Look at", "Look through", "Ask", NULL, "Drink", "Status",
	"Go to", "Return", "Bribe", "Examine"};

int actionNumParams[NPC_JUMP_ADDRESS+1] = {0, 
	1, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 0, 1,
	0, 1, 1, 1, 1, 0, 0, 2, 0, 1, 0, 0, 1, 1, 2, 2, 5, 2, 2, 1};

// Room data holding class

RoomData::RoomData(RoomResource *rec, MemoryBlock *pathData) { 
	roomNumber = READ_LE_UINT16(&rec->roomNumber);
	descId = READ_LE_UINT16(&rec->descId);
	sequenceOffset = READ_LE_UINT16(&rec->sequenceOffset);
	numLayers = READ_LE_UINT16(&rec->numLayers);

	paths.load(pathData->data() + (roomNumber - 1) * ROOM_PATHS_SIZE);

	for (int ctr = 0; ctr < 4; ++ctr)
		layers[ctr] = READ_LE_UINT16(&rec->layers[ctr]);

	clippingXStart = READ_LE_UINT16(&rec->clippingXStart);
	clippingXEnd = READ_LE_UINT16(&rec->clippingXEnd);
	walkBounds.left = READ_LE_INT16(&rec->walkBounds.xs);
	walkBounds.right = READ_LE_INT16(&rec->walkBounds.xe);
	walkBounds.top = READ_LE_INT16(&rec->walkBounds.ys);
	walkBounds.bottom = READ_LE_INT16(&rec->walkBounds.ye);
}

// Room exit hotspot area holding class

RoomExitHotspotData::RoomExitHotspotData(RoomExitHotspotResource *rec) {
	hotspotId = READ_LE_UINT16(&rec->hotspotId);
	xs = READ_LE_INT16(&rec->xs);
	ys = READ_LE_INT16(&rec->ys);
	xe = READ_LE_INT16(&rec->xe);
	ye = READ_LE_INT16(&rec->ye);
	cursorNum = rec->cursorNum;
	destRoomNumber = READ_LE_UINT16(&rec->destRoomNumber);
}

//  Room exit class

RoomExitData::RoomExitData(RoomExitResource *rec) {
	xs = rec->xs; 
	ys = rec->ys;
	xe = rec->xe;
	ye = rec->ye;
	sequenceOffset = rec->sequenceOffset;
	roomNumber = rec->newRoom;
	x = rec->newRoomX;
	y = rec->newRoomY;

	switch (rec->direction) {
	case 0x80: 
		direction = UP;
		break;
	case 0x40: 
		direction = DOWN;
		break;
	case 0x20: 
		direction = LEFT;
		break;
	case 0x10: 
		direction = RIGHT;
		break;
	default:
		direction = NO_DIRECTION;
		break;
	}
}

bool RoomExitData::insideRect(int16 xp, int16 yp) {
	return ((xp >= xs) && (xp < xe) && (yp >= ys) && (yp < ye));
}

RoomExitData *RoomExitList::checkExits(int16 xp, int16 yp) {
	iterator i;
	for (i = begin(); i != end(); i++) {
		RoomExitData *rec = *i;
		if (rec->insideRect(xp, yp)) {
			return rec;
		}
	}
	return NULL;
}

// Room paths

bool RoomPathsData::isOccupied(int x, int y) {
	if ((x < 0) || (y < 0) || (x >= ROOM_PATHS_WIDTH) || (y >= ROOM_PATHS_HEIGHT))
		// Off screen, so flag as not occupied
		return false;
	return (_data[y * 5 + (x >> 3)] & (0x80 >> (x % 8))) != 0;
}

void RoomPathsData::setOccupied(int x, int y, int width) {
	if ((x < 0) || (y < 0) || (x >= ROOM_PATHS_WIDTH) || (y >= ROOM_PATHS_HEIGHT))
		return;
	byte *p = &_data[y * 5 + (x / 8)];
	byte bitMask = 0x80 >> (x % 8);

	for (int bitCtr = 0; bitCtr < width; ++bitCtr) {
		*p |= bitMask;
		bitMask >>= 1;
		if (bitMask == 0) {
			++p;
			bitMask = 0x80;
		}
	}
}

void RoomPathsData::clearOccupied(int x, int y, int width) {
	if ((x < 0) || (y < 0) || (x >= ROOM_PATHS_WIDTH) || (y >= ROOM_PATHS_HEIGHT))
		return;
	byte *p = &_data[y * 5 + (x / 8)];
	byte bitMask = 0x80 >> (x % 8);

	for (int bitCtr = 0; bitCtr < width; ++bitCtr) {
		*p &= ~bitMask;
		bitMask >>= 1;
		if (bitMask == 0) {
			++p;
			bitMask = 0x80;
		}
	}
}

// decompresses the bit-packed data for which parts of a room are occupied
// into a byte array. It also adds a column and row of padding around the
// edges of the screen, and extends occupied areas to adjust for the width
// of the chracter

void RoomPathsData::decompress(RoomPathsDecompressedData &dataOut, int characterWidth) {
	byte *pIn = &_data[ROOM_PATHS_SIZE - 1];
	uint16 *pOut = &dataOut[DECODED_PATHS_WIDTH * DECODED_PATHS_HEIGHT - 1];
	byte v;
	int paddingCtr;
	int charWidth = (characterWidth - 1) >> 3;
	int charCtr = 0;
	bool charState = false;

	// Handle padding for last row, including left/right edge padding, as
	// well as the right column of the second row
	for (paddingCtr = 0; paddingCtr < (DECODED_PATHS_WIDTH + 1); ++paddingCtr)
		*pOut-- = 0;

	for (int y = 0; y < ROOM_PATHS_HEIGHT; ++y) {
		for (int x = 0; x < (ROOM_PATHS_WIDTH / 8); ++x) {
			// Get next byte, which containing bits for 8 blocks
			v = *pIn--;		

			for (int bitCtr = 0; bitCtr < 8; ++bitCtr) {
				bool isSet = (v & 1) != 0;
				v >>= 1;

				if (charState) {
					// Handling occupied characters adjusted for character width
					if (isSet) 
						// Reset character counter
						charCtr = charWidth;

					*pOut-- = 0xffff;
					charState = (--charCtr != 0);

				} else {
					// Normal decompression
					if (!isSet) {
						// Flag block is available for walking on
						*pOut-- = 0;
					} else {
						// Flag block is occupied
						*pOut-- = 0xffff;

						// Handling for adjusting for character widths
						charCtr = charWidth - 1;
						charState = charCtr >= 0;
					}
				}
			}
		}

		// Store 2 words to allow off-screen row-start/prior row end
		*pOut-- = 0;
		*pOut-- = 0;
		charState = false;
	}

	// Handle padding for final top row - no need for end column, as end of prior
	// row provided padding for it
	for (paddingCtr = 0; paddingCtr < (ROOM_PATHS_WIDTH + 1); ++paddingCtr)
		*pOut-- = 0;
}

// Room exit joins class

RoomExitJoinData::RoomExitJoinData(RoomExitJoinResource *rec) {
	hotspot1Id = READ_LE_UINT16(&rec->hotspot1Id);
	h1CurrentFrame = rec->h1CurrentFrame;
	h1DestFrame = rec->h1DestFrame;
	h1OpenSound = rec->h1OpenSound;
	h1CloseSound = rec->h1CloseSound;
	hotspot2Id = READ_LE_UINT16(&rec->hotspot2Id);
	h2CurrentFrame = rec->h2CurrentFrame;
	h2DestFrame = rec->h2DestFrame;
	h2OpenSound = rec->h2OpenSound;
	h2CloseSound = rec->h2CloseSound;
	blocked = rec->blocked;
}

// Hotspot action record

HotspotActionData::HotspotActionData(HotspotActionResource *rec) {
	action = (Action) rec->action;
	sequenceOffset = READ_LE_UINT16(&rec->sequenceOffset);
}

uint16 HotspotActionList::getActionOffset(Action action) {
	iterator i;
	for (i = begin(); i != end(); ++i) {
		HotspotActionData *rec = *i;
		if (rec->action == action) return rec->sequenceOffset;
	}

	return 0;
}


// Hotspot data

HotspotData::HotspotData(HotspotResource *rec) {
	hotspotId = READ_LE_UINT16(&rec->hotspotId);
	nameId = READ_LE_UINT16(&rec->nameId);
	descId = READ_LE_UINT16(&rec->descId);
	descId2 = READ_LE_UINT16(&rec->descId2);
	actions = READ_LE_UINT32(&rec->actions);
	actionsOffset = READ_LE_UINT16(&rec->actionsOffset);
	flags = (byte) (actions >> 24) & 0xf0;
	actions &= 0xfffffff;

	roomNumber = READ_LE_UINT16(&rec->roomNumber);
	layer = rec->layer;
	scriptLoadFlag = rec->scriptLoadFlag;
	loadOffset = READ_LE_UINT16(&rec->loadOffset);
	startX = READ_LE_INT16(&rec->startX);
	startY = READ_LE_INT16(&rec->startY);
	width = READ_LE_UINT16(&rec->width);
	height = READ_LE_UINT16(&rec->height);
	widthCopy = READ_LE_UINT16(&rec->widthCopy);
	heightCopy = READ_LE_UINT16(&rec->heightCopy);
	yCorrection = READ_LE_UINT16(&rec->yCorrection);
	walkX = READ_LE_INT16(&rec->walkX);
	walkY = READ_LE_UINT16(&rec->walkY);
	talkX = rec->talkX;
	talkY = rec->talkY;
	colourOffset = READ_LE_UINT16(&rec->colourOffset);
	animRecordId = READ_LE_UINT16(&rec->animRecordId);
	sequenceOffset = READ_LE_UINT16(&rec->sequenceOffset);
	tickProcOffset = READ_LE_UINT16(&rec->tickProcOffset);
	tickTimeout = READ_LE_UINT16(&rec->tickTimeout);
	tickSequenceOffset = READ_LE_UINT16(&rec->tickSequenceOffset);
	npcSchedule = READ_LE_UINT16(&rec->npcSchedule);
}

// Hotspot override data

HotspotOverrideData::HotspotOverrideData(HotspotOverrideResource *rec) {
	hotspotId = READ_LE_UINT16(&rec->hotspotId);
	xs = READ_LE_INT16(&rec->xs);
	ys = READ_LE_INT16(&rec->ys);
	xe = READ_LE_INT16(&rec->xe);
	ye = READ_LE_INT16(&rec->ye);
}

// Hotspot animation movement frame

MovementData::MovementData(MovementResource *rec) {
	frameNumber = READ_LE_UINT16(&rec->frameNumber);
	xChange = READ_LE_INT16(&rec->xChange);
	yChange = READ_LE_INT16(&rec->yChange);
}

// List of movement frames

bool MovementDataList::getFrame(uint16 currentFrame, int16 &xChange, 
							   int16 &yChange, uint16 &nextFrame) {
	if (empty()) return false;
	bool foundFlag = false;
	iterator i;

	for (i = begin(); i != end(); ++i) {
		MovementData *rec = *i;
		if (foundFlag || (i == begin())) {
			xChange = rec->xChange;
			yChange = rec->yChange;
			nextFrame = rec->frameNumber;
			if (foundFlag) return true;
		}
		if (rec->frameNumber == currentFrame) foundFlag = true;
	}

	return true;
}


// Hotspot animation data

HotspotAnimData::HotspotAnimData(HotspotAnimResource *rec) {
	animRecordId = READ_LE_UINT16(&rec->animRecordId);
	animId = READ_LE_UINT16(&rec->animId);
	flags = READ_LE_UINT16(&rec->flags);

	upFrame = rec->upFrame;
	downFrame = rec->downFrame;
	leftFrame = rec->leftFrame;
	rightFrame = rec->rightFrame;
}

// Hotspot action lists

HotspotActionList::HotspotActionList(uint16 id, byte *data) {
	recordId = id;
	uint16  numItems = READ_LE_UINT16(data);
	data += 2;

	HotspotActionResource *actionRec = (HotspotActionResource *) data;
	
	for (int actionCtr = 0; actionCtr < numItems; ++actionCtr, ++actionRec) {
		HotspotActionData *actionEntry = new HotspotActionData(actionRec);
		push_back(actionEntry);
	}
}

HotspotActionList *HotspotActionSet::getActions(uint16 recordId) {
	HotspotActionSet::iterator i;
	for (i = begin(); i != end(); ++i) {
		HotspotActionList *list = *i;
		if (list->recordId == recordId) return list;
	}

	return NULL;
}

// The following class holds the set of offsets for a character's talk set

TalkHeaderData::TalkHeaderData(uint16 charId, uint16 *entries) {
	uint16 *src, *dest;
	characterId = charId;

	// Get number of entries
	_numEntries = 0;
	src = entries;
	while (READ_LE_UINT16(src) != 0xffff) { ++src; ++_numEntries; }

	// Duplicate the list
	_data = (uint16 *) Memory::alloc(_numEntries * sizeof(uint16));
	src = entries; dest = _data;

	for (int ctr = 0; ctr < _numEntries; ++ctr, ++src, ++dest)
		*dest = READ_LE_UINT16(src);
}

TalkHeaderData::~TalkHeaderData() {
	free(_data);
}

uint16 TalkHeaderData::getEntry(int index) {
	if (index >= _numEntries) 
		error("Invalid talk index %d specified for hotspot %xh", 
			_numEntries, characterId);
	return _data[index];
}

// The following class holds a single talking entry

TalkEntryData::TalkEntryData(TalkDataResource *rec) {
	preSequenceId = FROM_LE_16(rec->preSequenceId);
	descId = FROM_LE_16(rec->descId);
	postSequenceId = FROM_LE_16(rec->postSequenceId);
}

// The following class acts as a container for all the talk entries and
// responses for a single record Id

TalkData::TalkData(uint16 id) {
	recordId = id;
}

TalkData::~TalkData() {
	entries.clear();
	responses.clear();
}

TalkEntryData *TalkData::getResponse(int index) {
	TalkEntryList::iterator i = responses.begin();
	int v = index;
	while (v-- > 0) {
		if (i == responses.end()) 
			error("Invalid talk response index %d specified", index);
		++i;
	}

	return *i;
}

// The following class handles a set of coordinates a character should walk to
// if they're to exit a room to a designated secondary room

RoomExitCoordinates::RoomExitCoordinates(RoomExitCoordinateEntryResource *rec) {
	int ctr;

	for (ctr = 0; ctr < ROOM_EXIT_COORDINATES_NUM_ENTRIES; ++ctr) {
		uint16 tempY = FROM_LE_16(rec->entries[ctr].y);
		_entries[ctr].x = FROM_LE_16(rec->entries[ctr].x);
		_entries[ctr].y = tempY & 0xfff;
		_entries[ctr].roomNumber = FROM_LE_16(rec->entries[ctr].roomNumber);
		_entries[ctr].hotspotIndexId = (tempY >> 12) << 4;
	}

	for (ctr = 0; ctr < ROOM_EXIT_COORDINATES_NUM_ROOMS; ++ctr) 
		_roomIndex[ctr] = rec->roomIndex[ctr];
}

RoomExitCoordinates &RoomExitCoordinatesList::getEntry(uint16 roomNumber) {
	RoomExitCoordinatesList::iterator i = begin();
	while (--roomNumber > 0) 
		++i;
	return **i;
}

RoomExitCoordinateData &RoomExitCoordinates::getData(uint16 destRoomNumber) {
	return _entries[_roomIndex[destRoomNumber - 1]];
}

// The following classes hold any sequence offsets that are being delayed

SequenceDelayData::SequenceDelayData(uint16 delay, uint16 seqOffset) {
	OSystem &system = System::getReference();

	_timeoutCtr = system.getMillis() + delay;
	_sequenceOffset = seqOffset;
}

void SequenceDelayList::addSequence(uint16 delay, uint16 seqOffset) {
	SequenceDelayData *entry = new SequenceDelayData(delay, seqOffset);
	push_back(entry);
}

void SequenceDelayList::tick() {
	uint32 currTime = System::getReference().getMillis();
	SequenceDelayList::iterator i;

	for (i = begin(); i != end(); i++) {
		SequenceDelayData *entry = *i;
		if (entry->_timeoutCtr >= currTime) {
			uint16 seqOffset = entry->_sequenceOffset;
			erase(i);
			Script::execute(seqOffset);
			return;
		}
	}
}

// The following classes hold the NPC schedule classes

CharacterScheduleEntry::CharacterScheduleEntry(Action theAction, ...) {
	_parent = NULL;
	_action = theAction;

	va_list u_Arg;
	va_start(u_Arg, theAction);

	for (int paramCtr = 0; paramCtr < actionNumParams[_action]; ++paramCtr) 
		_params[paramCtr] = (uint16) va_arg(u_Arg, int);

	va_end(u_Arg);
}

CharacterScheduleEntry::CharacterScheduleEntry(CharacterScheduleSet *parentSet, 
		CharacterScheduleResource *&rec) {
	_parent = parentSet;

	if ((rec->action == 0) || (rec->action > NPC_JUMP_ADDRESS))
		error("Invalid action encountered reading NPC schedule");

	_action = (Action) rec->action;
	for (int index = 0; index < actionNumParams[_action]; ++index) 
		_params[index] = FROM_LE_16(rec->params[index]);

	rec = (CharacterScheduleResource *) ((byte *) rec + 
		(actionNumParams[_action] + 1) * sizeof(uint16));
}

int CharacterScheduleEntry::numParams() { 
	return actionNumParams[_action]; 
}

uint16 CharacterScheduleEntry::param(int index) {
	if ((index < 0) || (index >= numParams()))
		error("Invalid parameter index %d on handling action %d", index, _action);
	return _params[index];
}

void CharacterScheduleEntry::setDetails(Action theAction, ...) {
	_action = theAction;

	va_list list;
	va_start(list, theAction);

	for (int paramCtr = 0; paramCtr < actionNumParams[_action]; ++paramCtr) 
		_params[paramCtr] = (uint16) va_arg(list, int);

	va_end(list);
}

CharacterScheduleEntry *CharacterScheduleEntry::next() {
	if (_parent) {
		CharacterScheduleSet::iterator i;
		for (i = _parent->begin(); i != _parent->end(); ++i) {
			if (*i == this) {
				++i;
				CharacterScheduleEntry *result = (i == _parent->end()) ? NULL : *i;
				return result;
			}
		}
	}

	return NULL;
}

uint16 CharacterScheduleEntry::id() {
	return parent()->getId(this);
}

CharacterScheduleSet::CharacterScheduleSet(CharacterScheduleResource *rec, uint16 setId) {
	// Load up all the entries in the schedule

	while (rec->action != 0) {
		CharacterScheduleEntry *r = new CharacterScheduleEntry(this, rec);
		push_back(r);
	}

	_id = setId;
}

// Given a support data entry identifier, locates that entry in the list of data sets

CharacterScheduleEntry *CharacterScheduleList::getEntry(uint16 id, CharacterScheduleSet *currentSet) {
	// Respond to the special no entry with no record
	if (id == 0xffff) return NULL;

	// Handle jumps within a current set versus external jumps
	if ((id >> 10) == 0) {
		// Jump within current set
		if (currentSet == NULL)
			error("Local support data jump encountered outside of a support data sequence");
	} else {
		// Inter-set jump - locate the appropriate set
		int index = (id >> 10) - 1;

		iterator i = begin();
		while ((i != end()) && (index > 0)) {
			++i;
			--index;
		}

		if (i == end()) 
			error("Invalid index %d specified for support data set", id >> 8);
		currentSet = *i;
	}

	// Get the indexed instruction in the specified set
	int instructionIndex = id & 0x3ff;
	CharacterScheduleSet::iterator i = currentSet->begin();
	while ((i != currentSet->end()) && (instructionIndex > 0)) {
		++i;
		--instructionIndex;
	}
	if (i == currentSet->end())
		error("Invalid index %d specified within support data set", id & 0x3ff);

	return *i;
}

uint16 CharacterScheduleSet::getId(CharacterScheduleEntry *rec) {
	// Return an Id for the entry based on the id of the set combined with the 
	// index of the specific entry
	uint16 result = _id << 10;

	iterator i;
	for (i = begin(); i != end(); ++i, ++result) 
		if (*i == rec) break;
	if (i == end())
		error("Parent child relationship missing in character schedule set");
	return result;
}

// This class handles an indexed hotspot entry - which is used by the NPC code to
// determine whether exiting a room to another given room has an exit hotspot or not

RoomExitIndexedHotspotData::RoomExitIndexedHotspotData(RoomExitIndexedHotspotResource *rec) {
	roomNumber = rec->roomNumber;
	hotspotIndex = rec->hotspotIndex;
	hotspotId = FROM_LE_16(rec->hotspotId);
}

uint16 RoomExitIndexedHotspotList::getHotspot(uint16 roomNumber, uint8 hotspotIndexId) {
	iterator i;
	for (i = begin(); i != end(); ++i) {
		RoomExitIndexedHotspotData *entry = *i;
		if ((entry->roomNumber == roomNumber) && (entry->hotspotIndex == hotspotIndexId))
			return entry->hotspotId;
	}

	// No hotspot
	return 0xffff;
}

// Field list and miscellaneous variables

ValueTableData::ValueTableData() {
	_numGroats = 0;
	_playerNewPos.roomNumber = 0;
	_playerNewPos.position.x = 0;
	_playerNewPos.position.y = 0;
	for (uint16 index = 0; index < NUM_VALUE_FIELDS; ++index)
		_fieldList[index] = 0;
}

bool ValueTableData::isKnownField(uint16 fieldIndex) {
	return ((fieldIndex <= 10) && (fieldIndex != 6)) ||
		(fieldIndex == 15) || ((fieldIndex >= 18) && (fieldIndex <= 20));
}

uint16 ValueTableData::getField(uint16 fieldIndex) {
	if (fieldIndex > NUM_VALUE_FIELDS)
		error("Invalid field index specified %d", fieldIndex);
//	if (!isKnownField(fieldIndex))
//		warning("Unknown field index %d in GET_FIELD opcode", fieldIndex);
	return _fieldList[fieldIndex];
}

uint16 ValueTableData::getField(FieldName fieldName) {
	return getField((uint16) fieldName);
}

void ValueTableData::setField(uint16 fieldIndex, uint16 value) {
	if (fieldIndex > NUM_VALUE_FIELDS)
		error("Invalid field index specified %d", fieldIndex);
	_fieldList[fieldIndex] = value;
//	if (!isKnownField(fieldIndex))
//		warning("Unknown field index %d in SET_FIELD opcode", fieldIndex);
}

void ValueTableData::setField(FieldName fieldName, uint16 value) {
	setField((uint16) fieldName, value);
}

} // end of namespace Lure
