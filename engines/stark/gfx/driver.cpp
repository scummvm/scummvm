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
#ifdef USE_OPENGL
#include "graphics/opengl/context.h"
#endif

namespace Stark {
namespace Gfx {

Driver *Driver::create() {
#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
	bool fullscreen = ConfMan.getBool("fullscreen");
	g_system->setupScreen(kOriginalWidth, kOriginalHeight, fullscreen, true);

	if (OpenGLContext.shadersSupported) {
		return new OpenGLSDriver();
	} else {
		error("Your system does not have the required OpenGL capabilities");
	}
#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

	error("No renderers have been found for this game");
}

const Graphics::PixelFormat Driver::getRGBAPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

void Driver::toggleFullscreen() const {
	if (!g_system->hasFeature(OSystem::kFeatureFullscreenToggleKeepsContext)) {
		warning("Unable to toggle the fullscreen state because the current backend would destroy the graphics context");
		return;
	}

	bool oldFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
	g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !oldFullscreen);
}

void Driver::computeScreenViewport() {
	int32 screenWidth = g_system->getWidth();
	int32 screenHeight = g_system->getHeight();

	if (g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
		// Aspect ratio correction
		int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * kOriginalWidth / kOriginalHeight);
		int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * kOriginalHeight / kOriginalWidth);
		_screenViewport = Common::Rect(viewportWidth, viewportHeight);

		// Pillarboxing
		_screenViewport.translate((screenWidth - viewportWidth) / 2,
			(screenHeight - viewportHeight) / 2);
	} else {
		// Aspect ratio correction disabled, just stretch
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	}
}

Common::Rect Driver::gameViewport() const {
	Common::Rect game = Common::Rect(_screenViewport.width(), _screenViewport.height() * kGameViewportHeight / kOriginalHeight);
	game.translate(_screenViewport.left, _screenViewport.top + _screenViewport.height() * kBottomBorderHeight / kOriginalHeight);

	return game;
}

Common::Point Driver::getScreenPosBounded(const Common::Point &point) const {
	Common::Point boundedPos = point;
	boundedPos.x = CLIP<int16>(boundedPos.x, _screenViewport.left, _screenViewport.right);

	return boundedPos;
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

void Driver::flipVertical(Graphics::Surface *s) {
	for (int y = 0; y < s->h / 2; ++y) {
		// Flip the lines
		byte *line1P = (byte *)s->getBasePtr(0, y);
		byte *line2P = (byte *)s->getBasePtr(0, s->h - y - 1);

		for (int x = 0; x < s->pitch; ++x)
			SWAP(line1P[x], line2P[x]);
	}
}

} // End of namespace Gfx
} // End of namespace Stark
