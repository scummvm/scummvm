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

#include "common/endian.h"
#include "common/system.h"

#include "engines/stark/gfx_opengl.h"
#include "engines/stark/stark.h"

#ifdef USE_OPENGL

namespace Stark {

GfxOpenGL::GfxOpenGL() {
	_storedDisplay = NULL;
	_emergFont = 0;
}

GfxOpenGL::~GfxOpenGL() {
	delete[] _storedDisplay;
	if (_emergFont && glIsList(_emergFont))
		glDeleteLists(_emergFont, 128);
}

byte *GfxOpenGL::setupScreen(int screenW, int screenH, bool fullscreen) {
	g_system->setupScreen(screenW, screenH, fullscreen, true);

	_screenWidth = screenW;
	_screenHeight = screenH;
	_screenBPP = 24;
	_isFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);

	char GLDriver[1024];
	sprintf(GLDriver, "Residual: %s/%s", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	g_system->setWindowCaption(GLDriver);

	_storedDisplay = new byte[_screenWidth * _screenHeight * 4];
	memset(_storedDisplay, 0, _screenWidth * _screenHeight * 4);

	GLfloat ambientSource[] = { 0.6f, 0.6f, 0.6f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientSource);

	return NULL;
}

const char *GfxOpenGL::getVideoDeviceName() {
	return "OpenGL Renderer";
}

void GfxOpenGL::setupCamera(float fov, float nclip, float fclip, float roll) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float right = nclip * tan(fov / 2 * (LOCAL_PI / 180));
	glFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(roll, 0, 0, -1);
}

void GfxOpenGL::positionCamera(Graphics::Vector3d pos, Graphics::Vector3d interest) {
	Graphics::Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Graphics::Vector3d(0, 1, 0);

	gluLookAt(pos.x(), pos.y(), pos.z(), interest.x(), interest.y(), interest.z(), up_vec.x(), up_vec.y(), up_vec.z());
}

void GfxOpenGL::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GfxOpenGL::flipBuffer() {
	g_system->updateScreen();
}

bool GfxOpenGL::isHardwareAccelerated() {
	return true;
}

static void glShadowProjection(Graphics::Vector3d light, Graphics::Vector3d plane, Graphics::Vector3d normal, bool dontNegate) {
	// Based on GPL shadow projection example by
	// (c) 2002-2003 Phaetos <phaetos@gaffga.de>
	float d, c;
	float mat[16];
	float nx, ny, nz, lx, ly, lz, px, py, pz;

	// for some unknown for me reason normal need negation
	nx = -normal.x();
	ny = -normal.y();
	nz = -normal.z();
	if (dontNegate) {
		nx = -nx;
		ny = -ny;
		nz = -nz;
	}
	lx = light.x();
	ly = light.y();
	lz = light.z();
	px = plane.x();
	py = plane.y();
	pz = plane.z();

	d = nx * lx + ny * ly + nz * lz;
	c = px * nx + py * ny + pz * nz - d;

	mat[0] = lx * nx + c;
	mat[4] = ny * lx;
	mat[8] = nz * lx;
	mat[12] = -lx * c - lx * d;

	mat[1] = nx * ly;
	mat[5] = ly * ny + c;
	mat[9] = nz * ly;
	mat[13] = -ly * c - ly * d;

	mat[2] = nx * lz;
	mat[6] = ny * lz;
	mat[10] = lz * nz + c;
	mat[14] = -lz * c - lz * d;

	mat[3] = nx;
	mat[7] = ny;
	mat[11] = nz;
	mat[15] = -d;

	glMultMatrixf((GLfloat *)mat);
}

void GfxOpenGL::set3DMode() {
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void GfxOpenGL::translateViewpointStart(Graphics::Vector3d pos, float pitch, float yaw, float roll) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(pos.x(), pos.y(), pos.z());
	glRotatef(yaw, 0, 0, 1);
	glRotatef(pitch, 1, 0, 0);
	glRotatef(roll, 0, 1, 0);
}

void GfxOpenGL::translateViewpointFinish() {
	glPopMatrix();
}

void GfxOpenGL::drawBitmap(Surface* bmp) {
	glPixelZoom(1.0f, -1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glRasterPos2f(-1.0f, 0.75f);
	glDrawPixels(bmp->_width, bmp->_height, GL_RGB, GL_UNSIGNED_BYTE, bmp->_pixels);
}

} // end of namespace Stark

#endif
