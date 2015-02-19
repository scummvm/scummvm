/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "graphics/surface.h"
#include "graphics/font.h"
#include "graphics/fontman.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/opengl.h"

#include "common/textconsole.h"

namespace Stark {
namespace Gfx {

Driver *Driver::create() {
	Driver *driver = NULL;

#ifdef USE_OPENGL
	// OpenGL
	driver = new OpenGLDriver();
	if (driver)
		return driver;
#endif // USE_OPENGL

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

} // End of namespace Gfx
} // End of namespace Stark
