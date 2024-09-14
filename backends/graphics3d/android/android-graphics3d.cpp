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

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

#include "common/tokenizer.h"
#include "graphics/blit.h"
#include "graphics/managed_surface.h"
#include "graphics/opengl/shader.h"
#include "graphics/opengl/context.h"

#include "backends/graphics3d/android/android-graphics3d.h"

// These helper macros let us setup our context only when the game has different settings than us
#define CONTEXT_SAVE_STATE(gl_param) GLboolean saved ## gl_param; GLCALL(saved ## gl_param = glIsEnabled(gl_param))
#define CONTEXT_SET_ENABLE(gl_param) if (!(saved ## gl_param)) { GLCALL(glEnable(gl_param)); }
#define CONTEXT_SET_DISABLE(gl_param) if (saved ## gl_param) { GLCALL(glDisable(gl_param)); }
// These helper macros do the opposite to get back what the game expected
#define CONTEXT_RESET_ENABLE(gl_param) if (!(saved ## gl_param)) { GLCALL(glDisable(gl_param)); }
#define CONTEXT_RESET_DISABLE(gl_param) if (saved ## gl_param) { GLCALL(glEnable(gl_param)); }

AndroidGraphics3dManager::AndroidGraphics3dManager() :
	_screenChangeID(0),
	_graphicsMode(0),
	_fullscreen(true),
	_ar_correction(true),
	_force_redraw(false),
	_virtkeybd_on(false),
	_game_texture(0),
	_frame_buffer(0),
	_cursorX(0),
	_cursorY(0),
	_overlay_texture(0),
	_overlay_background(nullptr),
	_show_overlay(false),
	_mouse_texture(nullptr),
	_mouse_texture_palette(nullptr),
	_mouse_texture_rgb(nullptr),
	_mouse_hotspot(),
	_mouse_dont_scale(false),
	_show_mouse(false),
	_touchcontrols_texture(new GLES8888Texture()),
	_old_touch_mode(OSystem_Android::TOUCH_MODE_TOUCHPAD) {

	if (JNI::egl_bits_per_pixel == 16) {
		// We default to RGB565 and RGBA5551 which is closest to what we setup in Java side
		_game_texture = new GLES565Texture();
		_overlay_texture = new GLES5551Texture();
		_overlay_background = new GLES565Texture();
		_mouse_texture_palette = new GLESFakePalette5551Texture();
	} else {
		// If not 16, this must be 24 or 32 bpp so make use of them
		_game_texture = new GLES888Texture();
		_overlay_texture = new GLES8888Texture();
		_overlay_background = new GLES8888Texture();
		_mouse_texture_palette = new GLESFakePalette8888Texture();
	}
	_mouse_texture = _mouse_texture_palette;

	initSurface();

	// in 3D, not in GUI
	dynamic_cast<OSystem_Android *>(g_system)->applyTouchSettings(true, false);
	dynamic_cast<OSystem_Android *>(g_system)->applyOrientationSettings();
}

AndroidGraphics3dManager::~AndroidGraphics3dManager() {
	// Reinitialize OpenGL for other manager
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glUseProgram(0);

	// Cleanup framebuffer before destroying context
	delete _frame_buffer;
	_frame_buffer = nullptr;

	deinitSurface();

	// These textures have been cleaned in deinitSurface
	// Deleting them now without a context is harmless
	delete _game_texture;
	delete _overlay_texture;
	delete _overlay_background;
	delete _mouse_texture_palette;
	delete _mouse_texture_rgb;
	delete _touchcontrols_texture;
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

	if (exts.size() > 0) {
		LOGI("\t%s", exts.c_str());
	}
}


void AndroidGraphics3dManager::initSurface() {
	LOGD("initializing 3D surface");

	assert(!JNI::haveSurface());

	if (!JNI::initSurface()) {
		error("JNI::initSurface failed");
	}

	_screenChangeID = JNI::surface_changeid;

	// Initialize OpenGLES context.
	OpenGLContext.initialize(OpenGL::kContextGLES2);
	logExtensions();
	GLESTexture::initGL();

	if (_game_texture) {
		_game_texture->reinit();
		// We had a frame buffer initialized, we must renew it as the game textured got renewed
		if (_frame_buffer) {
			delete _frame_buffer;
			_frame_buffer = new OpenGL::FrameBuffer(_game_texture->getTextureName(),
	                                        _game_texture->width(), _game_texture->height(),
	                                        _game_texture->texWidth(), _game_texture->texHeight());

		}
	}

	// We don't have any content to display: just make sure surface is clean
	if (_overlay_background) {
		_overlay_background->release();
	}

	if (_overlay_texture) {
		_overlay_texture->reinit();
		initOverlay();
	}

	if (_mouse_texture) {
		_mouse_texture->reinit();
	}

	if (_mouse_texture_palette && _mouse_texture != _mouse_texture_palette) {
		_mouse_texture_palette->reinit();
	}

	if (_touchcontrols_texture) {
		_touchcontrols_texture->reinit();
	}
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().setDrawer(
	    this, JNI::egl_surface_width, JNI::egl_surface_height);

	updateScreenRect();
	// double buffered, flip twice
	clearScreen(kClearUpdate, 2);
}

