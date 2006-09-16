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
#include "common/config-manager.h"
#include "common/debugger.cpp"
#include "lure/debugger.h"
#include "lure/res.h"
#include "lure/res_struct.h"
#include "lure/room.h"
#include "lure/strings.h"

namespace Lure {

Debugger::Debugger(): Common::Debugger<Debugger>() {
	DCmd_Register("continue",			&Debugger::cmd_exit);
	DCmd_Register("exit",				&Debugger::cmd_exit);
	DCmd_Register("help",				&Debugger::cmd_help);
	DCmd_Register("quit",				&Debugger::cmd_exit);
	DCmd_Register("enter",				&Debugger::cmd_enterRoom);
	DCmd_Register("rooms",				&Debugger::cmd_listRooms);
	DCmd_Register("fields",				&Debugger::cmd_listFields);
	DCmd_Register("setfield",			&Debugger::cmd_setField);
	DCmd_Register("queryfield",			&Debugger::cmd_queryField);
	DCmd_Register("give",				&Debugger::cmd_giveItem);
	DCmd_Register("hotspots",			&Debugger::cmd_hotspots);
	DCmd_Register("hotspot",			&Debugger::cmd_hotspot);
}

static int strToInt(const char *s) {
	if (!*s) 
		// No string at all
		return 0;
	else if (strcmp(s, "player") == 0) 
		return PLAYER_ID;
	else if (strcmp(s, "ratpouch") == 0) 
		return RATPOUCH_ID;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	int result = 0;
	const char *p = s;
	char ch;
	while ((ch = toupper(*p++)) != 'H') {
		if ((ch >= '0') && (ch <= '9'))
			result = (result << 4) + (ch - '0');
		else if ((ch >= 'A') && (ch <= 'F'))
			result = (result << 4) + (ch - 'A' + 10);
		else
			break;
	}
	return result;
}

void Debugger::preEnter() {
	// NO IMPLEMENTATION
}

void Debugger::postEnter() {
	// NO IMPLEMENTATION
}

bool Debugger::cmd_enterRoom(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	uint remoteFlag = 0;

	if (argc > 1) {
		int roomNumber = strToInt(argv[1]);

		// Validate that it's an existing room
		if (res.getRoom(roomNumber) == NULL) {
			DebugPrintf("specified number was not a valid room\n");
			return true;
		}

		if (argc > 2) {
			remoteFlag = strToInt(argv[2]);
		} 

		room.leaveRoom();
		room.setRoomNumber(roomNumber);
		if (!remoteFlag) 
			res.getActiveHotspot(PLAYER_ID)->setRoomNumber(roomNumber);

		_detach_now = true;
		return false;
	}

	DebugPrintf("Syntax: room <roomnum> [<remoteview>]\n");
	DebugPrintf("A non-zero value for reomteview will change the room without ");
	DebugPrintf("moving the player.\n");
	return true;
}

bool Debugger::cmd_exit(int argc, const char **argv) {
	_detach_now = true;
	return false;
}

bool Debugger::cmd_help(int argc, const char **argv) {
	// console normally has 39 line width
	// wrap around nicely
	int width = 0, size, i;

	DebugPrintf("Commands are:\n");
	for (i = 0 ; i < _dcmd_count ; i++) {
		size = strlen(_dcmds[i].name) + 1;

		if ((width + size) >= 69) {
			DebugPrintf("\n");
			width = size;
		} else
			width += size;

		DebugPrintf("%s ", _dcmds[i].name);
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger::cmd_listRooms(int argc, const char **argv) {
	RoomDataList &rooms = Resources::getReference().roomData();
	RoomDataList::iterator i;
	StringData &strings = StringData::getReference();
	char buffer[MAX_DESC_SIZE];
	int ctr = 0;

	DebugPrintf("Available rooms are:\n");
	for (i = rooms.begin(); i != rooms.end(); ++i) {
		RoomData *room = *i;
		strings.getString(room->roomNumber, buffer);
		// DEBUG: Explictly note the second drawbridge room as "Alt" for now
		if (ctr == 42) { strcat(buffer, " (alt)"); }

		DebugPrintf("#%d - %s", room->roomNumber, buffer);

		if (++ctr % 3 == 0) DebugPrintf("\n");
		else {
			// Write out spaces between columns
			int numSpaces = 25 - strlen(buffer) - ((room->roomNumber >= 10) ? 2 : 1);
			char *s = buffer;
			while (numSpaces-- > 0) *s++ = ' ';
			*s = '\0';
			DebugPrintf("%s", buffer);
		}
	}
	DebugPrintf("\n");
	DebugPrintf("Current room: %d\n", Room::getReference().roomNumber());
	return true;
}

bool Debugger::cmd_listFields(int argc, const char **argv) {
	ValueTableData &fields = Resources::getReference().fieldList();

	for (int ctr = 0; ctr < fields.size(); ++ctr) {
		DebugPrintf("(%-2d): %-5d", ctr, fields.getField(ctr));
		if (!((ctr + 1) % 7)) 
			DebugPrintf("\n");
	}
	DebugPrintf("\n");
	return true;
}

bool Debugger::cmd_setField(int argc, const char **argv) {
	ValueTableData &fields = Resources::getReference().fieldList();

	if (argc >= 3) {
		int fieldNum = strToInt(argv[1]);
		uint16 value = strToInt(argv[2]);
		
		if ((fieldNum < 0) || (fieldNum >= fields.size())) {
			// Invalid field number
			DebugPrintf("Invalid field number specified\n");
		} else {
			// Set the field value
			fields.setField(fieldNum, value);
		}
	} else {
		DebugPrintf("Syntax: setfield <field_number> <value>\n");
	}

	return true;
}

bool Debugger::cmd_queryField(int argc, const char **argv) {
	ValueTableData &fields = Resources::getReference().fieldList();

	if (argc > 1) {
		int fieldNum = strToInt(argv[1]);
		if ((fieldNum < 0) || (fieldNum >= fields.size())) {
			// Invalid field number
			DebugPrintf("Invalid field number specified\n");
		} else {
			// Get the field value
			DebugPrintf("Field %d is %d (%xh)\n", fieldNum, 
				fields.getField(fieldNum), fields.getField(fieldNum));
		}
	} else {
		DebugPrintf("Syntax: queryfield <field_num>\n");
	}

	return true;
}

bool Debugger::cmd_giveItem(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	uint16 itemNum;
	uint16 charNum = PLAYER_ID;
	HotspotData *charHotspot, *itemHotspot;

	if (argc >= 2) {
		itemNum = strToInt(argv[1]);

		if (argc == 3) 
			charNum = strToInt(argv[2]);

		itemHotspot = res.getHotspot(itemNum);
		charHotspot = res.getHotspot(charNum);

		if (itemHotspot == NULL) {
			DebugPrintf("The specified item does not exist\n");
		} else if ((itemNum < 0x408) || (itemNum >= 0x2710)) {
			DebugPrintf("The specified item number is not an object\n");
		} else if ((charNum < PLAYER_ID) || (charNum >= 0x408) ||
				   (charHotspot == NULL)) {
			DebugPrintf("The specified character does not exist");
		} else {
			// Set the item's room number to be the destination character
			itemHotspot->roomNumber = charNum;
		}
	} else {
		DebugPrintf("Syntax: give <item_id> [<character_id>]\n");
	}

	return true;
}

bool Debugger::cmd_hotspots(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	Room &room = Room::getReference();
	char buffer[MAX_DESC_SIZE];
	
	if (argc > 1) {
		if (strcmp(argv[1], "active") == 0) {
			// Loop for displaying active hotspots
			HotspotList::iterator i;
			for (i = res.activeHotspots().begin(); i != res.activeHotspots().end(); ++i) {
				Hotspot *hotspot = *i;

				if (hotspot->nameId() == 0) strcpy(buffer, "none");
				else strings.getString(hotspot->nameId(), buffer);

				DebugPrintf("%4xh - %s pos=(%d,%d,%d)\n", hotspot->hotspotId(), buffer,
					hotspot->x(), hotspot->y(), hotspot->roomNumber());
			}
		} else {
			// Presume it's a room's hotspots
			uint16 roomNumber = (argc >= 3) ? strToInt(argv[2]) : room.roomNumber();

			HotspotDataList::iterator i;
			for (i = res.hotspotData().begin(); i != res.hotspotData().end(); ++i) {
				HotspotData *hotspot = *i;

				if (hotspot->roomNumber == roomNumber) {
					if (hotspot->nameId == 0) strcpy(buffer, "none");
					else strings.getString(hotspot->nameId, buffer);

					DebugPrintf("%4xh - %s pos=(%d,%d,%d)\n", hotspot->hotspotId, buffer,
					hotspot->startX, hotspot->startY, hotspot->roomNumber);
				}
			}
		}

	} else {
		DebugPrintf("Syntax: hotspots ['active' | ['room' | 'room' '<room_number>']]\n");
		DebugPrintf("Gives a list of all the currently active hotspots, or the hotspots\n");
		DebugPrintf("present in either the current room or a designated one\n");
	}

	return true;
}

bool Debugger::cmd_hotspot(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	StringList &stringList = res.stringList();
	char buffer[MAX_DESC_SIZE];
	HotspotData *hs;
	Hotspot *h;

	if (argc < 2) {
		DebugPrintf("hotspot <hotspot_id> ['paths' | 'schedule' | 'actions']\n");
		return true;
	} 
	hs = res.getHotspot(strToInt(argv[1]));
	if (!hs) {
		DebugPrintf("Unknown hotspot specified\n");
		return true;
	}

	h = res.getActiveHotspot(hs->hotspotId);
	if (argc == 2) {
		// Show the hotspot properties
		strings.getString(hs->nameId, buffer);
		DebugPrintf("name = %d - %s, descs = (%d,%d)\n", hs->nameId, buffer, 
			hs->descId, hs->descId2);
		DebugPrintf("actions = %xh, offset = %xh\n", hs->actions, hs->actionsOffset);
		DebugPrintf("flags = %xh, layer = %d\n", hs->flags, hs->layer);
		DebugPrintf("position = %d,%d,%d\n", hs->startX, hs->startY, hs->roomNumber);
		DebugPrintf("size = %d,%d, alt = %d,%d, yCorrection = %d\n", 
			hs->width, hs->height,  hs->widthCopy, hs->heightCopy, hs->yCorrection);
		DebugPrintf("Talk bubble offset = %d,%d\n", hs->talkX, hs->talkY);
		DebugPrintf("load offset = %xh, script load = %d\n", hs->loadOffset, hs->scriptLoadFlag);
		DebugPrintf("Animation Id = %xh, Colour offset = %d\n", hs->animRecordId, hs->colourOffset);
		DebugPrintf("Script offset = %xh, Tick Script offset = %xh\n", 
			hs->sequenceOffset, hs->tickSequenceOffset);
		DebugPrintf("Tick Proc offset = %xh\n", hs->tickProcOffset);
		DebugPrintf("Tick timeout = %d\n", hs->tickTimeout);
		DebugPrintf("NPC Shcedule = %xh\n", hs->npcSchedule);
		DebugPrintf("Character mode = %d, delay ctr = %d, pause ctr = %d\n", 
			hs->characterMode, hs->delayCtr, hs->pauseCtr);

		if (h != NULL) {
			DebugPrintf("Frame Number = %d of %d\n", h->frameNumber(), h->numFrames());
			DebugPrintf("Persistant = %s\n", h->persistant() ? "true" : "false");
		}

	} else if (strcmp(argv[2], "actions") == 0) {
			// List the action set for the character
			for (int action = GET; action <= EXAMINE; ++action) {
				uint16 offset = res.getHotspotAction(hs->actionsOffset, (Action) action);
				const char *actionStr = stringList.getString(action);

				if (offset >= 0x8000) {
					DebugPrintf("%s - Message %xh\n",  actionStr, offset & 0x7ff);
				} else if (offset != 0) {
					DebugPrintf("%s - Script %xh\n", actionStr, offset);
				}
			}
	} else {
		if (!h) 
			DebugPrintf("The specified hotspot is not currently active\n");
		else if (strcmp(argv[2], "paths") == 0) {
			// List any paths for a charcter
			h->pathFinder().list(buffer);
			DebugPrintf("%s", buffer);
		} 
		else if (strcmp(argv[2], "schedule") == 0) {
			// List any current schedule for the character
			h->currentActions().list(buffer);
			DebugPrintf("%s", buffer);
		}
		else if (strcmp(argv[2], "pixels") == 0) {
			// List the pixel data for the hotspot
			HotspotAnimData &pData = h->anim();
			DebugPrintf("Record Id = %xh\n", pData.animRecordId);
			DebugPrintf("Flags = %d\n", pData.flags);
			DebugPrintf("Frames: up=%d down=%d left=%d right=%d\n",
				pData.upFrame, pData.downFrame, pData.leftFrame, pData.rightFrame);		
			DebugPrintf("Current frame = %d of %d\n", h->frameNumber(), h->numFrames());
		}
	}

	DebugPrintf("\n");
	return true;
}

} // End of namespace Lure
