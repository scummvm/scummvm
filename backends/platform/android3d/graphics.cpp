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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/tokenizer.h"
#include "graphics/conversion.h"
#include "graphics/opengl/shader.h"
#include "graphics/opengl/context.h"

#include "backends/platform/android3d/android.h"
#include "backends/platform/android3d/events.h"
#include "backends/platform/android3d/graphics.h"
#include "backends/platform/android3d/jni-android.h"

AndroidGraphicsManager::AndroidGraphicsManager() :
	_screenChangeID(0),
	_graphicsMode(0),
	_opengl(false),
	_fullscreen(true),
	_ar_correction(true),
	_force_redraw(false),
	_game_texture(0),
	_game_pbuf(),
	_frame_buffer(0),
	_cursorX(0),
	_cursorY(0),
	_overlay_texture(0),
	_show_overlay(false),
	_mouse_texture(0),
	_mouse_texture_palette(0),
	_mouse_texture_rgb(0),
	_mouse_hotspot(),
	_mouse_keycolor(0),
	_show_mouse(false),
	_use_mouse_palette(false)
{
	_game_texture = new GLESFakePalette565Texture();
	_overlay_texture = new GLES4444Texture();
	_mouse_texture_palette = new GLESFakePalette5551Texture();
	_mouse_texture = _mouse_texture_palette;

}

AndroidGraphicsManager::~AndroidGraphicsManager() {
	delete _game_texture;
	delete _overlay_texture;
	delete _mouse_texture_palette;
	delete _mouse_texture_rgb;

}

static void logExtensions() {
	const char *ext_string =
		reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

	LOGI("Extensions:");

	Common::String exts;
	Common::StringTokenizer tokenizer(ext_string, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		exts += token + " ";
		if (exts.size() > 100) {
			LOGI("\t%s", exts.c_str());
			exts = "";
		}
	}

	if (exts.size() > 0)
		LOGI("\t%s", exts.c_str());
}


void AndroidGraphicsManager::initSurface() {
	LOGD("initializing surface");

	assert(!JNI::haveSurface());

	JNI::initSurface();

	_screenChangeID = JNI::surface_changeid;

	// Initialize OpenGLES context.
	OpenGLContext.initialize(OpenGL::kOGLContextGLES2);
	logExtensions();
	GLESTexture::initGL();

	if (_game_texture)
		_game_texture->reinit();

	if (_overlay_texture) {
		_overlay_texture->reinit();
		initOverlay();
	}

	if (_mouse_texture)
		_mouse_texture->reinit();

	initViewport();
	updateScreenRect();
	// double buffered, flip twice
	clearScreen(kClearUpdate, 2);
	updateEventScale();
}

void AndroidGraphicsManager::deinitSurface() {
	if (!JNI::haveSurface())
		return;

	LOGD("deinitializing surface");

	_screenChangeID = JNI::surface_changeid;

	// release texture resources
	if (_game_texture)
		_game_texture->release();

	if (_overlay_texture)
		_overlay_texture->release();

	if (_mouse_texture)
		_mouse_texture->release();

	OpenGL::ContextGL::destroy();

	JNI::deinitSurface();
}

void AndroidGraphicsManager::updateScreen() {
	//ENTER();

	GLTHREADCHECK;

	if (!JNI::haveSurface())
		return;

		if (_game_pbuf) {
			int pitch = _game_texture->width() * _game_texture->getPixelFormat().bytesPerPixel;
			_game_texture->updateBuffer(0, 0, _game_texture->width(), _game_texture->height(),
					_game_pbuf.getRawBuffer(), pitch);
		}

		if (!_force_redraw &&
				!_game_texture->dirty() &&
				!_overlay_texture->dirty() &&
				!_mouse_texture->dirty())
			return;

		_force_redraw = false;

		if (_frame_buffer) {
			_frame_buffer->detach();
			glViewport(0,0, JNI::egl_surface_width, JNI::egl_surface_height);
		}

		// clear pointer leftovers in dead areas
		clearScreen(kClear);

		_game_texture->drawTextureRect();
		if (!_show_overlay) {
			glEnable(GL_BLEND);
			dynamic_cast<OSystem_Android *>(g_system)->getTouchControls()->draw();
		}

		int cs = _mouse_targetscale;

		if (_show_overlay) {
			// ugly, but the modern theme sets a wacko factor, only god knows why
			cs = 1;

			GLCALL(_overlay_texture->drawTextureRect());
		}

		if (_show_mouse && !_mouse_texture->isEmpty()) {
			const Common::Point &mouse = g_system->getEventManager()->getMousePos();
			if (_show_overlay) {
				_mouse_texture->drawTexture(mouse.x * cs, mouse.y * cs, _mouse_texture->width(), _mouse_texture->height());
			}
	}

	if (!JNI::swapBuffers())
		LOGW("swapBuffers failed: 0x%x", glGetError());

	if (_frame_buffer)
		_frame_buffer->attach();
}

