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

#if defined(USE_OPENGL_SHADERS)

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
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar *log = new GLchar[logSize];
		glGetShaderInfoLog(shader, logSize, nullptr, log);
		error("Could not compile shader %s: %s", name.c_str(), log);
	}

	return shader;
}

static GLuint createCompatShader(const char *shaderSource, GLenum shaderType, const Common::String &name) {
	const GLchar *versionSource = OpenGLContext.type == kContextGLES2 ? "#version 100\n" : "#version 120\n";
	const GLchar *compatSource =
			shaderType == GL_VERTEX_SHADER ? compatVertex : compatFragment;
	const GLchar *shaderSources[] = {
		versionSource,
		compatSource,
		compatUniformBool,
		shaderSource
	};

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 4, shaderSources, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar *log = new GLchar[logSize];
		glGetShaderInfoLog(shader, logSize, nullptr, log);
		error("Could not compile shader %s: %s", name.c_str(), log);
	}

	return shader;
}

static GLuint loadShaderFromFile(const char *base, const char *extension, GLenum shaderType) {
	const Common::String filename = Common::String(base) + "." + extension;
	const GLchar *shaderSource = readFile(filename);

	GLuint shader = createCompatShader(shaderSource, shaderType, filename);

	delete[] shaderSource;

	return shader;
}

/**
 * A deleter for OpenGL programs pointers which can be used with Common::SharedPtr.
 */
struct SharedPtrProgramDeleter {
	void operator()(GLuint *ptr) {
		if (ptr) {
			glDeleteProgram(*ptr);
		}
		delete ptr;
	}
};

ShaderGL *ShaderGL::_previousShader = nullptr;

ShaderGL::ShaderGL(const Common::String &name, GLuint vertexShader, GLuint fragmentShader, const char **attributes)
	: _name(name) {
	assert(attributes);
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	for (int idx = 0; attributes[idx]; ++idx) {
		glBindAttribLocation(shaderProgram, idx, attributes[idx]);
		_attributes.push_back(VertexAttrib(idx, attributes[idx]));
	}
	glLinkProgram(shaderProgram);

	GLint status;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logSize;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);
		GLchar *log = new GLchar[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, nullptr, log);
		error("Could not link shader %s: %s", name.c_str(), log);
	}

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	_shaderNo = Common::SharedPtr<GLuint>(new GLuint(shaderProgram), SharedPtrProgramDeleter());
	_uniforms = Common::SharedPtr<UniformsMap>(new UniformsMap());
}

ShaderGL *ShaderGL::fromStrings(const Common::String &name, const char *vertex, const char *fragment, const char **attributes) {
	GLuint vertexShader = createDirectShader(vertex, GL_VERTEX_SHADER, name + ".vertex");
	GLuint fragmentShader = createDirectShader(fragment, GL_FRAGMENT_SHADER, name + ".fragment");
	return new ShaderGL(name, vertexShader, fragmentShader, attributes);
}

ShaderGL *ShaderGL::fromFiles(const char *vertex, const char *fragment, const char **attributes) {
	GLuint vertexShader = loadShaderFromFile(vertex, "vertex", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShaderFromFile(fragment, "fragment", GL_FRAGMENT_SHADER);

	Common::String name = Common::String::format("%s/%s", vertex, fragment);
	return new ShaderGL(name, vertexShader, fragmentShader, attributes);
}

void ShaderGL::use(bool forceReload) {
	static uint32 previousNumAttributes = 0;
	if (this == _previousShader && !forceReload)
		return;

	// The previous shader might have had more attributes. Disable any extra ones.
	if (_attributes.size() < previousNumAttributes) {
		for (uint32 i = _attributes.size(); i < previousNumAttributes; ++i) {
			glDisableVertexAttribArray(i);
		}
	}

	_previousShader = this;
	previousNumAttributes = _attributes.size();

	glUseProgram(*_shaderNo);
	for (uint32 i = 0; i < _attributes.size(); ++i) {
		VertexAttrib &attrib = _attributes[i];
		if (attrib._enabled) {
			glEnableVertexAttribArray(i);
			glBindBuffer(GL_ARRAY_BUFFER, attrib._vbo);
			glVertexAttribPointer(i, attrib._size, attrib._type, attrib._normalized, attrib._stride, (const GLvoid *)attrib._offset);
		} else {
			glDisableVertexAttribArray(i);
			switch (attrib._size) {
			case 2:
				glVertexAttrib2fv(i, attrib._const);
				break;
			case 3:
				glVertexAttrib3fv(i, attrib._const);
				break;
			case 4:
				glVertexAttrib4fv(i, attrib._const);
				break;
			}
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint ShaderGL::createBuffer(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) {
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(target, vbo);
	glBufferData(target, size, data, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return vbo;
}

void ShaderGL::freeBuffer(GLuint vbo) {
	glDeleteBuffers(1, &vbo);
}

VertexAttrib &ShaderGL::getAttributeAt(uint32 idx) {
	assert(idx < _attributes.size());
	return _attributes[idx];
}

VertexAttrib &ShaderGL::getAttribute(const char *attrib) {
	for (uint32 i = 0; i < _attributes.size(); ++i)
		if (_attributes[i]._name.equals(attrib))
			return _attributes[i];
	error("Could not find attribute %s in shader %s", attrib, _name.c_str());
	return _attributes[0];
}

void ShaderGL::enableVertexAttribute(const char *attrib, GLuint vbo, GLint size, GLenum type, GLboolean normalized, GLsizei stride, uint32 offset) {
	VertexAttrib &va = getAttribute(attrib);
	va._enabled = true;
	va._vbo = vbo;
	va._size = size;
	va._type = type;
	va._normalized = normalized;
	va._stride = stride;
	va._offset = offset;
}

void ShaderGL::disableVertexAttribute(const char *attrib, int size, const float *data) {
	VertexAttrib &va = getAttribute(attrib);
	va._enabled = false;
	va._size = size;
	for (int i = 0; i < size; ++i)
		va._const[i] = data[i];
}

void ShaderGL::unbind() {
	glUseProgram(0);
	_previousShader = nullptr;
}

ShaderGL::~ShaderGL() {
	// If this is the currently active shader, unbind
	if (_previousShader == this) {
		unbind();
	}
}

} // End of namespace OpenGL

#endif