void AndroidGraphics3dManager::deinitSurface() {
	if (!JNI::haveSurface()) {
		return;
	}

	LOGD("deinitializing 3D surface");

	_screenChangeID = JNI::surface_changeid;

	// release texture resources
	if (_game_texture) {
		_game_texture->release();
	}

	if (_overlay_texture) {
		_overlay_texture->release();
	}

	if (_overlay_background) {
		_overlay_background->release();
	}

	if (_mouse_texture) {
		_mouse_texture->release();
	}

	if (_mouse_texture_palette && _mouse_texture != _mouse_texture_palette) {
		_mouse_texture_palette->release();
	}

	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().setDrawer(
	    nullptr, 0, 0);
	if (_touchcontrols_texture) {
		_touchcontrols_texture->release();
	}

	OpenGLContext.reset();

	JNI::deinitSurface();
}

void AndroidGraphics3dManager::resizeSurface() {
	LOGD("resizing 3D surface");

	if (!JNI::haveSurface()) {
		initSurface();
		return;
	}

	JNI::deinitSurface();
	if (!JNI::initSurface()) {
		error("JNI::initSurface failed");
	}

	_screenChangeID = JNI::surface_changeid;

	if (_overlay_texture) {
		initOverlay();
	}

	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().setDrawer(
	    this, JNI::egl_surface_width, JNI::egl_surface_height);

	updateScreenRect();
	// double buffered, flip twice
	clearScreen(kClearUpdate, 2);
}

