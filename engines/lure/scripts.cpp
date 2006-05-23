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

#include "lure/scripts.h"
#include "lure/res.h"
#include "lure/game.h"
#include "common/stack.h"
#include "common/endian.h"

namespace Lure {

// This list of hotspot Ids are used by sequence method #5 to deallocate a set 
// of hotspot Ids at once
uint16 dealloc_list_1[] = {0x13F2, 0x418, 0x2711, 0x2712, 0x40D, 0x3EA, 0x411, 0};
uint16 dealloc_list_2[] = {0x2729, 0x272A, 0x272B, 0x272C, 0x272E, 0x272E, 0x272F, 0};
uint16 dealloc_list_3[] = {0x3EF, 0x3E9, 0x3EB, 0x3EC, 0x3ED, 0x3EE, 0x3F0, 0x3F1, 
	0x420, 0x429, 0x436, 0x2715, 0x2716, 0x2717, 0x2718, 0x2719, 0x271A, 0x271E, 
	0x271F, 0x2720, 0x2721, 0x2722, 0x2725, 0x2726, 0};
uint16 *hotspot_dealloc_set[4] = {&dealloc_list_1[0], &dealloc_list_2[0],
	&dealloc_list_3[1], &dealloc_list_3[0]};

/*------------------------------------------------------------------------*/
/*-  Script Method List                                                  -*/
/*-                                                                      -*/
/*------------------------------------------------------------------------*/

// activateHotspot
// Activates a hotspot entry for active use

void Script::activateHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources::getReference().activateHotspot(hotspotId);
}

// setHotspotScript
// Sets a hotspot's animation script offset from a master table of offsets

void Script::setHotspotScript(uint16 hotspotId, uint16 scriptIndex, uint16 v3) {
	Resources &r = Resources::getReference();
	uint16 offset = r.getHotspotScript(scriptIndex);
	HotspotData *rsc = r.getHotspot(hotspotId);
	rsc->sequenceOffset = offset;
}

// Clears the sequence delay list

void Script::clearSequenceDelayList(uint16 v1, uint16 scriptIndex, uint16 v3) {
	Resources::getReference().delayList().clear();
}

// Deactivates a set of predefined of hotspots in a given list index

void Script::deactivateHotspotSet(uint16 listIndex, uint16 v2, uint16 v3) {
	if (listIndex >= 3) 
		error("Script::deactiveHotspotSet - Invalid list index");
	Resources &res = Resources::getReference();
	uint16 *hotspotId = hotspot_dealloc_set[listIndex];

	while (*hotspotId != 0) {
		res.deactivateHotspot(*hotspotId);
		++hotspotId;
	}
}

// deactivates the specified hotspot from active animation

void Script::deactivateHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &rsc = Resources::getReference();
	if (hotspotId < START_NONVISUAL_HOTSPOT_ID) 
		rsc.deactivateHotspot(hotspotId);
	HotspotData *hs = rsc.getHotspot(hotspotId);
	hs->flags |= 0x20;
	if (hotspotId < START_NONVISUAL_HOTSPOT_ID) 
		hs->layer = 0xff;
}

// Sets the offset for the table of action sequence offsets for the given
// hotspot

void Script::setActionsOffset(uint16 hotspotId, uint16 offset, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(hotspotId);
	
	if (!res.getHotspotActions(offset))
		warning("Hotspot %xh set to invalid actions offset %d",
			hotspotId, offset);
	
	hotspot->actionsOffset = offset;
}

// Add a sequence to be executed after a specified delay

void Script::addDelayedSequence(uint16 seqOffset, uint16 delay, uint16 v3) {
	SequenceDelayList &list = Resources::getReference().delayList();
	list.addSequence(delay, seqOffset);
}

// Checks whether the given character is in the specified room, and stores
// the result in the general value field

void Script::characterInRoom(uint16 characterId, uint16 roomNumber, uint16 v3) {
	Resources &res = Resources::getReference();
	uint16 result = 0;
	if (characterId >= PLAYER_ID) {
		HotspotData *hotspot = res.getHotspot(characterId);
		if (hotspot->roomNumber == roomNumber)
			result = 1;
	}

	res.fieldList().setField(GENERAL, result);
}

// Changes the given hotspot's name to a new name

void Script::setHotspotName(uint16 hotspotId, uint16 nameId, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->nameId = nameId;
}

// Unsure about this method, but at the moment I think it plays a sound

