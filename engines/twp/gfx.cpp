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

#include "twp/gfx.h"
#include "common/debug.h"
#include "graphics/opengl/debug.h"
#include "graphics/opengl/context.h"
#include "graphics/opengl/system_headers.h"

namespace Twp {

static Texture gEmptyTexture;

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
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, getFormat(surface.format.bytesPerPixel), width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
}

void Texture::bind(const Texture *pTexture) {
	if (pTexture && pTexture->id) {
		GL_CALL(glBindTexture(GL_TEXTURE_2D, pTexture->id));
	} else {
		GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
	}
}

Shader::Shader() {
}

Shader::~Shader() {
}

void Shader::init(const char *vertex, const char *fragment) {
	if (vertex) {
		_vertex = loadShader(vertex, GL_VERTEX_SHADER);
	}
	if (fragment) {
		_fragment = loadShader(fragment, GL_FRAGMENT_SHADER);
	}
	GL_CALL(program = glCreateProgram());
	GL_CALL(glAttachShader(program, _vertex));
	GL_CALL(glAttachShader(program, _fragment));
	GL_CALL(glLinkProgram(program));
}

uint32 Shader::loadShader(const char *code, uint32 shaderType) {
	uint32 result;
	GL_CALL(result = glCreateShader(shaderType));
	GL_CALL(glShaderSource(result, 1, &code, nullptr));
	GL_CALL(glCompileShader(result));
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
	GL_CALL(loc = glGetUniformLocation(program, name));
	return loc;
}

void Shader::setUniform(const char *name, Math::Matrix4 value) {
	GLint prev;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prev);
	glUseProgram(program);
	int loc = getUniformLocation(name);
	GL_CALL(glUniformMatrix4fv(loc, 1, GL_FALSE, value.getData()));
	glUseProgram(prev);
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
	_defaultShader.init(vsrc, fsrc);
	_shader = &_defaultShader;
	_mvp = ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);

	GL_CALL(glGenBuffers(1, &_vbo));
	GL_CALL(glGenBuffers(1, &_ebo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo));
	GL_CALL(_posLoc = glGetAttribLocation(_defaultShader.program, "a_position"));
	GL_CALL(_colLoc = glGetAttribLocation(_defaultShader.program, "a_color"));
	GL_CALL(_texCoordsLoc = glGetAttribLocation(_defaultShader.program, "a_texCoords"));
	GL_CALL(_texLoc = glGetUniformLocation(_defaultShader.program, "u_texture"));
	GL_CALL(_trsfLoc = glGetUniformLocation(_defaultShader.program, "u_transform"));
	GL_CALL(glVertexAttribPointer(_posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0));
	GL_CALL(glEnableVertexAttribArray(_posLoc));
	GL_CALL(glVertexAttribPointer(_colLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float))));
	GL_CALL(glEnableVertexAttribArray(_colLoc));
	GL_CALL(glVertexAttribPointer(_texCoordsLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float))));
	GL_CALL(glEnableVertexAttribArray(_texCoordsLoc));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Gfx::clear(Color color) {
	glClearColor(color.rgba.r, color.rgba.g, color.rgba.b, color.rgba.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

Math::Matrix4 Gfx::getFinalTransform(Math::Matrix4 trsf) {
	Math::Matrix4 t(trsf);
	t.transpose();
	return t * _mvp;
}

void Gfx::noTexture() {
	_texture = &gEmptyTexture;
	GL_CALL(glBindTexture(GL_TEXTURE_2D, gEmptyTexture.id));
}

void Gfx::drawLines(Vertex *vertices, int count, Math::Matrix4 trsf) {
	noTexture();
	drawPrimitives(GL_LINE_STRIP, vertices, count, trsf);
}

void Gfx::drawPrimitives(uint32 primitivesType, Vertex *vertices, int v_size, Math::Matrix4 trsf) {
	if (v_size > 0) {
		// set blending
		GL_CALL(glEnable(GL_BLEND));
		GL_CALL(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
		GL_CALL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

		GL_CALL(glUseProgram(_shader->program));

		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
		GL_CALL(glEnableVertexAttribArray(_posLoc));
		GL_CALL(glVertexAttribPointer(_posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0));
		GL_CALL(glEnableVertexAttribArray(_colLoc));
		GL_CALL(glVertexAttribPointer(_colLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float))));
		GL_CALL(glEnableVertexAttribArray(_texCoordsLoc));
		GL_CALL(glVertexAttribPointer(_texCoordsLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float))));

		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * v_size, vertices, GL_STREAM_DRAW));

		GL_CALL(glActiveTexture(GL_TEXTURE0));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, _texture->id));
		GL_CALL(glUniform1i(0, 0));

		Math::Matrix4 m = getFinalTransform(trsf);
		_shader->setUniform("u_transform", m);
		GL_CALL(glDrawArrays((GLenum)primitivesType, 0, v_size));

		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GL_CALL(glDisableVertexAttribArray(0));
		GL_CALL(glDisableVertexAttribArray(1));
		GL_CALL(glDisableVertexAttribArray(2));

		glDisable(GL_BLEND);
	}
}

