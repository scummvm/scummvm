/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CRUISE_MENU_H
#define CRUISE_MENU_H

namespace Cruise {

struct menuStruct {
	const char *stringPtr;
	gfxEntryStruct *gfx;
	int x;
	int y;
	int numElements;
	menuElementStruct *ptrNextElement;
};

typedef struct menuStruct menuStruct;

extern menuStruct *menuTable[8];

menuStruct *createMenu(int X, int Y, const char *menuName);
void addSelectableMenuEntry(int var0, int var1, menuStruct * pMenu, int var2,
    int color, const char *menuText);
void updateMenuMouse(int mouseX, int mouseY, menuStruct * pMenu);
int processMenu(menuStruct * pMenu);
void freeMenu(menuStruct * pMenu);
int playerMenu(int menuX, int menuY);

} // End of namespace Cruise

#endif
