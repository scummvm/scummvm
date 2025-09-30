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

#include "common/config-manager.h"

#include "engines/util.h"

#if defined(USE_TINYGL)

#include "engines/wintermute/base/gfx/3dutils.h"
#include "engines/wintermute/base/gfx/tinygl/base_render_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/base_surface_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/mesh3ds_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/meshx_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/shadow_volume_tinygl.h"

namespace Wintermute {

BaseRenderer3D *makeTinyGL3DRenderer(BaseGame *inGame) {
	return new BaseRenderTinyGL(inGame);
}

BaseRenderTinyGL::BaseRenderTinyGL(BaseGame *inGame) : BaseRenderer3D(inGame) {
	_flipInProgress = false;
}

BaseRenderTinyGL::~BaseRenderTinyGL() {
	_camera = nullptr; // ref only
	TinyGL::destroyContext();
}

bool BaseRenderTinyGL::initRenderer(int width, int height, bool windowed) {
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

	Graphics::PixelFormat pixelFormat = getPixelFormat();
	initGraphics(width, height, &pixelFormat);
	if (g_system->getScreenFormat() != pixelFormat) {
		warning("Couldn't setup GFX-backend for %dx%dx%d", width, height, pixelFormat.bytesPerPixel * 8);
		return false;
	}

	debug(2, "INFO: TinyGL front buffer pixel format: %s", pixelFormat.toString().c_str());
	TinyGL::createContext(width, height, pixelFormat, 512, true, false/*ConfMan.getBool("dirtyrects")*/, 64 * 1024 * 1024);

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

bool BaseRenderTinyGL::flip() {
	if (_flipInProgress) {
		return false;
	}
	_flipInProgress = true;

	_lastTexture = nullptr;

	postfilter();

	Common::List<Common::Rect> dirtyAreas;
	TinyGL::presentBuffer(dirtyAreas);

	Graphics::Surface glBuffer;
	TinyGL::getSurfaceRef(glBuffer);

	if (!dirtyAreas.empty()) {
		for (Common::List<Common::Rect>::iterator itRect = dirtyAreas.begin(); itRect != dirtyAreas.end(); ++itRect) {
			g_system->copyRectToScreen(glBuffer.getBasePtr((*itRect).left, (*itRect).top), glBuffer.pitch, (*itRect).left, (*itRect).top, (*itRect).width(), (*itRect).height());
		}
	}

	g_system->updateScreen();

	_state = RSTATE_NONE;

	_flipInProgress = false;
	return true;
}

bool BaseRenderTinyGL::clear() {
	if(!_game->_editorMode) {
		tglViewport(0, _height, _width, _height);
	}
	tglClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT | TGL_STENCIL_BUFFER_BIT);
	return true;
}

bool BaseRenderTinyGL::setup2D(bool force) {
	if (_state != RSTATE_2D || force) {
		_state = RSTATE_2D;

		tglDisable(TGL_LIGHTING);
		tglDisable(TGL_DEPTH_TEST);

		tglEnable(TGL_BLEND);
		setSpriteBlendMode(Graphics::BLEND_NORMAL);

		tglEnable(TGL_ALPHA_TEST);
		tglAlphaFunc(TGL_GEQUAL, 0.0f);

		tglFrontFace(TGL_CCW);  // WME DX have CW
		tglEnable(TGL_CULL_FACE);
		tglDisable(TGL_STENCIL_TEST);

		tglDisable(TGL_FOG);
	}

	return true;
}

bool BaseRenderTinyGL::setup3D(Camera3D *camera, bool force) {
	if (_state != RSTATE_3D || force) {
		_state = RSTATE_3D;

		tglEnable(TGL_NORMALIZE);

		tglEnable(TGL_DEPTH_TEST);
		tglEnable(TGL_LIGHTING);
		tglEnable(TGL_ALPHA_TEST);
		// WME uses 8 as a reference value and Direct3D expects it to be in the range [0, 255]
		tglAlphaFunc(TGL_GEQUAL, 8 / 255.0f);

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
		tglEnable(TGL_LIGHTING);

		for (int i = 0; i < getMaxActiveLights(); ++i) {
			tglLightfv(TGL_LIGHT0 + i, TGL_POSITION, _lightPositions[i]);
			tglLightfv(TGL_LIGHT0 + i, TGL_SPOT_DIRECTION, _lightDirections[i]);
		}

		// fog
		bool fogEnabled;
		uint32 fogColor;
		float fogStart, fogEnd;
		_game->getFogParams(&fogEnabled, &fogColor, &fogStart, &fogEnd);
		if (fogEnabled) {
			tglEnable(TGL_FOG);
			TGLfloat color[4] = { RGBCOLGetR(fogColor) / 255.0f,
			                      RGBCOLGetG(fogColor) / 255.0f,
			                      RGBCOLGetB(fogColor) / 255.0f,
			                      RGBCOLGetA(fogColor) / 255.0f };
			tglFogfv(TGL_FOG_COLOR, color);
			tglFogi(TGL_FOG_MODE, TGL_LINEAR);
			tglFogf(TGL_FOG_START, fogStart);
			tglFogf(TGL_FOG_END, fogEnd);

		} else {
			tglDisable(TGL_FOG);
		}

		setProjection();
	}

	return true;
}

void BaseRenderTinyGL::setAmbientLightRenderState() {
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
	tglLightModelfv(TGL_LIGHT_MODEL_AMBIENT, value);
}

bool BaseRenderTinyGL::setupLines() {
	if (_state != RSTATE_LINES) {
		_state = RSTATE_LINES;

		float value[] = { 0, 0, 0, 0 };

		tglDisable(TGL_LIGHTING);
		tglDisable(TGL_DEPTH_TEST);
		tglFrontFace(TGL_CW); // WME DX have CCW
		tglEnable(TGL_CULL_FACE);
		tglEnable(TGL_BLEND);
		tglEnable(TGL_ALPHA_TEST);
		tglLightModelfv(TGL_LIGHT_MODEL_AMBIENT, value);

		tglBindTexture(TGL_TEXTURE_2D, 0);
		tglDisable(TGL_TEXTURE_2D);
		_lastTexture = nullptr;
	}

	return true;
}

bool BaseRenderTinyGL::drawSpriteEx(BaseSurface *tex, const Common::Rect32 &rect,
	                            const DXVector2 &pos, const DXVector2 &rot,
	                            const DXVector2 &scale,
	                            float angle, uint32 color, bool alphaDisable,
	                            Graphics::TSpriteBlendMode blendMode,
	                            bool mirrorX, bool mirrorY) {
	BaseSurfaceTinyGL *texture = dynamic_cast<BaseSurfaceTinyGL *>(tex);
	if (!texture)
		return false;

	if (_forceAlphaColor != 0) {
		color = _forceAlphaColor;
	}

	float width = (rect.right - rect.left) * scale._x;
	float height = (rect.bottom - rect.top) * scale._y;

	float texLeft = (float)rect.left / (float)width;
	float texTop = (float)rect.top / (float)height;
	float texRight = (float)rect.right / (float)width;
	float texBottom = (float)rect.bottom / (float)height;

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
		tglDisable(TGL_ALPHA_TEST);
		tglDisable(TGL_BLEND);
	}