void AndroidGraphics3dManager::updateScreen() {
	//ENTER();

	GLTHREADCHECK;

	if (!JNI::haveSurface()) {
		return;
	}

	// Sets _forceRedraw if needed
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().beforeDraw();

	if (!_force_redraw &&
	        !_game_texture->dirty() &&
	        !_overlay_texture->dirty() &&
	        !_mouse_texture->dirty()) {
		return;
	}

	_force_redraw = false;

	// Save the game state
	GLint savedBlendSrcRGB, savedBlendDstRGB, savedBlendSrcAlpha, savedBlendDstAlpha,
	      savedBlendEqRGB, savedBlendEqAlpha;
	GLint savedViewport[4];
	CONTEXT_SAVE_STATE(GL_BLEND);
	GLCALL(glGetIntegerv(GL_BLEND_SRC_RGB, &savedBlendSrcRGB));
	GLCALL(glGetIntegerv(GL_BLEND_DST_RGB, &savedBlendDstRGB));
	GLCALL(glGetIntegerv(GL_BLEND_SRC_ALPHA, &savedBlendSrcAlpha));
	GLCALL(glGetIntegerv(GL_BLEND_DST_ALPHA, &savedBlendDstAlpha));
	GLCALL(glGetIntegerv(GL_BLEND_EQUATION_RGB, &savedBlendEqRGB));
	GLCALL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &savedBlendEqAlpha));
	CONTEXT_SAVE_STATE(GL_CULL_FACE);
	CONTEXT_SAVE_STATE(GL_DEPTH_TEST);
	CONTEXT_SAVE_STATE(GL_DITHER);
	CONTEXT_SAVE_STATE(GL_POLYGON_OFFSET_FILL);
	CONTEXT_SAVE_STATE(GL_SAMPLE_ALPHA_TO_COVERAGE);
	CONTEXT_SAVE_STATE(GL_SAMPLE_COVERAGE);
	CONTEXT_SAVE_STATE(GL_SCISSOR_TEST);
	CONTEXT_SAVE_STATE(GL_STENCIL_TEST);
	GLCALL(glGetIntegerv(GL_VIEWPORT, savedViewport));

	if (_frame_buffer) {
		_frame_buffer->detach();
	}

	// Make sure everything we need is correctly set up
	// Enable what we need and disable the other if it is not already
	CONTEXT_SET_ENABLE(GL_BLEND);
	if (savedBlendSrcRGB != GL_SRC_ALPHA ||
	    savedBlendDstRGB != GL_ONE_MINUS_SRC_ALPHA ||
	    savedBlendSrcAlpha != GL_SRC_ALPHA ||
	    savedBlendDstAlpha != GL_ONE_MINUS_SRC_ALPHA) {
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}
	if (savedBlendEqRGB != GL_FUNC_ADD ||
	    savedBlendEqAlpha != GL_FUNC_ADD) {
		GLCALL(glBlendEquation(GL_FUNC_ADD));
	}
	CONTEXT_SET_DISABLE(GL_CULL_FACE);
	CONTEXT_SET_DISABLE(GL_DEPTH_TEST);
	CONTEXT_SET_DISABLE(GL_DITHER);
	CONTEXT_SET_DISABLE(GL_POLYGON_OFFSET_FILL);
	CONTEXT_SET_DISABLE(GL_SAMPLE_ALPHA_TO_COVERAGE);
	CONTEXT_SET_DISABLE(GL_SAMPLE_COVERAGE);
	CONTEXT_SET_DISABLE(GL_SCISSOR_TEST);
	CONTEXT_SET_DISABLE(GL_STENCIL_TEST);

	GLCALL(glViewport(0, 0, JNI::egl_surface_width, JNI::egl_surface_height));

	if (_frame_buffer) {
		// clear pointer leftovers in dead areas
		clearScreen(kClear);

		_game_texture->drawTextureRect();
	}

	if (_show_overlay) {
		// If the overlay is in game we expect the game to continue drawing
		if (_overlay_in_gui) {
			clearScreen(kClear);
			if (_overlay_background && _overlay_background->getTextureName() != 0) {
				GLCALL(_overlay_background->drawTextureRect());
			}
		}
		GLCALL(_overlay_texture->drawTextureRect());

		if (_show_mouse && !_mouse_texture->isEmpty()) {
			_mouse_texture->drawTexture(_cursorX - _mouse_hotspot_scaled.x, _cursorY - _mouse_hotspot_scaled.y,
						    _mouse_width_scaled, _mouse_width_scaled);
		}
	}

	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().draw();

	if (!JNI::swapBuffers()) {
		LOGW("swapBuffers failed: 0x%x", glGetError());
	}

	// Here we restore back the GLES state so if we enabled something we disable it back if it needs too and vice versa

	CONTEXT_RESET_ENABLE(GL_BLEND);
	if (savedGL_BLEND && (
	    savedBlendSrcRGB != GL_SRC_ALPHA ||
	    savedBlendDstRGB != GL_ONE_MINUS_SRC_ALPHA ||
	    savedBlendSrcAlpha != GL_SRC_ALPHA ||
	    savedBlendDstAlpha != GL_ONE_MINUS_SRC_ALPHA)) {
		GLCALL(glBlendFuncSeparate(savedBlendSrcRGB, savedBlendDstRGB,
		                           savedBlendSrcAlpha, savedBlendDstAlpha));
	}
	if (savedGL_BLEND && (
	    savedBlendEqRGB != GL_FUNC_ADD ||
	    savedBlendEqAlpha != GL_FUNC_ADD)) {
		GLCALL(glBlendEquationSeparate(savedBlendEqRGB, savedBlendEqAlpha));
	}
	CONTEXT_RESET_DISABLE(GL_CULL_FACE);
	CONTEXT_RESET_DISABLE(GL_DEPTH_TEST);
	CONTEXT_RESET_DISABLE(GL_DITHER);
	CONTEXT_RESET_DISABLE(GL_POLYGON_OFFSET_FILL);
	CONTEXT_RESET_DISABLE(GL_SAMPLE_ALPHA_TO_COVERAGE);
	CONTEXT_RESET_DISABLE(GL_SAMPLE_COVERAGE);
	CONTEXT_RESET_DISABLE(GL_SCISSOR_TEST);
	CONTEXT_RESET_DISABLE(GL_STENCIL_TEST);

	// Restore game viewport
	GLCALL(glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]));

	// Don't keep our texture attached to avoid the engine writing on it if it forgets to setup its own texture
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	// Unload our program to make sure engine will use its own
	GLESBaseTexture::unbindShader();

	if (_frame_buffer) {
		_frame_buffer->attach();
	}
}

void AndroidGraphics3dManager::displayMessageOnOSD(const Common::U32String &msg) {
	ENTER("%s", msg.encode().c_str());

	JNI::displayMessageOnOSD(msg);
}

