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

namespace MacVenture {

using namespace Graphics::MacGUIConstants;
using namespace Graphics::MacWindowConstants;
class MacVentureEngine;

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

class Gui {

public:
	Gui(MacVentureEngine *engine, Common::MacResManager *resman);
	~Gui();

	void draw();
	bool processEvent(Common::Event &event);
	void handleMenuAction(MenuAction action);
	bool processCommandEvents(WindowClick click, Common::Event &event);

private: // Attributes

	MacVentureEngine *_engine;
	Common::MacResManager *_resourceManager;

	Graphics::ManagedSurface _screen;
	Graphics::MacWindowManager _wm;

	Graphics::MacWindow *_outConsoleWindow;
	Graphics::Menu *_menu;

private: // Methods

	void initGUI();
	bool loadMenus();
	void loadBorder(Graphics::MacWindow * target, Common::String filename, bool active);

};

class CommandButton {

enum {
	kCommandsLeftPadding = 0,
	kCommandsTopPadding = 0
};

public:
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
