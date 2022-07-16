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

#include "common/scummsys.h"
#include "common/config-manager.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL) && !USE_FORCED_GLES

#include "graphics/opengl/shader.h"

#include "graphics/opengl/context.h"

namespace OpenGL {

static const char *compatVertex =
	"#if defined(GL_ES)\n"
		"#define ROUND(x) (sign(x) * floor(abs(x) + .5))\n"
		"#define in attribute\n"
		"#define out varying\n"
	"#elif __VERSION__ < 130\n"
		"#define ROUND(x) (sign(x) * floor(abs(x) + .5))\n"
		"#define highp\n"
		"#define in attribute\n"
		"#define out varying\n"
	"#else\n"
		"#define ROUND(x) round(x)\n"
	"#endif\n";

static const char *compatFragment =
	"#if defined(GL_ES)\n"
		"#define in varying\n"
		"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
			"precision highp float;\n"
		"#else\n"
			"precision mediump float;\n"
		"#endif\n"
		"#define OUTPUT\n"
		"#define outColor gl_FragColor\n"
		"#define texture texture2D\n"
	"#elif __VERSION__ < 130\n"
		"#define in varying\n"
		"#define OUTPUT\n"
		"#define outColor gl_FragColor\n"
		"#define texture texture2D\n"
	"#else\n"
		"#define OUTPUT out vec4 outColor;\n"
	"#endif\n";

// OGLES2 on AmigaOS doesn't support uniform booleans, let's introduce some shim
#if defined(AMIGAOS)
static const char *compatUniformBool =
	"#define UBOOL mediump int\n"
	"#define UBOOL_TEST(v) (v != 0)\n";
#else
static const char *compatUniformBool =
	"#define UBOOL bool\n"
	"#define UBOOL_TEST(v) v\n";
#endif


static const GLchar *readFile(const Common::String &filename) {
	Common::File file;
	Common::String shaderDir;

	// Allow load shaders from source code directory without install them.
	// It's used for development purpose.
	// Additionally allow load shaders outside distribution data path,
	// 'extrapath' is used temporary in SearchMan.
	SearchMan.addDirectory("GRIM_SHADERS", "engines/grim", 0, 2);
	SearchMan.addDirectory("MYST3_SHADERS", "engines/myst3", 0, 2);
	SearchMan.addDirectory("STARK_SHADERS", "engines/stark", 0, 2);
	SearchMan.addDirectory("WINTERMUTE_SHADERS", "engines/wintermute/base/gfx/opengl", 0, 2);
	SearchMan.addDirectory("PLAYGROUND3D_SHADERS", "engines/playground3d", 0, 2);
	if (ConfMan.hasKey("extrapath")) {
		SearchMan.addDirectory("EXTRA_PATH", Common::FSNode(ConfMan.get("extrapath")), 0, 2);
	}
#if !defined(IPHONE)
	shaderDir = "shaders/";
#endif
	file.open(shaderDir + filename);
	if (!file.isOpen())
		error("Could not open shader %s!", filename.c_str());
	SearchMan.remove("GRIM_SHADERS");
	SearchMan.remove("MYST3_SHADERS");
	SearchMan.remove("STARK_SHADERS");
	SearchMan.remove("WINTERMUTE_SHADERS");
	SearchMan.remove("PLAYGROUND3D_SHADERS");
	SearchMan.remove("EXTRA_PATH");

	const int32 size = file.size();
	GLchar *shaderSource = new GLchar[size + 1];

	file.read(shaderSource, size);
	file.close();
	shaderSource[size] = '\0';
	return shaderSource;
}

static GLuint createDirectShader(const char *shaderSource, GLenum shaderType, const Common::String &name) {
	GLuint shader;
	GL_ASSIGN(shader, glCreateShader(shaderType));
	GL_CALL(glShaderSource(shader, 1, &shaderSource, NULL));
	GL_CALL(glCompileShader(shader));

	GLint status;
	GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
	if (status != GL_TRUE) {
		GLint logSize;
		GL_CALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize));
		GLchar *log = new GLchar[logSize];
		GL_CALL(glGetShaderInfoLog(shader, logSize, nullptr, log));
		error("Could not compile shader %s: %s", name.c_str(), log);
	}

	return shader;
}