void Script::playSound(uint16 v1, uint16 v2, uint16 v3) {
	// No implementation currently	
}

// Displays the given string resource Id in a dialog

void Script::displayDialog(uint16 stringId, uint16 v2, uint16 v3) {
	Dialog::show(stringId);
}

// Flags for remotely viewing a room

void Script::remoteRoomViewSetup(uint16 v1, uint16 v2, uint16 v3) {
	Hotspot *player = Resources::getReference().getActiveHotspot(PLAYER_ID);

	player->setTickProc(0); // disable player actions
	Resources::getReference().fieldList().setField(OLD_ROOM_NUMBER,
		player->roomNumber());
}

// Checks the status of the cell door, and starts music depending on it's state

void Script::checkCellDoor(uint16 v1, uint16 v2, uint16 v3) {
	// In the original game, this method checks to see if the cell door
	// is currently open, if it is, starts a music sequence. 
	// TODO: Implement starting music if cell door is open
}

void Script::playMusic(uint16 musicNum, uint16 v2, uint16 v3) {
	// TODO: Play a given music
	warning("TODO: Play music #%d", musicNum);
}

// Gets the current blocked state for the given door and stores it in the
// general value field

void Script::getDoorBlocked(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	RoomExitJoinData *joinRec = res.getExitJoin(hotspotId);
	res.fieldList().setField(GENERAL, joinRec->blocked);
}

// Checks whether the Skorl is in the cell

void Script::isSkorlInCell(uint16 v1, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(SKORL_ID);
	uint16 v = (hotspot->roomNumber == 1) ? 0 : 1;
	res.fieldList().setField(GENERAL, v);
}

// Sets a character to a given hotspot script, and sets the character's current 
// action to executing a script

void Script::setBlockingHotspotScript(uint16 charId, uint16 scriptIndex, uint16 v3) {
	Resources &r = Resources::getReference();
	uint16 offset = r.getHotspotScript(scriptIndex);

	Hotspot *hs = r.getActiveHotspot(charId);
	hs->setScript(offset);
	hs->currentActions().top().setAction(EXEC_HOTSPOT_SCRIPT);
	hs->setOccupied(true);
}

// Decrements the number of inventory itemst he player has

void Script::decrInventoryItems(uint16 v1, uint16 v2, uint16 v3) {
	// module doesn't use a static counter for the number of
	// inventory items, so don't do anything
}

// Sets the current frame number for the given hotspot

void Script::setFrameNumber(uint16 hotspotId, uint16 frameNumber, uint16 v3) {
	Hotspot *hotspot = Resources::getReference().getActiveHotspot(hotspotId);
	hotspot->setFrameNumber(frameNumber);
}

// Disables the given hotspot from being highlighted by the cursor

void Script::disableHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->flags |= 0x20;	
}

// Called when the sack is cut with the knife

void Script::cutSack(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *data = res.getHotspot(SACK_ID);
	data->startY = 138;
	Hotspot *activeHotspot = res.getActiveHotspot(SACK_ID);
	if (activeHotspot)
		activeHotspot->setPosition(data->startX, data->startY);
}

// Increase the player's number by the specified amount

void Script::increaseNumGroats(uint16 v1, uint16 numGroats, uint16 v3) {
	ValueTableData &fields = Resources::getReference().fieldList();
	fields.numGroats() += numGroats;
}

// Enables the flags for the given hotspot for it to be actively highlighted

void Script::enableHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	// Clear flag 0x20 and add flag 0x80
	hotspot->flags = (hotspot->flags & 0xdf) | 0x80;
}

// Transforms the player

void Script::transformPlayer(uint16 v1, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(TRANSFORM_ID);
	HotspotData *player = res.getHotspot(PLAYER_ID);

	hotspot->roomNumber = player->roomNumber;
	hotspot->startX = player->startX - 14;
	hotspot->startY = player->startY - 10;
	
	Hotspot *activeHotspot = res.addHotspot(TRANSFORM_ID);
	activeHotspot->setFrameNumber(0);
	activeHotspot->setScript(0x630);
}

// Marks the door in room 14 for closing

void Script::room14DoorClose(uint16 v1, uint16 v2, uint16 v3) {
	RoomExitJoinData *joinRec = Resources::getReference().getExitJoin(0x2719);
	joinRec->blocked = 1;
}

