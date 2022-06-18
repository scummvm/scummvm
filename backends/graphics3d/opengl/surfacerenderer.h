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

#ifndef BACKENDS_GRAPHICS3D_OPENGL_SURFACE_RENDERER_H
#define BACKENDS_GRAPHICS3D_OPENGL_SURFACE_RENDERER_H

#include "math/rect2d.h"

#include "graphics/opengl/system_headers.h"

namespace OpenGL {

class Shader;
class TextureGL;

/**
 * A renderer to draw textures as two dimensional surfaces to the screen.
 */
class SurfaceRenderer {
public:
	SurfaceRenderer();
	virtual ~SurfaceRenderer();

	/**
	 * Push the current OpenGL state, and set up the adequate state for calling the render method.
	 */
	virtual void prepareState() = 0;

	/**
	 * Draw a 2D surface from the specified texture.
	 *
	 * The destination rectangle must be specified in the following coordinates system:
	 * x: left [0.0, 1.0] right
	 * y: top [0.0, 1.0] bottom
	 *
	 */
	virtual void render(const TextureGL *tex, const Math::Rect2d &dest) = 0;

	/**
	 * Pop the OpenGL state to restore it as it was before calling the prepareState method.
	 */
	virtual void restorePreviousState() = 0;

	/**
	 * Invert the surface along the Y coordinate
	 *
	 * Causes the image to be drawn upside down
	 */
	void setFlipY(bool flipY);

	/**
	 * Set the alpha blending with already drawn content
	 */
	void enableAlphaBlending(bool enable);

protected:
	bool _flipY;
	bool _alphaBlending;
};

#if defined(USE_OPENGL_GAME)

class FixedSurfaceRenderer : public SurfaceRenderer {
public:
	virtual ~FixedSurfaceRenderer();

	// SurfaceRenderer API
	void prepareState() override;
	void render(const TextureGL *tex, const Math::Rect2d &dest) override;
	void restorePreviousState() override;
};

#endif

#if defined(USE_OPENGL_SHADERS)

class ShaderSurfaceRenderer : public SurfaceRenderer {
public:
	ShaderSurfaceRenderer();
	virtual ~ShaderSurfaceRenderer();

	// SurfaceRenderer API
	void prepareState() override;
	void render(const TextureGL *tex, const Math::Rect2d &dest) override;
	void restorePreviousState() override;

private:
	Shader *_boxShader;
	GLuint _boxVerticesVBO;
	GLboolean _prevStateDepthTest;
	GLboolean _prevStateDepthWriteMask;
	GLboolean _prevStateBlend;
	GLint _prevStateBlendFunc;
	GLint _prevStateViewport[4];
};

#endif

SurfaceRenderer *createBestSurfaceRenderer();

} // End of namespace OpenGL

#endif // GRAPHICS_OPENGL_SURFACE_RENDERER_H
