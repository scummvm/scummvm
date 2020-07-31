/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/wintermute/base/gfx/opengl/base_renderer3d.h"
#include "math/glmath.h"

namespace Wintermute {

BaseRenderer3D::BaseRenderer3D(Wintermute::BaseGame *inGame) : BaseRenderer(inGame), _overrideAmbientLightColor(false) {
}

BaseRenderer3D::~BaseRenderer3D() {
}

void BaseRenderer3D::project(const Math::Matrix4 &worldMatrix, const Math::Vector3d &point, int &x, int &y) {
	Math::Matrix4 tmp = worldMatrix;
	tmp.transpose();
	Math::Vector3d windowCoords;
	Math::Matrix4 modelMatrix = tmp * _lastViewMatrix;
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	Math::gluMathProject(point, modelMatrix.getData(), _projectionMatrix3d.getData(), viewport, windowCoords);
	x = windowCoords.x();
	// The Wintermute script code will expect a Direct3D viewport
	y = viewport[3] - windowCoords.y();
}

Math::Ray BaseRenderer3D::rayIntoScene(int x, int y) {
	Math::Vector3d direction((((2.0f * x) / _viewportRect.width()) - 1) / _projectionMatrix3d(0, 0),
							 -(((2.0f * y) / _viewportRect.height()) - 1) / _projectionMatrix3d(1, 1),
							 -1.0f);

	Math::Matrix4 m = _lastViewMatrix;
	m.inverse();
	m.transpose();
	m.transform(&direction, false);

	Math::Vector3d origin = m.getPosition();
	return Math::Ray(origin, direction);
}

} // namespace Wintermute
