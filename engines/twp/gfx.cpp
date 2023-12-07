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

#include "gfx.h"
#include "common/debug.h"
#include "graphics/opengl/context.h"
#include "graphics/opengl/system_headers.h"

namespace Twp {

static Texture gEmptyTexture;

#ifdef DEBUG
	#define GL_CHECK(expr)                           \
		do {                                         \
			(expr);                                  \
			checkGLError(__FILE__, __LINE__, #expr); \
		} while (false)
	#define GL_CHECK0()                           \
		do {                                      \
			checkGLError(__FILE__, __LINE__, ""); \
		} while (false)
#else
	#define GL_CHECK(expr) (expr)
	#define GL_CHECK0()
#endif

static void checkGLError(const char *filename, int line, const char *info) {
	int err = glGetError();
	if (err != GL_NO_ERROR) {
		const char *name = nullptr;
		const char *desc = nullptr;
		switch (err) {
		case GL_INVALID_ENUM:
			name = "GL_INVALID_ENUM";
			desc = "An unacceptable value is specified for an enumerated argument.";
			break;
		case GL_INVALID_VALUE:
			name = "GL_INVALID_VALUE";
			desc = "A numeric argument is out of range.";
			break;
		case GL_INVALID_OPERATION:
			name = "GL_INVALID_OPERATION";
			desc = "The specified operation is not allowed in the current state.";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			name = "GL_INVALID_FRAMEBUFFER_OPERATION";
			desc = "The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete.";
			break;
		case GL_OUT_OF_MEMORY:
			name = "GL_OUT_OF_MEMORY";
			desc = "There is not enough memory left to execute the command.";
			break;
		default:
			break;
		}
		if (name) {
			debug("%s: %s at %s:%d(%s)", name, desc, filename, line, info);
		} else {
			debug("Code %d at %s:%d(%s)", err, filename, line, info);
		}
	}
}

Math::Matrix4 ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
	Math::Matrix4 result;
	float *m = result.getData();
	m[0] = 2.f / (right - left);
	m[5] = 2.f / (top - bottom);
	m[10] = -2.f / (zFar - zNear);
	m[12] = -(right + left) / (right - left);
	m[13] = -(top + bottom) / (top - bottom);
	m[14] = -(zFar + zNear) / (zFar - zNear);
	m[15] = 1;
	return result;
}

struct Use {
public:
	Use(GLint program) : _prev(0), program(program) {
		glGetIntegerv(GL_CURRENT_PROGRAM, &_prev);
		if (_prev != program)
			glUseProgram(program);
	}

	~Use() {
		if (_prev != program)
			glUseProgram(_prev);
	}

private:
	GLint _prev, program;
};

static Use use(GLint program) {
	Use use(program);
	return use;
}

static GLint getFormat(int channels) {
	switch (channels) {
	case 3:
		return GL_RGB;
	case 4:
		return GL_RGBA;
	default:
		error("Can't get format for %d channels", channels);
	}
}

void Texture::load(const Graphics::Surface &surface) {
	width = surface.w;
	height = surface.h;
	const void *data = surface.getPixels();
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, surface.format.bytesPerPixel);
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, getFormat(surface.format.bytesPerPixel), width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
}

void Texture::bind(const Texture *pTexture) {
	if (pTexture && pTexture->id) {
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, pTexture->id));
	} else {
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}
}

Shader::Shader() {
}

void Shader::init(const char *vertex, const char *fragment) {
	if (vertex) {
		_vertex = loadShader(vertex, GL_VERTEX_SHADER);
	}
	if (fragment) {
		_fragment = loadShader(fragment, GL_FRAGMENT_SHADER);
	}
	GL_CHECK(program = glCreateProgram());
	GL_CHECK(glAttachShader(program, _vertex));
	GL_CHECK(glAttachShader(program, _fragment));
	GL_CHECK(glLinkProgram(program));
}

uint32 Shader::loadShader(const char *code, uint32 shaderType) {
	uint32 result;
	GL_CHECK(result = glCreateShader(shaderType));
	GL_CHECK(glShaderSource(result, 1, &code, nullptr));
	GL_CHECK(glCompileShader(result));
	statusShader(result);
	return result;
}

