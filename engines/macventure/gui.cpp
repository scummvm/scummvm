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

/* priority, name, action, shortcut, enabled*/
#define MV_MENU5(p, n, a, s, e) Graphics::MenuData{p, n, a, s, e}
#define MV_MENU4(p, n, a, s) Graphics::MenuData{p, n, a, s, false}
#define MV_MENUtop(n, a, s) Graphics::MenuData{-1, n, a, s, true}

static const Graphics::MenuData menuSubItems[] = {
	{ -1, "Hello World",	0, 0, false },
	{ 0, "How yo duin",	0, 0, false },
};

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
	_outConsoleWindow = _wm.addWindow(false, true, true);
	_outConsoleWindow->setDimensions(Common::Rect(20, 20, 120, 120));
	_outConsoleWindow->setActive(false);

	_menu = _wm.addMenu();

	if (!loadMenus())
		error("Could not load menus");

	_menu->calcDimensions();

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
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	if ((resArray = _resourceManager->getResIDArray(MKTAG('M', 'E', 'N', 'U'))).size() == 0)
		return false;

	_menu->addMenuItem("(c)");
	_menu->addMenuSubItem(0, "Hello", 0, 0, 'K', false);

	int i = 1;
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('M', 'E', 'N', 'U'), *iter);
		bool enabled;
		uint16 key;
		uint8 titleLength;
		char* title;

		Graphics::MenuData data;
		int menunum = -1; // High level menus have level -1
		/* Skip menuID, width, height, resourceID, placeholder */
		for (int skip = 0; skip < 5; skip++) { res->readUint16BE(); }
		enabled = res->readUint32BE();
		titleLength = res->readByte();
		title = new char[titleLength + 1];
		res->read(title, titleLength);
		title[titleLength] = '\0';

		if (titleLength > 2) {
			_menu->addMenuItem(title);

			// Read submenu items
			while (titleLength = res->readByte()) {
				title = new char[titleLength + 1];
				res->read(title, titleLength);
				title[titleLength] = '\0';
				// Skip icon
				res->readUint16BE();
				key = res->readUint16BE();
				// Skip key, mark, style
				for (int skip = 0; skip < 2; skip++) { res->readUint16BE(); }
				_menu->addMenuSubItem(i, title, 0, 0, key, false);
			}
		}	

		i++;
	}	

	return true;
	
}

} // End of namespace MacVenture
