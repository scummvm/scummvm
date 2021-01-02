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

#include "twine/debugger/debug.h"

#include "common/system.h"
#include "twine/debugger/debug_grid.h"
#include "twine/debugger/debug_scene.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/screens.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

void Debug::debugFillButton(int32 x, int32 y, int32 width, int32 height, int8 color) {
	uint8 *ptr = (uint8 *)_engine->frontVideoBuffer.getBasePtr(x, y);
	const int32 offset = SCREEN_WIDTH - width;

	for (int32 i = 0; i < height; i++) {
		for (int32 j = 0; j < width; j++) {
			*ptr++ = color;
		}
		ptr += offset;
	}
}

void Debug::debugDrawButton(const Common::Rect &rect, const char *text, int32 textLeft, int32 textRight, int32 isActive, int8 color) {
	debugFillButton(rect.left + 1, rect.top + 1, rect.right - rect.left - 1, rect.bottom - rect.top - 1, color);
	_engine->_menu->drawBox(rect);
	_engine->drawText(textLeft, textRight, text, 0);
	_engine->copyBlockPhys(rect);
}

void Debug::debugDrawWindowBox(const Common::Rect &rect, int32 alpha) {
	_engine->_interface->drawTransparentBox(rect, alpha);
	_engine->_menu->drawBox(rect);
	//_engine->copyBlockPhys(rect);
}

void Debug::debugDrawWindowButtons(int32 w) {
	DebugWindowStruct &window = debugWindows[w];
	for (int32 b = 0; b < window.numButtons; b++) {
		DebugButtonStruct &btn = window.debugButtons[b];
		const char *text = btn.text;
		const int32 textLeft = btn.textLeft;
		const int32 textTop = btn.textTop;
		const int32 isActive = btn.isActive;
		int8 color = btn.color;
		if (isActive > 0) {
			color = btn.activeColor;
		}

		debugDrawButton(btn.rect, text, textLeft, textTop, isActive, color);
	}
}

void Debug::debugDrawWindow(int32 w) {
	DebugWindowStruct &window = debugWindows[w];
	const Common::Rect &rect = window.rect;
	const int32 alpha = window.alpha;

	debugDrawWindowBox(rect, alpha);

	if (window.numLines > 0) {
		for (int32 l = 0; l < window.numLines; l++) {
			_engine->drawText(rect.left + 10, rect.top + l * 20 + 5, window.text[l], 0);
		}
	}

	_engine->copyBlockPhys(rect);

	debugDrawWindowButtons(w);
}

int32 Debug::debugTypeUseMenu(int32 type) {
	for (int32 w = 0; w < numDebugWindows; w++) {
		DebugWindowStruct &window = debugWindows[w];
		if (window.isActive <= 0) {
			continue;
		}
		for (int32 b = 0; b < window.numButtons; b++) {
			DebugButtonStruct &btn = window.debugButtons[b];
			if (btn.type != type) {
				continue;
			}
			const int submenu = btn.submenu;
			if (submenu > 0) {
				debugWindows[submenu].isActive = !debugWindows[submenu].isActive;
			}
			return submenu;
		}
	}
	return 0;
}

void Debug::debugResetButtonsState() {
	for (int32 w = 0; w < numDebugWindows; w++) {
		DebugWindowStruct &window = debugWindows[w];
		if (window.isActive <= 0) {
			continue;
		}
		for (int32 b = 0; b < window.numButtons; b++) {
			DebugButtonStruct &btn = window.debugButtons[b];
			if (btn.type > -1) {
				continue;
			}
			btn.isActive = 0;
		}
	}
}

void Debug::debugRefreshButtons(int32 type) {
	for (int32 w = 0; w < numDebugWindows; w++) {
		DebugWindowStruct &window = debugWindows[w];
		if (window.isActive <= 0) {
			continue;
		}
		for (int32 b = 0; b < window.numButtons; b++) {
			DebugButtonStruct &btn = window.debugButtons[b];
			if (btn.type != type) {
				continue;
			}
			const char *text = btn.text;
			const int32 textLeft = btn.textLeft;
			const int32 textTop = btn.textTop;
			int8 color = btn.color;
			const int32 isActive = btn.isActive = !btn.isActive;

			if (isActive > 0) {
				color = btn.activeColor;
			}

			debugDrawButton(btn.rect, text, textLeft, textTop, isActive, color);

			if (btn.submenu && isActive > 0) {
				debugDrawWindow(btn.submenu);
			}
		}
	}
}