Common::Point AndroidGraphics3dManager::convertScreenToVirtual(int &x, int &y) const {
	const GLESBaseTexture *tex = getActiveTexture();
	const Common::Rect &screenRect = tex->getDrawRect();

	// Clip in place the coordinates that comes handy to call setMousePosition
	x = CLIP<int>(x, screenRect.left, screenRect.right - 1);
	y = CLIP<int>(y, screenRect.top, screenRect.bottom - 1);

	// Now convert this to virtual coordinates using texture virtual size
	const uint16 virtualWidth = tex->width();
	const uint16 virtualHeight = tex->height();

	int virtualX = ((x - screenRect.left) * virtualWidth + screenRect.width() / 2) / screenRect.width();
	int virtualY = ((y - screenRect.top) * virtualHeight + screenRect.height() / 2) / screenRect.height();

	return Common::Point(CLIP<int>(virtualX, 0, virtualWidth - 1),
	                     CLIP<int>(virtualY, 0, virtualHeight - 1));
}

Common::Point AndroidGraphics3dManager::convertVirtualToScreen(int x, int y) const {
	const GLESBaseTexture *tex = getActiveTexture();
	const uint16 virtualWidth = tex->width();
	const uint16 virtualHeight = tex->height();
	const Common::Rect &screenRect = tex->getDrawRect();

	int screenX = screenRect.left + (x * screenRect.width() + virtualWidth / 2) / virtualWidth;
	int screenY = screenRect.top + (y * screenRect.height() + virtualHeight / 2) / virtualHeight;

	return Common::Point(CLIP<int>(screenX, screenRect.left, screenRect.right - 1),
	                     CLIP<int>(screenY, screenRect.top, screenRect.bottom - 1));
}

bool AndroidGraphics3dManager::notifyMousePosition(Common::Point &mouse) {
	// At entry, mouse is in screen coordinates like the texture draw rectangle
	int x = mouse.x, y = mouse.y;
	Common::Point vMouse = convertScreenToVirtual(x, y);

	// Our internal mouse position is in screen coordinates
	// convertScreenToVirtual just clipped coordinates so we are safe
	setMousePosition(x, y);

	// Now modify mouse to translate to virtual coordinates for the caller
	mouse = vMouse;

	return true;
}

const OSystem::GraphicsMode *AndroidGraphics3dManager::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
		{ "default", "Default", 0 },
		{ 0, 0, 0 },
	};

	return s_supportedGraphicsModes;
}

int AndroidGraphics3dManager::getDefaultGraphicsMode() const {
	return 0;
}

bool AndroidGraphics3dManager::setGraphicsMode(int mode, uint flags) {
	return true;
}

int AndroidGraphics3dManager::getGraphicsMode() const {
	return _graphicsMode;
}

bool AndroidGraphics3dManager::hasFeature(OSystem::Feature f) const {
	if (f == OSystem::kFeatureCursorPalette ||
	        f == OSystem::kFeatureCursorAlpha ||
	        f == OSystem::kFeatureOpenGLForGame ||
	        f == OSystem::kFeatureAspectRatioCorrection) {
		return true;
	}
	if (f == OSystem::kFeatureOverlaySupportsAlpha) {
		return _overlay_texture->getPixelFormat().aBits() > 3;
	}
	return false;
}

void AndroidGraphics3dManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
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

bool AndroidGraphics3dManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		return true;
	case OSystem::kFeatureFullscreenMode:
		return _fullscreen;
	case OSystem::kFeatureAspectRatioCorrection:
		return _ar_correction;
	default:
		return false;
	}
}

