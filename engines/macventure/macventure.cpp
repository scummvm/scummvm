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

#include "common/system.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"
#include "engines/util.h"

#include "macventure/macventure.h"

// To move
#include "common/file.h"

namespace MacVenture {

enum {
	kMaxMenuTitleLength = 30
};

MacVentureEngine::MacVentureEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst) {
	_gameDescription = gameDesc;
	_rnd = new Common::RandomSource("macventure");

	_debugger= NULL;

	debug("MacVenture::MacVentureEngine()");
}

MacVentureEngine::~MacVentureEngine() {
	debug("MacVenture::~MacVentureEngine()");

	DebugMan.clearAllDebugChannels();
	delete _rnd;
	delete _debugger;
	delete _gui;
}

Common::Error MacVentureEngine::run() {
	debug("MacVenture::MacVentureEngine::init()");

	initGraphics(kScreenWidth, kScreenHeight, true);

	_debugger = new Console(this);

	// Additional setup.
	debug("MacVentureEngine::init");	

	// Your main even loop should be (invoked from) here.
	debug("MacVentureEngine::go: Hello, World!");

	_resourceManager = new Common::MacResManager();
	if (!_resourceManager->open(getGameFileName()))
		error("Could not open %s as a resource fork", getGameFileName());

	if (!loadMenuData())
		error("Could not load menu data from %s", getGameFileName());

	_gui = new Gui(this);

	_shouldQuit = false;
	while (!_shouldQuit) {
		processEvents();

		_gui->draw();

		g_system->updateScreen();
		g_system->delayMillis(50);
	}

	return Common::kNoError;
}

void MacVentureEngine::processEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
			case Common::EVENT_QUIT:
				_shouldQuit = true;
				break;
			default:
				break;
		}
	}
}

bool MacVentureEngine::loadMenuData() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;
	
	if ((resArray = _resourceManager->getResIDArray(MKTAG('M', 'E', 'N', 'U'))).size() == 0)
		return false;

	_menuData = new Graphics::MenuData[resArray.size()];
	int i = 0;

	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('M', 'E', 'N', 'U'), *iter);

		Graphics::MenuData data;
		int menunum = -1;
		for (int i = 0; i < 5; i++) {
			res->readUint16BE();
			// Skip menuID, width, height, resourceID, placeholder
		}		
		bool enabled = res->readUint32BE();
		uint8 titleLength = res->readByte();
		char* title = new char[titleLength+1];
		res->read(title, titleLength);
		title[titleLength] = '\0';

		_menuData[i] = { menunum, title, 0, 0, enabled};
		i++;
	}

	// Override last value (end-of-menu) with our end-of-menu
	_menuData[resArray.size() - 1] = { 0, 0, 0, 0, false };

	return true;
}

Graphics::MenuData* MacVentureEngine::getMenuData() {
	return _menuData;
}


} // End of namespace MacVenture
