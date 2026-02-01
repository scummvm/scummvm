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
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_generic.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_geometry.h"
#include "engines/wintermute/ad/ad_walkplane.h"
#include "engines/wintermute/ad/ad_waypoint_group3d.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/gfx/3dcamera.h"
#include "engines/wintermute/base/gfx/3dlight.h"
#include "engines/wintermute/platform_osystem.h"

#include "graphics/opengl/system_headers.h"

#include "common/config-manager.h"

#if defined(USE_OPENGL_GAME)

#include "engines/wintermute/base/gfx/3dutils.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume_opengl.h"

namespace Wintermute {

BaseRenderer3D *makeOpenGL3DRenderer(BaseGame *inGame) {
	return new BaseRenderOpenGL3D(inGame);
}

BaseRenderOpenGL3D::BaseRenderOpenGL3D(BaseGame *inGame) : BaseRenderer3D(inGame) {
}

BaseRenderOpenGL3D::~BaseRenderOpenGL3D() {
	_camera = nullptr; // ref only
}

bool BaseRenderOpenGL3D::initRenderer(int width, int height, bool windowed) {
	_simpleShadow[0].x = -1.0f;
	_simpleShadow[0].y = 0.0f;
	_simpleShadow[0].z = 1.0f;
	_simpleShadow[0].nx = 0.0f;
	_simpleShadow[0].ny = 1.0f;
	_simpleShadow[0].nz = 0.0f;
	_simpleShadow[0].u = 0.0f;
	_simpleShadow[0].v = 1.0f;

	_simpleShadow[1].x = -1.0f;
	_simpleShadow[1].y = 0.0f;
	_simpleShadow[1].z = -1.0f;
	_simpleShadow[1].nx = 0.0f;
	_simpleShadow[1].ny = 1.0f;
	_simpleShadow[1].nz = 0.0f;
	_simpleShadow[1].u = 1.0f;
	_simpleShadow[1].v = 1.0f;

	_simpleShadow[2].x = 1.0f;
	_simpleShadow[2].y = 0.0f;
	_simpleShadow[2].z = 1.0f;
	_simpleShadow[2].nx = 0.0f;
	_simpleShadow[2].ny = 1.0f;
	_simpleShadow[2].nz = 0.0f;
	_simpleShadow[2].u = 0.0f;
	_simpleShadow[2].v = 0.0f;

	_simpleShadow[3].x = 1.0f;
	_simpleShadow[3].y = 0.0f;
	_simpleShadow[3].z = -1.0f;
	_simpleShadow[3].nx = 0.0f;
	_simpleShadow[3].ny = 1.0f;
	_simpleShadow[3].nz = 0.0f;
	_simpleShadow[3].u = 1.0f;
	_simpleShadow[3].v = 0.0f;

	// filter post process: greyscale, sepia
	glGenTextures(1, &_postfilterTexture);
	glBindTexture(GL_TEXTURE_2D, _postfilterTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	setSpriteBlendMode(Graphics::BLEND_NORMAL, true);

	_windowed = !ConfMan.getBool("fullscreen");
	_width = width;
	_height = height;

	g_system->showMouse(false);

	setViewport(0, 0, width, height);

	setProjection();

	_postFilterMode = kPostFilterOff;

	_active = true;

	_game->_supportsRealTimeShadows = true;

	setDefaultAmbientLightColor();
	_lightPositions.resize(getMaxActiveLights());
	_lightDirections.resize(getMaxActiveLights());

	return true;
}

bool BaseRenderOpenGL3D::flip() {
	_lastTexture = nullptr;

	// Store blend mode and cull face mode
	GLboolean stateBlend, stateCullFace, stateAlpha;
	glGetBooleanv(GL_BLEND, &stateBlend);
	glGetBooleanv(GL_CULL_FACE, &stateCullFace);
	glGetBooleanv(GL_ALPHA_TEST, &stateAlpha);

	postfilter();

	// Disable blend mode and cull face to prevent interfere with backend renderer
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	g_system->updateScreen();

	// Restore blend mode and cull face state
	if (stateBlend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	if (stateCullFace)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if (stateAlpha)
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);

	_state = RSTATE_NONE;
	return true;
}

bool BaseRenderOpenGL3D::clear() {
	if(!_game->_editorMode) {
		glViewport(0, _height, _width, _height);
	}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

bool BaseRenderOpenGL3D::setup2D(bool force) {
	if (_state != RSTATE_2D || force) {
		_state = RSTATE_2D;

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		setSpriteBlendMode(Graphics::BLEND_NORMAL);

		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.0f);

		glFrontFace(GL_CW);  // WME DX have CCW
		glEnable(GL_CULL_FACE);
		glDisable(GL_STENCIL_TEST);

		glDisable(GL_FOG);
	}

	return true;
}

bool BaseRenderOpenGL3D::setup3D(Camera3D *camera, bool force) {
	if (_state != RSTATE_3D || force) {
		_state = RSTATE_3D;

		glEnable(GL_NORMALIZE);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_ALPHA_TEST);
		// WME uses 8 as a reference value and Direct3D expects it to be in the range [0, 255]
		glAlphaFunc(GL_GEQUAL, 8 / 255.0f);

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

		// lighting
		glEnable(GL_LIGHTING);

		for (int i = 0; i < getMaxActiveLights(); ++i) {
			glLightfv(GL_LIGHT0 + i, GL_POSITION, _lightPositions[i]);
			glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, _lightDirections[i]);
		}

		// fog
		bool fogEnabled;
		uint32 fogColor;
		float fogStart, fogEnd;
		_game->getFogParams(&fogEnabled, &fogColor, &fogStart, &fogEnd);
		if (fogEnabled) {
			glEnable(GL_FOG);
			GLfloat color[4] = { RGBCOLGetR(fogColor) / 255.0f,
			                     RGBCOLGetG(fogColor) / 255.0f,
			                     RGBCOLGetB(fogColor) / 255.0f,
			                     RGBCOLGetA(fogColor) / 255.0f };
			glFogfv(GL_FOG_COLOR, color);
			glFogi(GL_FOG_MODE, GL_LINEAR);
			glFogf(GL_FOG_START, fogStart);
			glFogf(GL_FOG_END, fogEnd);

		} else {
			glDisable(GL_FOG);
		}

		setProjection();
	}

