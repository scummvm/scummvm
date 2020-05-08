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

#ifndef GRAPHICS_TEXT_RENDERER_H
#define GRAPHICS_TEXT_RENDERER_H

#include "common/str.h"
#include "common/ustr.h"
#include "common/str-bidi.h"
#include "graphics/font.h"

namespace Graphics {

/**
 * high-level methods for drawing strings (with bidi support).
 */
namespace TextRenderer {

	// TODO: Add doxygen comments to this
	void drawString(Surface *dst, const Graphics::Font &font, const Common::String &str, const Common::CodePage page, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true);
	void drawU32String(Surface *dst, const Graphics::Font &font, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0);
	void drawString(ManagedSurface *dst, const Graphics::Font &font, const Common::String &str, const Common::CodePage page, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true);
	void drawU32String(ManagedSurface *dst, const Graphics::Font &font, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0);
}

} // End of namespace Graphics

#endif
