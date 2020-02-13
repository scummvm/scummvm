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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui_load_image.h"
#include "ultima/nuvie/gui/the_font.h"

namespace Ultima {
namespace Nuvie {

/************************************************************************/
/* C functions for image loading                                        */
/*                                                                      */
/************************************************************************/

Graphics::ManagedSurface *GUI_LoadImage(int w, int h, uint8 *pal, uint8 *data) {
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

static Graphics::ManagedSurface *the_font = NULL;
static Graphics::ManagedSurface *the_font_6x8 = NULL;
static Graphics::ManagedSurface *the_font_gump = NULL;

Graphics::ManagedSurface *GUI_DefaultFont(void) {
	if (the_font == NULL) {
		the_font = GUI_LoadImage(font_w, font_h, font_pal, font_data);
	}
	return (the_font);
}

Graphics::ManagedSurface *GUI_Font6x8(void) {
	if (the_font_6x8 == NULL) {
		the_font_6x8 = GUI_LoadImage(font_6x8_w, font_6x8_h, font_pal, font_6x8_data);
	}
	return (the_font_6x8);
}

Graphics::ManagedSurface *GUI_FontGump(void) {
	if (the_font_gump == NULL) {
		the_font_gump = GUI_LoadImage(font_gump_w, font_gump_h, font_pal, font_gump_data);
	}
	return (the_font_gump);
}

uint8 *GUI_FontGumpWData(void) {
	return font_gump_w_data;
}

} // End of namespace Nuvie
} // End of namespace Ultima