// Sets the sequence result to 1 if the given secondary description for a 
// hotspot is empty (for inventory items, this gives the description before
// the item is initially picked up)

void Script::checkDroppedDesc(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(hotspotId);
	uint16 seqResult = (hotspot->descId2 == 0) ? 1 : 0;
	res.fieldList().setField(SEQUENCE_RESULT, seqResult);
}

// Marks the given door hotspot for closing

void Script::doorClose(uint16 hotspotId, uint16 v2, uint16 v3) {
	RoomExitJoinData *joinRec = Resources::getReference().getExitJoin(hotspotId);
	if (!joinRec) error("Tried to close a non-door");
	joinRec->blocked = 1;
}

// Marks the given door hotspot for opening

void Script::doorOpen(uint16 hotspotId, uint16 v2, uint16 v3) {
	RoomExitJoinData *joinRec = Resources::getReference().getExitJoin(hotspotId);
	if (!joinRec) error("Tried to close a non-door");
	joinRec->blocked = 0;
}

// Lookup the given message Id for the specified character and display in a dialog

void Script::displayMessage(uint16 messageId, uint16 characterId, uint16 unknownVal) {
	Dialog::showMessage(messageId, characterId);
}

// Creates a new dispatch action with the given support data entry

void Script::setNewSupportData(uint16 hotspotId, uint16 index, uint16 v3) {
	Resources &res = Resources::getReference();
	uint16 dataId = res.getCharOffset(index);
	CharacterScheduleEntry *entry = res.charSchedules().getEntry(dataId);

	Hotspot *h = res.getActiveHotspot(hotspotId);
	h->currentActions().addFront(DISPATCH_ACTION, entry, h->roomNumber());
}

// Replaces the existing current action with a new dispatch data entry

void Script::setSupportData(uint16 hotspotId, uint16 index, uint16 v3) {
	Resources &res = Resources::getReference();
	uint16 dataId = res.getCharOffset(index);

	CharacterScheduleEntry *entry = res.charSchedules().getEntry(dataId);
	Hotspot *h = res.getActiveHotspot(hotspotId);

	h->currentActions().pop();
	h->currentActions().addFront(DISPATCH_ACTION, entry, h->roomNumber());
}

// Assign the given hotspot item to the player's inventory

void Script::givePlayerItem(uint16 hotspotId, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->roomNumber = PLAYER_ID;
	hotspot->flags |= 0x80;
}

// Decrease the number of graots the player has

void Script::decreaseNumGroats(uint16 characterId, uint16 numGroats, uint16 v3) {
	ValueTableData &fields = Resources::getReference().fieldList();
	fields.numGroats() -= numGroats;
}

// Sets the tick handler for the village Skorl to an alternate handler

void Script::setVillageSkorlTickProc(uint16 v1, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(0x3F1);
	hotspot->tickProcOffset = 0x7efa;
}

// Stores the current number of groats in the general field

void Script::getNumGroats(uint16 v1, uint16 v2, uint16 v3) {
	ValueTableData fields = Resources::getReference().fieldList();
	fields.setField(GENERAL, fields.numGroats());
}

// Loads the specified animation, completely bypassing the standard process
// of checking for a load proc/sequence

void Script::animationLoad(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources::getReference().addHotspot(hotspotId);
}

// Adds the passed actions to the available actions for the given hotspot

void Script::addActions(uint16 hotspotId, uint16 actions, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->actions |= actions;
}


typedef void(*SequenceMethodPtr)(uint16, uint16, uint16);

struct SequenceMethodRecord {
	uint8 methodIndex;
	SequenceMethodPtr proc;
};

SequenceMethodRecord scriptMethods[] = {
	{0, Script::activateHotspot}, 
	{1, Script::setHotspotScript},
	{4, Script::clearSequenceDelayList},
	{5, Script::deactivateHotspotSet},
	{6, Script::deactivateHotspot},
	{8, Script::addDelayedSequence},
	{10, Script::characterInRoom},
	{11, Script::setActionsOffset},
	{12, Script::setHotspotName},
	{13, Script::playSound},
	{16, Script::displayDialog},
	{18, Script::remoteRoomViewSetup},
	{20, Script::checkCellDoor},
	{21, Script::playMusic},
	{22, Script::getDoorBlocked},
	{23, Script::isSkorlInCell},
	{27, Script::setBlockingHotspotScript},
	{28, Script::decrInventoryItems},
	{30, Script::setFrameNumber},
	{32, Script::disableHotspot},
	{33, Script::cutSack},
	{34, Script::increaseNumGroats},
	{35, Script::enableHotspot},
	{37, Script::transformPlayer},
	{39, Script::room14DoorClose},
	{40, Script::checkDroppedDesc},
	{42, Script::doorClose},
	{44, Script::doorOpen},
	{47, Script::displayMessage},
	{48, Script::setNewSupportData},
	{49, Script::setSupportData},
	{50, Script::givePlayerItem},
	{51, Script::decreaseNumGroats},
	{54, Script::setVillageSkorlTickProc},
	{57, Script::getNumGroats},
	{62, Script::animationLoad},
	{63, Script::addActions},
	{65, Script::checkCellDoor},
	{0xff, NULL}};
	
