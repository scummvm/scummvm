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

#include "graphics/surface.h"
#include "graphics/font.h"
#include "graphics/fontman.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/opengls.h"

#include "common/textconsole.h"

namespace Stark {
namespace Gfx {

Driver *Driver::create() {
	Driver *driver = NULL;

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
	// OpenGL Shaders
	driver = new OpenGLSDriver();
	if (driver)
		return driver;
#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

	error("Couldn't instance any graphics driver");
}

Texture *Driver::createTextureFromString(const Common::String &str, uint32 color) {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);

	// TODO: The hardcoded widths here are not exactly perfect.
	Common::Array<Common::String> lines;
	font->wordWrapText(str, 580, lines);

	int height = font->getFontHeight();
	int width = 0;
	for (int i = 0; i < lines.size(); i++) {
		width = MAX(width, font->getStringWidth(lines[i]));
	}

	Graphics::Surface surface;
	surface.create(width, height*lines.size(), Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	for (int i = 0; i < lines.size(); i++) {
		font->drawString(&surface, lines[i], 0, height*i, 580, color);
	}
	Texture *texture = createTexture(&surface);
	surface.free();
	return texture;
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

Common::Point Driver::getScreenPosBounded(Common::Point point) {
	Common::Point boundedPos = point;
	boundedPos.x = CLIP<int16>(boundedPos.x, _screenViewport.left, _screenViewport.right);
	return boundedPos;
}

Common::Point Driver::scalePoint(Common::Point point) {
	// Most of the engine expects 640x480 coordinates
	Common::Point scaledPosition = point;
	scaledPosition.x -= _screenViewport.left;
	scaledPosition.y -= _screenViewport.top;
	scaledPosition.x = CLIP<int16>(scaledPosition.x, 0, _screenViewport.width());
	scaledPosition.y = CLIP<int16>(scaledPosition.y, 0, _screenViewport.height());
	scaledPosition.x *= Gfx::Driver::kOriginalWidth / (float)_screenViewport.width();
	scaledPosition.y *= Gfx::Driver::kOriginalHeight / (float)_screenViewport.height();

	return scaledPosition;
}

} // End of namespace Gfx
} // End of namespace Stark
