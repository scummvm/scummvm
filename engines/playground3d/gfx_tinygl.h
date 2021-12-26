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

#ifndef PLAYGROUND3D_GFX_TINYGL_H
#define PLAYGROUND3D_GFX_TINYGL_H

#include "common/rect.h"
#include "common/system.h"

#include "math/vector3d.h"

#include "graphics/tinygl/tinygl.h"

#include "engines/playground3d/gfx.h"

namespace Playground3d {

class TinyGLRenderer : public Renderer {
public:
	TinyGLRenderer(OSystem *_system);
	virtual ~TinyGLRenderer();

	void init() override;
	void deinit() override;

	void clear(const Math::Vector4d &clearColor) override;
	void loadTextureRGB(Graphics::Surface *texture) override;
	void loadTextureRGBA(Graphics::Surface *texture) override;
	void loadTextureRGB565(Graphics::Surface *texture) override;
	void loadTextureRGBA5551(Graphics::Surface *texture) override;
	void loadTextureRGBA4444(Graphics::Surface *texture) override;

	void setupViewport(int x, int y, int width, int height) override;
	void drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) override;
	void drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) override;
	void dimRegionInOut(float fade) override;
	void drawInViewport() override;
	void drawRgbaTexture() override;

	void flipBuffer() override;

private:
	Math::Vector3d _pos;
	TGLuint _textureRgbaId[5];
	TGLuint _textureRgbId[5];
	TGLuint _textureRgb565Id[2];
	TGLuint _textureRgba5551Id[2];
	TGLuint _textureRgba4444Id[2];
	TinyGL::BlitImage *_blitImageRgba;
	TinyGL::BlitImage *_blitImageRgb;
	TinyGL::BlitImage *_blitImageRgb565;
	TinyGL::BlitImage *_blitImageRgba5551;
	TinyGL::BlitImage *_blitImageRgba4444;

	void drawFace(uint face);
};

} // End of namespace Playground3d

#endif // PLAYGROUND3D_GFX_TINYGL_H