	return true;
}

void BaseRenderOpenGL3D::setAmbientLightRenderState() {
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
		uint32 color = _game->getAmbientLightColor();

		a = RGBCOLGetA(color);
		r = RGBCOLGetR(color);
		g = RGBCOLGetG(color);
		b = RGBCOLGetB(color);
	}

	float value[] = { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, value);
}

bool BaseRenderOpenGL3D::setupLines() {
	if (_state != RSTATE_LINES) {
		_state = RSTATE_LINES;

		float value[] = { 0, 0, 0, 0 };

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glFrontFace(GL_CW); // WME DX have CCW
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, value);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		_lastTexture = nullptr;
	}

	return true;
}

bool BaseRenderOpenGL3D::drawSpriteEx(BaseSurface *tex, const Common::Rect32 &rect,
	                              const DXVector2 &pos, const DXVector2 &rot,
	                              const DXVector2 &scale,
	                              float angle, uint32 color, bool alphaDisable,
	                              Graphics::TSpriteBlendMode blendMode,
	                              bool mirrorX, bool mirrorY) {
	BaseSurfaceOpenGL3D *texture = dynamic_cast<BaseSurfaceOpenGL3D *>(tex);
	if (!texture)
		return false;

	if (_forceAlphaColor != 0) {
		color = _forceAlphaColor;
	}

	float width = (rect.right - rect.left) * scale._x;
	float height = (rect.bottom - rect.top) * scale._y;

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

	SpriteVertex vertices[4];

	// texture coords
	vertices[0].u = texLeft;
	vertices[0].v = texBottom;

	vertices[1].u = texLeft;
	vertices[1].v = texTop;

	vertices[2].u = texRight;
	vertices[2].v = texBottom;

	vertices[3].u = texRight;
	vertices[3].v = texTop;

	// position coords
	vertices[0].x = pos._x;
	vertices[0].y = pos._y + height;
	vertices[0].z = 0.9f;

	vertices[1].x = pos._x;
	vertices[1].y = pos._y;
	vertices[1].z = 0.9f;

	vertices[2].x = pos._x + width;
	vertices[2].y = pos._y + height;
	vertices[2].z = 0.9f;

	vertices[3].x = pos._x + width;
	vertices[3].y = pos._y;
	vertices[3].z = 0.9f;

	if (angle != 0) {
		DXVector2 sc(1.0f, 1.0f);
		DXVector2 rotation(rot._x, rot._y);
		transformVertices(vertices, &rotation, &sc, angle);
	}

	for (int i = 0; i < 4; i++) {
		vertices[i].x += _drawOffsetX;
		vertices[i].y += _drawOffsetY;
	}

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

	setSpriteBlendMode(blendMode);
	if (alphaDisable) {
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
	}

	if (_lastTexture != texture) {
		_lastTexture = texture;
		glBindTexture(GL_TEXTURE_2D, texture->getTextureName());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// for sprites we clamp to the edge, to avoid line fragments at the edges
		// this is not done by wme, but centering pixel by 0.5
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glEnable(GL_TEXTURE_2D);
	}

	glViewport(0, 0, _width, _height);
	setProjection2D();

	glFrontFace(GL_CW);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(SpriteVertex), &vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SpriteVertex), &vertices[0].u);
	glColorPointer(4, GL_FLOAT, sizeof(SpriteVertex), &vertices[0].r);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (alphaDisable) {
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
	}

	return true;
}

