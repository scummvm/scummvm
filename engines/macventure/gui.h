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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef MACVENTURE_GUI_H
#define MACVENTURE_GUI_H

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macmenu.h"

#include "graphics/font.h"

#include "macventure/image.h"

namespace MacVenture {

using namespace Graphics::MacGUIConstants;
using namespace Graphics::MacWindowConstants;
class MacVentureEngine;
typedef uint32 ObjID;

//namespace MacVentureMenuActions {
enum MenuAction {
	kMenuActionAbout,
	kMenuActionNew,
	kMenuActionOpen,
	kMenuActionSave,
	kMenuActionSaveAs,
	kMenuActionQuit,
	kMenuActionUndo,
	kMenuActionCut,
	kMenuActionCopy,
	kMenuActionPaste,
	kMenuActionClear,
	kMenuActionCleanUp,
	kMenuActionMessUp,

	kMenuActionCommand
};
//} using namespace MacVentureMenuActions;

enum WindowReference {
	kCommandsWindow = 0x80,
	kMainGameWindow = 0x81,
	kOutConsoleWindow = 0x82,
	kSelfWindow = 0x83,
	kExitsWindow = 0x84,
	kDiplomaWindow = 0x85
};

enum MVWindowType {
	kDocument = 0x00,
	kDBox = 0x01,
	kPlainDBox = 0x02,
	kAltBox = 0x03,
	kNoGrowDoc = 0x04,
	kMovableDBox = 0x05,
	kZoomDoc = 0x08,
	kZoomNoGrow = 0x0c,
	kRDoc16 = 0x10,
	kRDoc4 = 0x12,
	kRDoc6 = 0x14,
	kRDoc10 = 0x16
};

struct WindowData {
	Common::Rect bounds;
	MVWindowType type;
	uint16 visible;
	uint16 hasCloseBox;
	WindowReference refcon;
	uint8 titleLength;
	Common::String title;
	Common::Array<ObjID> children;
	bool updateScroll;
};

enum ControlReference {
	kControlExitBox = 0,
	kControlExamine = 1,
	kControlOpen = 2,
	kControlClose = 3,
	kControlSpeak = 4,
	kControlOperate = 5,
	kControlGo = 6,
	kControlHit = 7,
	kControlConsume = 8
};

struct ControlData {
	Common::Rect bounds;
	uint16 scrollValue;
	uint8 visible;
	uint16 scrollMax;
	uint16 scrollMin;
	uint16 cdef;
	uint32 refcon;
	uint8 titleLength;
	char* title;
	uint16 border;
};

struct BorderBounds {
	uint16 leftOffset;
	uint16 topOffset;
	uint16 rightOffset;
	uint16 bottomOffset;

	BorderBounds(uint16 l, uint16 t, uint16 r, uint16 b) : leftOffset(l), topOffset(t), rightOffset(r), bottomOffset(b) {}
};


class Gui {

public:
	Gui(MacVentureEngine *engine, Common::MacResManager *resman);
	~Gui();

	void draw();
	void drawMenu();
	bool processEvent(Common::Event &event);
	void handleMenuAction(MenuAction action);
	void updateWindow(WindowReference winID, bool containerOpen);

	WindowReference createInventoryWindow();
	bool tryCloseWindow(WindowReference winID);

	// Event processors
	bool processCommandEvents(WindowClick click, Common::Event &event);
	bool processMainGameEvents(WindowClick click, Common::Event &event);
	bool processOutConsoleEvents(WindowClick click, Common::Event &event);
	bool processSelfEvents(WindowClick click, Common::Event &event);
	bool processExitsEvents(WindowClick click, Common::Event &event);
	bool processDiplomaEvents(WindowClick click, Common::Event &event);
	bool processInventoryEvents(WindowClick click, Common::Event &event);

	const WindowData& getWindowData(WindowReference reference);

	const Graphics::Font& getCurrentFont();

	// Modifiers
	void bringToFront(WindowReference window);
	void setWindowTitle(WindowReference winID, Common::String string);
	void updateWindowInfo(WindowReference ref, ObjID objID, const Common::Array<ObjID> &children);

	// Ugly switches
	BorderBounds borderBounds(MVWindowType type);

private: // Attributes

	MacVentureEngine *_engine;
	Common::MacResManager *_resourceManager;

	Graphics::ManagedSurface _screen;
	Graphics::MacWindowManager _wm;

	Common::List<WindowData> *_windowData;
	Common::List<CommandButton> *_controlData;

	Graphics::MacWindow *_controlsWindow;
	Graphics::MacWindow *_mainGameWindow;
	Graphics::MacWindow *_outConsoleWindow;
	Graphics::MacWindow *_selfWindow;
	Graphics::MacWindow *_exitsWindow;
	Graphics::MacWindow *_diplomaWindow;
	Common::Array<Graphics::MacWindow*> _inventoryWindows;
	Graphics::Menu *_menu;

private: // Methods


	// Initializers
	void initGUI();
	void initWindows();

	// Loaders
	bool loadMenus();
	bool loadWindows();
	bool loadControls();
	void loadBorder(Graphics::MacWindow * target, Common::String filename, bool active);

	// Drawers
	void drawWindows();
	void drawCommandsWindow();
	void drawMainGameWindow();
	void drawSelfWindow();

	// Finders
	WindowData& findWindowData(WindowReference reference);

};

class CommandButton {

enum {
	kCommandsLeftPadding = 0,
	kCommandsTopPadding = 0
};

public:

	CommandButton() {
		_gui = nullptr;
	}

	CommandButton(ControlData data, Gui *g) {
		_data = data;
		_gui = g;
	}
	~CommandButton() {}

	void draw(Graphics::ManagedSurface &surface) const {

		surface.fillRect(_data.bounds, kColorWhite);
		surface.frameRect(_data.bounds, kColorBlack);

		const Graphics::Font &font = _gui->getCurrentFont();
		Common::String title(_data.title);
		font.drawString(
			&surface,
			title,
			_data.bounds.left,
			_data.bounds.top,
			_data.bounds.right - _data.bounds.left,
			kColorBlack,
			Graphics::kTextAlignCenter);
	}

	bool isInsideBounds(const Common::Point point) const {
		return _data.bounds.contains(point);
	}

	const ControlData& getData() const {
		return _data;
	}

private:
	ControlData _data;
	Gui *_gui;
};

} // End of namespace MacVenture

#endif
