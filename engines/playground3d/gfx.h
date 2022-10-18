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

#ifndef PLAYGROUND3D_GFX_H
#define PLAYGROUND3D_GFX_H

#include "common/rect.h"
#include "common/system.h"

#include "math/frustum.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Playground3d {

class Renderer {
public:
	Renderer(OSystem *system);
	virtual ~Renderer();

	virtual void init() = 0;
	virtual void deinit() = 0;
	virtual void clear(const Math::Vector4d &clearColor) = 0;

	/**
	 *  Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer() { }

	Common::Rect viewport() const;

	void setupCameraPerspective(float pitch, float heading, float fov);

	static const int kOriginalWidth = 640;
	static const int kOriginalHeight = 480;

	void computeScreenViewport();

	virtual void setupViewport(int x, int y, int width, int height) = 0;
	virtual void loadTextureRGBA(Graphics::Surface *texture) = 0;
	virtual void loadTextureRGB(Graphics::Surface *texture) = 0;
	virtual void loadTextureRGB565(Graphics::Surface *texture) = 0;
	virtual void loadTextureRGBA5551(Graphics::Surface *texture) = 0;
	virtual void loadTextureRGBA4444(Graphics::Surface *texture) = 0;
	virtual void drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) = 0;
	virtual void drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) = 0;
	virtual void dimRegionInOut(float fade) = 0;
	virtual void drawInViewport() = 0;
	virtual void drawRgbaTexture() = 0;

	virtual void enableFog(const Math::Vector4d &fogColor) = 0;

protected:
	OSystem *_system;

	Common::Rect _screenViewport;

	Math::Matrix4 _projectionMatrix;
	Math::Matrix4 _modelViewMatrix;
	Math::Matrix4 _mvpMatrix;

	static const float cubeVertices[11 * 6 * 4];
	Graphics::Surface *_texture;

	Math::Matrix4 makeProjectionMatrix(float fov, float nearClip, float farClip) const;
};

Renderer *CreateGfxOpenGL(OSystem *system);
Renderer *CreateGfxOpenGLShader(OSystem *system);
Renderer *CreateGfxTinyGL(OSystem *system);
Renderer *createRenderer(OSystem *system);

} // End of namespace Playground3d

#endif // PLAYGROUND3D_GFX_H
