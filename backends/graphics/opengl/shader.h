/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_OPENGL_SHADER_H
#define BACKENDS_GRAPHICS_OPENGL_SHADER_H

#include "backends/graphics/opengl/opengl-sys.h"

#if !USE_FORCED_GL && !USE_FORCED_GLES

#include "common/str.h"

namespace OpenGL {

enum {
	kPositionAttribLocation = 0,
	kTexCoordAttribLocation = 1,
	kColorAttribLocation    = 2
};

extern const char *const g_defaultVertexShader;
extern const char *const g_defaultFragmentShader;

class Shader {
public:
	Shader(const Common::String &vertex, const Common::String &fragment);
	~Shader() { destroy(); }

	/**
	 * Destroy the shader program.
	 *
	 * This keeps the vertex and fragment shader sources around and thus
	 * allows for recreating the shader on context recreation.
	 */
	void destroy();

	/**
	 * Recreate shader program.
	 *
	 * @return true on success, false on failure.
	 */
	bool recreate();

	/**
	 * Make shader active.
	 *
	 * @param projectionMatrix Projection matrix to use.
	 */
	void activate(const GLfloat *projectionMatrix);
private:
	/**
	 * Vertex shader sources.
	 */
	const Common::String _vertex;

	/**
	 * Fragment shader sources.
	 */
	const Common::String _fragment;

	/**
	 * Shader program handle.
	 */
	GLuint _program;

	/**
	 * Location of the matrix uniform in the shader program.
	 */
	GLint _projectionLocation;

	/**
	 * Location of the texture sampler location in the shader program.
	 */
	GLint _textureLocation;

	/**
	 * Compile a vertex or fragment shader.
	 *
	 * @param source     Sources to the shader.
	 * @param shaderType Type of shader to compile (GL_FRAGMENT_SHADER or
	 *                   GL_VERTEX_SHADER)
	 * @return The shader object or 0 on failure.
	 */
	static GLuint compileShader(const char *source, GLenum shaderType);
};

} // End of namespace OpenGL

#endif // !USE_FORCED_GL && !USE_FORCED_GLES

#endif
