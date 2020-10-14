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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/debug.h"

#include "common/system.h"
#include "twine/debug_grid.h"
#include "twine/debug_scene.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/redraw.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

void Debug::debugFillButton(int32 X, int32 Y, int32 width, int32 height, int8 color) {
	int32 i, j;
	uint8 *ptr;
	int32 offset;

	ptr = _engine->frontVideoBuffer + _engine->screenLookupTable[Y] + X;
	offset = 640 - (width);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			*(ptr++) = color;
		}
		ptr += offset;
	}
}

void Debug::debugDrawButton(int32 left, int32 top, int32 right, int32 bottom, const char *text, int32 textLeft, int32 textRight, int32 isActive, int8 color) {
	debugFillButton(left + 1, top + 1, right - left - 1, bottom - top - 1, color);
	_engine->_menu->drawBox(left, top, right, bottom);
	_engine->drawText(textLeft, textRight, text, 0);
	_engine->copyBlockPhys(left, top, right, bottom);
}

void Debug::debugDrawWindowBox(int32 left, int32 top, int32 right, int32 bottom, int32 alpha) {
	_engine->_interface->drawTransparentBox(left, top, right, bottom, alpha);
	_engine->_menu->drawBox(left, top, right, bottom);
	//_engine->copyBlockPhys(left,top,right,bottom);
}

void Debug::debugDrawWindowButtons(int32 w) {
	int32 b;

	for (b = 0; b < debugWindows[w].numButtons; b++) {
		int32 left = debugWindows[w].debugButtons[b].left;
		int32 top = debugWindows[w].debugButtons[b].top;
		int32 right = debugWindows[w].debugButtons[b].right;
		int32 bottom = debugWindows[w].debugButtons[b].bottom;
		const char *text = debugWindows[w].debugButtons[b].text;
		int32 textLeft = debugWindows[w].debugButtons[b].textLeft;
		int32 textTop = debugWindows[w].debugButtons[b].textTop;
		int32 isActive = debugWindows[w].debugButtons[b].isActive;
		int8 color = debugWindows[w].debugButtons[b].color;
		if (isActive > 0)
			color = debugWindows[w].debugButtons[b].activeColor;

		debugDrawButton(left, top, right, bottom, text, textLeft, textTop, isActive, color);
	}
}

void Debug::debugDrawWindow(int32 w) {
	int32 left = debugWindows[w].left;
	int32 top = debugWindows[w].top;
	int32 right = debugWindows[w].right;
	int32 bottom = debugWindows[w].bottom;
	int32 alpha = debugWindows[w].alpha;

	debugDrawWindowBox(left, top, right, bottom, alpha);

	if (debugWindows[w].numLines > 0) {
		int32 l;

		for (l = 0; l < debugWindows[w].numLines; l++) {
			_engine->drawText(left + 10, top + l * 20 + 5, debugWindows[w].text[l], 0);
		}
	}

	_engine->copyBlockPhys(left, top, right, bottom);

	debugDrawWindowButtons(w);
}

