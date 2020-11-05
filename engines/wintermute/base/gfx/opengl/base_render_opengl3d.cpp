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
#include "engines/wintermute/base/gfx/3ds/light3d.h"
#include "graphics/opengl/system_headers.h"
#include "math/glmath.h"

#if (defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)) && !defined(USE_GLES2)

#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume_opengl.h"

#if defined(__MINGW32__) && defined (SDL_BACKEND) && !defined(USE_GLEW)
// We need SDL.h for SDL_GL_GetProcAddress.
#include "backends/platform/sdl/sdl-sys.h"
PFNGLACTIVETEXTUREPROC glActiveTexturePtr;
#endif

namespace Wintermute {
BaseRenderer3D *makeOpenGL3DRenderer(BaseGame *inGame) {
	return new BaseRenderOpenGL3D(inGame);
}

BaseRenderOpenGL3D::BaseRenderOpenGL3D(BaseGame *inGame)
	: BaseRenderer3D(inGame), _spriteBatchMode(false)  {
	setDefaultAmbientLightColor();

	_lightPositions.resize(maximumLightsCount());
	_lightDirections.resize(maximumLightsCount());
	(void)_spriteBatchMode; // silence warning

#if defined(__MINGW32__) && defined (SDL_BACKEND) && !defined(USE_GLEW)
	union {
		void *obj_ptr;
		void (APIENTRY *func_ptr)();
	} u;
	// We're casting from an object pointer to a function pointer, the
	// sizes need to be the same for this to work.
	assert(sizeof(u.obj_ptr) == sizeof(u.func_ptr));
	u.obj_ptr = SDL_GL_GetProcAddress("glActiveTexture");
	glActiveTexturePtr = (PFNGLACTIVETEXTUREPROC)u.func_ptr;
	assert(glActiveTexturePtr);
#endif
}

BaseRenderOpenGL3D::~BaseRenderOpenGL3D() {
}

void BaseRenderOpenGL3D::setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode) {
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
		warning("BaseRenderOpenGL3D::setSpriteBlendMode unsupported blend mode %i", blendMode);
	}
}

void BaseRenderOpenGL3D::setAmbientLight() {
	byte a = 0;
	byte r = 0;
	byte g = 0;
	byte b = 0;

	if (_overrideAmbientLightColor) {
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

int BaseRenderOpenGL3D::maximumLightsCount() {
	GLint maxLightCount = 0;
	glGetIntegerv(GL_MAX_LIGHTS, &maxLightCount);
	return maxLightCount;
}

void BaseRenderOpenGL3D::enableLight(int index) {
	glEnable(GL_LIGHT0 + index);
}

void BaseRenderOpenGL3D::disableLight(int index) {
	glDisable(GL_LIGHT0 + index);
}

void BaseRenderOpenGL3D::setLightParameters(int index, const Math::Vector3d &position, const Math::Vector3d &direction, const Math::Vector4d &diffuse, bool spotlight) {
	float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	glLightfv(GL_LIGHT0 + index, GL_DIFFUSE, diffuse.getData());
	glLightfv(GL_LIGHT0 + index, GL_AMBIENT, zero);
	glLightfv(GL_LIGHT0 + index, GL_SPECULAR, zero);

	_lightPositions[index].x() = position.x();
	_lightPositions[index].y() = position.y();
	_lightPositions[index].z() = position.z();
	_lightPositions[index].w() = 1.0f;

	if (spotlight) {
		_lightDirections[index] = direction;
		glLightfv(GL_LIGHT0 + index, GL_SPOT_DIRECTION, direction.getData());

		glLightf(GL_LIGHT0 + index, GL_SPOT_EXPONENT, 0.0f);
		// wme sets the phi angle to 1.0 (in radians)
		// so either 180/pi or (180/pi)/2 should give the same result
		glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, (180.0f / (float)M_PI));
	} else {
		glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180.0f);
	}
}

void BaseRenderOpenGL3D::enableCulling() {
	glEnable(GL_CULL_FACE);
}

