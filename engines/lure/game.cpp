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
#include "lure/scripts.h"
#include "lure/res_struct.h"
#include "lure/animseq.h"

#include "common/config-manager.h"

namespace Lure {

static Game *int_game = NULL;

Game &Game::getReference() {
	return *int_game;
}

Game::Game() {
	int_game = this;
	_debugger = new Debugger();
	_slowSpeedFlag = true;
	_soundFlag = true;
}

Game::~Game() {
	delete _debugger;
}

void Game::nextFrame() {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Room &room = Room::getReference();
	HotspotList::iterator i;

	res.pausedList().countdown();
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

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot ticks begin");
	for (int idCtr = 0; idCtr < idSize; ++idCtr) {
		Hotspot *hotspot = res.getActiveHotspot(idList[idCtr]);
		if (hotspot) {
			fields.setField(CHARACTER_HOTSPOT_ID, hotspot->hotspotId());
			hotspot->tick();
		}
	}
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot ticks end");

	delete[] idList;
	Screen::getReference().update();
}

void Game::execute() {
	OSystem &system = System::getReference();
	Room &room = Room::getReference();
	Resources &res = Resources::getReference();
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();
	ValueTableData &fields = res.fieldList();

	uint32 timerVal = system.getMillis();

	screen.empty();
	//_screen.resetPalette();
	screen.setPaletteEmpty();

	setState(0);

	Script::execute(STARTUP_SCRIPT);

	int bootParam = ConfMan.getInt("boot_param");
	handleBootParam(bootParam);

	// Set the player direction
	res.getActiveHotspot(PLAYER_ID)->setDirection(UP);

	room.update();
	mouse.setCursorNum(CURSOR_ARROW);
	mouse.cursorOn();
	
	while (!events.quitFlag) {
		while (!events.quitFlag && (_state == 0)) {
			// If time for next frame, allow everything to update
			if (system.getMillis() > timerVal + GAME_FRAME_DELAY) {
				timerVal = system.getMillis();
				nextFrame();
			}

			res.delayList().tick();

			while (events.pollEvent()) {
				if (events.type() == OSystem::EVENT_KEYDOWN) {
					uint16 roomNum = room.roomNumber();

					if ((events.event().kbd.flags == OSystem::KBD_CTRL) &&
						(events.event().kbd.keycode == 'd')) {
						// Activate the debugger
						_debugger->attach();
						break;
					}

					switch (events.event().kbd.ascii) {
					case 27:
						events.quitFlag = true;
						break;

					case '+':
						while (++roomNum <= 51) 
							if (res.getRoom(roomNum) != NULL) break; 
						if (roomNum == 52) roomNum = 1;

						room.leaveRoom();
						room.setRoomNumber(roomNum);
						break;

					case '-':
						if (roomNum == 1) roomNum = 55;
						while (res.getRoom(--roomNum) == NULL) ;

						room.leaveRoom();
						room.setRoomNumber(roomNum);
						break;

					case '*':
						res.getActiveHotspot(PLAYER_ID)->setRoomNumber(
							room.roomNumber());
						break;

					case 267:  // keypad '/'
						room.setShowInfo(!room.showInfo());
						break;

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
				strcpy(room.statusLine(), "");
				bool remoteFlag = fields.getField(OLD_ROOM_NUMBER) != 0;
				room.setRoomNumber(destRoom, remoteFlag);
				fields.setField(NEW_ROOM_NUMBER, 0);
			}

			destRoom = fields.playerNewPos().roomNumber;
			if (destRoom != 0) {
				playerChangeRoom();
			}

			system.updateScreen();
			system.delayMillis(10);

			if (_debugger->isAttached())
				_debugger->onFrame();
		}

		// If Skorl catches player, show the catching animation
		if ((_state & GS_CAUGHT) != 0) {
			Palette palette(SKORL_CATCH_PALETTE_ID);
			AnimationSequence *anim = new AnimationSequence(screen, system, 
				SKORL_CATCH_ANIM_ID, palette, false);
			mouse.cursorOff();
			anim->show();
			mouse.cursorOn();
		}

		// If the Restart/Restore dialog is needed, show it
		if ((_state & GS_RESTORE_RESTART) != 0) {
			// TODO: Restore/Restart dialog - for now, simply flag for exit
			events.quitFlag = true;
		}
	}

	room.leaveRoom();
}

void Game::handleMenuResponse(uint8 selection) {
	Common::String filename;

	switch (selection) {
	case MENUITEM_CREDITS:
		doShowCredits();
		break;

	case MENUITEM_RESTART_GAME: 
		break;

	case MENUITEM_SAVE_GAME:
		SaveRestoreDialog::show(true);
		break;

	case MENUITEM_RESTORE_GAME: 
		SaveRestoreDialog::show(false);
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
	SequenceDelayList &delayList = Resources::getReference().delayList();

	uint16 roomNum = fields.playerNewPos().roomNumber;
	fields.playerNewPos().roomNumber = 0;
	Point &newPos = fields.playerNewPos().position;
	delayList.clear();

	RoomData *roomData = res.getRoom(roomNum);
	assert(roomData);
	roomData->flags |= HOTSPOTFLAG_FOUND;

	// Check for any room change animation

	int animFlag = fields.getField(ROOM_EXIT_ANIMATION);
	if (animFlag == 1)
		displayChuteAnimation();
	else if (animFlag != 0)
		displayBarrelAnimation();
	fields.setField(ROOM_EXIT_ANIMATION, 0);

	// Change to the new room
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	player->currentActions().clear();
	player->setRoomNumber(roomNum);
	//player->setPosition((newPos.x & 0xfff8) || 5, newPos.y & 0xfff8);
	player->setPosition(newPos.x, newPos.y);
	player->setOccupied(true);
	room.setRoomNumber(roomNum, false);
}

void Game::displayChuteAnimation()
{
	OSystem &system = System::getReference();
	Resources &res = Resources::getReference();
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();

	ValueTableData &fields = res.fieldList();
	Palette palette(CHUTE_PALETTE_ID);

	debugC(ERROR_INTERMEDIATE, kLureDebugAnimations, "Starting chute animation");
	mouse.cursorOff();

	AnimationSequence *anim = new AnimationSequence(screen, system, 
		CHUTE_ANIM_ID, palette, false);
	anim->show();
	delete anim;
	
	anim = new AnimationSequence(screen, system, CHUTE2_ANIM_ID, 
		palette, false);	
	anim->show();
	delete anim;

	anim = new AnimationSequence(screen, system, CHUTE3_ANIM_ID, 
		palette, false);	
	anim->show();
	delete anim;

	mouse.cursorOn();
	fields.setField(82, 1);
}

void Game::displayBarrelAnimation()
{
	OSystem &system = System::getReference();
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();

	debugC(ERROR_INTERMEDIATE, kLureDebugAnimations, "Starting barrel animation");
	Palette palette(BARREL_PALETTE_ID);
	AnimationSequence *anim = new AnimationSequence(screen, system, 
		BARREL_ANIM_ID, palette, false);
	mouse.cursorOff();
	anim->show();
	mouse.cursorOn();
}

void Game::handleClick() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	Mouse &mouse = Mouse::getReference();
	uint16 oldRoomNumber = fields.getField(OLD_ROOM_NUMBER);

	if (room.checkInTalkDialog()) {
		// Close the active talk dialog
		room.setTalkDialog(0, 0, 0, 0);
	} else if (oldRoomNumber != 0) {
		// Viewing a room remotely - handle returning to prior room
		if ((room.roomNumber() != 35) || (fields.getField(87) == 0)) {
			// Reset player tick proc and signal to change back to the old room
			res.getActiveHotspot(PLAYER_ID)->setTickProc(PLAYER_TICK_PROC_ID); 
			fields.setField(NEW_ROOM_NUMBER, oldRoomNumber);
			fields.setField(OLD_ROOM_NUMBER, 0);
		}
	} else if ((room.cursorState() == CS_TALKING) ||
			   (res.getTalkState() != TALK_NONE)) {
		// Currently talking, so let its tick proc handle it
	} else if (mouse.y() < MENUBAR_Y_SIZE) {
		uint8 response = Menu::getReference().execute();
		if (response != MENUITEM_NONE)
			handleMenuResponse(response);
	} else if ((room.cursorState() == CS_SEQUENCE) ||
			   (room.cursorState() == CS_BUMPED)) { 
		// No action necessary
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
	Screen &screen = Screen::getReference();
	ValueTableData &fields = res.fieldList();
	StringList &stringList = res.stringList();
	StringData &strings = StringData::getReference();
	Mouse &mouse = Mouse::getReference();
	char *statusLine = room.statusLine();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	HotspotData *hotspot, *useHotspot;
	Action action;
	uint32 actions;
	uint16 itemId = 0xffff;
	bool hasItems;

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
		statusLine = room.statusLine();
		strcpy(statusLine, "");
		room.update();
		screen.update();

		action = PopupMenu::Show(actions);

		if (action != NONE) {
			sprintf(statusLine, "%s ", stringList.getString(action));
			statusLine += strlen(statusLine);
		}

		switch (action) {
		case LOOK:
		case STATUS:
			breakFlag = true;
			break;

		case ASK:
			hotspot = res.getHotspot(room.hotspotId());
			assert(hotspot);
			strings.getString(hotspot->nameId, statusLine);
			strcat(statusLine, stringList.getString(S_FOR));
			statusLine += strlen(statusLine);

			itemId = PopupMenu::ShowItems(GET);
			breakFlag = ((itemId != 0xffff) && (itemId != 0xfffe));
			break;

		case TELL:
			hotspot = res.getHotspot(room.hotspotId());
			assert(hotspot);
			strings.getString(hotspot->nameId, statusLine);
			strcat(statusLine, stringList.getString(S_TO));
			breakFlag = GetTellActions();
			break;

		case GIVE:
		case USE:
		case EXAMINE:
		case DRINK:
			hasItems = (res.numInventoryItems() != 0);
			if (!hasItems)
				strcat(statusLine, stringList.getString(S_NOTHING));
			statusLine += strlen(statusLine);

			room.update();
			screen.update();
			mouse.waitForRelease();

			if (hasItems) {
				if (action != DRINK)
					hotspot = res.getHotspot(room.hotspotId());
				itemId = PopupMenu::ShowInventory();
				breakFlag = (itemId != 0xffff);
				if (breakFlag) { 
					fields.setField(USE_HOTSPOT_ID, itemId);
					if ((action == GIVE) || (action == USE)) {
						// Add in the "X to " or "X on " section of give/use action
						useHotspot = res.getHotspot(itemId);
						assert(useHotspot);
						strings.getString(useHotspot->nameId, statusLine);
						if (action == GIVE) 
							strcat(statusLine, stringList.getString(S_TO));
						else 
							strcat(statusLine, stringList.getString(S_ON));
						statusLine += strlen(statusLine);
					}
					else if ((action == DRINK) || (action == EXAMINE))
						hotspot = res.getHotspot(itemId);
				}
			}
			break;

		default:
			hotspot = res.getHotspot(room.hotspotId());
			breakFlag = true;
			break;
		}
	}

	if (action != NONE) {
		player->stopWalking();

		if (hotspot == NULL) {
			doAction(action, 0, itemId);
		} else {
			// Add the hotspot name to the status line and then go do the action
			strings.getString(hotspot->nameId, statusLine);
			doAction(action, hotspot->hotspotId, itemId);
		}
	} else {
		// Clear the status line
		strcpy(room.statusLine(), "");
	}
}

void Game::handleLeftClick() {
	Room &room = Room::getReference();
	Mouse &mouse = Mouse::getReference();
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	StringList &stringList = res.stringList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);

	room.setCursorState(CS_NONE);
	player->stopWalking();
	player->setDestHotspot(0);
	player->setActionCtr(0);
	strcpy(room.statusLine(), "");

	if ((room.destRoomNumber() == 0) && (room.hotspotId() != 0)) {	
		// Handle look at hotspot
		sprintf(room.statusLine(), "%s ", stringList.getString(LOOK_AT));
		HotspotData *hotspot = res.getHotspot(room.hotspotId());
		assert(hotspot);
		strings.getString(hotspot->nameId, room.statusLine() + strlen(room.statusLine()));
		doAction(LOOK_AT, room.hotspotId(), 0xffff);

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

bool Game::GetTellActions() {
	Resources &res = Resources::getReference();
	Screen &screen = Screen::getReference();
	Room &room = Room::getReference();
	StringData &strings = StringData::getReference();
	StringList &stringList = res.stringList();
	char *statusLine = room.statusLine();
	uint16 fullCommands[MAX_TELL_COMMANDS * 3 + 1];
	uint16 *commands = &fullCommands[1];
	char *statusLinePos[MAX_TELL_COMMANDS][4];
	int commandIndex = 0;
	int paramIndex = 0;
	uint16 selectionId;
	char selectionName[MAX_DESC_SIZE];
	HotspotData *hotspot;
	Action action;
	const char *continueStrsList[2] = {stringList.getString(S_AND_THEN), stringList.getString(S_FINISH)};

	// First word will be the destination character
	fullCommands[0] = room.hotspotId();

	// Loop for getting tell commands

	while ((commandIndex >= 0) && (commandIndex < MAX_TELL_COMMANDS)) {

		// Loop for each sub-part of commands: Action, up to two params, and 
		// a "and then" selection to allow for more commands

		while ((paramIndex >= 0) && (paramIndex <= 4)) {
			// Update status line
			statusLine += strlen(statusLine);
			statusLinePos[commandIndex][paramIndex] = statusLine;
			room.update();
			screen.update();

			switch (paramIndex) {
			case 0:
                // Prompt for selection of action to perform
				action = PopupMenu::Show(0x6A07FD);
				if (action == NONE) {
					// Move backwards to prior specified action
					--commandIndex;
					if (commandIndex < 0) 
						paramIndex = -1;
					else
					{
						paramIndex = 3;
						statusLine = statusLinePos[commandIndex][paramIndex];
						*statusLine = '\0';
					}
					break;
				}

				// Add the action to the status line
				sprintf(statusLine + strlen(statusLine), "%s ", stringList.getString(action));

				// Handle any processing for the action
				commands[commandIndex * 3] = (uint16) action;
				commands[commandIndex * 3 + 1] = 0;
				commands[commandIndex * 3 + 2] = 0;
				++paramIndex;
				break;

			case 1:
				// First parameter
				action = (Action) commands[commandIndex * 3]; 
				if (action != RETURN) {
					// Prompt for selection
					if ((action != USE) && (action != DRINK) && (action != GIVE)) 
						selectionId = PopupMenu::ShowItems(action);
					else
						selectionId = PopupMenu::ShowItems(GET);

					if ((selectionId == 0xffff) || (selectionId == 0xfffe)) {
						// Move back to prompting for action
						--paramIndex;
						statusLine = statusLinePos[commandIndex][paramIndex];
						*statusLine = '\0';
						break;
					}
							
					if (selectionId < NOONE_ID) {
						// Must be a room selection
						strings.getString(selectionId, selectionName);
					} else {
						hotspot = res.getHotspot(selectionId);
						assert(hotspot);
						strings.getString(hotspot->nameId, selectionName);
					}

					// Store selected entry
					commands[commandIndex * 3 + 1] = selectionId;
					strcat(statusLine, selectionName);
				}

				++paramIndex;
				break;

			case 2:
				// Second parameter
				action = (Action) commands[commandIndex * 3]; 
				if (action == ASK)
					strcat(statusLine, " for ");
				else if (action == GIVE)
					strcat(statusLine, " to ");
				else if (action == USE)
					strcat(statusLine, " on ");
				else {
					// All other commads don't need a second parameter
					++paramIndex;
					break;
				}

				// Get the second parameter
				selectionId = PopupMenu::ShowItems(GET);
				if ((selectionId == 0xfffe) || (selectionId == 0xffff)) {
					--paramIndex;
					statusLine = statusLinePos[commandIndex][paramIndex];
					*statusLine = '\0';
				} else {
					// Display the second parameter
					hotspot = res.getHotspot(selectionId);
					assert(hotspot);
					strings.getString(hotspot->nameId, selectionName);
					strcat(statusLine, selectionName);

					commands[commandIndex * 3 + 2] = selectionId;
					++paramIndex;
				}
				break;

			case 3:
				// Prompting for "and then" for more commands
				if (commandIndex == MAX_TELL_COMMANDS - 1) {
					// No more commands allowed
					++commandIndex;
					paramIndex = -1;
				} else {
					// Only prompt if less than 8 commands entered
					selectionId = PopupMenu::Show(2, continueStrsList);

					switch (selectionId) {
					case 0:
						// Get ready for next command
						sprintf(statusLine + strlen(statusLine), " %s ", continueStrsList[0]);
						++commandIndex;
						paramIndex = 0;
						break;

					case 1:
						// Increment for just selected command, and add a large amount
						// to signal that the command sequence is complete
						commandIndex += 1 + 0x100;
						paramIndex = -1;
						break;

					default:
						// Move to end of just completed command
						action = (Action) commands[commandIndex * 3]; 
						if (action == RETURN)
							paramIndex = 0;
						else if ((action == ASK) || (action == GIVE) || (action == USE))
							paramIndex = 2;
						else
							paramIndex = 1;
					
						statusLine = statusLinePos[commandIndex][paramIndex];
						*statusLine = '\0';
					}
				}
			}
		}
	}

	bool result = (commandIndex != -1);
	if (result) {
		commandIndex &= 0xff;
		assert((commandIndex > 0) && (commandIndex <= MAX_TELL_COMMANDS));
		strcpy(statusLinePos[0][0], "..");
		room.update();
		screen.update();

		CharacterScheduleEntry *playerRec = res.playerSupportRecord();
  		playerRec->setDetails2(TELL, commandIndex * 3 + 1, &fullCommands[0]);
	}

	return result;
}

void Game::doAction(Action action, uint16 hotspotId, uint16 usedId) {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);

	fields.setField(CHARACTER_HOTSPOT_ID, PLAYER_ID);
	fields.setField(ACTIVE_HOTSPOT_ID, hotspotId);

	res.setCurrentAction(action);
	room.setCursorState(CS_ACTION);

	// Set the action
	CharacterScheduleEntry *rec = res.playerSupportRecord();
	if (action != TELL)
		rec->setDetails(action, hotspotId, usedId);
	player->currentActions().addFront(DISPATCH_ACTION, rec, player->roomNumber());
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
	StringList &sl = Resources::getReference().stringList();

	_slowSpeedFlag = !_slowSpeedFlag;
	menu.getMenu(2).entries()[1] = sl.getString(_slowSpeedFlag ? S_SLOW_TEXT : S_FAST_TEXT);
}

void Game::doSound() {
	Menu &menu = Menu::getReference();
	StringList &sl = Resources::getReference().stringList();

	_soundFlag = !_soundFlag;
	menu.getMenu(2).entries()[2] = sl.getString(_soundFlag ? S_SOUND_ON : S_SOUND_OFF);
}

void Game::handleBootParam(int value) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Room &room = Room::getReference();
	Hotspot *h;

	switch (value) {
	case 0:
		// No parameter - load the first room
		room.setRoomNumber(1);
		break;

	case 1:
		// Set player to be in rack room with a few items
		// Setup Skorl in cell room
		h = res.getActiveHotspot(SKORL_ID);
		h->setRoomNumber(1);
		h->setPosition(140, 120);
		h->currentActions().top().setSupportData(0x1400);
		fields.setField(11, 1);
	
		// Set up player
		h = res.getActiveHotspot(PLAYER_ID);
		h->setRoomNumber(4);
		h->setPosition(150, 110);
		res.getHotspot(0x2710)->roomNumber = PLAYER_ID;  // Bottle
		res.getHotspot(0x2713)->roomNumber = PLAYER_ID;  // Knife

		room.setRoomNumber(4);
		break;

	case 2:
		// Set the player up in the outer cell with a full bottle & knife
		h = res.getActiveHotspot(PLAYER_ID);
		h->setRoomNumber(2);
		h->setPosition(100, 110);
		res.getHotspot(0x2710)->roomNumber = PLAYER_ID;  // Bottle
		fields.setField(BOTTLE_FILLED, 1);
		res.getHotspot(0x2713)->roomNumber = PLAYER_ID;  // Knife

		room.setRoomNumber(2);
		break;

	default:
		room.setRoomNumber(value);
		break;
	}
}

} // end of namespace Lure