bool BaseRenderOpenGL3D::commitSpriteBatch() {
	// nothing to implement
	return true;
}

bool BaseRenderOpenGL3D::startSpriteBatch() {
	// nothing to implement
	return true;
}

bool BaseRenderOpenGL3D::endSpriteBatch() {
	// nothing to implement
	return true;
}

DXMatrix *BaseRenderOpenGL3D::buildMatrix(DXMatrix* out, const DXVector2 *centre, const DXVector2 *scaling, float angle) {
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

void BaseRenderOpenGL3D::transformVertices(struct SpriteVertex *vertices, const DXVector2 *centre, const DXVector2 *scaling, float angle) {
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

bool BaseRenderOpenGL3D::setProjection() {
	DXMatrix matProj;

	float resWidth, resHeight;
	float layerWidth, layerHeight;
	float modWidth, modHeight;
	bool customViewport;
	getProjectionParams(&resWidth, &resHeight, &layerWidth, &layerHeight, &modWidth, &modHeight, &customViewport);

	Common::Rect32 rc;
	_game->getCurrentViewportRect(&rc);
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

	float offsetX = (float)_game->_offsetX;
	float offsetY = (float)_game->_offsetY;

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

bool BaseRenderOpenGL3D::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	setupLines();

	x1 += _drawOffsetX;
	x2 += _drawOffsetX;
	y1 += _drawOffsetY;
	y2 += _drawOffsetY;

	// position coords
	RectangleVertex vertices[2];
	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = 0.9f;
	vertices[1].x = x2;
	vertices[1].y = y2;
	vertices[1].z = 0.9f;

	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	glViewport(0, 0, _width, _height);
	setProjection2D();

	glColor4ub(r, g, b, a);

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(RectangleVertex), &vertices[0].x);

	glDrawArrays(GL_LINES, 0, 2);

	glDisableClientState(GL_VERTEX_ARRAY);

	return true;
}

bool BaseRenderOpenGL3D::fillRect(int x, int y, int w, int h, uint32 color) {
	setupLines();

	x += _drawOffsetX;
	y += _drawOffsetY;

	// position coords
	RectangleVertex vertices[4];
	vertices[0].x = x;
	vertices[0].y = y + h;
	vertices[0].z = 0.9f;
	vertices[1].x = x;
	vertices[1].y = y;
	vertices[1].z = 0.9f;
	vertices[2].x = x + w;
	vertices[2].y = y + h;
	vertices[2].z = 0.9f;
	vertices[3].x = x + w;
	vertices[3].y = y;
	vertices[3].z = 0.9f;

	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	glViewport(0, 0, _width, _height);
	setProjection2D();

	glColor4ub(r, g, b, a);

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(RectangleVertex), &vertices[0].x);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);

	setup2D();
	return true;
}

bool BaseRenderOpenGL3D::fadeToColor(byte r, byte g, byte b, byte a) {
	float left, right, bottom, top;

	left = _viewportRect.left;
	right = _viewportRect.right;
	bottom = _viewportRect.bottom;
	top = _viewportRect.top;

	// position coords
	RectangleVertex vertices[4];
	vertices[0].x = left;
	vertices[0].y = bottom;
	vertices[0].z = 0.0f;
	vertices[1].x = left;
	vertices[1].y = top;
	vertices[1].z = 0.0f;
	vertices[2].x = right;
	vertices[2].y = bottom;
	vertices[2].z = 0.0f;
	vertices[3].x = right;
	vertices[3].y = top;
	vertices[3].z = 0.0f;


	glEnable(GL_BLEND);
	setSpriteBlendMode(Graphics::BLEND_NORMAL);

	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	_lastTexture = nullptr;

	glViewport(0, 0, _width, _height);
	setProjection2D();

	glColor4ub(r, g, b, a);

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(RectangleVertex), &vertices[0].x);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);

	setup2D(true);

	return true;
}

