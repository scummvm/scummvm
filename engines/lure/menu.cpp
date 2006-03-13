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

#include "lure/menu.h"
#include "lure/luredefs.h"
#include "lure/decode.h"
#include "lure/surface.h"
#include "lure/system.h"
#include "lure/res_struct.h"
#include "lure/res.h"
#include "lure/strings.h"

namespace Lure {

MenuRecord::MenuRecord(uint16 hsxstartVal, uint16 hsxendVal, uint16 xstartVal, 
					   uint16 widthVal, const char *strings) {
	_xstart = xstartVal; _width = widthVal;
	_hsxstart = hsxstartVal; _hsxend = hsxendVal;

	// Figure out the number of entries
	const char *sPtr = strings;
	_numEntries = 1;
	while ((sPtr = strchr(sPtr, ',')) != NULL) {
		++_numEntries;
		++sPtr;
	}

	// Set up the list of entries
	char *sCopy = strdup(strings);
	char *s;
	_entries = (char **) malloc(sizeof(char *) * _numEntries);
	uint8 index = 0;
	s = sCopy;
	while (s != NULL) {
		_entries[index++] = s;
		s = strchr(s, ',');
		if (s != NULL) *s++ = '\0'; // replace comma with NULL
	}
}

MenuRecord::~MenuRecord() {
	free(_entries[0]);	// Delete string data for all the menu items
	free(_entries);		// Free the list
}

char *MenuRecord::getEntry(uint8 index) {
	if (index >= _numEntries) error("Invalid menuitem index specified: %d", index);
	return _entries[index];
}

/*--------------------------------------------------------------------------*/

static Menu *int_menu = NULL;

Menu::Menu() {
	int_menu = this;

	MemoryBlock *data = Disk::getReference().getEntry(MENU_RESOURCE_ID);
	PictureDecoder decoder;
	_menu = decoder.decode(data, SCREEN_SIZE);
	delete data;

	_menus[0] = new MenuRecord(40, 87, 20, 80, "Credits");
	_menus[1] = new MenuRecord(127, 179, 100, 120, "Restart game,Save game,Restore game");
	_menus[2] = new MenuRecord(224, 281, 210, 105, "Quit,Slow Text\x8b,Sound on ");
	_selectedMenu = NULL;
}

Menu::~Menu() {
	for (int ctr=0; ctr<NUM_MENUS; ++ctr) delete _menus[ctr];
	delete _menu;
}

Menu &Menu::getReference() {
	return *int_menu;
}

uint8 Menu::execute() {
	OSystem &system = System::getReference();
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();

	mouse.setCursorNum(CURSOR_ARROW);
	system.copyRectToScreen(_menu->data(), FULL_SCREEN_WIDTH, 0, 0, 
		FULL_SCREEN_WIDTH, MENUBAR_Y_SIZE);
	system.updateScreen();

	_selectedMenu = NULL;
	_surfaceMenu = NULL;
	_selectedIndex = 0;

	while (mouse.lButton() || mouse.rButton()) {
		if (events.pollEvent()) {
			if (events.quitFlag) return MENUITEM_NONE;

			if (mouse.y() < MENUBAR_Y_SIZE)
			{
				MenuRecord *p = getMenuAt(mouse.x());

				if (_selectedMenu != p) {
					// If necessary, remove prior menu
					if (_selectedMenu) {
						toggleHighlight(_selectedMenu);
//						screen.updateArea(_selectedMenu->xstart(), MENUBAR_Y_SIZE,
//							_surfaceMenu->width(), _surfaceMenu->height());
						screen.updateArea(0, MENUBAR_Y_SIZE, FULL_SCREEN_WIDTH, 
							_surfaceMenu->height());
						delete _surfaceMenu;
						_surfaceMenu = NULL;
						_selectedIndex = 0;
					}						

					_selectedMenu = p;

					// If a new menu is selected, show it
					if (_selectedMenu) {
						toggleHighlight(_selectedMenu);
						_surfaceMenu = Surface::newDialog(
							_selectedMenu->width(), _selectedMenu->numEntries(), 
							_selectedMenu->entries(), false, MENU_UNSELECTED_COLOUR);
						_surfaceMenu->copyToScreen(_selectedMenu->xstart(), MENUBAR_Y_SIZE);
					}

					system.copyRectToScreen(_menu->data(), FULL_SCREEN_WIDTH, 0, 0, 
						FULL_SCREEN_WIDTH, MENUBAR_Y_SIZE);
					system.updateScreen();
				}
			}

			// Check for changing selected index
			uint8 index = getIndexAt(mouse.x(), mouse.y());
			if (index != _selectedIndex) {
				if (_selectedIndex != 0) toggleHighlightItem(_selectedIndex);
				_selectedIndex = index;
				if (_selectedIndex != 0) toggleHighlightItem(_selectedIndex);
			}
		}
	}

	if (_surfaceMenu) delete _surfaceMenu;

	// Deselect the currently selected menu header
	if (_selectedMenu) 
		toggleHighlight(_selectedMenu);

	// Restore the previous screen
	screen.update();
	
	if ((_selectedMenu == NULL) || (_selectedIndex == 0)) return MENUITEM_NONE;
	else if (_selectedMenu == _menus[0]) return MENUITEM_CREDITS;
	else if (_selectedMenu == _menus[1]) {
		switch (_selectedIndex) {
			case 1: return MENUITEM_RESTART_GAME;
			case 2: return MENUITEM_SAVE_GAME;
			case 3: return MENUITEM_RESTORE_GAME;
		}
	} else {
		switch (_selectedIndex) {
			case 1: return MENUITEM_QUIT;
			case 2: return MENUITEM_TEXT_SPEED;
			case 3: return MENUITEM_SOUND;
		}
	}
	return MENUITEM_NONE;
}	

MenuRecord *Menu::getMenuAt(int x) {
	for (int ctr = 0; ctr < NUM_MENUS; ++ctr)
		if ((x >= _menus[ctr]->hsxstart()) && (x <= _menus[ctr]->hsxend())) 
			return _menus[ctr];

	return NULL;
}

uint8 Menu::getIndexAt(uint16 x, uint16 y) {
	if (!_selectedMenu) return 0;

	int ys = MENUBAR_Y_SIZE + DIALOG_EDGE_SIZE + 3;
	int ye = MENUBAR_Y_SIZE + _surfaceMenu->height() - DIALOG_EDGE_SIZE - 3;
	if ((y < ys) || (y > ye)) return 0;

	uint16 yRelative = y - ys;
	uint8 index = (uint8) (yRelative / 8) + 1;
	if (index > _selectedMenu->numEntries()) index = _selectedMenu->numEntries();
	return index;
}

void Menu::toggleHighlight(MenuRecord *menuRec) {
	byte *addr = _menu->data();

	for (uint16 y=0; y<MENUBAR_Y_SIZE; ++y) {
		for (uint16 x=menuRec->hsxstart(); x<=menuRec->hsxend(); ++x) {
			if (addr[x] == MENUBAR_SELECTED_COLOUR) addr[x] = 0;
			else if (addr[x] == 0) addr[x] = MENUBAR_SELECTED_COLOUR;
		}
		addr += FULL_SCREEN_WIDTH;
	}
}

void Menu::toggleHighlightItem(uint8 index) {
	byte *p = _surfaceMenu->data().data() + (DIALOG_EDGE_SIZE + 3 + 
		((index - 1) * 8)) * _surfaceMenu->width();
	uint32 numBytes = 8 * _surfaceMenu->width();

	while (numBytes-- > 0) {
		if (*p == MENU_UNSELECTED_COLOUR) *p = MENU_SELECTED_COLOUR;
		else if (*p == MENU_SELECTED_COLOUR) *p = MENU_UNSELECTED_COLOUR;
		++p;
	}

	_surfaceMenu->copyToScreen(_selectedMenu->xstart(), MENUBAR_Y_SIZE);
}

/*--------------------------------------------------------------------------*/

uint16 PopupMenu::ShowInventory() {
	Resources &rsc = Resources::getReference();
	StringData &strings = StringData::getReference();

	uint16 numItems = rsc.numInventoryItems();
	uint16 itemCtr = 0;
	char **itemNames = (char **) Memory::alloc(sizeof(char *) * numItems);
	uint16 *idList = (uint16 *) Memory::alloc(sizeof(uint16) * numItems);

	HotspotDataList::iterator i;
	for (i = rsc.hotspotData().begin(); i != rsc.hotspotData().end(); ++i) {
		HotspotData *hotspot = *i;
		if (hotspot->roomNumber == PLAYER_ID) {
			idList[itemCtr] = hotspot->hotspotId;
			char *hotspotName = itemNames[itemCtr++] = (char *) malloc(MAX_HOTSPOT_NAME_SIZE);
			strings.getString(hotspot->nameId, hotspotName, NULL, NULL);
		}
	}
	
	uint16 result = Show(numItems, (const char **) itemNames);
	if (result != 0xffff) result = idList[result];

	for (itemCtr = 0; itemCtr < numItems; ++itemCtr)
		free(itemNames[itemCtr]);

	delete itemNames;
	delete idList;
	return result;
}

Action PopupMenu::Show(uint32 actionMask) {
	int numEntries = 0;
	uint32 v = actionMask;
	int index;

	for (index = 1; index <= EXAMINE; ++index, v >>= 1) {
		if (v & 1) ++numEntries;
	}

	const char **strList = (const char **) Memory::alloc(sizeof(char *) * numEntries);

	v = actionMask;
	int strIndex = 0;
	for (index=1; index<=EXAMINE; ++index, v >>= 1) {
		if (v & 1) 
			strList[strIndex++] = actionList[index];
	}

	uint16 result = Show(numEntries, strList);
	
	if (result == 0xffff) return NONE;

	v = actionMask;
	for (index = 1; index <= EXAMINE; ++index, v >>= 1) {
		if (v & 1) 
			if (result-- == 0) return (Action) index;
	}

	delete strList;
	return NONE;
}

Action PopupMenu::Show(int numEntries, Action *actions) {
	const char **strList = (const char **) Memory::alloc(sizeof(char *) * numEntries);
	Action *actionPtr = actions;
	for (int index = 0; index < numEntries; ++index)
		strList[index] = actionList[*actionPtr++];
	uint16 result = Show(numEntries, strList);
	
	delete strList;
	if (result == 0xffff) return NONE;
	else return actions[result];
}

uint16 PopupMenu::Show(int numEntries, const char *actions[]) {
	if (numEntries == 0) return 0xffff;
	Events &e = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	OSystem &system = System::getReference();
	Screen &screen = Screen::getReference();
	Rect r;

	mouse.cursorOff();
	uint16 oldX = mouse.x();
	uint16 oldY = mouse.y();
	const uint16 yMiddle = FULL_SCREEN_HEIGHT / 2;
	mouse.setPosition(FULL_SCREEN_WIDTH / 2, yMiddle);

	// Round up number of lines in dialog to next odd number
	uint16 numLines = (numEntries / 2) * 2 + 1;
	if (numLines > 5) numLines = 5;

	// Figure out the character width
	uint16 numCols = 0;
	for (int ctr = 0; ctr < numEntries; ++ctr) {
		int len = strlen(actions[ctr]);
		if (len > numCols)
			numCols = len;
	}

	// Create the dialog surface
	Surface *s = new Surface(DIALOG_EDGE_SIZE * 2 + numCols * FONT_WIDTH, 
		DIALOG_EDGE_SIZE * 2 + numLines * FONT_HEIGHT);
	s->createDialog();

	int selectedIndex = 0;
	bool refreshFlag = true;
	r.left = DIALOG_EDGE_SIZE;
	r.right = s->width() - DIALOG_EDGE_SIZE - 1;
	r.top = DIALOG_EDGE_SIZE;
	r.bottom = s->height() - DIALOG_EDGE_SIZE - 1;

	for (;;) {
		if (refreshFlag) {
			// Set up the contents of the menu
			s->fillRect(r, 0);

			for (int index = 0; index < numLines; ++index) {
				int actionIndex = selectedIndex - (numEntries / 2) + index;
				if ((actionIndex >= 0) && (actionIndex < numEntries)) {
					s->writeString(DIALOG_EDGE_SIZE, DIALOG_EDGE_SIZE + index * FONT_HEIGHT,
						actions[actionIndex], true, 
						(index == (numLines / 2)) ? MENU_SELECTED_COLOUR : MENU_UNSELECTED_COLOUR,
						false);
				}
			}

			s->copyToScreen(0, yMiddle-(s->height() / 2));
			system.updateScreen();
			refreshFlag = false;
		}

		while (e.pollEvent()) {
			if (e.quitFlag) {
				selectedIndex = 0xffff;
				goto bail_out;
			}

			else if (e.type() == OSystem::EVENT_KEYDOWN) {
				byte ch = e.event().kbd.ascii;
				uint16 keycode = e.event().kbd.keycode;

				if (((keycode == 0x108) || (keycode == 0x111)) && (selectedIndex > 0)) {
					--selectedIndex;
					refreshFlag = true;
				} else if (((keycode == 0x102) || (keycode == 0x112)) && 
						(selectedIndex < numEntries-1)) {
					++selectedIndex;
					refreshFlag = true;
				} else if ((ch == '\xd') || (keycode == 0x10f)) {
					goto bail_out;
				} else if (ch == '\x1b') {
					selectedIndex = 0xffff;
					goto bail_out;
				}

			} else if (e.type() == OSystem::EVENT_LBUTTONDOWN) {
				mouse.waitForRelease();
				goto bail_out;

			} else if (e.type() == OSystem::EVENT_RBUTTONDOWN) {
				mouse.waitForRelease();
				selectedIndex = 0xffff;
				goto bail_out;
			}
		}

		// Warping the mouse to "neutral" even if the top/bottom menu
		// entry has been reached has both pros and cons. It makes the
		// menu behave a bit more sensibly, but it also makes it harder
		// to move the mouse pointer out of the ScummVM window.

		if (mouse.y() < yMiddle - POPMENU_CHANGE_SENSITIVITY) {
			if (selectedIndex > 0) {
				--selectedIndex;
				refreshFlag = true;
			}
			mouse.setPosition(FULL_SCREEN_WIDTH / 2, yMiddle);
		} else if (mouse.y() > yMiddle + POPMENU_CHANGE_SENSITIVITY) {
			if (selectedIndex < numEntries - 1) {
				++selectedIndex;
				refreshFlag = true;
			}
			mouse.setPosition(FULL_SCREEN_WIDTH / 2, yMiddle);
		}

		system.delayMillis(20);
	}

bail_out:
	mouse.setPosition(oldX, oldY);
	mouse.cursorOn();
	screen.update();
	return selectedIndex;
}

} // end of namespace Lure