	if (_lastTexture != texture) {
		_lastTexture = texture;
	}

	tglViewport(0, 0, _width, _height);
	setProjection2D();

	tglFrontFace(TGL_CW);

	TinyGL::BlitImage *blitImage = texture->getBlitImage();
	int blitImageWidth, blitImageHeight;
	tglGetBlitImageSize(blitImage, blitImageWidth, blitImageHeight);

	int posX = _viewportRect.left + vertices[1].x;
	int posY = _viewportRect.top + vertices[1].y;
	TinyGL::BlitTransform transform(posX, posY);

	int srcX = (int)(texLeft * width);
	int srcY = (int)(texTop * height);
	int srcW = (int)((texRight - texLeft) * width);
	int srcH = (int)((texBottom - texTop) * height);

	transform.sourceRectangle(srcX, srcY, srcW, srcH);
	if (scale._x != 1.0f || scale._y != 1.0f) {
		transform.scale(width, rect.bottom - rect.top);
	}
	tglBlit(blitImage, transform);

	if (alphaDisable) {
		tglEnable(TGL_ALPHA_TEST);
		tglEnable(TGL_BLEND);
	}

	return true;
}

bool BaseRenderTinyGL::commitSpriteBatch() {
	// nothing to implement
	return true;
}

bool BaseRenderTinyGL::startSpriteBatch() {
	// nothing to implement
	return true;
}