BaseImage *BaseRenderOpenGL3D::takeScreenshot(int newWidth, int newHeight) {
	BaseImage *screenshot = new BaseImage();
	Graphics::Surface *surface = new Graphics::Surface();
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatRGBA32();
	surface->create(_viewportRect.width(), _viewportRect.height(), format);

	glReadPixels(_viewportRect.left, _viewportRect.height() - _viewportRect.bottom,
	             _viewportRect.width(), _viewportRect.height(),
	             GL_RGBA, GL_UNSIGNED_BYTE, surface->getPixels());
	screenshot->copyFrom(surface, newWidth, newHeight, Graphics::FLIP_V);
	delete surface;
	return screenshot;
}

bool BaseRenderOpenGL3D::enableShadows() {
	return true;
}

bool BaseRenderOpenGL3D::disableShadows() {
	return true;
}

void BaseRenderOpenGL3D::displaySimpleShadow(BaseObject *object) {
	if (!_ready || !object)
		return;

	BaseSurface *shadowImage;
	if (object->_shadowImage) {
		shadowImage = object->_shadowImage;
	} else {
		shadowImage = _game->_shadowImage;
	}

	if (!shadowImage) {
		return;
	}

	DXMatrix scale, trans, rot, finalm;
	DXMatrixScaling(&scale, object->_shadowSize * object->_scale3D, 1.0f, object->_shadowSize * object->_scale3D);
	DXMatrixRotationY(&rot, degToRad(object->_angle));
	DXMatrixTranslation(&trans, object->_posVector._x, object->_posVector._y, object->_posVector._z);
	DXMatrixMultiply(&finalm, &scale, &rot);
	DXMatrixMultiply(&finalm, &finalm, &trans);
	setWorldTransform(finalm);

	glFrontFace(GL_CCW);

	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_2D);
	static_cast<BaseSurfaceOpenGL3D *>(shadowImage)->setTexture();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].x);
	glNormalPointer(GL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].nx);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].u);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
}

void BaseRenderOpenGL3D::setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode, bool forceChange) {
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

bool BaseRenderOpenGL3D::shadowVolumeSupported() {
	return true;
}

int BaseRenderOpenGL3D::getMaxActiveLights() {
	GLint maxLightCount = 0;
	glGetIntegerv(GL_MAX_LIGHTS, &maxLightCount);
	return maxLightCount;
}

bool BaseRenderOpenGL3D::invalidateTexture(BaseSurface *texture) {
	if (_lastTexture == texture)
		_lastTexture = nullptr;

	return true;
}

bool BaseRenderOpenGL3D::invalidateDeviceObjects() {
	return STATUS_OK;
}

bool BaseRenderOpenGL3D::restoreDeviceObjects() {
	return STATUS_OK;
}

bool BaseRenderOpenGL3D::resetDevice() {
	return STATUS_OK;
}

// implements D3D LightEnable()
void BaseRenderOpenGL3D::lightEnable(int index, bool enable) {
	if (enable)
		glEnable(GL_LIGHT0 + index);
	else
		glDisable(GL_LIGHT0 + index);
}

// backend layer 3DLight::SetLight
void BaseRenderOpenGL3D::setLightParameters(int index, const DXVector3 &position, const DXVector3 &direction, const DXVector4 &diffuse, bool spotlight) {
	float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	glLightfv(GL_LIGHT0 + index, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0 + index, GL_AMBIENT, zero);
	glLightfv(GL_LIGHT0 + index, GL_SPECULAR, zero);

	_lightPositions[index]._x = position._x;
	_lightPositions[index]._y = position._y;
	_lightPositions[index]._z = position._z;
	_lightPositions[index]._w = 1.0f;

	if (spotlight) {
		_lightDirections[index] = direction;
		glLightfv(GL_LIGHT0 + index, GL_SPOT_DIRECTION, direction);

		glLightf(GL_LIGHT0 + index, GL_SPOT_EXPONENT, 0.0f);
		// WME sets the theta angle to 0.5 (radians) and (28.64789 degree) - inner cone
		// WME sets the phi angle to 1.0 (radians) and (57.29578 degree) - outer cone
		// inner cone - angle within which the spotlight has maximum intensity
		// outer cone - angle at the edge of the spotlight's cone
		// 0 <-> 28.64789 - maximum light intensity
		// 28.64789 <-> 57.29578 - light fades smoothly to zero
		// 57.29578 <-> 90 - there is no light
		// The smooth transition between inner code and outer cone create soft spotlight
		// There is no replacement for smooth transition in fixed OpenGL lights.
		// So, inner cone angle is used instead for better visual match
		glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, 0.5f * (180.0f / (float)M_PI));
	} else {
		glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180.0f);
	}
}