void AndroidGraphics3dManager::showOverlay(bool inGUI) {
	ENTER();

	if (_show_overlay && inGUI == _overlay_in_gui) {
		return;
	}

	if (inGUI) {
		_old_touch_mode = JNI::getTouchMode();
		// in 3D, in overlay
		dynamic_cast<OSystem_Android *>(g_system)->applyTouchSettings(true, true);
		dynamic_cast<OSystem_Android *>(g_system)->applyOrientationSettings();
	} else if (_overlay_in_gui) {
		// Restore touch mode active before overlay was shown
		JNI::setTouchMode(_old_touch_mode);
	}

	_show_overlay = true;
	_overlay_in_gui = inGUI;
	_force_redraw = true;

	// If there is a game running capture the screen, so that it can be shown "below" the overlay.
	if (_overlay_background) {
		_overlay_background->release();

		if (g_engine) {
			GLint savedViewport[4];
			GLCALL(glGetIntegerv(GL_VIEWPORT, savedViewport));

			if (_frame_buffer) {
				_frame_buffer->detach();
			}

			GLCALL(glViewport(0, 0, JNI::egl_surface_width, JNI::egl_surface_height));
			_overlay_background->reinit();
			_overlay_background->allocBuffer(_overlay_texture->width(), _overlay_texture->height());
			_overlay_background->setDrawRect(0, 0,
			                                 JNI::egl_surface_width, JNI::egl_surface_height);
			_overlay_background->readPixels();
			_overlay_background->setGameTexture();

			// Restore game viewport
			GLCALL(glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]));

			if (_frame_buffer) {
				_frame_buffer->attach();
			}
		}
	}

	if (inGUI) {
		warpMouse(_overlay_texture->width() / 2, _overlay_texture->height() / 2);
	}
}

void AndroidGraphics3dManager::hideOverlay() {
	ENTER();

	if (!_show_overlay) {
		return;
	}

	_show_overlay = false;

	_overlay_background->release();

	if (_overlay_in_gui) {
		// Restore touch mode active before overlay was shown
		JNI::setTouchMode(_old_touch_mode);

		warpMouse(_game_texture->width() / 2, _game_texture->height() / 2);

		dynamic_cast<OSystem_Android *>(g_system)->applyOrientationSettings();
	}

	_overlay_in_gui = false;

	// double buffered, flip twice
	clearScreen(kClearUpdate, 2);
}

void AndroidGraphics3dManager::clearOverlay() {
	ENTER();

	GLTHREADCHECK;

	_overlay_texture->fillBuffer(0);
}

void AndroidGraphics3dManager::grabOverlay(Graphics::Surface &surface) const {
	ENTER("%p", &surface);

	GLTHREADCHECK;

	const Graphics::Surface *overlaySurface = _overlay_texture->surface_const();

	assert(surface.w >= overlaySurface->w);
	assert(surface.h >= overlaySurface->h);
	assert(surface.format.bytesPerPixel == overlaySurface->format.bytesPerPixel);

	const byte *src = (const byte *)overlaySurface->getPixels();
	byte *dst = (byte *)surface.getPixels();
	Graphics::copyBlit(dst, src, surface.pitch, overlaySurface->pitch,
	                   overlaySurface->w, overlaySurface->h, overlaySurface->format.bytesPerPixel);
}

void AndroidGraphics3dManager::copyRectToOverlay(const void *buf, int pitch,
        int x, int y, int w, int h) {
	ENTER("%p, %d, %d, %d, %d, %d", buf, pitch, x, y, w, h);

	GLTHREADCHECK;

	_overlay_texture->updateBuffer(x, y, w, h, buf, pitch);
}

int16 AndroidGraphics3dManager::getOverlayHeight() const {
	return _overlay_texture->height();
}

int16 AndroidGraphics3dManager::getOverlayWidth() const {
	return _overlay_texture->width();
}

Graphics::PixelFormat AndroidGraphics3dManager::getOverlayFormat() const {
	return _overlay_texture->getPixelFormat();
}

int16 AndroidGraphics3dManager::getHeight() const {
	if (_frame_buffer)
		return _frame_buffer->getHeight();
	else
		return _overlay_texture->height();
}

int16 AndroidGraphics3dManager::getWidth() const {
	if (_frame_buffer)
		return _frame_buffer->getWidth();
	else
		return _overlay_texture->width();
}

void AndroidGraphics3dManager::setPalette(const byte *colors, uint start, uint num) {
	// We should never end up here in 3D
	assert(false);
}

void AndroidGraphics3dManager::grabPalette(byte *colors, uint start, uint num) const {
	// We should never end up here in 3D
	assert(false);
}

Graphics::Surface *AndroidGraphics3dManager::lockScreen() {
	// We should never end up here in 3D
	assert(false);

	return nullptr;
}

void AndroidGraphics3dManager::unlockScreen() {
	// We should never end up here in 3D
	assert(false);
}

void AndroidGraphics3dManager::fillScreen(uint32 col) {
	// We should never end up here in 3D
	assert(false);
}

void AndroidGraphics3dManager::fillScreen(const Common::Rect &r, uint32 col) {
	// We should never end up here in 3D
	assert(false);
}

void AndroidGraphics3dManager::copyRectToScreen(const void *buf, int pitch,
        int x, int y, int w, int h) {
	// We should never end up here in 3D
	assert(false);
}

