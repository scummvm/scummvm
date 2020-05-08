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

#include "common/str-bidi.h"
#include "graphics/text_renderer.h"

#ifdef USE_FRIBIDI
#include <fribidi/fribidi.h>
#endif

namespace Graphics {

namespace TextRenderer {

void drawString(Surface *dst, const Graphics::Font &font, const Common::String &str, const Common::CodePage page, int x, int y, int w, uint32 color, TextAlign align, int deltax, bool useEllipsis) {
    font.drawString(dst, convertBiDiString(str, page), x, y, w, color, align, deltax, useEllipsis);
}

void drawU32String(Surface *dst, const Graphics::Font &font,const Common::U32String &str,  int x, int y, int w, uint32 color, TextAlign align, int deltax) {
    font.drawString(dst, convertBiDiU32String(str), x, y, w, color, align, deltax);
}

void drawString(ManagedSurface *dst, const Graphics::Font &font, const Common::String &str, const Common::CodePage page, int x, int y, int w, uint32 color, TextAlign align, int deltax, bool useEllipsis) {
    font.drawString(dst, convertBiDiString(str, page), x, y, w, color, align, deltax, useEllipsis);
}

void drawU32String(ManagedSurface *dst, const Graphics::Font &font, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align, int deltax) {
    font.drawString(dst, convertBiDiU32String(str), x, y, w, color, align, deltax);
}

} // End of namespace TextRenderer

} // End of namespace Graphics
