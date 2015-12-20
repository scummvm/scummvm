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

#include "backends/graphics/opengl/shader.h"

#if !USE_FORCED_GLES

#include "common/textconsole.h"

namespace OpenGL {

const char *const g_defaultVertexShader = 
	"attribute vec4 position;\n"
	"attribute vec2 texCoordIn;\n"
	"attribute vec4 blendColorIn;\n"
	"\n"
	"uniform mat4 projection;\n"
	"\n"
	"varying vec2 texCoord;\n"
	"varying vec4 blendColor;\n"
	"\n"
	"void main(void) {\n"
	"\ttexCoord    = texCoordIn;\n"
	"\tblendColor  = blendColorIn;\n"
	"\tgl_Position = projection * position;\n"
	"}\n";

#if !USE_FORCED_GLES2
const char *const g_defaultFragmentShaderGL =
	"varying vec2 texCoord;\n"
	"varying vec4 blendColor;\n"
	"\n"
	"uniform sampler2D texture;\n"
	"\n"
	"void main(void) {\n"
	"\tgl_FragColor = blendColor * texture2D(texture, texCoord);\n"
	"}\n";
#endif

#if !USE_FORCED_GL
const char *const g_defaultFragmentShaderGLES2 =
	"varying lowp vec2 texCoord;\n"
	"varying lowp vec4 blendColor;\n"
	"\n"
	"uniform sampler2D texture;\n"
	"\n"
	"void main(void) {\n"
	"\tgl_FragColor = blendColor * texture2D(texture, texCoord);\n"
	"}\n";
#endif

#if !USE_FORCED_GLES2

ShaderARB::ShaderARB(const Common::String &vertex, const Common::String &fragment)
    : Shader(vertex, fragment), _program(0), _projectionLocation(-1), _textureLocation(-1) {
}

ShaderARB::~ShaderARB() {
	// According to extension specification glDeleteObjectARB silently ignores
	// 0. However, with nVidia drivers this can cause GL_INVALID_VALUE, thus
	// we do not call it with 0 as parameter to avoid warnings.
	if (_program) {
		GL_CALL_SAFE(glDeleteObjectARB, (_program));
	}
}

void ShaderARB::destroy() {
	// According to extension specification glDeleteObjectARB silently ignores
	// 0. However, with nVidia drivers this can cause GL_INVALID_VALUE, thus
	// we do not call it with 0 as parameter to avoid warnings.
	if (_program) {
		GL_CALL(glDeleteObjectARB(_program));
	}
	_program = 0;
}

bool ShaderARB::recreate() {
	// Make sure any old programs are destroyed properly.
	destroy();

	GLhandleARB vertexShader = compileShader(_vertex.c_str(), GL_VERTEX_SHADER_ARB);
	if (!vertexShader) {
		return false;
	}

	GLhandleARB fragmentShader = compileShader(_fragment.c_str(), GL_FRAGMENT_SHADER_ARB);
	if (!fragmentShader) {
		GL_CALL(glDeleteObjectARB(vertexShader));
		return false;
	}

	GL_ASSIGN(_program, glCreateProgramObjectARB());
	if (!_program) {
		GL_CALL(glDeleteObjectARB(vertexShader));
		GL_CALL(glDeleteObjectARB(fragmentShader));
		return false;
	}

	GL_CALL(glAttachObjectARB(_program, vertexShader));
	GL_CALL(glAttachObjectARB(_program, fragmentShader));

	GL_CALL(glBindAttribLocationARB(_program, kPositionAttribLocation, "position"));
	GL_CALL(glBindAttribLocationARB(_program, kTexCoordAttribLocation, "texCoordIn"));
	GL_CALL(glBindAttribLocationARB(_program, kColorAttribLocation,    "blendColorIn"));

	GL_CALL(glLinkProgramARB(_program));

	GL_CALL(glDetachObjectARB(_program, fragmentShader));
	GL_CALL(glDeleteObjectARB(fragmentShader));

	GL_CALL(glDetachObjectARB(_program, vertexShader));
	GL_CALL(glDeleteObjectARB(vertexShader));

	GLint result;
	GL_CALL(glGetObjectParameterivARB(_program, GL_OBJECT_LINK_STATUS_ARB, &result));
	if (result == GL_FALSE) {
		GLint logSize;
		GL_CALL(glGetObjectParameterivARB(_program, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logSize));

		GLchar *log = new GLchar[logSize];
		GL_CALL(glGetInfoLogARB(_program, logSize, nullptr, log));
		warning("Could not link shader: \"%s\"", log);
		delete[] log;

		destroy();
		return false;
	}

	GL_ASSIGN(_projectionLocation, glGetUniformLocationARB(_program, "projection"));
	if (_projectionLocation == -1) {
		warning("Shader misses \"projection\" uniform.");
		destroy();
		return false;
	}

	GL_ASSIGN(_textureLocation, glGetUniformLocationARB(_program, "texture"));
	if (_textureLocation == -1) {
		warning("Shader misses \"texture\" uniform.");
		destroy();
		return false;
	}

	return true;
}

void ShaderARB::activate(const GLfloat *projectionMatrix) {
	// Activate program.
	GL_CALL(glUseProgramObjectARB(_program));

	// Set projection matrix.
	GL_CALL(glUniformMatrix4fvARB(_projectionLocation, 1, GL_FALSE, projectionMatrix));

	// We always use texture unit 0.
	GL_CALL(glUniform1iARB(_textureLocation, 0));
}

GLhandleARB ShaderARB::compileShader(const char *source, GLenum shaderType) {
	GLuint handle;
	GL_ASSIGN(handle, glCreateShaderObjectARB(shaderType));
	if (!handle) {
		return 0;
	}

	GL_CALL(glShaderSourceARB(handle, 1, &source, nullptr));
	GL_CALL(glCompileShaderARB(handle));

	GLint result;
	GL_CALL(glGetObjectParameterivARB(handle, GL_OBJECT_COMPILE_STATUS_ARB, &result));
	if (result == GL_FALSE) {
		GLint logSize;
		GL_CALL(glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logSize));

		GLchar *log = new GLchar[logSize];
		GL_CALL(glGetInfoLogARB(handle, logSize, nullptr, log));
		warning("Could not compile shader \"%s\": \"%s\"", source, log);
		delete[] log;

		GL_CALL(glDeleteObjectARB(handle));
		return 0;
	}

