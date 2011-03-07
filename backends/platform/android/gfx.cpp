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

#include "graphics/conversion.h"

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

#ifdef USE_RGB_COLOR
Graphics::PixelFormat OSystem_Android::getScreenFormat() const {
	return _game_texture->getPixelFormat();
}

Common::List<Graphics::PixelFormat> OSystem_Android::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> res;
	res.push_back(GLES565Texture::getPixelFormat());
	res.push_back(GLES5551Texture::getPixelFormat());
	res.push_back(GLES4444Texture::getPixelFormat());
	res.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return res;
}

Common::String OSystem_Android::getPixelFormatName(const Graphics::PixelFormat &format) const {
	if (format.bytesPerPixel == 1)
		return "CLUT8";

	if (format.aLoss == 8)
		return Common::String::format("RGB%u%u%u",
										8 - format.rLoss,
										8 - format.gLoss,
										8 - format.bLoss);

	return Common::String::format("RGBA%u%u%u%u",
									8 - format.rLoss,
									8 - format.gLoss,
									8 - format.bLoss,
									8 - format.aLoss);
}

void OSystem_Android::initTexture(GLESTexture **texture,
									uint width, uint height,
									const Graphics::PixelFormat *format,
									bool alphaPalette) {
	assert(texture);
	Graphics::PixelFormat format_clut8 =
		Graphics::PixelFormat::createFormatCLUT8();
	Graphics::PixelFormat format_current;
	Graphics::PixelFormat format_new;

	if (*texture)
		format_current = (*texture)->getPixelFormat();
	else
		format_current = Graphics::PixelFormat();

	if (format)
		format_new = *format;
	else
		format_new = format_clut8;

	if (format_current != format_new) {
		if (*texture)
			LOGD("switching pixel format from: %s",
					getPixelFormatName((*texture)->getPixelFormat()).c_str());

		delete *texture;

		if (format_new == GLES565Texture::getPixelFormat())
			*texture = new GLES565Texture();
		else if (format_new == GLES5551Texture::getPixelFormat())
			*texture = new GLES5551Texture();
		else if (format_new == GLES4444Texture::getPixelFormat())
			*texture = new GLES4444Texture();
		else {
			// TODO what now?
			if (format_new != format_clut8)
				LOGE("unsupported pixel format: %s",
					getPixelFormatName(format_new).c_str());

			if (alphaPalette)
				*texture = new GLESPalette8888Texture;
			else
				*texture = new GLESPalette888Texture;
		}

		LOGD("new pixel format: %s",
				getPixelFormatName((*texture)->getPixelFormat()).c_str());
	}

	(*texture)->allocBuffer(width, height);
	(*texture)->fillBuffer(0);
}
#endif

void OSystem_Android::initSurface() {
	LOGD("initializing surface");

	assert(!JNI::haveSurface());

	_screen_changeid = JNI::surface_changeid;
	_egl_surface_width = JNI::egl_surface_width;
	_egl_surface_height = JNI::egl_surface_height;

	assert(_egl_surface_width > 0 && _egl_surface_height > 0);

	JNI::initSurface();

	// Initialise OpenGLES context.
	GLESTexture::initGLExtensions();

	if (_game_texture)
		_game_texture->reinit();

	if (_overlay_texture) {
		_overlay_texture->reinit();
		initOverlay();
	}

	if (_mouse_texture)
		_mouse_texture->reinit();
}

void OSystem_Android::deinitSurface() {
	if (!JNI::haveSurface())
		return;

	LOGD("deinitializing surface");

	_screen_changeid = JNI::surface_changeid;
	_egl_surface_width = 0;
	_egl_surface_height = 0;

	// release texture resources
	if (_game_texture)
		_game_texture->release();

	if (_overlay_texture)
		_overlay_texture->release();

	if (_mouse_texture)
		_mouse_texture->release();

	JNI::deinitSurface();
}

void OSystem_Android::initViewport() {
	LOGD("initializing viewport");

	assert(JNI::haveSurface());

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

	GLCALL(glViewport(0, 0, _egl_surface_width, _egl_surface_height));

	GLCALL(glMatrixMode(GL_PROJECTION));
	GLCALL(glLoadIdentity());
	GLCALL(glOrthof(0, _egl_surface_width, _egl_surface_height, 0, -1, 1));
	GLCALL(glMatrixMode(GL_MODELVIEW));
	GLCALL(glLoadIdentity());

	clearFocusRectangle();
}

void OSystem_Android::initOverlay() {
	int overlay_width = _egl_surface_width;
	int overlay_height = _egl_surface_height;

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
}

void OSystem_Android::initSize(uint width, uint height,
								const Graphics::PixelFormat *format) {
	ENTER("%d, %d, %p", width, height, format);

	GLTHREADCHECK;

#ifdef USE_RGB_COLOR
	initTexture(&_game_texture, width, height, format, false);
#else
	_game_texture->allocBuffer(width, height);
	_game_texture->fillBuffer(0);
#endif
	// Don't know mouse size yet - it gets reallocated in
	// setMouseCursor.  We need the palette allocated before
	// setMouseCursor however, so just take a guess at the desired
	// size (it's small).
	_mouse_texture_palette->allocBuffer(20, 20);

	// clear screen
	GLCALL(glClearColorx(0, 0, 0, 1 << 16));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT));
	JNI::swapBuffers();
}

int OSystem_Android::getScreenChangeID() const {
	return _screen_changeid;
}

