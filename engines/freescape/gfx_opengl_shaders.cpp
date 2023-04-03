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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "common/config-manager.h"
#include "common/math.h"
#include "common/system.h"
#include "math/glmath.h"

#include "freescape/objects/object.h"
#include "freescape/gfx_opengl_shaders.h"
#include "freescape/gfx_opengl_texture.h"

namespace Freescape {

static const GLfloat bitmapVertices[] = {
	// XS   YT
	0.0, 0.0,
	1.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
};

Renderer *CreateGfxOpenGLShader(int screenW, int screenH, Common::RenderMode renderMode) {
	return new OpenGLShaderRenderer(screenW, screenH, renderMode);
}

OpenGLShaderRenderer::OpenGLShaderRenderer(int screenW, int screenH, Common::RenderMode renderMode) : Renderer(screenW, screenH, renderMode) {
	_verts = nullptr;
	_triangleShader = nullptr;
	_triangleVBO = 0;

	_bitmapShader = nullptr;
	_bitmapVBO = 0;

	_texturePixelFormat = OpenGLTexture::getRGBAPixelFormat();
	_isAccelerated = true;
}

OpenGLShaderRenderer::~OpenGLShaderRenderer() {
	OpenGL::Shader::freeBuffer(_triangleVBO);
	delete _triangleShader;
	OpenGL::Shader::freeBuffer(_bitmapVBO);
	delete _bitmapShader;
	free(_verts);
}

Texture *OpenGLShaderRenderer::createTexture(const Graphics::Surface *surface) {
	return new OpenGLTexture(surface);
}

void OpenGLShaderRenderer::freeTexture(Texture *texture) {
	delete texture;
}

void OpenGLShaderRenderer::init() {
	computeScreenViewport();

	_verts = (Vertex *)malloc(sizeof(Vertex) * kVertexArraySize);

	static const char *triangleAttributes[] = { "position", nullptr };
	_triangleShader = OpenGL::Shader::fromFiles("freescape_triangle", triangleAttributes);
	_triangleVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(Vertex) * kVertexArraySize, _verts, GL_DYNAMIC_DRAW);
	// TODO: Check if 3 * sizeof(float) == sizeof(Vertex)
	_triangleShader->enableVertexAttribute("position", _triangleVBO, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	static const char *bitmapAttributes[] = { "position", "texcoord", nullptr };
	_bitmapShader = OpenGL::Shader::fromFiles("freescape_bitmap", bitmapAttributes);
	_bitmapVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(bitmapVertices), bitmapVertices);
	_bitmapShader->enableVertexAttribute("position", _bitmapVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	_bitmapShader->enableVertexAttribute("texcoord", _bitmapVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	setViewport(_viewport);
}

void OpenGLShaderRenderer::setViewport(const Common::Rect &rect) {
	_viewport = Common::Rect(
					_screenViewport.width() * rect.width() / _screenW,
					_screenViewport.height() * rect.height() / _screenH
					);

	_viewport.translate(
					_screenViewport.left + _screenViewport.width() * rect.left / _screenW,
					_screenViewport.top + _screenViewport.height() * rect.top / _screenH
					);

	_unscaledViewport = rect;
	glViewport(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
	glScissor(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
}

void OpenGLShaderRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	_bitmapShader->use();
	_bitmapShader->setUniform("flipY", glTexture->_upsideDown);

	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, glTexture->_id);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	_bitmapShader->unbind();
}

void OpenGLShaderRenderer::updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) {
	float aspectRatio = _screenW / (float)_screenH;
	float xmaxValue = nearClipPlane * tan(Common::deg2rad(fov) / 2);
	float ymaxValue = xmaxValue / aspectRatio;
	_projectionMatrix = Math::makeFrustumMatrix(xmaxValue, -xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
}

void OpenGLShaderRenderer::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) {
	Math::Vector3d up_vec(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	Math::Matrix4 viewMatrix;
	viewMatrix.translate(-pos);
    viewMatrix.transpose();

	_modelViewMatrix = viewMatrix * lookMatrix;

	Math::Matrix4 proj = _projectionMatrix;
	Math::Matrix4 model = _modelViewMatrix;
	proj.transpose();
	model.transpose();
	_mvpMatrix = proj * model;
	_mvpMatrix.transpose();
}

void OpenGLShaderRenderer::renderSensorShoot(byte color, const Math::Vector3d sensor, const Math::Vector3d player, const Common::Rect viewArea) {
	/*glColor3ub(255, 255, 255);
	glLineWidth(20);
	polygonOffset(true);
	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, sensor);
	copyToVertexArray(1, player);
	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);
	polygonOffset(false);
	glLineWidth(1);*/
}

// TODO: move inside the shader?
float remap(float f, float s) {
	return 2. * f / s - 1;
}

void OpenGLShaderRenderer::renderPlayerShoot(byte color, const Common::Point position, const Common::Rect viewArea) {
	uint8 a, r, g, b;

	Math::Matrix4 identity;
	identity(0, 0) = 1.0;
	identity(1, 1) = 1.0;
	identity(2, 2) = 1.0;
	identity(3, 3) = 1.0;

	_triangleShader->use();
	_triangleShader->setUniform("mvpMatrix", identity);

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderZX) {
		r = g = b = 255;
	} else {
		uint32 pixel = 0x0;
		glReadPixels(g_system->getWidth() / 2, g_system->getHeight() / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
		_texturePixelFormat.colorToARGB(pixel, a, r, g, b);
		color = indexFromColor(r, g, b);
		readFromPalette((color + 3) % (_renderMode == Common::kRenderCGA ? 4 : 16), r, g, b);
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	useColor(r, g, b);

	glLineWidth(5); // It will not work in every OpenGL implementation since the
					 // spec doesn't require support for line widths other than 1
	copyToVertexArray(0, Math::Vector3d(remap(viewArea.left, _screenW), remap(viewArea.height() - viewArea.top,  _screenH), 0));
	copyToVertexArray(1, Math::Vector3d(remap(position.x,  _screenW), remap(_screenH - position.y, _screenH), 0));

	copyToVertexArray(2, Math::Vector3d(remap(viewArea.left, _screenW), remap(viewArea.height() - viewArea.top + 3, _screenH), 0));
	copyToVertexArray(3, Math::Vector3d(remap(position.x, _screenW), remap(_screenH - position.y, _screenH), 0));

	copyToVertexArray(4, Math::Vector3d(remap(viewArea.right, _screenW), remap(_screenH - viewArea.bottom, _screenH), 0));
	copyToVertexArray(5, Math::Vector3d(remap(position.x,  _screenW), remap(_screenH - position.y, _screenH), 0));

	copyToVertexArray(6, Math::Vector3d(remap(viewArea.right, _screenW), remap(_screenH - viewArea.bottom + 3, _screenH), 0));
	copyToVertexArray(7, Math::Vector3d(remap(position.x,  _screenW), remap(_screenH - position.y, _screenH), 0));

	glBindBuffer(GL_ARRAY_BUFFER, _triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), _verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glDrawArrays(GL_LINES, 0, 8);

	glLineWidth(1);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void OpenGLShaderRenderer::renderFace(const Common::Array<Math::Vector3d> &vertices) {
	assert(vertices.size() >= 2);
	const Math::Vector3d &v0 = vertices[0];

	_triangleShader->use();
	_triangleShader->setUniform("mvpMatrix", _mvpMatrix);

	if (vertices.size() == 2) {
		const Math::Vector3d &v1 = vertices[1];
		if (v0 == v1)
			return;

		copyToVertexArray(0, v0);
		copyToVertexArray(1, v1);

		glLineWidth(MAX(1, g_system->getWidth() / 192));

		glBindBuffer(GL_ARRAY_BUFFER, _triangleVBO);
		glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(float), _verts, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glDrawArrays(GL_LINES, 0, 2);

		glLineWidth(1);
		return;
	}

	uint vi = 0;
	for (uint i = 1; i < vertices.size() - 1; i++) { // no underflow since vertices.size() > 2
		const Math::Vector3d &v1 = vertices[i];
		const Math::Vector3d &v2 = vertices[i + 1];
		vi = 3 * (i - 1); // no underflow since i >= 1
		copyToVertexArray(vi + 0, v0);
		copyToVertexArray(vi + 1, v1);
		copyToVertexArray(vi + 2, v2);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, (vi + 3) * 3 * sizeof(float), _verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	glDrawArrays(GL_TRIANGLES, 0, vi + 3);
}

void OpenGLShaderRenderer::polygonOffset(bool enabled) {
	if (enabled) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-10.0f, 1.0f);
	} else {
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void OpenGLShaderRenderer::setStippleData(byte *data) {
	if (!data)
		return;

	//_variableStippleArray = data;
	//for (int i = 0; i < 128; i++)
	//	_variableStippleArray[i] = data[(i / 16) % 4];
}

void OpenGLShaderRenderer::useStipple(bool enabled) {
	/*if (enabled) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f, -1.0f);
		glEnable(GL_POLYGON_STIPPLE);
		if (_renderMode == Common::kRenderZX  ||
			_renderMode == Common::kRenderCPC ||
			_renderMode == Common::kRenderCGA)
			glPolygonStipple(_variableStippleArray);
		else
			glPolygonStipple(_defaultStippleArray);
	} else {
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_STIPPLE);
	}*/
}

void OpenGLShaderRenderer::useColor(uint8 r, uint8 g, uint8 b) {
	Math::Vector3d color(r / 256.0, g / 256.0, b / 256.0);
	_triangleShader->use();
	_triangleShader->setUniform("color", color);
}

void OpenGLShaderRenderer::clear(uint8 color) {
	uint8 r, g, b;

	if (_colorRemaps && _colorRemaps->contains(color)) {
		color = (*_colorRemaps)[color];
	}

	readFromPalette(color, r, g, b);
	glClearColor(0, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLShaderRenderer::drawFloor(uint8 color) {
	/*uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple;
	assert(getRGBAt(color, r1, g1, b1, r2, g2, b2, stipple)); // TODO: move check inside this function
	glColor3ub(r1, g1, b1);

	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(-100000.0, 0.0, -100000.0));
	copyToVertexArray(1, Math::Vector3d(100000.0, 0.0, -100000.0));
	copyToVertexArray(2, Math::Vector3d(100000.0, 0.0, 100000.0));
	copyToVertexArray(3, Math::Vector3d(-100000.0, 0.0, 100000.0));
	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);*/
}

void OpenGLShaderRenderer::flipBuffer() {}

Graphics::Surface *OpenGLShaderRenderer::getScreenshot() {
	Common::Rect screen = viewport();
	Graphics::Surface *s = new Graphics::Surface();
	s->create(screen.width(), screen.height(), OpenGLTexture::getRGBAPixelFormat());
	glReadPixels(screen.left, screen.top, screen.width(), screen.height(), GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());
	flipVertical(s);
	return s;
	return nullptr;
}

} // End of namespace Freescape
