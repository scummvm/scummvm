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

#ifndef GRAPHICS_OPENGL_SURFACE_RENDERER_H
#define GRAPHICS_OPENGL_SURFACE_RENDERER_H

#include "math/rect2d.h"

#include "graphics/opengl/system_headers.h"

namespace OpenGL {

class Shader;
class Texture;

/**
 * A renderer to draw textures as two dimensional surfaces to the screen.
 */
class SurfaceRenderer {
public:
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
	virtual void render(const Texture *tex, const Math::Rect2d &dest, bool flipY = false) = 0;

	/**
	 * Pop the OpenGL state to restore it as it was before calling the prepareState method.
	 */
	virtual void restorePreviousState() = 0;
};

#ifndef USE_GLES2

class FixedSurfaceRenderer : public SurfaceRenderer {
public:
	virtual ~FixedSurfaceRenderer();

	// SurfaceRenderer API
	void prepareState() override;
	void render(const Texture *tex, const Math::Rect2d &dest, bool flipY) override;
	void restorePreviousState() override;
};

#endif

#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

class ShaderSurfaceRenderer : public SurfaceRenderer {
public:
	ShaderSurfaceRenderer();
	virtual ~ShaderSurfaceRenderer();

	// SurfaceRenderer API
	void prepareState() override;
	void render(const Texture *tex, const Math::Rect2d &dest, bool flipY) override;
	void restorePreviousState() override;

private:
	Shader *_boxShader;
	GLuint _boxVerticesVBO;
};

#endif

SurfaceRenderer *createBestSurfaceRenderer();

} // End of namespace OpenGL

#endif // GRAPHICS_OPENGL_SURFACE_RENDERER_H
