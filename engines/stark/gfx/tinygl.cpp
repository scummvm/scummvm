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

#include "common/system.h"
#include "common/config-manager.h"

#include "math/matrix4.h"

#include "engines/stark/gfx/tinygl.h"
#include "engines/stark/gfx/tinyglactor.h"
#include "engines/stark/gfx/tinyglbitmap.h"
#include "engines/stark/gfx/tinyglprop.h"
#include "engines/stark/gfx/tinyglsurface.h"
#include "engines/stark/gfx/tinyglfade.h"
#include "engines/stark/gfx/tinygltexture.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

#include "graphics/surface.h"

namespace Stark {
namespace Gfx {

TinyGLDriver::TinyGLDriver() {
}

TinyGLDriver::~TinyGLDriver() {
	TinyGL::destroyContext();
}

void TinyGLDriver::init() {
	computeScreenViewport();

	TinyGL::createContext(kOriginalWidth, kOriginalHeight, g_system->getScreenFormat(), 512, true, ConfMan.getBool("dirtyrects"));

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
	tglDisable(TGL_LIGHTING);
}

void TinyGLDriver::setScreenViewport(bool noScaling) {
	if (noScaling) {
		_viewport = Common::Rect(g_system->getWidth(), g_system->getHeight());
		_unscaledViewport = _viewport;
	} else {
		_viewport = _screenViewport;
		_unscaledViewport = Common::Rect(kOriginalWidth, kOriginalHeight);
	}

	tglViewport(_viewport.left, _viewport.top, _viewport.width(), _viewport.height());
}

void TinyGLDriver::setViewport(const Common::Rect &rect) {
	_viewport = Common::Rect(_screenViewport.width() * rect.width() / kOriginalWidth,
	                         _screenViewport.height() * rect.height() / kOriginalHeight);

	_viewport.translate(_screenViewport.left + _screenViewport.width() * rect.left / kOriginalWidth,
	                    _screenViewport.top + _screenViewport.height() * rect.top / kOriginalHeight);

	_unscaledViewport = rect;

	tglViewport(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
}

void TinyGLDriver::clearScreen() {
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT | TGL_STENCIL_BUFFER_BIT);
}

void TinyGLDriver::flipBuffer() {
	Common::List<Common::Rect> dirtyAreas;
	TinyGL::presentBuffer(dirtyAreas);

	Graphics::Surface glBuffer;
	TinyGL::getSurfaceRef(glBuffer);

	if (!dirtyAreas.empty()) {
		for (Common::List<Common::Rect>::iterator itRect = dirtyAreas.begin(); itRect != dirtyAreas.end(); ++itRect) {
			g_system->copyRectToScreen(glBuffer.getBasePtr((*itRect).left, (*itRect).top), glBuffer.pitch,
			                           (*itRect).left, (*itRect).top, (*itRect).width(), (*itRect).height());
		}
	}

	g_system->updateScreen();
}

Texture *TinyGLDriver::createTexture() {
	return new TinyGlTexture();
}

Bitmap *TinyGLDriver::createBitmap(const Graphics::Surface *surface, const byte *palette) {
	TinyGlBitmap *texture = new TinyGlBitmap();

	if (surface) {
		texture->update(surface, palette);
	}

	return texture;
}

VisualActor *TinyGLDriver::createActorRenderer() {
	return new TinyGLActorRenderer(this);
}

VisualProp *TinyGLDriver::createPropRenderer() {
	return new TinyGLPropRenderer(this);
}

SurfaceRenderer *TinyGLDriver::createSurfaceRenderer() {
	return new TinyGLSurfaceRenderer(this);
}

FadeRenderer *TinyGLDriver::createFadeRenderer() {
	return new TinyGLFadeRenderer(this);
}

void TinyGLDriver::start2DMode() {
	// This blend mode prevents color fringes due to filtering.
	// It requires the textures to have their color values pre-multiplied
	// with their alpha value. This is the "Premultiplied Alpha" technique.
	tglBlendFunc(TGL_ONE, TGL_ONE_MINUS_SRC_ALPHA);
	tglEnable(TGL_BLEND);

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);
}

void TinyGLDriver::end2DMode() {
	tglDisable(TGL_BLEND);
	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
}

void TinyGLDriver::set3DMode() {
	tglEnable(TGL_DEPTH_TEST);
	tglDepthFunc(TGL_LESS);

	// Stencil test are only used in rendering shadows
	// They are manually enabled and disabled there
	tglStencilFunc(TGL_EQUAL, 0, 0xFF);
	tglStencilOp(TGL_KEEP, TGL_KEEP, TGL_INCR);
}

bool TinyGLDriver::computeLightsEnabled() {
	return false;
}

Common::Rect TinyGLDriver::getViewport() const {
	return _viewport;
}

Common::Rect TinyGLDriver::getUnscaledViewport() const {
	return _unscaledViewport;
}

Graphics::Surface *TinyGLDriver::getViewportScreenshot() const {
	Graphics::Surface *tmp = TinyGL::copyFromFrameBuffer(getRGBAPixelFormat());
	Graphics::Surface *s = new Graphics::Surface();
	s->create(_viewport.width(), _viewport.height(), getRGBAPixelFormat());
	byte *src = (byte *)tmp->getPixels();
	s->copyRectToSurface(src + tmp->pitch * _viewport.top + _viewport.left * tmp->format.bytesPerPixel,
	                     tmp->pitch, 0, 0, _viewport.width(), _viewport.height());
	tmp->free();
	delete tmp;
	return s;
}

} // End of namespace Gfx
} // End of namespace Stark