void BaseRenderOpenGL3D::disableCulling() {
	glDisable(GL_CULL_FACE);
}

bool BaseRenderOpenGL3D::enableShadows() {
	warning("BaseRenderOpenGL3D::enableShadows not implemented yet");
	return true;
}

bool BaseRenderOpenGL3D::disableShadows() {
	warning("BaseRenderOpenGL3D::disableShadows not implemented yet");
	return true;
}

void BaseRenderOpenGL3D::displayShadow(BaseObject *object, const Math::Vector3d &lightPos, bool lightPosRelative) {
	BaseSurface *shadowImage = _gameRef->_shadowImage;

	if (object->_shadowImage) {
		shadowImage = object->_shadowImage;
	}

	if (!shadowImage) {
		return;
	}

	Math::Matrix4 scale;
	scale.setToIdentity();
	scale(0, 0) = object->_shadowSize * object->_scale3D;
	scale(1, 1) = 1.0f;
	scale(2, 2) = object->_shadowSize * object->_scale3D;

	float sinOfAngle = object->_angle.getSine();
	float cosOfAngle = object->_angle.getCosine();
	Math::Matrix4 rotation;
	rotation.setToIdentity();
	rotation(0, 0) = cosOfAngle;
	rotation(0, 2) = sinOfAngle;
	rotation(2, 0) = -sinOfAngle;
	rotation(2, 2) = cosOfAngle;
	Math::Matrix4 translation;
	translation.setToIdentity();
	translation.setPosition(object->_posVector);

	Math::Matrix4 worldTransformation = translation * rotation * scale;
	worldTransformation.transpose();
	worldTransformation = worldTransformation * _lastViewMatrix;

	glLoadMatrixf(worldTransformation.getData());

	glDepthMask(false);
	glEnable(GL_TEXTURE_2D);
	static_cast<BaseSurfaceOpenGL3D *>(shadowImage)->setTexture();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].x);
	glNormalPointer(GL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].nx);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SimpleShadowVertex), &_simpleShadow[0].u);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDepthMask(true);
	glLoadMatrixf(_lastViewMatrix.getData());
}

bool BaseRenderOpenGL3D::stencilSupported() {
	// assume that we have a stencil buffer
	return true;
}

BaseImage *BaseRenderOpenGL3D::takeScreenshot() {
	warning("BaseRenderOpenGL3D::takeScreenshot not yet implemented");
	return nullptr;
}

bool BaseRenderOpenGL3D::saveScreenShot(const Common::String &filename, int sizeX, int sizeY) {
	warning("BaseRenderOpenGL3D::saveScreenshot not yet implemented");
	return true;
}

void BaseRenderOpenGL3D::setWindowed(bool windowed) {
	warning("BaseRenderOpenGL3D::setWindowed not yet implemented");
}

void BaseRenderOpenGL3D::fadeToColor(byte r, byte g, byte b, byte a) {
	setProjection2D();

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

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, vertexSize, vertices + 4);
	glColorPointer(4, GL_UNSIGNED_BYTE, vertexSize, vertices);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	setup2D(true);
}