static GLuint createCompatShader(const char *shaderSource, GLenum shaderType, const Common::String &name, int compatGLSLVersion) {
	GLchar versionSource[20];
	if (OpenGLContext.type == kContextGLES2) {
		switch(compatGLSLVersion) {
			case 100:
			case 110:
			case 120:
				// GLSL ES 1.00 is a subset of GLSL 1.20
				compatGLSLVersion = 100;
				break;
			default:
				error("Invalid GLSL version %d", compatGLSLVersion);
		}
	} else {
		switch(compatGLSLVersion) {
			case 100:
			case 110:
			case 120:
				break;
			default:
				error("Invalid GLSL version %d", compatGLSLVersion);
		}
	}

	if (OpenGLContext.glslVersion < compatGLSLVersion) {
		error("Required GLSL version %d is not supported (%d maximum)", compatGLSLVersion, OpenGLContext.glslVersion);
	}

	sprintf(versionSource, "#version %d\n", compatGLSLVersion);

	const GLchar *compatSource =
			shaderType == GL_VERTEX_SHADER ? compatVertex : compatFragment;
	const GLchar *shaderSources[] = {
		versionSource,
		compatSource,
		compatUniformBool,
		shaderSource
	};

	GLuint shader;
	GL_ASSIGN(shader, glCreateShader(shaderType));
	GL_CALL(glShaderSource(shader, 4, shaderSources, NULL));
	GL_CALL(glCompileShader(shader));

	GLint status;
	GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
	if (status != GL_TRUE) {
		GLint logSize;
		GL_CALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize));
		GLchar *log = new GLchar[logSize];
		GL_CALL(glGetShaderInfoLog(shader, logSize, nullptr, log));
		error("Could not compile shader %s: %s", name.c_str(), log);
	}

	return shader;
}

static GLuint loadShaderFromFile(const char *base, const char *extension, GLenum shaderType, int compatGLSLVersion) {
	const Common::String filename = Common::String(base) + "." + extension;
	const GLchar *shaderSource = readFile(filename);

	GLuint shader;
	if (compatGLSLVersion) {
		shader = createCompatShader(shaderSource, shaderType, filename, compatGLSLVersion);
	} else {
		shader = createDirectShader(shaderSource, shaderType, filename);
	}

	delete[] shaderSource;

	return shader;
}

/**
 * A deleter for OpenGL programs pointers which can be used with Common::SharedPtr.
 */
struct SharedPtrProgramDeleter {
	void operator()(GLuint *ptr) {
		if (ptr) {
			GL_CALL(glDeleteProgram(*ptr));
		}
		delete ptr;
	}
};

Shader *Shader::_previousShader = nullptr;
uint32 Shader::previousNumAttributes = 0;


Shader::Shader(const Common::String &name, GLuint vertexShader, GLuint fragmentShader, const char *const *attributes)
	: _name(name) {
	assert(attributes);
	GLuint shaderProgram;
	GL_ASSIGN(shaderProgram, glCreateProgram());
	GL_CALL(glAttachShader(shaderProgram, vertexShader));
	GL_CALL(glAttachShader(shaderProgram, fragmentShader));

	for (int idx = 0; attributes[idx]; ++idx) {
		GL_CALL(glBindAttribLocation(shaderProgram, idx, attributes[idx]));
		_attributes.push_back(VertexAttrib(idx, attributes[idx]));
	}
	GL_CALL(glLinkProgram(shaderProgram));

	GLint status;
	GL_CALL(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status));
	if (status != GL_TRUE) {
		GLint logSize;
		GL_CALL(glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize));
		GLchar *log = new GLchar[logSize];
		GL_CALL(glGetProgramInfoLog(shaderProgram, logSize, nullptr, log));
		error("Could not link shader %s: %s", name.c_str(), log);
	}

	GL_CALL(glDetachShader(shaderProgram, vertexShader));
	GL_CALL(glDetachShader(shaderProgram, fragmentShader));

	GL_CALL(glDeleteShader(vertexShader));
	GL_CALL(glDeleteShader(fragmentShader));

	_shaderNo = Common::SharedPtr<GLuint>(new GLuint(shaderProgram), SharedPtrProgramDeleter());
	_uniforms = Common::SharedPtr<UniformsMap>(new UniformsMap());
}

Shader *Shader::fromStrings(const Common::String &name, const char *vertex, const char *fragment, const char *const *attributes, int compatGLSLVersion) {
	GLuint vertexShader, fragmentShader;

	if (compatGLSLVersion) {
		vertexShader = createCompatShader(vertex, GL_VERTEX_SHADER, name + ".vertex", compatGLSLVersion);
		fragmentShader = createCompatShader(fragment, GL_FRAGMENT_SHADER, name + ".fragment", compatGLSLVersion);
	} else {
		vertexShader = createDirectShader(vertex, GL_VERTEX_SHADER, name + ".vertex");
		fragmentShader = createDirectShader(fragment, GL_FRAGMENT_SHADER, name + ".fragment");
	}
	return new Shader(name, vertexShader, fragmentShader, attributes);
}