void AndroidGraphicsManager::displayMessageOnOSD(const Common::U32String &msg) {
	ENTER("%s", msg.encode().c_str());

	JNI::displayMessageOnOSD(msg);
}

bool AndroidGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	clipMouse(mouse);
	setMousePosition(mouse.x, mouse.y);
	return true;
}

const OSystem::GraphicsMode *AndroidGraphicsManager::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
		{ "default", "Default", 0 },
		{ 0, 0, 0 },
	};

	return s_supportedGraphicsModes;
}

int AndroidGraphicsManager::getDefaultGraphicsMode() const {
	return 0;
}

bool AndroidGraphicsManager::setGraphicsMode(int mode, uint flags) {
	return true;
}

int AndroidGraphicsManager::getGraphicsMode() const {
	return _graphicsMode;
}

bool AndroidGraphicsManager::hasFeature(OSystem::Feature f) const {
	if (f == OSystem::kFeatureCursorPalette ||
			f == OSystem::kFeatureFullscreenMode ||
			f == OSystem::kFeatureAspectRatioCorrection) {
		return true;
	}
	return false;
}

void AndroidGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		_use_mouse_palette = enable;
		if (!enable)
			disableCursorPalette();
		break;
	case OSystem::kFeatureFullscreenMode:
		_fullscreen = enable;
		updateScreenRect();
		break;
	case OSystem::kFeatureAspectRatioCorrection:
		_ar_correction = enable;
		updateScreenRect();
		break;
	default:
		break;
	}
}

bool AndroidGraphicsManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		return _use_mouse_palette;
	case OSystem::kFeatureFullscreenMode:
		return _fullscreen;
	case OSystem::kFeatureAspectRatioCorrection:
		return _ar_correction;
	default:
		return false;
	}
}

void AndroidGraphicsManager::showOverlay() {
	ENTER();

	_show_overlay = true;
	_force_redraw = true;

	updateEventScale();

	warpMouse(_overlay_texture->width() / 2, _overlay_texture->height() / 2);

	GLCALL(glDisable(GL_SCISSOR_TEST));
}

void AndroidGraphicsManager::hideOverlay() {
	ENTER();

	_show_overlay = false;

	updateEventScale();

	warpMouse(_game_texture->width() / 2, _game_texture->height() / 2);

	// double buffered, flip twice
	clearScreen(kClearUpdate, 2);

	GLCALL(glEnable(GL_SCISSOR_TEST));
}

void AndroidGraphicsManager::clearOverlay() {
	ENTER();

	GLTHREADCHECK;

	_overlay_texture->fillBuffer(0);
}

void AndroidGraphicsManager::grabOverlay(void *buf, int pitch) const {
	ENTER("%p, %d", buf, pitch);

	GLTHREADCHECK;

	const Graphics::Surface *surface = _overlay_texture->surface_const();
	assert(surface->format.bytesPerPixel == sizeof(uint16));

	byte *dst = (byte *)buf;
	const byte *src = (const byte *)surface->getPixels();
	uint h = surface->h;

	do {
		memcpy(dst, src, surface->w * surface->format.bytesPerPixel);
		src += surface->pitch;
		dst += pitch;
	} while (--h);
}

