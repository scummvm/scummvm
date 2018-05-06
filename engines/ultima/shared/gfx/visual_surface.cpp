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

#include "ultima/shared/gfx/visual_surface.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game_base.h"
#include "ultima/shared/gfx/font.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

VisualSurface::VisualSurface(const Graphics::ManagedSurface &src, const Rect &bounds) :
		Graphics::ManagedSurface(src), _bounds(bounds) {
}

void VisualSurface::drawPoint(const Point &pt, byte color) {
	fillRect(Rect(pt.x, pt.y, pt.x + 1, pt.y + 1), color);
}

void VisualSurface::writeString(const Common::String &msg, const Point &pt, byte color) {
	_textPos = pt;
	writeString(msg, color);
}

void VisualSurface::writeString(const Common::String &msg, byte color) {
	Gfx::Font *font = g_vm->_game->getFont();
	_textPos.x += font->writeString(*this, msg, _textPos, color);
}

void VisualSurface::writeChar(unsigned char c, const Point &pt, byte color) {
	_textPos = pt;
	writeChar(c, color);
}

void VisualSurface::writeChar(unsigned char c, byte color) {
	Gfx::Font *font = g_vm->_game->getFont();
	font->writeChar(*this, c, _textPos, color);
	_textPos.x += font->charWidth(c);
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