void AndroidGraphics3dManager::initSize(uint width, uint height,
                                        const Graphics::PixelFormat *format) {
	// resize game texture
	ENTER("%d, %d, %p", width, height, format);

	// We do only 3D with this manager and in 3D there is no format
	assert(format == nullptr);

	bool engineSupportsArbitraryResolutions = !g_engine ||
	        g_engine->hasFeature(Engine::kSupportsArbitraryResolutions);
	if (engineSupportsArbitraryResolutions) {
		width = JNI::egl_surface_width;
		height = JNI::egl_surface_height;
	}

	GLTHREADCHECK;

	_game_texture->allocBuffer(width, height);
	_game_texture->setGameTexture();

	delete _frame_buffer;

	if (!engineSupportsArbitraryResolutions) {
		_frame_buffer = new OpenGL::FrameBuffer(_game_texture->getTextureName(),
		                                        _game_texture->width(), _game_texture->height(),
		                                        _game_texture->texWidth(), _game_texture->texHeight());
		_frame_buffer->attach();
	}

	// Don't know mouse size yet - it gets reallocated in
	// setMouseCursor.  We need the palette allocated before
	// setMouseCursor however, so just take a guess at the desired
	// size (it's small).
	_mouse_texture_palette->allocBuffer(20, 20);

	updateScreenRect();

	clearScreen(kClear);
}

int AndroidGraphics3dManager::getScreenChangeID() const {
	return _screenChangeID;
}

bool AndroidGraphics3dManager::showMouse(bool visible) {
	ENTER("%d", visible);

	_show_mouse = visible;

	return true;
}

void AndroidGraphics3dManager::warpMouse(int x, int y) {
	// x and y are in virtual coordinates
	ENTER("%d, %d", x, y);

	// Check active coordinate instead of screen coordinate to avoid warping
	// the mouse if it is still within the same virtual pixel
	// Don't take the risk of modifying _cursorX and _cursorY
	int cx = _cursorX, cy = _cursorY;
	const Common::Point currentMouse = convertScreenToVirtual(cx, cy);
	if (currentMouse.x == x && currentMouse.y == y) {
		// Same virtual coordinates: nothing to do
		return;
	}

	const Common::Point sMouse = convertVirtualToScreen(x, y);

	// Our internal mouse position is in screen coordinates
	// convertVirtualToScreen just clipped coordinates so we are safe
	setMousePosition(sMouse.x, sMouse.y);

	// Events pushed to Android system are in screen coordinates too
	// They are converted back by notifyMousePosition later
	Common::Event e;
	e.type = Common::EVENT_MOUSEMOVE;
	e.mouse = sMouse;

	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(e);
}

void AndroidGraphics3dManager::updateCursorScaling() {
	// By default we use the unscaled versions.
	_mouse_hotspot_scaled = _mouse_hotspot;
	_mouse_width_scaled = _mouse_texture->width();
	_mouse_height_scaled = _mouse_texture->height();

	// In case scaling is actually enabled we will scale the cursor according
	// to the game screen.
	uint16 w = getWidth();
	uint16 h = getHeight();

	if (!_mouse_dont_scale && w && h) {
		const frac_t screen_scale_factor_x = intToFrac(_game_texture->getDrawRect().width()) / w;
		const frac_t screen_scale_factor_y = intToFrac(_game_texture->getDrawRect().height()) / h;

		_mouse_hotspot_scaled = Common::Point(
			fracToInt(_mouse_hotspot_scaled.x * screen_scale_factor_x),
			fracToInt(_mouse_hotspot_scaled.y * screen_scale_factor_y));

		_mouse_width_scaled  = fracToInt(_mouse_width_scaled * screen_scale_factor_x);
		_mouse_height_scaled = fracToInt(_mouse_height_scaled * screen_scale_factor_y);
	}
}

