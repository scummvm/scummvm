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

bool BaseRenderer3D::setAmbientLightColor(uint32 color) {
	_ambientLightColor = color;
	_overrideAmbientLightColor = true;
	setAmbientLight();
	return true;
}

bool BaseRenderer3D::setDefaultAmbientLightColor() {
	_ambientLightColor = 0x00000000;
	_overrideAmbientLightColor = false;
	setAmbientLight();
	return true;
}

void BaseRenderer3D::project(const Math::Matrix4 &worldMatrix, const Math::Vector3d &point, int &x, int &y) {
	Math::Matrix4 tmp = worldMatrix;
	tmp.transpose();
	Math::Vector3d windowCoords;
	Math::Matrix4 modelMatrix = tmp * _lastViewMatrix;
	int viewport[4] = { _viewportRect.left, _viewportRect.top, _viewportRect.width(), _viewportRect.height()};
	Math::gluMathProject(point, modelMatrix.getData(), _projectionMatrix3d.getData(), viewport, windowCoords);
	x = windowCoords.x();
	// The Wintermute script code will expect a Direct3D viewport
	y = viewport[3] - windowCoords.y();
}

Rect32 BaseRenderer3D::getViewPort() {
	return _viewportRect;
}

Graphics::PixelFormat BaseRenderer3D::getPixelFormat() const {
	return OpenGL::Texture::getRGBAPixelFormat();
}

void BaseRenderer3D::fade(uint16 alpha) {
	fadeToColor(0, 0, 0, (byte)(255 - alpha));
}

void BaseRenderer3D::initLoop() {
	deleteRectList();
	setup2D();
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

bool BaseRenderer3D::drawSprite(BaseSurfaceOpenGL3D &tex, const Wintermute::Rect32 &rect,
								float zoomX, float zoomY, const Wintermute::Vector2 &pos,
								uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
								bool mirrorX, bool mirrorY) {
	Vector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	return drawSpriteEx(tex, rect, pos, Vector2(0.0f, 0.0f), scale, 0.0f, color, alphaDisable, blendMode, mirrorX, mirrorY);
}

} // namespace Wintermute