void AndroidGraphicsManager::copyRectToOverlay(const void *buf, int pitch,
										int x, int y, int w, int h) {
	ENTER("%p, %d, %d, %d, %d, %d", buf, pitch, x, y, w, h);

	GLTHREADCHECK;

	_overlay_texture->updateBuffer(x, y, w, h, buf, pitch);
}

int16 AndroidGraphicsManager::getOverlayHeight() const {
	return _overlay_texture->height();
}

int16 AndroidGraphicsManager::getOverlayWidth() const {
	return _overlay_texture->width();
}

Graphics::PixelFormat AndroidGraphicsManager::getOverlayFormat() const {
	return _overlay_texture->getPixelFormat();
}

int16 AndroidGraphicsManager::getHeight() const {
	return _game_texture->height();
}

int16 AndroidGraphicsManager::getWidth() const {
	return _game_texture->width();
}

void AndroidGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

#ifdef USE_RGB_COLOR
	assert(_game_texture->hasPalette());
#endif

	GLTHREADCHECK;

	if (!_use_mouse_palette)
		setCursorPaletteInternal(colors, start, num);

	const Graphics::PixelFormat &pf = _game_texture->getPalettePixelFormat();
	byte *p = _game_texture->palette() + start * 2;

	for (uint i = 0; i < num; ++i, colors += 3, p += 2)
		WRITE_UINT16(p, pf.RGBToColor(colors[0], colors[1], colors[2]));
}

void AndroidGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	ENTER("%p, %u, %u", colors, start, num);

#ifdef USE_RGB_COLOR
	assert(_game_texture->hasPalette());
#endif

	GLTHREADCHECK;

	const Graphics::PixelFormat &pf = _game_texture->getPalettePixelFormat();
	const byte *p = _game_texture->palette_const() + start * 2;

	for (uint i = 0; i < num; ++i, colors += 3, p += 2)
		pf.colorToRGB(READ_UINT16(p), colors[0], colors[1], colors[2]);
}

Graphics::Surface *AndroidGraphicsManager::lockScreen() {
	ENTER();

	GLTHREADCHECK;

	Graphics::Surface *surface = _game_texture->surface();
	assert(surface->getPixels());

	return surface;
}

void AndroidGraphicsManager::unlockScreen() {
	ENTER();

	GLTHREADCHECK;

	assert(_game_texture->dirty());
}

void AndroidGraphicsManager::fillScreen(uint32 col) {
	ENTER("%u", col);

	GLTHREADCHECK;

	_game_texture->fillBuffer(col);
}

void AndroidGraphicsManager::copyRectToScreen(const void *buf, int pitch,
										int x, int y, int w, int h) {
	ENTER("%p, %d, %d, %d, %d, %d", buf, pitch, x, y, w, h);

	GLTHREADCHECK;

	_game_texture->updateBuffer(x, y, w, h, buf, pitch);
}

void AndroidGraphicsManager::initSize(uint width, uint height,
								const Graphics::PixelFormat *format) {
	setupScreen(width, height, true, true);
}

void AndroidGraphicsManager::initSizeIntern(uint width, uint height,
								const Graphics::PixelFormat *format) {
	ENTER("%d, %d, %p", width, height, format);

	GLTHREADCHECK;

#ifdef USE_RGB_COLOR
	initTexture(&_game_texture, width, height, format);
#else
	_game_texture->allocBuffer(width, height);
#endif
#ifdef USE_GLES2
	_frame_buffer = new OpenGL::FrameBuffer(_game_texture->getTextureName(), _game_texture->width(), _game_texture->height(), _game_texture->texWidth(), _game_texture->texHeight());
	_frame_buffer->attach();
#endif

	updateScreenRect();
	updateEventScale();

	// Don't know mouse size yet - it gets reallocated in
	// setMouseCursor.  We need the palette allocated before
	// setMouseCursor however, so just take a guess at the desired
	// size (it's small).
	_mouse_texture_palette->allocBuffer(20, 20);

	clearScreen(kClear);
}

