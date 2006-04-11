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

#include "lure/game.h"
#include "lure/strings.h"
#include "lure/room.h"
#include "lure/system.h"
#include "lure/debug-input.h"
#include "lure/debug-methods.h"
#include "lure/scripts.h"
#include "lure/res_struct.h"

namespace Lure {

static Game *int_game = NULL;

Game &Game::getReference() {
	return *int_game;
}

Game::Game() {
	int_game = this;
	_slowSpeedFlag = true;
	_soundFlag = true;
}

void Game::nextFrame() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	HotspotList::iterator i;

	room.checkCursor();
	room.update();

	// Call the tick method for each hotspot - this is somewaht complicated
	// by the fact that a tick proc can unload both itself and/or others,
	// so we first get a list of the Ids, and call the tick proc for each 
	// id in sequence if it's still active

	uint16 *idList = new uint16[res.activeHotspots().size()];
	int idSize = 0;
	for (i = res.activeHotspots().begin(); i != res.activeHotspots().end(); ++i) {
		Hotspot *hotspot = *i;
		idList[idSize++] = hotspot->hotspotId();
	}

	for (int idCtr = 0; idCtr < idSize; ++idCtr) {
		Hotspot *hotspot = res.getActiveHotspot(idList[idCtr]);
		if (hotspot)
			hotspot->tick();
	}

	delete[] idList;
	Screen::getReference().update();
}

void Game::execute() {
	OSystem &system = System::getReference();
	Room &r = Room::getReference();
	Resources &res = Resources::getReference();
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();
	//Menu &menu = Menu::getReference();
	ValueTableData &fields = res.fieldList();

	uint32 timerVal = system.getMillis();

	screen.empty();
	//_screen.resetPalette();
	screen.setPaletteEmpty();

	Script::execute(STARTUP_SCRIPT);

	// Load the first room
	r.setRoomNumber(1);

	// Set the player direction
	res.getActiveHotspot(PLAYER_ID)->setDirection(UP);

	r.update();
	mouse.setCursorNum(CURSOR_ARROW);
	mouse.cursorOn();
	
	while (!events.quitFlag) {
		// If time for next frame, allow everything to update
		if (system.getMillis() > timerVal + GAME_FRAME_DELAY) {
			timerVal = system.getMillis();
			nextFrame();			
		}
		res.delayList().tick();

		if (events.pollEvent()) {
			if (events.type() == OSystem::EVENT_KEYDOWN) {
				uint16 roomNum = r.roomNumber();

#ifdef LURE_DEBUG
				if (events.event().kbd.keycode == 282) {
					doDebugMenu();
					continue;
				}
#endif

				switch (events.event().kbd.ascii) {
				case 27:
					events.quitFlag = true;
					break;

#ifdef LURE_DEBUG
				case '+':
					while (++roomNum <= 51) 
						if (res.getRoom(roomNum) != NULL) break; 
					if (roomNum == 52) roomNum = 1;

					r.leaveRoom();
					r.setRoomNumber(roomNum);
					break;

				case '-':
					if (roomNum == 1) roomNum = 55;
					while (res.getRoom(--roomNum) == NULL) ;

					r.leaveRoom();
					r.setRoomNumber(roomNum);
					break;

				case '*':
					res.getActiveHotspot(PLAYER_ID)->setRoomNumber(
						r.roomNumber());
					break;
#endif
				default:
					break;
				}
			}

			if ((events.type() == OSystem::EVENT_LBUTTONDOWN) ||
				(events.type() == OSystem::EVENT_RBUTTONDOWN)) 
				handleClick();
		}

		uint16 destRoom;
		destRoom = fields.getField(NEW_ROOM_NUMBER);
		if (destRoom != 0) {
			// Need to change the current room
			bool remoteFlag = fields.getField(OLD_ROOM_NUMBER) != 0;
			r.setRoomNumber(destRoom, remoteFlag);
			fields.setField(NEW_ROOM_NUMBER, 0);
		}

		destRoom = fields.playerNewPos().roomNumber;
		if (destRoom != 0) {
			playerChangeRoom();
		}
	}

	r.leaveRoom();
}

