/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/endian.h"
#include "lure/luredefs.h"
#include "lure/debugger.h"
#include "lure/decode.h"
#include "lure/game.h"
#include "lure/res.h"
#include "lure/res_struct.h"
#include "lure/room.h"
#include "lure/scripts.h"
#include "lure/strings.h"

namespace Lure {

Debugger::Debugger(): GUI::Debugger() {
	registerCmd("continue",			WRAP_METHOD(Debugger, cmdExit));
	registerCmd("enter",				WRAP_METHOD(Debugger, cmd_enterRoom));
	registerCmd("rooms",				WRAP_METHOD(Debugger, cmd_listRooms));
	registerCmd("fields",				WRAP_METHOD(Debugger, cmd_listFields));
	registerCmd("setfield",			WRAP_METHOD(Debugger, cmd_setField));
	registerCmd("queryfield",			WRAP_METHOD(Debugger, cmd_queryField));
	registerCmd("give",				WRAP_METHOD(Debugger, cmd_giveItem));
	registerCmd("hotspots",			WRAP_METHOD(Debugger, cmd_hotspots));
	registerCmd("hotspot",			WRAP_METHOD(Debugger, cmd_hotspot));
	registerCmd("room",				WRAP_METHOD(Debugger, cmd_room));
	registerCmd("showanim",			WRAP_METHOD(Debugger, cmd_showAnim));
	registerCmd("strings",			WRAP_METHOD(Debugger, cmd_saveStrings));
	registerCmd("debug",				WRAP_METHOD(Debugger, cmd_debug));
	registerCmd("script",				WRAP_METHOD(Debugger, cmd_script));
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
		if (res.getRoom(roomNumber) == nullptr) {
			debugPrintf("specified number was not a valid room\n");
			return true;
		}

		if (argc > 2) {
			remoteFlag = strToInt(argv[2]);
		}

		room.leaveRoom();
		room.setRoomNumber(roomNumber);
		if (!remoteFlag)
			res.getActiveHotspot(PLAYER_ID)->setRoomNumber(roomNumber);

		detach();
		return false;
	}

	debugPrintf("Syntax: room <roomnum> [<remoteview>]\n");
	debugPrintf("A non-zero value for reomteview will change the room without ");
	debugPrintf("moving the player.\n");
	return true;
}

bool Debugger::cmd_listRooms(int argc, const char **argv) {
	RoomDataList &rooms = Resources::getReference().roomData();
	StringData &strings = StringData::getReference();
	char buffer[MAX_DESC_SIZE];
	int ctr = 0;

	debugPrintf("Available rooms are:\n");
	for (RoomDataList::iterator i = rooms.begin(); i != rooms.end(); ++i) {
		RoomData const &room = **i;
		// Explicitly note the second drawbridge room as "Alt"
		if (room.roomNumber == 49) {
			strings.getString(47, buffer);
			Common::strcat_s(buffer, " (alt)");
		} else {
			strings.getString(room.roomNumber, buffer);
		}

		debugPrintf("#%d - %s", room.roomNumber, buffer);

		if (++ctr % 3 == 0) debugPrintf("\n");
		else {
			// Write out spaces between columns
			int numSpaces = 25 - strlen(buffer) - (room.roomNumber >= 10 ? 2 : 1);
			char *s = buffer;
			while (numSpaces-- > 0) *s++ = ' ';
			*s = '\0';
			debugPrintf("%s", buffer);
		}
	}
	debugPrintf("\n");
	debugPrintf("Current room: %d\n", Room::getReference().roomNumber());

	return true;
}