int AndroidGraphicsManager::getScreenChangeID() const {
	return _screenChangeID;
}

bool AndroidGraphicsManager::showMouse(bool visible) {
	ENTER("%d", visible);

	_show_mouse = visible;

	return true;
}

void AndroidGraphicsManager::warpMouse(int x, int y) {
	ENTER("%d, %d", x, y);

	Common::Event e;

	e.type = Common::EVENT_MOUSEMOVE;
	e.mouse.x = x;
	e.mouse.y = y;

	clipMouse(e.mouse);

	setMousePosition(e.mouse.x, e.mouse.y);

	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(e);
}

void AndroidGraphicsManager::setMouseCursor(const void *buf, uint w, uint h,
										int hotspotX, int hotspotY,
										uint32 keycolor, bool dontScale,
										const Graphics::PixelFormat *format) {
	ENTER("%p, %u, %u, %d, %d, %u, %d, %p", buf, w, h, hotspotX, hotspotY,
			keycolor, dontScale, format);

	GLTHREADCHECK;

#ifdef USE_RGB_COLOR
	if (format && format->bytesPerPixel > 1) {
		if (_mouse_texture != _mouse_texture_rgb) {
			LOGD("switching to rgb mouse cursor");

			assert(!_mouse_texture_rgb);
			_mouse_texture_rgb = new GLES5551Texture();
			_mouse_texture_rgb->setLinearFilter(_graphicsMode == 1);
		}

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
		assert(keycolor < 256);

		byte *p = _mouse_texture_palette->palette() + _mouse_keycolor * 2;
		WRITE_UINT16(p, READ_UINT16(p) | 1);

		_mouse_keycolor = keycolor;

		p = _mouse_texture_palette->palette() + _mouse_keycolor * 2;
		WRITE_UINT16(p, READ_UINT16(p) & ~1);
	}

	if (w == 0 || h == 0)
		return;

	if (_mouse_texture == _mouse_texture_palette) {
		_mouse_texture->updateBuffer(0, 0, w, h, buf, w);
	} else {
		uint16 pitch = _mouse_texture->pitch();

		byte *tmp = new byte[pitch * h];

		// meh, a 16bit cursor without alpha bits... this is so silly
		if (!crossBlit(tmp, (const byte *)buf, pitch, w * 2, w, h,
						_mouse_texture->getPixelFormat(),
						*format)) {
			LOGE("crossblit failed");

			delete[] tmp;

			_mouse_texture->allocBuffer(0, 0);

			return;
		}

		const uint16 *s = (const uint16 *)buf;
		uint16 *d = (uint16 *)tmp;
		for (uint16 y = 0; y < h; ++y, d += pitch / 2 - w)
			for (uint16 x = 0; x < w; ++x, d++)
				if (*s++ == (keycolor & 0xffff))
					*d = 0;

		_mouse_texture->updateBuffer(0, 0, w, h, tmp, pitch);

		delete[] tmp;
	}

	_mouse_hotspot = Common::Point(hotspotX, hotspotY);
	// TODO: Adapt to the new "do not scale" cursor logic.
	_mouse_targetscale = 1;
}

void AndroidGraphicsManager::setCursorPaletteInternal(const byte *colors,
												uint start, uint num) {
	const Graphics::PixelFormat &pf =
		_mouse_texture_palette->getPalettePixelFormat();
	byte *p = _mouse_texture_palette->palette() + start * 2;

	for (uint i = 0; i < num; ++i, colors += 3, p += 2)
		WRITE_UINT16(p, pf.RGBToColor(colors[0], colors[1], colors[2]));

	p = _mouse_texture_palette->palette() + _mouse_keycolor * 2;
	WRITE_UINT16(p, READ_UINT16(p) & ~1);
}

void AndroidGraphicsManager::setCursorPalette(const byte *colors,
										uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

	GLTHREADCHECK;

	if (!_mouse_texture->hasPalette()) {
		LOGD("switching to paletted mouse cursor");

		_mouse_texture = _mouse_texture_palette;

		delete _mouse_texture_rgb;
		_mouse_texture_rgb = 0;
	}

	setCursorPaletteInternal(colors, start, num);
	_use_mouse_palette = true;
}

