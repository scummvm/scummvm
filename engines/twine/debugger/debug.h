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

#ifndef TWINE_DEBUG_H
#define TWINE_DEBUG_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace TwinE {

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

struct DebugButtonStruct {
	Common::Rect rect { 0, 0, 0, 0 };
	const char *text = "";
	int32 textLeft = 0;
	int32 textTop = 0;
	int32 isActive = 0;
	int32 color = 0;
	int32 activeColor = 0;
	int32 submenu = 0;
	int32 type = 0;
};

struct DebugWindowStruct {
	Common::Rect rect { 0, 0, 0, 0 };
	int32 alpha = 0;
	int32 isActive = 0;
	int32 numLines = 0;
	const char *text[20] {0};
	int32 numButtons = 0;
	DebugButtonStruct debugButtons[50];
};

class TwinEEngine;

class Debug {
private:
	TwinEEngine *_engine;

	DebugWindowStruct debugWindows[10];
	int32 numDebugWindows = 0;
	void debugFillButton(int32 x, int32 y, int32 width, int32 height, int8 color);
	void debugDrawButton(const Common::Rect &rect, const char *text, int32 textLeft, int32 textRight, int32 isActive, int8 color);
	void debugDrawWindowBox(const Common::Rect &rect, int32 alpha);
	void debugDrawWindowButtons(int32 w);
	void debugDrawWindow(int32 w);
	int32 debugTypeUseMenu(int32 type);
	void debugResetButtonsState();
	void debugRefreshButtons(int32 type);
	void debugDrawWindows();
	void debugResetButton(int32 type);
	void debugRedrawScreen();
	int32 debugGetActionsState(int32 type);
	void debugSetActions(int32 type);
	void debugAddButton(int32 window, const Common::Rect &rect, const char *text, int32 textLeft, int32 textTop, int32 isActive, int32 color, int32 activeColor, int32 submenu, int32 type);
	void debugAddWindowText(int32 window, const char *text);
	void debugAddWindow(const Common::Rect &rect, int32 alpha, int32 isActive);
	void debugLeftMenu();
	int32 debugProcessButton(int32 x, int32 y);
	void debugPlasmaWindow(const char *text, int32 color);
	void debugProcessWindow();

public:
	Debug(TwinEEngine *engine) : _engine(engine) {}
	void processDebug();
};

} // namespace TwinE

#endif
