/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#if defined(__ANDROID__)

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni.h"

static inline GLfixed xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}

const OSystem::GraphicsMode *OSystem_Android::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
		{ "default", "Default", 1 },
		{ 0, 0, 0 },
	};

	return s_supportedGraphicsModes;
}

int OSystem_Android::getDefaultGraphicsMode() const {
	return 1;
}

bool OSystem_Android::setGraphicsMode(const char *mode) {
	ENTER("%s", mode);

	return true;
}

bool OSystem_Android::setGraphicsMode(int mode) {
	ENTER("%d", mode);

	return true;
}

int OSystem_Android::getGraphicsMode() const {
	return 1;
}

void OSystem_Android::setupSurface() {
	ENTER();

	if (!JNI::setupSurface())
		return;

	// EGL set up with a new surface.  Initialise OpenGLES context.
	GLESTexture::initGLExtensions();

	// Turn off anything that looks like 3D ;)
	GLCALL(glDisable(GL_CULL_FACE));
	GLCALL(glDisable(GL_DEPTH_TEST));
	GLCALL(glDisable(GL_LIGHTING));
	GLCALL(glDisable(GL_FOG));
	GLCALL(glDisable(GL_DITHER));

	GLCALL(glShadeModel(GL_FLAT));
	GLCALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST));

	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	GLCALL(glEnableClientState(GL_VERTEX_ARRAY));
	GLCALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

	GLCALL(glEnable(GL_TEXTURE_2D));

	if (!_game_texture)
		_game_texture = new GLESPaletteTexture();
	else
		_game_texture->reinitGL();

	if (!_overlay_texture)
		_overlay_texture = new GLES4444Texture();
	else
		_overlay_texture->reinitGL();

	if (!_mouse_texture)
		_mouse_texture = new GLESPaletteATexture();
	else
		_mouse_texture->reinitGL();

	GLCALL(glViewport(0, 0, _egl_surface_width, _egl_surface_height));

	GLCALL(glMatrixMode(GL_PROJECTION));
	GLCALL(glLoadIdentity());
	GLCALL(glOrthof(0, _egl_surface_width, _egl_surface_height, 0, -1, 1));
	GLCALL(glMatrixMode(GL_MODELVIEW));
	GLCALL(glLoadIdentity());

	clearFocusRectangle();
}

void OSystem_Android::initSize(uint width, uint height,
								const Graphics::PixelFormat *format) {
	ENTER("%d, %d, %p", width, height, format);

	GLTHREADCHECK;

	_game_texture->allocBuffer(width, height);

	GLuint overlay_width = _egl_surface_width;
	GLuint overlay_height = _egl_surface_height;

	// the 'normal' theme layout uses a max height of 400 pixels. if the
	// surface is too big we use only a quarter of the size so that the widgets
	// don't get too small. if the surface height has less than 800 pixels, this
	// enforces the 'lowres' layout, which will be scaled back up by factor 2x,
	// but this looks way better than the 'normal' layout scaled by some
	// calculated factors
	if (overlay_height > 480) {
		overlay_width /= 2;
		overlay_height /= 2;
	}

	LOGI("overlay size is %ux%u", overlay_width, overlay_height);

	_overlay_texture->allocBuffer(overlay_width, overlay_height);

	// Don't know mouse size yet - it gets reallocated in
	// setMouseCursor.  We need the palette allocated before
	// setMouseCursor however, so just take a guess at the desired
	// size (it's small).
	_mouse_texture->allocBuffer(20, 20);
}

int16 OSystem_Android::getHeight() {
	return _game_texture->height();
}

int16 OSystem_Android::getWidth() {
	return _game_texture->width();
}

void OSystem_Android::setPalette(const byte *colors, uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

	GLTHREADCHECK;

	if (!_use_mouse_palette)
		_setCursorPalette(colors, start, num);

	memcpy(_game_texture->palette() + start * 3, colors, num * 3);
}

void OSystem_Android::grabPalette(byte *colors, uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

	GLTHREADCHECK;

	memcpy(colors, _game_texture->palette_const() + start * 3, num * 3);
}

void OSystem_Android::copyRectToScreen(const byte *buf, int pitch,
										int x, int y, int w, int h) {
	ENTER("%p, %d, %d, %d, %d, %d", buf, pitch, x, y, w, h);

	GLTHREADCHECK;

	_game_texture->updateBuffer(x, y, w, h, buf, pitch);
}

