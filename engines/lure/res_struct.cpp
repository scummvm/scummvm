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

// Room data holding class

RoomData::RoomData(RoomResource *rec) {
	roomNumber = READ_LE_UINT16(&rec->roomNumber);
	descId = READ_LE_UINT16(&rec->descId);
	sequenceOffset = READ_LE_UINT16(&rec->sequenceOffset);
	numLayers = READ_LE_UINT16(&rec->numLayers);

	for (int ctr = 0; ctr < 4; ++ctr)
		layers[ctr] = READ_LE_UINT16(&rec->layers[ctr]);

	clippingXStart = READ_LE_UINT16(&rec->clippingXStart);
	clippingXEnd = READ_LE_UINT16(&rec->clippingXEnd);
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
	return ((xp >= xs) && (xp <= xe) && (yp >= ys) && (yp <= ye));
}

RoomExitData *RoomExitList::checkExits(int16 xp, int16 yp) {
	iterator i;
	for (i = begin(); i != end(); i++) {
		RoomExitData *rec = *i;
		if (rec->insideRect(xp, yp)) return rec;
	}
	return NULL;
}

// Room exit joins class

RoomExitJoinData::RoomExitJoinData(RoomExitJoinResource *rec) {
	hotspot1Id = READ_LE_UINT16(&rec->hotspot1Id);
	h1CurrentFrame = rec->h1CurrentFrame;
	h1DestFrame = rec->h1DestFrame;
	h1Unknown = READ_LE_UINT16(&rec->h1Unknown);
	hotspot2Id = READ_LE_UINT16(&rec->hotspot2Id);
	h2CurrentFrame = rec->h2CurrentFrame;
	h2DestFrame = rec->h2DestFrame;
	h2Unknown = READ_LE_UINT16(&rec->h2Unknown);
	blocked = rec->blocked;
	unknown = rec->unknown;
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
	talkX = rec->talkX;
	talkY = rec->talkY;
	colourOffset = READ_LE_UINT16(&rec->colourOffset);
	animRecordId = READ_LE_UINT16(&rec->animRecordId);
	sequenceOffset = READ_LE_UINT16(&rec->sequenceOffset);
	tickProcOffset = READ_LE_UINT16(&rec->tickProcOffset);
	tickTimeout = READ_LE_UINT16(&rec->tickTimeout);
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

// Field list and miscellaneous variables

ValueTableData::ValueTableData() {
	_numGroats = 0;

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
	if (!isKnownField(fieldIndex))
		warning("Unknown field index %d in GET_FIELD opcode", fieldIndex);
	return _fieldList[fieldIndex];
}

uint16 ValueTableData::getField(FieldName fieldName) {
	return getField((uint16) fieldName);
}

void ValueTableData::setField(uint16 fieldIndex, uint16 value) {
	if (fieldIndex > NUM_VALUE_FIELDS)
		error("Invalid field index specified %d", fieldIndex);
	_fieldList[fieldIndex] = value;
	if (!isKnownField(fieldIndex))
		warning("Unknown field index %d in SET_FIELD opcode", fieldIndex);
}

void ValueTableData::setField(FieldName fieldName, uint16 value) {
	setField((uint16) fieldName, value);
}

} // end of namespace Lure
