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

#include "engines/myst3/scene.h"

namespace Myst3 {

void Scene::init(int width, int height) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)width/(GLfloat)height, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	
	_cameraPitch = 0.0f;
	_cameraYaw = 0.0f;
}

void Scene::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glColor3f(1.0f, 1.0f, 1.0f);
}

void Scene::setupCamera() {
	// Rotate the model to simulate the rotation of the camera
	glLoadIdentity();
	glRotatef(_cameraPitch, 1.0f, 0.0f, 0.0f);
	glRotatef(_cameraYaw, 0.0f, 1.0f, 0.0f);
}

void Scene::updateCamera(Common::Point &mouse) {
	_cameraYaw += mouse.x / 3.0f;
	_cameraPitch += mouse.y / 3.0f;

	_cameraPitch = CLIP(_cameraPitch, -90.0f, 90.0f);
}

} // end of namespace Myst3