void AndroidGraphicsManager::disableCursorPalette() {
	// when disabling the cursor palette, and we're running a clut8 game,
	// it expects the game palette to be used for the cursor
	if (_game_texture->hasPalette()) {
		const byte *src = _game_texture->palette_const();
		byte *dst = _mouse_texture_palette->palette();

		const Graphics::PixelFormat &pf_src =
			_game_texture->getPalettePixelFormat();
		const Graphics::PixelFormat &pf_dst =
			_mouse_texture_palette->getPalettePixelFormat();

		uint8 r, g, b;

		for (uint i = 0; i < 256; ++i, src += 2, dst += 2) {
			pf_src.colorToRGB(READ_UINT16(src), r, g, b);
			WRITE_UINT16(dst, pf_dst.RGBToColor(r, g, b));
		}

		byte *p = _mouse_texture_palette->palette() + _mouse_keycolor * 2;
		WRITE_UINT16(p, READ_UINT16(p) & ~1);
	}
}

bool AndroidGraphicsManager::lockMouse(bool lock) {
	_show_mouse = lock;
	return true;
}

void AndroidGraphicsManager::setupScreen(uint screenW, uint screenH, bool fullscreen, bool accel3d) {
	setupScreen(screenW, screenH, fullscreen, accel3d, true);
}

void AndroidGraphicsManager::setupScreen(uint screenW, uint screenH, bool fullscreen, bool accel3d, bool isGame) {
	_opengl = accel3d;
	initViewport();

	if (_opengl) {
		// resize game texture
		initSizeIntern(screenW, screenH, 0);
		if (isGame)
			_game_texture->setGameTexture();
		// format is not used by the gfx_opengl driver, use fake format
		_game_pbuf.set(Graphics::PixelFormat(), 0);

	} else {
		Graphics::PixelFormat format = GLES565Texture::pixelFormat();
		initSizeIntern(screenW, screenH, &format);
		// as there is no support for the texture surface's lock/unlock mechanism in gfx_tinygl/...
		// do not use _game_texture->surface()->pixels directly
		_game_pbuf.create(_game_texture->getPixelFormat(),
				_game_texture->width() * _game_texture->height(), DisposeAfterUse::YES);
	}
}

void AndroidGraphicsManager::clipMouse(Common::Point &p) const {
	const GLESBaseTexture *tex = getActiveTexture();

	p.x = CLIP(p.x, tex->getDrawRect().left, tex->getDrawRect().right);
	p.y = CLIP(p.y, tex->getDrawRect().top, tex->getDrawRect().bottom);
}

#ifdef USE_RGB_COLOR
Graphics::PixelFormat AndroidGraphicsManager::getScreenFormat() const {
	return _game_texture->getPixelFormat();
}

Common::List<Graphics::PixelFormat> AndroidGraphicsManager::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> res;
	res.push_back(GLES565Texture::pixelFormat());
	res.push_back(GLES5551Texture::pixelFormat());
	res.push_back(GLES4444Texture::pixelFormat());
	res.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return res;
}
#endif

void AndroidGraphicsManager::updateScreenRect() {
	Common::Rect rect(0, 0, JNI::egl_surface_width, JNI::egl_surface_height);

	_overlay_texture->setDrawRect(rect);

	uint16 w = _game_texture->width();
	uint16 h = _game_texture->height();

	if (w && h && _ar_correction) {

		float dpi[2];
		JNI::getDPI(dpi);

		float screen_ar;
		if (dpi[0] != 0.0 && dpi[1] != 0.0) {
			// horizontal orientation
			screen_ar = (dpi[1] * JNI::egl_surface_width) /
						(dpi[0] * JNI::egl_surface_height);
		} else {
			screen_ar = float(JNI::egl_surface_width) / float(JNI::egl_surface_height);
		}

		float game_ar = float(w) / float(h);

		if (screen_ar > game_ar) {
			rect.setWidth(round(JNI::egl_surface_height * game_ar));
			rect.moveTo((JNI::egl_surface_width - rect.width()) / 2, 0);
		} else {
			rect.setHeight(round(JNI::egl_surface_width / game_ar));
			rect.moveTo((JNI::egl_surface_height - rect.height()) / 2, 0);
		}
	}

	_game_texture->setDrawRect(rect);
}

