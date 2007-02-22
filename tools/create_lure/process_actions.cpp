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

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "create_lure_dat.h"

enum Action {
	GET = 1, PUSH = 3, PULL = 4, OPERATE = 5, OPEN = 6,	CLOSE = 7, LOCK = 8,
	UNLOCK = 9,	USE = 10, GIVE = 11, TALK_TO = 12, TELL = 13, BUY = 14,
	LOOK = 15, LOOK_AT = 16, LOOK_THROUGH = 17,	ASK = 18, DRINK = 20,
	STATUS = 21, GO_TO = 22, RETURN = 23, BRIBE = 24, EXAMINE = 25,
	NPC_SET_ROOM_AND_BLOCKED_OFFSET = 28, NPC_UNKNOWN1 = 29, NPC_EXEC_SCRIPT = 30, 
	NPC_UNKNOWN2 = 31, NPC_SET_RAND_DEST = 32, NPC_WALKING_CHECK = 33, 
	NPC_SET_SUPPORT_OFFSET = 34, NPC_SUPPORT_OFFSET_COND = 35, 
	NPC_DISPATCH_ACTION = 36, NPC_UNKNOWN3 = 37, NPC_UNKNOWN4 = 38, 
	NPC_START_TALKING = 39, NPC_JUMP_ADDRESS = 40,
	NONE = 0
};

struct CurrentActionOutput {
	uint8 action;
	uint8 hsAction;
	uint16 roomNumber;
	uint16 hotspotId;
	uint16 usedId;
};

int numParams[NPC_JUMP_ADDRESS+1] = {0, 
	1, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 0, 1,
	0, 1, 1, 1, 1, 0, 0, 2, 1, 1, 0, 0, 1, 1, 2, 2, 5, 2, 2, 1};

#ifdef ENGLISH_LURE
#define NUM_JUMP_OFFSETS 2
uint16 jumpOffsets[2] = {0x87be, 0x881c};
#endif

#define MAX_BUFFER_ENTRIES 63
#define MAX_INSTRUCTION_ENTRIES 300
#define SCHEDULE_DATA_OFFSET 0x80

struct SupportStructure {
	uint16 offset;
	int numInstructions;
	uint16 instructionOffsets[MAX_INSTRUCTION_ENTRIES];	
	uint16 resourceOffset;
};

SupportStructure supportList[MAX_BUFFER_ENTRIES];
uint16 numSupportEntries = 0;

#define FORWARD_JUMP_ALLOWANCE 0x30


uint16 get_sequence_index(uint16 offset, int supportIndex) {
	int index;

	if (supportIndex != -1) {
		// Within a sequence, so if an offset is within it, it's a local jump
		for (index = 0; index < supportList[supportIndex].numInstructions; ++index) {
			if (supportList[supportIndex].instructionOffsets[index] == offset)
				return index;
		}
	}

	for (int index = 0; index <= numSupportEntries; ++index) {
		SupportStructure &rec = supportList[index];

		if ((rec.numInstructions > 0) &&
			(offset >= rec.instructionOffsets[0]) && 
			(offset <= rec.instructionOffsets[rec.numInstructions - 1])) {
			// Scan through the entry's insruction list
			for (int iIndex = 0; iIndex < rec.numInstructions; ++iIndex) {
				if (rec.instructionOffsets[iIndex] == offset) {
					return ((index + 1) << 10) | iIndex;
				}
			}
		}
	}

	return 0xffff;
}

struct SymbolTableEntry {
	uint16 *p;
	bool globalNeeded;
};