bool BaseRenderTinyGL::endSpriteBatch() {
	// nothing to implement
	return true;
}

DXMatrix *BaseRenderTinyGL::buildMatrix(DXMatrix* out, const DXVector2 *centre, const DXVector2 *scaling, float angle) {
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

void BaseRenderTinyGL::transformVertices(struct SpriteVertex *vertices, const DXVector2 *centre, const DXVector2 *scaling, float angle) {
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

bool BaseRenderTinyGL::setProjection() {
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

bool BaseRenderTinyGL::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
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

	tglViewport(0, 0, _width, _height);
	setProjection2D();

	tglColor4ub(r, g, b, a);

	tglEnableClientState(TGL_VERTEX_ARRAY);

	tglVertexPointer(3, TGL_FLOAT, sizeof(RectangleVertex), &vertices[0].x);

	tglDrawArrays(TGL_LINES, 0, 2);

	tglDisableClientState(TGL_VERTEX_ARRAY);

	return true;
}

bool BaseRenderTinyGL::fillRect(int x, int y, int w, int h, uint32 color) {
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

	tglViewport(0, 0, _width, _height);
	setProjection2D();

	tglColor4ub(r, g, b, a);

	tglEnableClientState(TGL_VERTEX_ARRAY);

	tglVertexPointer(3, TGL_FLOAT, sizeof(RectangleVertex), &vertices[0].x);

	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglDisableClientState(TGL_VERTEX_ARRAY);

	setup2D();
	return true;
}

bool BaseRenderTinyGL::fadeToColor(byte r, byte g, byte b, byte a) {
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


	tglEnable(TGL_BLEND);
	setSpriteBlendMode(Graphics::BLEND_NORMAL);

	tglDisable(TGL_DEPTH_TEST);
	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDisable(TGL_TEXTURE_2D);
	_lastTexture = nullptr;

	tglViewport(0, 0, _width, _height);
	setProjection2D();

	tglColor4ub(r, g, b, a);

	tglEnableClientState(TGL_VERTEX_ARRAY);

	tglVertexPointer(3, TGL_FLOAT, sizeof(RectangleVertex), &vertices[0].x);

	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglDisableClientState(TGL_VERTEX_ARRAY);

	setup2D(true);

	return true;
}

BaseImage *BaseRenderTinyGL::takeScreenshot(int newWidth, int newHeight) {
	BaseImage *screenshot = new BaseImage();
	Graphics::Surface *surface = TinyGL::copyFromFrameBuffer(Graphics::PixelFormat::createFormatRGBA32());
	screenshot->copyFrom(surface, newWidth, newHeight);
	delete surface;
	return screenshot;
}

bool BaseRenderTinyGL::enableShadows() {
	return true;
}

bool BaseRenderTinyGL::disableShadows() {
	return true;
}

void BaseRenderTinyGL::displaySimpleShadow(BaseObject *object) {
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

	tglFrontFace(TGL_CCW);

	tglDepthMask(TGL_FALSE);
	tglEnable(TGL_TEXTURE_2D);
	static_cast<BaseSurfaceTinyGL *>(shadowImage)->setTexture();

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_NORMAL_ARRAY);
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);

	tglVertexPointer(3, TGL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].x);
	tglNormalPointer(TGL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].nx);
	tglTexCoordPointer(2, TGL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].u);

	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisableClientState(TGL_NORMAL_ARRAY);
	tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);

	tglDisable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_TRUE);
}

void BaseRenderTinyGL::setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode, bool forceChange) {
	if (blendMode == _blendMode && !forceChange)
		return;

	_blendMode = blendMode;

	switch (_blendMode) {
	case Graphics::BLEND_NORMAL:
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
		break;

	case Graphics::BLEND_ADDITIVE:
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE);
		break;

	case Graphics::BLEND_SUBTRACTIVE:
		tglBlendFunc(TGL_ZERO, TGL_ONE_MINUS_SRC_COLOR);
		break;

	default:
		break;
	}
}

