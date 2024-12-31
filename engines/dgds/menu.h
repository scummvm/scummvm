/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	// Request data numbers, most are same in both RoTD and China VCRs
	kMenuNone = 0,
	kMenuMainBeamish = 3,
	kMenuMain = 5,
	kMenuControls = 7,
	kMenuOptions = 29,
	kMenuCalibrate = 30,
	kMenuFiles = 21,
	kMenuGameOver = 41,
	kMenuRestart = 39,
	kMenuCantSave = 36,
	kMenuSaveNeedName = 43,
	kMenuNotSavedDiskFull = 42,
	kMenuReplaceSave = 38,
	kMenuSaveDlg = 23,
	kMenuChangeDir = 25,
	kMenuAskCreateDir = 26,
	kMenuInvalidDir = 37,
	kMenuCalibrateJoystick = 28,
	kMenuCalibrateMouse = 32,
	kMenuReallyQuit = 35,
	kMenuReplayArcade = 45,
	kMenuSaveBeforeArcade = 46,
	kMenuArcadeFrustrated = 47,
	kMenuSkipPlayIntro = 50,
	kMenuSkipArcade = 52,
	kMenuWillyHelp = 54,
	kMenuWillyCredits = 56,
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
	void onMouseLUp(const Common::Point &mouse);
	void onMouseLDown(const Common::Point &mouse);
	void onMouseMove(const Common::Point &mouse);
	bool menuShown() const { return _curMenu != kMenuNone; }
	void hideMenu() { _curMenu = kMenuNone; }

	void setRequestData(const REQFileData &data);
	void readRESData(const char *fname);
	void prevChoice();
	void nextChoice();
	void activateChoice();
	void onTick();

private:
	Gadget *getClickedMenuItem(const Common::Point &mouseClick);
	void drawMenuText(Graphics::ManagedSurface &dst);
	void drawCreditsText(Graphics::ManagedSurface &dst);
	void toggleGadget(int16 gadgetId, bool enable);
	void configureGadget(MenuId menu, Gadget *gadget);
	bool updateOptionsGadget(Gadget *gadget);
	void handleClick(const Common::Point &mouse);
	void handleClickOptionsMenu(const Common::Point &mouse);
	void handleClickSkipPlayIntroMenu(const Common::Point &mouse);
	void loadCredits();

	void putMouseOnSelectedItem();
	Gadget *getSelectedItem();

	Common::HashMap<int, RequestData> _menuRequests;

	Common::Array<Common::Array<Common::String>> _helpStrings;
	Common::Array<Common::String> _credits;

	SliderGadget *_dragGadget;
	Common::Point _dragStartPt;
	int _selectedItem;
	int _numSelectable;
	int _creditsOffset;
};

} // End of namespace Dgds

#endif // DGDS_DGDS_H