void Gfx::drawPrimitives(uint32 primitivesType, Vertex *vertices, int v_size, uint32 *indices, int i_size, Math::Matrix4 trsf) {
	if (i_size > 0) {
		// set blending
		GL_CALL(glEnable(GL_BLEND));
		GL_CALL(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
		GL_CALL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

		GL_CALL(glUseProgram(_shader->program));

		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));

		GL_CALL(glEnableVertexAttribArray(0));
		GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0));
		GL_CALL(glEnableVertexAttribArray(2));
		GL_CALL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(2 * sizeof(float))));
		GL_CALL(glEnableVertexAttribArray(1));
		GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float))));

		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * v_size, vertices, GL_STREAM_DRAW));
		GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo));
		GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * i_size, indices, GL_STREAM_DRAW));

		GL_CALL(glActiveTexture(GL_TEXTURE0));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, _texture->id));
		GL_CALL(glUniform1i(0, 0));

		_shader->setUniform("u_transform", getFinalTransform(trsf));
		_shader->applyUniforms();
		GL_CALL(glDrawElements(primitivesType, i_size, GL_UNSIGNED_INT, NULL));

		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GL_CALL(glDisableVertexAttribArray(0));
		GL_CALL(glDisableVertexAttribArray(1));
		GL_CALL(glDisableVertexAttribArray(2));

		glDisable(GL_BLEND);
	}
}

void Gfx::draw(Vertex *vertices, int v_size, uint32 *indices, int i_size, Math::Matrix4 trsf) {
	drawPrimitives(GL_TRIANGLES, vertices, v_size, indices, i_size, trsf);
}

void Gfx::drawQuad(Math::Vector2d size, Color color, Math::Matrix4 trsf) {
	float w = size.getX();
	float h = size.getY();
	float x = 0;
	float y = 0;
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

void Gfx::drawSprite(Common::Rect textRect, Texture &texture, Color color, Math::Matrix4 trsf, bool flipX, bool flipY) {
	float l = textRect.left / (float)texture.width;
	float r = textRect.right / (float)texture.width;
	float t = textRect.top / (float)texture.height;
	float b = textRect.bottom / (float)texture.height;
	if (flipX)
		SWAP(l, r);
	if (flipY)
		SWAP(t, b);

	Math::Vector2d pos;
	Vertex vertices[] = {
		{.pos = {pos.getX() + textRect.width(), pos.getY() + textRect.height()}, .texCoords = {r, t}, .color = color},
		{.pos = {pos.getX() + textRect.width(), pos.getY()}, .texCoords = {r, b}, .color = color},
		{.pos = {pos.getX(), pos.getY()}, .texCoords = {l, b}, .color = color},
		{.pos = {pos.getX(), pos.getY() + textRect.height()}, .texCoords = {l, t}, .color = color}};
	_texture = &texture;
	GL_CALL(glBindTexture(GL_TEXTURE_2D, texture.id));
	uint32 quadIndices[] = {
		0, 1, 3,
		1, 2, 3};
	draw(vertices, 4, quadIndices, 6, trsf);
}

void Gfx::camera(Math::Vector2d size) {
	_cameraSize = size;
	_mvp = ortho(0.f, size.getX(), 0.f, size.getY(), -1.f, 1.f);
}

void Gfx::use(Shader *shader) {
	_shader = shader ? shader : &_defaultShader;
}
} // namespace Twp
