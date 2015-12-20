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

#if !USE_FORCED_GLES

#include "common/str.h"

namespace OpenGL {

enum {
	kPositionAttribLocation = 0,
	kTexCoordAttribLocation = 1,
	kColorAttribLocation    = 2
};

extern const char *const g_defaultVertexShader;
#if !USE_FORCED_GLES2
extern const char *const g_defaultFragmentShaderGL;
#endif
#if !USE_FORCED_GL
extern const char *const g_defaultFragmentShaderGLES2;
#endif

class Shader {
public:
	Shader(const Common::String &vertex, const Common::String &fragment)
	    : _vertex(vertex), _fragment(fragment) {}
	virtual ~Shader() {}

	/**
	 * Destroy the shader program.
	 *
	 * This keeps the vertex and fragment shader sources around and thus
	 * allows for recreating the shader on context recreation.
	 */
	virtual void destroy() = 0;

	/**
	 * Recreate shader program.
	 *
	 * @return true on success, false on failure.
	 */
	virtual bool recreate() = 0;

	/**
	 * Make shader active.
	 *
	 * @param projectionMatrix Projection matrix to use.
	 */
	virtual void activate(const GLfloat *projectionMatrix) = 0;
protected:
	/**
	 * Vertex shader sources.
	 */
	const Common::String _vertex;

	/**
	 * Fragment shader sources.
	 */
	const Common::String _fragment;
};

#if !USE_FORCED_GLES2
class ShaderARB : public Shader {
public:
	ShaderARB(const Common::String &vertex, const Common::String &fragment);
	virtual ~ShaderARB() { destroy(); }

	virtual void destroy();

	virtual bool recreate();

	virtual void activate(const GLfloat *projectionMatrix);
private:
	/**
	 * Shader program handle.
	 */
	GLhandleARB _program;

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
	 * @param shaderType Type of shader to compile (GL_FRAGMENT_SHADER_ARB or
	 *                   GL_VERTEX_SHADER_ARB)
	 * @return The shader object or 0 on failure.
	 */
	static GLhandleARB compileShader(const char *source, GLenum shaderType);
};
#endif // !USE_FORCED_GLES2

#if !USE_FORCED_GL
class ShaderGLES2 : public Shader {
public:
	ShaderGLES2(const Common::String &vertex, const Common::String &fragment);
	virtual ~ShaderGLES2() { destroy(); }

	virtual void destroy();

	virtual bool recreate();

	virtual void activate(const GLfloat *projectionMatrix);
private:
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
#endif

} // End of namespace OpenGL

#endif // !USE_FORCED_GLES

#endif
