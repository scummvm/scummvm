/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/events.h"
#include "common/error.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "gui/debugger.h"

#include "engines/engine.h"

#include "engines/myst3/myst3.h"
#include "engines/myst3/database.h"

#include "graphics/jpeg.h"
#include "graphics/conversion.h"

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Myst3 {

Myst3Engine::Myst3Engine(OSystem *syst, int gameFlags) :
		Engine(syst), _system(syst), _scriptEngine(this),
		_db(0) {
	_console = new Console(this);
}

Myst3Engine::~Myst3Engine() {
	delete _console;
}

Common::Error Myst3Engine::run() {
	const int w = 800;
	const int h = 600;

	_db = new Database("M3.exe");

	goToNode(1, 245); // LEIS

	_system->setupScreen(w, h, false, true);
	_system->showMouse(false);

	_scene.init(w, h);
	_node.load(_archive, 1);
	
	for(;;) {
		// Process events
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT) {
				return Common::kNoError;
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				_scene.updateCamera(event.relMouse);
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				Common::Point mouse = _scene.getMousePos();
				NodePtr nodeData = _db->getNodeData(_node.getId());

				for (uint j = 0; j < nodeData->hotspots.size(); j++) {
					if (nodeData->hotspots[j].isPointInRects(mouse)) {
						_scriptEngine.run(&nodeData->hotspots[j].script);
					}
				}
			} else if (event.type == Common::EVENT_KEYDOWN) {
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_console->attach();
						_console->onFrame();
						_scene.init(w, h);
					}
					break;
				default:
					break;
				}
			}
		}
		
		_scene.clear();
		_scene.setupCamera();

		_node.draw();

		_system->updateScreen();
		_system->delayMillis(10);
	}

	_node.unload();

	_archive.close();

	delete _db;

	return Common::kNoError;
}

void Myst3Engine::goToNode(uint16 nodeID, uint8 roomID) {
	_node.unload();

	if (roomID != 0) {
		char roomName[8];

		_db->loadRoomScripts(roomID);

		_db->getRoomName(roomName);
		Common::String nodeFile = Common::String::format("%snodes.m3a", roomName);

		_archive.close();
		if (!_archive.open(nodeFile.c_str())) {
			error("Unable to open archive %s", nodeFile.c_str());
		}
	}

	_node.load(_archive, nodeID);
}

} // end of namespace Myst3
