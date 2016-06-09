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

#include "common/file.h"
#include "image/bmp.h"

#include "macventure/macventure.h"
#include "macventure/gui.h"

namespace MacVenture {

enum MenuAction;

enum {
	kMenuHighLevel = -1,
	kMenuAbout = 0,
	kMenuFile = 1,
	kMenuEdit = 2,
	kMenuSpecial = 3
};

static const Graphics::MenuData menuSubItems[] = {
	{ kMenuHighLevel,	"File",				0, 0, false },
	{ kMenuHighLevel,	"Edit",				0, 0, false },		
	{ kMenuHighLevel,	"Special",			0, 0, false },
	{ kMenuHighLevel,	"Font",				0, 0, false },
	{ kMenuHighLevel,	"FontSize",			0, 0, false },

	//{ kMenuAbout,		"About",			kMenuActionAbout, 0, true},

	{ kMenuFile,		"New",				kMenuActionNew, 0, true },
	{ kMenuFile,		NULL,				0, 0, false },
	{ kMenuFile,		"Open...",			kMenuActionOpen, 0, true },
	{ kMenuFile,		"Save",				kMenuActionSave, 0, true },
	{ kMenuFile,		"Save as...",		kMenuActionSaveAs, 0, true },
	{ kMenuFile,		NULL,				0, 0, false },
	{ kMenuFile,		"Quit",				kMenuActionQuit, 0, true },

	{ kMenuEdit,		"Undo",				kMenuActionUndo, 'Z', true },
	{ kMenuEdit,		NULL,				0, 0, false },
	{ kMenuEdit,		"Cut",				kMenuActionCut, 'K', true },
	{ kMenuEdit,		"Copy",				kMenuActionCopy, 'C', true },
	{ kMenuEdit,		"Paste",			kMenuActionPaste, 'V', true },
	{ kMenuEdit,		"Clear",			kMenuActionClear, 'B', true },

	{ kMenuSpecial,		"Clean Up",			kMenuActionCleanUp, 0, true },
	{ kMenuSpecial,		"Mess Up",			kMenuActionMessUp, 0, true },

	{ 0,				NULL,				0, 0, false }
};

bool outConsoleWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
void menuCommandsCallback(int action, Common::String &text, void *data);

Gui::Gui(MacVentureEngine *engine, Common::MacResManager *resman) {
	_engine = engine;
	_resourceManager = resman;
	initGUI();
}

Gui::~Gui() {

}

void Gui::draw() {
	_wm.draw();
}

bool Gui::processEvent(Common::Event &event) {
	return _wm.processEvent(event);
}

void Gui::initGUI() {
	_screen.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_wm.setScreen(&_screen);

	// Menu
	_menu = _wm.addMenu();
	if (!loadMenus())
		error("Could not load menus");
	_menu->setCommandsCallback(menuCommandsCallback, this);
	_menu->calcDimensions();

	// In-game Output Console
	_outConsoleWindow = _wm.addWindow(false, true, true);
	_outConsoleWindow->setDimensions(Common::Rect(20, 20, 120, 120));
	_outConsoleWindow->setActive(false);
	_outConsoleWindow->setCallback(outConsoleWindowCallback, this);
	loadBorder(_outConsoleWindow, "border_inac.bmp", false);	
}

void Gui::loadBorder(Graphics::MacWindow * target, Common::String filename, bool active) {
	Common::File borderfile;

	if (!borderfile.open(filename)) {
		debug(1, "Cannot open border file");
		return;
	}

	Image::BitmapDecoder bmpDecoder;
	Common::SeekableReadStream *stream = borderfile.readStream(borderfile.size());
	Graphics::Surface source;
	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface();

	if (stream) {
		debug(4, "Loading %s border from %s", (active ? "active" : "inactive"), filename);
		bmpDecoder.loadStream(*stream);
		source = *(bmpDecoder.getSurface());

		source.convertToInPlace(surface->getSupportedPixelFormat(), bmpDecoder.getPalette());
		surface->create(source.w, source.h, source.format);
		surface->copyFrom(source);
		surface->applyColorKey(255, 0, 255, false);

		target->setBorder(*surface, active);

		borderfile.close();

		delete stream;
	}
}

bool Gui::loadMenus() {

	// We assume that, if there are static menus, we don't need dynamic ones
	if (menuSubItems) {
		_menu->addStaticMenus(menuSubItems);
		return true;
	}

	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	if ((resArray = _resourceManager->getResIDArray(MKTAG('M', 'E', 'N', 'U'))).size() == 0)
		return false;	

	_menu->addMenuSubItem(0, "Abb", kMenuActionAbout, 0, 'A', true);

	int i = 1;
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('M', 'E', 'N', 'U'), *iter);
		bool enabled;
		uint16 key;
		uint16 style;
		uint8 titleLength;
		char* title;

		int menunum = -1; // High level menus have level -1
		/* Skip menuID, width, height, resourceID, placeholder */
		for (int skip = 0; skip < 5; skip++) { res->readUint16BE(); }
		enabled = res->readUint32BE();
		titleLength = res->readByte();
		title = new char[titleLength + 1];
		res->read(title, titleLength);
		title[titleLength] = '\0';

		if (titleLength > 1) {
			_menu->addMenuItem(title);

			// Read submenu items
			while (titleLength = res->readByte()) {
				title = new char[titleLength + 1];
				res->read(title, titleLength);
				title[titleLength] = '\0';
				// Skip icon
				res->readUint16BE();
				// Read key
				key = res->readUint16BE();
				// Skip mark
				res->readUint16BE();
				// Read style
				style = res->readUint16BE();
				_menu->addMenuSubItem(i, title, 0, style, key, false);
			}
		}	

		i++;
	}

	return true;	
}

/* CALLBACKS */
bool outConsoleWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui) {
	return true;
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	Gui *g = (Gui *)data;

	g->handleMenuAction((MenuAction)action);
}

/* HANDLERS */
void Gui::handleMenuAction(MenuAction action) {
	switch (action)	{
	case MacVenture::kMenuActionAbout:
		debug("MacVenture Menu Action: About");
		break;
	case MacVenture::kMenuActionNew:
		debug("MacVenture Menu Action: New");
		break;
	case MacVenture::kMenuActionOpen:
		debug("MacVenture Menu Action: Open");
		break;
	case MacVenture::kMenuActionSave:
		debug("MacVenture Menu Action: Save");
		break;
	case MacVenture::kMenuActionSaveAs:
		debug("MacVenture Menu Action: Save As");
		break;
	case MacVenture::kMenuActionQuit:
		debug("MacVenture Menu Action: Quit");
		break;
	case MacVenture::kMenuActionUndo:
		debug("MacVenture Menu Action: Undo");
		break;
	case MacVenture::kMenuActionCut:
		debug("MacVenture Menu Action: Cut");
		break;
	case MacVenture::kMenuActionCopy:
		debug("MacVenture Menu Action: Copy");
		break;
	case MacVenture::kMenuActionPaste:
		debug("MacVenture Menu Action: Paste");
		break;
	case MacVenture::kMenuActionClear:
		debug("MacVenture Menu Action: Clear");
		break;
	case MacVenture::kMenuActionCleanUp:
		debug("MacVenture Menu Action: Clean Up");
		break;
	case MacVenture::kMenuActionMessUp:
		debug("MacVenture Menu Action: Mess Up");
		break;
	case MacVenture::kMenuActionCommand:
		debug("MacVenture Menu Action: GENERIC");
		break;
	default:
		break;
	}
}

} // End of namespace MacVenture
