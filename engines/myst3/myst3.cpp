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
#include "engines/myst3/archive.h"
#include "engines/myst3/room.h"

#include "graphics/jpeg.h"
#include "graphics/conversion.h"

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Myst3 {

Room room;

float CAMERA_Pitch = 0.0f;
float CAMERA_Yaw = 0.0f;

void sbInit(const char *fileName, int index) {

	Archive archive;
	if (!archive.open(fileName)) {
		error("Unable to open archive");
	}

	room.load(archive, index);
		
	archive.close();
}

void setupCamera(float camera_yaw, float camera_pitch) {
	// Rotate the model to simulate the rotation of the camera
	glLoadIdentity();
	glRotatef( camera_pitch, 1.0f, 0.0f, 0.0f );
	glRotatef( camera_yaw, 0.0f, 1.0f, 0.0f );	
}

void init() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective (45.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glColor3f(1.0f, 1.0f, 1.0f);
}

void Myst3Engine::dumpArchive(const char *fileName) {
	Archive archive;
	if (!archive.open(fileName)) {
		error("Unable to open archive");
	}
	
	archive.dumpDirectory();
	archive.dumpToFiles();
	archive.close();
}

Myst3Engine::Myst3Engine(OSystem *syst, int gameFlags) :
		Engine(syst), _system(syst) {

}

Myst3Engine::~Myst3Engine() {

}

Common::Error Myst3Engine::run() {
	int w = 800;
	int h = 600;
	
	_system->setupScreen(w, h, false, true);
	
	init();

	sbInit("MAISnodes.m3a", 2);
	
	for(;;) {
		// Process events
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT)
				return Common::kNoError;
		}
		
		clear();

		CAMERA_Yaw += 0.1f;
		//CAMERA_Pitch += 0.2f;

		setupCamera(CAMERA_Yaw, CAMERA_Pitch);

		room.draw();

		_system->updateScreen();
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

} // end of namespace Myst3
