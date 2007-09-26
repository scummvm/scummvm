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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * This is a utility for extracting needed resource data from different language
 * version of the Lure of the Temptress lure.exe executable files into a new file 
 * lure.dat - this file is required for the ScummVM  Lure of the Temptress module 
 * to work properly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "create_lure_dat.h"

using namespace Common;

File outputFile, lureExe;
Common::Language language;
uint16 dataSegment;

#define NUM_BYTES_VALIDATE 1024
#define ENGLISH_FILE_CHECKSUM 0xFD70
#define ITALIAN_FILE_CHECKSUM 0x109AD

Common::Language processedLanguages[100];
int langIndex = 0;
uint16 animOffsets[MAX_NUM_ANIM_RECORDS];
int animIndex = 0;
uint16 actionOffsets[MAX_NUM_ACTION_RECORDS];
int actionIndex = 0;

#define TALK_NUM_ENTRIES 28
#define MAX_TALK_LISTS 300

uint16 talkOffsets[MAX_TALK_LISTS];
int talkOffsetIndex = 0;

void add_anim_record(uint16 offset) {
	for (int ctr = 0; ctr < animIndex; ++ctr)
		if (animOffsets[ctr] == offset) return;
	if (animIndex == MAX_NUM_ANIM_RECORDS) {
		printf("Animation record offset table size exceeded\n");
		exit(1);
	}
	animOffsets[animIndex++] = offset;
}

void add_action_list(uint16 offset) {
	for (int ctr = 0; ctr < actionIndex; ++ctr)
		if (actionOffsets[ctr] == offset) return;
	if (actionIndex == MAX_NUM_ACTION_RECORDS) {
		printf("Action record offset table size exceeded\n");
		exit(1);
	}
	actionOffsets[actionIndex++] = offset;
}

