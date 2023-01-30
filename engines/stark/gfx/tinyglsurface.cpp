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

#include "engines/stark/gfx/tinyglsurface.h"
#include "engines/stark/gfx/tinyglbitmap.h"
#include "engines/stark/gfx/color.h"

#include "graphics/tinygl/tinygl.h"

namespace Stark {
namespace Gfx {

TinyGLSurfaceRenderer::TinyGLSurfaceRenderer(TinyGLDriver *gfx) :
		SurfaceRenderer(),
		_gfx(gfx) {
}

TinyGLSurfaceRenderer::~TinyGLSurfaceRenderer() {
}

void TinyGLSurfaceRenderer::render(const Bitmap *bitmap, const Common::Point &dest) {
	render(bitmap, dest, bitmap->width(), bitmap->height());
}

void TinyGLSurfaceRenderer::render(const Bitmap *bitmap, const Common::Point &dest, uint width, uint height) {
	if (width == 0 || height == 0)
		return;
	_gfx->start2DMode();

	Math::Vector2d sizeWH;
	if (_noScalingOverride) {
		sizeWH = normalizeCurrentCoordinates(width, height);
	} else {
		sizeWH = normalizeOriginalCoordinates(width, height);
	}
	auto verOffsetXY = normalizeOriginalCoordinates(dest.x, dest.y);
	auto nativeViewport = _gfx->getViewport();
	auto viewport = Math::Vector2d(nativeViewport.width(), nativeViewport.height());
	auto blitImage = ((TinyGlBitmap *)const_cast<Bitmap *>(bitmap))->getBlitImage();
	int blitImageWidth, blitImageHeight;
	tglGetBlitImageSize(blitImage, blitImageWidth, blitImageHeight);
	int posX = viewport.getX() * verOffsetXY.getX() + nativeViewport.left;
	int posY = viewport.getY() * verOffsetXY.getY() + nativeViewport.top;
	TinyGL::BlitTransform transform(posX, posY);

	// W/A for not clipped bitmaps in prompt dialog
	if (width == 256 && height == 256) {
		blitImageHeight = viewport.getY() - dest.y;
		blitImageWidth = viewport.getX() - dest.x;
	}

	transform.sourceRectangle(0, 0, blitImageWidth, blitImageHeight);
	transform.tint(1.0, 1.0 - _fadeLevel, 1.0 - _fadeLevel, 1.0 - _fadeLevel);
	tglBlit(blitImage, transform);

	_gfx->end2DMode();
}

void TinyGLSurfaceRenderer::fill(const Color &color, const Common::Point &dest, uint width, uint height) {
	_gfx->start2DMode();

	SurfaceVertex vertices[4] = {};
	convertToVertices(vertices, dest, width, height);

	tglMatrixMode(TGL_PROJECTION);
	tglPushMatrix();
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	tglLoadIdentity();

	tglDisable(TGL_TEXTURE_2D);

	tglEnableClientState(TGL_VERTEX_ARRAY);

	tglVertexPointer(2, TGL_FLOAT, sizeof(SurfaceVertex), &vertices[0].x);
	tglColor4f((color.r / 255.0f) - _fadeLevel, (color.g / 255.0f) - _fadeLevel, (color.b / 255.0f) - _fadeLevel, color.a / 255.0f);

	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();

	tglMatrixMode(TGL_PROJECTION);
	tglPopMatrix();

	_gfx->end2DMode();
}

void TinyGLSurfaceRenderer::convertToVertices(SurfaceVertex *vertices, const Common::Point &dest, uint width, uint height) const {
	const Math::Vector2d surfaceVertices[] = {
		// X   Y
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
	};

	Math::Vector2d verSizeWH;
	if (_noScalingOverride) {
		verSizeWH = normalizeCurrentCoordinates(width, height);
	} else {
		verSizeWH = normalizeOriginalCoordinates(width, height);
	}
	auto verOffsetXY = normalizeOriginalCoordinates(dest.x, dest.y);
	auto nativeViewport = _gfx->getViewport();
	auto viewport = Math::Vector2d(nativeViewport.width(), nativeViewport.height());

	for (int32 v = 0; v < 4; v++) {
		Math::Vector2d pos = verOffsetXY + (surfaceVertices[v] * verSizeWH);

		if (_snapToGrid) {
			// Align vertex coordinates to the native pixel grid
			// This ensures text does not get garbled by nearest neighbors scaling
			pos.setX(floor(pos.getX() * viewport.getX() + 0.5) / viewport.getX());
			pos.setY(floor(pos.getY() * viewport.getY() + 0.5) / viewport.getY());
		}

		// position coords
		vertices[v].x = pos.getX() * 2.0 - 1.0;
		vertices[v].y = -1.0 * (pos.getY() * 2.0 - 1.0);
	}
}

Math::Vector2d TinyGLSurfaceRenderer::normalizeOriginalCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getUnscaledViewport();
	return Math::Vector2d(x / (float)viewport.width(), y / (float)viewport.height());
}

Math::Vector2d TinyGLSurfaceRenderer::normalizeCurrentCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getViewport();
	return Math::Vector2d(x / (float)viewport.width(), y / (float)viewport.height());
}

} // End of namespace Gfx
} // End of namespace Stark
