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
#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"

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

void sbInit() {

		for (int i = 0; i < 6; i++) {
			char fileName[250];
			sprintf(fileName, "1-%d.jpg", i + 1);

			Common::File jpegFile;
			if (!jpegFile.open(fileName)) {
				error("Unable to open cube face %d", i);
			}
					
			Graphics::JPEG jpeg;
			jpeg.read(&jpegFile);

			room.setFaceTextureJPEG(i, &jpeg);

			jpegFile.close();

		}
}

void DrawSkyBox(float camera_yaw, float camera_pitch)
{
	// Réglage de l'orientation
	glPushMatrix();
	glLoadIdentity();
	glRotatef( camera_pitch, 1.0f, 0.0f, 0.0f );
	glRotatef( camera_yaw, 0.0f, 1.0f, 0.0f );	
	
	room.draw();

	// Réinitialisation de la matrice ModelView
	glPopMatrix();
}


void Render(float camera_yaw, float camera_pitch)
{
	// Configuration des états OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D); 
	glDisable(GL_LIGHTING);

	// Désactivation de l'écriture dans le DepthBuffer
	glDepthMask(GL_FALSE);

	// Rendu de la SkyBox
	DrawSkyBox(camera_yaw, camera_pitch);

	// Réactivation de l'écriture dans le DepthBuffer
	glDepthMask(GL_TRUE);

	// Réinitialisation des états OpenGL
	glDisable(GL_TEXTURE_2D); 
	glEnable(GL_LIGHTING);
}

void Myst3Engine::dumpArchive(const char *fileName) {
	Common::File archiveFile;
	if (!archiveFile.open(fileName)) {
		error("Unable to open archive");
	}
	
	Archive archive;
	archive.readFromStream(archiveFile);
	archive.dumpDirectory();
	archive.dumpToFiles(archiveFile);
	
	archiveFile.close();
}

Myst3Engine::Myst3Engine(OSystem *syst, int gameFlags) :
		Engine(syst), _system(syst) {

}

Myst3Engine::~Myst3Engine() {

}

Common::Error Myst3Engine::run() {
	//dumpArchive("MAISnodes.m3a");
	
	int w = 800;
	int h = 600;
	
	_system->setupScreen(w, h, false, true);
	
	//glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective (45.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	sbInit();
	
	
	
	for(;;) {
		// Process events
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT)
				return Common::kNoError;
		}
		
		// Initialisation des etats OpenGL
		glEnable( GL_COLOR_MATERIAL );
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );
		glEnable( GL_DEPTH_TEST );

		// Initialisation de la scene
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
		glLoadIdentity();
		glColor3f( 1.0f, 1.0f, 1.0f );

		// Placement de la camera	
		//glTranslatef( 0.0f, 0.0f, -8.0f );
		glRotatef( CAMERA_Pitch, 1.0f, 0.0f, 0.0f );
		glRotatef( CAMERA_Yaw, 0.0f, 1.0f, 0.0f );	

CAMERA_Yaw += 0.1f;
//CAMERA_Pitch += 0.2f;

		// Rendu de la skybox
		Render( CAMERA_Yaw, CAMERA_Pitch );

		// Reinitialisation des etats OpenGL
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_LIGHT0 );
		glDisable( GL_LIGHTING );
		glDisable( GL_COLOR_MATERIAL );

		// Dession de l'image
		_system->updateScreen();
		_system->delayMillis(10);
		//glutSwapBuffers();
		//glutPostRedisplay();
	}

	return Common::kNoError;
}

} // end of namespace Myst3