void read_basic_palette(byte *&data, uint16 &totalSize) {
	totalSize = PALETTE_SIZE;
	uint16 dataStart = 0xC0A7;
	if (language == IT_ITA) dataStart = 0xC107;

	lureExe.seek(dataStart);
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

#define ALT_PALETTE_1 0x1757
#define ALT_PALETTE_1_SIZE 180
#define ALT_PALETTE_2 0x180B
#define ALT_PALETTE_2_SIZE 24

void read_replacement_palette(byte *&data, uint16 &totalSize) {
	totalSize = ALT_PALETTE_1_SIZE + ALT_PALETTE_2_SIZE;
	data = (byte *) malloc(totalSize);

	lureExe.seek(dataSegment + ALT_PALETTE_1);
	lureExe.read(data, ALT_PALETTE_1_SIZE);
	lureExe.seek(dataSegment + ALT_PALETTE_2);
	lureExe.read(data + ALT_PALETTE_1_SIZE, ALT_PALETTE_2_SIZE);
}

void read_dialog_data(byte *&data, uint16 &totalSize) {
	uint32 segmentStart = 0x1dcb0;
	if (language == IT_ITA) segmentStart = 0x1ddd0;

	totalSize = DIALOG_SIZE;
	lureExe.seek(segmentStart);
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_talk_dialog_data(byte *&data, uint16 &totalSize) {
	uint32 segmentStart = 0x1de00;
	if (language == IT_ITA) segmentStart = 0x1df20;

	totalSize = TALK_DIALOG_SIZE;
	lureExe.seek(segmentStart);
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_room_data(byte *&data, uint16 &totalSize)  {
	data = (byte *) malloc(MAX_DATA_SIZE);
	memset(data, 0, MAX_DATA_SIZE);

	uint16 *offsetPtr = (uint16 *) data;
	uint16 offset = (ROOM_NUM_ENTRIES + 1) * sizeof(uint16);
	uint16 pixelOffset;
	RoomResource buffer;
	RoomHeaderEntry headerEntry;
	RoomRectIn bounds;

	uint16 dataStart = 0xbf40;
	uint16 walkAreaOffset = 0x2eb1;
	if (language == IT_ITA) {
		dataStart = 0xc000;
		walkAreaOffset = 0x2ec0;
	}

	for (int index = 0; index < ROOM_NUM_ENTRIES; ++index) {

		lureExe.seek(dataSegment + dataStart + index * 9);
		lureExe.read(&headerEntry, sizeof(RoomHeaderEntry));

		if ((FROM_LE_16(headerEntry.offset) != 0) && 
			(FROM_LE_16(headerEntry.offset) != 0xffff) &&
			(FROM_LE_16(headerEntry.roomNumber) != 0)) {
			// Store offset of room entry
			*offsetPtr++ = TO_LE_16(offset);

			// Copy over basic room details
			lureExe.seek(dataSegment + FROM_LE_16(headerEntry.offset));
			lureExe.read(&buffer, sizeof(RoomResource));
			RoomResourceOutput *rec = (RoomResourceOutput *) (data + offset);
			rec->hdrFlags = headerEntry.hdrFlags;
			rec->actions = FROM_LE_32(buffer.actions);
			rec->roomNumber = index; 
			rec->descId = headerEntry.descId;
			rec->numLayers = buffer.numLayers;
			memcpy(rec->layers, buffer.layers, 8);
			rec->sequenceOffset = buffer.sequenceOffset;			
			rec->clippingXStart = TO_LE_16(FROM_LE_16(buffer.clippingXStart) - 0x80);
			rec->clippingXEnd = (FROM_LE_16(buffer.clippingXEnd) == 0) ? 0 : 
			  TO_LE_16(FROM_LE_16(buffer.clippingXEnd) - 0x80);
			rec->exitTime = FROM_LE_32(buffer.exitTime);
			rec->areaFlag = buffer.areaFlag;
			rec->numExits = 0;

			offset += sizeof(RoomResourceOutput);

			// Copy over room exits
			for (int foo = 0; ; ++foo) {
				RoomResourceExit1 *p = (RoomResourceExit1 *) (data + offset);
				lureExe.read(p, sizeof(RoomResourceExit1));
				if (FROM_LE_16(p->xs) == 0xffff) break;

				rec->numExits = TO_LE_16(FROM_LE_16(rec->numExits) + 1);
				p->xs = TO_LE_16(FROM_LE_16(p->xs) - 0x80);
				p->ys = TO_LE_16(FROM_LE_16(p->ys) - 0x80);
				p->xe = TO_LE_16(FROM_LE_16(p->xe) - 0x80);
				p->ye = TO_LE_16(FROM_LE_16(p->ye) - 0x80);

				offset += sizeof(RoomResourceExit1);
				RoomResourceExit2 *p2 = (RoomResourceExit2 *) (data + offset);

				if (FROM_LE_16(p->sequenceOffset) == 0xffff) {
					lureExe.read(p2, sizeof(RoomResourceExit2));
					p2->newRoomX = TO_LE_16(FROM_LE_16(p2->newRoomX) - 0x80);
					p2->newRoomY = TO_LE_16(FROM_LE_16(p2->newRoomY) - 0x80);
				} else {
					p2->newRoom = 0;
					p2->direction = 0;
					p2->newRoomX = 0;
					p2->newRoomY = 0;
				}

				offset += sizeof(RoomResourceExit2);
			}

			// Handle the random destination walk bounds for the room

			lureExe.seek(dataSegment + walkAreaOffset + 
				buffer.walkBoundsIndex * sizeof(RoomRectIn));
			lureExe.read(&bounds, sizeof(RoomRectIn));
			rec->walkBounds.xs = TO_LE_16(FROM_LE_16(bounds.xs) - 0x80);
			rec->walkBounds.xe = TO_LE_16(FROM_LE_16(bounds.xe) - 0x80);
			rec->walkBounds.ys = TO_LE_16(FROM_LE_16(bounds.ys) - 0x80);
			rec->walkBounds.ye = TO_LE_16(FROM_LE_16(bounds.ye) - 0x80);

			// If the room has a default pixel blocks list, add the references
			if (buffer.pixelListOffset != 0) {
				lureExe.seek(dataSegment + FROM_LE_16(buffer.pixelListOffset));
				pixelOffset = lureExe.readWord();
				while (pixelOffset != 0) {
					add_anim_record(pixelOffset);
					pixelOffset = lureExe.readWord();
				}
			}
		}
	}

	WRITE_LE_UINT16(offsetPtr, 0xffff);
	totalSize = offset;
}

uint16 englishTickProcOffsets[] = {
	0x41BD, 0x4f82, 0x5e44, 0x625e, 0x6571, 0x7207, 0x7c14, 0x7c24, 0x7efa, 0x7f02,
	0x7F37, 0x7f3a, 0x7f54, 0x7f69, 0x7fa1, 0x8009, 0x80c6, 0x813f, 0x8180, 0x81b3,
	0x81f3, 0x820e, 0x8241, 0x82a0, 0x85ce, 0x862d, 0x865A, 0x86FA, 0x86FF, 0x871E,
	0x873D, 0x8742, 0x8747, 0x87B3, 0x87EC, 0x882A, 0x8ABD, 0x982D, 0x98B6, 
	0xffff
};

uint16 italianTickProcOffsets[] = {
	0x4205, 0x4fca, 0x5e8c, 0x62a6, 0x65b9, 0x724f, 0x7c5c, 0x7c6c, 0x7f58, 0x7f60,
	0x7f95, 0x7f98, 0x7fb2, 0x7fc7, 0x7fff, 0x8019, 0x8067, 0x8124, 0x819d, 0x81de, 
	0x8211, 0x8251, 0x826c, 0x829f, 0x82fe, 0x862c, 0x868b, 0x8758, 0x875D, 0x877C, 
    0x879B, 0x87a0, 0x87a5, 0x8811, 0x884a, 0x8888,	0x8b20,	0x988f, 0x9918, 
	0xffff
};

uint16 englishOffsets[4] = {0x5d98, 0x5eb8, 0x623e, 0x63b1};
uint16 italianOffsets[4] = {0x5e58, 0x5f78, 0x62fe, 0x6471};

uint16 englishLoadOffsets[] = {0x3afe, 0x41BD, 0x7167, 0x7172, 0x8617, 0x88ac, 0};
uint16 italianLoadOffsets[] = {0x3b46, 0x4205, 0x71af, 0x71ba, 0x8675, 0x890a, 0};

void read_hotspot_data(byte *&data, uint16 &totalSize)  {
	uint16 startId[4] = {0x3e8, 0x408, 0x2710, 0x7530};
	int walkNumEntries = 0;
	int walkCtr;
	int numEntries;
	HotspotWalkToRecord rec;
	HotspotWalkToRecord *walkList;
	HotspotHeaderEntry entryHeader;
	HotspotResource entry;
	uint16 dataSize;
	HotspotResourceOutput *r;
	CurrentActionInput action;

	// Set up list pointers for various languages
	uint16 *offsets = &englishOffsets[0];
	uint16 *procList = &englishTickProcOffsets[0];
	uint16 *loadOffsets = &englishLoadOffsets[0];
	uint16 walkToOffset = 0xBC4B;
	if (language == IT_ITA) {
		offsets = &italianOffsets[0];
		procList = &italianTickProcOffsets[0];
		walkToOffset = 0xBD0B;
		loadOffsets = &italianLoadOffsets[0];
	}

	// Allocate enough space for output hotspot list
	data = (byte *) malloc(MAX_HOTSPOTS * sizeof(HotspotResourceOutput));

	// Determine number of hotspot walk to entries

	lureExe.seek(dataSegment + walkToOffset);
	do {
		++walkNumEntries;
		lureExe.read(&rec, sizeof(HotspotWalkToRecord));
	} while (TO_LE_16(rec.hotspotId) != 0);
	--walkNumEntries;

	dataSize = walkNumEntries * sizeof(HotspotWalkToRecord);
	walkList = (HotspotWalkToRecord *) malloc(dataSize);
	lureExe.seek(dataSegment + walkToOffset);
	lureExe.read(walkList, sizeof(HotspotWalkToRecord) * walkNumEntries);

	// Main code for creating the hotspot list

	r = (HotspotResourceOutput *) data;
	numEntries = 0;

	for (int tableNum = 0; tableNum < 4; ++tableNum) {
		uint16 hotspotIndex = 0;
		for (;;) {
			lureExe.seek(dataSegment + offsets[tableNum] +  hotspotIndex * 9);
			lureExe.read(&entryHeader, sizeof(HotspotHeaderEntry));
			if (FROM_LE_16(entryHeader.offset) == 0xffff) break;
			if (FROM_LE_16(entryHeader.offset) == 0) {
				++hotspotIndex;
				continue;
			}

			memset(r, 0, sizeof(HotspotResourceOutput));
			r->hotspotId = TO_LE_16(startId[tableNum] + hotspotIndex);
			r->nameId = entryHeader.resourceId;
			r->descId = entryHeader.descId; 
			r->descId2 = entryHeader.descId2;
			r->hdrFlags = entryHeader.hdrFlags;

			// Get the hotspot data
			lureExe.seek(dataSegment + entryHeader.offset);
			lureExe.read(&entry, sizeof(HotspotResource));
			
			r->actions = entry.actions;
			r->roomNumber = entry.roomNumber;
			r->startX = TO_LE_16(FROM_LE_16(entry.startX) - 0x80);
			r->startY = TO_LE_16(FROM_LE_16(entry.startY) - 0x80);

			r->width = entry.width;
			r->height = entry.height;
			r->widthCopy = entry.widthCopy;
			r->heightCopy = entry.heightCopy;
			r->yCorrection = entry.yCorrection;
			r->talkX = entry.talkX;
			r->talkY = entry.talkY;
			r->characterMode = entry.characterMode;
			r->delayCtr = entry.delayCtr;
			r->tickSequenceOffset = entry.tickSequenceOffset;

			r->layer = entry.layer;
			r->colourOffset = entry.colourOffset;
			r->hotspotScriptOffset = entry.hotspotScriptOffset;
			r->talkScriptOffset = entry.talkScriptOffset;
			r->flags = entry.flags;

			// Handle any necessary translation of script load offsets

			r->scriptLoadFlag = entry.scriptLoadFlag;
			if (r->scriptLoadFlag || (tableNum == 3))
				// Load offset is in script segment, so leave as is
				r->loadOffset = entry.loadOffset;
			else {
				// Translate load offset to an index
				int loadIndex = 0;
				while ((loadOffsets[loadIndex] != FROM_LE_16(entry.loadOffset)) &&
						(loadOffsets[loadIndex] != 0))
					++loadIndex;

				if (loadOffsets[loadIndex] == 0) {
					printf("Unknown load offset encountered for hotspot %xh offset %xh\n", 
						startId[tableNum] + hotspotIndex,
						FROM_LE_16(entry.loadOffset));
					exit(1);
				}

				r->loadOffset = TO_LE_16(loadIndex + 1);
			}

			if (tableNum == 3) {
				r->tickProcId = 0;
			} else {
				// Scan through the proc list for the correct offset
				int procIndex = 0;
				while ((procList[procIndex] != FROM_LE_16(entry.tickProcOffset)) &&
						(procList[procIndex] != 0xffff))
					++procIndex;

				if (procList[procIndex] == 0xffff) {
					if ((FROM_LE_16(entry.tickProcOffset) != 0xe00) &&
						(FROM_LE_16(entry.tickProcOffset) != 2))
						printf("Could not find a tick proc handler for hotspot %xh offset %xh\n", 
							startId[tableNum] + hotspotIndex,
							FROM_LE_16(entry.tickProcOffset));
					r->tickProcId = 0;
				}
				else
					r->tickProcId = TO_LE_16(procIndex + 1);
			}

			// Special check for the tinderbox hotspot to set it's room number correctly - the original 
			// game used this as a backup against people trying to hack the copy protection
			if (startId[tableNum] + hotspotIndex == 0x271C)
				r->roomNumber = TO_LE_16(28);
if (startId[tableNum] + hotspotIndex == 0x46b) r->tickProcId = 1;

			// Find the walk-to coordinates for the hotspot
			uint16 findId = FROM_LE_16(r->hotspotId);
			walkCtr = 0;
			while (walkCtr < walkNumEntries) {
				uint16 id = FROM_LE_16(walkList[walkCtr].hotspotId);
				if ((id == findId) || ((findId == 1007) && (id == 0xffff)))
					break;
				++walkCtr;
			}
					
			if (walkCtr == walkNumEntries) {
				r->walkX = 0;
				r->walkY = 0;
			} else {
				r->walkX = TO_LE_16(FROM_LE_16(walkList[walkCtr].x) - 0x80);
				uint16 y = FROM_LE_16(walkList[walkCtr].y);
				r->walkY = TO_LE_16((y & 0x8000) | (uint16) ((int16) (y & 0x7fff) - 0x80));
			}

			// Use the offset of the animation data as a dummy Id for the data
			r->animRecordId = entry.animOffset;
			r->tickTimeout = entry.tickTimeout;
			add_anim_record(FROM_LE_16(entry.animOffset));

			// Add in the actions offset table
			r->actionsOffset = entry.actionsOffset;
			if (FROM_LE_16(entry.actionsOffset) != 0)
				add_action_list(FROM_LE_16(entry.actionsOffset));

			if (FROM_LE_16(r->hotspotId) >= 0x408) {
				// Hotspot is not an NPC
				r->npcSchedule = 0;
			} else {
				// Check for an NPC schedule
				lureExe.seek(dataSegment + entryHeader.offset + 0x63);
				lureExe.read(&action, sizeof(CurrentActionInput));

				if (action.action != 2) 
					r->npcSchedule = 0;
				else {
					r->npcSchedule = get_sequence_index(FROM_LE_16(action.dataOffset));
				}
			}

			++hotspotIndex;
			++r;
			++numEntries;

			if (numEntries == MAX_HOTSPOTS) {
				printf("Ran out of stack spaces for hotspot copying\n");
				exit(1);
			}
		}
	}

	r->hotspotId = TO_LE_16(0xffff);
	totalSize = numEntries * sizeof(HotspotResourceOutput) + 2;

	// Dispose of hotspot walk-to co-ordinate list
	free(walkList);
}

void read_hotspot_override_data(byte *&data, uint16 &totalSize) 
{
	lureExe.seek(dataSegment + HOTSPOT_OVERRIDE_OFFSET);
	int numOverrides = 0;
	HotspotOverride rec;

	// Determine number of hotspot overrides
	do {
		++numOverrides;
		lureExe.read(&rec, sizeof(HotspotOverride));
	} while (FROM_LE_16(rec.hotspotId) != 0);
	--numOverrides;

	// Prepare output data and read in all entries at once
	totalSize = numOverrides * sizeof(HotspotOverride) + 2;
	data = (byte *) malloc(totalSize);
	lureExe.seek(dataSegment + HOTSPOT_OVERRIDE_OFFSET);
	lureExe.read(data, totalSize - 2);
	WRITE_LE_UINT16(data + totalSize - 2, 0xffff);

	// Post-process the coordinates
	HotspotOverride *p = (HotspotOverride *) data;
	for (int overrideCtr = 0; overrideCtr < numOverrides; ++overrideCtr, ++p) {
		p->xs = TO_LE_16(FROM_LE_16(p->xs) - 0x80);
		p->xe = TO_LE_16(FROM_LE_16(p->xe) - 0x80);
		p->ys = TO_LE_16(FROM_LE_16(p->ys) - 0x80);
		p->ye = TO_LE_16(FROM_LE_16(p->ye) - 0x80);
	}
}

void read_room_exits(byte *&data, uint16 &totalSize) {
	RoomExitHotspotRecord rec;
	uint16 offsets[NUM_ROOM_EXITS];
	uint16 numEntries[NUM_ROOM_EXITS];
	int roomCtr;
	totalSize = (NUM_ROOM_EXITS + 1) * sizeof(uint16);

	uint16 dataStart = 0x2F61;
	if (language == IT_ITA) dataStart = 0x2f70;

	lureExe.seek(dataSegment + dataStart);
	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr)
		offsets[roomCtr] = lureExe.readWord();

	// First loop to find total of room exit records there are
	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr) {
		numEntries[roomCtr] = 0;
		if (offsets[roomCtr] == 0) continue;

		// Get number of exits for the room
		lureExe.seek(dataSegment + offsets[roomCtr]);
		lureExe.read(&rec, sizeof(RoomExitHotspotRecord));
		while (FROM_LE_16(rec.xs) != 0) {
			totalSize += sizeof(RoomExitHotspotOutputRecord);
			numEntries[roomCtr]++;
			lureExe.read(&rec, sizeof(RoomExitHotspotRecord));
		}
		totalSize += sizeof(uint16); // save room for room list end flag
	}

	// Alloacte the total needed space
	data = (byte *) malloc(totalSize);
	uint16 *offset = (uint16 *) data;
	uint16 destIndex = (NUM_ROOM_EXITS + 1) * sizeof(uint16);
	uint16 entryCtr;
	
	// Loop to build up the result table

	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr) {
		if (offsets[roomCtr] == 0) {
			*offset++ = 0;		// No entries
		} else {
			// Read in the entries for the room
			*offset++ = TO_LE_16(destIndex);

			RoomExitHotspotOutputRecord *destP = (RoomExitHotspotOutputRecord *) 
				(data + destIndex);

			lureExe.seek(dataSegment + offsets[roomCtr]);

			for (entryCtr = 0; entryCtr < numEntries[roomCtr]; ++entryCtr, ++destP) {
				lureExe.read(&rec, sizeof(RoomExitHotspotRecord));

				// Copy over the record
				destP->xs = TO_LE_16(FROM_LE_16(rec.xs) - 0x80);
				destP->xe = TO_LE_16(FROM_LE_16(rec.xe) - 0x80);
				destP->ys = TO_LE_16(FROM_LE_16(rec.ys) - 0x80);
				destP->ye = TO_LE_16(FROM_LE_16(rec.ye) - 0x80);
				destP->hotspotId = rec.hotspotId;
				destP->cursorNum = rec.cursorNum;
				destP->destRoomNumber = rec.destRoomNumber;
			}
			
			destIndex += numEntries[roomCtr] * sizeof(RoomExitHotspotOutputRecord);
			WRITE_LE_UINT16(data + destIndex, 0xffff);
			destIndex += sizeof(uint16);
		}
	}
	WRITE_LE_UINT16(offset, 0xffff);
}