void Shader::statusShader(uint32 shader) {
	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		int logLength;
		char message[1024];
		glGetShaderInfoLog(shader, 1024, &logLength, &message[0]);
		debug("%s", message);
	}
}

int Shader::getUniformLocation(const char *name) {
	int loc;
	GL_CHECK(loc = glGetUniformLocation(program, name));
	return loc;
}

void Shader::setUniform(const char *name, Math::Matrix4 value) {
	use(program);
	int loc = getUniformLocation(name);
	GL_CHECK(glUniformMatrix4fv(loc, 1, GL_FALSE, value.getData()));
}

Gfx::Gfx() : _vbo(0), _ebo(0) {
}

void Gfx::init() {
	Graphics::PixelFormat fmt(4, 8, 8, 8, 8, 0, 8, 16, 24);
	byte pixels[] = {0xFF, 0xFF, 0xFF, 0xFF};
	Graphics::Surface empty;
	empty.w = 1;
	empty.h = 1;
	empty.format = fmt;
	empty.setPixels(pixels);
	gEmptyTexture.load(empty);
	const char *vsrc = R"(#version 110
	uniform mat4 u_transform;
	attribute vec2 a_position;
	attribute vec4 a_color;
	attribute vec2 a_texCoords;
	varying vec4 v_color;
	varying vec2 v_texCoords;
	void main() {
		gl_Position = u_transform * vec4(a_position, 0.0, 1.0);
		v_color = a_color;
		v_texCoords = a_texCoords;
	})";
	const char* fsrc = R"(#version 110
	varying vec4 v_color;
	varying vec2 v_texCoords;
	uniform sampler2D u_texture;
	void main() {
		vec4 tex_color = texture2D(u_texture, v_texCoords);
		gl_FragColor = v_color * tex_color;
	})";
	_shader.init(vsrc, fsrc);
	_mvp = ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);

	GL_CHECK(glGenBuffers(1, &_vbo));
	GL_CHECK(glGenBuffers(1, &_ebo));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo));
	GLint p, c, t, tex, tr;
	GL_CHECK(p = glGetAttribLocation(_shader.program, "a_position"));
	GL_CHECK(c = glGetAttribLocation(_shader.program, "a_color"));
	GL_CHECK(t = glGetAttribLocation(_shader.program, "a_texCoords"));
	GL_CHECK(tex = glGetUniformLocation(_shader.program, "u_texture"));
	GL_CHECK(tr = glGetUniformLocation(_shader.program, "u_transform"));
	GL_CHECK(glVertexAttribPointer(p, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0));
	GL_CHECK(glEnableVertexAttribArray(p));
	GL_CHECK(glVertexAttribPointer(c, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float))));
	GL_CHECK(glEnableVertexAttribArray(c));
	GL_CHECK(glVertexAttribPointer(t, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float))));
	GL_CHECK(glEnableVertexAttribArray(c));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Gfx::clear(Color color) {
	glClearColor(color.rgba.r, color.rgba.g, color.rgba.b, color.rgba.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

Math::Matrix4 Gfx::getFinalTransform(Math::Matrix4 trsf) {
	return _mvp * trsf;
}

void Gfx::noTexture() {
	_texture = &gEmptyTexture;
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, gEmptyTexture.id));
}

void Gfx::drawLines(Vertex *vertices, int count, Math::Matrix4 trsf) {
	noTexture();
	drawPrimitives(GL_LINE_STRIP, vertices, count, trsf);
}

void Gfx::drawPrimitives(uint32 primitivesType, Vertex *vertices, int v_size, Math::Matrix4 trsf) {
	if (v_size > 0) {
		// set blending
		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
		GL_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

		GL_CHECK(glUseProgram(_shader.program));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
		GL_CHECK(glEnableVertexAttribArray(0));
		GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0));
		GL_CHECK(glEnableVertexAttribArray(2));
		GL_CHECK(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float))));
		GL_CHECK(glEnableVertexAttribArray(1));
		GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float))));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * v_size, vertices, GL_STREAM_DRAW));

		GL_CHECK(glActiveTexture(GL_TEXTURE0));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, _texture->id));
		GL_CHECK(glUniform1i(0, 0));

		_shader.setUniform("u_transform", getFinalTransform(trsf));
		GL_CHECK(glDrawArrays((GLenum)primitivesType, 0, v_size));

		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GL_CHECK(glDisableVertexAttribArray(0));
		GL_CHECK(glDisableVertexAttribArray(1));
		GL_CHECK(glDisableVertexAttribArray(2));

		glDisable(GL_BLEND);
	}
}