bool Debugger::cmd_listFields(int argc, const char **argv) {
	ValueTableData &fields = Resources::getReference().fieldList();

	for (int ctr = 0; ctr < fields.size(); ++ctr) {
		debugPrintf("(%-2d): %-5d", ctr, fields.getField(ctr));
		if (!((ctr + 1) % 7))
			debugPrintf("\n");
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::cmd_setField(int argc, const char **argv) {
	ValueTableData &fields = Resources::getReference().fieldList();

	if (argc >= 3) {
		int fieldNum = strToInt(argv[1]);
		uint16 value = strToInt(argv[2]);

		if ((fieldNum < 0) || (fieldNum >= fields.size())) {
			// Invalid field number
			debugPrintf("Invalid field number specified\n");
		} else {
			// Set the field value
			fields.setField(fieldNum, value);
		}
	} else {
		debugPrintf("Syntax: setfield <field_number> <value>\n");
	}

	return true;
}

bool Debugger::cmd_queryField(int argc, const char **argv) {
	ValueTableData &fields = Resources::getReference().fieldList();

	if (argc > 1) {
		int fieldNum = strToInt(argv[1]);
		if ((fieldNum < 0) || (fieldNum >= fields.size())) {
			// Invalid field number
			debugPrintf("Invalid field number specified\n");
		} else {
			// Get the field value
			debugPrintf("Field %d is %d (%xh)\n", fieldNum,
				fields.getField(fieldNum), fields.getField(fieldNum));
		}
	} else {
		debugPrintf("Syntax: queryfield <field_num>\n");
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

		if (itemHotspot == nullptr) {
			debugPrintf("The specified item does not exist\n");
		} else if (itemNum < 0x408) {
			debugPrintf("The specified item number is not an object\n");
		} else if ((charNum < PLAYER_ID) || (charNum >= 0x408) ||
				   (charHotspot == nullptr)) {
			debugPrintf("The specified character does not exist");
		} else {
			// Set the item's room number to be the destination character
			itemHotspot->roomNumber = charNum;
		}
	} else {
		debugPrintf("Syntax: give <item_id> [<character_id>]\n");
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
				Hotspot const &hotspot = **i;

				if (hotspot.nameId() == 0) Common::strcpy_s(buffer, "none");
				else strings.getString(hotspot.nameId(), buffer);

				debugPrintf("%4xh - %s pos=(%d,%d,%d)\n", hotspot.hotspotId(), buffer,
					hotspot.x(), hotspot.y(), hotspot.roomNumber());
			}
		} else {
			// Presume it's a room's hotspots
			uint16 roomNumber = (argc >= 3) ? strToInt(argv[2]) : room.roomNumber();

			HotspotDataList::iterator i;
			for (i = res.hotspotData().begin(); i != res.hotspotData().end(); ++i) {
				HotspotData const &hotspot = **i;

				if (hotspot.roomNumber == roomNumber) {
					if (hotspot.nameId == 0) Common::strcpy_s(buffer, "none");
					else strings.getString(hotspot.nameId, buffer);

					debugPrintf("%4xh - %s pos=(%d,%d,%d)\n", hotspot.hotspotId, buffer,
					hotspot.startX, hotspot.startY, hotspot.roomNumber);
				}
			}
		}

	} else {
		debugPrintf("Syntax: hotspots ['active' | ['room' | 'room' '<room_number>']]\n");
		debugPrintf("Gives a list of all the currently active hotspots, or the hotspots\n");
		debugPrintf("present in either the current room or a designated one\n");
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
		debugPrintf("hotspot <hotspot_id> ['paths' | 'schedule' | 'actions' | 'activate' | 'deactivate' | 'setpos']\n");
		return true;
	}
	hs = res.getHotspot(strToInt(argv[1]));
	if (!hs) {
		debugPrintf("Unknown hotspot specified\n");
		return true;
	}

	h = res.getActiveHotspot(hs->hotspotId);
	if (argc == 2) {
		// Show the hotspot properties
		strings.getString(hs->nameId, buffer);
		debugPrintf("name = %d - %s, descs = (%d,%d)\n", hs->nameId, buffer,
			hs->descId, hs->descId2);
		debugPrintf("actions = %xh, offset = %xh\n", hs->actions, hs->actionsOffset);
		debugPrintf("flags = %xh, layer = %d\n", hs->flags, hs->layer);
		debugPrintf("position = %d,%d,%d\n", hs->startX, hs->startY, hs->roomNumber);
		debugPrintf("size = %d,%d, alt = %d,%d, yCorrection = %d\n",
			hs->width, hs->height,  hs->widthCopy, hs->heightCopy, hs->yCorrection);
		debugPrintf("Talk bubble offset = %d,%d\n", hs->talkX, hs->talkY);
		debugPrintf("load offset = %xh, script load = %d\n", hs->loadOffset, hs->scriptLoadFlag);
		debugPrintf("Animation Id = %xh, Color offset = %d\n", hs->animRecordId, hs->colorOffset);
		debugPrintf("Talk Script offset = %xh, Tick Script offset = %xh\n",
			hs->talkScriptOffset, hs->tickScriptOffset);
		debugPrintf("Tick Proc offset = %xh\n", hs->tickProcId);
		debugPrintf("Tick timeout = %d\n", hs->tickTimeout);
		debugPrintf("Character mode = %d, delay ctr = %d, pause ctr = %d\n",
			hs->characterMode, hs->delayCtr, hs->pauseCtr);

		if (h != nullptr) {
			debugPrintf("Frame Number = %d of %d\n", h->frameNumber(), h->numFrames());
			debugPrintf("Persistent = %s\n", h->persistent() ? "true" : "false");
		}

	} else if (strcmp(argv[2], "actions") == 0) {
			// List the action set for the character
			for (int action = GET; action <= EXAMINE; ++action) {
				uint16 offset = res.getHotspotAction(hs->actionsOffset, (Action) action);
				const char *actionStr = stringList.getString(action);

				if (offset >= 0x8000) {
					debugPrintf("%s - Message %xh\n",  actionStr, offset & 0x7ff);
				} else if (offset != 0) {
					debugPrintf("%s - Script %xh\n", actionStr, offset);
				}
			}
	} else if (strcmp(argv[2], "activate") == 0) {
		// Activate the hotspot
		res.activateHotspot(hs->hotspotId);
		hs->flags &= ~HOTSPOTFLAG_MENU_EXCLUSION;
		debugPrintf("Activated\n");

	} else if (strcmp(argv[2], "deactivate") == 0) {
		// Deactivate the hotspot
		res.deactivateHotspot(hs->hotspotId);
		hs->flags |= HOTSPOTFLAG_MENU_EXCLUSION;
		debugPrintf("Deactivated\n");

	} else {
		if (strcmp(argv[2], "schedule") == 0) {
			// List any current schedule for the character
			debugPrintf("%s", hs->npcSchedule.getDebugInfo().c_str());
		}
		if (!h)
			debugPrintf("The specified hotspot is not currently active\n");
		else if (strcmp(argv[2], "paths") == 0) {
			// List any paths for a charcter
			debugPrintf("%s", h->pathFinder().getDebugInfo().c_str());
		}
		else if (strcmp(argv[2], "pixels") == 0) {
			// List the pixel data for the hotspot
			HotspotAnimData &pData = h->anim();
			debugPrintf("Record Id = %xh\n", pData.animRecordId);
			debugPrintf("Flags = %d\n", pData.flags);
			debugPrintf("Frames: up=%d down=%d left=%d right=%d\n",
				pData.upFrame, pData.downFrame, pData.leftFrame, pData.rightFrame);
			debugPrintf("Current frame = %d of %d\n", h->frameNumber(), h->numFrames());
		}
		else if (strcmp(argv[2], "setpos") == 0) {
			// Set the hotspot position
			if (argc >= 5)
				h->setPosition(strToInt(argv[3]), strToInt(argv[4]));
			if (argc >= 6)
				h->setRoomNumber(strToInt(argv[5]));
			debugPrintf("Done.\n");
		}
	}

	debugPrintf("\n");
	return true;
}