const GLESBaseTexture *AndroidGraphicsManager::getActiveTexture() const {
	if (_show_overlay)
		return _overlay_texture;
	else
		return _game_texture;
}

void AndroidGraphicsManager::initOverlay() {
	// minimum of 320x200
	// (surface can get smaller when opening the virtual keyboard on *QVGA*)
	int overlay_width = MAX(JNI::egl_surface_width, 320);
	int overlay_height = MAX(JNI::egl_surface_height, 200);

	// the 'normal' theme layout uses a max height of 400 pixels. if the
	// surface is too big we use only a quarter of the size so that the widgets
	// don't get too small. if the surface height has less than 800 pixels, this
	// enforces the 'lowres' layout, which will be scaled back up by factor 2x,
	// but this looks way better than the 'normal' layout scaled by some
	// calculated factors
//	while (overlay_height > 480) {
//		overlay_width /= 2;
//		overlay_height /= 2;
//	}

	LOGI("overlay size is %ux%u", overlay_width, overlay_height);

	_overlay_texture->allocBuffer(overlay_width, overlay_height);
	_overlay_texture->setDrawRect(0, 0,
									JNI::egl_surface_width, JNI::egl_surface_height);
}

void AndroidGraphicsManager::initViewport() {
	LOGD("initializing viewport");

	assert(JNI::haveSurface());

	GLCALL(glDisable(GL_CULL_FACE));
	GLCALL(glDisable(GL_DEPTH_TEST));

	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	GLCALL(glViewport(0, 0, JNI::egl_surface_width, JNI::egl_surface_height));
	LOGD("viewport size: %dx%d", JNI::egl_surface_width, JNI::egl_surface_height);
}

void AndroidGraphicsManager::updateEventScale() {
	dynamic_cast<OSystem_Android *>(g_system)->updateEventScale(getActiveTexture());
}

void AndroidGraphicsManager::clearScreen(FixupType type, byte count) {
	assert(count > 0);

	bool sm = _show_mouse;
	_show_mouse = false;

	GLCALL(glDisable(GL_SCISSOR_TEST));

	for (byte i = 0; i < count; ++i) {
		// clear screen
		GLCALL(glClearColor(0, 0, 0, 1 << 16));
		if (_opengl) {
			GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
		} else {
			GLCALL(glClear(GL_COLOR_BUFFER_BIT));
		}

		switch (type) {
		case kClear:
			break;

		case kClearSwap:
			JNI::swapBuffers();
			break;

		case kClearUpdate:
			_force_redraw = true;
			updateScreen();
			break;
		}
	}

	if (!_show_overlay)
		GLCALL(glEnable(GL_SCISSOR_TEST));

	_show_mouse = sm;
	_force_redraw = true;
}

#ifdef USE_RGB_COLOR
void AndroidGraphicsManager::initTexture(GLESBaseTexture **texture,
									uint width, uint height,
									const Graphics::PixelFormat *format) {
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
					(*texture)->getPixelFormat().toString().c_str());

		delete *texture;

		if (format_new == GLES565Texture::pixelFormat())
			*texture = new GLES565Texture();
		else if (format_new == GLES5551Texture::pixelFormat())
			*texture = new GLES5551Texture();
		else if (format_new == GLES4444Texture::pixelFormat())
			*texture = new GLES4444Texture();
		else {
			// TODO what now?
			if (format_new != format_clut8)
				LOGE("unsupported pixel format: %s",
					format_new.toString().c_str());

			*texture = new GLESFakePalette565Texture;
		}

		LOGD("new pixel format: %s",
				(*texture)->getPixelFormat().toString().c_str());
	}

	(*texture)->allocBuffer(width, height);
}
#endif

#endif