bool BaseRenderOpenGL3D::fill(byte r, byte g, byte b, Common::Rect *rect) {
	glClearColor(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

bool BaseRenderOpenGL3D::setViewport(int left, int top, int right, int bottom) {
	_viewportRect.setRect(left, top, right, bottom);
	glViewport(left, _height - bottom, right - left, bottom - top);
	return true;
}

bool BaseRenderOpenGL3D::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	glBegin(GL_LINES);
	glColor4b(r, g, b, a);
	glVertex3f(x1, y1, 0.9f);
	glVertex3f(x2, y2, 0.9f);
	glEnd();

	return true;
}

bool BaseRenderOpenGL3D::drawRect(int x1, int y1, int x2, int y2, uint32 color, int width) {
	warning("BaseRenderOpenGL3D::drawRect not yet implemented");
	return true;
}

bool BaseRenderOpenGL3D::setProjection() {
	// is the viewport already set here?
	float viewportWidth = _viewportRect.right - _viewportRect.left;
	float viewportHeight = _viewportRect.bottom - _viewportRect.top;

	float verticalViewAngle = _fov;
	float aspectRatio = float(viewportWidth) / float(viewportHeight);
	float top = _nearPlane * tanf(verticalViewAngle * 0.5f);

	float scaleMod = static_cast<float>(_height) / static_cast<float>(viewportHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-top * aspectRatio, top * aspectRatio, -top, top, _nearPlane, _farPlane);
	glGetFloatv(GL_PROJECTION_MATRIX, _projectionMatrix3d.getData());

	_projectionMatrix3d(0, 0) *= scaleMod;
	_projectionMatrix3d(1, 1) *= scaleMod;

	glLoadMatrixf(_projectionMatrix3d.getData());

	glMatrixMode(GL_MODELVIEW);
	return true;
}

bool BaseRenderOpenGL3D::setProjection2D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, -1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	return true;
}

void BaseRenderOpenGL3D::setWorldTransform(const Math::Matrix4 &transform) {
	Math::Matrix4 tmp = transform;
	tmp.transpose();
	Math::Matrix4 newModelViewTransform = tmp * _lastViewMatrix;
	glLoadMatrixf(newModelViewTransform.getData());
}

bool BaseRenderOpenGL3D::windowedBlt() {
	warning("BaseRenderOpenGL3D::windowedBlt not yet implemented");
	return true;
}

void Wintermute::BaseRenderOpenGL3D::onWindowChange() {
	warning("BaseRenderOpenGL3D::onWindowChange not yet implemented");
}

bool BaseRenderOpenGL3D::initRenderer(int width, int height, bool windowed) {
	_windowed = windowed;
	_width = width;
	_height = height;

	_nearPlane = 90.0f;
	_farPlane = 10000.0f;

	setViewport(0, 0, width, height);

	_active = true;
	// setup a proper state
	setup2D(true);

	_simpleShadow[0].x = -1.0f;
	_simpleShadow[0].y = 0.0f;
	_simpleShadow[0].z = -1.0f;
	_simpleShadow[0].nx = 0.0f;
	_simpleShadow[0].ny = 1.0f;
	_simpleShadow[0].nz = 0.0f;
	_simpleShadow[0].u = 0.0f;
	_simpleShadow[0].v = 1.0f;

	_simpleShadow[1].x = -1.0f;
	_simpleShadow[1].y = 0.0f;
	_simpleShadow[1].z = 1.0f;
	_simpleShadow[1].nx = 0.0f;
	_simpleShadow[1].ny = 1.0f;
	_simpleShadow[1].nz = 0.0f;
	_simpleShadow[1].u = 1.0f;
	_simpleShadow[1].v = 1.0f;

	_simpleShadow[2].x = 1.0f;
	_simpleShadow[2].y = 0.0f;
	_simpleShadow[2].z = -1.0f;
	_simpleShadow[2].nx = 0.0f;
	_simpleShadow[2].ny = 1.0f;
	_simpleShadow[2].nz = 0.0f;
	_simpleShadow[2].u = 0.0f;
	_simpleShadow[2].v = 0.0f;

	_simpleShadow[3].x = 1.0f;
	_simpleShadow[3].y = 0.0f;
	_simpleShadow[3].z = 1.0f;
	_simpleShadow[3].nx = 0.0f;
	_simpleShadow[3].ny = 1.0f;
	_simpleShadow[3].nz = 0.0f;
	_simpleShadow[3].u = 1.0f;
	_simpleShadow[3].v = 0.0f;

	// The ShaderSurfaceRenderer sets an array buffer which appearently conflicts with us
	// Reset it!
#if defined(USE_OPENGL_SHADERS)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

	return true;
}

bool Wintermute::BaseRenderOpenGL3D::flip() {
	g_system->updateScreen();
	return true;
}

bool BaseRenderOpenGL3D::indicatorFlip() {
	warning("BaseRenderOpenGL3D::indicatorFlip not yet implemented");
	return true;
}