/*------------------------------------------------------------------------*/
/*-  Script Execution                                                    -*/
/*-                                                                      -*/
/*------------------------------------------------------------------------*/

uint16 Script::execute(uint16 startOffset) {
	Resources &r = Resources::getReference();
	ValueTableData &fields = r.fieldList();
	MemoryBlock *scriptData = r.scriptData();
	byte *scripts = scriptData->data();
	Common::Stack<uint16> stack;
	Common::Stack<uint16> methodStack;
	byte opcode;
	uint16 param, v1, v2;
	uint16 param1, param2, param3;
	uint16 fieldNum;
	uint32 tempVal;
	SequenceMethodPtr ptr;
	SequenceMethodRecord *rec;

	uint16 offset = startOffset;
	bool breakFlag = false;
	param = 0;
	fields.setField(SEQUENCE_RESULT, 0);

	while (!breakFlag) {
		if (offset >= scriptData->size()) 
			error("Script failure in script %d - invalid offset %d", startOffset, offset);

		opcode = scripts[offset++];
		if ((opcode & 1) != 0) {
			// Flag to read next two bytes as active parameter
			if (offset >= scriptData->size()-2) 
				error("Script failure in script %d - invalid offset %d", startOffset, offset);

			param = READ_LE_UINT16(scripts + offset);
			offset += 2;
		}
		opcode >>= 1;	// Discard param bit from opcode byte

		switch (opcode) {
		case S_OPCODE_ABORT:
		case S_OPCODE_ABORT2:
		case S_OPCODE_ABORT3:
			methodStack.clear();
			break;

		case S_OPCODE_ADD:
			stack.push(stack.pop() + stack.pop());
			break;

		case S_OPCODE_SUBTRACT:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 - v1);
			break;

		case S_OPCODE_MULTIPLY:
			tempVal = stack.pop() * stack.pop();
			stack.push(tempVal & 0xffff);
			param = (uint16) (tempVal >> 16);
			break;

		case S_OPCODE_DIVIDE:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 / v1);
			param = v2 % v1;      // remainder
			break;

		case S_OPCODE_NOT_EQUALS:
			stack.push((stack.pop() != stack.pop()) ? 0 : 1);
			break;

		case S_OPCODE_EQUALS:
			stack.push((stack.pop() == stack.pop()) ? 0 : 1);
			break;

		case S_OPCODE_GT:
			stack.push((stack.pop() > stack.pop()) ? 1 : 0);
			break;

		case S_OPCODE_LT:
			stack.push((stack.pop() < stack.pop()) ? 1 : 0);
			break;

		case S_OPCODE_LT2:
			stack.push((stack.pop() < stack.pop()) ? 1 : 0);
			break;

		case S_OPCODE_GT2:
			stack.push((stack.pop() > stack.pop()) ? 1 : 0);
			break;

		case S_OPCODE_AND:
			stack.push(stack.pop() & stack.pop());
			break;

		case S_OPCODE_OR:
			stack.push(stack.pop() | stack.pop());
			break;

		case S_OPCODE_LOGICAL_AND:
			stack.push(((stack.pop() != 0) && (stack.pop() != 0)) ? 1 : 0);
			break;

		case S_OPCODE_LOGICAL_OR:
			stack.push(((stack.pop() != 0) || (stack.pop() != 0)) ? 1 : 0);
			break;

		case S_OPCODE_GET_FIELD:
			// Opcode not yet fully implemented
			fieldNum = param >> 1;
			v1 = fields.getField(fieldNum);
			stack.push(v1);
			break;

		case S_OPCODE_SET_FIELD:
			// Opcode not yet fully implemented
			fieldNum = param >> 1;
			v1 = stack.pop();
			fields.setField(fieldNum, v1);
			break;

		case S_OPCODE_PUSH:
			stack.push(param);
			break;

		case S_OPCODE_SUBROUTINE:
			methodStack.push(offset);
			offset = param;
			break;

		case S_OPCODE_EXEC:
			param1 = 0; param2 = 0; param3 = 0;
			if (!stack.empty()) param1 = stack.pop();
			if (!stack.empty()) param2 = stack.pop();
			if (!stack.empty()) param3 = stack.pop();

			rec = &scriptMethods[0];
			while ((rec->methodIndex != 0xff) && (rec->methodIndex != param))
				++rec;

			if (rec->methodIndex == 0xff) 
				warning("Undefined script method %d", param);
			else {
				ptr = rec->proc;
				ptr(param1, param2, param3);
			}
			break;

		case S_OPCODE_COND_JUMP:
			v1 = stack.pop();
			if (v1 == 0) offset += (int16) param;
			break;

		case S_OPCODE_JUMP:
			offset += (int16) param;
			break;

		case S_OPCODE_RANDOM:
			param = r.random() >> 8; // make number between 0 to 255
			break;

		case S_OPCODE_END:
			// Signal to end the execution
			if (!methodStack.empty()) 
				offset = methodStack.pop();
			else
				breakFlag = true;
			break;

		default:
			error("Unknown script opcode %d", opcode);
			break;
		}
	}

	return fields.getField(SEQUENCE_RESULT);
}

