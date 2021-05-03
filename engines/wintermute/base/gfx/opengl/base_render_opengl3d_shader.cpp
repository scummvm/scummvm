/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/wintermute/ad/ad_block.h"
#include "engines/wintermute/ad/ad_generic.h"
#include "engines/wintermute/ad/ad_walkplane.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/3ds/camera3d.h"
#include "graphics/opengl/system_headers.h"
#include "math/glmath.h"

#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d_shader.h"
#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl_shader.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl_shader.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume_opengl_shader.h"

namespace Wintermute {
BaseRenderer3D *makeOpenGL3DShaderRenderer(BaseGame *inGame) {
	return new BaseRenderOpenGL3DShader(inGame);
}

#include "common/pack-start.h"

struct SpriteVertexShader {
	float x;
	float y;
	float u;
	float v;
	float r;
	float g;
	float b;
	float a;
} PACKED_STRUCT;

#include "common/pack-end.h"

BaseRenderOpenGL3DShader::BaseRenderOpenGL3DShader(BaseGame *inGame)
	: BaseRenderer3D(inGame), _spriteBatchMode(false), _flatShadowMaskShader(nullptr) {
	(void)_spriteBatchMode; // silence warning
}

BaseRenderOpenGL3DShader::~BaseRenderOpenGL3DShader() {
	glDeleteBuffers(1, &_spriteVBO);
	glDeleteTextures(1, &_flatShadowRenderTexture);
	glDeleteRenderbuffers(1, &_flatShadowDepthBuffer);
	glDeleteFramebuffers(1, &_flatShadowFrameBuffer);
}

void BaseRenderOpenGL3DShader::setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode) {
	switch (blendMode) {
	case Graphics::BLEND_NORMAL:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case Graphics::BLEND_ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;

	case Graphics::BLEND_SUBTRACTIVE:
		// wme3d takes the color value here
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		break;

	default:
		warning("BaseRenderOpenGL3DShader::setSpriteBlendMode unsupported blend mode %i", blendMode);
	}
}

void BaseRenderOpenGL3DShader::setAmbientLight() {
	byte a = RGBCOLGetA(_ambientLightColor);
	byte r = RGBCOLGetR(_ambientLightColor);
	byte g = RGBCOLGetG(_ambientLightColor);
	byte b = RGBCOLGetB(_ambientLightColor);

	if (!_overrideAmbientLightColor) {
		uint32 color = _gameRef->getAmbientLightColor();

		a = RGBCOLGetA(color);
		r = RGBCOLGetR(color);
		g = RGBCOLGetG(color);
		b = RGBCOLGetB(color);
	}

	Math::Vector4d value;
	value.x() = r / 255.0f;
	value.y() = g / 255.0f;
	value.z() = b / 255.0f;
	value.w() = a / 255.0f;

	_modelXShader->use();
	_modelXShader->setUniform("ambientLight", value);
}

int BaseRenderOpenGL3DShader::maximumLightsCount() {
	return 8;
}

void BaseRenderOpenGL3DShader::enableLight(int index) {
	_modelXShader->use();
	Common::String uniform = Common::String::format("lights[%i].enabled", index);
	_modelXShader->setUniform1f(uniform.c_str(), 1.0f);
}

void BaseRenderOpenGL3DShader::disableLight(int index) {
	_modelXShader->use();
	Common::String uniform = Common::String::format("lights[%i].enabled", index);
	_modelXShader->setUniform1f(uniform.c_str(), -1.0f);
}

void BaseRenderOpenGL3DShader::setLightParameters(int index, const Math::Vector3d &position, const Math::Vector3d &direction, const Math::Vector4d &diffuse, bool spotlight) {
	Math::Vector4d position4d;
	position4d.x() = position.x();
	position4d.y() = position.y();
	position4d.z() = position.z();
	position4d.w() = 1.0f;

	Math::Vector4d direction4d;
	direction4d.x() = direction.x();
	direction4d.y() = direction.y();
	direction4d.z() = direction.z();
	direction4d.w() = 0.0f;

	if (spotlight) {
		direction4d.w() = -1.0f;
	}

	_modelXShader->use();

	Common::String uniform = Common::String::format("lights[%i]._position", index);
	_modelXShader->setUniform(uniform.c_str(), position4d);

	uniform = Common::String::format("lights[%i]._direction", index);
	_modelXShader->setUniform(uniform.c_str(), direction4d);

	uniform = Common::String::format("lights[%i]._color", index);
	_modelXShader->setUniform(uniform.c_str(), diffuse);
}

