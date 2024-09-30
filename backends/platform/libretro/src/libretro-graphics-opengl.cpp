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

void LibretroOpenGLGraphics::refreshScreen(){
	dynamic_cast<LibretroTimerManager *>(LIBRETRO_G_SYSTEM->getTimerManager())->checkThread(THREAD_SWITCH_UPDATE);
}

void LibretroOpenGLGraphics::setMousePosition(int x, int y){
	OpenGL::OpenGLGraphicsManager::setMousePosition(x, y);
}

void LibretroOpenGLGraphics::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	/* Workaround to fix a cursor glitch (e.g. GUI with Classic theme) occurring when any overlay is activated from retroarch (e.g. keyboard overlay).
	   Currently no feedback is available from frontend to detect if overlays are toggled to delete _cursor only if needed.
	   @TODO: root cause to be investigated. */
	delete _cursor;
	_cursor = nullptr;
	OpenGL::OpenGLGraphicsManager::setMouseCursor(buf, w, h, hotspotX, hotspotY, keycolor, dontScale, format, mask);
}

void LibretroOpenGLGraphics::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	/* Override for ScummVM Launcher */
	if (nullptr == ConfMan.getActiveDomain()){
		width = RES_W_OVERLAY;
		height = RES_H_OVERLAY;
	}
	retro_set_size(width, height);
	handleResize(width, height);
	OpenGL::OpenGLGraphicsManager::initSize(width, height, format);
	LIBRETRO_G_SYSTEM->refreshRetroSettings();
}

void LibretroHWFramebuffer::activateInternal(){
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, retro_get_hw_fb()));
}

void LibretroOpenGLGraphics::resetContext(OpenGL::ContextType contextType) {
	const Graphics::PixelFormat rgba8888 =
#ifdef SCUMM_LITTLE_ENDIAN
									   Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#else
									   Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#endif
	notifyContextDestroy();
	notifyContextCreate(contextType, new LibretroHWFramebuffer(), rgba8888, rgba8888);

	if (_overlayInGUI)
		g_gui.scheduleFullRedraw();
}
