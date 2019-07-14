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
#include "common/util.h"

namespace Common {
DECLARE_SINGLETON(OpenGL::ShaderManager);
}

namespace OpenGL {

namespace {

#pragma mark - Builtin Shader Sources -

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

const char *const g_defaultFragmentShader =
	"varying vec2 texCoord;\n"
	"varying vec4 blendColor;\n"
	"\n"
	"uniform sampler2D shaderTexture;\n"
	"\n"
	"void main(void) {\n"
	"\tgl_FragColor = blendColor * texture2D(shaderTexture, texCoord);\n"
	"}\n";

const char *const g_lookUpFragmentShader =
	"varying vec2 texCoord;\n"
	"varying vec4 blendColor;\n"
	"\n"
	"uniform sampler2D shaderTexture;\n"
	"uniform sampler2D palette;\n"
	"\n"
	"const float adjustFactor = 255.0 / 256.0 + 1.0 / (2.0 * 256.0);"
	"\n"
	"void main(void) {\n"
	"\tvec4 index = texture2D(shaderTexture, texCoord);\n"
	"\tgl_FragColor = blendColor * texture2D(palette, vec2(index.a * adjustFactor, 0.0));\n"
	"}\n";


// Taken from: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_03#OpenGL_ES_2_portability
const char *const g_precisionDefines =
	"#ifdef GL_ES\n"
	"\t#if defined(GL_FRAGMENT_PRECISION_HIGH) && GL_FRAGMENT_PRECISION_HIGH == 1\n"
	"\t\tprecision highp float;\n"
	"\t#else\n"
	"\t\tprecision mediump float;\n"
	"\t#endif\n"
	"#else\n"
	"\t#define highp\n"
	"\t#define mediump\n"
	"\t#define lowp\n"
	"#endif\n";

} // End of anonymous namespace

#pragma mark - Uniform Values -

void ShaderUniformInteger::set(GLint location) const {
	GL_CALL(glUniform1i(location, _value));
}

void ShaderUniformFloat::set(GLint location) const {
	GL_CALL(glUniform1f(location, _value));
}

void ShaderUniformMatrix44::set(GLint location) const {
	GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, _matrix));
}

#pragma mark - Shader Implementation -

Shader::Shader(const Common::String &vertex, const Common::String &fragment)
    : _vertex(vertex), _fragment(fragment), _isActive(false), _program(0), _uniforms() {
	recreate();
}

Shader::~Shader() {
	// According to extension specification glDeleteObjectARB silently ignores
	// 0. However, with nVidia drivers this can cause GL_INVALID_VALUE, thus
	// we do not call it with 0 as parameter to avoid warnings.
	if (_program) {
		GL_CALL_SAFE(glDeleteProgram, (_program));
	}
}

void Shader::destroy() {
	// According to extension specification glDeleteObjectARB silently ignores
	// 0. However, with nVidia drivers this can cause GL_INVALID_VALUE, thus
	// we do not call it with 0 as parameter to avoid warnings.
	if (_program) {
		GL_CALL(glDeleteProgram(_program));
		_program = 0;
	}
}

bool Shader::recreate() {
	// Make sure any old programs are destroyed properly.
	destroy();

	GLshader vertexShader = compileShader(_vertex.c_str(), GL_VERTEX_SHADER);
	if (!vertexShader) {
		return false;
	}

	GLshader fragmentShader = compileShader(_fragment.c_str(), GL_FRAGMENT_SHADER);
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

	// Set program object in case shader is active during recreation.
	if (_isActive) {
		GL_CALL(glUseProgram(_program));
	}

	for (UniformMap::iterator i = _uniforms.begin(), end = _uniforms.end(); i != end; ++i) {
		i->_value.location = getUniformLocation(i->_key.c_str());
		i->_value.altered = true;
		if (_isActive) {
			i->_value.set();
		}
	}

	return true;
}

void Shader::activate() {
	// Activate program.
	GL_CALL(glUseProgram(_program));

	// Reset changed uniform values.
	for (UniformMap::iterator i = _uniforms.begin(), end = _uniforms.end(); i != end; ++i) {
		i->_value.set();
	}

	_isActive = true;
}

void Shader::deactivate() {
	_isActive = false;
}

GLint Shader::getAttributeLocation(const char *name) const {
	GLint result = -1;
	GL_ASSIGN(result, glGetAttribLocation(_program, name));
	return result;
}

GLint Shader::getUniformLocation(const char *name) const {
	GLint result = -1;
	GL_ASSIGN(result, glGetUniformLocation(_program, name));
	return result;
}

bool Shader::setUniform(const Common::String &name, ShaderUniformValue *value) {
	UniformMap::iterator uniformIter = _uniforms.find(name);
	Uniform *uniform;

	if (uniformIter == _uniforms.end()) {
		const GLint location = getUniformLocation(name.c_str());
		if (location == -1) {
			delete value;
			return false;
		}

		uniform = &_uniforms[name];
		uniform->location = location;
	} else {
		uniform = &uniformIter->_value;
	}

	uniform->value = Common::SharedPtr<ShaderUniformValue>(value);
	uniform->altered = true;
	if (_isActive) {
		uniform->set();
	}

	return true;
}

GLshader Shader::compileShader(const char *source, GLenum shaderType) {
	const GLchar *versionSource = g_context.type == kContextGLES2 ? "#version 100\n" : "#version 120\n";
	GLshader handle;
	GL_ASSIGN(handle, glCreateShader(shaderType));
	if (!handle) {
		return 0;
	}

	const char *const shaderSources[] = {
		versionSource,
		g_precisionDefines,
		source
	};

	GL_CALL(glShaderSource(handle, ARRAYSIZE(shaderSources), shaderSources, nullptr));
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

ShaderManager::ShaderManager() : _initializeShaders(true) {
	for (int i = 0; i < ARRAYSIZE(_builtIn); ++i) {
		_builtIn[i] = nullptr;
	}
}

ShaderManager::~ShaderManager() {
	for (int i = 0; i < ARRAYSIZE(_builtIn); ++i) {
		delete _builtIn[i];
	}
}

void ShaderManager::notifyDestroy() {
	for (int i = 0; i < ARRAYSIZE(_builtIn); ++i) {
		_builtIn[i]->destroy();
	}
}

void ShaderManager::notifyCreate() {
	if (_initializeShaders) {
		_initializeShaders = false;

		_builtIn[kDefault] = new Shader(g_defaultVertexShader, g_defaultFragmentShader);
		_builtIn[kCLUT8LookUp] = new Shader(g_defaultVertexShader, g_lookUpFragmentShader);
		_builtIn[kCLUT8LookUp]->setUniform1I("palette", 1);

		for (uint i = 0; i < kMaxUsages; ++i) {
			_builtIn[i]->setUniform1I("shaderTexture", 0);
		}
	} else {
		for (int i = 0; i < ARRAYSIZE(_builtIn); ++i) {
			_builtIn[i]->recreate();
		}
	}
}

Shader *ShaderManager::query(ShaderUsage shader) const {
	if (shader == kMaxUsages) {
		warning("OpenGL: ShaderManager::query used with kMaxUsages");
		return nullptr;
	}

	return _builtIn[shader];
}

} // End of namespace OpenGL

#endif // !USE_FORCED_GLES