uint16 process_action_sequence_entry(int supportIndex, byte *data, uint16 remainingSize) {
	SupportStructure &rec = supportList[supportIndex];
	uint16 startOffset = rec.offset;
	uint16 maxOffset = 0;
#ifdef ENGLISH_LURE
	if (startOffset == 0x7dcb) { startOffset = 0x7d9d; maxOffset = 0x7dcb; }
	if (startOffset == 0x7248) { startOffset = 0x71ce; maxOffset = 0x7248; }
	if (startOffset == 0x79a8) { startOffset = 0x785c; maxOffset = 0x79a8; }
	if (startOffset == 0x6f4f) { startOffset = 0x6e5d; maxOffset = 0x6fe5; }
	if (startOffset == 0x76ec) { startOffset = 0x734a; maxOffset = 0x77a2; }
#endif
//printf("Start=%xh max=%xh\n", startOffset, maxOffset);
	SymbolTableEntry symbolTable[MAX_INSTRUCTION_ENTRIES];
	uint16 numSymbols = 0;
	uint16 offset = startOffset;
	uint16 totalSize = 0;
	uint16 actionNum = 0;
	uint16 paramIndex;
	uint16 params[5];
	uint16 index;
	uint16 *pOut = (uint16 *) data;

	lure_exe.seek(DATA_SEGMENT + startOffset);
	rec.numInstructions = 0;

	for (;;) {
		if (remainingSize < 10) {
			printf("Ran out of space to process NPC action sequences\n");
			exit(1);
		}

		// Check for end of sequence set with prior instruction
		if ((actionNum == NPC_SET_SUPPORT_OFFSET) && ((maxOffset == 0) ||
			(offset > maxOffset)))
			break;

		// Mark the offset of the next instruction
		rec.instructionOffsets[rec.numInstructions++] = offset;
		if (rec.numInstructions == MAX_INSTRUCTION_ENTRIES) {
			printf("A method exceeded the maximum allowable number of instructions\n");
			exit(1);
		}

		// Get in the next action
		actionNum = lure_exe.readWord();

//		printf("%xh - action=%d", offset, actionNum);

		if (actionNum == 0) {
			// At end of script block
//			printf("\n");
			break;
		}
		else if (actionNum > NPC_JUMP_ADDRESS) {
			// Unknown action code - halt execution
			printf("%xh - unknown action %d\n", offset, actionNum);
			exit(1);
		}

		*pOut++ = TO_LE_16(actionNum);

		// Read in any action parameters
		for (int paramCtr = 0; paramCtr < numParams[actionNum]; ++paramCtr)
			params[paramCtr] = lure_exe.readWord();

		switch(actionNum) {
		case NPC_SET_ROOM_AND_BLOCKED_OFFSET:
		case NPC_SET_SUPPORT_OFFSET:
		case NPC_SUPPORT_OFFSET_COND:
		case NPC_DISPATCH_ACTION:
			// These instructions have a support record parameter. Store the
			// offset the parameter will be in the output data so we can come
			// back at the end and resolve it
			paramIndex = (actionNum == NPC_SET_SUPPORT_OFFSET) ? 0 : 1;
			symbolTable[numSymbols].globalNeeded = actionNum == NPC_SET_ROOM_AND_BLOCKED_OFFSET;
			symbolTable[numSymbols].p = pOut + paramIndex;
			++numSymbols;

			// Special check for forward references - it's considered to be in
			// the same block if it's forward within 100h blocks
			if ((params[paramIndex] > offset) && 
				(params[paramIndex] < offset + FORWARD_JUMP_ALLOWANCE) &&
				(params[paramIndex] > maxOffset)) {
				maxOffset = params[paramIndex];
			} 
			break;

		case NPC_JUMP_ADDRESS:
			// Make sure the address is in the known list
			index = 0;
			while ((index < NUM_JUMP_OFFSETS) && (jumpOffsets[index] != params[0]))
				++index;
			
			if (index != NUM_JUMP_OFFSETS)
				// Replace code offset with an index			
				params[0] = index;
			else {
				printf("Encountered unrecognised NPC code jump point: %xh\n", params[0]);
				exit(1);
			}
			break;

		default:
			break;
		}

		// Output parameters
		for (paramIndex = 0; paramIndex < numParams[actionNum]; ++paramIndex)
		{
			*pOut++ = TO_LE_16(params[paramIndex]);
//			printf(" %xh", TO_LE_16(params[paramIndex]));
		}
//		printf("\n");

		// Increase size
		totalSize += (numParams[actionNum] + 1) * sizeof(uint16); 
		offset = startOffset + totalSize;
		remainingSize -= (numParams[actionNum] + 1) * sizeof(uint16);
	}

	// Flag an end of the sequence
	*pOut++ = 0;
	totalSize += sizeof(uint16);

	// handle post-processing of the symbol list

	for (int symbolCtr = 0; symbolCtr < numSymbols; ++symbolCtr) {
		if (READ_LE_UINT16(symbolTable[symbolCtr].p) == 0) 
			// No Id special constant
			WRITE_LE_UINT16(symbolTable[symbolCtr].p, 0xffff);
		else {
			// Handle resolving the constant
			index = get_sequence_index(READ_LE_UINT16(symbolTable[symbolCtr].p), 
				symbolTable[symbolCtr].globalNeeded ? -1 : supportIndex);
//printf("Symbol %xh => %xh\n", *symbolTable[symbolCtr].p, index);
			if (index != 0xffff) {
				// Jump found - so replace symbol entry with it 
				WRITE_LE_UINT16(symbolTable[symbolCtr].p, index);
			} else {
				printf("Sequence contained unknown offset %xh\n", 
					READ_LE_UINT16(symbolTable[symbolCtr].p));
				exit(1);
			}
		}
	}

	return totalSize;
}