/*------------------------------------------------------------------------*/
/*-  Hotspot Script Handler                                              -*/
/*-                                                                      -*/
/*------------------------------------------------------------------------*/

int16 HotspotScript::nextVal(MemoryBlock *data, uint16 &offset) {
	if (offset >= data->size() - 1) 
		error("Script failure - invalid offset");
	int16 value = READ_LE_UINT16(data->data() + offset);
	offset += 2;
	return value;
}

bool HotspotScript::execute(Hotspot *h)
{
	Resources &r = Resources::getReference();
	MemoryBlock *scriptData = r.hotspotScriptData();
	uint16 offset = h->script();
	int16 opcode = 0;
	int16 param1, param2;
	bool breakFlag = false;

	while (!breakFlag) {
		opcode = nextVal(scriptData, offset);

		switch (opcode) {
		case S2_OPCODE_TIMEOUT:
			param1 = nextVal(scriptData, offset);
			h->setTickCtr(param1);
			h->setScript(offset);
			breakFlag = true;
			break;

		case S2_OPCODE_POSITION:
			param1 = nextVal(scriptData, offset);
			param2 = nextVal(scriptData, offset);
			h->setPosition(param1 - 0x80, param2 - 0x80);
			break;
			
		case S2_OPCODE_CHANGE_POS:
			param1 = nextVal(scriptData, offset);
			param2 = nextVal(scriptData, offset);
			h->setPosition(h->x() + param1, h->y() + param2);
			break;

		case S2_OPCODE_UNLOAD:
			breakFlag = true;
			break;

		case S2_OPCODE_DIMENSIONS:
			param1 = nextVal(scriptData, offset) << 4;
			param2 = nextVal(scriptData, offset);
			h->setSize((uint16) param1, (uint16) param2);
			break;

		case S2_OPCODE_JUMP:
			offset = (uint16) nextVal(scriptData, offset);
			break;

		case S2_OPCODE_ANIMATION:
			param1 = nextVal(scriptData, offset);
			h->setAnimation(param1);
			break;

		case S2_OPCODE_UNKNOWN_247:
			param1 = nextVal(scriptData, offset);
			param2 = nextVal(scriptData, offset);
//			warning("UNKNOWN_247 stub called");
			break;

		case S2_OPCODE_UNKNOWN_258:
			param1 = nextVal(scriptData, offset);
//			warning("UNKNOWN_258 stub called");
			break;

		case S2_OPCODE_ACTIONS:
			param1 = nextVal(scriptData, offset) << 4;
			param2 = nextVal(scriptData, offset);
			h->setActions((uint32) param1 | ((uint32) param2 << 16));
			break;

		default:
			// Set the animation frame number
			h->setFrameNumber(opcode);
			h->setScript(offset);
			breakFlag = true;
			break;
		}
	}

	return (opcode == S2_OPCODE_UNLOAD);
}

} // end of namespace Lure