void Gfx::drawPrimitives(uint32 primitivesType, Vertex *vertices, int v_size, uint32 *indices, int i_size, Math::Matrix4 trsf) {
	if (i_size > 0) {
		// set blending
		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
		GL_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

		GL_CHECK(glUseProgram(_shader.program));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _vbo));

		GL_CHECK(glEnableVertexAttribArray(0));
		GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0));
		GL_CHECK(glEnableVertexAttribArray(2));
		GL_CHECK(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float))));
		GL_CHECK(glEnableVertexAttribArray(1));
		GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float))));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * v_size, vertices, GL_STREAM_DRAW));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * i_size, indices, GL_STREAM_DRAW));

		GL_CHECK(glActiveTexture(GL_TEXTURE0));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, _texture->id));
		GL_CHECK(glUniform1i(0, 0));

		_shader.setUniform("u_transform", getFinalTransform(trsf));
		GL_CHECK(glDrawElements(primitivesType, i_size, GL_UNSIGNED_INT, NULL));

		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GL_CHECK(glDisableVertexAttribArray(0));
		GL_CHECK(glDisableVertexAttribArray(1));
		GL_CHECK(glDisableVertexAttribArray(2));

		glDisable(GL_BLEND);
	}
}

void Gfx::draw(Vertex *vertices, int v_size, uint32 *indices, int i_size, Math::Matrix4 trsf) {
	drawPrimitives(GL_TRIANGLES, vertices, v_size, indices, i_size, trsf);
}

void Gfx::drawQuad(Math::Vector2d pos, Math::Vector2d size, Color color, Math::Matrix4 trsf) {
	float w = size.getX();
	float h = size.getY();
	float x = pos.getX();
	float y = pos.getY();
	Vertex vertices[] = {
		Vertex{.pos = {x + w, y + h}, .texCoords = {1, 0}, .color = color},
		Vertex{.pos = {x + w, y}, .texCoords = {1, 1}, .color = color},
		Vertex{.pos = {x, y}, .texCoords = {0, 1}, .color = color},
		Vertex{.pos = {x, y + h}, .texCoords = {0, 0}, .color = color}};
	noTexture();
	uint32 quadIndices[] = {
		0, 1, 3,
		1, 2, 3};
	draw(vertices, 4, quadIndices, 6, trsf);
}

void Gfx::drawSprite(Math::Vector2d pos, Common::Rect textRect, Texture &texture, Color color, Math::Matrix4 trsf, bool flipX, bool flipY) {
	float l = textRect.left / (float)texture.width;
	float r = textRect.right / (float)texture.width;
	float t = textRect.top / (float)texture.height;
	float b = textRect.bottom / (float)texture.height;
	if (flipX)
		SWAP(l, r);
	if (flipY)
		SWAP(t, b);

	Vertex vertices[] = {
		{.pos = {pos.getX() + textRect.width(), pos.getY() + textRect.height()}, .texCoords = {r, t}, .color = color},
		{.pos = {pos.getX() + textRect.width(), pos.getY()}, .texCoords = {r, b}, .color = color},
		{.pos = {pos.getX(), pos.getY()}, .texCoords = {l, b}, .color = color},
		{.pos = {pos.getX(), pos.getY() + textRect.height()}, .texCoords = {l, t}, .color = color}};
	_texture = &texture;
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture.id));
	uint32 quadIndices[] = {
		0, 1, 3,
		1, 2, 3};
	draw(vertices, 4, quadIndices, 6, trsf);
}

void Gfx::camera(float w, float h) {
	_cameraSize = Math::Vector2d(w, h);
	_mvp = ortho(0.f, w, 0.f, h, -1.f, 1.f);
}
}