// backend layer AdSceneGeometry::Render
void BaseRenderOpenGL3D::renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
	                                     const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) {
	DXMatrix matIdentity;
	DXMatrixIdentity(&matIdentity);

	if (camera)
		_game->_renderer3D->setup3D(camera, true);

	setWorldTransform(matIdentity);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CW); // WME DX have CCW
	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);


	// render walk planes
	for (int32 i = 0; i < planes.getSize(); i++) {
		if (planes[i]->_active) {
			planes[i]->_mesh->render(true);
		}
	}

	// render blocks
	for (int32 i = 0; i < blocks.getSize(); i++) {
		if (blocks[i]->_active) {
			blocks[i]->_mesh->render(true);
		}
	}

	// render generic objects
	for (int32 i = 0; i < generics.getSize(); i++) {
		if (generics[i]->_active) {
			generics[i]->_mesh->render(true);
		}
	}

	// render waypoints
	AdScene *scene = ((AdGame *)_game)->_scene;
	AdSceneGeometry *geom = scene->_geom;
	if (geom && geom->_wptMarker) {
		DXMatrix viewMat, projMat, worldMat;
		DXVector3 vec2d(0.0f, 0.0f, 0.0f);

		getViewTransform(&viewMat);
		getProjectionTransform(&projMat);
		DXMatrixIdentity(&worldMat);

		DXViewport vport = getViewPort();

		setup2D();

		for (int32 i = 0; i < geom->_waypointGroups.getSize(); i++) {
			for (int32 j = 0; j < geom->_waypointGroups[i]->_points.getSize(); j++) {
				DXVec3Project(&vec2d, geom->_waypointGroups[i]->_points[j], &vport, &projMat, &viewMat, &worldMat);
				geom->_wptMarker->display(vec2d._x + scene->getOffsetLeft() - _drawOffsetX, vec2d._y + scene->getOffsetTop() - _drawOffsetY);
			}
		}
	}

	glDisable(GL_COLOR_MATERIAL);
}