void OSystem_Android::updateScreen() {
	//ENTER();

	GLTHREADCHECK;

	if (!_force_redraw &&
			!_game_texture->dirty() &&
			!_overlay_texture->dirty() &&
			!_mouse_texture->dirty())
		return;

	_force_redraw = false;

	GLCALL(glPushMatrix());

	if (_shake_offset != 0 ||
			(!_focus_rect.isEmpty() &&
			!Common::Rect(_game_texture->width(),
							_game_texture->height()).contains(_focus_rect))) {
		// These are the only cases where _game_texture doesn't
		// cover the entire screen.
		GLCALL(glClearColorx(0, 0, 0, 1 << 16));
		GLCALL(glClear(GL_COLOR_BUFFER_BIT));

		// Move everything up by _shake_offset (game) pixels
		GLCALL(glTranslatex(0, -_shake_offset << 16, 0));
	}

	if (_focus_rect.isEmpty()) {
		_game_texture->drawTexture(0, 0,
									_egl_surface_width, _egl_surface_height);
	} else {
		GLCALL(glPushMatrix());
		GLCALL(glScalex(xdiv(_egl_surface_width, _focus_rect.width()),
						xdiv(_egl_surface_height, _focus_rect.height()),
						1 << 16));
		GLCALL(glTranslatex(-_focus_rect.left << 16,
							-_focus_rect.top << 16, 0));
		GLCALL(glScalex(xdiv(_game_texture->width(), _egl_surface_width),
						xdiv(_game_texture->height(), _egl_surface_height),
						1 << 16));

		_game_texture->drawTexture(0, 0,
									_egl_surface_width, _egl_surface_height);
		GLCALL(glPopMatrix());
	}

	int cs = _mouse_targetscale;

	if (_show_overlay) {
		// ugly, but the modern theme sets a wacko factor, only god knows why
		cs = 1;

		GLCALL(_overlay_texture->drawTexture(0, 0,
												_egl_surface_width,
												_egl_surface_height));
	}

	if (_show_mouse) {
		GLCALL(glPushMatrix());

		// Scale up ScummVM -> OpenGL (pixel) coordinates
		int texwidth, texheight;

		if (_show_overlay) {
			texwidth = getOverlayWidth();
			texheight = getOverlayHeight();
		} else {
			texwidth = getWidth();
			texheight = getHeight();
		}

		GLCALL(glScalex(xdiv(_egl_surface_width, texwidth),
						xdiv(_egl_surface_height, texheight),
						1 << 16));

		GLCALL(glTranslatex((-_mouse_hotspot.x * cs) << 16,
							(-_mouse_hotspot.y * cs) << 16,
							0));

		// Note the extra half texel to position the mouse in
		// the middle of the x,y square:
		const Common::Point& mouse = getEventManager()->getMousePos();
		GLCALL(glTranslatex((mouse.x << 16) | 1 << 15,
							(mouse.y << 16) | 1 << 15, 0));

		GLCALL(glScalex(cs << 16, cs << 16, 1 << 16));

		_mouse_texture->drawTexture();

		GLCALL(glPopMatrix());
	}

	GLCALL(glPopMatrix());

	if (!JNI::swapBuffers()) {
		// Context lost -> need to reinit GL
		JNI::destroySurface();
		setupSurface();
	}
}

Graphics::Surface *OSystem_Android::lockScreen() {
	ENTER();

	GLTHREADCHECK;

	Graphics::Surface *surface = _game_texture->surface();
	assert(surface->pixels);

	return surface;
}

void OSystem_Android::unlockScreen() {
	ENTER();

	GLTHREADCHECK;

	assert(_game_texture->dirty());
}

void OSystem_Android::setShakePos(int shake_offset) {
	ENTER("%d", shake_offset);

	if (_shake_offset != shake_offset) {
		_shake_offset = shake_offset;
		_force_redraw = true;
	}
}

void OSystem_Android::fillScreen(uint32 col) {
	ENTER("%u", col);

	GLTHREADCHECK;

	assert(col < 256);
	_game_texture->fillBuffer(col);
}

void OSystem_Android::setFocusRectangle(const Common::Rect& rect) {
	ENTER("%d, %d, %d, %d", rect.left, rect.top, rect.right, rect.bottom);

	if (_enable_zoning) {
		_focus_rect = rect;
		_force_redraw = true;
	}
}

