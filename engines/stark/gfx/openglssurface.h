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

#ifndef STARK_GFX_OPENGL_S_SURFACE_H
#define STARK_GFX_OPENGL_S_SURFACE_H

#include "engines/stark/gfx/surfacerenderer.h"

#include "math/vector2d.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

namespace OpenGL {
class ShaderGL;
}

namespace Stark {
namespace Gfx {

class OpenGLSDriver;
class Texture;

/**
 * An programmable pipeline OpenGL surface renderer
 */
class OpenGLSSurfaceRenderer : public SurfaceRenderer {
public:
	OpenGLSSurfaceRenderer(OpenGLSDriver *gfx);
	virtual ~OpenGLSSurfaceRenderer();

	// SurfaceRenderer API
	void render(const Texture *texture, const Common::Point &dest) override;
	void render(const Texture *texture, const Common::Point &dest, uint width, uint height) override;

private:
	Math::Vector2d normalizeOriginalCoordinates(int x, int y) const;
	Math::Vector2d normalizeCurrentCoordinates(int x, int y) const;

	OpenGLSDriver *_gfx;
	OpenGL::ShaderGL *_shader;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#endif // STARK_GFX_OPENGL_S_SURFACE_H