#ifdef LURE_DEBUG

#define NUM_DEBUG_ITEMS 4
const char *debugItems[NUM_DEBUG_ITEMS] = 
		{"Toggle Info", "Set Room", "Show Active HS", "Show Room HS"};

void Game::doDebugMenu() {
	uint16 index = PopupMenu::Show(NUM_DEBUG_ITEMS, debugItems);
	Room &r = Room::getReference();
	Resources &res = Resources::getReference();
	
	switch (index) {
	case 0:
		// Toggle co-ordinates
		r.setShowInfo(!r.showInfo());
		break;

	case 1:
		// Set room number:
		uint32 roomNumber;
		if (!input_integer("Enter room number:", roomNumber)) return;
		if (res.getRoom(roomNumber))
			r.setRoomNumber(roomNumber);
		else
			Dialog::show("The room does not exist");
		break;

	case 2:
		// Show active hotspots
        showActiveHotspots();
		break;

	case 3:
		// Show hotspots in room
		showRoomHotspots();
		break;

	default:
		break;
	}
}

#endif

void Game::handleMenuResponse(uint8 selection) {
	switch (selection) {
	case MENUITEM_CREDITS:
		doShowCredits();
		break;

	case MENUITEM_RESTART_GAME: 
	case MENUITEM_SAVE_GAME:
	case MENUITEM_RESTORE_GAME: 
		break;

	case MENUITEM_QUIT:
		doQuit();
		break;

	case MENUITEM_TEXT_SPEED:
		doTextSpeed();
		break;

	case MENUITEM_SOUND:
		doSound();
	}
}

void Game::playerChangeRoom() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	uint16 roomNum = fields.playerNewPos().roomNumber;
	fields.playerNewPos().roomNumber = 0;
	Point &newPos = fields.playerNewPos().position;

	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	player->currentActions().clear();
	player->setRoomNumber(roomNum);
	//player->setPosition((newPos.x & 0xfff8) || 5, newPos.y & 0xfff8);
	player->setPosition(newPos.x, newPos.y);
	room.setRoomNumber(roomNum, false);
}

void Game::handleClick() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	Mouse &mouse = Mouse::getReference();
	uint16 oldRoomNumber = fields.getField(OLD_ROOM_NUMBER);

	if (room.checkInTalkDialog()) {
		// Close the active talk dialog
		room.setTalkDialog(0, 0);
	} else if (oldRoomNumber != 0) {
		// Viewing a room remotely - handle returning to prior room
		if ((room.roomNumber() != 35) || (fields.getField(87) == 0)) {
			// Reset player tick proc and signal to change back to the old room
			res.getActiveHotspot(PLAYER_ID)->setTickProc(PLAYER_TICK_PROC_ID); 
			fields.setField(NEW_ROOM_NUMBER, oldRoomNumber);
			fields.setField(OLD_ROOM_NUMBER, 0);
		}
	} else if (res.getTalkState() != TALK_NONE) {
		// Currently talking, so let it's tick proc handle it
	} else if (mouse.y() < MENUBAR_Y_SIZE) {
		uint8 response = Menu::getReference().execute();
		if (response != MENUITEM_NONE)
			handleMenuResponse(response);
	} else {
		if (mouse.lButton())
			handleLeftClick();
		else
			handleRightClickMenu();
	}
}