void OSystem_Android::clearFocusRectangle() {
	ENTER();

	if (_enable_zoning) {
		_focus_rect = Common::Rect();
		_force_redraw = true;
	}
}

void OSystem_Android::showOverlay() {
	ENTER();

	_show_overlay = true;
	_force_redraw = true;
}

void OSystem_Android::hideOverlay() {
	ENTER();

	_show_overlay = false;
	_force_redraw = true;
}

void OSystem_Android::clearOverlay() {
	ENTER();

	GLTHREADCHECK;

	_overlay_texture->fillBuffer(0);

	// breaks more than it fixes, disabled for now
	// Shouldn't need this, but works around a 'blank screen' bug on Nexus1
	//updateScreen();
}

void OSystem_Android::grabOverlay(OverlayColor *buf, int pitch) {
	ENTER("%p, %d", buf, pitch);

	GLTHREADCHECK;

	// We support overlay alpha blending, so the pixel data here
	// shouldn't actually be used.	Let's fill it with zeros, I'm sure
	// it will be fine...
	const Graphics::Surface *surface = _overlay_texture->surface_const();
	assert(surface->bytesPerPixel == sizeof(buf[0]));

	int h = surface->h;

	do {
		memset(buf, 0, surface->w * sizeof(buf[0]));

		// This 'pitch' is pixels not bytes
		buf += pitch;
	} while (--h);
}

void OSystem_Android::copyRectToOverlay(const OverlayColor *buf, int pitch,
										int x, int y, int w, int h) {
	ENTER("%p, %d, %d, %d, %d, %d", buf, pitch, x, y, w, h);

	GLTHREADCHECK;

	const Graphics::Surface *surface = _overlay_texture->surface_const();
	assert(surface->bytesPerPixel == sizeof(buf[0]));

	// This 'pitch' is pixels not bytes
	_overlay_texture->updateBuffer(x, y, w, h, buf, pitch * sizeof(buf[0]));

	// Shouldn't need this, but works around a 'blank screen' bug on Nexus1?
	//updateScreen();
}

int16 OSystem_Android::getOverlayHeight() {
	return _overlay_texture->height();
}

int16 OSystem_Android::getOverlayWidth() {
	return _overlay_texture->width();
}

bool OSystem_Android::showMouse(bool visible) {
	ENTER("%d", visible);

	_show_mouse = visible;

	return true;
}

void OSystem_Android::warpMouse(int x, int y) {
	ENTER("%d, %d", x, y);

	// We use only the eventmanager's idea of the current mouse
	// position, so there is nothing extra to do here.
}

void OSystem_Android::setMouseCursor(const byte *buf, uint w, uint h,
										int hotspotX, int hotspotY,
										uint32 keycolor, int cursorTargetScale,
										const Graphics::PixelFormat *format) {
	ENTER("%p, %u, %u, %d, %d, %u, %d, %p", buf, w, h, hotspotX, hotspotY,
			keycolor, cursorTargetScale, format);

	GLTHREADCHECK;

	assert(keycolor < 256);

	_mouse_texture->allocBuffer(w, h);

	// Update palette alpha based on keycolor
	byte *palette = _mouse_texture->palette();
	int i = 256;

	do {
		palette[3] = 0xff;
		palette += 4;
	} while (--i);

	palette = _mouse_texture->palette();
	palette[keycolor * 4 + 3] = 0x00;

	_mouse_texture->updateBuffer(0, 0, w, h, buf, w);

	_mouse_hotspot = Common::Point(hotspotX, hotspotY);
	_mouse_targetscale = cursorTargetScale;
}

void OSystem_Android::_setCursorPalette(const byte *colors,
										uint start, uint num) {
	byte *palette = _mouse_texture->palette() + start * 4;

	do {
		for (int i = 0; i < 3; ++i)
			palette[i] = colors[i];

		// Leave alpha untouched to preserve keycolor

		palette += 4;
		colors += 3;
	} while (--num);
}

void OSystem_Android::setCursorPalette(const byte *colors,
										uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

	GLTHREADCHECK;

	_setCursorPalette(colors, start, num);
	_use_mouse_palette = true;
}

void OSystem_Android::disableCursorPalette(bool disable) {
	ENTER("%d", disable);

	_use_mouse_palette = !disable;
}

#endif

