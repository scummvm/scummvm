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
#include "engines/wintermute/base/gfx/3dlight.h"

#include "graphics/opengl/system_headers.h"

#include "common/config-manager.h"

#include "math/glmath.h"

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
	setDefaultAmbientLightColor();

	_lightPositions.resize(getMaxActiveLights());
	_lightDirections.resize(getMaxActiveLights());
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



	_windowed = !ConfMan.getBool("fullscreen");
	_width = width;
	_height = height;

	setViewport(0, 0, width, height);

	setProjection();

	_active = true;

	return true;
}

bool BaseRenderOpenGL3D::flip() {
	_lastTexture = nullptr;
	// Disable blend mode to prevent interfere with backend renderer
	bool prevStateBlend = glIsEnabled(GL_BLEND);
	glDisable(GL_BLEND);
	g_system->updateScreen();
	prevStateBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	return true;
}

bool BaseRenderOpenGL3D::fill(byte r, byte g, byte b, Common::Rect *rect) {
	if(!_gameRef->_editorMode) {
		glViewport(0, _height, _width, _height);
	}
	glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

bool BaseRenderOpenGL3D::setup2D(bool force) {
	if (_state != RSTATE_2D || force) {
		_state = RSTATE_2D;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		setSpriteBlendMode(Graphics::BLEND_NORMAL);

		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.0f);

		glPolygonMode(GL_FRONT, GL_FILL);
		glFrontFace(GL_CCW);  // WME DX have CW
		glEnable(GL_CULL_FACE);
		glDisable(GL_STENCIL_TEST);

		glDisable(GL_FOG);

		//
		// WME has this. It's unknown if it's really needed
		// Support this would require higher OpenGL version
		//
		// D3DTSS_COLOROP               = D3DTOP_MODULATE
		// D3DTSS_COLORARG1             = D3DTA_TEXTURE
		// D3DTSS_COLORARG              = D3DTA_DIFFUSE
		// D3DTSS_ALPHAOP               = D3DTOP_MODULATE
		// D3DTSS_ALPHAARG1             = D3DTA_TEXTURE
		// D3DTSS_ALPHAARG2             = D3DTA_DIFFUSE
		// D3DTSS_MIPFILTER             = D3DTEXF_NONE
		// D3DTSS_TEXCOORDINDEX         = 0
		// D3DTSS_TEXTURETRANSFORMFLAGS = D3DTTFF_DISABLE
	}

	return true;
}

bool BaseRenderOpenGL3D::setup3D(Camera3D *camera, bool force) {
	if (_state != RSTATE_3D || force) {
		_state = RSTATE_3D;

		glEnable(GL_NORMALIZE);

		//
		// WME has this. It's unknown if it's really needed
		// Support this would require higher OpenGL version
		//
		// D3DTSS_COLORARG1 = D3DTA_TEXTURE
		// D3DTSS_COLORARG2 = D3DTA_CURRENT
		// D3DTSS_COLOROP   = D3DTOP_MODULATE
		// D3DTSS_ALPHAOP   = D3DTOP_SELECTARG1

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_ALPHA_TEST);
		// WME uses 8 as a reference value and Direct3D expects it to be in the range [0, 255]
		glAlphaFunc(GL_GEQUAL, 8 / 255.0f);

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
		_gameRef->getFogParams(&fogEnabled, &fogColor, &fogStart, &fogEnd);
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
		uint32 color = _gameRef->getAmbientLightColor();

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

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glFrontFace(GL_CW); // WME DX have CCW
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, value);

		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		_lastTexture = nullptr;
	}

	return true;
}

bool BaseRenderOpenGL3D::drawSpriteEx(BaseSurface *tex, const Wintermute::Rect32 &rect,
								  const Wintermute::Vector2 &pos, const Wintermute::Vector2 &rot, const Wintermute::Vector2 &scale,
								  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
								  bool mirrorX, bool mirrorY) {

	BaseSurfaceOpenGL3D *texture = dynamic_cast<BaseSurfaceOpenGL3D *>(tex);

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
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;
		vertices[i].a = a;
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
			glDisable(GL_ALPHA_TEST);
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

		setProjection2D();

		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		
		glVertexPointer(3, GL_FLOAT, sizeof(SpriteVertex), &vertices[0].x);
		glTexCoordPointer(2, GL_FLOAT, sizeof(SpriteVertex), &vertices[0].u);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SpriteVertex), &vertices[0].r);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		
		if (alphaDisable) {
			glEnable(GL_ALPHA_TEST);
			glEnable(GL_BLEND);
		}
	}

	return true;
}