Shader *Shader::fromFiles(const char *vertex, const char *fragment, const char *const *attributes, int compatGLSLVersion) {
	GLuint vertexShader = loadShaderFromFile(vertex, "vertex", GL_VERTEX_SHADER, compatGLSLVersion);
	GLuint fragmentShader = loadShaderFromFile(fragment, "fragment", GL_FRAGMENT_SHADER, compatGLSLVersion);

	Common::String name = Common::String::format("%s/%s", vertex, fragment);
	return new Shader(name, vertexShader, fragmentShader, attributes);
}

void Shader::use(bool forceReload) {
	if (this == _previousShader && !forceReload)
		return;

	// The previous shader might have had more attributes. Disable any extra ones.
	if (_attributes.size() < previousNumAttributes) {
		for (uint32 i = _attributes.size(); i < previousNumAttributes; ++i) {
			GL_CALL(glDisableVertexAttribArray(i));
		}
	}

	_previousShader = this;
	previousNumAttributes = _attributes.size();

	GL_CALL(glUseProgram(*_shaderNo));
	for (uint32 i = 0; i < _attributes.size(); ++i) {
		VertexAttrib &attrib = _attributes[i];
		if (attrib._enabled) {
			GL_CALL(glEnableVertexAttribArray(i));
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, attrib._vbo));
			GL_CALL(glVertexAttribPointer(i, attrib._size, attrib._type, attrib._normalized, attrib._stride, (const void *)attrib._pointer));
		} else {
			GL_CALL(glDisableVertexAttribArray(i));
			switch (attrib._size) {
			case 2:
				GL_CALL(glVertexAttrib2fv(i, attrib._const));
				break;
			case 3:
				GL_CALL(glVertexAttrib3fv(i, attrib._const));
				break;
			case 4:
				GL_CALL(glVertexAttrib4fv(i, attrib._const));
				break;
			}
		}
	}
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

GLuint Shader::createBuffer(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) {
	GLuint vbo;
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(target, vbo));
	GL_CALL(glBufferData(target, size, data, usage));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	return vbo;
}

void Shader::freeBuffer(GLuint vbo) {
	GL_CALL(glDeleteBuffers(1, &vbo));
}

bool Shader::addAttribute(const char *attrib) {
	uint32 i;
	for (i = 0; i < _attributes.size(); ++i)
		if (_attributes[i]._name.equals(attrib))
			return true;

	GLint result = -1;
	GL_ASSIGN(result, glGetAttribLocation(*_shaderNo, attrib));
	if (result == -1)
		return false;

	GL_CALL(glBindAttribLocation(*_shaderNo, i, attrib));
	_attributes.push_back(VertexAttrib(i, attrib));
	return true;
}

VertexAttrib &Shader::getAttributeAt(uint32 idx) {
	assert(idx < _attributes.size());
	return _attributes[idx];
}

VertexAttrib &Shader::getAttribute(const char *attrib) {
	for (uint32 i = 0; i < _attributes.size(); ++i)
		if (_attributes[i]._name.equals(attrib))
			return _attributes[i];
	error("Could not find attribute %s in shader %s", attrib, _name.c_str());
	return _attributes[0];
}

void Shader::enableVertexAttribute(const char *attrib, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {
	VertexAttrib &va = getAttribute(attrib);
	va._enabled = true;
	va._vbo = 0;
	va._size = size;
	va._type = type;
	va._normalized = normalized;
	va._stride = stride;
	va._pointer = (uintptr)pointer;
}

void Shader::enableVertexAttribute(const char *attrib, GLuint vbo, GLint size, GLenum type, GLboolean normalized, GLsizei stride, uint32 offset) {
	VertexAttrib &va = getAttribute(attrib);
	va._enabled = true;
	va._vbo = vbo;
	va._size = size;
	va._type = type;
	va._normalized = normalized;
	va._stride = stride;
	va._pointer = offset;
}

void Shader::disableVertexAttribute(const char *attrib, int size, const float *data) {
	VertexAttrib &va = getAttribute(attrib);
	va._enabled = false;
	va._size = size;
	for (int i = 0; i < size; ++i)
		va._const[i] = data[i];
}

void Shader::unbind() {
	GL_CALL(glUseProgram(0));
	_previousShader = nullptr;

	// Disable all vertex attributes as well
	for (uint32 i = 0; i < previousNumAttributes; ++i) {
		GL_CALL(glDisableVertexAttribArray(i));
	}
	previousNumAttributes = 0;
}

Shader::~Shader() {
	// If this is the currently active shader, unbind
	if (_previousShader == this) {
		unbind();
	}
}

} // End of namespace OpenGL

#endif
