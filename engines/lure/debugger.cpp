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
 */


#include "common/config-manager.h"
#include "common/endian.h"
#include "lure/luredefs.h"
#include "lure/debugger.h"
#include "lure/decode.h"
#include "lure/res.h"
#include "lure/res_struct.h"
#include "lure/room.h"
#include "lure/strings.h"

namespace Lure {

Debugger::Debugger(): GUI::Debugger() {
	DCmd_Register("continue",			WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("enter",				WRAP_METHOD(Debugger, cmd_enterRoom));
	DCmd_Register("rooms",				WRAP_METHOD(Debugger, cmd_listRooms));
	DCmd_Register("fields",				WRAP_METHOD(Debugger, cmd_listFields));
	DCmd_Register("setfield",			WRAP_METHOD(Debugger, cmd_setField));
	DCmd_Register("queryfield",			WRAP_METHOD(Debugger, cmd_queryField));
	DCmd_Register("give",				WRAP_METHOD(Debugger, cmd_giveItem));
	DCmd_Register("hotspots",			WRAP_METHOD(Debugger, cmd_hotspots));
	DCmd_Register("hotspot",			WRAP_METHOD(Debugger, cmd_hotspot));
	DCmd_Register("room",				WRAP_METHOD(Debugger, cmd_room));
	DCmd_Register("showanim",			WRAP_METHOD(Debugger, cmd_showAnim));
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

bool Debugger::cmd_listRooms(int argc, const char **argv) {
	RoomDataList &rooms = Resources::getReference().roomData();
	StringData &strings = StringData::getReference();
	char buffer[MAX_DESC_SIZE];
	int ctr = 0;

	DebugPrintf("Available rooms are:\n");
	for (RoomDataList::iterator i = rooms.begin(); i != rooms.end(); ++i) {
		RoomData *room = *i;
		// Explictly note the second drawbridge room as "Alt"
		if (room->roomNumber == 49) {
			strings.getString(47, buffer);
			strcat(buffer, " (alt)");
		} else {
			strings.getString(room->roomNumber, buffer);
		}

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
		DebugPrintf("hotspot <hotspot_id> ['paths' | 'schedule' | 'actions' | 'activate' | 'deactivate']\n");
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
		DebugPrintf("Talk Script offset = %xh, Tick Script offset = %xh\n",
			hs->talkScriptOffset, hs->tickScriptOffset);
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
	} else if (strcmp(argv[2], "activate") == 0) {
		// Activate the hotspot
		res.activateHotspot(hs->hotspotId);
		hs->flags &= !HOTSPOTFLAG_MENU_EXCLUSION;
		DebugPrintf("Activated\n");

	} else if (strcmp(argv[2], "deactivate") == 0) {
		// Activate the hotspot
		res.deactivateHotspot(hs->hotspotId);
		hs->flags |= HOTSPOTFLAG_MENU_EXCLUSION;
		DebugPrintf("Deactivated\n");

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

const char *directionList[5] = {"UP", "DOWN", "LEFT", "RIGHT", "NONE"};

bool Debugger::cmd_room(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	char buffer[MAX_DESC_SIZE];

	if (argc < 2) {
		DebugPrintf("room <room_number>\n");
		return true;
	}
	int roomNumber = strToInt(argv[1]);
	RoomData *room = res.getRoom(roomNumber);
	if (!room) {
		DebugPrintf("Unknown room specified\n");
		return true;
	}

	// Show the room details
	strings.getString(roomNumber, buffer);
	DebugPrintf("room #%d - %s\n", roomNumber,  buffer);
	strings.getString(room->descId, buffer);
	DebugPrintf("%s\n", buffer);
	DebugPrintf("Horizontal clipping = %d->%d walk area=(%d,%d)-(%d,%d)\n",
		room->clippingXStart, room->clippingXEnd,
		room->walkBounds.left, room->walkBounds.top,
		room->walkBounds.right, room->walkBounds.bottom);

	DebugPrintf("Exit hotspots:");
	RoomExitHotspotList &exits = room->exitHotspots;
	if (exits.empty())
		DebugPrintf(" none\n");
	else {
		RoomExitHotspotList::iterator i;
		for (i = exits.begin(); i != exits.end(); ++i) {
			RoomExitHotspotData *rec = *i;

			DebugPrintf("\nArea - (%d,%d)-(%d,%d) Room=%d Cursor=%d Hotspot=%xh",
				rec->xs, rec->ys, rec->xe, rec->ye, rec->destRoomNumber, rec->cursorNum, rec->hotspotId);
		}

		DebugPrintf("\n");
	}

	DebugPrintf("Room exits:");
	if (room->exits.empty())
		DebugPrintf(" none\n");
	else {
		RoomExitList::iterator i2;
		for (i2 = room->exits.begin(); i2 != room->exits.end(); ++i2) {
			RoomExitData *rec2 = *i2;

			DebugPrintf("\nExit - (%d,%d)-(%d,%d) Dest=%d,(%d,%d) Dir=%s Sequence=%xh",
				rec2->xs, rec2->ys, rec2->xe, rec2->ye, rec2->roomNumber,
				rec2->x, rec2->y, directionList[rec2->direction], rec2->sequenceOffset);
		}

		DebugPrintf("\n");
	}

	return true;
}

bool Debugger::cmd_showAnim(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	if (argc < 2) {
		DebugPrintf("showAnim animId [[frame_width frame_height] | list]\n");
		return true;
	}

	// Get the animation Id
	int animId = strToInt(argv[1]);
	HotspotAnimData *data = res.getAnimation(animId);
	if (data == NULL) {
		DebugPrintf("No such animation Id exists\n");
		return true;
	}

	// Figure out the total size of the animation - this will be used for guestimating
	// frame sizes, or validating that a specified frame size is correct
	MemoryBlock *src = Disk::getReference().getEntry(data->animId);

	int numFrames = READ_LE_UINT16(src->data());
	uint16 *headerEntry = (uint16 *) (src->data() + 2);
	assert((numFrames >= 1) && (numFrames < 100));

	// Calculate total needed size for output and create memory block to hold it
	uint32 totalSize = 0;
	for (uint16 ctr = 0; ctr < numFrames; ++ctr, ++headerEntry) {
		totalSize += (READ_LE_UINT16(headerEntry) + 31) / 32;
	}
	totalSize = (totalSize + 0x81) << 4;
	MemoryBlock *dest = Memory::allocate(totalSize);

	uint32 srcStart = (numFrames + 1) * sizeof(uint16) + 6;
	uint32 destSize = AnimationDecoder::decode_data(src, dest, srcStart) - 0x40;

	// Figure out the frame size
	int frameSize;

	if ((data->flags & PIXELFLAG_HAS_TABLE) != 0) {
		// Table based animation, so get frame size from frame 1 offset
		frameSize = READ_LE_UINT16(src->data());
	} else {
		// Get frame size from dividing uncompressed size by number of frames
		frameSize = destSize / numFrames;
	}

	// Free up the data
	delete src;
	delete dest;

	int width, height;

	if (argc == 4) {
		// Width and height specified
		width = strToInt(argv[2]);
		height = strToInt(argv[3]);

		if ((width * height) != (frameSize * 2)) {
			DebugPrintf("Warning: Total size = %d, Frame size (%d,%d) * %d frames = %d bytes\n",
				destSize, width, height, numFrames, width * height * numFrames / 2);
		}
	} else {
		// Guestimate a frame size
		frameSize = destSize / numFrames;

		// Figure out the approximate starting point of a width 3/4 the frame size
		width = frameSize * 3 / 4;

		bool descFlag = (argc == 3);
		if (descFlag) DebugPrintf("Target size = %d\n", frameSize * 2);

		while ((width > 0) && (descFlag || (((frameSize * 2) % width) != 0))) {
			if (((frameSize * 2) % width) == 0)
				DebugPrintf("Frame size (%d,%d) found\n", width, frameSize * 2 / width);
			--width;
		}

		if (argc == 3) {
			DebugPrintf("Done\n");
			return true;
		} else if (width == 0) {
			DebugPrintf("Total size = %d, # frames = %d, frame Size = %d - No valid frame dimensions\n",
				destSize, numFrames, frameSize);
			return true;
		}

		height = (frameSize * 2) / width;
		DebugPrintf("# frames = %d, guestimated frame size = (%d,%d)\n",
			numFrames, width, height);
	}

	// Bottle object is used as a handy hotspot holder that doesn't have any
	// tick proc behaviour that we need to worry about
	Hotspot *hotspot = res.activateHotspot(BOTTLE_HOTSPOT_ID);
	hotspot->setLayer(0xfe);
	hotspot->setSize(width, height);

	Hotspot *player = res.activateHotspot(PLAYER_ID);
	hotspot->setColourOffset(player->resource()->colourOffset);

	hotspot->setAnimation(animId);

	DebugPrintf("Done\n");
	return true;
}

} // End of namespace Lure
