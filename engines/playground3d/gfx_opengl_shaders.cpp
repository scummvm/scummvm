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

#include "common/rect.h"
#include "common/textconsole.h"

#if defined(USE_OPENGL_SHADERS)

#include "graphics/surface.h"

#include "math/glmath.h"
#include "math/vector2d.h"
#include "math/rect2d.h"
#include "math/quat.h"

#include "graphics/opengl/shader.h"

#include "engines/playground3d/gfx.h"
#include "engines/playground3d/gfx_opengl_shaders.h"

namespace Playground3d {

static const GLfloat offsetVertices[] = {
	//  X      Y
	// 1st triangle
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	 0.0f, -1.0f,
	// 2nd triangle
	-0.5f,  0.5f,
	 0.5f,  0.5f,
	 0.0f, -0.5f,
};

static const GLfloat dimRegionVertices[] = {
	//  X      Y
	-0.5f,  0.5f,
	 0.5f,  0.5f,
	-0.5f, -0.5f,
	 0.5f, -0.5f,
};

static const GLfloat boxVertices[] = {
	//  X      Y
	// static green box
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f,
	// moving red box
	-0.1f,  0.1f,
	 0.1f,  0.1f,
	-0.1f, -0.1f,
	 0.1f, -0.1f,
};

static const GLfloat bitmapVertices[] = {
	//  X      Y     S     T
	-0.2f,  0.2f, 0.0f, 0.0f,
	 0.2f,  0.2f, 1.0f, 0.0f,
	-0.2f, -0.2f, 0.0f, 1.0f,
	 0.2f, -0.2f, 1.0f, 1.0f,
};

Renderer *CreateGfxOpenGLShader(OSystem *system) {
	return new ShaderRenderer(system);
}

ShaderRenderer::ShaderRenderer(OSystem *system) :
		Renderer(system),
		_currentViewport(kOriginalWidth, kOriginalHeight),
		_cubeShader(nullptr),
		_offsetShader(nullptr),
		_fadeShader(nullptr),
		_viewportShader(nullptr),
		_bitmapShader(nullptr),
		_cubeVBO(0),
		_offsetVBO(0),
		_fadeVBO(0),
		_viewportVBO(0),
		_bitmapVBO(0) {
}

ShaderRenderer::~ShaderRenderer() {
	OpenGL::Shader::freeBuffer(_cubeVBO);
	OpenGL::Shader::freeBuffer(_offsetVBO);
	OpenGL::Shader::freeBuffer(_fadeVBO);
	OpenGL::Shader::freeBuffer(_viewportVBO);
	OpenGL::Shader::freeBuffer(_bitmapVBO);

	delete _cubeShader;
	delete _offsetShader;
	delete _fadeShader;
	delete _viewportShader;
	delete _bitmapShader;
}

void ShaderRenderer::init() {
	debug("Initializing OpenGL Renderer with shaders");

	computeScreenViewport();

	static const char *cubeAttributes[] = { "position", "normal", "color", "texcoord", nullptr };
	_cubeShader = OpenGL::Shader::fromFiles("playground3d_cube", cubeAttributes);
	_cubeVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices);
	_cubeShader->enableVertexAttribute("texcoord", _cubeVBO, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
	_cubeShader->enableVertexAttribute("position", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 8);
	_cubeShader->enableVertexAttribute("normal", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 20);
	_cubeShader->enableVertexAttribute("color", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 32);

	static const char *offsetAttributes[] = { "position", nullptr };
	_offsetShader = OpenGL::Shader::fromFiles("playground3d_offset", offsetAttributes);
	_offsetVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(offsetVertices), offsetVertices);
	_offsetShader->enableVertexAttribute("position", _offsetVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	static const char *fadeAttributes[] = { "position", nullptr };
	_fadeShader = OpenGL::Shader::fromFiles("playground3d_fade", fadeAttributes);
	_fadeVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(dimRegionVertices), dimRegionVertices);
	_fadeShader->enableVertexAttribute("position", _fadeVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	static const char *viewportAttributes[] = { "position", nullptr };
	_viewportShader = OpenGL::Shader::fromFiles("playground3d_viewport", viewportAttributes);
	_viewportVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices);
	_viewportShader->enableVertexAttribute("position", _viewportVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	static const char *bitmapAttributes[] = { "position", "texcoord", nullptr };
	_bitmapShader = OpenGL::Shader::fromFiles("playground3d_bitmap", bitmapAttributes);
	_bitmapVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(bitmapVertices), bitmapVertices);
	_bitmapShader->enableVertexAttribute("position", _bitmapVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 0);
	_bitmapShader->enableVertexAttribute("texcoord", _bitmapVBO, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 8);

	glGenTextures(5, _textureRgbaId);
	glGenTextures(5, _textureRgbId);
	glGenTextures(2, _textureRgb565Id);
	glGenTextures(2, _textureRgba5551Id);
	glGenTextures(2, _textureRgba4444Id);
}

