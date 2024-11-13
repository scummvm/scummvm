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

#ifndef BACKENDS_LIBRETRO_GRAPHICS_OPENGL_H
#define BACKENDS_LIBRETRO_GRAPHICS_OPENGL_H

#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/opengl/texture.h"

namespace OpenGL {
class Surface;
}

class LibretroOpenGLGraphics : public OpenGL::OpenGLGraphicsManager {
public:
	LibretroOpenGLGraphics(OpenGL::ContextType contextType);
	bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) override {
		return true;
	};
	void refreshScreen() override;
	void setSystemMousePosition(const int x, const int y) override {};
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) override;
	void initSize(uint width, uint height, const Graphics::PixelFormat *format) override;
	void setMousePosition(int x, int y);
	void resetContext(OpenGL::ContextType contextType);
	OSystem::TransactionError endGFXTransaction() override;
	bool hasFeature(OSystem::Feature f) const override;
protected:
	bool gameNeedsAspectRatioCorrection() const override {
		return false;
	}
	void handleResizeImpl(const int width, const int height) override;
private:
	void overrideCursorScaling(void);
};

class LibretroHWFramebuffer : public OpenGL::Backbuffer {

protected:
	void activateInternal() override;
};

#endif //BACKENDS_LIBRETRO_GRAPHICS_OPENGL_H