int16 OSystem_Android::getHeight() {
	return _game_texture->height();
}

int16 OSystem_Android::getWidth() {
	return _game_texture->width();
}

void OSystem_Android::setPalette(const byte *colors, uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

#ifdef USE_RGB_COLOR
	assert(_game_texture->getPixelFormat().bytesPerPixel == 1);
#endif

	GLTHREADCHECK;

	memcpy(((GLESPaletteTexture *)_game_texture)->palette() + start * 3,
			colors, num * 3);

	if (!_use_mouse_palette)
		setCursorPaletteInternal(colors, start, num);
}

void OSystem_Android::grabPalette(byte *colors, uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

#ifdef USE_RGB_COLOR
	assert(_game_texture->getPixelFormat().bytesPerPixel == 1);
#endif

	GLTHREADCHECK;

	memcpy(colors, ((GLESPaletteTexture *)_game_texture)->palette() + start * 3,
			num * 3);
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

	if (!JNI::haveSurface())
		return;

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
		_game_texture->drawTexture(0, 0, _egl_surface_width,
									_egl_surface_height);
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

		_game_texture->drawTexture(0, 0, _egl_surface_width,
									_egl_surface_height);
		GLCALL(glPopMatrix());
	}

	int cs = _mouse_targetscale;

	if (_show_overlay) {
		// ugly, but the modern theme sets a wacko factor, only god knows why
		cs = 1;

		GLCALL(_overlay_texture->drawTexture(0, 0, _egl_surface_width,
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

	if (!JNI::swapBuffers())
		LOGW("swapBuffers failed: 0x%x", glGetError());
}

Graphics::Surface *OSystem_Android::lockScreen() {
	ENTER();

	GLTHREADCHECK;

	// TODO this doesn't return any pixel data for non CLUT8
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

	uint h = surface->h;

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

#ifdef USE_RGB_COLOR
	if (format && format->bytesPerPixel > 1) {
		if (_mouse_texture != _mouse_texture_rgb)
			LOGD("switching to rgb mouse cursor");

		_mouse_texture_rgb = new GLES5551Texture();
		_mouse_texture = _mouse_texture_rgb;
	} else {
		if (_mouse_texture != _mouse_texture_palette)
			LOGD("switching to paletted mouse cursor");

		_mouse_texture = _mouse_texture_palette;

		delete _mouse_texture_rgb;
		_mouse_texture_rgb = 0;
	}
#endif

	_mouse_texture->allocBuffer(w, h);

	if (_mouse_texture == _mouse_texture_palette) {
		// Update palette alpha based on keycolor
		byte *palette = _mouse_texture_palette->palette();

		for (uint i = 0; i < 256; ++i, palette += 4)
			palette[3] = 0xff;

		_mouse_texture_palette->palette()[keycolor * 4 + 3] = 0;
	}

	if (w == 0 || h == 0)
		return;

	if (_mouse_texture == _mouse_texture_palette) {
		_mouse_texture->updateBuffer(0, 0, w, h, buf, w);
	} else {
		uint16 pitch = _mouse_texture->pitch();

		byte *tmp = new byte[pitch * h];

		// meh, a 16bit cursor without alpha bits... this is so silly
		if (!crossBlit(tmp, buf, pitch, w * 2, w, h,
						_mouse_texture->getPixelFormat(),
						*format)) {
			LOGE("crossblit failed");

			delete[] tmp;

			_mouse_texture->fillBuffer(0);

			return;
		}

		uint16 *s = (uint16 *)buf;
		uint16 *d = (uint16 *)tmp;
		for (uint16 y = 0; y < h; ++y, d += pitch / 2 - w)
			for (uint16 x = 0; x < w; ++x, d++)
				if (*s++ != (keycolor & 0xffff))
					*d |= 1;

		_mouse_texture->updateBuffer(0, 0, w, h, tmp, pitch);

		delete[] tmp;
	}

	_mouse_hotspot = Common::Point(hotspotX, hotspotY);
	_mouse_targetscale = cursorTargetScale;
}

void OSystem_Android::setCursorPaletteInternal(const byte *colors,
												uint start, uint num) {
	byte *palette = _mouse_texture_palette->palette() + start * 4;

	for (uint i = 0; i < num; ++i, palette += 4, colors += 3) {
		palette[0] = colors[0];
		palette[1] = colors[1];
		palette[2] = colors[2];
		// Leave alpha untouched to preserve keycolor
	}
}

void OSystem_Android::setCursorPalette(const byte *colors,
										uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

	GLTHREADCHECK;

	if (_mouse_texture->getPixelFormat().bytesPerPixel != 1) {
		LOGD("switching to paletted mouse cursor");

		_mouse_texture = _mouse_texture_palette;

		delete _mouse_texture_rgb;
		_mouse_texture_rgb = 0;
	}

	setCursorPaletteInternal(colors, start, num);
	_use_mouse_palette = true;
}

void OSystem_Android::disableCursorPalette(bool disable) {
	ENTER("%d", disable);

	// when disabling the cursor palette, and we're running a clut8 game,
	// it expects the game palette to be used for the cursor
	if (disable && _game_texture->getPixelFormat().bytesPerPixel == 1) {
		byte *src = ((GLESPaletteTexture *)_game_texture)->palette();
		byte *dst = _mouse_texture_palette->palette();

		for (uint i = 0; i < 256; ++i, src += 3, dst += 4) {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			// Leave alpha untouched to preserve keycolor
		}
	}

	_use_mouse_palette = !disable;
}

#endif