bool BaseRenderOpenGL3D::forcedFlip() {
	warning("BaseRenderOpenGL3D::forcedFlip not yet implemented");
	return true;
}

bool BaseRenderOpenGL3D::setup2D(bool force) {
	if (_renderState != RSTATE_2D || force) {
		_renderState = RSTATE_2D;

		// some states are still missing here

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_FOG);
		glLightModeli(GL_LIGHT_MODEL_AMBIENT, 0);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glAlphaFunc(GL_GEQUAL, 0.0f);
		glPolygonMode(GL_FRONT, GL_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if defined(__MINGW32__) && defined (SDL_BACKEND) && !defined(USE_GLEW)
		glActiveTexturePtr(GL_TEXTURE0);
#elif defined(__MORPHOS__)
		glActiveTextureARB(GL_TEXTURE0);
#else
		glActiveTexture(GL_TEXTURE0);
#endif
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);

#if defined(__MINGW32__) && defined (SDL_BACKEND) && !defined(USE_GLEW)
		glActiveTexturePtr(GL_TEXTURE1);
#elif defined(__MORPHOS__)
		glActiveTextureARB(GL_TEXTURE1);
#else
		glActiveTexture(GL_TEXTURE1);
#endif
		glDisable(GL_TEXTURE_2D);

#if defined(__MINGW32__) && defined (SDL_BACKEND) && !defined(USE_GLEW)
		glActiveTexturePtr(GL_TEXTURE0);
#elif defined(__MORPHOS__)
		glActiveTextureARB(GL_TEXTURE0);
#else
		glActiveTexture(GL_TEXTURE0);
#endif
		
		glViewport(0, 0, _width, _height);
		setProjection2D();
	}

	return true;
}

bool BaseRenderOpenGL3D::setup3D(Camera3D *camera, bool force) {
	if (_renderState != RSTATE_3D || force) {
		_renderState = RSTATE_3D;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		// wme uses 8 as a reference value and Direct3D expects it to be in the range [0, 255]
		// 8 / 255 ~ 0.0313
		glAlphaFunc(GL_GEQUAL, 0.0313);

		setAmbientLight();

		glEnable(GL_NORMALIZE);

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
			glLoadMatrixf(viewMatrix.getData());
			glTranslatef(-camera->_position.x(), -camera->_position.y(), -camera->_position.z());
			glGetFloatv(GL_MODELVIEW_MATRIX, _lastViewMatrix.getData());
		} else {
			glLoadMatrixf(_lastViewMatrix.getData());
		}

		for (int i = 0; i < maximumLightsCount(); ++i) {
			glLightfv(GL_LIGHT0 + i, GL_POSITION, _lightPositions[i].getData());
			glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, _lightDirections[i].getData());
		}

		FogParameters fogParameters;
		_gameRef->getFogParams(fogParameters);

		if (fogParameters._enabled) {
			glEnable(GL_FOG);
			glFogi(GL_FOG_MODE, GL_LINEAR);
			glFogf(GL_FOG_START, fogParameters._start);
			glFogf(GL_FOG_END, fogParameters._end);

			uint32 fogColor = fogParameters._color;
			GLfloat color[4] = { RGBCOLGetR(fogColor) / 255.0f, RGBCOLGetG(fogColor) / 255.0f, RGBCOLGetB(fogColor) / 255.0f, RGBCOLGetA(fogColor) / 255.0f };
			glFogfv(GL_FOG_COLOR, color);
		} else {
			glDisable(GL_FOG);
		}

		glViewport(_viewportRect.left, _height - _viewportRect.bottom, _viewportRect.width(), _viewportRect.height());
		_viewport3dRect = _viewportRect;
		setProjection();
	}

	return true;
}

