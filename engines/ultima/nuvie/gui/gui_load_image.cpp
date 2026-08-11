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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui_load_image.h"
#include "ultima/nuvie/gui/the_font.h"

namespace Ultima {
namespace Nuvie {

/************************************************************************/
/* C functions for image loading                                        */
/*                                                                      */
/************************************************************************/

Graphics::ManagedSurface *GUI_LoadImage(int w, int h, const uint8 *pal, const uint8 *data) {
	Graphics::ManagedSurface *image = new Graphics::ManagedSurface(w, h,
		Graphics::PixelFormat::createFormatCLUT8());

	if (image) {
		for (int row = 0; row < h; ++row) {
			memcpy((uint8 *)image->getBasePtr(0, row), data, w);
			data += w;
		}

		image->setPalette(pal, 0, 256);
	}

	return image;
}

/************************************************************************/
/* C functions for default font support                                 */
/*                                                                      */
/************************************************************************/

static Graphics::ManagedSurface *the_font = nullptr;
static Graphics::ManagedSurface *the_font_6x8 = nullptr;
static Graphics::ManagedSurface *the_font_gump = nullptr;

Graphics::ManagedSurface *GUI_DefaultFont(void) {
	if (the_font == nullptr) {
		the_font = GUI_LoadImage(font_w, font_h, font_pal, font_data);
	}
	return the_font;
}

Graphics::ManagedSurface *GUI_Font6x8(void) {
	if (the_font_6x8 == nullptr) {
		the_font_6x8 = GUI_LoadImage(font_6x8_w, font_6x8_h, font_pal, font_6x8_data);
	}
	return the_font_6x8;
}

Graphics::ManagedSurface *GUI_FontGump(void) {
	if (the_font_gump == nullptr) {
		the_font_gump = GUI_LoadImage(font_gump_w, font_gump_h, font_pal, font_gump_data);
	}
	return the_font_gump;
}

const uint8 *GUI_FontGumpWData(void) {
	return font_gump_w_data;
}

} // End of namespace Nuvie
} // End of namespace Ultima