bool BaseRenderTinyGL::stencilSupported() {
	// assume that we have a stencil buffer
	return true;
}

int BaseRenderTinyGL::getMaxActiveLights() {
	TGLint maxLightCount = 0;
	tglGetIntegerv(TGL_MAX_LIGHTS, &maxLightCount);
	return maxLightCount;
}

bool BaseRenderTinyGL::invalidateTexture(BaseSurface *texture) {
	if (_lastTexture == texture)
		_lastTexture = nullptr;

	return true;
}

bool BaseRenderTinyGL::invalidateDeviceObjects() {
	return STATUS_OK;
}

bool BaseRenderTinyGL::restoreDeviceObjects() {
	return STATUS_OK;
}

bool BaseRenderTinyGL::resetDevice() {
	return STATUS_OK;
}

// implements D3D LightEnable()
void BaseRenderTinyGL::lightEnable(int index, bool enable) {
	if (enable)
		tglEnable(TGL_LIGHT0 + index);
	else
		tglDisable(TGL_LIGHT0 + index);
}

// backend layer 3DLight::SetLight
void BaseRenderTinyGL::setLightParameters(int index, const DXVector3 &position, const DXVector3 &direction, const DXVector4 &diffuse, bool spotlight) {
	float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	tglLightfv(TGL_LIGHT0 + index, TGL_DIFFUSE, diffuse);
	tglLightfv(TGL_LIGHT0 + index, TGL_AMBIENT, zero);
	tglLightfv(TGL_LIGHT0 + index, TGL_SPECULAR, zero);

	_lightPositions[index]._x = position._x;
	_lightPositions[index]._y = position._y;
	_lightPositions[index]._z = position._z;
	_lightPositions[index]._w = 1.0f;

	if (spotlight) {
		_lightDirections[index] = direction;
		tglLightfv(TGL_LIGHT0 + index, TGL_SPOT_DIRECTION, direction);

		tglLightf(TGL_LIGHT0 + index, TGL_SPOT_EXPONENT, 0.0f);
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
		tglLightf(TGL_LIGHT0 + index, TGL_SPOT_CUTOFF, 0.5f * (180.0f / (float)M_PI));
	} else {
		tglLightf(TGL_LIGHT0 + index, TGL_SPOT_CUTOFF, 180.0f);
	}
}

// backend layer AdSceneGeometry::Render
void BaseRenderTinyGL::renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
	                                   const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) {
	DXMatrix matIdentity;
	DXMatrixIdentity(&matIdentity);

	if (camera)
		_game->_renderer3D->setup3D(camera, true);

	setWorldTransform(matIdentity);

	tglDisable(TGL_LIGHTING);
	tglDisable(TGL_DEPTH_TEST);
	tglFrontFace(TGL_CW); // WME DX have CCW
	tglEnable(TGL_BLEND);
	tglDisable(TGL_ALPHA_TEST);
	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDisable(TGL_TEXTURE_2D);

	tglColorMaterial(TGL_FRONT_AND_BACK, TGL_DIFFUSE);
	tglEnable(TGL_COLOR_MATERIAL);


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

	tglDisable(TGL_COLOR_MATERIAL);
}

// backend layer 3DShadowVolume::Render()
void BaseRenderTinyGL::renderShadowGeometry(const BaseArray<AdWalkplane *> &planes,
	                                    const BaseArray<AdBlock *> &blocks,
	                                    const BaseArray<AdGeneric *> &generics, Camera3D *camera) {
	DXMatrix matIdentity;
	DXMatrixIdentity(&matIdentity);

	if (camera)
		_game->_renderer3D->setup3D(camera, true);

	setWorldTransform(matIdentity);

	// disable color write
	setSpriteBlendMode(Graphics::BLEND_UNKNOWN);
	tglBlendFunc(TGL_ZERO, TGL_ONE);
	tglColorMask(TGL_FALSE, TGL_FALSE, TGL_FALSE, TGL_FALSE);

	// no texture
	_lastTexture = nullptr;
	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDisable(TGL_TEXTURE_2D);

	tglFrontFace(TGL_CW); // WME DX have CCW

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
	tglColorMask(TGL_TRUE, TGL_TRUE, TGL_TRUE, TGL_TRUE);
}