void Debug::debugDrawWindows() {
	for (int32 w = 0; w < numDebugWindows; w++) {
		DebugWindowStruct &window = debugWindows[w];
		if (window.isActive > 0) {
			debugDrawWindow(w);
		}
	}
}

void Debug::debugResetButton(int32 type) {
	for (int32 w = 0; w < numDebugWindows; w++) {
		DebugWindowStruct &window = debugWindows[w];
		if (window.isActive <= 0) {
			continue;
		}
		for (int32 b = 0; b < window.numButtons; b++) {
			DebugButtonStruct &btn = window.debugButtons[b];
			if (btn.type != type) {
				continue;
			}
			const int submenu = btn.submenu;
			btn.isActive = 0;
			if (submenu > 0) {
				debugWindows[submenu].debugButtons[b].isActive = !debugWindows[submenu].debugButtons[b].isActive;
			}
			break;
		}
	}
}

void Debug::debugRedrawScreen() {
	_engine->_redraw->redrawEngineActions(true);
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	debugDrawWindows();
}

int32 Debug::debugGetActionsState(int32 type) {
	int32 state = 0;

	switch (type) {
	case FREE_CAMERA:
		state = _engine->_debugGrid->useFreeCamera ? 1 : 0;
		break;
	case CHANGE_SCENE:
		state = _engine->_debugGrid->canChangeScenes ? 1 : 0;
		break;
	case SHOW_ZONES:
		state = _engine->_debugScene->showingZones ? 1 : 0;
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

void Debug::debugAddButton(int32 window, const Common::Rect &rect, const char *text, int32 textLeft, int32 textTop, int32 isActive, int32 color, int32 activeColor, int32 submenu, int32 type) {
	const int32 button = debugWindows[window].numButtons;
	DebugButtonStruct &btn = debugWindows[window].debugButtons[button];
	btn.rect = rect;
	btn.text = text;
	btn.textLeft = textLeft;
	btn.textTop = textTop;
	btn.isActive = debugGetActionsState(type);
	btn.color = color;
	btn.activeColor = activeColor;
	btn.submenu = submenu;
	btn.type = type;
	debugWindows[window].numButtons++;
}

void Debug::debugAddWindowText(int32 window, const char *text) {
	int32 line = debugWindows[window].numLines;
	debugWindows[window].text[line] = text;
	debugWindows[window].numLines++;
}

void Debug::debugAddWindow(const Common::Rect &rect, int32 alpha, int32 isActive) {
	debugWindows[numDebugWindows].rect = rect;
	debugWindows[numDebugWindows].alpha = alpha;
	debugWindows[numDebugWindows].numButtons = 0;
	debugWindows[numDebugWindows].isActive = isActive;
	numDebugWindows++;
}

void Debug::debugLeftMenu() {
	// left menu window
	debugAddWindow(Common::Rect(5, 60, 200, 474), 4, 1);
	debugAddButton(0, Common::Rect(5, 55, 160, 75), "Use free camera", 30, 60, 0, 87, 119, NO_MENU, FREE_CAMERA);
	debugAddButton(0, Common::Rect(161, 55, 200, 75), "info", 171, 60, 0, 87, 119, FREE_CAMERA_INFO_MENU, -1);
	debugAddButton(0, Common::Rect(5, 76, 160, 96), "Change scenes", 30, 81, 0, 87, 119, NO_MENU, CHANGE_SCENE);
	debugAddButton(0, Common::Rect(161, 76, 200, 96), "info", 171, 81, 0, 87, 119, CHANGE_SCENE_INFO_MENU, -2);
	debugAddButton(0, Common::Rect(5, 97, 200, 117), "Show celling grids", 30, 102, 0, 87, 119, NO_MENU, 3);
	debugAddButton(0, Common::Rect(5, 118, 200, 138), "Show zones", 30, 123, 0, 87, 119, ZONES_MENU, SHOW_ZONES);

	// add submenu windows
	//   - free camera window
	debugAddWindow(Common::Rect(205, 55, 634, 160), 4, 0);
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "When enable, use the following keys to browse through the scenes:");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - S to go North");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - X to go South");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - Z to go West");
	debugAddWindowText(FREE_CAMERA_INFO_MENU, "           - C to go East");

	//   - change scene window
	debugAddWindow(Common::Rect(205, 55, 634, 137), 4, 0);
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, "When enable, use the following keys to change to another scene:");
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, "           - R to go Next Scene");
	debugAddWindowText(CHANGE_SCENE_INFO_MENU, "           - F to go Previous Scene");

	//   - zones window
	debugAddWindow(Common::Rect(205, 55, 634, 97), 4, 0);
	debugAddWindowText(ZONES_MENU, "You can enable or disable each zone type:");
	debugAddButton(ZONES_MENU, Common::Rect(205, 118, 350, 138), "Cube Zones", 215, 123, 1, 87, 119, 0, SHOW_ZONE_CUBE);
	debugAddButton(ZONES_MENU, Common::Rect(205, 139, 350, 159), "Camera Zones", 215, 144, 2, 87, 119, 0, SHOW_ZONE_CAMERA);
	debugAddButton(ZONES_MENU, Common::Rect(205, 160, 350, 180), "Scenaric Zones", 215, 165, 3, 87, 119, 0, SHOW_ZONE_SCENARIC);
	debugAddButton(ZONES_MENU, Common::Rect(205, 181, 350, 201), "Celling Grid Zones", 215, 186, 4, 87, 119, 0, SHOW_ZONE_CELLINGGRID);
	debugAddButton(ZONES_MENU, Common::Rect(205, 202, 350, 222), "Object Zones", 215, 207, 5, 87, 119, 0, SHOW_ZONE_OBJECT);
	debugAddButton(ZONES_MENU, Common::Rect(205, 223, 350, 243), "Text Zones", 215, 228, 6, 87, 119, 0, SHOW_ZONE_TEXT);
	debugAddButton(ZONES_MENU, Common::Rect(205, 244, 350, 264), "Ladder Zones", 215, 249, 7, 87, 119, 0, SHOW_ZONE_LADDER);
}