int32 Debug::debugTypeUseMenu(int32 type) {
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

void Debug::debugResetButtonsState() {
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

void Debug::debugRefreshButtons(int32 type) {
	int32 w, b;

	for (w = 0; w < numDebugWindows; w++) {
		if (debugWindows[w].isActive > 0) {
			for (b = 0; b < debugWindows[w].numButtons; b++) {
				if (debugWindows[w].debugButtons[b].type == type) {
					int32 left = debugWindows[w].debugButtons[b].left;
					int32 top = debugWindows[w].debugButtons[b].top;
					int32 right = debugWindows[w].debugButtons[b].right;
					int32 bottom = debugWindows[w].debugButtons[b].bottom;
					const char *text = debugWindows[w].debugButtons[b].text;
					int32 textLeft = debugWindows[w].debugButtons[b].textLeft;
					int32 textTop = debugWindows[w].debugButtons[b].textTop;
					int8 color = debugWindows[w].debugButtons[b].color;
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

void Debug::debugDrawWindows() {
	int32 w;

	for (w = 0; w < numDebugWindows; w++) {
		if (debugWindows[w].isActive > 0) {
			debugDrawWindow(w);
		}
	}
}

void Debug::debugResetButton(int32 type) {
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

void Debug::debugRedrawScreen() {
	_engine->_redraw->redrawEngineActions(1);
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	debugDrawWindows();
}

int32 Debug::debugGetActionsState(int32 type) {
	int32 state = 0;

	switch (type) {
	case FREE_CAMERA:
		state = _engine->_debugGrid->useFreeCamera;
		break;
	case CHANGE_SCENE:
		state = _engine->_debugGrid->canChangeScenes;
		break;
	case SHOW_ZONES:
		state = _engine->_debugScene->showingZones;
		break;
	case SHOW_ZONE_CUBE:
	case SHOW_ZONE_CAMERA:
	case SHOW_ZONE_SCENARIC:
	case SHOW_ZONE_CELLINGGRID:
	case SHOW_ZONE_OBJECT:
	case SHOW_ZONE_TEXT:
	case SHOW_ZONE_LADDER:
		state = _engine->_debugScene->typeZones;
		break;
	default:
		break;
	}
	return state;
}

void Debug::debugSetActions(int32 type) {
	switch (type) {
	case FREE_CAMERA:
		_engine->_debugGrid->useFreeCamera = !_engine->_debugGrid->useFreeCamera;
		break;

	case CHANGE_SCENE:
		_engine->_debugGrid->canChangeScenes = !_engine->_debugGrid->canChangeScenes;
		break;

	case SHOW_ZONES:
		_engine->_debugScene->showingZones = !_engine->_debugScene->showingZones;
		debugResetButton(-1);
		debugResetButton(-2);
		debugRedrawScreen();
		break;
	case SHOW_ZONE_CUBE:
		if (_engine->_debugScene->showingZones) {
			if (_engine->_debugScene->typeZones & 0x01)
				_engine->_debugScene->typeZones &= ~0x01;
			else
				_engine->_debugScene->typeZones |= 0x01;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_CAMERA:
		if (_engine->_debugScene->showingZones) {
			if (_engine->_debugScene->typeZones & 0x02)
				_engine->_debugScene->typeZones &= ~0x02;
			else
				_engine->_debugScene->typeZones |= 0x02;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_SCENARIC:
		if (_engine->_debugScene->showingZones) {
			if (_engine->_debugScene->typeZones & 0x04)
				_engine->_debugScene->typeZones &= ~0x04;
			else
				_engine->_debugScene->typeZones |= 0x04;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_CELLINGGRID:
		if (_engine->_debugScene->showingZones) {
			if (_engine->_debugScene->typeZones & 0x08)
				_engine->_debugScene->typeZones &= ~0x08;
			else
				_engine->_debugScene->typeZones |= 0x08;
			debugRedrawScreen();
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_OBJECT:
		if (_engine->_debugScene->showingZones) {
			if (_engine->_debugScene->typeZones & 0x10)
				_engine->_debugScene->typeZones &= ~0x10;
			else
				_engine->_debugScene->typeZones |= 0x10;
			debugRedrawScreen();
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_TEXT:
		if (_engine->_debugScene->showingZones) {
			if (_engine->_debugScene->typeZones & 0x20)
				_engine->_debugScene->typeZones &= ~0x20;
			else
				_engine->_debugScene->typeZones |= 0x20;
			debugRedrawScreen();
		}
		break;
	case SHOW_ZONE_LADDER:
		if (_engine->_debugScene->showingZones) {
			if (_engine->_debugScene->typeZones & 0x40)
				_engine->_debugScene->typeZones &= ~0x40;
			else
				_engine->_debugScene->typeZones |= 0x40;
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

void Debug::debugAddButton(int32 window, int32 left, int32 top, int32 right, int32 bottom, const char *text, int32 textLeft, int32 textTop, int32 isActive, int32 color, int32 activeColor, int32 submenu, int32 type) {
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

void Debug::debugAddWindowText(int32 window, const char *text) {
	int32 line = debugWindows[window].numLines;
	debugWindows[window].text[line] = text;
	debugWindows[window].numLines++;
}

void Debug::debugAddWindow(int32 left, int32 top, int32 right, int32 bottom, int32 alpha, int32 isActive) {
	debugWindows[numDebugWindows].left = left;
	debugWindows[numDebugWindows].top = top;
	debugWindows[numDebugWindows].right = right;
	debugWindows[numDebugWindows].bottom = bottom;
	debugWindows[numDebugWindows].alpha = alpha;
	debugWindows[numDebugWindows].numButtons = 0;
	debugWindows[numDebugWindows].isActive = isActive;
	numDebugWindows++;
}

void Debug::debugLeftMenu() {
	// left menu window
	debugAddWindow(5, 60, 200, 474, 4, 1);
	debugAddButton(0, 5, 55, 160, 75, "Use free camera", 30, 60, 0, 87, 119, NO_MENU, FREE_CAMERA);
	debugAddButton(0, 161, 55, 200, 75, "info", 171, 60, 0, 87, 119, FREE_CAMERA_INFO_MENU, -1);
	debugAddButton(0, 5, 76, 160, 96, "Change scenes", 30, 81, 0, 87, 119, NO_MENU, CHANGE_SCENE);
	debugAddButton(0, 161, 76, 200, 96, "info", 171, 81, 0, 87, 119, CHANGE_SCENE_INFO_MENU, -2);
	debugAddButton(0, 5, 97, 200, 117, "Show celling grids", 30, 102, 0, 87, 119, NO_MENU, 3);
	debugAddButton(0, 5, 118, 200, 138, "Show zones", 30, 123, 0, 87, 119, ZONES_MENU, SHOW_ZONES);

	// add submenu windows
	//   - free camera window
	debugAddWindow(205, 55, 634, 160, 4, 0);
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "When enable, use the following keys to browse through the scenes:");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - S to go North");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - X to go South");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - Z to go West");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - C to go East");

	//   - change scene window
	debugAddWindow(205, 55, 634, 137, 4, 0);
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, "When enable, use the following keys to change to another scene:");
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, "           - R to go Next Scene");
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, "           - F to go Previous Scene");

	//   - zones window
	debugAddWindow(205, 55, 634, 97, 4, 0);
	debugAddWindowText(ZONES_MENU, "You can enable or disable each zone type:");
	debugAddButton(ZONES_MENU, 205, 118, 350, 138, "Cube Zones", 215, 123, 1, 87, 119, 0, SHOW_ZONE_CUBE);
	debugAddButton(ZONES_MENU, 205, 139, 350, 159, "Camera Zones", 215, 144, 2, 87, 119, 0, SHOW_ZONE_CAMERA);
	debugAddButton(ZONES_MENU, 205, 160, 350, 180, "Scenaric Zones", 215, 165, 3, 87, 119, 0, SHOW_ZONE_SCENARIC);
	debugAddButton(ZONES_MENU, 205, 181, 350, 201, "Celling Grid Zones", 215, 186, 4, 87, 119, 0, SHOW_ZONE_CELLINGGRID);
	debugAddButton(ZONES_MENU, 205, 202, 350, 222, "Object Zones", 215, 207, 5, 87, 119, 0, SHOW_ZONE_OBJECT);
	debugAddButton(ZONES_MENU, 205, 223, 350, 243, "Text Zones", 215, 228, 6, 87, 119, 0, SHOW_ZONE_TEXT);
	debugAddButton(ZONES_MENU, 205, 244, 350, 264, "Ladder Zones", 215, 249, 7, 87, 119, 0, SHOW_ZONE_LADDER);
}

int32 Debug::debugProcessButton(int32 X, int32 Y) {
	int32 i;
	int32 j;

	for (i = 0; i < numDebugWindows; i++) {
		for (j = 0; j < debugWindows[i].numButtons; j++) {
			if (X > (debugWindows[i].debugButtons[j].left) && X < (debugWindows[i].debugButtons[j].right) && Y > (debugWindows[i].debugButtons[j].top) && Y < (debugWindows[i].debugButtons[j].bottom)) {
				return (debugWindows[i].debugButtons[j].type);
			}
		}
	}

	return 0;
}

void Debug::debugPlasmaWindow(const char *text, int32 color) {
	int32 textSize;
	_engine->_menu->processPlasmaEffect(5, color);
	if (!(_engine->getRandomNumber() % 5)) {
		_engine->_menu->plasmaEffectPtr[_engine->getRandomNumber() % 320 * 10 + 6400] = 255;
	}
	textSize = _engine->_text->getTextSize(text);
	_engine->_text->drawText((SCREEN_WIDTH / 2) - (textSize / 2), 10, text);
	_engine->_menu->drawBox(5, 5, 634, 50);
	_engine->copyBlockPhys(5, 5, 634, 50);
}

void Debug::debugProcessWindow() {
	if (_engine->rightMouse) {
		int32 quit = 0;
		const char *text = "Game Debug Window";
		int32 color = 64;
		int32 colorIdx = 4;
		int32 count = 0;
		MouseStatusStruct mouseData;
		_engine->rightMouse = 0;
		_engine->leftMouse = 0;

		_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

		debugResetButtonsState();
		if (numDebugWindows == 0)
			debugLeftMenu();
		debugDrawWindows();

		do {
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				quit = 1;
			}
			_engine->getMousePositions(&mouseData);

			if (mouseData.left) {
				int type = 0;
				if ((type = debugProcessButton(mouseData.X, mouseData.Y)) != NO_ACTION) { // process menu item
					if (debugTypeUseMenu(type)) {
						_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
						_engine->copyBlockPhys(205, 55, 634, 474);
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
			if (mouseData.right) {
				quit = 1;
			}

			_engine->_system->delayMillis(1000 / 25); // rest

			count++;
		} while (!quit);
		_engine->_redraw->reqBgRedraw = 1;
	}
}

void Debug::processDebug(int16 pKey) {
	if (!_engine->cfgfile.Debug) {
		return;
	}
	debugProcessWindow();

	_engine->_debugGrid->changeGrid(pKey);
	_engine->_debugGrid->changeGridCamera(pKey);
	_engine->_debugGrid->applyCellingGrid(pKey);
}

} // namespace TwinE
