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
 * This is a utility for extracting needed resource data from the Lure of the Temptress
 * lure.exe file into a new file lure.dat - this file is required for the ScummVM
 * Lure of the Temptress module to work properly
 *
 * TODO: 
 * Some of the field values, such as hotspot tick proc offsets, will vary with
 * different language versions. These will need to be remapped to a language independant
 * value once I've fully implemented the English version.
 * Some areas of the data segment are still to be decoded
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "create_lure_dat.h"

File lure_exe;

uint16 animOffsets[MAX_NUM_ANIM_RECORDS];
int animIndex = 0;
uint16 actionOffsets[MAX_NUM_ACTION_RECORDS];
int actionIndex = 0;


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
	lure_exe.seek(PALETTE_OFFSET);
	data = (byte *) malloc(PALETTE_SIZE);
	lure_exe.read(data, totalSize);
}

#define ALT_PALETTE_1 0x1757
#define ALT_PALETTE_1_SIZE 180
#define ALT_PALETTE_2 0x180B
#define ALT_PALETTE_2_SIZE 24

void read_replacement_palette(byte *&data, uint16 &totalSize) {
	totalSize = ALT_PALETTE_1_SIZE + ALT_PALETTE_2_SIZE;
	data = (byte *) malloc(totalSize);

	lure_exe.seek(DATA_SEGMENT + ALT_PALETTE_1);
	lure_exe.read(data, ALT_PALETTE_1_SIZE);
	lure_exe.seek(DATA_SEGMENT + ALT_PALETTE_2);
	lure_exe.read(data + ALT_PALETTE_1_SIZE, ALT_PALETTE_2_SIZE);
}

void read_dialog_data(byte *&data, uint16 &totalSize) {
	totalSize = DIALOG_SIZE;
	lure_exe.seek(DIALOG_OFFSET);
	data = (byte *) malloc(DIALOG_SIZE);
	lure_exe.read(data, totalSize);
}

void read_talk_dialog_data(byte *&data, uint16 &totalSize) {
	totalSize = TALK_DIALOG_SIZE;
	lure_exe.seek(TALK_DIALOG_OFFSET, SEEK_SET);
	data = (byte *) malloc(TALK_DIALOG_SIZE);
	lure_exe.read(data, totalSize);
}