void read_room_exit_joins(byte *&data, uint16 &totalSize) {
	RoomExitHotspotJoinRecord rec, *p;
	int numRecords = 0;
	uint32 unused;

	uint16 dataStart = 0xce30;
	if (language == IT_ITA) dataStart = 0xcef0;
	lureExe.seek(dataSegment + dataStart);

	do {
		lureExe.read(&rec, sizeof(RoomExitHotspotJoinRecord));
		lureExe.read(&unused, sizeof(uint32));
		++numRecords;
	} while (FROM_LE_16(rec.hotspot1Id) != 0);
	--numRecords;

	// Allocate the data and read in all the records
	totalSize = (numRecords * sizeof(RoomExitHotspotJoinRecord)) + 2;
	data = (byte *) malloc(totalSize);
	lureExe.seek(dataSegment + dataStart);
	
	p = (RoomExitHotspotJoinRecord *) data;
	for (int recordCtr = 0; recordCtr < numRecords; ++recordCtr)
	{
		lureExe.read(p, sizeof(RoomExitHotspotJoinRecord));
		lureExe.read(&unused, sizeof(uint32));
		++p;
	}

	WRITE_LE_UINT16(p, 0xffff);
}

// This next method reads in the animation and movement data

#define NUM_LANGUAGES 2
struct AnimListRecord {
	uint16 languages[NUM_LANGUAGES];
};

