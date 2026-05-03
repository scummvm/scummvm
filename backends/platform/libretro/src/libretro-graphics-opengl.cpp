/* Copyright (C) 2024 Giovanni Cascione <ing.cascione@gmail.com>
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
#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/opengl/framebuffer.h"
#include "graphics/opengl/debug.h"

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-graphics-opengl.h"

#include "gui/gui-manager.h"

LibretroOpenGLGraphics::LibretroOpenGLGraphics(OpenGL::ContextType contextType) {
	resetContext(contextType);
}

void LibretroOpenGLGraphics::refreshScreen() {
	dynamic_cast<LibretroTimerManager *>(LIBRETRO_G_SYSTEM->getTimerManager())->checkThread(THREAD_SWITCH_UPDATE);
}

void LibretroOpenGLGraphics::setMousePosition(int x, int y) {
	OpenGL::OpenGLGraphicsManager::setMousePosition(x, y);
}

void LibretroOpenGLGraphics::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, const Graphics::PixelFormat *format, const byte *mask, frac_t scaleX, frac_t scaleY) {
	/* Workaround to fix a cursor glitch (e.g. GUI with Classic theme) occurring when any overlay is activated from retroarch (e.g. keyboard overlay).
	   Currently no feedback is available from frontend to detect if overlays are toggled to delete _cursor only if needed.
	   @TODO: root cause to be investigated. */
	delete _cursor;
	_cursor = nullptr;

	OpenGL::OpenGLGraphicsManager::setMouseCursor(buf, w, h, hotspotX, hotspotY, keycolor, format, mask, scaleX, scaleY);

	overrideCursorScaling();

}

void LibretroOpenGLGraphics::overrideCursorScaling() {
	OpenGL::OpenGLGraphicsManager::recalculateCursorScaling();

	if (_cursor) {
		const float screenScaleFactorX = (_cursorScaleX == 0 || ! _overlayVisible) ? 1.0f : (float)getWindowHeight() / 200; /* hard coded as base resolution 320x200 is hard coded upstream */
		const float screenScaleFactorY = (_cursorScaleY == 0 || ! _overlayVisible) ? 1.0f : (float)getWindowHeight() / 200; /* hard coded as base resolution 320x200 is hard coded upstream */

		const float cursorScaleFactorX = screenScaleFactorX * _cursorScaleX;
		const float cursorScaleFactorY = screenScaleFactorY * _cursorScaleY;

		_cursorHotspotXScaled = _cursorHotspotX * cursorScaleFactorX;
		_cursorWidthScaled    = _cursor->getWidth() * cursorScaleFactorX;

		_cursorHotspotYScaled = _cursorHotspotY * cursorScaleFactorY;
		_cursorHeightScaled   = _cursor->getHeight() * cursorScaleFactorY;
	}
}

void LibretroOpenGLGraphics::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	bool force_gui_redraw = false;
	/* Override for ScummVM Launcher */
	if (nullptr == ConfMan.getActiveDomain()) {
		/* 0 w/h is used to notify libretro gui res settings is changed */
		force_gui_redraw = (width == 0);
		width = retro_setting_get_gui_res_w();
		height = retro_setting_get_gui_res_h();
	}
	/* no need to update now libretro gui res settings changes if not in ScummVM launcher */
	if (! width)
		return;

	retro_set_size(width, height);
	handleResize(width, height);
	OpenGL::OpenGLGraphicsManager::initSize(width, height, format);
	LIBRETRO_G_SYSTEM->refreshRetroSettings();

	if (force_gui_redraw)
		g_gui.checkScreenChange();
}

OSystem::TransactionError LibretroOpenGLGraphics::endGFXTransaction() {
	OSystem::TransactionError res = OpenGL::OpenGLGraphicsManager::endGFXTransaction();
	overrideCursorScaling();

	return res;
}

void LibretroOpenGLGraphics::handleResizeImpl(const int width, const int height) {
	OpenGL::OpenGLGraphicsManager::handleResizeImpl(width, height);
	overrideCursorScaling();
}

bool LibretroOpenGLGraphics::hasFeature(OSystem::Feature f) const {
	return
#ifdef SCUMMVM_NEON
	(f == OSystem::kFeatureCpuNEON) ||
#endif
#ifdef USE_OPENGL_GAME
	(f == OSystem::kFeatureOpenGLForGame) ||
#endif
#ifdef USE_OPENGL_SHADERS
	(f == OSystem::kFeatureShadersForGame) ||
#endif
	OpenGL::OpenGLGraphicsManager::hasFeature(f);
}

void LibretroHWFramebuffer::activateInternal() {
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, retro_get_hw_fb()));
}

void LibretroOpenGLGraphics::resetContext(OpenGL::ContextType contextType) {
	const Graphics::PixelFormat rgba8888 = OpenGL::Texture::getRGBAPixelFormat();
	notifyContextDestroy();
	notifyContextCreate(contextType, new LibretroHWFramebuffer(), rgba8888, rgba8888);

	if (_overlayInGUI)
		g_gui.checkScreenChange();
}
