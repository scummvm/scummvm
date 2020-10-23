/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/opengls.h"

#include "common/config-manager.h"

#include "graphics/surface.h"
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
#include "graphics/opengl/context.h"
#endif

#include "engines/util.h"

namespace Stark {
namespace Gfx {

Driver *Driver::create() {
	Driver *driver = nullptr;
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	initGraphics3d(kOriginalWidth, kOriginalHeight);

	bool backendCapableOpenGL = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);

	if (backendCapableOpenGL) {
#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
		if (OpenGLContext.shadersSupported) {
			driver = new OpenGLSDriver();
		} else {
			error("Your system does not have the required OpenGL capabilities");
		}
#endif
	}
#endif
	if (driver)
		return driver;
	error("No renderers have been found for this game");
}

const Graphics::PixelFormat Driver::getRGBAPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

bool Driver::computeScreenViewport() {
	int32 screenWidth = g_system->getWidth();
	int32 screenHeight = g_system->getHeight();

	Common::Rect viewport;
	if (g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
		// Aspect ratio correction
		int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * kOriginalWidth / kOriginalHeight);
		int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * kOriginalHeight / kOriginalWidth);
		viewport = Common::Rect(viewportWidth, viewportHeight);

		// Pillarboxing
		viewport.translate((screenWidth - viewportWidth) / 2,
			(screenHeight - viewportHeight) / 2);
	} else {
		// Aspect ratio correction disabled, just stretch
		viewport = Common::Rect(screenWidth, screenHeight);
	}

	if (viewport == _screenViewport) {
		return false;
	}

	_screenViewport = viewport;
	return true;
}

Common::Rect Driver::gameViewport() const {
	Common::Rect game = Common::Rect(_screenViewport.width(), _screenViewport.height() * kGameViewportHeight / kOriginalHeight);
	game.translate(_screenViewport.left, _screenViewport.top + _screenViewport.height() * kTopBorderHeight / kOriginalHeight);

	return game;
}

Common::Point Driver::convertCoordinateCurrentToOriginal(const Common::Point &point) const {
	// Most of the engine expects 640x480 coordinates
	Common::Point scaledPosition = point;
	scaledPosition.x -= _screenViewport.left;
	scaledPosition.y -= _screenViewport.top;
	scaledPosition.x = CLIP<int16>(scaledPosition.x, 0, _screenViewport.width());
	scaledPosition.y = CLIP<int16>(scaledPosition.y, 0, _screenViewport.height());
	scaledPosition.x *= kOriginalWidth / (float)_screenViewport.width();
	scaledPosition.y *= kOriginalHeight / (float)_screenViewport.height();

	return scaledPosition;
}

uint Driver::scaleWidthOriginalToCurrent(uint width) const {
	return _screenViewport.width() * width / kOriginalWidth;
}

uint Driver::scaleHeightOriginalToCurrent(uint height) const {
	return _screenViewport.height() * height / kOriginalHeight;
}

uint Driver::scaleWidthCurrentToOriginal(uint width) const {
	return kOriginalWidth * width / _screenViewport.width();
}

uint Driver::scaleHeightCurrentToOriginal(uint height) const {
	return kOriginalHeight * height / _screenViewport.height();
}

void Driver::flipVertical(Graphics::Surface *s) {
	for (int y = 0; y < s->h / 2; ++y) {
		// Flip the lines
		byte *line1P = (byte *)s->getBasePtr(0, y);
		byte *line2P = (byte *)s->getBasePtr(0, s->h - y - 1);

		for (int x = 0; x < s->pitch; ++x)
			SWAP(line1P[x], line2P[x]);
	}
}

bool Driver::isPosInScreenBounds(const Common::Point &point) const {
    return _screenViewport.contains(point);
}

} // End of namespace Gfx
} // End of namespace Stark
