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

#include "engines/stark/gfx/opengl.h"

#include "common/system.h"

#include "math/matrix4.h"

#if defined(USE_OPENGL_GAME)

#include "engines/stark/gfx/openglactor.h"
#include "engines/stark/gfx/openglprop.h"
#include "engines/stark/gfx/openglsurface.h"
#include "engines/stark/gfx/openglfade.h"
#include "engines/stark/gfx/opengltexture.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

#include "graphics/surface.h"

namespace Stark {
namespace Gfx {

OpenGLDriver::OpenGLDriver() {
	_computeLights = true;
}

OpenGLDriver::~OpenGLDriver() {
}

void OpenGLDriver::init() {
	computeScreenViewport();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
}

void OpenGLDriver::setScreenViewport(bool noScaling) {
	if (noScaling) {
		_viewport = Common::Rect(g_system->getWidth(), g_system->getHeight());
		_unscaledViewport = _viewport;
	} else {
		_viewport = _screenViewport;
		_unscaledViewport = Common::Rect(kOriginalWidth, kOriginalHeight);
	}

	glViewport(_viewport.left, _viewport.top, _viewport.width(), _viewport.height());
}

void OpenGLDriver::setViewport(const Common::Rect &rect) {
	_viewport = Common::Rect(
			_screenViewport.width() * rect.width() / kOriginalWidth,
			_screenViewport.height() * rect.height() / kOriginalHeight
			);

	_viewport.translate(
			_screenViewport.left + _screenViewport.width() * rect.left / kOriginalWidth,
			_screenViewport.top + _screenViewport.height() * rect.top / kOriginalHeight
			);

	_unscaledViewport = rect;

	glViewport(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
}

void OpenGLDriver::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLDriver::flipBuffer() {
	g_system->updateScreen();
}

void OpenGLDriver::setupLights(const LightEntryArray &lights) {
	static const uint maxLights = 10;

	assert(lights.size() >= 1);
	assert(lights.size() <= maxLights);

	const LightEntry *ambient = lights[0];
	assert(ambient->type == LightEntry::kAmbient); // The first light must be the ambient light

	Math::Matrix4 viewMatrix = StarkScene->getViewMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for (uint i = 0; i < lights.size(); i++) {
		const LightEntry *l = lights[i];
		GLfloat ambientColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat lightColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat lightDir[] = { 0.0f, 0.0f, -1.0f };
		GLfloat cutoff = 180.0f;
		GLfloat spotExp = 0.0f;
		GLfloat c_attenuation = 1.0f;
		GLfloat l_attenuation = 0.0f;
		GLfloat q_attenuation = 0.0f;

		Math::Vector4d worldPosition;
		worldPosition.x() = l->position.x();
		worldPosition.y() = l->position.y();
		worldPosition.z() = l->position.z();
		worldPosition.w() = 1.0;

		Math::Vector4d eyePosition = viewMatrix * worldPosition;

		Math::Vector3d worldDirection = l->direction;
		Math::Vector3d eyeDirection = viewMatrix.getRotation() * worldDirection;
		eyeDirection.normalize();

		switch (l->type) {
			case LightEntry::kPoint:
				lightColor[0] = (GLfloat)l->color.x();
				lightColor[1] = (GLfloat)l->color.y();
				lightColor[2] = (GLfloat)l->color.z();
				lightPos[0] = (GLfloat)eyePosition.x();
				lightPos[1] = (GLfloat)eyePosition.y();
				lightPos[2] = (GLfloat)eyePosition.z();
				break;
			case LightEntry::kDirectional:
				lightColor[0] = (GLfloat)l->color.x();
				lightColor[1] = (GLfloat)l->color.y();
				lightColor[2] = (GLfloat)l->color.z();
				lightPos[0] = (GLfloat)-eyeDirection.x();
				lightPos[1] = (GLfloat)-eyeDirection.y();
				lightPos[2] = (GLfloat)-eyeDirection.z();
				lightPos[3] = 0;
				break;
			case LightEntry::kSpot:
				lightColor[0] = (GLfloat)l->color.x();
				lightColor[1] = (GLfloat)l->color.y();
				lightColor[2] = (GLfloat)l->color.z();
				lightPos[0] = (GLfloat)eyePosition.x();
				lightPos[1] = (GLfloat)eyePosition.y();
				lightPos[2] = (GLfloat)eyePosition.z();
				lightDir[0] = (GLfloat)eyeDirection.x();
				lightDir[1] = (GLfloat)eyeDirection.y();
				lightDir[2] = (GLfloat)eyeDirection.z();
				cutoff = (l->outerConeAngle.getDegrees() + l->innerConeAngle.getDegrees()) / 2.26f;
				break;
			case LightEntry::kAmbient:
				lightColor[0] = (GLfloat)l->color.x();
				lightColor[1] = (GLfloat)l->color.y();
				lightColor[2] = (GLfloat)l->color.z();
				break;
			default:
				break;
		}

		glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambientColor);
		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, lightColor);
		glLightfv(GL_LIGHT0 + i, GL_POSITION, lightPos);
		glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, lightDir);
		glLightf(GL_LIGHT0 + i, GL_SPOT_EXPONENT, spotExp);
		glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, cutoff);
		glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, c_attenuation);
		glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, l_attenuation);
		glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, q_attenuation);
		glEnable(GL_LIGHT0 + i);
	}

	for (uint i = lights.size() - 1; i < maxLights; i++) {
		// Make sure unused lights are disabled
		glDisable(GL_LIGHT0 + i + 1);
	}
}

Texture *OpenGLDriver::createTexture(const Graphics::Surface *surface, const byte *palette) {
	OpenGlTexture *texture = new OpenGlTexture();

	if (surface) {
		texture->update(surface, palette);
	}

	return texture;
}

Texture *OpenGLDriver::createBitmap(const Graphics::Surface *surface, const byte *palette) {
	return createTexture(surface, palette);
}

VisualActor *OpenGLDriver::createActorRenderer() {
	return new OpenGLActorRenderer(this);
}

VisualProp *OpenGLDriver::createPropRenderer() {
	return new OpenGLPropRenderer(this);
}

SurfaceRenderer *OpenGLDriver::createSurfaceRenderer() {
	return new OpenGLSurfaceRenderer(this);
}

FadeRenderer *OpenGLDriver::createFadeRenderer() {
	return new OpenGLFadeRenderer(this);
}

void OpenGLDriver::start2DMode() {
	// Enable alpha blending
	glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD); // It's the default

	// This blend mode prevents color fringes due to filtering.
	// It requires the textures to have their color values pre-multiplied
	// with their alpha value. This is the "Premultiplied Alpha" technique.
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	if (!_computeLights)
		glDisable(GL_LIGHTING);
}

void OpenGLDriver::end2DMode() {
	// Disable alpha blending
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void OpenGLDriver::set3DMode() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Blending and stencil test are only used in rendering shadows
	// They are manually enabled and disabled there
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	if (!_computeLights)
		glEnable(GL_LIGHTING);
}

bool OpenGLDriver::computeLightsEnabled() {
	return _computeLights;
}

Common::Rect OpenGLDriver::getViewport() const {
	return _viewport;
}

Common::Rect OpenGLDriver::getUnscaledViewport() const {
	return _unscaledViewport;
}

Graphics::Surface *OpenGLDriver::getViewportScreenshot() const {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(_viewport.width(), _viewport.height(), getRGBAPixelFormat());

	glReadPixels(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height(),
	             GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());

	flipVertical(s);

	return s;
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_GAME)