void read_room_data(byte *&data, uint16 &totalSize) 
{
	data = (byte *) malloc(MAX_DATA_SIZE);
	memset(data, 0, MAX_DATA_SIZE);

	uint16 *offsetPtr = (uint16 *) data;
	uint16 offset = (ROOM_NUM_ENTRIES + 1) * sizeof(uint16);
	uint16 pixelOffset;
	RoomResource buffer;
	int outputIndex = 0;
	RoomHeaderEntry headerEntry;
	RoomRectIn bounds;

	for (int index = 0; index < ROOM_NUM_ENTRIES; ++index) {

		lure_exe.seek(DATA_SEGMENT + ROOM_TABLE + index * 9);
		lure_exe.read(&headerEntry, sizeof(RoomHeaderEntry));

		if ((FROM_LE_16(headerEntry.offset) != 0) && 
			(FROM_LE_16(headerEntry.offset) != 0xffff) &&
			(FROM_LE_16(headerEntry.roomNumber) != 0)) {
			// Store offset of room entry
			*offsetPtr++ = TO_LE_16(offset);

			// Copy over basic room details
			lure_exe.seek(DATA_SEGMENT + headerEntry.offset);
			lure_exe.read(&buffer, sizeof(RoomResource));
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
			for (;;) {
				RoomResourceExit1 *p = (RoomResourceExit1 *) (data + offset);
				lure_exe.read(p, sizeof(RoomResourceExit1));
				if (FROM_LE_16(p->xs) == 0xffff) break;

				rec->numExits = TO_LE_16(FROM_LE_16(rec->numExits) + 1);
				p->xs = TO_LE_16(FROM_LE_16(p->xs) - 0x80);
				p->ys = TO_LE_16(FROM_LE_16(p->ys) - 0x80);
				p->xe = TO_LE_16(FROM_LE_16(p->xe) - 0x80);
				p->ye = TO_LE_16(FROM_LE_16(p->ye) - 0x80);
				offset += sizeof(RoomResourceExit1);
				RoomResourceExit2 *p2 = (RoomResourceExit2 *) (data + offset);

				if (FROM_LE_16(p->sequenceOffset) == 0xffff) {
					lure_exe.read(p2, sizeof(RoomResourceExit2));
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
			lure_exe.seek(DATA_SEGMENT + WALK_AREAS_OFFSET + 
				buffer.walkBoundsIndex * sizeof(RoomRectIn));
			lure_exe.read(&bounds, sizeof(RoomRectIn));
			rec->walkBounds.xs = TO_LE_16(FROM_LE_16(bounds.xs) - 0x80);
			rec->walkBounds.xe = TO_LE_16(FROM_LE_16(bounds.xe) - 0x80);
			rec->walkBounds.ys = TO_LE_16(FROM_LE_16(bounds.ys) - 0x80);
			rec->walkBounds.ye = TO_LE_16(FROM_LE_16(bounds.ye) - 0x80);

			// If the room has a default pixel blocks list, add the references
			if (buffer.pixelListOffset != 0) {
				lure_exe.seek(DATA_SEGMENT + FROM_LE_16(buffer.pixelListOffset));
				pixelOffset = lure_exe.readWord();
				while (pixelOffset != 0) {
					add_anim_record(pixelOffset);
					pixelOffset = lure_exe.readWord();
				}
			}
		}
	}

	WRITE_LE_UINT16(offsetPtr, 0xffff);
	totalSize = offset;
}

void read_hotspot_data(byte *&data, uint16 &totalSize) 
{
	uint16 offsets[4] = {0x5d98, 0x5eb8, 0x623e, 0x63b1};
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

	// Allocate enough space for output hotspot list
	data = (byte *) malloc(MAX_HOTSPOTS * sizeof(HotspotResourceOutput));

	// Determine number of hotspot walk to entries
	lure_exe.seek(DATA_SEGMENT + HOTSPOT_WALK_TO_OFFSET);
	do {
		++walkNumEntries;
		lure_exe.read(&rec, sizeof(HotspotWalkToRecord));
	} while (TO_LE_16(rec.hotspotId) != 0);
	--walkNumEntries;

	dataSize = walkNumEntries * sizeof(HotspotWalkToRecord);
	walkList = (HotspotWalkToRecord *) malloc(dataSize);
	lure_exe.seek(DATA_SEGMENT + HOTSPOT_WALK_TO_OFFSET);
	lure_exe.read(walkList, sizeof(HotspotWalkToRecord) * walkNumEntries);

	// Main code for creating the hotspot list

	r = (HotspotResourceOutput *) data;
	numEntries = 0;

	for (int tableNum = 0; tableNum < 4; ++tableNum) {
		uint16 hotspotIndex = 0;
		for (;;) {
			lure_exe.seek(DATA_SEGMENT + offsets[tableNum] +  hotspotIndex * 9);
			lure_exe.read(&entryHeader, sizeof(HotspotHeaderEntry));
			if (FROM_LE_16(entryHeader.offset) == 0xffff) break;

			memset(r, 0, sizeof(HotspotResourceOutput));
			r->hotspotId = TO_LE_16(startId[tableNum] + hotspotIndex);
			r->nameId = entryHeader.resourceId;
			r->descId = entryHeader.descId; //TO_LE_16(FROM_LE_16(entryHeader.descId) & 0x1fff);
			r->descId2 = entryHeader.descId2; //TO_LE_16(FROM_LE_16(entryHeader.descId2) & 0x1fff);
			r->hdrFlags = entryHeader.hdrFlags;

			// Get the hotspot data
			lure_exe.seek(DATA_SEGMENT + entryHeader.offset);
			lure_exe.read(&entry, sizeof(HotspotResource));
			
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
			r->scriptLoadFlag = entry.scriptLoadFlag;
			r->loadOffset = entry.loadOffset;
			r->colourOffset = entry.colourOffset;
			r->hotspotScriptOffset = entry.hotspotScriptOffset;
			r->talkScriptOffset = entry.talkScriptOffset;
			r->tickProcOffset = entry.tickProcOffset;
			r->flags = entry.flags;

			// Special check for the tinderbox hotspot to set it's room number correctly - the original 
			// game used this as a backup against people trying to hack the copy protection
			if (startId[tableNum] + hotspotIndex == 0x271C)
				r->roomNumber = TO_LE_16(28);

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
				lure_exe.seek(DATA_SEGMENT + entryHeader.offset + 0x63);
				lure_exe.read(&action, sizeof(CurrentActionInput));

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
	lure_exe.seek(DATA_SEGMENT + HOTSPOT_OVERRIDE_OFFSET);
	int numOverrides = 0;
	HotspotOverride rec;

	// Determine number of hotspot overrides
	do {
		++numOverrides;
		lure_exe.read(&rec, sizeof(HotspotOverride));
	} while (FROM_LE_16(rec.hotspotId) != 0);
	--numOverrides;

	// Prepare output data and read in all entries at once
	totalSize = numOverrides * sizeof(HotspotOverride) + 2;
	data = (byte *) malloc(totalSize);
	lure_exe.seek(DATA_SEGMENT + HOTSPOT_OVERRIDE_OFFSET);
	lure_exe.read(data, totalSize - 2);
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

	lure_exe.seek(DATA_SEGMENT + ROOM_EXITS_OFFSET);
	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr)
		offsets[roomCtr] = lure_exe.readWord();

	// First loop to find total of room exit records there are
	for (roomCtr = 0; roomCtr < NUM_ROOM_EXITS; ++roomCtr) {
		numEntries[roomCtr] = 0;
		if (offsets[roomCtr] == 0) continue;

		// Get number of exits for the room
		lure_exe.seek(DATA_SEGMENT + offsets[roomCtr]);
		lure_exe.read(&rec, sizeof(RoomExitHotspotRecord));
		while (FROM_LE_16(rec.xs) != 0) {
			totalSize += sizeof(RoomExitHotspotOutputRecord);
			numEntries[roomCtr]++;
			lure_exe.read(&rec, sizeof(RoomExitHotspotRecord));
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

			lure_exe.seek(DATA_SEGMENT + offsets[roomCtr]);

			for (entryCtr = 0; entryCtr < numEntries[roomCtr]; ++entryCtr, ++destP) {
				lure_exe.read(&rec, sizeof(RoomExitHotspotRecord));

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
	lure_exe.seek(DATA_SEGMENT + ROOM_EXIT_JOINS_OFFSET);
	int numRecords = 0;
	uint32 unused;

	lure_exe.seek(DATA_SEGMENT + ROOM_EXIT_JOINS_OFFSET);
	do {
		lure_exe.read(&rec, sizeof(RoomExitHotspotJoinRecord));
		lure_exe.read(&unused, sizeof(uint32));
		++numRecords;
	} while (FROM_LE_16(rec.hotspot1Id) != 0);
	--numRecords;

	// Allocate the data and read in all the records
	totalSize = (numRecords * sizeof(RoomExitHotspotJoinRecord)) + 2;
	data = (byte *) malloc(totalSize);
	lure_exe.seek(DATA_SEGMENT + ROOM_EXIT_JOINS_OFFSET);
	
	p = (RoomExitHotspotJoinRecord *) data;
	for (int recordCtr = 0; recordCtr < numRecords; ++recordCtr)
	{
		lure_exe.read(p, sizeof(RoomExitHotspotJoinRecord));
		lure_exe.read(&unused, sizeof(uint32));
		++p;
	}
	WRITE_LE_UINT16(p, 0xffff);
}

// This next method reads in the animation and movement data. At the moment I
// figure out which animations have valid movement record sets by finding
// animations whose four direction offsets are near each other. There's 
// probably a better method than this, but it'll do for now

void read_anim_data(byte *&data, uint16 &totalSize) {
	// Add special pixel records
	add_anim_record(0x55C0);		// Player midswing animation
	add_anim_record(0x55C9);		// Player mid-level defend
	add_anim_record(0x55D2);		// Player high-level strike
	add_anim_record(0x55DB);		// Player high-level defend
	add_anim_record(0x55E4);		// Player low-level strike
	add_anim_record(0x55ED);		// Player low-level defend
	add_anim_record(0x55F6);		// Player fight animation
	add_anim_record(0x55FF);		// Pig fight animation
	add_anim_record(0x5611);		// Player mid-level strike
	add_anim_record(0x5623);		// Pig fight animation
	add_anim_record(0x562C);		// Misc fight animation
	add_anim_record(0x5635);		// Pig fight animation
	add_anim_record(0x563E);		// Player recoiling from hit
	add_anim_record(0x5647);		// Pig recoiling from hit
	add_anim_record(0x5650);		// Pig dies
	add_anim_record(0x5915);		// Blacksmith hammering
	add_anim_record(0x59ED);		// Ewan's alternate animation
	add_anim_record(0x59FF);		// Dragon breathing fire
	add_anim_record(0x5A08);		// Dragon breathing fire 2
	add_anim_record(0x5A11);		// Dragon breathing fire 3
	add_anim_record(0x5A1A);		// Player turning winch in room #48
	add_anim_record(0x5A59);		// Player pulling lever in room #48
	add_anim_record(0x5A62);		// Minnow pulling lever in room #48
	add_anim_record(0x5AAA);		// Goewin mixing potion
	add_anim_record(0x5C95);
	add_anim_record(0x5CAA);		// Selena animation
	add_anim_record(0x5CE9);		// Blacksmith in bar?
	add_anim_record(0x5D28);		// Goewin animation

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
		lure_exe.seek(DATA_SEGMENT + animOffsets[ctr]);
		lure_exe.read(&inRec, sizeof(AnimRecord));

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
					lure_exe.seek(DATA_SEGMENT + offset);
					lure_exe.read(&move, sizeof(MovementRecord));

					while (FROM_LE_16(move.frameNumber) != 0xffff) {
						movementSize += sizeof(MovementRecord);
						lure_exe.read(&move, sizeof(MovementRecord));
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
		lure_exe.seek(DATA_SEGMENT + animOffsets[ctr]);
		lure_exe.read(&inRec, sizeof(AnimRecord));

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

					lure_exe.seek(DATA_SEGMENT + offset);
					lure_exe.read(&move, sizeof(MovementRecord));
					destMove = (MovementRecord *) (data + moveOffset);

					while (FROM_LE_16(move.frameNumber) != 0xffff) {
						destMove->frameNumber = move.frameNumber;
						destMove->xChange = move.xChange;
						destMove->yChange = move.yChange;

						moveOffset += sizeof(MovementRecord);
						++destMove;
						lure_exe.read(&move, sizeof(MovementRecord));
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
	lure_exe.seek(SCRIPT_SEGMENT);
	
	totalSize = SCRIPT_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lure_exe.read(data, totalSize);
}

void read_script2_data(byte *&data, uint16 &totalSize) {
	lure_exe.seek(SCRIPT2_SEGMENT);
	
	totalSize = SCRIPT2_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lure_exe.read(data, totalSize);
}

void read_hotspot_script_offsets(byte *&data, uint16 &totalSize) {
	lure_exe.seek(DATA_SEGMENT + HOTSPOT_SCRIPT_LIST);
	
	totalSize = HOTSPOT_SCRIPT_SIZE;
	data = (byte *) malloc(totalSize);
	lure_exe.read(data, totalSize);
}

void read_messages_segment(byte *&data, uint16 &totalSize) {
	lure_exe.seek(MESSAGES_SEGMENT);
	totalSize = MESSAGES_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lure_exe.read(data, totalSize);
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

		lure_exe.seek(DATA_SEGMENT + actionOffsets[ctr]);
		uint16 *numItems = (uint16 *) (data + offset);
		lure_exe.read(numItems, sizeof(uint16));
		offset += 2;

		if (READ_UINT16(numItems) > 0) {
			lure_exe.read(data + offset, READ_UINT16(numItems) * 3);
			offset += READ_UINT16(numItems) * 3;
		}
	}
	header->recordId = TO_LE_16(0xffff);
}

// Reads in the talk data 

#define TALK_OFFSET 0x505c
#define TALK_NUM_ENTRIES 28
#define MAX_TALK_LISTS 300

uint16 talkOffsets[MAX_TALK_LISTS];
int talkOffsetIndex = 0;

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

	lure_exe.seek(DATA_SEGMENT + TALK_OFFSET);
	lure_exe.read(&entries[0], sizeof(TalkEntry) * TALK_NUM_ENTRIES);

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

	for (int entryCtr = 0; entryCtr < TALK_NUM_ENTRIES; ++entryCtr) {
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
		lure_exe.seek(DATA_SEGMENT + startOffset);
		int size = nextOffset - startOffset;
		uint16 *talkOffset = (uint16 *) (data + offset);
		lure_exe.read(talkOffset, size);

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

uint16 giveTalkIds[6] = {0xCF5E, 0xCF14, 0xCF90, 0xCFAA, 0xCFD0, 0xCFF6};

void read_talk_data(byte *&data, uint16 &totalSize) {
	uint16 responseOffset;
	int talkCtr, subentryCtr;
	uint16 size;

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
	sortedList[talkCtr] = 0xf010;
	int numTalks = talkCtr;

	// Loop through the talk list

	for (talkCtr = 0; talkCtr < numTalks; ++talkCtr) {
		uint16 startOffset = sortedList[talkCtr];
		uint16 nextOffset = sortedList[talkCtr+1];

		header->recordId = startOffset;
		header->listOffset = offset;
		
		lure_exe.seek(DATA_SEGMENT + startOffset);
		responseOffset = lure_exe.readWord();
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
			printf("Failure reading talk data\n");
			exit(1);
		}

		// Read in the list of talk entries
		lure_exe.read(data + offset, size);
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
			printf("Failure reading talk data\n");
			exit(1);
		}
		
		size = nextOffset - responseOffset;
		if ((size % 6) != 0) size -= (size % 6);

		if ((size % 6) != 0) {
			printf("Failure reading talk data\n");
			exit(1);
		}

		lure_exe.read(data + offset, size);
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
	lure_exe.seek(DATA_SEGMENT + PATHFIND_OFFSET);
	
	totalSize = PATHFIND_SIZE;
	data = (byte *) malloc(totalSize);
	lure_exe.read(data, totalSize);
}

void read_room_exit_coordinate_data(byte *&data, uint16 &totalSize) 
{
	// Read in the exit coordinates list	
	int roomNum, entryNum;
	uint16 x, y;
	RoomExitCoordinateEntryInputResource dataIn;

	totalSize = EXIT_COORDINATES_NUM_ROOMS * sizeof(RoomExitCoordinateEntryOutputResource) + 2; 
	data = (byte *) malloc(totalSize);
	lure_exe.seek(DATA_SEGMENT + EXIT_COORDINATES_OFFSET);
	WRITE_LE_UINT16(data + totalSize - 2, 0xffff);

	// Post process the list to adjust data
	RoomExitCoordinateEntryOutputResource *rec = (RoomExitCoordinateEntryOutputResource *) data;
	for (roomNum = 1; roomNum <= EXIT_COORDINATES_NUM_ROOMS; ++roomNum, ++rec) {
		lure_exe.read(&dataIn, sizeof(RoomExitCoordinateEntryInputResource));

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
	lure_exe.seek(DATA_SEGMENT + EXIT_HOTSPOTS_OFFSET);

	lure_exe.read(rec, sizeof(RoomExitIndexedHotspotResource));
	while (FROM_LE_16(rec->roomNumber) != 0) {
		++rec;
		totalSize += sizeof(RoomExitIndexedHotspotResource);
		lure_exe.read(rec, sizeof(RoomExitIndexedHotspotResource));
	}

	WRITE_LE_UINT16(rec, 0xffff);
	totalSize += sizeof(uint16);
}

void save_fight_segment(byte *&data, uint16 &totalSize) {
	lure_exe.seek(FIGHT_SEGMENT);
	
	totalSize = FIGHT_SEGMENT_SIZE;
	data = (byte *) malloc(totalSize);
	lure_exe.read(data, totalSize);
}

#define NUM_TEXT_ENTRIES 40
const char *text_strings[NUM_TEXT_ENTRIES] = {
	"Get", NULL, "Push", "Pull", "Operate", "Open", "Close", "Lock", "Unlock", "Use", 
	"Give", "Talk to", "Tell", "Buy", "Look", "Look at", "Look through", "Ask", NULL, 
	"Drink", "Status", "Go to", "Return", "Bribe", "Examine",
	"Credits", "Restart game", "Save game", "Restore game", "Quit", "Fast Text", "Slow Text", 
	"Sound on", "Sound off", "(nothing)", " for ", " to ", " on ", "and then", "finish"};


void save_text_strings(byte *&data, uint16 &totalSize) {
	int index;

	// Calculate the total needed space
	totalSize = sizeof(uint16);
	for (index = 0; index < NUM_TEXT_ENTRIES; ++index) {
		if (text_strings[index] != NULL) 
			totalSize += strlen(text_strings[index]);
		++totalSize;
	}

	// Duplicate the text strings list into a data buffer
	data = (byte *) malloc(totalSize);
	*((uint16 *) data) = TO_LE_16(NUM_TEXT_ENTRIES);
	char *p = (char *) data + sizeof(uint16);

	for (index = 0; index < NUM_TEXT_ENTRIES; ++index) {
		if (text_strings[index] == NULL)
			*p++ = '\0';
		else {
			strcpy(p, text_strings[index]);
			p += strlen(p) + 1;
		}
	}
}

void getEntry(uint8 entryIndex, uint16 &resourceId, byte *&data, uint16 &size)
{
	resourceId = 0x3f01 + entryIndex;

	switch (entryIndex) 
	{
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

	default:
		data = NULL;
		size = 0;
		resourceId = 0xffff;
		break;
	}
}

void createFile(const char *outFilename)
{
	FileEntry rec;
	uint32 startOffset, numBytes;
	uint16 resourceId;
	uint16 resourceSize;
	byte *resourceData;
	bool resourceFlag;
	byte tempBuffer[32];

	File f;
	f.open(outFilename, kFileWriteMode);
	memset(tempBuffer, 0, 32);

	// Write header
	f.write("heywow", 6);
	f.writeWord(0);

	startOffset = 0x600;
	resourceFlag = true;
	for (int resIndex=0; resIndex<0xBE; ++resIndex)
	{
		resourceData = NULL;

		// Get next data entry
		if (resourceFlag)
			// Get resource details
			getEntry(resIndex, resourceId, resourceData, resourceSize); 

		// Write out the next header entry
		f.seek(8 + resIndex * 8);
		if (resourceSize == 0) 
		{
			// Unused entry
			memset(&rec, 0xff, sizeof(FileEntry));
			resourceFlag = false;
		}
		else
		{
			rec.id = TO_LE_16(resourceId);
			rec.offset = TO_LE_16(startOffset >> 5);
			rec.sizeExtension = (uint8) ((resourceSize >> 16) & 0xff);
			rec.size = TO_LE_16(resourceSize & 0xffff);
			rec.unused = 0xff;
		}

		f.write(&rec, sizeof(FileEntry));

		// Write out the resource
		if (resourceFlag)
		{
			f.seek(startOffset);
			f.write(resourceData, resourceSize);
			startOffset += resourceSize;
			free(resourceData);		// Free the data block

			// Write out enough bytes to move to the next 32 byte boundary
			numBytes = 0x20 * ((startOffset + 0x1f) / 0x20) - startOffset;
			if (numBytes != 0) 
			{
				f.write(tempBuffer, numBytes);
				startOffset += numBytes;
			}
		}
	}

	// Store file version in final file slot
	f.seek(0xBF * 8);
	FileEntry fileVersion;
	memset(&fileVersion, 0xff, sizeof(FileEntry));
	fileVersion.unused = VERSION_MAJOR;
	fileVersion.sizeExtension = VERSION_MINOR;
	f.write(&fileVersion, sizeof(FileEntry));

	f.close();
}

// validate_executable
// Validates that the correct executable is being used to generate the
// resource file. Eventually the resource file creator will need to work
// with the other language executables, but for now just make 

#define NUM_BYTES_VALIDATE 1024
#define FILE_CHECKSUM 64880

void validate_executable() {
	uint32 sumTotal = 0;
	byte buffer[NUM_BYTES_VALIDATE];
	lure_exe.read(buffer, NUM_BYTES_VALIDATE);
	for (int ctr = 0; ctr < NUM_BYTES_VALIDATE; ++ctr) 
		sumTotal += buffer[ctr];

	if (sumTotal != FILE_CHECKSUM) {
		printf("Lure executable not correct English version\n");
		exit(1);
	}
}


int main(int argc, char *argv[])
{
	const char *inFilename = (argc >= 2) ? argv[1] : "c:\\games\\lure\\lure.exe";
	const char *outFilename = (argc == 3) ? argv[2] : "c:\\games\\lure\\lure.dat";

	if (!lure_exe.open(inFilename))
	{
		if (argc == 1) 
			printf("Format: %s input_exe_filename output_filename\n", argv[0]);
		else
			printf("Could not open file: %s\n", inFilename);
	} 
	else 
	{
		validate_executable();
		createFile(outFilename);	
		lure_exe.close();
	}
}