bool BaseRenderOpenGL3D::setupLines() {
	if (_renderState != RSTATE_LINES) {
		_renderState = RSTATE_LINES;

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return true;
}

BaseSurface *Wintermute::BaseRenderOpenGL3D::createSurface() {
	return new BaseSurfaceOpenGL3D(_gameRef, this);
}

#include "common/pack-start.h"

struct SpriteVertex {
	float u;
	float v;
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
	float x;
	float y;
	float z;
} PACKED_STRUCT;

#include "common/pack-end.h"

bool BaseRenderOpenGL3D::drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Wintermute::Rect32 &rect,
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

	SpriteVertex vertices[4] = {};

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
	vertices[0].z = -0.9f;

	vertices[1].x = pos.x - 0.5f;
	vertices[1].y = correctedYPos - height - 0.5f;
	vertices[1].z = -0.9f;

	vertices[2].x = pos.x + width - 0.5f;
	vertices[2].y = correctedYPos - 0.5f;
	vertices[2].z = -0.9f;

	vertices[3].x = pos.x + width - 0.5f;
	vertices[3].y = correctedYPos - height - 0.5;
	vertices[3].z = -0.9f;

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
		Vector2 correctedRot(rot.x, (rot.y - offset) * -1.0f + offset);
		Math::Matrix3 transform = build2dTransformation(correctedRot, angle);

		for (int i = 0; i < 4; ++i) {
			Math::Vector3d vertexPostion(vertices[i].x, vertices[i].y, 1.0f);
			transform.transformVector(&vertexPostion);

			vertices[i].x = vertexPostion.x();
			vertices[i].y = vertexPostion.y();
		}
	}

	if (alphaDisable) {
		glDisable(GL_ALPHA_TEST);
	}

	setSpriteBlendMode(blendMode);

	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(SpriteVertex), &vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SpriteVertex), &vertices[0].u);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SpriteVertex), &vertices[0].r);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (alphaDisable) {
		glEnable(GL_ALPHA_TEST);
	}

	return true;
}

void BaseRenderOpenGL3D::renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
                                             const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	_gameRef->_renderer3D->setup3D(camera, true);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisableClientState(GL_COLOR_ARRAY);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glBindTexture(GL_TEXTURE_2D, 0);

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
			Math::Vector3d right = lights[i]->_position + Math::Vector3d(1000.0f, 0.0f, 0.0f);
			Math::Vector3d up = lights[i]->_position + Math::Vector3d(0.0f, 1000.0f, 0.0f);
			Math::Vector3d backward = lights[i]->_position + Math::Vector3d(0.0f, 0.0f, 1000.0f);
			Math::Vector3d left = lights[i]->_position + Math::Vector3d(-1000.0f, 0.0f, 0.0f);
			Math::Vector3d down = lights[i]->_position + Math::Vector3d(0.0f, -1000.0f, 0.0f);
			Math::Vector3d forward = lights[i]->_position + Math::Vector3d(0.0f, 0.0f, -1000.0f);

			glVertex3fv(lights[i]->_position.getData());
			glVertex3fv(right.getData());
			glVertex3fv(lights[i]->_position.getData());
			glVertex3fv(up.getData());
			glVertex3fv(lights[i]->_position.getData());
			glVertex3fv(backward.getData());
			glVertex3fv(lights[i]->_position.getData());
			glVertex3fv(left.getData());
			glVertex3fv(lights[i]->_position.getData());
			glVertex3fv(down.getData());
			glVertex3fv(lights[i]->_position.getData());
			glVertex3fv(forward.getData());
			glEnd();
		}
	}

	glDisable(GL_COLOR_MATERIAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void BaseRenderOpenGL3D::renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks, const BaseArray<AdGeneric *> &generics, Camera3D *camera) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
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

Mesh3DS *BaseRenderOpenGL3D::createMesh3DS() {
	return new Mesh3DSOpenGL();
}

MeshX *BaseRenderOpenGL3D::createMeshX() {
	return new MeshXOpenGL(_gameRef);
}

ShadowVolume *BaseRenderOpenGL3D::createShadowVolume() {
	return new ShadowVolumeOpenGL(_gameRef);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL) && !defined(USE_GLES2)
