// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "driver_gl.h"		// Driver interface
#include "debug.h"		// error(), warning(), etc
#include "font.h"		// builtin emergency font

Driver *g_driver;

// Constructor. Should create the driver and open screens, etc.
Driver::Driver(int screenW, int screenH, int screenBPP) {
	char GLDriver[1024];

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (SDL_SetVideoMode(screenW, screenH, screenBPP, SDL_OPENGL) == 0)
		error("Could not initialize video");

	sprintf(GLDriver, "Residual: %s/%s", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	SDL_WM_SetCaption(GLDriver, "Residual");

	// Load emergency built-in font
	loadEmergFont();

	_smushNumTex = 0;
}

void Driver::setupCamera(float fov, float nclip, float fclip, float roll) {
	// Set perspective transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(std::atan(std::tan(fov_ / 2 * (M_PI / 180)) * 0.75) * 2 * (180 / M_PI), 4.0f / 3, nclip_, fclip_);

	float right = nclip * std::tan(fov / 2 * (M_PI / 180));
	glFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Vector3d up_vec(0, 0, 1);
	glRotatef(roll, 0, 0, -1);
}

void Driver::positionCamera(Vector3d pos, Vector3d interest) {
	Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Vector3d(0, 1, 0);

	gluLookAt(pos.x(), pos.y(), pos.z(), 
		interest.x(), interest.y(), interest.z(),
		up_vec.x(), up_vec.y(), up_vec.z());

}

void Driver::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Driver::flipBuffer() {
	SDL_GL_SwapBuffers();
}

void Driver::startActorDraw(Vector3d pos, float yaw, float pitch, float roll) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pos.x(), pos.y(), pos.z());
	glRotatef(yaw, 0, 0, 1);
	glRotatef(pitch, 1, 0, 0);
	glRotatef(roll, 0, 1, 0);
}

void Driver::finishActorDraw() {
	glPopMatrix();
}

void Driver::drawDepthBitmap(int x, int y, int w, int h, char *data) {
	//	if (num != 0) {
	//		warning("Animation not handled yet in GL texture path !\n");
	//	}

	if (y + h == 480) {
		glRasterPos2i(x, 479);
		glBitmap(0, 0, 0, 0, 0, -1, NULL);
	}
	else
		glRasterPos2i(x, y + h);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glDrawPixels(w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LESS);
}

void Driver::prepareSmushFrame(int width, int height, byte *bitmap) {
	// remove if already exist
	if (_smushNumTex > 0) {
		glDeleteTextures(_smushNumTex, _smushTexIds);
		delete[] _smushTexIds;
		_smushNumTex = 0;
	}

	// create texture
	_smushNumTex = ((width + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
		((height + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
	_smushTexIds = new GLuint[_smushNumTex];
	glGenTextures(_smushNumTex, _smushTexIds);
	for (int i = 0; i < _smushNumTex; i++) {
		glBindTexture(GL_TEXTURE_2D, _smushTexIds[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0,
			GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

	int curTexIdx = 0;
	for (int y = 0; y < height; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < width; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= width) ? (width - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= height) ? (height - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glTexSubImage2D(GL_TEXTURE_2D, 
				0,
				0, 0,
				t_width, t_height,
				GL_RGB,
				GL_UNSIGNED_SHORT_5_6_5,
				bitmap + (y * 2 * width) + (2 * x));
			curTexIdx++;
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	_smushWidth = width;
	_smushHeight = height;
}

void Driver::drawSmushFrame(int offsetX, int offsetY) {
	// prepare view
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// A lot more may need to be put there : disabling Alpha test, blending, ...
	// For now, just keep this here :-)

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	// draw
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_SCISSOR_TEST);

	glScissor(offsetX, 480 - (offsetY + _smushHeight),
		  _smushWidth, _smushHeight);

	int curTexIdx = 0;
	for (int y = 0; y < _smushHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _smushWidth; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= _smushWidth) ? (_smushWidth - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= _smushHeight) ? (_smushHeight - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(x + offsetX, y + offsetY);
			glTexCoord2f(1.0, 0.0);
			glVertex2i(x + offsetX + BITMAP_TEXTURE_SIZE, y + offsetY);
			glTexCoord2f(1.0, 1.0);
			glVertex2i(x + offsetX + BITMAP_TEXTURE_SIZE, y + offsetY + BITMAP_TEXTURE_SIZE);
			glTexCoord2f(0.0, 1.0);
			glVertex2i(x + offsetX, y + offsetY + BITMAP_TEXTURE_SIZE);
			glEnd();
			curTexIdx++;
		}
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

// Load emergency font
void Driver::loadEmergFont() {
	int i;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	emergFont = glGenLists(128);
	for (i=32;i<127;i++) {
		glNewList(emergFont + i, GL_COMPILE);
		glBitmap(8, 13, 0, 2, 10, 0, font[i-32]);
		glEndList();
	}
}

// Draw text string using emergency font
void Driver::drawEmergString(int x, int y, const char *text, Color &fgColor) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 640, 480, 0, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

        glColor3f(fgColor.red(), fgColor.green(), fgColor.blue());
        glRasterPos2i(x, y);

        glListBase(emergFont);
        //glCallLists(strlen(strrchr(text, '/')) - 1, GL_UNSIGNED_BYTE, strrchr(text, '/') + 1);
        glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLubyte *) text);

        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
}
