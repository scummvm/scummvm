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

Renderer *CreateGfxOpenGLShader(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics) {
	return new OpenGLShaderRenderer(screenW, screenH, renderMode, authenticGraphics);
}

OpenGLShaderRenderer::OpenGLShaderRenderer(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics) : Renderer(screenW, screenH, renderMode, authenticGraphics) {
	_verts = nullptr;
	_triangleShader = nullptr;
	_triangleVBO = 0;

	_bitmapShader = nullptr;
	_bitmapVBO = 0;

	_texturePixelFormat = getRGBAPixelFormat();
	_isAccelerated = true;
}

OpenGLShaderRenderer::~OpenGLShaderRenderer() {
	OpenGL::Shader::freeBuffer(_triangleVBO);
	delete _triangleShader;
	OpenGL::Shader::freeBuffer(_bitmapVBO);
	delete _bitmapShader;
	free(_verts);
}

Texture *OpenGLShaderRenderer::createTexture(const Graphics::Surface *surface, bool is3D) {
	return new OpenGLTexture(surface);
}

void OpenGLShaderRenderer::freeTexture(Texture *texture) {
	delete texture;
}

Common::Point OpenGLShaderRenderer::nativeResolution() {
	GLint vect[4];
	glGetIntegerv(GL_VIEWPORT, vect);
	return Common::Point(vect[2], vect[3]);
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

	// populate default stipple data for shader rendering
	for(int i = 0; i < 128; i++)
		_defaultShaderStippleArray[i] = _defaultStippleArray[i];

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

void OpenGLShaderRenderer::updateProjectionMatrix(float fov, float aspectRatio, float nearClipPlane, float farClipPlane) {
	float xmaxValue = nearClipPlane * tan(Math::deg2rad(fov) / 2);
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
void OpenGLShaderRenderer::renderSensorShoot(byte color, const Math::Vector3d sensor, const Math::Vector3d target, const Common::Rect viewArea) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	useColor(255, 255, 255);

	glLineWidth(20);
	copyToVertexArray(0, sensor);
	copyToVertexArray(1, target);

	glBindBuffer(GL_ARRAY_BUFFER, _triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), _verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	glDrawArrays(GL_LINES, 0, 2);
	glLineWidth(1);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

// TODO: move inside the shader?
float remap(float f, float s) {
	return 2. * f / s - 1;
}

void OpenGLShaderRenderer::renderPlayerShootBall(byte color, const Common::Point _position, int frame, const Common::Rect viewArea) {
	uint8 r, g, b;

	Math::Matrix4 identity;
	identity(0, 0) = 1.0;
	identity(1, 1) = 1.0;
	identity(2, 2) = 1.0;
	identity(3, 3) = 1.0;

	_triangleShader->use();
	_triangleShader->setUniform("useStipple", false);
	_triangleShader->setUniform("mvpMatrix", identity);

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderZX) {
		r = g = b = 255;
	} else {
		r = g = b = 255;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	useColor(r, g, b);

	int triangleAmount = 20;
	float twicePi = (float)(2.0 * M_PI);
	float coef = (9 - frame) / 9.0;
	float radius = (1 - coef) * 4.0;

	Common::Point position(_position.x, _screenH - _position.y);

	Common::Point initial_position(viewArea.left + viewArea.width() / 2 + 2, _screenH - (viewArea.height() + viewArea.top));
	Common::Point ball_position = coef * position + (1 - coef) * initial_position;

	copyToVertexArray(0, Math::Vector3d(remap(ball_position.x, _screenW), remap(ball_position.y, _screenH), 0));

	for(int i = 0; i <= triangleAmount; i++) {
		float x = remap(ball_position.x + (radius * cos(i *  twicePi / triangleAmount)), _screenW);
		float y = remap(ball_position.y + (radius * sin(i * twicePi / triangleAmount)), _screenH);
		copyToVertexArray(i + 1, Math::Vector3d(x, y, 0));
	}

	glBindBuffer(GL_ARRAY_BUFFER, _triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, (triangleAmount + 2) * 3 * sizeof(float), _verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glDrawArrays(GL_TRIANGLE_FAN, 0, (triangleAmount + 2));

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void OpenGLShaderRenderer::renderPlayerShootRay(byte color, const Common::Point position, const Common::Rect viewArea) {
	uint8 r, g, b;

	Math::Matrix4 identity;
	identity(0, 0) = 1.0;
	identity(1, 1) = 1.0;
	identity(2, 2) = 1.0;
	identity(3, 3) = 1.0;

	_triangleShader->use();
	_triangleShader->setUniform("useStipple", false);
	_triangleShader->setUniform("mvpMatrix", identity);

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderZX) {
		r = g = b = 255;
	} else {
		r = g = b = 255;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
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

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void OpenGLShaderRenderer::drawCelestialBody(Math::Vector3d position, float radius, byte color) {
	uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple = nullptr;
	getRGBAt(color, 0, r1, g1, b1, r2, g2, b2, stipple);

	useColor(r1, g1, b1);

	int triangleAmount = 20;
	float twicePi = (float)(2.0 * M_PI);
	float adj = 1.25; // Perspective correction

	// Quick billboard effect inspired from this code:
	// http://www.lighthouse3d.com/opengl/billboarding/index.php?billCheat
	/*Math::Matrix4 mvpMatrix = _mvpMatrix;

	for(int i = 2; i < 4; i++)
		for(int j = 2; j < 4; j++ ) {
			if (i == 2)
				continue;
			if (i == j)
				_mvpMatrix.setValue(i, j, 1.0);
			else
				_mvpMatrix.setValue(i, j, 0.0);
		}*/

	_triangleShader->use();
	_triangleShader->setUniform("useStipple", false);
	_triangleShader->setUniform("mvpMatrix", _mvpMatrix);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	copyToVertexArray(0, position);

	for(int i = 0; i <= triangleAmount; i++) {
		float x = position.x();
		float y = position.y() + (radius * cos(i *  twicePi / triangleAmount));
		float z = position.z() + (adj * radius * sin(i * twicePi / triangleAmount));
		copyToVertexArray(i + 1, Math::Vector3d(x, y, z));
	}

	glBindBuffer(GL_ARRAY_BUFFER, _triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, (triangleAmount + 2) * 3 * sizeof(float), _verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glDrawArrays(GL_TRIANGLE_FAN, 0, (triangleAmount + 2));

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	//_mvpMatrix = mvpMatrix;
}

void OpenGLShaderRenderer::renderCrossair(const Common::Point crossairPosition) {
	Math::Matrix4 identity;
	identity(0, 0) = 1.0;
	identity(1, 1) = 1.0;
	identity(2, 2) = 1.0;
	identity(3, 3) = 1.0;

	_triangleShader->use();
	_triangleShader->setUniform("useStipple", false);
	_triangleShader->setUniform("mvpMatrix", identity);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	useColor(255, 255, 255);

	glLineWidth(MAX(2, g_system->getWidth() / 192)); // It will not work in every OpenGL implementation since the
					 // spec doesn't require support for line widths other than 1

	copyToVertexArray(0, Math::Vector3d(remap(crossairPosition.x - 3, _screenW), remap(_screenH - crossairPosition.y, _screenH), 0));
	copyToVertexArray(1, Math::Vector3d(remap(crossairPosition.x - 1, _screenW), remap(_screenH - crossairPosition.y, _screenH), 0));

	copyToVertexArray(2, Math::Vector3d(remap(crossairPosition.x + 1, _screenW), remap(_screenH - crossairPosition.y, _screenH), 0));
	copyToVertexArray(3, Math::Vector3d(remap(crossairPosition.x + 3, _screenW), remap(_screenH - crossairPosition.y, _screenH), 0));

	copyToVertexArray(4, Math::Vector3d(remap(crossairPosition.x, _screenW), remap(_screenH - crossairPosition.y - 3, _screenH), 0));
	copyToVertexArray(5, Math::Vector3d(remap(crossairPosition.x, _screenW), remap(_screenH - crossairPosition.y - 1, _screenH), 0));

	copyToVertexArray(6, Math::Vector3d(remap(crossairPosition.x, _screenW), remap(_screenH - crossairPosition.y + 1, _screenH), 0));
	copyToVertexArray(7, Math::Vector3d(remap(crossairPosition.x, _screenW), remap(_screenH - crossairPosition.y + 3, _screenH), 0));

	glBindBuffer(GL_ARRAY_BUFFER, _triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), _verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glDrawArrays(GL_LINES, 0, 8);

	glLineWidth(1);
	glDisable(GL_BLEND);
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

void OpenGLShaderRenderer::depthTesting(bool enabled) {
	if (enabled) {
		// If we re-enable depth testing, we need to clear the depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
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
		data = _defaultStippleArray;

	for (int i = 0; i < 128; i++)
		_variableStippleArray[i] = data[i];
}

void OpenGLShaderRenderer::useStipple(bool enabled) {
	_triangleShader->use();
	_triangleShader->setUniform("useStipple", enabled);

	if (enabled) {
		GLfloat factor = 0;
		glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(factor - 0.5f, -1.0f);
		if (_renderMode == Common::kRenderZX    ||
			_renderMode == Common::kRenderCPC   ||
			_renderMode == Common::kRenderCGA   ||
			_renderMode == Common::kRenderHercG)
			_triangleShader->setUniform("stipple", 128, _variableStippleArray);
		else
			_triangleShader->setUniform("stipple", 128, _defaultShaderStippleArray);
	} else {
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void OpenGLShaderRenderer::useColor(uint8 r, uint8 g, uint8 b) {
	Math::Vector3d color(r / 256.0, g / 256.0, b / 256.0);
	_triangleShader->use();
	_triangleShader->setUniform("color", color);
}

void OpenGLShaderRenderer::clear(uint8 r, uint8 g, uint8 b, bool ignoreViewport) {
	if (ignoreViewport)
		glDisable(GL_SCISSOR_TEST);
	glClearColor(r / 255., g / 255., b / 255., 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (ignoreViewport)
		glEnable(GL_SCISSOR_TEST);
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
	s->create(screen.width(), screen.height(), getRGBAPixelFormat());
	glReadPixels(screen.left, screen.top, screen.width(), screen.height(), GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());
	flipVertical(s);
	return s;
}

} // End of namespace Freescape
