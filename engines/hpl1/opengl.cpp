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

#include "hpl1/opengl.h"
#include "common/rect.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "hpl1/debug.h"

#ifdef USE_OPENGL

namespace Hpl1 {

static const char *getErrorString(const GLenum code) {
	switch (code) {
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	}
	return "unrecognized error";
}

void checkOGLErrors(const char *function, const char *file, int line) {
	GLenum code;
	while ((code = glGetError()) != GL_NO_ERROR)
		logError(kDebugOpenGL, "Opengl error: \'%s\' in function %s (%s:%d)\n", getErrorString(code), function, file, line);
}

static Common::Rect getGLViewport() {
	int viewportSize[4];
	GL_CHECK(glGetIntegerv(GL_VIEWPORT, viewportSize));
	return Common::Rect(viewportSize[0], viewportSize[1], viewportSize[2], viewportSize[3]);
}

static Graphics::PixelFormat getRGBAPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

Common::ScopedPtr<Graphics::Surface> createGLViewportScreenshot() {
	Common::ScopedPtr<Graphics::Surface> screen(new Graphics::Surface());
	Common::Rect viewportSize = getGLViewport();
	screen->create(viewportSize.width(), viewportSize.height(), getRGBAPixelFormat());
	GL_CHECK(glReadPixels(viewportSize.left, g_system->getHeight() - viewportSize.bottom, viewportSize.width(), viewportSize.height(), GL_RGBA, GL_UNSIGNED_BYTE, screen->getPixels()));
	screen->flipVertical(Common::Rect(screen->w, screen->h));
	return screen;
}

} // End of namespace Hpl1

#endif // USE_OPENGL
