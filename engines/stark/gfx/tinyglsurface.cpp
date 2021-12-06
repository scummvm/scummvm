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

#include "engines/stark/gfx/tinyglsurface.h"
#include "engines/stark/gfx/tinyglbitmap.h"
#include "engines/stark/gfx/texture.h"

#include "graphics/tinygl/zblit.h"

namespace Stark {
namespace Gfx {

TinyGLSurfaceRenderer::TinyGLSurfaceRenderer(TinyGLDriver *gfx) :
		SurfaceRenderer(),
		_gfx(gfx) {
}

TinyGLSurfaceRenderer::~TinyGLSurfaceRenderer() {
}

void TinyGLSurfaceRenderer::render(const Texture *texture, const Common::Point &dest) {
	render(texture, dest, texture->width(), texture->height());
}

void TinyGLSurfaceRenderer::render(const Texture *texture, const Common::Point &dest, uint width, uint height) {
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
	auto blitImage = ((TinyGlBitmap *)const_cast<Texture *>(texture))->getBlitTexture();
	int blitTextureWidth, blitTextureHeight;
	tglGetBlitImageSize(blitImage, blitTextureWidth, blitTextureHeight);
	int posX = viewport.getX() * verOffsetXY.getX() + nativeViewport.left;
	int posY = viewport.getY() * verOffsetXY.getY() + nativeViewport.top;
	TinyGL::BlitTransform transform(posX, posY);

	// WA for not clipped textues in prompt dialog
	if (width == 256 && height == 256) {
		blitTextureHeight = viewport.getY() - dest.y;
		blitTextureWidth = viewport.getX() - dest.x;
	}

	transform.sourceRectangle(0, 0, blitTextureWidth, blitTextureHeight);
	transform.tint(1.0, 1.0 - _fadeLevel, 1.0 - _fadeLevel, 1.0 - _fadeLevel);
	tglBlit(blitImage, transform);

	_gfx->end2DMode();
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
