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

#include "ultima/shared/gfx/visual_surface.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/gfx/font.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

VisualSurface::VisualSurface(const Graphics::ManagedSurface &src, const Rect &bounds, GameBase *game) :
		_bounds(bounds), _textColor(255), _bgColor(0) {
	copyFrom(src);
	Shared::Game *sharedGame = dynamic_cast<Shared::Game *>(game);
	if (sharedGame) {
		_textColor = sharedGame->_textColor;
		_bgColor = sharedGame->_bgColor;
	}
}

void VisualSurface::drawPoint(const Point &pt, byte color) {
	fillRect(Rect(pt.x, pt.y, pt.x + 1, pt.y + 1), color);
}

void VisualSurface::writeString(const Common::String &msg, const Point &pt, int color, int bgColor) {
	_textPos = pt;
	writeString(msg, color == -1 ? _textColor : color, bgColor == -1 ? _bgColor : bgColor);
}

void VisualSurface::writeString(const Common::String &msg, int color, int bgColor) {
	Gfx::Font *font = g_vm->_game->getFont();
	font->writeString(*this, msg, _textPos, color == -1 ? _textColor : color, bgColor == -1 ? _bgColor : bgColor);
}

void VisualSurface::writeChar(unsigned char c, const Point &pt, int color, int bgColor) {
	_textPos = pt;
	writeChar(c, color == -1 ? _textColor : color, bgColor == -1 ? _bgColor : bgColor);
}

void VisualSurface::writeChar(unsigned char c, int color, int bgColor) {
	Gfx::Font *font = g_vm->_game->getFont();
	font->writeChar(*this, c, _textPos, color == -1 ? _textColor : color, bgColor == -1 ? _bgColor : bgColor);
}

size_t VisualSurface::fontHeight() {
	Gfx::Font *font = g_vm->_game->getFont();
	return font->lineHeight();
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
