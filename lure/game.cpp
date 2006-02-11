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
	_remoteView = false;
}

void Game::nextFrame() {
	Resources &r = Resources::getReference();
	HotspotList::iterator i = r.activeHotspots().begin();
	HotspotList::iterator iTemp;

	// Note the somewhat more complicated loop style as a hotspot tick handler may
	// unload the hotspot and accompanying record
	for (; i != r.activeHotspots().end(); i = iTemp) {
		iTemp = i;
		++iTemp;
		Hotspot &h = *i.operator*();
		h.tick();
	}
}

void Game::execute() {
	OSystem &system = System::getReference();
	Room &r = Room::getReference();
	Resources &res = Resources::getReference();
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();
	Menu &menu = Menu::getReference();
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
		r.update();
		system.delayMillis(10);

		while (events.pollEvent()) {
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

			if (mouse.y() < MENUBAR_Y_SIZE) 
			{
				if (mouse.getCursorNum() != CURSOR_MENUBAR) mouse.setCursorNum(CURSOR_MENUBAR);
				if ((mouse.getCursorNum() == CURSOR_MENUBAR) && mouse.lButton())
				{
					uint8 responseId = menu.execute();
					mouse.setCursorNum((mouse.y() < MENUBAR_Y_SIZE) ? CURSOR_MENUBAR : CURSOR_ARROW);
					if (responseId != MENUITEM_NONE)
						handleMenuResponse(responseId);
				}
			} else {
				if (mouse.getCursorNum() == CURSOR_MENUBAR) mouse.setCursorNum(CURSOR_ARROW);
				
				if (events.type() == OSystem::EVENT_MOUSEMOVE)
					r.cursorMoved();

				if (mouse.rButton()) handleRightClickMenu();
				else if (mouse.lButton()) handleLeftClick();
			}
		}

		uint16 destRoom = fields.getField(NEW_ROOM_NUMBER);
		if (_remoteView && (destRoom != 0)) {
			// Show a remote view of the specified room
			uint16 currentRoom = r.roomNumber();
			r.setRoomNumber(destRoom, true);

			// This code eventually needs to be moved into the main loop so that,
			// amongst other things, the tick handlers controlling animation can work
			while (!events.quitFlag && !mouse.lButton() && !mouse.rButton()) {
				while (events.pollEvent()) {
					if ((events.type() == OSystem::EVENT_KEYDOWN) && 
						(events.event().kbd.ascii == 27))
						events.quitFlag = true;
					if (events.type() == OSystem::EVENT_MOUSEMOVE)
						r.cursorMoved();
				}

				if (system.getMillis() > timerVal + GAME_FRAME_DELAY) {
					timerVal = system.getMillis();
					nextFrame();			
				}
				res.delayList().tick();
				r.update();
				system.delayMillis(10);
			}

			fields.setField(NEW_ROOM_NUMBER, 0);
			Hotspot *player = res.getActiveHotspot(PLAYER_ID);
			player->setTickProc(0x5e44); // reattach player handler
			_remoteView = false;
			r.setRoomNumber(currentRoom);
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

void Game::handleRightClickMenu() {
	Room &r = Room::getReference();
	Resources &res = Resources::getReference();
	ValueTableData &fields = Resources::getReference().fieldList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	HotspotData *hotspot;
	Action action;
	uint32 actions;
	uint16 itemId;

	if (r.hotspotId() != 0) {
		// Get hotspot actions
		actions = r.hotspotActions();
	} else {
		// Standard actions - drink, examine, look, status
		actions = 0x1184000;
	}

	// If no inventory items remove entries that require them
	if (res.numInventoryItems() == 0) 
		actions &= 0xFEF3F9FD;

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
				hotspot = res.getHotspot(r.hotspotId());
			itemId = PopupMenu::ShowInventory();
			breakFlag = (itemId != 0xffff);
			if (breakFlag) 
				fields.setField(USE_HOTSPOT_ID, itemId);
			break;

		default:
			hotspot = res.getHotspot(r.hotspotId());
			breakFlag = true;
			break;
		}
	}

	// Set fields used by the script interpreter
	fields.setField(CHARACTER_HOTSPOT_ID, PLAYER_ID);
	if (hotspot) {
		fields.setField(ACTIVE_HOTSPOT_ID, hotspot->hotspotId);
		if ((action != USE) && (action != GIVE)) {
			fields.setField(USE_HOTSPOT_ID, hotspot->hotspotId);
		}
	}

	if (action != NONE)
		player->doAction(action, hotspot);
}

void Game::handleLeftClick() {
	Room &room = Room::getReference();
	Mouse &mouse = Mouse::getReference();
	Resources &resources = Resources::getReference();

	if (room.hotspotId()) {
		// Handle look at hotspot
		HotspotData *hs = resources.getHotspot(room.hotspotId());
		Hotspot *player = resources.getActiveHotspot(PLAYER_ID);
		room.setAction(LOOK_AT);
		room.update();
		player->doAction(LOOK_AT, hs);
		room.setAction(NONE);
	} else {
		// Walk to mouse click. TODO: still need to recognise other actions,
		// such as to room exits or closing an on-screen floating dialog
		Hotspot *hs =  resources.getActiveHotspot(PLAYER_ID);
		hs->walkTo(mouse.x(), mouse.y(), 0);
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
	OSystem &system = System::getReference();
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
		system.delayMillis(10);
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