int32 Debug::debugProcessButton(int32 x, int32 y) {
	for (int32 i = 0; i < numDebugWindows; i++) {
		for (int32 j = 0; j < debugWindows[i].numButtons; j++) {
			const Common::Rect &rect = debugWindows[i].debugButtons[j].rect;
			if (rect.contains(x, y)) {
				return debugWindows[i].debugButtons[j].type;
			}
		}
	}

	return 0;
}

void Debug::debugPlasmaWindow(const char *text, int32 color) {
	_engine->_menu->processPlasmaEffect(Common::Rect(0, 0, PLASMA_WIDTH, PLASMA_HEIGHT), color);
	if (!(_engine->getRandomNumber() % 5)) {
		_engine->_menu->plasmaEffectPtr[_engine->getRandomNumber() % PLASMA_WIDTH * 10 + 6400] = 255;
	}
	const int32 textSize = _engine->_text->getTextSize(text);
	_engine->_text->drawText((SCREEN_WIDTH / 2) - (textSize / 2), 10, text);
	const Common::Rect rect(5, 5, 634, 50);
	_engine->_menu->drawBox(rect);
	_engine->copyBlockPhys(rect);
}

void Debug::debugProcessWindow() {
	if (!_engine->_input->toggleActionIfActive(TwinEActionType::DebugMenu)) {
		return;
	}
	const char *text = "Game Debug Window";
	int32 colorIdx = 4;
	int32 count = 0;

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	debugResetButtonsState();
	if (numDebugWindows == 0) {
		debugLeftMenu();
	}
	debugDrawWindows();

	for (;;) {
		ScopedFPS scopedFps(25);
		_engine->readKeys();
		if (_engine->shouldQuit()) {
			break;
		}
		const Common::Point &point = _engine->_input->getMousePositions();

		if (_engine->_input->toggleActionIfActive(TwinEActionType::DebugMenuActivate)) {
			int type = 0;
			if ((type = debugProcessButton(point.x, point.y)) != NO_ACTION) { // process menu item
				if (debugTypeUseMenu(type)) {
					_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
					_engine->copyBlockPhys(205, 55, 634, 474);
				}

				debugRefreshButtons(type);
				debugSetActions(type);
			}
		}

		// draw window plasma effect
		if (count == 256) {
			colorIdx++;
			count = 0;
		}
		int32 color = colorIdx * 16;
		if (color >= 240) {
			color = 64;
			colorIdx = 4;
		}
		debugPlasmaWindow(text, color);

		if (_engine->_input->toggleActionIfActive(TwinEActionType::DebugMenu)) {
			break;
		}

		count++;
	}
	_engine->_redraw->reqBgRedraw = true;
}

void Debug::processDebug() {
	if (!_engine->cfgfile.Debug) {
		return;
	}
	debugProcessWindow();

	_engine->_debugGrid->changeGrid();
	_engine->_debugGrid->changeGridCamera();
	_engine->_debugGrid->applyCellingGrid();
}

} // namespace TwinE
