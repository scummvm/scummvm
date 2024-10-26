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

#include "engines/wintermute/ad/ad_block.h"
#include "engines/wintermute/ad/ad_generic.h"
#include "engines/wintermute/ad/ad_walkplane.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/gfx/3dcamera.h"

#include "graphics/opengl/system_headers.h"

#include "common/config-manager.h"

#include "math/glmath.h"

#if defined(USE_OPENGL_SHADERS)

#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d_shader.h"
#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl_shader.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl_shader.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume_opengl_shader.h"

namespace Wintermute {

BaseRenderer3D *makeOpenGL3DShaderRenderer(BaseGame *inGame) {
	return new BaseRenderOpenGL3DShader(inGame);
}

struct SpriteVertexShader {
	float x;
	float y;
	float u;
	float v;
	float r;
	float g;
	float b;
	float a;
};

BaseRenderOpenGL3DShader::BaseRenderOpenGL3DShader(BaseGame *inGame) : BaseRenderer3D(inGame) {
	_flatShadowMaskShader = nullptr;
}

BaseRenderOpenGL3DShader::~BaseRenderOpenGL3DShader() {
	_camera = nullptr;

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

void BaseRenderOpenGL3DShader::setAmbientLightRenderState() {
	byte a = RGBCOLGetA(_ambientLightColor);
	byte r = RGBCOLGetR(_ambientLightColor);
	byte g = RGBCOLGetG(_ambientLightColor);
	byte b = RGBCOLGetB(_ambientLightColor);

	if (!_ambientLightOverride) {
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

	_xmodelShader->use();
	_xmodelShader->setUniform("ambientLight", value);
}

int BaseRenderOpenGL3DShader::getMaxActiveLights() {
	return 8;
}

void BaseRenderOpenGL3DShader::lightEnable(int index, bool enable) {
	_xmodelShader->use();
	Common::String uniform = Common::String::format("lights[%i].enabled", index);
	if (enable)
		_xmodelShader->setUniform1f(uniform.c_str(), 1.0f);
	else
		_xmodelShader->setUniform1f(uniform.c_str(), -1.0f);
}

void BaseRenderOpenGL3DShader::setLightParameters(int index, const DXVector3 &position,
                                                  const DXVector3 &direction,
                                                  const DXVector4 &diffuse, bool spotlight) {
	Math::Vector4d position4d;
	position4d.x() = position._x;
	position4d.y() = position._y;
	position4d.z() = position._z;
	position4d.w() = 1.0f;

	Math::Vector4d direction4d;
	direction4d.x() = direction._x;
	direction4d.y() = direction._y;
	direction4d.z() = direction._z;
	direction4d.w() = 0.0f;

	if (spotlight) {
		direction4d.w() = -1.0f;
	}

	Math::Vector4d diffuse4d;
	diffuse4d.x() = diffuse._x;
	diffuse4d.y() = diffuse._y;
	diffuse4d.z() = diffuse._z;
	diffuse4d.w() = 0.0f;


	_xmodelShader->use();

	Common::String uniform = Common::String::format("lights[%i]._position", index);
	_xmodelShader->setUniform(uniform.c_str(), position4d);

	uniform = Common::String::format("lights[%i]._direction", index);
	_xmodelShader->setUniform(uniform.c_str(), direction4d);

	uniform = Common::String::format("lights[%i]._color", index);
	_xmodelShader->setUniform(uniform.c_str(), diffuse4d);
}

void BaseRenderOpenGL3DShader::enableCulling() {
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
}

void BaseRenderOpenGL3DShader::disableCulling() {
	glDisable(GL_CULL_FACE);
}

bool BaseRenderOpenGL3DShader::enableShadows() {
	return true; // TODO: reimplement. Shadows are broken for a while since it use not allowed binding to frame buffer
	if (_flatShadowMaskShader == nullptr) {
		_flatShadowColor = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.5f);

		_shadowTextureWidth = 512;
		_shadowTextureHeight = 512;

		float nearPlane = 1.0f;
		float farPlane = 10000.0f;
		float fovy = static_cast<float>(M_PI / 4.0f);

		float top = nearPlane *  tanf(fovy * 0.5f);
		float bottom = -top;
		float right = top;
		float left = -right;

		float deltaX = (-0.5f * (right - left)) / _shadowTextureWidth;
		float deltaY = (0.5f * (top - bottom)) / _shadowTextureHeight;

		Math::Matrix4 lightProjection = Math::makeFrustumMatrix(left + deltaX, right + deltaX, bottom + deltaY, top + deltaY, nearPlane, farPlane);

		_flatShadowXModelShader->use();
		_flatShadowXModelShader->setUniform("projMatrix", lightProjection);

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
		_flatShadowMaskShader = OpenGL::Shader::fromFiles("wme_flat_shadow_mask", flatShadowMaskAttributes);
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

void BaseRenderOpenGL3DShader::displayShadow(BaseObject *object, const DXVector3 *lightPos, bool lightPosRelative) {
	return; // TODO: reimplement. Shadows are broken for a while since it use not allowed binding to frame buffer
	if (_flatShadowMaskShader) {
		if (object->_shadowType <= SHADOW_SIMPLE) {
			// TODO: Display simple shadow here
			return;
		}

		DXVector3 position = *lightPos;

		if (lightPosRelative) {
			position = object->_posVector + *lightPos;
		}

		DXMatrix lightView = DXMatrix(Math::makeLookAtMatrix(Math::Vector3d(position),
																   Math::Vector3d(object->_posVector),
																   Math::Vector3d(0.0f, 1.0f, 0.0f)).getData());
		DXMatrix translation;
		DXMatrixTranslation(&translation, -position._x, -position._y, -position._z);
		DXMatrixTranspose(&translation, &translation);
		DXMatrixMultiply(&lightView, &translation, &lightView);

		Math::Matrix4 lightViewMatrix;
		lightViewMatrix.setData(lightView);
		_flatShadowXModelShader->use();
		_flatShadowXModelShader->setUniform("viewMatrix", lightViewMatrix);

		Math::Matrix4 worldMatrix;
		worldMatrix.setData(object->_worldMatrix);
		worldMatrix.transpose();
		_flatShadowXModelShader->setUniform("modelMatrix", worldMatrix);

		byte a = RGBCOLGetA(object->_shadowColor);
		byte r = RGBCOLGetR(object->_shadowColor);
		byte g = RGBCOLGetG(object->_shadowColor);
		byte b = RGBCOLGetB(object->_shadowColor);

		_flatShadowColor.x() = r / 255.0f;
		_flatShadowColor.y() = g / 255.0f;
		_flatShadowColor.z() = b / 255.0f;
		_flatShadowColor.w() = a / 255.0f;
		_flatShadowXModelShader->setUniform("shadowColor", _flatShadowColor);

		glBindFramebuffer(GL_FRAMEBUFFER, _flatShadowFrameBuffer);

		GLint currentViewport[4];
		glGetIntegerv(GL_VIEWPORT, currentViewport);
		glViewport(1, 1, _shadowTextureWidth - 2, _shadowTextureHeight - 2);

		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		object->_xmodel->renderFlatShadowModel();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);

		glDisable(GL_DEPTH_WRITEMASK);

		DXMatrix shadowPos;
		DXMatrixTranslation(&shadowPos, object->_posVector._x, object->_posVector._y, object->_posVector._z);
		DXMatrixTranspose(&shadowPos, &shadowPos);

		Math::Matrix4 viewMatrix, projectionMatrix, shadowPosition;
		viewMatrix.setData(_viewMatrix);
		projectionMatrix.setData(_projectionMatrix);
		shadowPosition.setData(shadowPos);
		_flatShadowMaskShader->use();
		_flatShadowMaskShader->setUniform("lightViewMatrix", lightViewMatrix);
		_flatShadowMaskShader->setUniform("worldMatrix", shadowPosition);
		_flatShadowMaskShader->setUniform("viewMatrix", viewMatrix);
		_flatShadowMaskShader->setUniform("projMatrix", projectionMatrix);
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
	BaseImage *screenshot = new BaseImage();
	Graphics::Surface *surface = new Graphics::Surface();
#ifdef SCUMM_BIG_ENDIAN
	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	Graphics::PixelFormat format(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
	surface->create(_viewportRect.width(), _viewportRect.height(), format);

	glReadPixels(_viewportRect.left, _viewportRect.height() - _viewportRect.bottom, _viewportRect.width(), _viewportRect.height(),
	             GL_RGBA, GL_UNSIGNED_BYTE, surface->getPixels());
	flipVertical(surface);
	Graphics::Surface *converted = surface->convertTo(getPixelFormat());
	screenshot->copyFrom(converted);
	delete surface;
	delete converted;
	return screenshot;
}

void BaseRenderOpenGL3DShader::setWindowed(bool windowed) {
	ConfMan.setBool("fullscreen", !windowed);
	g_system->beginGFXTransaction();
	g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !windowed);
	g_system->endGFXTransaction();
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

	Math::Matrix4 projectionMatrix2d;
	projectionMatrix2d.setData(_projectionMatrix2d);
	_fadeShader->use();
	_fadeShader->setUniform("color", color);
	_fadeShader->setUniform("projMatrix", projectionMatrix2d);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	setup2D(true);
}

bool BaseRenderOpenGL3DShader::fill(byte r, byte g, byte b, Common::Rect *rect) {
	glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

bool BaseRenderOpenGL3DShader::setViewport(int left, int top, int right, int bottom) {
	_viewportRect.setRect(left, top, right, bottom);
	_viewport._x = left;
	_viewport._y = top;
	_viewport._width = right - left;
	_viewport._height = bottom - top;
	_viewport._minZ = 0.0f;
	_viewport._maxZ = 1.0f;
	glViewport(left, _height - bottom, right - left, bottom - top);
	glDepthRange(_viewport._minZ, _viewport._maxZ);
	return true;
}

bool BaseRenderOpenGL3DShader::setViewport3D(DXViewport *viewport) {
	_viewport = *viewport;
	glViewport(_viewport._x, _height - _viewport._height, _viewport._width, _viewport._height);
	glDepthRange(_viewport._minZ, _viewport._maxZ);
	return true;
}

bool BaseRenderOpenGL3DShader::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);

	float lineCoords[4];

	lineCoords[0] = x1;
	lineCoords[1] = _height - y1;
	lineCoords[2] = x2;
	lineCoords[3] = _height - y2;

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

	Math::Matrix4 projectionMatrix2d;
	projectionMatrix2d.setData(_projectionMatrix2d);
	_lineShader->use();
	_lineShader->setUniform("color", colorValue);
	_lineShader->setUniform("projMatrix", projectionMatrix2d);

	glDrawArrays(GL_LINES, 0, 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

bool BaseRenderOpenGL3DShader::setProjection() {
	DXMatrix matProj;

	float resWidth, resHeight;
	float layerWidth, layerHeight;
	float modWidth, modHeight;
	bool customViewport;
	getProjectionParams(&resWidth, &resHeight, &layerWidth, &layerHeight, &modWidth, &modHeight, &customViewport);

	Rect32 rc;
	_gameRef->getCurrentViewportRect(&rc);
	float viewportWidth = (float)rc.right - (float)rc.left;
	float viewportHeight = (float)rc.bottom - (float)rc.top;

	// margins
	int mleft = rc.left;
	int mright = resWidth - viewportWidth - rc.left;
	int mtop = rc.top;
	int mbottom = resHeight - viewportHeight - rc.top;

	DXMatrixPerspectiveFovLH(&matProj, _fov, viewportWidth / viewportHeight, _nearClipPlane, _farClipPlane);

	float scaleMod = resHeight / viewportHeight;
	float scaleRatio = MAX(layerWidth / resWidth, layerHeight / resHeight) /** 1.05*/;

	float offsetX = (float)_gameRef->_offsetX;
	float offsetY = (float)_gameRef->_offsetY;

	if (!customViewport) {
		offsetX -= _drawOffsetX;
		offsetY -= _drawOffsetY;
	}

	matProj.matrix._11 *= scaleRatio * scaleMod;
	matProj.matrix._22 *= scaleRatio * scaleMod;
	matProj.matrix._31 = -(offsetX + (mleft - mright) / 2 - modWidth) / viewportWidth * 2.0f;
	matProj.matrix._32 =  (offsetY + (mtop - mbottom) / 2 - modHeight) / viewportHeight * 2.0f;

	return setProjectionTransform(matProj);
}

bool BaseRenderOpenGL3DShader::setProjection2D() {
	float nearPlane = -1.0f;
	float farPlane = 100.0f;

	DXMatrixIdentity(&_projectionMatrix2d);

	_projectionMatrix2d.matrix._11 = 2.0f / _width;
	_projectionMatrix2d.matrix._22 = 2.0f / _height;
	_projectionMatrix2d.matrix._33 = 2.0f / (farPlane - nearPlane);

	_projectionMatrix2d.matrix._41 = -1.0f;
	_projectionMatrix2d.matrix._42 = -1.0f;
	_projectionMatrix2d.matrix._43 = -(farPlane + nearPlane) / (farPlane - nearPlane);

	Math::Matrix4 projectionMatrix2d;
	projectionMatrix2d.setData(_projectionMatrix2d);
	_shadowMaskShader->use();
	_shadowMaskShader->setUniform("projMatrix", projectionMatrix2d);
	return true;
}

bool BaseRenderOpenGL3DShader::setWorldTransform(const DXMatrix &transform) {
	_worldMatrix = transform;
	DXMatrix newInvertedTranspose, world = transform;
	DXMatrixMultiply(&newInvertedTranspose, &world, &_viewMatrix);
	DXMatrixInverse(&newInvertedTranspose, nullptr, &newInvertedTranspose);
	DXMatrixTranspose(&newInvertedTranspose, &newInvertedTranspose);

	Math::Matrix4 modelMatrix, normalMatrix;
	modelMatrix.setData(world);
	normalMatrix.setData(newInvertedTranspose);

	_xmodelShader->use();
	_xmodelShader->setUniform("modelMatrix", modelMatrix);
	_xmodelShader->setUniform("normalMatrix", normalMatrix);

	_shadowVolumeShader->use();
	_shadowVolumeShader->setUniform("modelMatrix", modelMatrix);

	return true;
}

bool BaseRenderOpenGL3DShader::setViewTransform(const DXMatrix &transform) {
	_viewMatrix = transform;
	return true;
}

bool BaseRenderOpenGL3DShader::setProjectionTransform(const DXMatrix &transform) {
	_projectionMatrix = transform;
	return true;
}

bool BaseRenderOpenGL3DShader::windowedBlt() {
	flip();
	return true;
}

void Wintermute::BaseRenderOpenGL3DShader::onWindowChange() {
	_windowed = !g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
}

bool BaseRenderOpenGL3DShader::initRenderer(int width, int height, bool windowed) {
	glGenBuffers(1, &_spriteVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _spriteVBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SpriteVertexShader), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	static const char *spriteAttributes[] = {"position", "texcoord", "color", nullptr};
	_spriteShader = OpenGL::Shader::fromFiles("wme_sprite", spriteAttributes);

	_spriteShader->enableVertexAttribute("position", _spriteVBO, 2, GL_FLOAT, false, sizeof(SpriteVertexShader), 0);
	_spriteShader->enableVertexAttribute("texcoord", _spriteVBO, 2, GL_FLOAT, false, sizeof(SpriteVertexShader), 8);
	_spriteShader->enableVertexAttribute("color", _spriteVBO, 4, GL_FLOAT, false, sizeof(SpriteVertexShader), 16);

	static const char *geometryAttributes[] = { "position", "color", nullptr };
	_geometryShader = OpenGL::Shader::fromFiles("wme_geometry", geometryAttributes);

	static const char *shadowVolumeAttributes[] = { "position", nullptr };
	_shadowVolumeShader = OpenGL::Shader::fromFiles("wme_shadow_volume", shadowVolumeAttributes);

	static const char *shadowMaskAttributes[] = { "position", nullptr };
	_shadowMaskShader = OpenGL::Shader::fromFiles("wme_shadow_mask", shadowMaskAttributes);

	DXMatrix m;
	DXMatrixIdentity(&m);
	_transformStack.push_back(m);

	static const char *XModelAttributes[] = {"position", "texcoord", "normal", nullptr};
	_xmodelShader = OpenGL::Shader::fromFiles("wme_modelx", XModelAttributes);

	setDefaultAmbientLightColor();

	for (int i = 0; i < getMaxActiveLights(); ++i) {
		setLightParameters(i, DXVector3(0, 0, 0), DXVector3(0, 0, 0), DXVector4(0, 0, 0, 0), false);
		lightEnable(i, false);
	}

	_windowed = !ConfMan.getBool("fullscreen");
	_width = width;
	_height = height;

	_nearClipPlane = 90.0f;
	_farClipPlane = 10000.0f;

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
	_fadeShader = OpenGL::Shader::fromFiles("wme_fade", fadeAttributes);
	_fadeShader->enableVertexAttribute("position", _fadeVBO, 2, GL_FLOAT, false, 8, 0);

	glGenBuffers(1, &_lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * 8, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	static const char *lineAttributes[] = { "position", nullptr };
	_lineShader = OpenGL::Shader::fromFiles("wme_line", lineAttributes);
	_lineShader->enableVertexAttribute("position", _lineVBO, 2, GL_FLOAT, false, 8, 0);

	static const char *flatShadowXModelAttributes[] = { "position", nullptr };
	_flatShadowXModelShader = OpenGL::Shader::fromFiles("wme_flat_shadow_modelx", flatShadowXModelAttributes);

	_active = true;

	setProjection();

	return true;
}

bool Wintermute::BaseRenderOpenGL3DShader::flip() {
	g_system->updateScreen();
	return true;
}

bool BaseRenderOpenGL3DShader::indicatorFlip() {
	flip();
	return true;
}

bool BaseRenderOpenGL3DShader::forcedFlip() {
	flip();
	return true;
}

bool BaseRenderOpenGL3DShader::setup2D(bool force) {
	if (_state != RSTATE_2D || force) {
		_state = RSTATE_2D;

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
	if (_state != RSTATE_3D || force) {
		_state = RSTATE_3D;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		setAmbientLightRenderState();

		if (camera)
			_camera = camera;
		if (_camera) {
			DXMatrix viewMatrix;
			_camera->getViewMatrix(&viewMatrix);
			setViewTransform(viewMatrix);

			_fov = _camera->_fov;

			if (_camera->_nearClipPlane >= 0.0f) {
				_nearClipPlane = _camera->_nearClipPlane;
			} else {
				_nearClipPlane = DEFAULT_NEAR_PLANE;
			}

			if (_camera->_farClipPlane >= 0.0f) {
				_farClipPlane = _camera->_farClipPlane;
			} else {
				_farClipPlane = DEFAULT_FAR_PLANE;
			}
		} else {
			_nearClipPlane = DEFAULT_NEAR_PLANE;
			_farClipPlane = DEFAULT_FAR_PLANE;
		}

		bool fogEnabled;
		uint32 fogColor;
		float fogStart, fogEnd;

		_gameRef->getFogParams(&fogEnabled, &fogColor, &fogStart, &fogEnd);
		if (fogEnabled) {
			// TODO: Implement fog
			GLfloat color[4];
			color[0] = RGBCOLGetR(fogColor) / 255.0f;
			color[1] = RGBCOLGetG(fogColor) / 255.0f;
			color[2] = RGBCOLGetB(fogColor) / 255.0f;
			color[3] = RGBCOLGetA(fogColor) / 255.0f;
			debug(5, "BaseRenderOpenGL3DShader::setup3D fog not yet implemented! [%f %f %f %f]", color[0], color[1], color[2], color[3]);
		} else {
			// TODO: Disable fog in shader
		}

		glViewport(_viewportRect.left, _height - _viewportRect.bottom, _viewportRect.width(), _viewportRect.height());

		setProjection();
	}

	Math::Matrix4 viewMatrix, projectionMatrix;
	viewMatrix.setData(_viewMatrix);
	projectionMatrix.setData(_projectionMatrix);
	_xmodelShader->use();
	_xmodelShader->setUniform("viewMatrix", viewMatrix);
	_xmodelShader->setUniform("projMatrix", projectionMatrix);
	// this is 8 / 255, since 8 is the value used by wme (as a DWORD)
	_xmodelShader->setUniform1f("alphaRef", 0.031f);

	_geometryShader->use();
	_geometryShader->setUniform("viewMatrix", viewMatrix);
	_geometryShader->setUniform("projMatrix", projectionMatrix);

	_shadowVolumeShader->use();
	_shadowVolumeShader->setUniform("viewMatrix", viewMatrix);
	_shadowVolumeShader->setUniform("projMatrix", projectionMatrix);

	return true;
}

bool BaseRenderOpenGL3DShader::setupLines() {
	if (_state != RSTATE_LINES) {
		_state = RSTATE_LINES;

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
	                                    const Wintermute::Vector2 &pos, const Wintermute::Vector2 &rot,
	                                    const Wintermute::Vector2 &scale, float angle, uint32 color,
	                                    bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
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
	vertices[0].x = pos.x;
	vertices[0].y = correctedYPos;

	vertices[1].x = pos.x;
	vertices[1].y = correctedYPos - height;

	vertices[2].x = pos.x + width;
	vertices[2].y = correctedYPos;

	vertices[3].x = pos.x + width;
	vertices[3].y = correctedYPos - height;

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

	Math::Matrix4 projectionMatrix2d;
	projectionMatrix2d.setData(_projectionMatrix2d);
	_spriteShader->use();
	_spriteShader->setUniform("alphaTest", !alphaDisable);
	_spriteShader->setUniform("transform", transform);
	_spriteShader->setUniform("projMatrix", projectionMatrix2d);

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

void BaseRenderOpenGL3DShader::renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
                                                    const BaseArray<AdGeneric *> &generics, Camera3D *camera) {
	DXMatrix matIdentity;
	DXMatrixIdentity(&matIdentity);

	if (camera)
		_gameRef->_renderer3D->setup3D(camera, true);

	setWorldTransform(matIdentity);

	// disable color write
	glBlendFunc(GL_ZERO, GL_ONE);

	glFrontFace(GL_CW);
	glBindTexture(GL_TEXTURE_2D, 0);

	// render blocks
	for (uint i = 0; i < blocks.size(); i++) {
		if (blocks[i]->_active && blocks[i]->_receiveShadows) {
			blocks[i]->_mesh->render();
		}
	}

	// render walk planes
	for (uint i = 0; i < planes.size(); i++) {
		if (planes[i]->_active && planes[i]->_receiveShadows) {
			planes[i]->_mesh->render();
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
	return new Mesh3DSOpenGLShader(_gameRef, _geometryShader);
}

XMesh *BaseRenderOpenGL3DShader::createXMesh() {
	return new XMeshOpenGLShader(_gameRef, _xmodelShader, _flatShadowXModelShader);
}

ShadowVolume *BaseRenderOpenGL3DShader::createShadowVolume() {
	return new ShadowVolumeOpenGLShader(_gameRef, _shadowVolumeShader, _shadowMaskShader);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)
