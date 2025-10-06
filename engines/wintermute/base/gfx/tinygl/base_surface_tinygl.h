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

#ifndef WINTERMUTE_BASE_SURFACE_TINYGL_H
#define WINTERMUTE_BASE_SURFACE_TINYGL_H

#include "engines/wintermute/base/gfx/base_surface.h"

#if defined(USE_TINYGL)

#include "graphics/tinygl/tinygl.h"

namespace Wintermute {

class BaseGame;
class BaseRenderer3D;

class BaseSurfaceTinyGL : public BaseSurface {
public:
	BaseSurfaceTinyGL(BaseGame *game, BaseRenderer3D *renderer);
	~BaseSurfaceTinyGL();

	bool invalidate() override;
	bool prepareToDraw() override;

	bool displayTransRotate(int x, int y, float rotate, int32 hotspotX, int32 hotspotY, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTransZoom(int x, int y, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTrans(int x, int y, Common::Rect32 rect, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0) override;
	bool display(int x, int y, Common::Rect32 rect, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTiled(int x, int y, Common::Rect32 rect, int numTimesX, int numTimesY) override;
	bool create(const char *filename, bool texture2D, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime = -1, bool keepLoaded = false) override;
	bool create(int width, int height) override;
	bool setAlphaImage(const char *filename) override;
	bool putSurface(const Graphics::Surface &surface, bool hasAlpha = false) override;
	bool putPixel(int x, int y, byte r, byte g, byte b, byte a) override;
	bool getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a = nullptr) const override;
	bool startPixelOp() override;
	bool endPixelOp() override;
	bool isTransparentAtLite(int x, int y) const override;

	void setTexture();

	int getWidth() override {
		return _width;
	}

	int getHeight() override {
		return _height;
	}

	uint getGLTextureWidth() const {
		return _texWidth;
	}

	uint getGLTextureHeight() const {
		return _texHeight;
	}

	TinyGL::BlitImage *getBlitImage() {
		return _blitImage;
	}

private:
	TGLuint _tex;
	BaseRenderer3D *_renderer;
	Graphics::Surface *_imageData;
	Graphics::Surface *_maskData;
	uint _texWidth{};
	uint _texHeight{};
	bool _pixelOpReady;
	bool _surfaceModified;
	TinyGL::BlitImage *_blitImage;
	bool _texture2D;

	bool loadImage();
	void writeAlpha(Graphics::Surface *surface, const Graphics::Surface *mask);
};

} // End of namespace Wintermute

#endif // defined(USE_TINYGL)

#endif