void BaseRenderOpenGL3DShader::enableCulling() {
	glEnable(GL_CULL_FACE);
}

void BaseRenderOpenGL3DShader::disableCulling() {
	glDisable(GL_CULL_FACE);
}

bool BaseRenderOpenGL3DShader::enableShadows() {
	if (_flatShadowMaskShader == nullptr) {
		_flatShadowColor = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.5f);

		_shadowTextureWidth = 512;
		_shadowTextureHeight = 512;

		float nearPlane = 1.0f;
		float farPlane = 10000.0f;
		float fovy = M_PI / 4.0f;

		float top = nearPlane *  tanf(fovy * 0.5f);
		float bottom = -top;
		float right = top;
		float left = -right;

		float deltaX = (-0.5f * (right - left)) / _shadowTextureWidth;
		float deltaY = (0.5f * (top - bottom)) / _shadowTextureHeight;

		Math::Matrix4 lightProjection = Math::makeFrustumMatrix(left + deltaX, right + deltaX, bottom + deltaY, top + deltaY, nearPlane, farPlane);

		_flatShadowModelXShader->use();
		_flatShadowModelXShader->setUniform("projMatrix", lightProjection);

		glGenTextures(1, &_flatShadowRenderTexture);
		glBindTexture(GL_TEXTURE_2D, _flatShadowRenderTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _shadowTextureWidth, _shadowTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenRenderbuffers(1, &_flatShadowDepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _flatShadowDepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _shadowTextureWidth, _shadowTextureHeight);

		glGenFramebuffers(1, &_flatShadowFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _flatShadowFrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _flatShadowRenderTexture, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _flatShadowDepthBuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		float flatShadowMaskVertices[12];
		flatShadowMaskVertices[0] = -250.0f;
		flatShadowMaskVertices[1] = 0.0f;
		flatShadowMaskVertices[2] = -250.0f;

		flatShadowMaskVertices[3] = -250.0f;
		flatShadowMaskVertices[4] = 0.0f;
		flatShadowMaskVertices[5] = 250.0f;

		flatShadowMaskVertices[6] = 250.0f;
		flatShadowMaskVertices[7] = 0.0f;
		flatShadowMaskVertices[8] = -250.0f;

		flatShadowMaskVertices[9] = 250.0f;
		flatShadowMaskVertices[10] = 0.0f;
		flatShadowMaskVertices[11] = 250.0f;

		glGenBuffers(1, &_flatShadowMaskVBO);
		glBindBuffer(GL_ARRAY_BUFFER, _flatShadowMaskVBO);
		glBufferData(GL_ARRAY_BUFFER, 4 * 12, flatShadowMaskVertices, GL_STATIC_DRAW);

		static const char *flatShadowMaskAttributes[] = { "position", nullptr };
		_flatShadowMaskShader = OpenGL::ShaderGL::fromFiles("wme_flat_shadow_mask", flatShadowMaskAttributes);
		_flatShadowMaskShader->enableVertexAttribute("position", _flatShadowMaskVBO, 3, GL_FLOAT, false, 12, 0);

		_flatShadowMaskShader->use();
		_flatShadowMaskShader->setUniform("lightProjMatrix", lightProjection);

		_gameRef->_supportsRealTimeShadows = true;
	}

	return true;
}

bool BaseRenderOpenGL3DShader::disableShadows() {
	warning("BaseRenderOpenGL3DShader::disableShadows not implemented yet");
	return true;
}

void BaseRenderOpenGL3DShader::displayShadow(BaseObject *object, const Math::Vector3d &lightPos, bool lightPosRelative) {
	if (_flatShadowMaskShader) {
		if (object->_shadowType <= SHADOW_SIMPLE) {
			// TODO: Display simple shadow here
			return;
		}

		Math::Vector3d position = lightPos;
		Math::Vector3d target = object->_posVector;

		if (lightPosRelative) {
			position = object->_posVector + lightPos;
		}

		Math::Matrix4 lightViewMatrix = Math::makeLookAtMatrix(position, target, Math::Vector3d(0.0f, 1.0f, 0.0f));
		Math::Matrix4 translation;
		translation.setPosition(-position);
		translation.transpose();
		lightViewMatrix = translation * lightViewMatrix;

		_flatShadowModelXShader->use();
		_flatShadowModelXShader->setUniform("viewMatrix", lightViewMatrix);

		Math::Matrix4 tmp = object->_worldMatrix;
		tmp.transpose();
		_flatShadowModelXShader->setUniform("modelMatrix", tmp);

		byte a = RGBCOLGetA(object->_shadowColor);
		byte r = RGBCOLGetR(object->_shadowColor);
		byte g = RGBCOLGetG(object->_shadowColor);
		byte b = RGBCOLGetB(object->_shadowColor);

		_flatShadowColor.x() = r / 255.0f;
		_flatShadowColor.y() = g / 255.0f;
		_flatShadowColor.z() = b / 255.0f;
		_flatShadowColor.w() = a / 255.0f;
		_flatShadowModelXShader->setUniform("shadowColor", _flatShadowColor);

		glBindFramebuffer(GL_FRAMEBUFFER, _flatShadowFrameBuffer);

		GLint currentViewport[4];
		glGetIntegerv(GL_VIEWPORT, currentViewport);
		glViewport(1, 1, _shadowTextureWidth - 2, _shadowTextureHeight - 2);

		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		object->_modelX->renderFlatShadowModel();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);

		glDisable(GL_DEPTH_WRITEMASK);

		Math::Matrix4 shadowPosition;
		shadowPosition.setToIdentity();
		shadowPosition.setPosition(object->_posVector);
		shadowPosition.transpose();

		_flatShadowMaskShader->use();
		_flatShadowMaskShader->setUniform("lightViewMatrix", lightViewMatrix);
		_flatShadowMaskShader->setUniform("worldMatrix", shadowPosition);
		_flatShadowMaskShader->setUniform("viewMatrix", _lastViewMatrix);
		_flatShadowMaskShader->setUniform("projMatrix", _projectionMatrix3d);
		_flatShadowMaskShader->setUniform("shadowColor", _flatShadowColor);

		glBindBuffer(GL_ARRAY_BUFFER, _flatShadowMaskVBO);
		glBindTexture(GL_TEXTURE_2D, _flatShadowRenderTexture);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindTexture(GL_TEXTURE_2D, 0);

		glEnable(GL_DEPTH_WRITEMASK);
	}
}