void Game::handleRightClickMenu() {
	Room &room = Room::getReference();
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	HotspotData *hotspot;
	Action action;
	uint32 actions;
	uint16 itemId;

	if (room.hotspotId() != 0) {
		// Get hotspot actions
		actions = room.hotspotActions();
	} else {
		// Standard actions - drink, examine, look, status
		actions = 0x1184000;
	}

	// If no inventory items remove entries that require them
	if (res.numInventoryItems() == 0) 
		actions &= 0xFEF3F9FD;

	// If the player hasn't any money, remove any bribe entry
	if (res.fieldList().numGroats() == 0)
		actions &= 0xFF7FFFFF;

	action = NONE;
	hotspot = NULL;

	bool breakFlag = false;
	while (!breakFlag) {
		action = PopupMenu::Show(actions);

		switch (action) {
		case LOOK:
		case STATUS:
			breakFlag = true;
			break;

		case GIVE:
		case USE:
		case EXAMINE:
		case DRINK:
			if (action != DRINK)
				hotspot = res.getHotspot(room.hotspotId());
			itemId = PopupMenu::ShowInventory();
			breakFlag = (itemId != 0xffff);
			if (breakFlag) 
				fields.setField(USE_HOTSPOT_ID, itemId);
			break;

		default:
			hotspot = res.getHotspot(room.hotspotId());
			breakFlag = true;
			break;
		}
	}

	// Set fields used by the script interpreter
	fields.setField(CHARACTER_HOTSPOT_ID, PLAYER_ID);
	if (hotspot) {
		fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
		if ((action != USE) && (action != GIVE)) {
			fields.setField(USE_HOTSPOT_ID, 0xffff);
		} 
	}

	if (action != NONE) {
		res.setCurrentAction(action);
		room.update();
		Screen::getReference().update();
		player->doAction(action, hotspot);

		if (action != TALK_TO)
			res.setCurrentAction(NONE);
	}
}

void Game::handleLeftClick() {
	Room &room = Room::getReference();
	Mouse &mouse = Mouse::getReference();
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);

	if ((room.destRoomNumber() == 0) && (room.hotspotId() != 0)) {	
		// Handle look at hotspot
		HotspotData *hs = res.getHotspot(room.hotspotId());

		fields.setField(CHARACTER_HOTSPOT_ID, PLAYER_ID);
		fields.setField(ACTIVE_HOTSPOT_ID, hs->hotspotId);
		fields.setField(USE_HOTSPOT_ID, 0xffff);

		res.setCurrentAction(LOOK_AT);
		room.update();
		Screen::getReference().update();
		player->doAction(LOOK_AT, hs);
		res.setCurrentAction(NONE);
	} else if (room.destRoomNumber() != 0) {
		// Walk to another room
		RoomExitCoordinateData &exitData = 
			res.coordinateList().getEntry(room.roomNumber()).getData(room.destRoomNumber());

		player->walkTo((exitData.x & 0xfff8) | 5, (exitData.y & 0xfff8), 
			room.hotspotId() == 0 ? 0xffff : room.hotspotId());
	} else {
		// Walking within room
		player->walkTo(mouse.x(), mouse.y(), 0);
	}
}

void Game::doShowCredits() {
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();

	mouse.cursorOff();
	Palette p(CREDITS_RESOURCE_ID - 1);
	Surface *s = Surface::getScreen(CREDITS_RESOURCE_ID);
	screen.setPalette(&p);	
	s->copyToScreen(0, 0);
	delete s;

	events.waitForPress();

	screen.resetPalette();
	screen.update();
	mouse.cursorOn();
}

void Game::doQuit() {
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();

	mouse.cursorOff();
	Surface *s = Surface::newDialog(190, "Are you sure (y/n)?");
	s->centerOnScreen();
	delete s;

	char key = '\0';
	do {
		if (events.pollEvent()) {
			if (events.event().type == OSystem::EVENT_KEYDOWN) {
				key = events.event().kbd.ascii;
				if ((key >= 'A') && (key <= 'Z')) key += 'a' - 'A';
			}
		}
	} while (((uint8) key != 27) && (key != 'y') && (key != 'n'));

	events.quitFlag = key == 'y';
	if (!events.quitFlag) {
		screen.update();
		mouse.cursorOn();
	}
}

void Game::doTextSpeed() {
	Menu &menu = Menu::getReference();

	_slowSpeedFlag = !_slowSpeedFlag;
	const char *pSrc = _slowSpeedFlag ? "Slow" : "Fast";
	char *pDest = menu.getMenu(2).getEntry(1);
	memcpy(pDest, pSrc, 4);
}

void Game::doSound() {
	Menu &menu = Menu::getReference();

	_soundFlag = !_soundFlag;
	const char *pSrc = _soundFlag ? "on " : "off";
	char *pDest = menu.getMenu(2).getEntry(2) + 6;
	memcpy(pDest, pSrc, 3);
}

} // end of namespace Lure