void AndroidGraphics3dManager::setMouseCursor(const void *buf, uint w, uint h,
        int hotspotX, int hotspotY,
        uint32 keycolor, bool dontScale,
        const Graphics::PixelFormat *format, const byte *mask) {
	ENTER("%p, %u, %u, %d, %d, %u, %d, %p, %p", buf, w, h, hotspotX, hotspotY,
	      keycolor, dontScale, format, mask);

	if (mask)
		warning("AndroidGraphics3dManager::setMouseCursor: Masks are not supported");

	GLTHREADCHECK;

#ifdef USE_RGB_COLOR
	if (format && format->bytesPerPixel > 1) {
		if (_mouse_texture != _mouse_texture_rgb) {
			LOGD("switching to rgb mouse cursor");

			assert(!_mouse_texture_rgb);
			if (JNI::egl_bits_per_pixel == 16) {
				_mouse_texture_rgb = new GLES5551Texture();
			} else {
				_mouse_texture_rgb = new GLES8888Texture();
			}
			_mouse_texture_rgb->setLinearFilter(_graphicsMode == 1);
			_mouse_texture_rgb->reinit();
		}

		_mouse_texture = _mouse_texture_rgb;
	} else {
		if (_mouse_texture != _mouse_texture_palette) {
			LOGD("switching to paletted mouse cursor");
		}

		_mouse_texture = _mouse_texture_palette;

		delete _mouse_texture_rgb;
		_mouse_texture_rgb = 0;
	}
#endif

	_mouse_texture->allocBuffer(w, h);

	if (_mouse_texture == _mouse_texture_palette) {
		assert(keycolor < 256);
		_mouse_texture->setKeycolor(keycolor);
	}

	if (w == 0 || h == 0) {
		return;
	}

	if (_mouse_texture == _mouse_texture_palette) {
		_mouse_texture->updateBuffer(0, 0, w, h, buf, w);
	} else {
		uint16 pitch = _mouse_texture->pitch();
		uint16 bpp = _mouse_texture->getPixelFormat().bytesPerPixel;

		byte *tmp = new byte[pitch * h];

		// meh, a n-bit cursor without alpha bits... this is so silly
		if (!crossBlit(tmp, (const byte *)buf, pitch, w * format->bytesPerPixel, w, h,
		               _mouse_texture->getPixelFormat(),
		               *format)) {
			LOGE("crossblit failed");

			delete[] tmp;

			_mouse_texture->allocBuffer(0, 0);

			return;
		}

		if (format->bytesPerPixel == 2) {
			const uint16 *s = (const uint16 *)buf;
			byte *d = tmp;
			for (uint16 y = 0; y < h; ++y, d += pitch / 2 - w)
				for (uint16 x = 0; x < w; ++x, d++)
					if (*s++ == (keycolor & 0xffff)) {
						memset(d, 0, bpp);
					}
		} else if (format->bytesPerPixel == 4) {
			const uint32 *s = (const uint32 *)buf;
			byte *d = tmp;
			for (uint16 y = 0; y < h; ++y, d += pitch / 2 - w)
				for (uint16 x = 0; x < w; ++x, d++)
					if (*s++ == (keycolor & 0xffffffff)) {
						memset(d, 0, bpp);
					}
		} else {
			error("AndroidGraphics3dManager::setMouseCursor: invalid bytesPerPixel %d", format->bytesPerPixel);
		}

		_mouse_texture->updateBuffer(0, 0, w, h, tmp, pitch);

		delete[] tmp;
	}

	_mouse_hotspot = Common::Point(hotspotX, hotspotY);
	_mouse_dont_scale = dontScale;

	updateCursorScaling();
}

void AndroidGraphics3dManager::setCursorPalette(const byte *colors,
        uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

	GLTHREADCHECK;

	if (!_mouse_texture->hasPalette()) {
		LOGD("switching to paletted mouse cursor");

		_mouse_texture = _mouse_texture_palette;

		delete _mouse_texture_rgb;
		_mouse_texture_rgb = 0;
	}

	_mouse_texture->setPalette(colors, start, num);
}

bool AndroidGraphics3dManager::lockMouse(bool lock) {
	_show_mouse = lock;
	return true;
}

#ifdef USE_RGB_COLOR
Graphics::PixelFormat AndroidGraphics3dManager::getScreenFormat() const {
	return _game_texture->getPixelFormat();
}

Common::List<Graphics::PixelFormat> AndroidGraphics3dManager::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> res;

	// empty list

	return res;
}
#endif

void AndroidGraphics3dManager::updateScreenRect() {
	Common::Rect rect(0, 0, JNI::egl_surface_width, JNI::egl_surface_height);

	_overlay_texture->setDrawRect(rect);

	// Clear the overlay background so it is not displayed distorted while resizing
	_overlay_background->release();

	uint16 w = getWidth();
	uint16 h = getHeight();

	if (w && h && _ar_correction) {

		JNI::DPIValues dpi;
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
			if (_virtkeybd_on) {
				rect.moveTo(0, (JNI::egl_surface_height - rect.height()));
			} else {
				rect.moveTo(0, (JNI::egl_surface_height - rect.height()) / 2);
			}
		}
	}

	_game_texture->setDrawRect(rect);

	updateCursorScaling();
}