bool BaseRenderOpenGL3DShader::stencilSupported() {
	// assume that we have a stencil buffer
	return true;
}

BaseImage *BaseRenderOpenGL3DShader::takeScreenshot() {
	warning("BaseRenderOpenGL3DShader::takeScreenshot not yet implemented");
	return nullptr;
}

bool BaseRenderOpenGL3DShader::saveScreenShot(const Common::String &filename, int sizeX, int sizeY) {
	warning("BaseRenderOpenGL3DShader::saveScreenshot not yet implemented");
	return true;
}

void BaseRenderOpenGL3DShader::setWindowed(bool windowed) {
	warning("BaseRenderOpenGL3DShader::setWindowed not yet implemented");
}

void BaseRenderOpenGL3DShader::fadeToColor(byte r, byte g, byte b, byte a) {
	setProjection2D();

	Math::Vector4d color;
	color.x() = r / 255.0f;
	color.y() = g / 255.0f;
	color.z() = b / 255.0f;
	color.w() = a / 255.0f;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, _fadeVBO);

	_fadeShader->use();
	_fadeShader->setUniform("color", color);
	_fadeShader->setUniform("projMatrix", _projectionMatrix2d);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	setup2D(true);
}

bool BaseRenderOpenGL3DShader::fill(byte r, byte g, byte b, Common::Rect *rect) {
	glClearColor(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

bool BaseRenderOpenGL3DShader::setViewport(int left, int top, int right, int bottom) {
	_viewportRect.setRect(left, top, right, bottom);
	glViewport(left, _height - bottom, right - left, bottom - top);
	return true;
}

bool BaseRenderOpenGL3DShader::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);

	float lineCoords[4];

	lineCoords[0] = x1;
	lineCoords[1] = y1;
	lineCoords[2] = x2;
	lineCoords[3] = y2;

	glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * 8, lineCoords);

	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	Math::Vector4d colorValue;
	colorValue.x() = r / 255.0f;
	colorValue.y() = g / 255.0f;
	colorValue.z() = b / 255.0f;
	colorValue.w() = a / 255.0f;

	_lineShader->use();
	_lineShader->setUniform("color", colorValue);
	_fadeShader->setUniform("projMatrix", _projectionMatrix2d);

	glDrawArrays(GL_LINES, 0, 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

bool BaseRenderOpenGL3DShader::drawRect(int x1, int y1, int x2, int y2, uint32 color, int width) {
	warning("BaseRenderOpenGL3DShader::drawRect not yet implemented");
	return true;
}

bool BaseRenderOpenGL3DShader::setProjection() {
	// is the viewport already set here?
	float viewportWidth = _viewportRect.right - _viewportRect.left;
	float viewportHeight = _viewportRect.bottom - _viewportRect.top;

	float verticalViewAngle = _fov;
	float aspectRatio = float(viewportWidth) / float(viewportHeight);

	float scaleMod = float(_height) / float(viewportHeight);

	float top = _nearPlane * tanf(verticalViewAngle * 0.5f);

	_projectionMatrix3d = Math::makeFrustumMatrix(-top * aspectRatio, top * aspectRatio, -top, top, _nearPlane, _farPlane);

	_projectionMatrix3d(0, 0) *= scaleMod;
	_projectionMatrix3d(1, 1) *= scaleMod;
	return true;
}

bool BaseRenderOpenGL3DShader::setProjection2D() {
	float nearPlane = -1.0f;
	float farPlane = 100.0f;

	_projectionMatrix2d.setToIdentity();

	_projectionMatrix2d(0, 0) = 2.0f / _width;
	_projectionMatrix2d(1, 1) = 2.0f / _height;
	_projectionMatrix2d(2, 2) = 2.0f / (farPlane - nearPlane);

	_projectionMatrix2d(3, 0) = -1.0f;
	_projectionMatrix2d(3, 1) = -1.0f;
	_projectionMatrix2d(3, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);

	_shadowMaskShader->use();
	_shadowMaskShader->setUniform("projMatrix", _projectionMatrix2d);
	return true;
}

void BaseRenderOpenGL3DShader::setWorldTransform(const Math::Matrix4 &transform) {
	Math::Matrix4 tmp = transform;
	tmp.transpose();

	Math::Matrix4 newInvertedTranspose = tmp * _lastViewMatrix;
	newInvertedTranspose.inverse();
	newInvertedTranspose.transpose();

	_modelXShader->use();
	_modelXShader->setUniform("modelMatrix", tmp);
	_modelXShader->setUniform("normalMatrix", newInvertedTranspose);

	_shadowVolumeShader->use();
	_shadowVolumeShader->setUniform("modelMatrix", tmp);
}

bool BaseRenderOpenGL3DShader::windowedBlt() {
	warning("BaseRenderOpenGL3DShader::windowedBlt not yet implemented");
	return true;
}

void Wintermute::BaseRenderOpenGL3DShader::onWindowChange() {
	warning("BaseRenderOpenGL3DShader::onWindowChange not yet implemented");
}

bool BaseRenderOpenGL3DShader::initRenderer(int width, int height, bool windowed) {
	glGenBuffers(1, &_spriteVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _spriteVBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SpriteVertexShader), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	static const char *spriteAttributes[] = {"position", "texcoord", "color", nullptr};
	_spriteShader = OpenGL::ShaderGL::fromFiles("wme_sprite", spriteAttributes);

	_spriteShader->enableVertexAttribute("position", _spriteVBO, 2, GL_FLOAT, false, sizeof(SpriteVertexShader), 0);
	_spriteShader->enableVertexAttribute("texcoord", _spriteVBO, 2, GL_FLOAT, false, sizeof(SpriteVertexShader), 8);
	_spriteShader->enableVertexAttribute("color", _spriteVBO, 4, GL_FLOAT, false, sizeof(SpriteVertexShader), 16);

	static const char *geometryAttributes[] = { "position", nullptr };
	_geometryShader = OpenGL::ShaderGL::fromFiles("wme_geometry", geometryAttributes);

	static const char *shadowVolumeAttributes[] = { "position", nullptr };
	_shadowVolumeShader = OpenGL::ShaderGL::fromFiles("wme_shadow_volume", shadowVolumeAttributes);

	static const char *shadowMaskAttributes[] = { "position", nullptr };
	_shadowMaskShader = OpenGL::ShaderGL::fromFiles("wme_shadow_mask", shadowMaskAttributes);

	_transformStack.push_back(Math::Matrix4());
	_transformStack.back().setToIdentity();

	static const char *modelXAttributes[] = {"position", "texcoord", "normal", nullptr};
	_modelXShader = OpenGL::ShaderGL::fromFiles("wme_modelx", modelXAttributes);

	setDefaultAmbientLightColor();

	for (int i = 0; i < maximumLightsCount(); ++i) {
		setLightParameters(i, Math::Vector3d(0, 0, 0), Math::Vector3d(0, 0, 0), Math::Vector4d(0, 0, 0, 0), false);
		disableLight(i);
	}

	_windowed = windowed;
	_width = width;
	_height = height;

	_nearPlane = 90.0f;
	_farPlane = 10000.0f;

	setViewport(0, 0, width, height);

	float fadeVertexCoords[8];

	fadeVertexCoords[0 * 2 + 0] = _viewportRect.left;
	fadeVertexCoords[0 * 2 + 1] = _viewportRect.bottom;
	fadeVertexCoords[1 * 2 + 0] = _viewportRect.left;
	fadeVertexCoords[1 * 2 + 1] = _viewportRect.top;
	fadeVertexCoords[2 * 2 + 0] = _viewportRect.right;
	fadeVertexCoords[2 * 2 + 1] = _viewportRect.bottom;
	fadeVertexCoords[3 * 2 + 0] = _viewportRect.right;
	fadeVertexCoords[3 * 2 + 1] = _viewportRect.top;

	glGenBuffers(1, &_fadeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _fadeVBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * 8, fadeVertexCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	static const char *fadeAttributes[] = { "position", nullptr };
	_fadeShader = OpenGL::ShaderGL::fromFiles("wme_fade", fadeAttributes);

	_fadeShader->enableVertexAttribute("position", _fadeVBO, 2, GL_FLOAT, false, 8, 0);

	glGenBuffers(1, &_lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * 8, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	static const char *lineAttributes[] = { "position", nullptr };
	_lineShader = OpenGL::ShaderGL::fromFiles("wme_line", lineAttributes);
	_lineShader->enableVertexAttribute("position", _lineVBO, 2, GL_FLOAT, false, 8, 0);

	static const char *flatShadowModelXAttributes[] = { "position", nullptr };
	_flatShadowModelXShader = OpenGL::ShaderGL::fromFiles("wme_flat_shadow_modelx", flatShadowModelXAttributes);

	_active = true;
	// setup a proper state
	setup2D(true);
	return true;
}

bool Wintermute::BaseRenderOpenGL3DShader::flip() {
	g_system->updateScreen();
	return true;
}

bool BaseRenderOpenGL3DShader::indicatorFlip() {
	warning("BaseRenderOpenGL3DShader::indicatorFlip not yet implemented");
	return true;
}

bool BaseRenderOpenGL3DShader::forcedFlip() {
	warning("BaseRenderOpenGL3DShader::forcedFlip not yet implemented");
	return true;
}

bool BaseRenderOpenGL3DShader::setup2D(bool force) {
	if (_renderState != RSTATE_2D || force) {
		_renderState = RSTATE_2D;

		// some states are still missing here

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glViewport(0, 0, _width, _height);

		setProjection2D();
	}

	return true;
}

bool BaseRenderOpenGL3DShader::setup3D(Camera3D *camera, bool force) {
	if (_renderState != RSTATE_3D || force) {
		_renderState = RSTATE_3D;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		setAmbientLight();

		if (camera) {
			_fov = camera->_fov;

			if (camera->_nearClipPlane >= 0.0f) {
				_nearPlane = camera->_nearClipPlane;
			}

			if (camera->_farClipPlane >= 0.0f) {
				_farPlane = camera->_farClipPlane;
			}

			Math::Matrix4 viewMatrix;
			camera->getViewMatrix(&viewMatrix);
			Math::Matrix4 cameraTranslate;
			cameraTranslate.setPosition(-camera->_position);
			cameraTranslate.transpose();
			viewMatrix = cameraTranslate * viewMatrix;
			_lastViewMatrix = viewMatrix;
		}

		FogParameters fogParameters;

		_gameRef->getFogParams(fogParameters);

		if (fogParameters._enabled) {
			// TODO: Implement fog
			GLfloat color[4];
			color[0] = RGBCOLGetR(fogParameters._color) / 255.0f;
			color[1] = RGBCOLGetG(fogParameters._color) / 255.0f;
			color[2] = RGBCOLGetB(fogParameters._color) / 255.0f;
			color[3] = RGBCOLGetA(fogParameters._color) / 255.0f;
			debug(5, "BaseRenderOpenGL3DShader::setup3D fog not yet implemented! [%f %f %f %f]", color[0], color[1], color[2], color[3]);
		} else {
			// TODO: Disable fog in shader
		}

		glViewport(_viewportRect.left, _height - _viewportRect.bottom, _viewportRect.width(), _viewportRect.height());
		_viewport3dRect = _viewportRect;

		setProjection();
	}

	_modelXShader->use();
	_modelXShader->setUniform("viewMatrix", _lastViewMatrix);
	_modelXShader->setUniform("projMatrix", _projectionMatrix3d);
	// this is 8 / 255, since 8 is the value used by wme (as a DWORD)
	_modelXShader->setUniform1f("alphaRef", 0.031f);

	_geometryShader->use();
	_geometryShader->setUniform("viewMatrix", _lastViewMatrix);
	_geometryShader->setUniform("projMatrix", _projectionMatrix3d);

	_shadowVolumeShader->use();
	_shadowVolumeShader->setUniform("viewMatrix", _lastViewMatrix);
	_shadowVolumeShader->setUniform("projMatrix", _projectionMatrix3d);

	return true;
}

bool BaseRenderOpenGL3DShader::setupLines() {
	if (_renderState != RSTATE_LINES) {
		_renderState = RSTATE_LINES;

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return true;
}

BaseSurface *Wintermute::BaseRenderOpenGL3DShader::createSurface() {
	return new BaseSurfaceOpenGL3D(_gameRef, this);
}

bool BaseRenderOpenGL3DShader::drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Wintermute::Rect32 &rect,
											const Wintermute::Vector2 &pos, const Wintermute::Vector2 &rot, const Wintermute::Vector2 &scale,
											float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
											bool mirrorX, bool mirrorY) {
	// original wme has a batch mode for sprites, we ignore this for the moment

	if (_forceAlphaColor != 0) {
		color = _forceAlphaColor;
	}

	float width = (rect.right - rect.left) * scale.x;
	float height = (rect.bottom - rect.top) * scale.y;

	glBindTexture(GL_TEXTURE_2D, tex.getTextureName());

	// for sprites we clamp to the edge, to avoid line fragments at the edges
	// this is not done by wme, though
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	int texWidth = tex.getGLTextureWidth();
	int texHeight = tex.getGLTextureHeight();

	float texLeft = (float)rect.left / (float)texWidth;
	float texTop = (float)rect.top / (float)texHeight;
	float texRight = (float)rect.right / (float)texWidth;
	float texBottom = (float)rect.bottom / (float)texHeight;

	float offset = _height / 2.0f;
	float correctedYPos = (pos.y - offset) * -1.0f + offset;

	if (mirrorX) {
		SWAP(texLeft, texRight);
	}

	if (mirrorY) {
		SWAP(texTop, texBottom);
	}

	SpriteVertexShader vertices[4] = {};

	// texture coords
	vertices[0].u = texLeft;
	vertices[0].v = texTop;

	vertices[1].u = texLeft;
	vertices[1].v = texBottom;

	vertices[2].u = texRight;
	vertices[2].v = texTop;

	vertices[3].u = texRight;
	vertices[3].v = texBottom;

	// position coords
	vertices[0].x = pos.x - 0.5f;
	vertices[0].y = correctedYPos - 0.5f;

	vertices[1].x = pos.x - 0.5f;
	vertices[1].y = correctedYPos - height - 0.5f;

	vertices[2].x = pos.x + width - 0.5f;
	vertices[2].y = correctedYPos - 0.5f;

	vertices[3].x = pos.x + width - 0.5f;
	vertices[3].y = correctedYPos - height - 0.5;

	// not exactly sure about the color format, but this seems to work
	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	for (int i = 0; i < 4; ++i) {
		vertices[i].r = r / 255.0f;
		vertices[i].g = g / 255.0f;
		vertices[i].b = b / 255.0f;
		vertices[i].a = a / 255.0f;
	}

	Math::Matrix3 transform;
	transform.setToIdentity();

	if (angle != 0) {
		Vector2 correctedRot(rot.x, (rot.y - offset) * -1.0f + offset);
		transform = build2dTransformation(correctedRot, angle);
		transform.transpose();
	}

	_spriteShader->use();
	_spriteShader->setUniform("alphaTest", !alphaDisable);
	_spriteShader->setUniform("projMatrix", _projectionMatrix2d);
	_spriteShader->setUniform("transform", transform);

	glBindBuffer(GL_ARRAY_BUFFER, _spriteVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(SpriteVertexShader), vertices);

	setSpriteBlendMode(blendMode);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return true;
}

void BaseRenderOpenGL3DShader::renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
												   const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) {
	// don't render scene geometry, as OpenGL ES 2 has no wireframe rendering and we don't have a shader alternative yet
}

void BaseRenderOpenGL3DShader::renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks, const BaseArray<AdGeneric *> &generics, Camera3D *camera) {
	setup3D(camera, true);

	// disable color write
	glBlendFunc(GL_ZERO, GL_ONE);

	glFrontFace(GL_CCW);
	glBindTexture(GL_TEXTURE_2D, 0);

	// render walk planes
	for (uint i = 0; i < planes.size(); i++) {
		if (planes[i]->_active && planes[i]->_receiveShadows) {
			planes[i]->_mesh->render();
		}
	}

	// render blocks
	for (uint i = 0; i < blocks.size(); i++) {
		if (blocks[i]->_active && blocks[i]->_receiveShadows) {
			blocks[i]->_mesh->render();
		}
	}

	// render generic objects
	for (uint i = 0; i < generics.size(); i++) {
		if (generics[i]->_active && generics[i]->_receiveShadows) {
			generics[i]->_mesh->render();
		}
	}

	setSpriteBlendMode(Graphics::BLEND_NORMAL);
}

Mesh3DS *BaseRenderOpenGL3DShader::createMesh3DS() {
	return new Mesh3DSOpenGLShader(_geometryShader);
}

MeshX *BaseRenderOpenGL3DShader::createMeshX() {
	return new MeshXOpenGLShader(_gameRef, _modelXShader, _flatShadowModelXShader);
}

ShadowVolume *BaseRenderOpenGL3DShader::createShadowVolume() {
	return new ShadowVolumeOpenGLShader(_gameRef, _shadowVolumeShader, _shadowMaskShader);
}

} // namespace Wintermute

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