	return handle;
}

#endif // !USE_FORCED_GLES2

#if !USE_FORCED_GL

ShaderGLES2::ShaderGLES2(const Common::String &vertex, const Common::String &fragment)
    : Shader(vertex, fragment), _program(0), _projectionLocation(-1), _textureLocation(-1) {
}

ShaderGLES2::~ShaderGLES2() {
	GL_CALL_SAFE(glDeleteProgram, (_program));
}

void ShaderGLES2::destroy() {
	GL_CALL(glDeleteProgram(_program));
	_program = 0;
}

bool ShaderGLES2::recreate() {
	// Make sure any old programs are destroyed properly.
	destroy();

	GLuint vertexShader = compileShader(_vertex.c_str(), GL_VERTEX_SHADER);
	if (!vertexShader) {
		return false;
	}

	GLuint fragmentShader = compileShader(_fragment.c_str(), GL_FRAGMENT_SHADER);
	if (!fragmentShader) {
		GL_CALL(glDeleteShader(vertexShader));
		return false;
	}

	GL_ASSIGN(_program, glCreateProgram());
	if (!_program) {
		GL_CALL(glDeleteShader(vertexShader));
		GL_CALL(glDeleteShader(fragmentShader));
		return false;
	}

	GL_CALL(glAttachShader(_program, vertexShader));
	GL_CALL(glAttachShader(_program, fragmentShader));

	GL_CALL(glBindAttribLocation(_program, kPositionAttribLocation, "position"));
	GL_CALL(glBindAttribLocation(_program, kTexCoordAttribLocation, "texCoordIn"));
	GL_CALL(glBindAttribLocation(_program, kColorAttribLocation,    "blendColorIn"));

	GL_CALL(glLinkProgram(_program));

	GL_CALL(glDetachShader(_program, fragmentShader));
	GL_CALL(glDeleteShader(fragmentShader));

	GL_CALL(glDetachShader(_program, vertexShader));
	GL_CALL(glDeleteShader(vertexShader));

	GLint result;
	GL_CALL(glGetProgramiv(_program, GL_LINK_STATUS, &result));
	if (result == GL_FALSE) {
		GLint logSize;
		GL_CALL(glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &logSize));

		GLchar *log = new GLchar[logSize];
		GL_CALL(glGetProgramInfoLog(_program, logSize, nullptr, log));
		warning("Could not link shader: \"%s\"", log);
		delete[] log;

		destroy();
		return false;
	}

	GL_ASSIGN(_projectionLocation, glGetUniformLocation(_program, "projection"));
	if (_projectionLocation == -1) {
		warning("Shader misses \"projection\" uniform.");
		destroy();
		return false;
	}

	GL_ASSIGN(_textureLocation, glGetUniformLocation(_program, "texture"));
	if (_textureLocation == -1) {
		warning("Shader misses \"texture\" uniform.");
		destroy();
		return false;
	}

	return true;
}

void ShaderGLES2::activate(const GLfloat *projectionMatrix) {
	// Activate program.
	GL_CALL(glUseProgram(_program));

	// Set projection matrix.
	GL_CALL(glUniformMatrix4fv(_projectionLocation, 1, GL_FALSE, projectionMatrix));

	// We always use texture unit 0.
	GL_CALL(glUniform1i(_textureLocation, 0));
}

GLuint ShaderGLES2::compileShader(const char *source, GLenum shaderType) {
	GLuint handle;
	GL_ASSIGN(handle, glCreateShader(shaderType));
	if (!handle) {
		return 0;
	}

	GL_CALL(glShaderSource(handle, 1, &source, nullptr));
	GL_CALL(glCompileShader(handle));

	GLint result;
	GL_CALL(glGetShaderiv(handle, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {
		GLint logSize;
		GL_CALL(glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize));

		GLchar *log = new GLchar[logSize];
		GL_CALL(glGetShaderInfoLog(handle, logSize, nullptr, log));
		warning("Could not compile shader \"%s\": \"%s\"", source, log);
		delete[] log;

		GL_CALL(glDeleteShader(handle));
		return 0;
	}

	return handle;
}

#endif // !!USE_FORCED_GL

} // End of namespace OpenGL

#endif // !USE_FORCED_GLES