const GLESBaseTexture *AndroidGraphics3dManager::getActiveTexture() const {
	if (!_frame_buffer || _show_overlay) {
		return _overlay_texture;
	} else {
		return _game_texture;
	}
}

void AndroidGraphics3dManager::initOverlay() {
	// minimum of 320x200
	// (surface can get smaller when opening the virtual keyboard on *QVGA*)
	int overlay_width = MAX(JNI::egl_surface_width, 320);
	int overlay_height = MAX(JNI::egl_surface_height, 200);

	LOGI("overlay size is %ux%u", overlay_width, overlay_height);

	_overlay_texture->allocBuffer(overlay_width, overlay_height);
	_overlay_texture->setDrawRect(0, 0,
	                              JNI::egl_surface_width, JNI::egl_surface_height);
}

void AndroidGraphics3dManager::clearScreen(FixupType type, byte count) {
	assert(count > 0);

	bool sm = _show_mouse;
	_show_mouse = false;

	CONTEXT_SAVE_STATE(GL_SCISSOR_TEST);
	CONTEXT_SET_DISABLE(GL_SCISSOR_TEST);

	for (byte i = 0; i < count; ++i) {
		// clear screen
		GLCALL(glClearColor(0.f, 0.f, 0.f, 1.f));
		GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

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

	CONTEXT_RESET_DISABLE(GL_SCISSOR_TEST);

	_show_mouse = sm;
	_force_redraw = true;
}

float AndroidGraphics3dManager::getHiDPIScreenFactor() const {
	JNI::DPIValues dpi;
	JNI::getDPI(dpi);
	// Scale down the Android factor else the GUI is too big and
	// there is not much options to go smaller
	return dpi[2] / 1.2f;
}

AndroidCommonGraphics::State AndroidGraphics3dManager::getState() const {
	AndroidCommonGraphics::State state;

	state.screenWidth   = getWidth();
	state.screenHeight  = getHeight();
	state.aspectRatio   = getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	state.fullscreen    = getFeatureState(OSystem::kFeatureFullscreenMode);
	state.cursorPalette = getFeatureState(OSystem::kFeatureCursorPalette);
#ifdef USE_RGB_COLOR
	state.pixelFormat   = _2d_pixel_format;
#endif
	return state;
}

bool AndroidGraphics3dManager::setState(const AndroidCommonGraphics::State &state) {
	// In 3d we don't have a pixel format so we ignore it but store it for when leaving 3d mode
	initSize(state.screenWidth, state.screenHeight, nullptr);
#ifdef USE_RGB_COLOR
	_2d_pixel_format = state.pixelFormat;
#endif
	setFeatureState(OSystem::kFeatureAspectRatioCorrection, state.aspectRatio);
	setFeatureState(OSystem::kFeatureFullscreenMode, state.fullscreen);
	setFeatureState(OSystem::kFeatureCursorPalette, state.cursorPalette);

	return true;
}

void AndroidGraphics3dManager::touchControlInitSurface(const Graphics::ManagedSurface &surf) {
	if (_touchcontrols_texture->width() == surf.w && _touchcontrols_texture->height() == surf.h) {
		return;
	}

	_touchcontrols_texture->allocBuffer(surf.w, surf.h);
	Graphics::Surface *dst = _touchcontrols_texture->surface();

	Graphics::crossBlit(
			(byte *)dst->getPixels(), (const byte *)surf.getPixels(),
			dst->pitch, surf.pitch,
			surf.w, surf.h,
			surf.format, dst->format);
}

void AndroidGraphics3dManager::touchControlNotifyChanged() {
	// Make sure we redraw the screen
	_force_redraw = true;
}

void AndroidGraphics3dManager::touchControlDraw(uint8 alpha, int16 x, int16 y, int16 w, int16 h, const Common::Rect &clip) {
	_touchcontrols_texture->setAlpha(alpha / 255.f);
	_touchcontrols_texture->drawTexture(x, y, w, h, clip);
}

void AndroidGraphics3dManager::applyTouchSettings() const {
	// in 3D, maybe in GUI
	dynamic_cast<OSystem_Android *>(g_system)->applyTouchSettings(true, _show_overlay && _overlay_in_gui);
}

void AndroidGraphics3dManager::syncVirtkeyboardState(bool virtkeybd_on) {
	_virtkeybd_on = virtkeybd_on;
	updateScreenRect();
	_force_redraw = true;
}