const char *directionList[5] = {"UP", "DOWN", "LEFT", "RIGHT", "NONE"};

bool Debugger::cmd_room(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	char buffer[MAX_DESC_SIZE];

	if (argc < 2) {
		debugPrintf("room <room_number>\n");
		return true;
	}
	int roomNumber = strToInt(argv[1]);
	RoomData *room = res.getRoom(roomNumber);
	if (!room) {
		debugPrintf("Unknown room specified\n");
		return true;
	}

	// Show the room details
	strings.getString(roomNumber, buffer);
	debugPrintf("room #%d - %s\n", roomNumber,  buffer);
	strings.getString(room->descId, buffer);
	debugPrintf("%s\n", buffer);
	debugPrintf("Horizontal clipping = %d->%d walk area=(%d,%d)-(%d,%d)\n",
		room->clippingXStart, room->clippingXEnd,
		room->walkBounds.left, room->walkBounds.top,
		room->walkBounds.right, room->walkBounds.bottom);

	debugPrintf("Exit hotspots:");
	RoomExitHotspotList &exits = room->exitHotspots;
	if (exits.empty())
		debugPrintf(" none\n");
	else {
		RoomExitHotspotList::iterator i;
		for (i = exits.begin(); i != exits.end(); ++i) {
			RoomExitHotspotData const &rec = **i;

			debugPrintf("\nArea - (%d,%d)-(%d,%d) Room=%d Cursor=%d Hotspot=%xh",
				rec.xs, rec.ys, rec.xe, rec.ye, rec.destRoomNumber, rec.cursorNum, rec.hotspotId);
		}

		debugPrintf("\n");
	}

	debugPrintf("Room exits:");
	if (room->exits.empty())
		debugPrintf(" none\n");
	else {
		RoomExitList::iterator i2;
		for (i2 = room->exits.begin(); i2 != room->exits.end(); ++i2) {
			RoomExitData const &rec2 = **i2;

			debugPrintf("\nExit - (%d,%d)-(%d,%d) Dest=%d,(%d,%d) Dir=%s Sequence=%xh",
				rec2.xs, rec2.ys, rec2.xe, rec2.ye, rec2.roomNumber,
				rec2.x, rec2.y, directionList[rec2.direction], rec2.sequenceOffset);
		}

		debugPrintf("\n");
	}

	return true;
}

