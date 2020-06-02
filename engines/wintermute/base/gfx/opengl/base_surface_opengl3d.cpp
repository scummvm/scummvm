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

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "graphics/transparent_surface.h"

namespace Wintermute {

BaseSurfaceOpenGL3D::BaseSurfaceOpenGL3D(BaseGame *game, BaseRenderOpenGL3D *renderer)
    : BaseSurface(game), tex(nullptr), renderer(renderer), pixelOpReady(false) {
}

bool BaseSurfaceOpenGL3D::invalidate() {
	warning("BaseSurfaceOpenGL3D::invalidate not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::displayHalfTrans(int x, int y, Rect32 rect) {
	warning("BaseSurfaceOpenGL3D::displayHalfTrans not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::isTransparentAt(int x, int y) {
	warning("BaseSurfaceOpenGL3D::isTransparentAt not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	warning("BaseSurfaceOpenGL3D::displayTransZoom not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::displayTrans(int x, int y, Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	renderer->drawSprite(*tex, rect, 100, 100, Vector2(x, y), alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceOpenGL3D::displayTransOffset(int x, int y, Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	warning("BaseSurfaceOpenGL3D::displayTransOffset not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::display(int x, int y, Rect32 rect, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	renderer->drawSprite(*tex, rect, 100, 100, Vector2(x, y), 0xFFFFFFFF, true, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceOpenGL3D::displayTransform(int x, int y, Rect32 rect, Rect32 newRect, const Graphics::TransformStruct &transform) {
	warning("BaseSurfaceOpenGL3D::displayTransform not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::displayZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, bool transparent, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	warning("BaseSurfaceOpenGL3D::displayZoom not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::displayTiled(int x, int y, Rect32 rect, int numTimesX, int numTimesY) {
	Vector2 scale(numTimesX, numTimesY);
	renderer->drawSpriteEx(*tex, rect, Vector2(x, y), Vector2(0, 0), scale, 0, 0xFFFFFFFF, true, Graphics::BLEND_NORMAL, false, false);
	return true;
}

bool BaseSurfaceOpenGL3D::restore() {
	warning("BaseSurfaceOpenGL3D::restore not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::create(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	BaseImage img = BaseImage();
	if (!img.loadFile(filename)) {
		return false;
	}

	if (img.getSurface()->format.bytesPerPixel == 1 && img.getPalette() == nullptr) {
		return false;
	}

	_filename = filename;

	if (defaultCK) {
		ckRed = 255;
		ckGreen = 0;
		ckBlue = 255;
	}

	_ckDefault = defaultCK;
	_ckRed = ckRed;
	_ckGreen = ckGreen;
	_ckBlue = ckBlue;

	bool needsColorKey = false;
	bool replaceAlpha = true;

	Graphics::Surface *surf = img.getSurface()->convertTo(OpenGL::Texture::getRGBAPixelFormat(), img.getPalette());

	if (_filename.hasSuffix(".bmp") && img.getSurface()->format.bytesPerPixel == 4) {
		// 32 bpp BMPs have nothing useful in their alpha-channel -> color-key
		needsColorKey = true;
		replaceAlpha = false;
	} else if (img.getSurface()->format.aBits() == 0) {
		needsColorKey = true;
	}

	if (needsColorKey) {
		Graphics::TransparentSurface trans(*surf);
		trans.applyColorKey(_ckRed, _ckGreen, _ckBlue, replaceAlpha);
	}

	tex = new OpenGL::Texture(*surf);
	delete surf;

	if (_lifeTime == 0 || lifeTime == -1 || lifeTime > _lifeTime) {
		_lifeTime = lifeTime;
	}

	_keepLoaded = keepLoaded;
	if (_keepLoaded) {
		_lifeTime = -1;
	}

	_valid = true;

	return true;
}

bool BaseSurfaceOpenGL3D::create(int width, int height) {
	tex = new OpenGL::Texture(width, height);
	_valid = true;
	return true;
}

bool BaseSurfaceOpenGL3D::putPixel(int x, int y, byte r, byte g, byte b, int a) {
	warning("BaseSurfaceOpenGL3D::putPixel not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a) {
	warning("BaseSurfaceOpenGL3D::getPixel not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::comparePixel(int x, int y, byte r, byte g, byte b, int a) {
	warning("BaseSurfaceOpenGL3D::comparePixel not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::startPixelOp() {
	glBindTexture(GL_TEXTURE_2D, tex->getTextureName());
	return true;
}

bool BaseSurfaceOpenGL3D::endPixelOp() {
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool BaseSurfaceOpenGL3D::isTransparentAtLite(int x, int y) {
	if (x < 0 || y < 0 || x >= tex->getWidth() || y >= tex->getHeight()) {
		return false;
	}

	if (!pixelOpReady) {
		return false;
	}

	uint8 *image_data = nullptr;

	// assume 32 bit rgba for now
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

	uint32 pixel = *reinterpret_cast<uint32 *>(image_data + y * tex->getWidth() * 4 + x * 4);
	pixel &= 0x000000FF;
	return pixel == 0;
}

}
