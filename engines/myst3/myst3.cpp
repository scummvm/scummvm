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

#include "graphics/jpeg.h"
#include "graphics/conversion.h"

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Myst3 {

GLuint cubeTextures[6];
float CAMERA_Pitch = 0.0f;
float CAMERA_Yaw = 0.0f;
static const int textureSize = 1024;

void sbInit() {

		// Chargement des six textures
		Graphics::Surface *texture_image[6];	
		
		for (int i = 0; i < 6; i++) {
			char fileName[250];
			sprintf(fileName, "1-%d.jpg", i + 1);

			Common::File jpegFile;
			if (!jpegFile.open(fileName)) {
				error("Unable to open cube face %d", i);
			}
			
			Graphics::JPEG jpeg;
			jpeg.read(&jpegFile);
			
			byte *y = (byte *)jpeg.getComponent(1)->getBasePtr(0, 0);
			byte *u = (byte *)jpeg.getComponent(2)->getBasePtr(0, 0);
			byte *v = (byte *)jpeg.getComponent(3)->getBasePtr(0, 0);
			
			texture_image[i] = new Graphics::Surface();
			texture_image[i]->create(jpeg.getComponent(1)->w, jpeg.getComponent(1)->h, 3);
			
			byte *ptr = (byte *)texture_image[i]->getBasePtr(0, 0);
			for (int j = 0; j < texture_image[i]->w * texture_image[i]->h; j++) {
				byte r, g, b;
				Graphics::YUV2RGB(*y++, *u++, *v++, r, g, b);
				*ptr++ = r;
				*ptr++ = g;
				*ptr++ = b;
			}
			
			jpegFile.close();
		}


		for (int i = 0; i < 6; i++)
		{
			// Génération d'une texture
			glGenTextures(1, &cubeTextures[i]);

			// Configuration de la texture courante
			glBindTexture(GL_TEXTURE_2D, cubeTextures[i]);

			if (texture_image[i])				
			{
				glTexImage2D(GL_TEXTURE_2D, 0, 3, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_image[i]->w, texture_image[i]->h, GL_RGB, GL_UNSIGNED_BYTE, texture_image[i]->pixels);
				//glTexImage2D(GL_TEXTURE_2D, 0, 3, texture_image[i]->w, texture_image[i]->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_image[i]->pixels);
								
				delete texture_image[i];
			}
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
}

void DrawSkyBox(float camera_yaw, float camera_pitch)
{
	// Taille du cube
	float t = 1.0f;

	// Portion de texture utilisée
	float s = 640 / (float)textureSize;

	// Réglage de l'orientation
	glPushMatrix();
	glLoadIdentity();
	glRotatef( camera_pitch, 1.0f, 0.0f, 0.0f );
	glRotatef( camera_yaw, 0.0f, 1.0f, 0.0f );	
	


	glBindTexture(GL_TEXTURE_2D, cubeTextures[4]);
	glBegin(GL_TRIANGLE_STRIP);			// X-
		glTexCoord2f(0, s); glVertex3f(-t,-t, t); 	
		glTexCoord2f(s, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f(-t, t, t);
		glTexCoord2f(s, 0); glVertex3f(-t, t,-t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, cubeTextures[3]);
	glBegin(GL_TRIANGLE_STRIP);			// X+
		glTexCoord2f(0, s); glVertex3f( t,-t,-t);
		glTexCoord2f(s, s); glVertex3f( t,-t, t);
		glTexCoord2f(0, 0); glVertex3f( t, t,-t); 
		glTexCoord2f(s, 0); glVertex3f( t, t, t); 	
	glEnd();

	glBindTexture(GL_TEXTURE_2D, cubeTextures[1]);
	glBegin(GL_TRIANGLE_STRIP);			// Y-
		glTexCoord2f(0, s); glVertex3f( t,-t,-t);
		glTexCoord2f(s, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f( t,-t, t);
		glTexCoord2f(s, 0); glVertex3f(-t,-t, t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, cubeTextures[5]);
	glBegin(GL_TRIANGLE_STRIP);			// Y+
		glTexCoord2f(0, s); glVertex3f( t, t, t);
		glTexCoord2f(s, s); glVertex3f(-t, t, t); 
		glTexCoord2f(0, 0); glVertex3f( t, t,-t);
		glTexCoord2f(s, 0); glVertex3f(-t, t,-t); 	
	glEnd();

	glBindTexture(GL_TEXTURE_2D, cubeTextures[0]);
	glBegin(GL_TRIANGLE_STRIP);			// Z-
		glTexCoord2f(0, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(s, s); glVertex3f( t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f(-t, t,-t);
		glTexCoord2f(s, 0); glVertex3f( t, t,-t); 
	glEnd();

	glBindTexture(GL_TEXTURE_2D, cubeTextures[2]);
	glBegin(GL_TRIANGLE_STRIP);			// Z+
		glTexCoord2f(0, s); glVertex3f( t,-t, t);
		glTexCoord2f(s, s); glVertex3f(-t,-t, t);
		glTexCoord2f(0, 0); glVertex3f( t, t, t);
		glTexCoord2f(s, 0); glVertex3f(-t, t, t);
	glEnd();

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