// backend layer 3DShadowVolume::Render()
void BaseRenderOpenGL3D::renderShadowGeometry(const BaseArray<AdWalkplane *> &planes,
	                                      const BaseArray<AdBlock *> &blocks,
	                                      const BaseArray<AdGeneric *> &generics, Camera3D *camera) {
	DXMatrix matIdentity;
	DXMatrixIdentity(&matIdentity);

	if (camera)
		_game->_renderer3D->setup3D(camera, true);

	setWorldTransform(matIdentity);

	// disable color write
	setSpriteBlendMode(Graphics::BLEND_UNKNOWN);
	glBlendFunc(GL_ZERO, GL_ONE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// no texture
	_lastTexture = nullptr;
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glFrontFace(GL_CW); // WME DX have CCW

	// render blocks
	for (int32 i = 0; i < blocks.getSize(); i++) {
		if (blocks[i]->_active && blocks[i]->_receiveShadows) {
			blocks[i]->_mesh->render();
		}
	}

	// render walk planes
	for (int32 i = 0; i < planes.getSize(); i++) {
		if (planes[i]->_active && planes[i]->_receiveShadows) {
			planes[i]->_mesh->render();
		}
	}

	// render generic objects
	for (int32 i = 0; i < generics.getSize(); i++) {
		if (generics[i]->_active && generics[i]->_receiveShadows) {
			generics[i]->_mesh->render();
		}
	}

	setSpriteBlendMode(Graphics::BLEND_NORMAL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

// implements D3D SetRenderState() D3DRS_CULLMODE - CCW
void BaseRenderOpenGL3D::enableCulling() {
	glFrontFace(GL_CW); // WME DX have CCW
	glEnable(GL_CULL_FACE);
}

// implements D3D SetRenderState() D3DRS_CULLMODE - NONE
void BaseRenderOpenGL3D::disableCulling() {
	glDisable(GL_CULL_FACE);
}

// implements D3D SetViewport() for 2D renderer
bool BaseRenderOpenGL3D::setViewport(int left, int top, int right, int bottom) {
	BasePlatform::setRect(&_viewportRect, left, top, right, bottom);
	_viewport._x = left;
	_viewport._y = top;
	_viewport._width = right - left;
	_viewport._height = bottom - top;
	glViewport(left, top, right - left, bottom - top);
	return true;
}

// implements D3D SetViewport() for 3D renderer
bool BaseRenderOpenGL3D::setViewport3D(DXViewport *viewport) {
	_viewport = *viewport;
	glViewport(_viewport._x, _height - _viewport._height, _viewport._width, _viewport._height);
	return true;
}

bool BaseRenderOpenGL3D::setProjection2D() {
	DXMatrix matrix2D;
	DXMatrixIdentity(&matrix2D);
	DXMatrixOrthoOffCenterLH(&matrix2D, 0, _width, _height, 0, 0.0f, 1.0f);

	// convert DX [0, 1] depth range to OpenGL [-1, 1] depth range.
	matrix2D.matrix._33 = 2.0f;
	matrix2D.matrix._43 = -1.0f;

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(matrix2D);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return true;
}

// implements SetTransform() D3DTS_WORLD
bool BaseRenderOpenGL3D::setWorldTransform(const DXMatrix &transform) {
	_worldMatrix = transform;
	DXMatrix newModelViewTransform, world = transform;
	DXMatrixMultiply(&newModelViewTransform, &world, &_viewMatrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(newModelViewTransform);
	return true;
}

// implements SetTransform() D3DTS_WIEW
bool BaseRenderOpenGL3D::setViewTransform(const DXMatrix &transform) {
	_viewMatrix = transform;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(transform);
	return true;
}

// implements SetTransform() D3DTS_PROJECTION
bool BaseRenderOpenGL3D::setProjectionTransform(const DXMatrix &transform) {
	_projectionMatrix = transform;

	// convert DX [0, 1] depth range to OpenGL [-1, 1] depth range.
	DXMatrix finalMatrix = transform;
	float range = 2.0f / (_farClipPlane - _nearClipPlane);
	finalMatrix.matrix._33 = range;
	finalMatrix.matrix._43 = -(_nearClipPlane + _farClipPlane) * range / 2;

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(finalMatrix);

	glMatrixMode(GL_MODELVIEW);

	glViewport(_viewportRect.left, _height - _viewportRect.bottom, _viewportRect.width(), _viewportRect.height());

	return true;
}

void BaseRenderOpenGL3D::postfilter() {
	if (_postFilterMode == kPostFilterOff)
		return;

	setup2D();
	glViewport(0, 0, _width, _height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLfloat vertices[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};

	GLfloat texCoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	if (_postFilterMode == kPostFilterBlackAndWhite ||
		_postFilterMode == kPostFilterSepia) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_CULL_FACE);


		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _postfilterTexture);

		g_system->presentBuffer();
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, _width, _height, 0);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
		GLfloat grayscaleWeights[] = {0.333f, 0.333f, 0.333f};
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, grayscaleWeights);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		g_system->presentBuffer();
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, _width, _height, 0);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_SUBTRACT);
		GLfloat whiteColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, whiteColor);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		g_system->presentBuffer();
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, _width, _height, 0);
		
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		GLfloat luminanceWeights[] = { 0.65f, 0.65f, 0.65f };
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, luminanceWeights);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		if (_postFilterMode == kPostFilterSepia) {
			g_system->presentBuffer();
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, _width, _height, 0);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			GLfloat sepiaWeights[] = { 1.0f, 0.88f, 0.71f, 1.0f };
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, sepiaWeights);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
			glDrawArrays(GL_QUADS, 0, 4);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

BaseSurface *BaseRenderOpenGL3D::createSurface() {
	return new BaseSurfaceOpenGL3D(_game, this);
}

Mesh3DS *BaseRenderOpenGL3D::createMesh3DS() {
	return new Mesh3DSOpenGL(_game);
}

XMesh *BaseRenderOpenGL3D::createXMesh() {
	return new XMeshOpenGL(_game);
}

ShadowVolume *BaseRenderOpenGL3D::createShadowVolume() {
	return new ShadowVolumeOpenGL(_game);
}

// ScummVM specific ends <--

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME)