bool Debugger::cmd_showAnim(int argc, const char **argv) {
	Resources &res = Resources::getReference();
	if (argc < 2) {
		debugPrintf("showAnim animId [[frame_width frame_height] | list]\n");
		return true;
	}

	// Get the animation Id
	int animId = strToInt(argv[1]);
	HotspotAnimData *data = res.getAnimation(animId);
	if (data == nullptr) {
		debugPrintf("No such animation Id exists\n");
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
			debugPrintf("Warning: Total size = %d, Frame size (%d,%d) * %d frames = %d bytes\n",
				destSize, width, height, numFrames, width * height * numFrames / 2);
		}
	} else {
		// Guestimate a frame size
		frameSize = destSize / numFrames;

		// Figure out the approximate starting point of a width 3/4 the frame size
		width = frameSize * 3 / 4;

		bool descFlag = (argc == 3);
		if (descFlag) debugPrintf("Target size = %d\n", frameSize * 2);

		while ((width > 0) && (descFlag || (((frameSize * 2) % width) != 0))) {
			if (((frameSize * 2) % width) == 0)
				debugPrintf("Frame size (%d,%d) found\n", width, frameSize * 2 / width);
			--width;
		}

		if (argc == 3) {
			debugPrintf("Done\n");
			return true;
		} else if (width == 0) {
			debugPrintf("Total size = %d, # frames = %d, frame Size = %d - No valid frame dimensions\n",
				destSize, numFrames, frameSize);
			return true;
		}

		height = (frameSize * 2) / width;
		debugPrintf("# frames = %d, guestimated frame size = (%d,%d)\n",
			numFrames, width, height);
	}

	// Bottle object is used as a handy hotspot holder that doesn't have any
	// tick proc behavior that we need to worry about
	Hotspot *hotspot = res.activateHotspot(BOTTLE_HOTSPOT_ID);
	hotspot->setLayer(0xfe);
	hotspot->setSize(width, height);

	Hotspot *player = res.activateHotspot(PLAYER_ID);
	hotspot->setColorOffset(player->resource()->colorOffset);

	hotspot->setAnimation(animId);

	debugPrintf("Done\n");
	return true;
}

bool Debugger::cmd_saveStrings(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("strings <stringId>\n");
		return true;
	}

	StringData &strings = StringData::getReference();

	char *buffer = (char *)malloc(32768);
	if (!buffer) {
		debugPrintf("Cannot allocate strings buffer\n");
		return true;
	}

	uint16 id = strToInt(argv[1]);
	strings.getString(id, buffer);
	debugPrintf("%s\n", buffer);

/* Commented out code for saving all text strings - note that 0x1000 is chosen
 * arbitrarily, so there'll be a bunch of garbage at the end, or the game will crash

	// Save all the strings to a text file - this

	FILE *f = fopen("strings.txt", "w");

	for (int index = 0; index < 0x1000; ++index) {
		strings.getString(index, buffer);
		fprintf(f, "%.4xh - %s\n", index, buffer);
	}

	fclose(f);

	debugPrintf("Done\n");
*/

	free(buffer);

	return true;
}

bool Debugger::cmd_debug(int argc, const char **argv) {
	Game &game = Game::getReference();
	Room &room = Room::getReference();

	if ((argc == 2) && (strcmp(argv[1], "on") == 0)) {
		debugPrintf("debug keys are on\n");
		game.debugFlag() = true;

	} else if ((argc == 2) && (strcmp(argv[1], "off") == 0)) {
		debugPrintf("debug keys are off\n");
		game.debugFlag() = false;
		room.setShowInfo(false);

	} else {
		debugPrintf("debug [on | off]]\n");
	}

	return true;
}

bool Debugger::cmd_script(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("script <script number> [param 1] [param 2] [param 3] [exit flag]\n");
		return true;
	}

	int scriptNumber = strToInt(argv[1]);
	if ((scriptNumber < 0) || (scriptNumber > 66)) {
		debugPrintf("An invalid script number was specified\n");
		return true;
	}

	uint16 param1 = 0, param2 = 0, param3 = 0;
	if (argc >= 3)
		param1 = strToInt(argv[2]);
	if (argc >= 4)
		param2 = strToInt(argv[3]);
	if (argc >= 5)
		param3 = strToInt(argv[4]);

	Script::executeMethod(scriptNumber, param1, param2, param3);
	debugPrintf("Script executed\n");
	return true;
}

} // End of namespace Lure
