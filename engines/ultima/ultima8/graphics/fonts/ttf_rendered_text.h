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

#ifndef ULTIMA8_GRAPHICS_FONTS_SHAPERENDEREDTEXT_H
#define ULTIMA8_GRAPHICS_FONTS_SHAPERENDEREDTEXT_H

#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima8 {

class TTFont;
class Texture;

class TTFRenderedText : public RenderedText {
public:
	TTFRenderedText(Graphics::ManagedSurface *texture, int width, int height, int vlead,
	                TTFont *font);
	~TTFRenderedText() override;

	void draw(RenderSurface *surface, int x, int y,
	                  bool destmasked = false) override;
	void drawBlended(RenderSurface *surface, int x, int y, uint32 col,
	                         bool destmasked = false) override;

protected:
	Graphics::ManagedSurface *_texture;
	TTFont *_font;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
