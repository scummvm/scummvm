/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/gfx/openglssurface.h"

#include "engines/stark/gfx/opengls.h"
#include "engines/stark/gfx/texture.h"

#include "graphics/opengl/shader.h"

namespace Stark {
namespace Gfx {

OpenGLSSurfaceRenderer::OpenGLSSurfaceRenderer(OpenGLSDriver *gfx) :
		SurfaceRenderer(),
		_gfx(gfx) {
	_shader = _gfx->createSurfaceShaderInstance();
}

OpenGLSSurfaceRenderer::~OpenGLSSurfaceRenderer() {
	delete _shader;
}

void OpenGLSSurfaceRenderer::render(const Texture *texture, const Common::Point &dest) {
	render(texture, dest, texture->width(), texture->height());
}

void OpenGLSSurfaceRenderer::render(const Texture *texture, const Common::Point &dest, uint width, uint height) {
	// Destination rectangle with given width and height
	_gfx->start2DMode();

	_shader->use();
	_shader->setUniform1f("fadeLevel", _fadeLevel);
	_shader->setUniform("verOffsetXY", normalizeOriginalCoordinates(dest.x, dest.y));
	if (_noScalingOverride) {
		_shader->setUniform("verSizeWH", normalizeCurrentCoordinates(width, height));
	} else {
		_shader->setUniform("verSizeWH", normalizeOriginalCoordinates(width, height));
	}

	texture->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_shader->unbind();
	_gfx->end2DMode();
}

Math::Vector2d OpenGLSSurfaceRenderer::normalizeOriginalCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getUnscaledViewport();

	Math::Vector2d normalized(x / (float)viewport.width(), y / (float)viewport.height());

	// Align vertex coordinates to the native pixel grid
	// This ensures text does not get garbled by nearest neighbors scaling
	Common::Rect nativeViewport = _gfx->getViewport();
	normalized.setX(floorf((normalized.getX() * nativeViewport.width())) / nativeViewport.width());
	normalized.setY(floorf((normalized.getY() * nativeViewport.height())) / nativeViewport.height());

	return normalized;
}

Math::Vector2d OpenGLSSurfaceRenderer::normalizeCurrentCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getViewport();
	return Math::Vector2d(x / (float)viewport.width(), y / (float)viewport.height());
}

} // End of namespace Gfx
} // End of namespace Stark
