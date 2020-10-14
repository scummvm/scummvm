/** @file debug.cpp
	@brief
	This file contains the main game debug window routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "debug.h"

#ifdef GAMEMOD
#include "debug.scene.h"
#include "debug.grid.h"
#include "scene.h"
#include "sdlengine.h"
#include "menu.h"
#include "interface.h"
#include "text.h"
#include "lbaengine.h"
#include "screens.h"
#include "redraw.h"

enum ButtonType {
	NO_ACTION,
	FREE_CAMERA,
	CHANGE_SCENE,
	SHOW_CELLING_GRID,
	SHOW_ZONES,
	SHOW_ZONE_CUBE,
	SHOW_ZONE_CAMERA,
	SHOW_ZONE_SCENARIC,
	SHOW_ZONE_CELLINGGRID,
	SHOW_ZONE_OBJECT,
	SHOW_ZONE_TEXT,
	SHOW_ZONE_LADDER
};

enum WindowType {
	NO_MENU,
	FREE_CAMERA_INFO_MENU,
	CHANGE_SCENE_INFO_MENU,
	ZONES_MENU
};

typedef struct DebugButtonStruct {
	int32 left;
	int32 top;
	int32 right;
	int32 bottom;
	int8  *text;
	int32 textLeft;
	int32 textTop;
	int32 isActive;
	int32 color;
	int32 activeColor;
	int32 submenu;
	int32 type;
} DebugButtonStruct;

typedef struct DebugWindowStruct {
	int32 left;
	int32 top;
	int32 right;
	int32 bottom;
	int32 alpha;
	int32 isActive;
	int32 numLines;
	int8  *text[20];
	int32 numButtons;
	DebugButtonStruct debugButtons[50];
} DebugWindowStruct;

DebugWindowStruct debugWindows[10];
int32 numDebugWindows = 0;


void debugFillButton(int32 X, int32 Y, int32 width, int32 height, int8 color) {
	int32 i, j;
	uint8 *ptr;
	int32 offset;

	ptr = frontVideoBuffer + screenLookupTable[Y] + X;
	offset = 640 - (width);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			*(ptr++) = color;
		}
		ptr += offset;
	}
}

void debugDrawButton(int32 left, int32 top, int32 right, int32 bottom, int8 *text, int32 textLeft, int32 textRight, int32 isActive, int8 color) {
	debugFillButton(left + 1, top + 1, right - left - 1, bottom - top - 1, color);
	drawBox(left, top, right, bottom);
	ttfDrawText(textLeft, textRight, text, 0);
	copyBlockPhys(left, top, right, bottom);
}

void debugDrawWindowBox(int32 left, int32 top, int32 right, int32 bottom, int32 alpha) {
	drawTransparentBox(left, top, right, bottom, alpha);
	drawBox(left, top, right, bottom);
	//copyBlockPhys(left,top,right,bottom);
}

void debugDrawWindowButtons(int32 w) {
	int32 b;

	for (b = 0; b < debugWindows[w].numButtons; b++) {
		int32 left = debugWindows[w].debugButtons[b].left;
		int32 top = debugWindows[w].debugButtons[b].top;
		int32 right = debugWindows[w].debugButtons[b].right;
		int32 bottom = debugWindows[w].debugButtons[b].bottom;
		int8  *text = debugWindows[w].debugButtons[b].text;
		int32 textLeft = debugWindows[w].debugButtons[b].textLeft;
		int32 textTop = debugWindows[w].debugButtons[b].textTop;
		int32 isActive = debugWindows[w].debugButtons[b].isActive;
		int8  color = debugWindows[w].debugButtons[b].color;
		if (isActive > 0)
			color = debugWindows[w].debugButtons[b].activeColor;

		debugDrawButton(left, top, right, bottom, text, textLeft, textTop, isActive, color);
	}
}

void debugDrawWindow(int32 w) {
	int32 left = debugWindows[w].left;
	int32 top = debugWindows[w].top;
	int32 right = debugWindows[w].right;
	int32 bottom = debugWindows[w].bottom;
	int32 alpha = debugWindows[w].alpha;

	debugDrawWindowBox(left, top, right, bottom, alpha);

	if (debugWindows[w].numLines > 0) {
		int32 l;

		for (l = 0; l < debugWindows[w].numLines; l++) {
			ttfDrawText(left + 10, top + l*20 + 5, debugWindows[w].text[l], 0);
		}
	}

	copyBlockPhys(left, top, right, bottom);

	debugDrawWindowButtons(w);
}

int32 debugTypeUseMenu(int32 type) {
	int32 w, b;

	for (w = 0; w < numDebugWindows; w++) {
		if (debugWindows[w].isActive > 0) {
			for (b = 0; b < debugWindows[w].numButtons; b++) {
				if (debugWindows[w].debugButtons[b].type == type) {
					int submenu = debugWindows[w].debugButtons[b].submenu;
					if (submenu > 0)
						debugWindows[submenu].isActive = !debugWindows[submenu].isActive;
					return submenu;
				}
			}
		}
	}
	return 0;
}

void debugResetButtonsState() {
	int w, b;
	for (w = 0; w < numDebugWindows; w++) {
		if (debugWindows[w].isActive > 0) {
			for (b = 0; b < debugWindows[w].numButtons; b++) {
				if (debugWindows[w].debugButtons[b].type <= -1)
					debugWindows[w].debugButtons[b].isActive = 0;
			}
		}
	}
}

void debugRefreshButtons(int32 type) {
	int32 w, b;

	for (w = 0; w < numDebugWindows; w++) {
		if (debugWindows[w].isActive > 0) {
			for (b = 0; b < debugWindows[w].numButtons; b++) {
				if (debugWindows[w].debugButtons[b].type == type) {
					int32 left = debugWindows[w].debugButtons[b].left;
					int32 top = debugWindows[w].debugButtons[b].top;
					int32 right = debugWindows[w].debugButtons[b].right;
					int32 bottom = debugWindows[w].debugButtons[b].bottom;
					int8  *text = debugWindows[w].debugButtons[b].text;
					int32 textLeft = debugWindows[w].debugButtons[b].textLeft;
					int32 textTop = debugWindows[w].debugButtons[b].textTop;
					int8  color = debugWindows[w].debugButtons[b].color;
					int32 isActive = debugWindows[w].debugButtons[b].isActive = !debugWindows[w].debugButtons[b].isActive;

					if (isActive > 0)
						color = debugWindows[w].debugButtons[b].activeColor;

					debugDrawButton(left, top, right, bottom, text, textLeft, textTop, isActive, color);

					if (debugWindows[w].debugButtons[b].submenu && isActive > 0)
						debugDrawWindow(debugWindows[w].debugButtons[b].submenu);
				}
			}
		}
	}
}

void debugDrawWindows() {
	int32 w;

	for (w = 0; w < numDebugWindows; w++) {
		if (debugWindows[w].isActive > 0) {
			debugDrawWindow(w);
		}
	}
}

void debugResetButton(int32 type) {
	int32 w, b;

	for (w = 0; w < numDebugWindows; w++) {
		if (debugWindows[w].isActive > 0) {
			for (b = 0; b < debugWindows[w].numButtons; b++) {
				if (debugWindows[w].debugButtons[b].type == type) {
					int submenu = debugWindows[w].debugButtons[b].submenu;
					debugWindows[w].debugButtons[b].isActive = 0;
					if (submenu > 0) {
						debugWindows[submenu].debugButtons[b].isActive = !debugWindows[submenu].debugButtons[b].isActive;
					}

					return;
				}
			}
		}
	}
}

void debugRedrawScreen() {
	redrawEngineActions(1);
	copyScreen(frontVideoBuffer, workVideoBuffer);
	debugDrawWindows();
}

int32 debugGetActionsState(int32 type) {
	int32 state = 0;

	switch (type) {
	case FREE_CAMERA:
		state = useFreeCamera;
		break;
	case CHANGE_SCENE:
		state = canChangeScenes;
		break;
	case SHOW_ZONES:
		state = showingZones;
		break;
	case SHOW_ZONE_CUBE:
	case SHOW_ZONE_CAMERA:
	case SHOW_ZONE_SCENARIC:
	case SHOW_ZONE_CELLINGGRID:
	case SHOW_ZONE_OBJECT:
	case SHOW_ZONE_TEXT:
	case SHOW_ZONE_LADDER:
		state = typeZones;
		break;
	default:
		break;
	}
	return state;
}

void debugSetActions(int32 type) {
	switch (type) {
	case FREE_CAMERA:
		useFreeCamera = !useFreeCamera;
		break;

	case CHANGE_SCENE:
		canChangeScenes = !canChangeScenes;
		break;

	case SHOW_ZONES:
		showingZones = !showingZones;
		debugResetButton(-1);
		debugResetButton(-2);
		debugRedrawScreen();
		break;
	case SHOW_ZONE_CUBE:
		if (showingZones) {
			if (typeZones & 0x01)
				typeZones &= ~0x01;
			else
				typeZones |= 0x01;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_CAMERA:
		if (showingZones) {
			if (typeZones & 0x02)
				typeZones &= ~0x02;
			else
				typeZones |= 0x02;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_SCENARIC:
		if (showingZones) {
			if (typeZones & 0x04)
				typeZones &= ~0x04;
			else
				typeZones |= 0x04;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_CELLINGGRID:
		if (showingZones) {
			if (typeZones & 0x08)
				typeZones &= ~0x08;
			else
				typeZones |= 0x08;
			debugRedrawScreen();
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_OBJECT:
		if (showingZones) {
			if (typeZones & 0x10)
				typeZones &= ~0x10;
			else
				typeZones |= 0x10;
			debugRedrawScreen();
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_TEXT:
		if (showingZones) {
			if (typeZones & 0x20)
				typeZones &= ~0x20;
			else
				typeZones |= 0x20;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_LADDER:
		if (showingZones) {
			if (typeZones & 0x40)
				typeZones &= ~0x40;
			else
				typeZones |= 0x40;
			debugRedrawScreen();
		}
		break;


	case -1:
		debugResetButton(-2);
		debugRedrawScreen();
		break;
	case -2:
		debugResetButton(-1);
		debugRedrawScreen();
		break;
	default:
		break;
	}
}

void debugAddButton(int32 window, int32 left, int32 top, int32 right, int32 bottom, int8 *text, int32 textLeft, int32 textTop, int32 isActive, int32 color, int32 activeColor, int32 submenu, int32 type) {
	int32 button = debugWindows[window].numButtons;
	debugWindows[window].debugButtons[button].left = left;
	debugWindows[window].debugButtons[button].top = top;
	debugWindows[window].debugButtons[button].right = right;
	debugWindows[window].debugButtons[button].bottom = bottom;
	debugWindows[window].debugButtons[button].text = text;
	debugWindows[window].debugButtons[button].textLeft = textLeft;
	debugWindows[window].debugButtons[button].textTop = textTop;
	debugWindows[window].debugButtons[button].isActive = debugGetActionsState(type);
	debugWindows[window].debugButtons[button].color = color;
	debugWindows[window].debugButtons[button].activeColor = activeColor;
	debugWindows[window].debugButtons[button].submenu = submenu;
	debugWindows[window].debugButtons[button].type = type;
	debugWindows[window].numButtons++;
}

void debugAddWindowText(int32 window, int8 *text) {
	int32 line = debugWindows[window].numLines;
	debugWindows[window].text[line] = text;
	debugWindows[window].numLines++;
}

void debugAddWindow(int32 left, int32 top, int32 right, int32 bottom, int32 alpha, int32 isActive) {
	debugWindows[numDebugWindows].left = left;
	debugWindows[numDebugWindows].top = top;
	debugWindows[numDebugWindows].right = right;
	debugWindows[numDebugWindows].bottom = bottom;
	debugWindows[numDebugWindows].alpha = alpha;
	debugWindows[numDebugWindows].numButtons = 0;
	debugWindows[numDebugWindows].isActive = isActive;
	numDebugWindows++;
}

void debugLeftMenu() {
	// left menu window
	debugAddWindow(5, 60, 200, 474, 4, 1);
	debugAddButton(0, 5, 55, 160, 75, (int8*) "Use free camera", 30, 60, 0, 87, 119, NO_MENU, FREE_CAMERA);
	debugAddButton(0, 161, 55, 200, 75, (int8*) "info", 171, 60, 0, 87, 119, FREE_CAMERA_INFO_MENU, -1);
	debugAddButton(0, 5, 76, 160, 96, (int8*) "Change scenes", 30, 81, 0, 87, 119, NO_MENU, CHANGE_SCENE);
	debugAddButton(0, 161, 76, 200, 96, (int8*) "info", 171, 81, 0, 87, 119, CHANGE_SCENE_INFO_MENU, -2);
	debugAddButton(0, 5, 97, 200, 117, (int8*) "Show celling grids", 30, 102, 0, 87, 119, NO_MENU, 3);
	debugAddButton(0, 5, 118, 200, 138, (int8*) "Show zones", 30, 123, 0, 87, 119, ZONES_MENU, SHOW_ZONES);

	// add submenu windows
	//   - free camera window
	debugAddWindow(205, 55, 634, 160, 4, 0);
	debugAddWindowText(FREE_CAMERA_INFO_MENU, (int8*) "When enable, use the following keys to browse through the scenes:");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, (int8*) "           - S to go North");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, (int8*) "           - X to go South");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, (int8*) "           - Z to go West");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, (int8*) "           - C to go East");

	//   - change scene window
	debugAddWindow(205, 55, 634, 137, 4, 0);
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, (int8*) "When enable, use the following keys to change to another scene:");
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, (int8*) "           - R to go Next Scene");
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, (int8*) "           - F to go Previous Scene");

	//   - zones window
	debugAddWindow(205, 55, 634, 97, 4, 0);
	debugAddWindowText(ZONES_MENU, (int8*) "You can enable or disable each zone type:");
	debugAddButton(ZONES_MENU, 205, 118, 350, 138, (int8*) "Cube Zones", 215, 123, 1, 87, 119, 0, SHOW_ZONE_CUBE);
	debugAddButton(ZONES_MENU, 205, 139, 350, 159, (int8*) "Camera Zones", 215, 144, 2, 87, 119, 0, SHOW_ZONE_CAMERA);
	debugAddButton(ZONES_MENU, 205, 160, 350, 180, (int8*) "Scenaric Zones", 215, 165, 3, 87, 119, 0, SHOW_ZONE_SCENARIC);
	debugAddButton(ZONES_MENU, 205, 181, 350, 201, (int8*) "Celling Grid Zones", 215, 186, 4, 87, 119, 0, SHOW_ZONE_CELLINGGRID);
	debugAddButton(ZONES_MENU, 205, 202, 350, 222, (int8*) "Object Zones", 215, 207, 5, 87, 119, 0, SHOW_ZONE_OBJECT);
	debugAddButton(ZONES_MENU, 205, 223, 350, 243, (int8*) "Text Zones", 215, 228, 6, 87, 119, 0, SHOW_ZONE_TEXT);
	debugAddButton(ZONES_MENU, 205, 244, 350, 264, (int8*) "Ladder Zones", 215, 249, 7, 87, 119, 0, SHOW_ZONE_LADDER);
}

int32 debugProcessButton(int32 X, int32 Y) {
	int32 i;
	int32 j;

	for (i = 0; i < numDebugWindows; i++) {
		for (j = 0; j < debugWindows[i].numButtons; j++) {
			if (X > (debugWindows[i].debugButtons[j].left)
			        && X < (debugWindows[i].debugButtons[j].right)
			        && Y > (debugWindows[i].debugButtons[j].top)
			        && Y < (debugWindows[i].debugButtons[j].bottom)) {
				return (debugWindows[i].debugButtons[j].type);
			}
		}
	}

	return 0;
}

void debugPlasmaWindow(int8 *text, int32 color) {
	int32 textSize;
	processPlasmaEffect(5, color);
	if (!(rand() % 5)) {
		plasmaEffectPtr[rand() % 320 * 10 + 6400] = 255;
	}
	textSize = getTextSize(text);
	drawText((SCREEN_WIDTH / 2) - (textSize / 2), 10, text);
	drawBox(5, 5, 634, 50);
	copyBlockPhys(5, 5, 634, 50);
}

void debugProcessWindow() {
	if (rightMouse) {
		int32 quit = 0;
		int8* text = (int8*) "Game Debug Window";
		int32 color = 64;
		int32 colorIdx = 4;
		int32 count = 0;
		MouseStatusStruct mouseData;
		rightMouse = 0;
		leftMouse = 0;

		copyScreen(frontVideoBuffer, workVideoBuffer);

		debugResetButtonsState();
		if (numDebugWindows == 0)
			debugLeftMenu();
		debugDrawWindows();

		do {
			readKeys();
			getMousePositions(&mouseData);

			if (mouseData.left) {
				int type = 0;
				if ((type = debugProcessButton(mouseData.X, mouseData.Y)) != NO_ACTION) { // process menu item
					if (debugTypeUseMenu(type)) {
						copyScreen(workVideoBuffer, frontVideoBuffer);
						copyBlockPhys(205, 55, 634, 474);
					}

					debugRefreshButtons(type);
					debugSetActions(type);
				}
				mouseData.left = 0;
			}

			// draw window plasma effect
			if (count == 256) {
				colorIdx++;
				count = 0;
			}
			color = colorIdx * 16;
			if (color >= 240) {
				color = 64;
				colorIdx = 4;
			}
			debugPlasmaWindow(text, color);

			// quit
			if (mouseData.right)
				quit = 1;

			fpsCycles(25); // rest

			count++;
		} while (!quit);
		reqBgRedraw = 1;
	}
}

void processDebug(int16 pKey) {
	debugProcessWindow();

	changeGrid(pKey);
	changeGridCamera(pKey);
	if (needChangeScene == 0);
	applyCellingGrid(pKey);
}

#endif

