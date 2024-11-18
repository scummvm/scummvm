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

#include "engines/wintermute/base/gfx/3dutils.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d_shader.h"
#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl_shader.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl_shader.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume_opengl_shader.h"

namespace Wintermute {

BaseRenderer3D *makeOpenGL3DShaderRenderer(BaseGame *inGame) {
	return new BaseRenderOpenGL3DShader(inGame);
}

BaseRenderOpenGL3DShader::BaseRenderOpenGL3DShader(BaseGame *inGame) : BaseRenderer3D(inGame) {
	setDefaultAmbientLightColor();
	_spriteVBO = 0;
	_alphaRef = 0;
}

BaseRenderOpenGL3DShader::~BaseRenderOpenGL3DShader() {
	_camera = nullptr; // ref only
	glDeleteBuffers(1, &_spriteVBO);
}

bool BaseRenderOpenGL3DShader::initRenderer(int width, int height, bool windowed) {

	glGenBuffers(1, &_spriteVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _spriteVBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	static const char *spriteAttributes[] = {"position", "texcoord", "color", nullptr};
	_spriteShader = OpenGL::Shader::fromFiles("wme_sprite", spriteAttributes);

	_spriteShader->enableVertexAttribute("position", _spriteVBO, 3, GL_FLOAT, false, sizeof(SpriteVertex), 0);
	_spriteShader->enableVertexAttribute("texcoord", _spriteVBO, 2, GL_FLOAT, false, sizeof(SpriteVertex), 12);
	_spriteShader->enableVertexAttribute("color", _spriteVBO, 4, GL_FLOAT, false, sizeof(SpriteVertex), 20);

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

	float fadeVertexCoords[8];

	fadeVertexCoords[0 * 2 + 0] = 0;
	fadeVertexCoords[0 * 2 + 1] = height;
	fadeVertexCoords[1 * 2 + 0] = 0;
	fadeVertexCoords[1 * 2 + 1] = 0;
	fadeVertexCoords[2 * 2 + 0] = width;
	fadeVertexCoords[2 * 2 + 1] = height;
	fadeVertexCoords[3 * 2 + 0] = width;
	fadeVertexCoords[3 * 2 + 1] = 0;

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



	_windowed = !ConfMan.getBool("fullscreen");
	_width = width;
	_height = height;

	setViewport(0, 0, width, height);

	setProjection();

	_active = true;

	return true;
}

bool BaseRenderOpenGL3DShader::flip() {
	_lastTexture = nullptr;
	// Disable blend mode to prevent interfere with backend renderer
	bool prevStateBlend = glIsEnabled(GL_BLEND);
	glDisable(GL_BLEND);
	g_system->updateScreen();
	prevStateBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	return true;
}

bool BaseRenderOpenGL3DShader::fill(byte r, byte g, byte b, Common::Rect *rect) {
	if(!_gameRef->_editorMode) {
		glViewport(0, _height, _width, _height);
	}
	glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

bool BaseRenderOpenGL3DShader::setup2D(bool force) {
	if (_state != RSTATE_2D || force) {
		_state = RSTATE_2D;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		setSpriteBlendMode(Graphics::BLEND_NORMAL);

		_alphaRef = 0.0f;

		glPolygonMode(GL_FRONT, GL_FILL);
		glFrontFace(GL_CCW);  // WME DX have CW
		glEnable(GL_CULL_FACE);
		glDisable(GL_STENCIL_TEST);
	}

	return true;
}

bool BaseRenderOpenGL3DShader::setup3D(Camera3D *camera, bool force) {
	if (_state != RSTATE_3D || force) {
		_state = RSTATE_3D;

		glEnable(GL_NORMALIZE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glEnable(GL_DEPTH_TEST);
		// WME uses 8 as a reference value and Direct3D expects it to be in the range [0, 255]
		_alphaRef = 8 / 255.0f;

		setAmbientLightRenderState();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


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

		setProjection();
	}

	Math::Matrix4 viewMatrix, projectionMatrix;
	viewMatrix.setData(_viewMatrix);
	projectionMatrix.setData(_glProjectionMatrix);
	_xmodelShader->use();
	_xmodelShader->setUniform("viewMatrix", viewMatrix);
	_xmodelShader->setUniform("projMatrix", projectionMatrix);
	_xmodelShader->setUniform1f("alphaRef", _alphaRef);
	_xmodelShader->setUniform("alphaTest", true);


	_geometryShader->use();
	_geometryShader->setUniform("viewMatrix", viewMatrix);
	_geometryShader->setUniform("projMatrix", projectionMatrix);

	_shadowVolumeShader->use();
	_shadowVolumeShader->setUniform("viewMatrix", viewMatrix);
	_shadowVolumeShader->setUniform("projMatrix", projectionMatrix);

	return true;
}

void BaseRenderOpenGL3DShader::setAmbientLightRenderState() {
	byte a = 0;
	byte r = 0;
	byte g = 0;
	byte b = 0;

	if (_ambientLightOverride) {
		a = RGBCOLGetA(_ambientLightColor);
		r = RGBCOLGetR(_ambientLightColor);
		g = RGBCOLGetG(_ambientLightColor);
		b = RGBCOLGetB(_ambientLightColor);
	} else {
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

	if (_xmodelShader) {
		_xmodelShader->use();
		_xmodelShader->setUniform("ambientLight", value);
	}
}

bool BaseRenderOpenGL3DShader::setupLines() {
	if (_state != RSTATE_LINES) {
		_state = RSTATE_LINES;

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_DEPTH_TEST);
		glFrontFace(GL_CW); // WME DX have CCW
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		_lineShader->use();
		_lineShader->setUniform1f("alphaRef", _alphaRef);
		_lineShader->setUniform("alphaTest", true);

		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		_lastTexture = nullptr;
	}

	return true;
}

bool BaseRenderOpenGL3DShader::drawSpriteEx(BaseSurface *tex, const Wintermute::Rect32 &rect,
										const Wintermute::Vector2 &pos, const Wintermute::Vector2 &rot,
										const Wintermute::Vector2 &scale, float angle, uint32 color,
										bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
										bool mirrorX, bool mirrorY) {
	BaseSurfaceOpenGL3D *texture = dynamic_cast<BaseSurfaceOpenGL3D *>(tex);
	if (!texture)
		return false;

	if (_spriteBatchMode) {
		_batchTexture = texture;
		_batchAlphaDisable = alphaDisable;
		_batchBlendMode = blendMode;
	}

	if (_forceAlphaColor != 0) {
		color = _forceAlphaColor;
	}

	float width = (rect.right - rect.left) * scale.x;
	float height = (rect.bottom - rect.top) * scale.y;

	int texWidth = texture->getGLTextureWidth();
	int texHeight = texture->getGLTextureHeight();

	float texLeft = (float)rect.left / (float)texWidth;
	float texTop = (float)rect.top / (float)texHeight;
	float texRight = (float)rect.right / (float)texWidth;
	float texBottom = (float)rect.bottom / (float)texHeight;

	if (mirrorX) {
		SWAP(texLeft, texRight);
	}

	if (mirrorY) {
		SWAP(texTop, texBottom);
	}

	SpriteVertex vertices[4] = {};

	// batch mode
	if (_spriteBatchMode) {
		// TODO
		commitSpriteBatch();
	}

	// Convert to OpenGL origin space
	SWAP(texTop, texBottom);

	// texture coords
	vertices[0].u = texLeft;
	vertices[0].v = texBottom;

	vertices[1].u = texLeft;
	vertices[1].v = texTop;

	vertices[2].u = texRight;
	vertices[2].v = texBottom;

	vertices[3].u = texRight;
	vertices[3].v = texTop;

	float offset = _height / 2.0f;
	float correctedYPos = (pos.y - offset) * -1.0f + offset;

	// position coords
	vertices[0].x = pos.x;
	vertices[0].y = correctedYPos;
	vertices[0].z = 0.9f;

	vertices[1].x = pos.x;
	vertices[1].y = correctedYPos - height;
	vertices[1].z = 0.9f;

	vertices[2].x = pos.x + width;
	vertices[2].y = correctedYPos;
	vertices[2].z = 0.9f;

	vertices[3].x = pos.x + width;
	vertices[3].y = correctedYPos - height;
	vertices[3].z = 0.9f;

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

	if (angle != 0) {
		DXVector2 sc(1.0f, 1.0f);
		DXVector2 rotation(rot.x, (rot.y - (_height / 2.0f)) * -1.0f + (_height / 2.0f));
		transformVertices(vertices, &rotation, &sc, degToRad(-angle));
	}

	if (_spriteBatchMode) {
		// TODO
	} else {
		setSpriteBlendMode(blendMode);
		if (alphaDisable) {
			_spriteShader->setUniform("alphaTest", false);
			glDisable(GL_BLEND);
		}

		if (_lastTexture != texture) {
			_lastTexture = texture;
			glBindTexture(GL_TEXTURE_2D, texture->getTextureName());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			// for sprites we clamp to the edge, to avoid line fragments at the edges
			// this is not done by wme, though
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glEnable(GL_TEXTURE_2D);
		}

		glBindBuffer(GL_ARRAY_BUFFER, _spriteVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(SpriteVertex), vertices);

		setProjection2D(_spriteShader);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (alphaDisable) {
			_spriteShader->setUniform("alphaTest", true);
			glEnable(GL_BLEND);
		}
	}

	return true;
}

bool BaseRenderOpenGL3DShader::commitSpriteBatch() {
	// render
	setSpriteBlendMode(_batchBlendMode);
	if (_batchAlphaDisable) {
		_spriteShader->setUniform("alphaTest", false);
		glDisable(GL_BLEND);
	}

	if (_lastTexture != _batchTexture) {
		_lastTexture = _batchTexture;
		glBindTexture(GL_TEXTURE_2D, _batchTexture->getTextureName());
	}

	// TODO

	if (_batchAlphaDisable) {
		_spriteShader->setUniform("alphaTest", true);
		glEnable(GL_BLEND);
	}

	return true;
}

bool BaseRenderOpenGL3DShader::startSpriteBatch() {
	//_spriteBatchMode = true;
	return true;
}

bool BaseRenderOpenGL3DShader::endSpriteBatch() {
	if (!_spriteBatchMode)
		return false;

	_spriteBatchMode = false;
	return commitSpriteBatch();
}

DXMatrix *BaseRenderOpenGL3DShader::buildMatrix(DXMatrix* out, const DXVector2 *centre, const DXVector2 *scaling, float angle) {
	DXMatrix matrices[5];

	DXMatrixTranslation(&matrices[0], -centre->_x, -centre->_y, 0);
	DXMatrixScaling(&matrices[1], scaling->_x, scaling->_y, 1);
	DXMatrixIdentity(&matrices[2]);
	DXMatrixIdentity(&matrices[3]);
	DXMatrixRotationZ(&matrices[2], angle);
	DXMatrixTranslation(&matrices[3], centre->_x, centre->_y, 0);

	matrices[4] = matrices[0] * matrices[1] * matrices[2] * matrices[3];
	*out = matrices[4];

	return out;
}

void BaseRenderOpenGL3DShader::transformVertices(struct SpriteVertex *vertices, const DXVector2 *centre, const DXVector2 *scaling, float angle) {
	DXMatrix matTransf, matVerts, matNew;

	buildMatrix(&matTransf, centre, scaling, angle);

	int cr;
	for (cr = 0; cr < 4; cr++) {
		matVerts(cr, 0) = vertices[cr].x;
		matVerts(cr, 1) = vertices[cr].y;
		matVerts(cr, 2) = vertices[cr].z;
		matVerts(cr, 3) = 1.0f;
	}

	matNew = matVerts * matTransf;

	for (cr = 0; cr < 4; cr++) {
		vertices[cr].x = matNew(cr, 0);
		vertices[cr].y = matNew(cr, 1);
		vertices[cr].z = matNew(cr, 2);
	}
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

	_lineShader->use();
	_lineShader->setUniform("color", colorValue);
	setProjection2D(_lineShader);

	glDrawArrays(GL_LINES, 0, 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

void BaseRenderOpenGL3DShader::fadeToColor(byte r, byte g, byte b, byte a) {
	Math::Vector4d color;
	color.x() = r / 255.0f;
	color.y() = g / 255.0f;
	color.z() = b / 255.0f;
	color.w() = a / 255.0f;

	setSpriteBlendMode(Graphics::BLEND_UNKNOWN);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glBindBuffer(GL_ARRAY_BUFFER, _fadeVBO);
	_lastTexture = nullptr;

	_fadeShader->use();
	_fadeShader->setUniform("color", color);
	setProjection2D(_fadeShader);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	setup2D(true);
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

bool BaseRenderOpenGL3DShader::enableShadows() {
	warning("BaseRenderOpenGL3DShader::disableShadows not implemented yet");
	return true;
}

bool BaseRenderOpenGL3DShader::disableShadows() {
	warning("BaseRenderOpenGL3DShader::disableShadows not implemented yet");
	return true;
}

void BaseRenderOpenGL3DShader::displayShadow(BaseObject *object, const DXVector3 *lightPos, bool lightPosRelative) {
	// TODO: to be implemented
	return;
}


void BaseRenderOpenGL3DShader::setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode, bool forceChange) {
	if (blendMode == _blendMode && !forceChange)
		return;

	_blendMode = blendMode;

	switch (_blendMode) {
	case Graphics::BLEND_NORMAL:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case Graphics::BLEND_ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;

	case Graphics::BLEND_SUBTRACTIVE:
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		break;

	default:
		break;
	}
}

bool BaseRenderOpenGL3DShader::stencilSupported() {
	// assume that we have a stencil buffer
	return true;
}

int BaseRenderOpenGL3DShader::getMaxActiveLights() {
	return 8;
}

bool BaseRenderOpenGL3DShader::invalidateTexture(BaseSurfaceOpenGL3D *texture) {
	if (_lastTexture == texture)
		_lastTexture = nullptr;
	if (_batchTexture == texture)
		_batchTexture = nullptr;

	return true;
}

// implements D3D LightEnable()
void BaseRenderOpenGL3DShader::lightEnable(int index, bool enable) {
	_xmodelShader->use();
	Common::String uniform = Common::String::format("lights[%i].enabled", index);
	if (enable)
		_xmodelShader->setUniform1f(uniform.c_str(), 1.0f);
	else
		_xmodelShader->setUniform1f(uniform.c_str(), -1.0f);
}

// backend layer 3DLight::SetLight
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

// backend layer AdSceneGeometry::Render
void BaseRenderOpenGL3DShader::renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
											   const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) {
	// don't render scene geometry, as OpenGL ES 2 has no wireframe rendering and we don't have a shader alternative yet
}

// backend layer 3DShadowVolume::Render()
void BaseRenderOpenGL3DShader::renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
													const BaseArray<AdGeneric *> &generics, Camera3D *camera) {
	DXMatrix matIdentity;
	DXMatrixIdentity(&matIdentity);

	if (camera)
		_gameRef->_renderer3D->setup3D(camera, true);

	setWorldTransform(matIdentity);

	// disable color write
	setSpriteBlendMode(Graphics::BLEND_UNKNOWN);
	glBlendFunc(GL_ZERO, GL_ONE);

	// no texture
	_lastTexture = nullptr;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFrontFace(GL_CW); // WME DX have CCW

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

// implements D3D SetRenderState() D3DRS_CULLMODE - CCW
void BaseRenderOpenGL3DShader::enableCulling() {
	glFrontFace(GL_CW); // WME DX have CCW
	glEnable(GL_CULL_FACE);
}

// implements D3D SetRenderState() D3DRS_CULLMODE - NONE
void BaseRenderOpenGL3DShader::disableCulling() {
	glDisable(GL_CULL_FACE);
}

// implements D3D SetViewport() for 2D renderer
bool BaseRenderOpenGL3DShader::setViewport(int left, int top, int right, int bottom) {
	_viewportRect.setRect(left, top, right, bottom);
	_viewport._x = left;
	_viewport._y = top;
	_viewport._width = right - left;
	_viewport._height = bottom - top;
	glViewport(left, _height - bottom, right - left, bottom - top);
	return true;
}

// implements D3D SetViewport() for 3D renderer
bool BaseRenderOpenGL3DShader::setViewport3D(DXViewport *viewport) {
	_viewport = *viewport;
	glViewport(_viewport._x, _height - _viewport._height, _viewport._width, _viewport._height);
	return true;
}

bool BaseRenderOpenGL3DShader::setProjection2D(OpenGL::Shader *shader) {
	DXMatrix matrix2D;
	DXMatrixOrthoOffCenterLH(&matrix2D, 0, _width, 0, _height, 0.0f, 1.0f);

	// convert DX [0, 1] depth range to OpenGL [-1, 1] depth range.
	matrix2D.matrix._33 = 2.0f;
	matrix2D.matrix._43 = -1.0f;

	Math::Matrix3 transform;
	transform.setToIdentity();
	Math::Matrix4 projectionMatrix2d;
	projectionMatrix2d.setData(matrix2D);
	shader->use();
	shader->setUniform("projMatrix", projectionMatrix2d);
	shader->setUniform("transform", transform);

	return true;
}

// implements SetTransform() D3DTS_WORLD
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

// implements SetTransform() D3DTS_WIEW
bool BaseRenderOpenGL3DShader::setViewTransform(const DXMatrix &transform) {
	_viewMatrix = transform;
	return true;
}

// implements SetTransform() D3DTS_PROJECTION
bool BaseRenderOpenGL3DShader::setProjectionTransform(const DXMatrix &transform) {
	_projectionMatrix = transform;

	// convert DX [0, 1] depth range to OpenGL [-1, 1] depth range.
	_glProjectionMatrix = transform;
	float range = 2.0f / (_farClipPlane - _nearClipPlane);
	_glProjectionMatrix.matrix._33 = range;
	_glProjectionMatrix.matrix._43 = -(_nearClipPlane + _farClipPlane) * range / 2;

	glViewport(_viewportRect.left, _height - _viewportRect.bottom, _viewportRect.width(), _viewportRect.height());

	return true;
}

BaseSurface *Wintermute::BaseRenderOpenGL3DShader::createSurface() {
	return new BaseSurfaceOpenGL3D(_gameRef, this);
}

Mesh3DS *BaseRenderOpenGL3DShader::createMesh3DS() {
	return new Mesh3DSOpenGLShader(_gameRef, _geometryShader);
}

XMesh *BaseRenderOpenGL3DShader::createXMesh() {
	return new XMeshOpenGLShader(_gameRef, _xmodelShader);
}

ShadowVolume *BaseRenderOpenGL3DShader::createShadowVolume() {
	return new ShadowVolumeOpenGLShader(_gameRef, _shadowVolumeShader, _shadowMaskShader);
}

// ScummVM specific ends <--

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)