AnimListRecord animDataList[] = {
	{{0x1830, 0x1830}},	// Copy protection header
	{{0x1839, 0x1839}},	// Copy protection wording header
	{{0x1842, 0x1842}},	// Copy protection numbers
	{{0x184B, 0x184B}},	// Restart/Restore buttons
	{{0x55C0, 0x5680}},	// Player midswing animation
	{{0x55C9, 0x5689}},	// Player mid-level defend
	{{0x55D2, 0x5692}},	// Player high-level strike
	{{0x55DB, 0x569B}},	// Player high-level defend
	{{0x55E4, 0x56A4}},	// Player low-level strike
	{{0x55ED, 0x56AD}},	// Player low-level defend
	{{0x55F6, 0x56B6}},	// Player fight animation
	{{0x55FF, 0x56BF}},	// Pig fight animation
	{{0x5611, 0x56D1}},	// Player mid-level strike
	{{0x5623, 0x56E3}},	// Pig fight animation
	{{0x562C, 0x56EC}},	// Misc fight animation
	{{0x5635, 0x56F5}},	// Pig fight animation
	{{0x563E, 0x56FE}},	// Player recoiling from hit
	{{0x5647, 0x5707}},	// Pig recoiling from hit
	{{0x5650, 0x5710}},	// Pig dies
	{{0x5810, 0x58D0}},	// Voice bubble
	{{0x5915, 0x59D5}},	// Blacksmith hammering
	{{0x59ED, 0x5AAD}},	// Ewan's alternate animation
	{{0x59FF, 0x5ABF}},	// Dragon breathing fire
	{{0x5A08, 0x5AC8}},	// Dragon breathing fire 2
	{{0x5A11, 0x5AD1}},	// Dragon breathing fire 3
	{{0x5A1A, 0x5ADA}},	// Player turning winch in room #48
	{{0x5A59, 0x5B19}},	// Player pulling lever in room #48
	{{0x5A62, 0x5B22}},	// Minnow pulling lever in room #48
	{{0x5AAA, 0x5B6A}},	// Goewin mixing potion
	{{0x5C95, 0x5D55}},
	{{0x5CAA, 0x5D6A}},	// Selena animation
	{{0x5CE9, 0x5DA9}},	// Blacksmith in bar?
	{{0x5D28, 0x5DE8}},	// Goewin animation
	{{0, 0}}
};

void read_anim_data(byte *&data, uint16 &totalSize) {
	// Add special pixel records
	int index = 0;
	if (language == IT_ITA) index = 1;

	AnimListRecord *p = &animDataList[0];
	while (p->languages[index] != 0) {
		add_anim_record(p->languages[index]);
		++p;;
	}

	// Get the animation data records
	AnimRecord inRec;
	MovementRecord move;
	MovementRecord *destMove;
	uint16 offset, moveOffset;
	uint16 startOffset;
	int ctr, dirCtr;
	int movementSize = 0;
	bool *includeAnim = (bool *) malloc(animIndex);

	// Loop to figure out the total number of movement records there are
	for (ctr = 0; ctr < animIndex; ++ctr) {
		lureExe.seek(dataSegment + animOffsets[ctr]);
		lureExe.read(&inRec, sizeof(AnimRecord));

		if ((FROM_LE_16(inRec.leftOffset) < 0x5000) || 
			(FROM_LE_16(inRec.rightOffset) < 0x5000) ||
			(abs(FROM_LE_16(inRec.leftOffset)-FROM_LE_16(inRec.rightOffset)) > 0x800) ||
			(abs(FROM_LE_16(inRec.rightOffset)-FROM_LE_16(inRec.upOffset)) > 0x800) ||
			(abs(FROM_LE_16(inRec.upOffset)-FROM_LE_16(inRec.downOffset)) > 0x800)) {
			// Animation doesn't have valid movement data
			includeAnim[ctr] = false;
		} else {
			includeAnim[ctr] = true;
			for (dirCtr=0; dirCtr<4; ++dirCtr) {
				switch (dirCtr) {
				case 0:
					offset = FROM_LE_16(inRec.leftOffset);
					break;
				case 1:
					offset = FROM_LE_16(inRec.rightOffset);
					break;
				case 2:
					offset = FROM_LE_16(inRec.upOffset);
					break;
				default:
					offset = FROM_LE_16(inRec.downOffset);
				}

				if (offset != 0) {
					lureExe.seek(dataSegment + offset);
					lureExe.read(&move, sizeof(MovementRecord));

					while (FROM_LE_16(move.frameNumber) != 0xffff) {
						movementSize += sizeof(MovementRecord);
						lureExe.read(&move, sizeof(MovementRecord));
					}
					movementSize += 2;
				}
			}
		}
	}

	totalSize = animIndex * sizeof(AnimRecordOutput) + 2 + movementSize;
	AnimRecordOutput *rec = (AnimRecordOutput *) malloc(totalSize);
	data = (byte *) rec;
	moveOffset = animIndex * sizeof(AnimRecordOutput) + 2;

	// Loop to get in the animation records
	for (ctr = 0; ctr < animIndex; ++ctr, ++rec) {
		lureExe.seek(dataSegment + animOffsets[ctr]);
		lureExe.read(&inRec, sizeof(AnimRecord));

		rec->animRecordId = animOffsets[ctr];
		rec->animId = inRec.animId;
		rec->flags = TO_LE_16(inRec.flags);

		rec->leftOffset = 0; 
		rec->rightOffset = 0;
		rec->upOffset = 0;
		rec->downOffset = 0;

		rec->upFrame = inRec.upFrame;
		rec->downFrame = inRec.downFrame;
		rec->leftFrame = inRec.leftFrame;
		rec->rightFrame = inRec.rightFrame;

		if (includeAnim[ctr]) {
			// Loop to get movement records
			uint16 *inDirs[4] = {&inRec.leftOffset, &inRec.rightOffset,
				&inRec.upOffset, &inRec.downOffset};
			uint16 *outDirs[4] = {&rec->leftOffset, &rec->rightOffset,
				&rec->upOffset, &rec->downOffset};

			for (dirCtr=0; dirCtr<4; ++dirCtr) {
				offset = READ_LE_UINT16(inDirs[dirCtr]);

				if (offset == 0) {
					startOffset = 0;
				} else {
					startOffset = moveOffset;

					lureExe.seek(dataSegment + offset);
					lureExe.read(&move, sizeof(MovementRecord));
					destMove = (MovementRecord *) (data + moveOffset);

					while (FROM_LE_16(move.frameNumber) != 0xffff) {
						destMove->frameNumber = move.frameNumber;
						destMove->xChange = move.xChange;
						destMove->yChange = move.yChange;

						moveOffset += sizeof(MovementRecord);
						++destMove;
						lureExe.read(&move, sizeof(MovementRecord));
					}
					
					destMove->frameNumber = TO_LE_16(0xffff);
					moveOffset += 2;
				}

				WRITE_LE_UINT16(outDirs[dirCtr], startOffset);
			}
		}
	}

	rec->animRecordId = TO_LE_16(0xffff);
	delete includeAnim;
}