void process_entry(uint16 offset, byte *data, uint16 &totalSize) {
	if (get_sequence_index(offset) == 0xffff) {
		// Process the next entry
		supportList[numSupportEntries].offset = offset;
		supportList[numSupportEntries].numInstructions = 0;
		supportList[numSupportEntries].resourceOffset = totalSize;
//printf("process_entry index=%d, offset=%xh\n", numSupportEntries + 1, offset);
		totalSize += process_action_sequence_entry(numSupportEntries, 
			data + totalSize,  MAX_DATA_SIZE - totalSize);

		++numSupportEntries;
		if (numSupportEntries == MAX_BUFFER_ENTRIES) {
			printf("Ran out of buffer space in processing NPC schedules\n");
			exit(1);
		}
	}
}

struct RoomRandomActionEntry {
	bool repeatable;
	uint16 offset;
};

struct RoomRandomActionSet {
	uint16 offset;
	uint8 numEntries;
	RoomRandomActionEntry *entries;
};

void read_action_sequence(byte *&data, uint16 &totalSize) 
{
	const uint16 hsOffset = 0x5d98;
	const uint16 hsStartId = 0x3e8;
	uint16 hotspotIndex;
	HotspotHeaderEntry entryHeader;
	CurrentActionInput action;
	uint16 *pHeader;
	int index, roomIndex;

	// Allocate enough space for output sequence list
	data = (byte *) malloc(MAX_DATA_SIZE);

	// Get a list of offsets used in the script engine
	uint16 offsetList[NUM_TABLED_ACTION_BLOCKS];
	lure_exe.seek(DATA_SEGMENT + TABLED_ACTIONS_OFFSET, SEEK_SET);
	for (index = 0; index < NUM_TABLED_ACTION_BLOCKS; ++index)
		offsetList[index] = lure_exe.readWord();
	totalSize = sizeof(uint16) * (NUM_TABLED_ACTION_BLOCKS + 1);

	/* Process the list of random actions that your follower can do in each room */
	RoomRandomActionSet *randomActions = new RoomRandomActionSet[RANDOM_ROOM_NUM_ENTRIES];

	// Get a list of the offsets for each room
	lure_exe.seek(DATA_SEGMENT + RANDOM_ACTIONS_OFFSET, SEEK_SET);
	for (roomIndex = 0; roomIndex < RANDOM_ROOM_NUM_ENTRIES; ++roomIndex)
	{
		randomActions[roomIndex].offset = lure_exe.readWord();
		randomActions[roomIndex].numEntries = 0;
		randomActions[roomIndex].entries = NULL;
	}

	// Next get the set of offsetes for the start of each sequence
	for (roomIndex = 0; roomIndex < RANDOM_ROOM_NUM_ENTRIES; ++roomIndex)
	{
		if (randomActions[roomIndex].offset == 0)
			continue;

		lure_exe.seek(DATA_SEGMENT + randomActions[roomIndex].offset, SEEK_SET);
		randomActions[roomIndex].numEntries = lure_exe.readByte();
		assert(randomActions[roomIndex].numEntries <= 8);
		randomActions[roomIndex].entries = new RoomRandomActionEntry[randomActions[roomIndex].numEntries];

		// Loop through the entries
		uint16 sequenceVal;
		uint16 offset = randomActions[roomIndex].offset + 1;
		for (uint8 entryCtr = 0; entryCtr < randomActions[roomIndex].numEntries; ++entryCtr)
		{
			randomActions[roomIndex].entries[entryCtr].repeatable = lure_exe.readWord() == 1;
			offset += 2;

			uint16 firstCommand = lure_exe.readWord();
			randomActions[roomIndex].entries[entryCtr].offset = 
				(firstCommand == 0xfffe) ? 0 : offset;
			
			offset += sizeof(uint16);
			while (lure_exe.readWord() != 0xffff) 
				offset += sizeof(uint16);
			offset += sizeof(uint16);
		}

		// Adjust the total size to accomodate random action data in the output
		totalSize += sizeof(uint16)  * randomActions[roomIndex].numEntries +
			(sizeof(uint16) * 2);
	}

	totalSize += sizeof(uint16) + MAX_BUFFER_ENTRIES * sizeof(uint16);

	numSupportEntries = 0;

	// Handle required initial entries - the Lure engine refers to them directly by
	// index, so they need to be first, and in that order
#ifdef ENGLISH_LURE
	process_entry(0x13c2, data, totalSize);
	process_entry(0xbb95, data, totalSize);
	process_entry(0x7060, data, totalSize);
	process_entry(0x728a, data, totalSize);
	process_entry(0x76ec, data, totalSize);
#endif

	// Process the script engine list 
	
	for (index = 0; index < NUM_TABLED_ACTION_BLOCKS; ++index) 
		if (offsetList[index] != 0)
			process_entry(offsetList[index], data, totalSize);

	// Next process each of the character hotspots

	hotspotIndex = 0;
	for (;;) {
		lure_exe.seek(DATA_SEGMENT + hsOffset + 
			hotspotIndex * sizeof(HotspotHeaderEntry));
		lure_exe.read(&entryHeader, sizeof(HotspotHeaderEntry));
		if (FROM_LE_16(entryHeader.offset) == 0xffff) break;
		++hotspotIndex;

		// Move to the action sequence area of the hotspot
		lure_exe.seek(DATA_SEGMENT + entryHeader.offset + 0x63);
		lure_exe.read(&action, sizeof(CurrentActionInput));
		if (FROM_LE_16(action.action) == 2) 
			process_entry(FROM_LE_16(action.dataOffset), data, totalSize);
	}

	// Finally process each of the random room actions

	for (roomIndex = 0; roomIndex < RANDOM_ROOM_NUM_ENTRIES; ++roomIndex)
	{
		for (index = 0; index < randomActions[roomIndex].numEntries; ++index)
		{
			if (randomActions[roomIndex].entries[index].offset != 0xfffe)
			{
//				printf("room=%d entry=%xh\n", roomIndex+1, randomActions[roomIndex].entries[index].offset);
				process_entry(randomActions[roomIndex].entries[index].offset, data, totalSize);
			}
		}
	}

	// Output the list used in the script engine

	pHeader = (uint16 *) data;
	for (index = 0; index < NUM_TABLED_ACTION_BLOCKS; ++index) 
		if (offsetList[index] == 0)
			*pHeader++ = 0;
		else
			*pHeader++ = TO_LE_16(get_sequence_index(offsetList[index]));
	*pHeader++ = TO_LE_16(0xffff);

	// Output the data for the random room actions

	for (roomIndex = 0; roomIndex < RANDOM_ROOM_NUM_ENTRIES; ++roomIndex)
	{
		if (randomActions[roomIndex].numEntries == 0)
			continue;

		*pHeader++ = TO_LE_16(roomIndex + 1);    // Save the room number

		// Create a word containing the number of available actions and a bit flag set
		// specifying which of the actions are repeatable (as opposed to once only)
		uint16 v = randomActions[roomIndex].numEntries;
		for (int entryCtr = 0; entryCtr < randomActions[roomIndex].numEntries; ++entryCtr)
			if (randomActions[roomIndex].entries[entryCtr].repeatable)
				v |= (0x100 << entryCtr);
		*pHeader++ = TO_LE_16(v);

		// Loop through the entries storing the action set to use
		for (int entryCtr = 0; entryCtr < randomActions[roomIndex].numEntries; ++entryCtr)
			if (randomActions[roomIndex].entries[entryCtr].offset == 0)
				*pHeader++ = 0;
			else
				*pHeader++ = TO_LE_16(get_sequence_index(randomActions[roomIndex].entries[entryCtr].offset));
	}
	*pHeader++ = TO_LE_16(0xffff);

	// Output the offsets of each action set

	for (index = 0; index < numSupportEntries; ++index) 
		*pHeader++ = TO_LE_16(supportList[index].resourceOffset);
	*pHeader++ = TO_LE_16(0xffff);

	// Free up the random room action array
	for (roomIndex = 0; roomIndex < 1; ++roomIndex)
	{
		if (randomActions[roomIndex].entries != NULL)
			delete randomActions[roomIndex].entries;
	}
	delete randomActions;
}	
