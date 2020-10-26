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

#ifndef WINTERMUTE_BASE_SURFACE_OPENGL3D_H
#define WINTERMUTE_BASE_SURFACE_OPENGL3D_H

#include "engines/wintermute/base/gfx/base_surface.h"
#include "graphics/opengl/texture.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

namespace Wintermute {

class BaseGame;
class BaseRenderer3D;

class BaseSurfaceOpenGL3D : public BaseSurface {
public:
	BaseSurfaceOpenGL3D(BaseGame* game, BaseRenderer3D* renderer);
	~BaseSurfaceOpenGL3D();

	bool invalidate() override;

	bool displayHalfTrans(int x, int y, Rect32 rect) override;
	bool isTransparentAt(int x, int y) override;
	bool displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTrans(int x, int y, Rect32 rect, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTransOffset(int x, int y, Rect32 rect, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0) override;
	bool display(int x, int y, Rect32 rect, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTransform(int x, int y, Rect32 rect, Rect32 newRect, const Graphics::TransformStruct &transform) override;
	bool displayZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, bool transparent = false, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTiled(int x, int y, Rect32 rect, int numTimesX, int numTimesY) override;
	bool restore() override;
	bool create(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime = -1, bool keepLoaded = false) override;
	bool create(int width, int height) override;
	bool putSurface(const Graphics::Surface &surface, bool hasAlpha = false) override;
	bool putPixel(int x, int y, byte r, byte g, byte b, int a = -1) override;
	bool getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a = nullptr) override;
	bool comparePixel(int x, int y, byte r, byte g, byte b, int a = -1) override;
	bool startPixelOp() override;
	bool endPixelOp() override;
	bool isTransparentAtLite(int x, int y) override;

	void setTexture();

	GLuint getTextureName() {
		return _tex;
	}

	uint getGLTextureWidth() const {
		return _texWidth;
	}

	uint getGLTextureHeight() const {
		return _texHeight;
	}

private:
	GLuint _tex;
	BaseRenderer3D *_renderer;
	Graphics::Surface *_imageData;
	uint _texWidth;
	uint _texHeight;
};

} // End of namespace Wintermute

#endif // defined(USE_OPENGL) && !defined(USE_GLES2)

#endif