bool BaseRenderOpenGL3D::commitSpriteBatch() {
	// render
	setSpriteBlendMode(_batchBlendMode);
	if (_batchAlphaDisable) {
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
	}

	if (_lastTexture != _batchTexture) {
		_lastTexture = _batchTexture;
		glBindTexture(GL_TEXTURE_2D, _batchTexture->getTextureName());
	}

	// TODO

	if (_batchAlphaDisable) {
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
	}

	return true;
}

bool BaseRenderOpenGL3D::startSpriteBatch() {
	//_spriteBatchMode = true;
	return true;
}

bool BaseRenderOpenGL3D::endSpriteBatch() {
	if (!_spriteBatchMode)
		return false;

	_spriteBatchMode = false;
	return commitSpriteBatch();
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

bool BaseRenderOpenGL3D::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	x1 += _drawOffsetX;
	x2 += _drawOffsetX;
	y1 += _drawOffsetY;
	y2 += _drawOffsetY;

	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	glBegin(GL_LINES);
		glColor4ub(r, g, b, a);
		glVertex3f(x1, _height - y1, 0.9f);
		glVertex3f(x2, _height - y2, 0.9f);
	glEnd();

	return true;
}

void BaseRenderOpenGL3D::fadeToColor(byte r, byte g, byte b, byte a) {
	const int vertexSize = 16;
	byte vertices[4 * vertexSize];
	float *vertexCoords = reinterpret_cast<float *>(vertices);

	vertexCoords[0 * 4 + 1] = _viewportRect.left;
	vertexCoords[0 * 4 + 2] = _viewportRect.bottom;
	vertexCoords[0 * 4 + 3] = 0.0f;
	vertexCoords[1 * 4 + 1] = _viewportRect.left;
	vertexCoords[1 * 4 + 2] = _viewportRect.top;
	vertexCoords[1 * 4 + 3] = 0.0f;
	vertexCoords[2 * 4 + 1] = _viewportRect.right;
	vertexCoords[2 * 4 + 2] = _viewportRect.bottom;
	vertexCoords[2 * 4 + 3] = 0.0f;
	vertexCoords[3 * 4 + 1] = _viewportRect.right;
	vertexCoords[3 * 4 + 2] = _viewportRect.top;
	vertexCoords[3 * 4 + 3] = 0.0f;

	vertices[0 * vertexSize + 0] = r;
	vertices[0 * vertexSize + 1] = g;
	vertices[0 * vertexSize + 2] = b;
	vertices[0 * vertexSize + 3] = a;
	vertices[1 * vertexSize + 0] = r;
	vertices[1 * vertexSize + 1] = g;
	vertices[1 * vertexSize + 2] = b;
	vertices[1 * vertexSize + 3] = a;
	vertices[2 * vertexSize + 0] = r;
	vertices[2 * vertexSize + 1] = g;
	vertices[2 * vertexSize + 2] = b;
	vertices[2 * vertexSize + 3] = a;
	vertices[3 * vertexSize + 0] = r;
	vertices[3 * vertexSize + 1] = g;
	vertices[3 * vertexSize + 2] = b;
	vertices[3 * vertexSize + 3] = a;

	setSpriteBlendMode(Graphics::BLEND_UNKNOWN);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	_lastTexture = nullptr;

	setProjection2D();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, vertexSize, vertices + 4);
	glColorPointer(4, GL_UNSIGNED_BYTE, vertexSize, vertices);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	setup2D(true);
}

