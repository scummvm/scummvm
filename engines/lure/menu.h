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

#ifndef __lure_menu_h__
#define __lure_menu_h__

#include "common/stdafx.h"
#include "common/str.h"
#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/screen.h"
#include "lure/surface.h"
#include "lure/events.h"

#define NUM_MENUS 3

namespace Lure {

class MenuRecord {
private:
	uint16 _xstart, _width;
	uint16 _hsxstart, _hsxend;
	char **_entries;
	uint8 _numEntries;
public:
	MenuRecord(uint16 hsxstartVal, uint16 hsxendVal, uint16 xstartVal, 
		uint16 widthVal, const char *strings); 
	~MenuRecord();

	uint16 xstart() { return _xstart; }
	uint16 width() { return _width; }
	uint16 hsxstart() { return _hsxstart; }
	uint16 hsxend() { return _hsxend; }
	uint8 numEntries() { return _numEntries; }
	char **entries() { return _entries; }
	char *getEntry(uint8 index);
};

class Menu {
private:
	MemoryBlock *_menu;
	MenuRecord *_menus[NUM_MENUS];
	MenuRecord *_selectedMenu;
	Surface *_surfaceMenu;
	uint8 _selectedIndex;

	MenuRecord *getMenuAt(int x);
	uint8 getIndexAt(uint16 x, uint16 y);	
	void toggleHighlight(MenuRecord *menuRec);
	void toggleHighlightItem(uint8 index);
public:
	Menu();
	~Menu();
	static Menu &getReference();
	uint8 execute();
	MenuRecord &getMenu(uint8 index) { return *_menus[index]; }
};

class PopupMenu {
public:
	static Action Show(uint32 actionMask);
	static Action Show(int numEntries, Action *actions);
	static uint16 Show(int numEntries, const char *actions[]);
	static uint16 ShowInventory();
};

} // End of namespace Lure

#endif
