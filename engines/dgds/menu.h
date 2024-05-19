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

#ifndef DGDS_MENU_H
#define DGDS_MENU_H

#include "common/error.h"
#include "common/events.h"
#include "common/platform.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "gui/debugger.h"

#include "dgds/resource.h"
#include "dgds/request.h"

namespace Dgds {

class REQFileData;

enum MenuId {
	kMenuNone = -1,
	kMenuMain = 0,
	kMenuControls = 1,
	kMenuOptions = 2,
	kMenuCalibrate = 3,
	kMenuRestart = 4,
	// 5: you cannot save your game right now
	// 6: game over
	kMenuFiles = 7,
	// 8: save game not saved because disk is full
	// 9: all game entries are full
	kMenuSave = 10,
	// 11: change directory - create directory
	// 12: change directory - invalid directory specified
	kMenuChangeDirectory = 13,
	kMenuJoystick = 14,
	kMenuMouse = 15,
	kMenuQuit = 16,
	// 17: I'm frustrated - keep trying / win arcade
	kMenuSkipPlayIntro = 18,
	// 19: save game before arcade
	// 20: replay arcade
};

class Menu {
private:
	Graphics::Surface _screenBuffer;
	MenuId _curMenu = kMenuMain;

public:
	Menu();
	virtual ~Menu();

	void setScreenBuffer();
	void drawMenu(MenuId menu = kMenuMain);
	void handleMenu(Common::Point &mouse);
	bool menuShown() const { return _curMenu != kMenuNone; }
	void hideMenu() { _curMenu = kMenuNone; }

	void setRequestData(const REQFileData &data) {
		_reqData = data;
	}

private:
	int16 getClickedMenuItem(Common::Point mouseClick);
	void drawMenuText(Graphics::ManagedSurface &dst);
	void toggleGadget(int16 gadgetId, bool enable);
	REQFileData _reqData;
};

} // End of namespace Dgds

#endif // DGDS_DGDS_H