// implements D3D SetRenderState() D3DRS_CULLMODE - CCW
void BaseRenderTinyGL::enableCulling() {
	tglFrontFace(TGL_CW); // WME DX have CCW
	tglEnable(TGL_CULL_FACE);
}

// implements D3D SetRenderState() D3DRS_CULLMODE - NONE
void BaseRenderTinyGL::disableCulling() {
	tglDisable(TGL_CULL_FACE);
}

// implements D3D SetViewport() for 2D renderer
bool BaseRenderTinyGL::setViewport(int left, int top, int right, int bottom) {
	BasePlatform::setRect(&_viewportRect, left, top, right, bottom);
	_viewport._x = left;
	_viewport._y = top;
	_viewport._width = right - left;
	_viewport._height = bottom - top;
	tglViewport(left, top, right - left, bottom - top);
	return true;
}

// implements D3D SetViewport() for 3D renderer
bool BaseRenderTinyGL::setViewport3D(DXViewport *viewport) {
	_viewport = *viewport;
	tglViewport(_viewport._x, _height - _viewport._height, _viewport._width, _viewport._height);
	return true;
}

bool BaseRenderTinyGL::setProjection2D() {
	DXMatrix matrix2D;
	DXMatrixIdentity(&matrix2D);
	DXMatrixOrthoOffCenterLH(&matrix2D, 0, _width, _height, 0, 0.0f, 1.0f);

	// convert DX [0, 1] depth range to OpenGL [-1, 1] depth range.
	matrix2D.matrix._33 = 2.0f;
	matrix2D.matrix._43 = -1.0f;

	tglMatrixMode(TGL_PROJECTION);
	tglLoadMatrixf(matrix2D);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	return true;
}

// implements SetTransform() D3DTS_WORLD
bool BaseRenderTinyGL::setWorldTransform(const DXMatrix &transform) {
	_worldMatrix = transform;
	DXMatrix newModelViewTransform, world = transform;
	DXMatrixMultiply(&newModelViewTransform, &world, &_viewMatrix);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadMatrixf(newModelViewTransform);
	return true;
}

// implements SetTransform() D3DTS_WIEW
bool BaseRenderTinyGL::setViewTransform(const DXMatrix &transform) {
	_viewMatrix = transform;
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadMatrixf(transform);
	return true;
}

// implements SetTransform() D3DTS_PROJECTION
bool BaseRenderTinyGL::setProjectionTransform(const DXMatrix &transform) {
	_projectionMatrix = transform;

	// convert DX [0, 1] depth range to OpenGL [-1, 1] depth range.
	DXMatrix finalMatrix = transform;
	float range = 2.0f / (_farClipPlane - _nearClipPlane);
	finalMatrix.matrix._33 = range;
	finalMatrix.matrix._43 = -(_nearClipPlane + _farClipPlane) * range / 2;

	tglMatrixMode(TGL_PROJECTION);
	tglLoadMatrixf(finalMatrix);

	tglMatrixMode(TGL_MODELVIEW);

	tglViewport(_viewportRect.left, _height - _viewportRect.bottom, _viewportRect.width(), _viewportRect.height());

	return true;
}

void BaseRenderTinyGL::postfilter() {
	if (_postFilterMode == kPostFilterOff)
		return;

	// not supproted under TinyGL
}

BaseSurface *BaseRenderTinyGL::createSurface() {
	return new BaseSurfaceTinyGL(_game, this);
}

Mesh3DS *BaseRenderTinyGL::createMesh3DS() {
	return new Mesh3DSTinyGL(_game);
}

XMesh *BaseRenderTinyGL::createXMesh() {
	return new XMeshTinyGL(_game);
}

ShadowVolume *BaseRenderTinyGL::createShadowVolume() {
	return new ShadowVolumeTinyGL(_game);
}

// ScummVM specific ends <--

} // namespace Wintermute

#endif // defined(USE_TINYGL)
