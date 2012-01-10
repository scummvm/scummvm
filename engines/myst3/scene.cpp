/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/scene.h"
#include "engines/myst3/node.h"

namespace Myst3 {

Scene::Scene():
		_cameraPitch(0.0f), _cameraHeading(0.0f)
{
}

void Scene::init(int width, int height) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void Scene::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glColor3f(1.0f, 1.0f, 1.0f);
}

void Scene::setupCameraOrtho2D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, _originalWidth, _originalHeight, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Scene::setupCameraPerspective() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, (GLfloat)_originalWidth /(GLfloat)_originalHeight, 0.1, 100.0);

	// Rotate the model to simulate the rotation of the camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(_cameraPitch, -1.0f, 0.0f, 0.0f);
	glRotatef(_cameraHeading - 180.0f, 0.0f, 1.0f, 0.0f);
}

void Scene::updateCamera(Common::Point &mouse) {
	_cameraPitch -= mouse.y / 3.0f;
	_cameraHeading += mouse.x / 3.0f;

	// Keep heading in 0..360 range
	if (_cameraHeading > 360.0f)
		_cameraHeading -= 360.0f;
	else if (_cameraHeading < 0.0f)
		_cameraHeading += 360.0f;

	// Keep pitch within allowed values
	_cameraPitch = CLIP(_cameraPitch, -60.0f, 80.0f);
}

void Scene::lookAt(float pitch, float heading) {
	_cameraPitch = pitch;
	_cameraHeading = heading;
}

void Scene::drawBlackRect(const Common::Rect &r) {
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0f, 0.0f, 0.0f);

	glBegin(GL_TRIANGLE_STRIP);
		glVertex3f( r.left, r.bottom, 0.0f);
		glVertex3f( r.right, r.bottom, 0.0f);
		glVertex3f( r.left, r.top, 0.0f);
		glVertex3f( r.right, r.top, 0.0f);
	glEnd();
}

void Scene::drawBlackBorders() {
	Common::Rect top = Common::Rect(_originalWidth, _topBorderHeight);

	Common::Rect bottom = Common::Rect(_originalWidth, _bottomBorderHeight);
	bottom.translate(0, _topBorderHeight + _frameHeight);

	drawBlackRect(top);
	drawBlackRect(bottom);
}

void Scene::drawSunspotFlare(const SunSpot &s) {
	Common::Rect frame = Common::Rect(_originalWidth, _frameHeight);
	frame.translate(0, _topBorderHeight);

	float r = ((s.color >> 16) & 0xFF) / 255.0;
	float g = ((s.color >>  8) & 0xFF) / 255.0;
	float b = ((s.color >>  0) & 0xFF) / 255.0;
	float a = s.intensity * s.radius / 255.0;

	glDisable(GL_TEXTURE_2D);
	glColor4f(r, g, b, a);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_TRIANGLE_STRIP);
		glVertex3f( frame.left, frame.bottom, 0.0f);
		glVertex3f( frame.right, frame.bottom, 0.0f);
		glVertex3f( frame.left, frame.top, 0.0f);
		glVertex3f( frame.right, frame.top, 0.0f);
	glEnd();

	glDisable(GL_BLEND);
}

} // end of namespace Myst3
