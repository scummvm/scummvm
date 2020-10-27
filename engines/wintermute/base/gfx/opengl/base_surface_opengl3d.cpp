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

#include "common/algorithm.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "graphics/transparent_surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"

namespace Wintermute {

void applyColorKey(Graphics::Surface &surf, byte ckRed, byte ckGreen, byte ckBlue, bool replaceAlpha) {
	// this is taken from Graphics::TransparentSurface
	// only difference is that we set the pixel
	// color to transparent black, like D3DX,
	// if it matches the color key
	for (int y = 0; y < surf.h; y++) {
		for (int x = 0; x < surf.w; x++) {
			uint32 pix = ((uint32 *)surf.getPixels())[y * surf.w + x];
			uint8 r, g, b, a;
			surf.format.colorToARGB(pix, a, r, g, b);
			if (r == ckRed && g == ckGreen && b == ckBlue) {
				a = 0;
				r = 0;
				g = 0;
				b = 0;
				((uint32 *)surf.getPixels())[y * surf.w + x] = surf.format.ARGBToColor(a, r, g, b);
			} else if (replaceAlpha) {
				a = 255;
				((uint32 *)surf.getPixels())[y * surf.w + x] = surf.format.ARGBToColor(a, r, g, b);
			}
		}
	}
}

BaseSurfaceOpenGL3D::BaseSurfaceOpenGL3D(BaseGame *game, BaseRenderer3D *renderer)
	: BaseSurface(game), _tex(0), _renderer(renderer), _imageData(nullptr), _texWidth(0), _texHeight(0) {
}

BaseSurfaceOpenGL3D::~BaseSurfaceOpenGL3D() {
	glDeleteTextures(1, &_tex);
	delete[] _imageData;
}

bool BaseSurfaceOpenGL3D::invalidate() {
	glDeleteTextures(1, &_tex);
	_imageData->free();
	delete[] _imageData;
	_imageData = nullptr;

	_valid = false;
	return true;
}

bool BaseSurfaceOpenGL3D::displayHalfTrans(int x, int y, Rect32 rect) {
	warning("BaseSurfaceOpenGL3D::displayHalfTrans not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::isTransparentAt(int x, int y) {
	prepareToDraw();

	uint8 alpha = reinterpret_cast<uint8 *>(_imageData->getPixels())[y * _width * 4 + x * 4 + 3];
	return alpha < 128;
}

bool BaseSurfaceOpenGL3D::displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	prepareToDraw();

	_renderer->drawSprite(*this, rect, zoomX, zoomY, Vector2(x, y), alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceOpenGL3D::displayTrans(int x, int y, Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	prepareToDraw();

	_renderer->drawSprite(*this, rect, 100, 100, Vector2(x, y), alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceOpenGL3D::displayTransOffset(int x, int y, Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	prepareToDraw();

	_renderer->drawSprite(*this, rect, 100, 100, Vector2(x + offsetX, y + offsetY), alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceOpenGL3D::display(int x, int y, Rect32 rect, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	prepareToDraw();

	_renderer->drawSprite(*this, rect, 100, 100, Vector2(x, y), 0xFFFFFFFF, true, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceOpenGL3D::displayTransform(int x, int y, Rect32 rect, Rect32 newRect, const Graphics::TransformStruct &transform) {
	prepareToDraw();

	Vector2 position(x, y);
	Vector2 rotation;
	rotation.x = x + transform._hotspot.x * (transform._zoom.x / 100.0f);
	rotation.y = y + transform._hotspot.y * (transform._zoom.y / 100.0f);
	Vector2 scale(transform._zoom.x / 100.0f, transform._zoom.y / 100.0f);

	_renderer->drawSpriteEx(*this, rect, position, rotation, scale, transform._angle, transform._rgbaMod, transform._alphaDisable, transform._blendMode, transform.getMirrorX(), transform.getMirrorY());
	return true;
}

bool BaseSurfaceOpenGL3D::displayZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, bool transparent, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	warning("BaseSurfaceOpenGL3D::displayZoom not yet implemented");
	return true;
}

bool BaseSurfaceOpenGL3D::displayTiled(int x, int y, Rect32 rect, int numTimesX, int numTimesY) {
	prepareToDraw();

	Vector2 scale(numTimesX, numTimesY);
	_renderer->drawSpriteEx(*this, rect, Vector2(x, y), Vector2(0, 0), scale, 0, 0xFFFFFFFF, true, Graphics::BLEND_NORMAL, false, false);
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

	if (_imageData) {
		_imageData->free();
		delete _imageData;
	}

	_imageData = img.getSurface()->convertTo(OpenGL::TextureGL::getRGBAPixelFormat(), img.getPalette());

	if (_filename.hasSuffix(".bmp") && img.getSurface()->format.bytesPerPixel == 4) {
		// 32 bpp BMPs have nothing useful in their alpha-channel -> color-key
		needsColorKey = true;
		replaceAlpha = false;
	} else if (img.getSurface()->format.aBits() == 0) {
		needsColorKey = true;
	}

	if (needsColorKey) {
		applyColorKey(*_imageData, ckRed, ckGreen, ckBlue, replaceAlpha);
	}

	putSurface(*_imageData);

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
	_width = width;
	_height = height;
	_texWidth = Common::nextHigher2(width);
	_texHeight = Common::nextHigher2(height);

	glGenTextures(1, &_tex);
	glBindTexture(GL_TEXTURE_2D, _tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	_valid = true;
	return true;
}

bool BaseSurfaceOpenGL3D::putSurface(const Graphics::Surface &surface, bool hasAlpha) {
	if (!_imageData) {
		_imageData = new Graphics::Surface();
	}

	if (_imageData && _imageData != &surface) {
		_imageData->copyFrom(surface);
	}

	_width = surface.w;
	_height = surface.h;
	_texWidth = Common::nextHigher2(_width);
	_texHeight = Common::nextHigher2(_height);

	if (_valid) {
		invalidate();
	}

	glGenTextures(1, &_tex);
	glBindTexture(GL_TEXTURE_2D, _tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, const_cast<void *>(surface.getPixels()));
	glBindTexture(GL_TEXTURE_2D, 0);
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
	prepareToDraw();
	return true;
}

bool BaseSurfaceOpenGL3D::endPixelOp() {
	return true;
}

bool BaseSurfaceOpenGL3D::isTransparentAtLite(int x, int y) {
	if (x < 0 || y < 0 || x >= _width || y >= _height) {
		return false;
	}

	if (_imageData == nullptr) {
		return false;
	}

	//TODO: Check for endianness issues
	uint8 alpha = reinterpret_cast<uint8 *>(_imageData->getPixels())[y * _width * 4 + x * 4 + 3];
	return alpha == 0;
}

void BaseSurfaceOpenGL3D::setTexture() {
	prepareToDraw();

	glBindTexture(GL_TEXTURE_2D, _tex);
}

} // End of namespace Wintermute

#endif // defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