void read_script_data(byte *&data, uint16 &totalSize) {
	uint32 scriptSegment = 0x1df00;
	if (language == IT_ITA) scriptSegment = 0x1e020;
	lureExe.seek(scriptSegment);
	
	totalSize = SCRIPT_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_script2_data(byte *&data, uint16 &totalSize) {
	uint32 scriptSegment = 0x19c70;
	if (language == IT_ITA) scriptSegment = 0x19D90;
	lureExe.seek(scriptSegment);
	
	totalSize = SCRIPT2_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_hotspot_script_offsets(byte *&data, uint16 &totalSize) {
	lureExe.seek(dataSegment + HOTSPOT_SCRIPT_LIST);
	
	totalSize = HOTSPOT_SCRIPT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_messages_segment(byte *&data, uint16 &totalSize) {
	lureExe.seek(MESSAGES_SEGMENT);
	totalSize = MESSAGES_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

// Reads in the list of actions used

void read_actions_list(byte *&data, uint16 &totalSize) {
	// Allocate enough space for output action list
	data = (byte *) malloc(MAX_DATA_SIZE);
	HotspotActionsRecord *header = (HotspotActionsRecord *) data;
	uint16 offset = actionIndex * sizeof(HotspotActionsRecord) + sizeof(uint16);

	for (int ctr = 0; ctr < actionIndex; ++ctr) {
		header->recordId = actionOffsets[ctr];
		header->offset = offset;
		++header;

		lureExe.seek(dataSegment + actionOffsets[ctr]);
		uint16 *numItems = (uint16 *) (data + offset);
		lureExe.read(numItems, sizeof(uint16));
		offset += 2;

		if (READ_UINT16(numItems) > 0) {
			lureExe.read(data + offset, READ_UINT16(numItems) * 3);
			offset += READ_UINT16(numItems) * 3;
		}
	}
	header->recordId = TO_LE_16(0xffff);
}

// Reads in the talk data 

void add_talk_offset(uint16 offset) {
	for (int ctr = 0; ctr < talkOffsetIndex; ++ctr) 
		if (talkOffsets[ctr] == offset) return;
	if (talkOffsetIndex == MAX_TALK_LISTS) {
		printf("Exceeded maximum talk offset list size\n");
		exit(1);
	}

	talkOffsets[talkOffsetIndex++] = offset;
}

struct TalkEntry {
	uint16 hotspotId;
	uint16 offset;
};

void read_talk_headers(byte *&data, uint16 &totalSize) {
	TalkEntry entries[TALK_NUM_ENTRIES];
	uint16 sortedOffsets[TALK_NUM_ENTRIES+1];
	int entryCtr, subentryCtr;

	uint16 dataStart = 0x505c;
	if (language == IT_ITA) dataStart = 0x511C;
	lureExe.seek(dataSegment + dataStart);
	lureExe.read(&entries[0], sizeof(TalkEntry) * TALK_NUM_ENTRIES);

	// Sort the entry offsets into a list - this is used to figure out each entry's size
	int currVal, prevVal = 0;
	for (entryCtr = 0; entryCtr < TALK_NUM_ENTRIES; ++entryCtr) {
		currVal = 0xffff;
		for (subentryCtr = 0; subentryCtr < TALK_NUM_ENTRIES; ++subentryCtr) {
			if ((FROM_LE_16(entries[subentryCtr].offset) < currVal) &&
				(FROM_LE_16(entries[subentryCtr].offset) > prevVal)) 
				currVal = FROM_LE_16(entries[subentryCtr].offset);
		}
		if (currVal == 0xffff) break;

		sortedOffsets[entryCtr] = currVal;
		prevVal = currVal;
	}
	sortedOffsets[entryCtr] = 0x5540; // end for end record

	data = (byte *) malloc(MAX_DATA_SIZE);
	TalkEntry *entry = (TalkEntry *) data;
	uint16 offset = TALK_NUM_ENTRIES * sizeof(TalkEntry) + sizeof(uint16);

	for (entryCtr = 0; entryCtr < TALK_NUM_ENTRIES; ++entryCtr) {
		entry->hotspotId = entries[entryCtr].hotspotId;
		entry->offset = TO_LE_16(offset);
		++entry;

		// Find the following offset in a sorted list
		int startOffset = FROM_LE_16(entries[entryCtr].offset);
		int nextOffset = 0;
		for (subentryCtr = 0; subentryCtr < TALK_NUM_ENTRIES; ++subentryCtr) {
			if (sortedOffsets[subentryCtr] == startOffset) {
				nextOffset = sortedOffsets[subentryCtr+1];
				break;
			}
		}
		if (nextOffset == 0) 
			exit(1);

		// Read in line entries into the data
		lureExe.seek(dataSegment + startOffset);
		int size = nextOffset - startOffset;
		uint16 *talkOffset = (uint16 *) (data + offset);
		lureExe.read(talkOffset, size);

		while (size > 0) {
			if (READ_UINT16(talkOffset) != 0) 
				add_talk_offset(READ_UINT16(talkOffset));
			size -= sizeof(uint16);
			offset += sizeof(uint16);
			talkOffset++;
		}

		WRITE_LE_UINT16(talkOffset, 0xffff);
		offset += 2;
	}

	add_talk_offset(0xffff);
	entry->hotspotId = TO_LE_16(0xffff);
	totalSize = offset + 2;
}

// Reads in the contents of the previously loaded talk lists

struct TalkRecord {
	uint16 recordId;
	uint16 listOffset;
	uint16 responsesOffset;
};

uint16 englishGiveTalkIds[7] = {0xCF5E, 0xCF14, 0xCF90, 0xCFAA, 0xCFD0, 0xCFF6, 0xf010};
uint16 italianGiveTalkIds[7] = {0xD01E, 0xCFD4, 0xD050, 0xD06A, 0xD090, 0xD0B6, 0xf0d0};

void read_talk_data(byte *&data, uint16 &totalSize) {
	uint16 responseOffset;
	int talkCtr, subentryCtr;
	uint16 size;

	uint16 *giveTalkIds = &englishGiveTalkIds[0];
	if (language == IT_ITA) giveTalkIds = &italianGiveTalkIds[0];

	for (talkCtr = 0; talkCtr < 6; ++talkCtr)
		add_talk_offset(giveTalkIds[talkCtr]);

	data = (byte *) malloc(MAX_DATA_SIZE);
	TalkRecord *header = (TalkRecord *) data;
	uint16 offset = talkOffsetIndex * sizeof(TalkRecord) + sizeof(uint16);

	uint16 *sortedList = (uint16 *) malloc((talkOffsetIndex+1) * sizeof(uint16));
	memset(sortedList, 0, (talkOffsetIndex+1) * sizeof(uint16));

	// Sort the entry offsets into a list - this is used to figure out each entry's size
	int currVal, prevVal = 0;
	for (talkCtr = 0; talkCtr < talkOffsetIndex; ++talkCtr) {
		currVal = 0xffff;
		for (subentryCtr = 0; subentryCtr < talkOffsetIndex; ++subentryCtr) {
			if ((talkOffsets[subentryCtr] < currVal) &&
				(talkOffsets[subentryCtr] > prevVal)) 
				currVal = talkOffsets[subentryCtr];
		}
		if (currVal == 0xffff) break;

		sortedList[talkCtr] = currVal;
		prevVal = currVal;
	}
	sortedList[talkCtr] = giveTalkIds[6];
	int numTalks = talkCtr;

	// Loop through the talk list

	for (talkCtr = 0; talkCtr < numTalks; ++talkCtr) {
		uint16 startOffset = sortedList[talkCtr];
		uint16 nextOffset = sortedList[talkCtr+1];

		header->recordId = startOffset;
		header->listOffset = offset;
		
		lureExe.seek(dataSegment + startOffset);
		responseOffset = lureExe.readWord();
		startOffset += 2;

		// Special handling for entry at 0d930h
		if (responseOffset == 0x8000) continue;

		// Calculate talk data size - if response is within record range,
		// use simple calculation size. Otherwise, read in full data until
		// end of record
		if ((responseOffset < startOffset) || (responseOffset >= nextOffset))
			size = nextOffset - startOffset;
		else
			size = responseOffset - startOffset;
		if ((size % 6) == 2) size -= 2;
		if ((size % 6) != 0) {
			printf("Failure reading talk data: size=%d\n", size);
			exit(1);
		}

		// Read in the list of talk entries
		lureExe.read(data + offset, size);
		offset += size;
		memset(data + offset, 0xff, 2);
		offset += 2;

		// Handle the response data
		header->responsesOffset = offset;

		// Scan through the list of record offsets and find the offset of
		// the following record. This is done because although the talk
		// records and responses are normally sequential, it can also
		// point into another record's talk responses

		nextOffset = 0;
		for (subentryCtr = 0; subentryCtr < numTalks; ++subentryCtr) {
			if ((responseOffset >= sortedList[subentryCtr]) &&
				(responseOffset < sortedList[subentryCtr+1])) {
				// Found a record				
				nextOffset = sortedList[subentryCtr+1];
				break;
			}
		}
		if (nextOffset < responseOffset) {
			printf("Failure reading talk data: no response found\n");
			exit(1);
		}
		
		size = nextOffset - responseOffset;
		if ((size % 6) != 0) size -= (size % 6);

		if ((size % 6) != 0) {
			printf("Failure reading talk data: newSize=%d\n", size);
			exit(1);
		}

		lureExe.read(data + offset, size);
		offset += size;
		WRITE_LE_UINT16(data + offset, 0xffff);
		offset += 2;

		++header;
	}

	header->recordId = TO_LE_16(0xffff);
	totalSize = offset;
	free(sortedList);
}

void read_room_pathfinding_data(byte *&data, uint16 &totalSize) {
	uint16 dataStart = 0x984A;
	if (language == IT_ITA) dataStart = 0x990A;
	lureExe.seek(dataSegment + dataStart);
	
	totalSize = PATHFIND_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

void read_room_exit_coordinate_data(byte *&data, uint16 &totalSize) 
{
	// Read in the exit coordinates list	
	int roomNum, entryNum;
	uint16 x, y;
	RoomExitCoordinateEntryInputResource dataIn;

	totalSize = EXIT_COORDINATES_NUM_ROOMS * sizeof(RoomExitCoordinateEntryOutputResource) + 2; 
	data = (byte *) malloc(totalSize);
	lureExe.seek(dataSegment + EXIT_COORDINATES_OFFSET);
	WRITE_LE_UINT16(data + totalSize - 2, 0xffff);

	// Post process the list to adjust data
	RoomExitCoordinateEntryOutputResource *rec = (RoomExitCoordinateEntryOutputResource *) data;
	for (roomNum = 1; roomNum <= EXIT_COORDINATES_NUM_ROOMS; ++roomNum, ++rec) {
		lureExe.read(&dataIn, sizeof(RoomExitCoordinateEntryInputResource));

		for (entryNum = 0; entryNum < ROOM_EXIT_COORDINATES_NUM_ENTRIES; ++entryNum) {
			x = FROM_LE_16(dataIn.entries[entryNum].x);
			y = FROM_LE_16(dataIn.entries[entryNum].y);
			if ((x != 0) || (y != 0)) {
				x -= 0x80;
				y = ((y & 0xfff) - 0x80) | (y & 0xf000);
			}

			RoomExitCoordinateResource *p = &rec->entries[entryNum];
			p->x = TO_LE_16(x);
			p->y = TO_LE_16(y);
			p->roomNumber = dataIn.entries[entryNum].roomNumber;
		}

		for (entryNum = 0; entryNum < ROOM_EXIT_COORDINATES_ENTRY_NUM_ROOMS; ++entryNum) {
			rec->roomIndex[entryNum] = TO_LE_16(FROM_LE_16(dataIn.roomIndex[entryNum]) / 6);
		}

		// WORKAROUND: Bugfix for the original game data to get to room #27 via rooms #10 or #11 
		if ((roomNum == 10) || (roomNum == 11))
			rec->roomIndex[26] = TO_LE_16(1);
	}
}

void read_room_exit_hotspots_data(byte *&data, uint16 &totalSize) {
	totalSize = 0;
	data = (byte *) malloc(MAX_DATA_SIZE);

	RoomExitIndexedHotspotResource *rec = (RoomExitIndexedHotspotResource *) data;
	
	uint16 dataStart = 0x2E57;
	if (language == IT_ITA) dataStart = 0x2E66;
	lureExe.seek(dataSegment + dataStart);
	
	lureExe.read(rec, sizeof(RoomExitIndexedHotspotResource));
	while (FROM_LE_16(rec->roomNumber) != 0) {
		++rec;
		totalSize += sizeof(RoomExitIndexedHotspotResource);
		lureExe.read(rec, sizeof(RoomExitIndexedHotspotResource));
	}

	WRITE_LE_UINT16(rec, 0xffff);
	totalSize += sizeof(uint16);
}

void save_fight_segment(byte *&data, uint16 &totalSize) {
	uint16 fightSegment = (uint16) 0x1C400;
	if (language == IT_ITA) fightSegment = (uint16) 0x1c520;
	lureExe.seek(fightSegment);
	
	totalSize = FIGHT_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

#define NUM_TEXT_ENTRIES 49
const char *englishTextStrings[NUM_TEXT_ENTRIES] = {
	"Get", NULL, "Push", "Pull", "Operate", "Open", "Close", "Lock", "Unlock", "Use", 
	"Give", "Talk to", "Tell", "Buy", "Look", "Look at", "Look through", "Ask", NULL, 
	"Drink", "Status", "Go to", "Return", "Bribe", "Examine",
	"Credits", "Restart game", "Save game", "Restore game", "Quit", "Fast Text", "Slow Text", 
	"Sound on", "Sound off", "(nothing)", " for ", " to ", " on ", "and then", "finish",
	"Are you sure (y/n)?",
	"a ", "the ", "a ", "a ", "an ", "an ", "an ", "an "
};

const char *italianTextStrings[NUM_TEXT_ENTRIES] = {
	"Prendi", NULL,	"Spingi", "Tira", "Aziona", "Apri", "Chiudi", "Blocca",
	"Sblocca", "Usa", "Dai", "Parla con", "Ordina a", "Buy", "Guarda", "Osserva",
	"Guarda tra", "Chiedi", NULL,  "Bevi", "Stato", "Vai a", "Ritorna",
	"Corrompi", "Esamina",
	"Inform", "Reavvia", "Salva gioco", "Ripristina", "Abbandona", "Testo lento", 
	"Testo veloce",  "Sonoro acceso", "Sonoro spento", 
	"(niente)", " per ", " a ", " su ", 
	"e poi", "finito", "Sei sicuro (s/n)?",
	NULL, "l' ", "la ", NULL, "le ", "i ", "il ", NULL 
};

void save_text_strings(byte *&data, uint16 &totalSize) {
	int index;

	const char **textStrings = &englishTextStrings[0];
	if (language == IT_ITA) textStrings = &italianTextStrings[0];

	// Calculate the total needed space
	totalSize = sizeof(uint16);
	for (index = 0; index < NUM_TEXT_ENTRIES; ++index) {
		if (textStrings[index] != NULL) 
			totalSize += strlen(textStrings[index]);
		++totalSize;
	}

	// Duplicate the text strings list into a data buffer
	data = (byte *) malloc(totalSize);
	*((uint16 *) data) = TO_LE_16(NUM_TEXT_ENTRIES);
	char *p = (char *) data + sizeof(uint16);

	for (index = 0; index < NUM_TEXT_ENTRIES; ++index) {
		if (textStrings[index] == NULL)
			*p++ = '\0';
		else {
			strcpy(p, textStrings[index]);
			p += strlen(p) + 1;
		}
	}
}

void save_sound_desc_data(byte *&data, uint16 &totalSize) {
	uint16 dataStart = 0x5671;
	if (language == IT_ITA) dataStart = 0x5731;
	lureExe.seek(dataSegment + dataStart);
	
	totalSize = SOUND_DESCS_SIZE;
	data = (byte *) malloc(totalSize);
	lureExe.read(data, totalSize);
}

struct DecoderEntry {
	const char *sequence;
	char character;
};

const DecoderEntry englishDecoders[] = {
	{"00", ' '}, {"0100", 'e'}, {"0101", 'o'}, {"0110", 't'}, {"01110", 'a'}, 
	{"01111", 'n'}, {"1000", 's'}, {"1001", 'i'}, {"1010", 'r'}, {"10110", 'h'}, 
	{"101110", 'u'}, {"1011110", 'l'}, {"1011111", 'd'}, {"11000", 'y'}, 
	{"110010", 'g'}, {"110011", '\0'}, {"110100", 'w'}, {"110101", 'c'}, 
	{"110110", 'f'}, {"1101110", '.'}, {"1101111", 'm'}, {"111000", 'p'}, 
	{"111001", 'b'}, {"1110100", ','}, {"1110101", 'k'}, {"1110110", '\''}, 
	{"11101110", 'I'}, {"11101111", 'v'}, {"1111000", '!'}, {"1111001", '\xb4'}, 
	{"11110100", 'T'}, {"11110101", '\xb5'}, {"11110110", '?'}, {"111101110", '\xb2'}, 
	{"111101111", '\xb3'}, {"11111000", 'W'}, {"111110010", 'H'}, {"111110011", 'A'}, 
	{"111110100", '\xb1'}, {"111110101", 'S'}, {"111110110", 'Y'}, {"1111101110", 'G'}, 
	{"11111011110", 'M'}, {"11111011111", 'N'}, {"111111000", 'O'}, {"1111110010", 'E'}, 
	{"1111110011", 'L'}, {"1111110100", '-'}, {"1111110101", 'R'}, {"1111110110", 'B'}, 
	{"11111101110", 'D'}, {"11111101111", '\xa6'}, {"1111111000", 'C'}, 
	{"11111110010", 'x'}, {"11111110011", 'j'}, {"1111111010", '\xac'}, 
	{"11111110110", '\xa3'}, {"111111101110", 'P'}, {"111111101111", 'U'}, 
	{"11111111000", 'q'}, {"11111111001", '\xad'}, {"111111110100", 'F'}, 
	{"111111110101", '1'}, {"111111110110", '\xaf'}, {"1111111101110", ';'}, 
	{"1111111101111", 'z'}, {"111111111000", '\xa5'}, {"1111111110010", '2'}, 
	{"1111111110011", '\xb0'}, {"111111111010", 'K'}, {"1111111110110", '%'}, 
	{"11111111101110", '\xa2'}, {"11111111101111", '5'}, {"1111111111000", ':'}, 
	{"1111111111001", 'J'}, {"1111111111010", 'V'}, {"11111111110110", '6'}, 
	{"11111111110111", '3'}, {"1111111111100", '\xab'}, {"11111111111010", '\xae'}, 
	{"111111111110110", '0'}, {"111111111110111", '4'}, {"11111111111100", '7'}, 
	{"111111111111010", '9'}, {"111111111111011", '"'}, {"111111111111100", '8'}, 
	{"111111111111101", '\xa7'}, {"1111111111111100", '/'}, {"1111111111111101", 'Q'}, 
	{"11111111111111100", '\xa8'}, {"11111111111111101", '('}, {"111111111111111100", ')'}, 
	{"111111111111111101", '\x99'}, {"11111111111111111", '\xa9'}, 
	{NULL, '\0'}
};

const DecoderEntry italianDecoders[] = {
	{"00", ' '}, {"010", (char) 0x69},	{"0110", (char) 0x6F}, {"01110", (char) 0x61}, {"01111", (char) 0x65},
	{"1000", (char) 0x72}, {"1001", (char) 0x6E}, {"1010", (char) 0x74}, {"10110", (char) 0x73}, {"101110", (char) 0x6C},
	{"101111", (char) 0x63}, {"11000", (char) 0x75}, {"110010", (char) 0x70}, {"110011", (char) 0x64}, {"110100", 0},
	{"110101", (char) 0x6D}, {"110110", (char) 0x67}, {"1101110", (char) 0x2E}, {"1101111", (char) 0x76},
	{"111000", (char) 0x68}, {"1110010", (char) 0x2C}, {"1110011", (char) 0x62}, {"1110100", (char) 0x66},
	{"1110101", (char) 0x21}, {"1110110", (char) 0xB5}, {"11101110", (char) 0xB1}, {"111011110", (char) 0xB3},
	{"111011111", (char) 0x7A}, {"1111000", (char) 0xB4}, {"11110010", (char) 0x27}, {"111100110", (char) 0x4E},
	{"111100111", (char) 0x4C}, {"11110100", (char) 0x3F}, {"111101010", (char) 0x85}, {"111101011", (char) 0x53},
	{"11110110", (char) 0x43}, {"111101110", (char) 0x4D}, {"1111011110", (char) 0xAC}, {"1111011111", (char) 0x49},
	{"11111000", (char) 0x45}, {"111110010", (char) 0x41}, {"1111100110", (char) 0x54}, {"1111100111", (char) 0xB2},
	{"111110100", (char) 0x71}, {"111110101", (char) 0x4F}, {"111110110", (char) 0x47}, {"1111101110", (char) 0xAB},
	{"11111011110", (char) 0x50}, {"11111011111", (char) 0x44}, {"111111000", (char) 0x81},
	{"1111110010", (char) 0x55}, {"11111100110", (char) 0xAE}, {"11111100111", (char) 0x52},
	{"1111110100", (char) 0xA6}, {"1111110101", (char) 0x56}, {"1111110110", (char) 0xA8},
	{"11111101110", (char) 0x42}, {"111111011110", (char) 0x51}, {"111111011111", (char) 0xB0},
	{"1111111000", (char) 0x95}, {"11111110010", (char) 0x48}, {"11111110011", (char) 0x2D},
	{"11111110100", (char) 0xA9}, {"11111110101", (char) 0x8A}, {"11111110110", (char) 0xA3},
	{"111111101110", (char) 0x46}, {"111111101111", (char) 0xA7}, {"11111111000", (char) 0x8D},
	{"11111111001", (char) 0x77}, {"11111111010", (char) 0x79}, {"111111110110", (char) 0x7F},
	{"1111111101110", (char) 0x6B}, {"1111111101111", (char) 0x31}, {"111111111000", (char) 0x3B},
	{"111111111001", (char) 0xA5}, {"111111111010", (char) 0x57}, {"1111111110110", (char) 0x32},
	{"11111111101110", (char) 0xAF}, {"11111111101111", (char) 0x35}, {"1111111111000", (char) 0xA2},
	{"1111111111001", (char) 0xAD}, {"1111111111010", (char) 0x25}, {"11111111110110", (char) 0x36},
	{"11111111110111", (char) 0x3A}, {"1111111111100", (char) 0x5A}, {"11111111111010", (char) 0x33},
	{"11111111111011", (char) 0x30}, {"11111111111100", (char) 0x34}, {"111111111111010", (char) 0x39},
	{"111111111111011", (char) 0x37}, {"111111111111100", (char) 0x38}, {"111111111111101", (char) 0x2F},
	{"1111111111111100", (char) 0x4B}, {"1111111111111101", (char) 0x22}, {"111111111111111000", (char) 0x09},
	{"111111111111111001", (char) 0x28}, {"11111111111111101", (char) 0x29}, {"111111111111111100", (char) 0x4A},
	{"111111111111111101", (char) 0x59}, {"11111111111111111", (char) 0x78},
	{NULL, '\0'}
};

void save_string_decoder_data(byte *&data, uint16 &totalSize) {
	const DecoderEntry *list = &englishDecoders[0];
	if (language == IT_ITA) list = &italianDecoders[0];

	totalSize = 1;
	const DecoderEntry *pSrc = list;
	while (pSrc->sequence != NULL) {
		totalSize += strlen(pSrc->sequence) + 2;
		++pSrc;
	}

	data = (byte *) malloc(totalSize);
	char *pDest = (char *)data;

	pSrc = list;
	while (pSrc->sequence != NULL) {
		*pDest++ = pSrc->character;
		strcpy(pDest, pSrc->sequence);
		pDest += strlen(pSrc->sequence) + 1;

		++pSrc;
	}

	*pDest = (char) 0xff;
}

void getEntry(uint8 entryIndex, uint16 &resourceId, byte *&data, uint16 &size) {
	resourceId = 0x3f01 + entryIndex;
	printf("Get resource #%d\n", entryIndex);
	switch (entryIndex) {
	case 0:
		// Copy the default palette to file
		read_basic_palette(data, size);
		break;

	case 1:
		// Copy the replacement palette fragments to file
		read_replacement_palette(data, size);
		break;

	case 2:
		// Copy the dialog segment data into the new vga file
		read_dialog_data(data, size);
		break;

	case 3:
		// Copy the talk dialog segment data into the new vga file
		read_talk_dialog_data(data, size);
		break;

	case 4:
		// Get the room info data
		read_room_data(data, size);
		break;

	case 5:
		// Get the action sequence set for NPC characters
		read_action_sequence(data, size);
		break;

	case 6:
		// Get the hotspot info data
		read_hotspot_data(data, size);
		break;

	case 7:
		// Get the hotspot override info data
		read_hotspot_override_data(data, size);
		break;

	case 8:
		// Get the list of room exits
		read_room_exits(data, size);
		break;

	case 9:
		// Get the list of room exit joins
		read_room_exit_joins(data, size);
		break;

	case 10:
		// Get the hotspot animation record data
		read_anim_data(data, size);
		break;

	case 11:
		// Get the script segment data
		read_script_data(data, size);
		break;

	case 12:
		// Get the second script segment data
		read_script2_data(data, size);
		break;

	case 13:
		// Get a list of hotspot script offsets
		read_hotspot_script_offsets(data, size);
		break;

	case 14:
		// Get the messages segment 
		read_messages_segment(data, size);
		break;

	case 15:
		// Get the actions list
		read_actions_list(data, size);
		break;

	case 16:
		// Get the talk header information
		read_talk_headers(data, size);
		break;

	case 17:
		// Get the talk data
		read_talk_data(data, size);
		break;

	case 18:
		// Get the pathfinding data
		read_room_pathfinding_data(data, size);
		break;

	case 19:
		// Get the room exit coordinate list
		read_room_exit_coordinate_data(data, size);
		break;

	case 20:
		// Read the room exit hotspot list
		read_room_exit_hotspots_data(data, size);
		break;

	case 21:
		// Save the fight segment data
		save_fight_segment(data, size);
		break;

	case 22:
		// Set up the list of text strings used by the game
		save_text_strings(data, size);
		break;

	case 23:
		// Save the sound header desc data
		save_sound_desc_data(data, size);
		break;

	case 24:
		// Save the decoder sequence list
		save_string_decoder_data(data, size);
		break;
		
	default:
		data = NULL;
		size = 0;
		resourceId = 0xffff;
		break;
	}
}

void openOutputFile(const char *outFilename) {
	outputFile.open(outFilename, kFileWriteMode);

	// Write header
	outputFile.write("lure", 4);
	outputFile.writeWord(0);

	outputFile.seek(0xBF * 8);
	FileEntry fileVersion;
	memset(&fileVersion, 0xff, sizeof(FileEntry));
	fileVersion.unused = VERSION_MAJOR;
	fileVersion.sizeExtension = VERSION_MINOR;
	outputFile.write(&fileVersion, sizeof(FileEntry));
}

void closeOutputFile() {
	outputFile.seek(6 + 5 * langIndex);
	outputFile.writeByte(0xff);
	outputFile.close();
}

void createFile(const char *outFilename) {
	FileEntry rec;
	uint32 startOffset, numBytes;
	uint32 outputStart;
	uint16 resourceId;
	uint16 resourceSize;
	byte *resourceData;
	bool resourceFlag;
	byte tempBuffer[32];

	memset(tempBuffer, 0, 32);

	// Reset list counters
	outputStart = ((outputFile.pos() + 0xff) / 0x100) * 0x100;
	startOffset = 0x600;
	animIndex = 0;
	actionIndex = 0;
	talkOffsetIndex = 0;

	// Write out the position of the next language set
	outputFile.seek(6 + 5 * (langIndex - 1));
	outputFile.writeByte(language);
	outputFile.writeLong(outputStart);

	// Write out start header
	outputFile.seek(outputStart);
	outputFile.write("heywow", 6);
	outputFile.writeWord(0);

	resourceFlag = true;
	for (int resIndex=0; resIndex < 0xBE; ++resIndex) {
		resourceData = NULL;

		// Get next data entry
		if (resourceFlag)
			// Get resource details
			getEntry(resIndex, resourceId, resourceData, resourceSize); 

		// Write out the next header entry
		outputFile.seek(outputStart + (resIndex + 1) * 8);
		if (resourceSize == 0) {
			// Unused entry
			memset(&rec, 0xff, sizeof(FileEntry));
			resourceFlag = false;
		} else {
			rec.id = TO_LE_16(resourceId);
			rec.offset = TO_LE_16(startOffset >> 5);
			rec.sizeExtension = (uint8) ((resourceSize >> 16) & 0xff);
			rec.size = TO_LE_16(resourceSize & 0xffff);
			rec.unused = 0xff;
		}

		outputFile.write(&rec, sizeof(FileEntry));

		// Write out the resource
		if (resourceFlag) {
			outputFile.seek(outputStart + startOffset);
			outputFile.write(resourceData, resourceSize);
			startOffset += resourceSize;
			free(resourceData);		// Free the data block

			// Write out enough bytes to move to the next 32 byte boundary
			numBytes = 0x20 * ((startOffset + 0x1f) / 0x20) - startOffset;
			if (numBytes != 0) 
			{
				outputFile.write(tempBuffer, numBytes);
				startOffset += numBytes;
			}
		}
	}

	// Move to the end of the written file
	outputFile.seek(0, SEEK_END);
}

// validate_executable
// Validates that the correct executable is being used to generate the
// resource file. Eventually the resource file creator will need to work
// with the other language executables, but for now just make 

bool validate_executable() {
	uint32 sumTotal = 0;
	byte buffer[NUM_BYTES_VALIDATE];
	lureExe.read(buffer, NUM_BYTES_VALIDATE);
	for (int ctr = 0; ctr < NUM_BYTES_VALIDATE; ++ctr) 
		sumTotal += buffer[ctr];

	if (sumTotal == ENGLISH_FILE_CHECKSUM) {
		language = EN_ANY;
		dataSegment = 0xAC50;
		printf("Detected English version\n");
	} else if (sumTotal == ITALIAN_FILE_CHECKSUM) {
		language = IT_ITA;
		dataSegment = 0xACB0;
		printf("Detected Italian version\n");
	} else {
		printf("Lure executable version not recognised. Checksum = %xh\n", sumTotal);
		return false;
	}

	// Double-check that the given language has not already been done
	for (int index = 0; index < langIndex; ++index) {
		if (processedLanguages[index] == language) {
			printf("Identical language executable listed multiple times\n");
			return false;
		}
	}

	processedLanguages[langIndex++] = language;
	return true;
}


int main(int argc, char *argv[]) {
	const char /**inFilename,*/ *outFilename;

	if (argc == 1) {
		printf("Format: %s output_filename [lureExecutable ..]\n", argv[0]);
		exit(0);
	}

	openOutputFile(argv[1]);

	for (int argi = 2; argi < argc; ++argi) {
		if (!lureExe.open(argv[argi])) 
			printf("Could not open file: %s\n", argv[argi]);
		else {
			if (validate_executable()) 
				createFile(outFilename);	
			lureExe.close();
		}
	}

	closeOutputFile();
}
