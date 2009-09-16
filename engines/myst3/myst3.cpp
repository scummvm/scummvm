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
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/error.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "engines/engine.h"

#include "engines/myst3/myst3.h"

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
		Engine(syst), _system(syst) {

}

Myst3Engine::~Myst3Engine() {

}

Common::Error Myst3Engine::run() {
	const int w = 800;
	const int h = 600;
	const char archiveFileName[] = "MAISnodes.m3a";
	const int roomID = 2;
	
	if (!_archive.open(archiveFileName)) {
		error("Unable to open archive");
	}
	
	_system->setupScreen(w, h, false, true);
	_system->showMouse(false);
	
	_scene.init(w, h);

	_room.load(_archive, roomID);
	
	for(;;) {
		// Process events
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT) {
				return Common::kNoError;
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				_scene.updateCamera(event.relMouse);
			}
		}
		
		_scene.clear();
		_scene.setupCamera();

		_room.draw();

		_system->updateScreen();
		_system->delayMillis(10);
	}

	_archive.close();

	return Common::kNoError;
}

} // end of namespace Myst3