BaseImage *BaseRenderOpenGL3D::takeScreenshot() {
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

bool BaseRenderOpenGL3D::enableShadows() {
	warning("BaseRenderOpenGL3D::enableShadows not implemented yet");
	return true;
}

bool BaseRenderOpenGL3D::disableShadows() {
	warning("BaseRenderOpenGL3D::disableShadows not implemented yet");
	return true;
}

void BaseRenderOpenGL3D::displayShadow(BaseObject *object, const DXVector3 *lightPos, bool lightPosRelative) {
	BaseSurface *shadowImage;
	if (object->_shadowImage) {
		shadowImage = object->_shadowImage;
	} else {
		shadowImage = _gameRef->_shadowImage;
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

bool BaseRenderOpenGL3D::stencilSupported() {
	// assume that we have a stencil buffer
	return true;
}

int BaseRenderOpenGL3D::getMaxActiveLights() {
	GLint maxLightCount = 0;
	glGetIntegerv(GL_MAX_LIGHTS, &maxLightCount);
	return maxLightCount;
}

bool BaseRenderOpenGL3D::invalidateTexture(BaseSurfaceOpenGL3D *texture) {
	if (_lastTexture == texture)
		_lastTexture = nullptr;
	if (_batchTexture == texture)
		_batchTexture = nullptr;

	return true;
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
		_gameRef->_renderer3D->setup3D(camera, true);

	setWorldTransform(matIdentity);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CW); // WME DX have CCW
	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);


	// render walk planes
	for (uint i = 0; i < planes.size(); i++) {
		if (planes[i]->_active) {
			planes[i]->_mesh->render();
		}
	}

	// render blocks
	for (uint i = 0; i < blocks.size(); i++) {
		if (blocks[i]->_active) {
			blocks[i]->_mesh->render();
		}
	}

	// render generic objects
	for (uint i = 0; i < generics.size(); i++) {
		if (generics[i]->_active) {
			generics[i]->_mesh->render();
		}
	}

	for (uint i = 0; i < lights.size(); ++i) {
		if (lights[i]->_active) {
			glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 0.0f);
			DXVector3 right = lights[i]->_pos + DXVector3(1000.0f, 0.0f, 0.0f);
			DXVector3 up = lights[i]->_pos + DXVector3(0.0f, 1000.0f, 0.0f);
			DXVector3 backward = lights[i]->_pos + DXVector3(0.0f, 0.0f, 1000.0f);
			DXVector3 left = lights[i]->_pos + DXVector3(-1000.0f, 0.0f, 0.0f);
			DXVector3 down = lights[i]->_pos + DXVector3(0.0f, -1000.0f, 0.0f);
			DXVector3 forward = lights[i]->_pos + DXVector3(0.0f, 0.0f, -1000.0f);

			glVertex3fv(lights[i]->_pos);
			glVertex3fv(right);
			glVertex3fv(lights[i]->_pos);
			glVertex3fv(up);
			glVertex3fv(lights[i]->_pos);
			glVertex3fv(backward);
			glVertex3fv(lights[i]->_pos);
			glVertex3fv(left);
			glVertex3fv(lights[i]->_pos);
			glVertex3fv(down);
			glVertex3fv(lights[i]->_pos);
			glVertex3fv(forward);
			glEnd();
		}
	}

	glDisable(GL_COLOR_MATERIAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// backend layer 3DShadowVolume::Render()
void BaseRenderOpenGL3D::renderShadowGeometry(const BaseArray<AdWalkplane *> &planes,
											  const BaseArray<AdBlock *> &blocks,
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
	_viewportRect.setRect(left, top, right, bottom);
	_viewport._x = left;
	_viewport._y = top;
	_viewport._width = right - left;
	_viewport._height = bottom - top;
	glViewport(left, _height - bottom, right - left, bottom - top);
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
	DXMatrixOrthoOffCenterLH(&matrix2D, 0, _width, 0, _height, 0.0f, 1.0f);

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

BaseSurface *Wintermute::BaseRenderOpenGL3D::createSurface() {
	return new BaseSurfaceOpenGL3D(_gameRef, this);
}

Mesh3DS *BaseRenderOpenGL3D::createMesh3DS() {
	return new Mesh3DSOpenGL(_gameRef);
}

XMesh *BaseRenderOpenGL3D::createXMesh() {
	return new XMeshOpenGL(_gameRef);
}

ShadowVolume *BaseRenderOpenGL3D::createShadowVolume() {
	return new ShadowVolumeOpenGL(_gameRef);
}

// ScummVM specific ends <--

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME)
