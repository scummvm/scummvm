/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/stark/gfx/openglssurface.h"

#include "engines/stark/gfx/opengls.h"
#include "engines/stark/gfx/bitmap.h"
#include "engines/stark/gfx/color.h"

#if defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/shader.h"

namespace Stark {
namespace Gfx {

OpenGLSSurfaceRenderer::OpenGLSSurfaceRenderer(OpenGLSDriver *gfx) :
		SurfaceRenderer(),
		_gfx(gfx) {
	_shader = _gfx->createSurfaceShaderInstance();
	_shaderFill = _gfx->createSurfaceFillShaderInstance();
}

OpenGLSSurfaceRenderer::~OpenGLSSurfaceRenderer() {
	delete _shaderFill;
	delete _shader;
}

void OpenGLSSurfaceRenderer::render(const Bitmap *bitmap, const Common::Point &dest) {
	render(bitmap, dest, bitmap->width(), bitmap->height());
}

void OpenGLSSurfaceRenderer::render(const Bitmap *bitmap, const Common::Point &dest, uint width, uint height) {
	// Destination rectangle with given width and height
	_gfx->start2DMode();

	_shader->use();
	_shader->setUniform1f("fadeLevel", _fadeLevel);
	_shader->setUniform1f("snapToGrid", _snapToGrid);
	_shader->setUniform("verOffsetXY", normalizeOriginalCoordinates(dest.x, dest.y));
	if (_noScalingOverride) {
		_shader->setUniform("verSizeWH", normalizeCurrentCoordinates(width, height));
	} else {
		_shader->setUniform("verSizeWH", normalizeOriginalCoordinates(width, height));
	}

	Common::Rect nativeViewport = _gfx->getViewport();
	_shader->setUniform("viewport", Math::Vector2d(nativeViewport.width(), nativeViewport.height()));

	bitmap->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_shader->unbind();
	_gfx->end2DMode();
}

void OpenGLSSurfaceRenderer::fill(const Color &color, const Common::Point &dest, uint width, uint height) {
	// Destination rectangle with given width and height
	_gfx->start2DMode();

	_shaderFill->use();
	_shaderFill->setUniform1f("fadeLevel", _fadeLevel);
	_shaderFill->setUniform1f("snapToGrid", _snapToGrid);
	_shaderFill->setUniform("verOffsetXY", normalizeOriginalCoordinates(dest.x, dest.y));
	if (_noScalingOverride) {
		_shaderFill->setUniform("verSizeWH", normalizeCurrentCoordinates(width, height));
	} else {
		_shaderFill->setUniform("verSizeWH", normalizeOriginalCoordinates(width, height));
	}

	Common::Rect nativeViewport = _gfx->getViewport();
	_shaderFill->setUniform("viewport", Math::Vector2d(nativeViewport.width(), nativeViewport.height()));

	_shaderFill->setUniform("color", Math::Vector4d(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_shaderFill->unbind();
	_gfx->end2DMode();
}

Math::Vector2d OpenGLSSurfaceRenderer::normalizeOriginalCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getUnscaledViewport();
	return Math::Vector2d(x / (float)viewport.width(), y / (float)viewport.height());
}

Math::Vector2d OpenGLSSurfaceRenderer::normalizeCurrentCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getViewport();
	return Math::Vector2d(x / (float)viewport.width(), y / (float)viewport.height());
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // if defined(USE_OPENGL_SHADERS)