void ShaderRenderer::deinit() {
	glDeleteTextures(5, _textureRgbaId);
	glDeleteTextures(5, _textureRgbId);
	glDeleteTextures(2, _textureRgb565Id);
	glDeleteTextures(2, _textureRgba5551Id);
	glDeleteTextures(2, _textureRgba4444Id);
}

void ShaderRenderer::clear(const Math::Vector4d &clearColor) {
	glClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ShaderRenderer::loadTextureRGBA(Graphics::Surface *texture) {
	glBindTexture(GL_TEXTURE_2D, _textureRgbaId[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->getPixels());
	glBindTexture(GL_TEXTURE_2D, _textureRgbaId[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture->getPixels());
}

void ShaderRenderer::loadTextureRGB(Graphics::Surface *texture) {
	glBindTexture(GL_TEXTURE_2D, _textureRgbId[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->getPixels());
}

void ShaderRenderer::loadTextureRGB565(Graphics::Surface *texture) {
	glBindTexture(GL_TEXTURE_2D, _textureRgb565Id[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture->getPixels());
}

void ShaderRenderer::loadTextureRGBA5551(Graphics::Surface *texture) {
	glBindTexture(GL_TEXTURE_2D, _textureRgba5551Id[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, texture->getPixels());
}

void ShaderRenderer::loadTextureRGBA4444(Graphics::Surface *texture) {
	glBindTexture(GL_TEXTURE_2D, _textureRgba4444Id[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, texture->getPixels());
}

void ShaderRenderer::setupViewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
}

void ShaderRenderer::enableFog(const Math::Vector4d &fogColor) {
}

void ShaderRenderer::drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_TEXTURE_2D);

	auto rotateMatrix = (Math::Quaternion::fromEuler(roll.x(), roll.y(), roll.z(), Math::EO_XYZ)).inverse().toMatrix();
	_cubeShader->use();
	_cubeShader->setUniform("textured", false);
	_cubeShader->setUniform("mvpMatrix", _mvpMatrix);
	_cubeShader->setUniform("rotateMatrix", rotateMatrix);
	_cubeShader->setUniform("modelPos", pos);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
}

void ShaderRenderer::drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_TEXTURE_2D);

	auto rotateMatrix = (Math::Quaternion::fromEuler(roll.x(), roll.y(), roll.z(), Math::EO_XYZ)).inverse().toMatrix();
	_offsetShader->use();
	_offsetShader->setUniform("mvpMatrix", _mvpMatrix);
	_offsetShader->setUniform("rotateMatrix", rotateMatrix);
	_offsetShader->setUniform("modelPos", pos);

	_offsetShader->setUniform("triColor", Math::Vector3d(0.0f, 1.0f, 0.0f));
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glPolygonOffset(-1.0f, 0.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	_offsetShader->setUniform("triColor", Math::Vector3d(1.0f, 1.0f, 1.0f));
	glDrawArrays(GL_TRIANGLES, 3, 3);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void ShaderRenderer::dimRegionInOut(float fade) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_TEXTURE_2D);

	_fadeShader->use();
	_fadeShader->setUniform1f("alphaLevel", 1.0 - fade);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	_fadeShader->unbind();
}

void ShaderRenderer::drawInViewport() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_TEXTURE_2D);

	_viewportShader->use();
	_viewportShader->setUniform("offset", Math::Vector2d(0.0f, 0.0f));
	_viewportShader->setUniform("color", Math::Vector3d(0.0f, 1.0f, 0.0f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_pos.setX(_pos.getX() + 0.01f);
	_pos.setY(_pos.getY() + 0.01f);
	if (_pos.getX() >= 1.1f) {
		_pos.setX(-1.1f);
		_pos.setY(-1.1f);
	}

	_viewportShader->setUniform("offset", _pos);
	_viewportShader->setUniform("color", Math::Vector3d(1.0f, 0.0f, 0.0f));
	glPolygonOffset(-1.0f, 0.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDisable(GL_POLYGON_OFFSET_FILL);
	_viewportShader->unbind();
}

void ShaderRenderer::drawRgbaTexture() {
	Math::Vector2d offset;
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	_bitmapShader->use();

	offset.setX(-0.8f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", offset);
	glBindTexture(GL_TEXTURE_2D, _textureRgbaId[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	offset.setX(-0.3f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", offset);
	glBindTexture(GL_TEXTURE_2D, _textureRgbId[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	offset.setX(0.2f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", offset);
	glBindTexture(GL_TEXTURE_2D, _textureRgb565Id[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	offset.setX(0.7f);
	offset.setY(0.8f);
	_bitmapShader->setUniform("offsetXY", offset);
	glBindTexture(GL_TEXTURE_2D, _textureRgba5551Id[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	offset.setX(-0.8f);
	offset.setY(0.2f);
	_bitmapShader->setUniform("offsetXY", offset);
	glBindTexture(GL_TEXTURE_2D, _textureRgba4444Id[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_bitmapShader->unbind();
}

} // End of namespace Playground3d

#endif
