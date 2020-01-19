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
 * GNU General Public License for more detail_surface.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

DrawingSupport::DrawingSupport(const Shared::Gfx::VisualSurface &s) : _surface(s) {
	_game = static_cast<Ultima1Game *>(g_vm->_game);
}

void DrawingSupport::drawFrame() {
	// Big border around the screen
	_surface.fillRect(Rect(0, 0, 317, 7), _game->_borderColor);
	_surface.fillRect(Rect(0, 6, 7, 200), _game->_borderColor);
	_surface.fillRect(Rect(313, 7, 320, 200), _game->_borderColor);
	_surface.fillRect(Rect(0, 193, 320, 200), _game->_borderColor);

	// Thin line on edge of big border
	_surface.vLine(7, 7, 192, _game->_edgeColor);
	_surface.vLine(312, 7, 192, _game->_edgeColor);
	_surface.hLine(7, 7, 312, _game->_edgeColor);
	_surface.hLine(7, 192, 312, _game->_edgeColor);

}

void DrawingSupport::drawGameFrame() {
	// Big border around the screen
	_surface.fillRect(Rect(0, 0, 317, 7), _game->_borderColor);
	_surface.fillRect(Rect(0, 153, 320, 159), _game->_borderColor);
	_surface.fillRect(Rect(0, 6, 7, 159), _game->_borderColor);
	_surface.fillRect(Rect(313, 7, 320, 159), _game->_borderColor);

	// Thin line on edge of big border
	_surface.vLine(7, 7, 152, _game->_edgeColor);
	_surface.vLine(312, 7, 152, _game->_edgeColor);
	_surface.hLine(7, 7, 312, _game->_edgeColor);
	_surface.hLine(7, 152, 312, _game->_edgeColor);

	// Draw separator at bottom of screen
	_surface.fillRect(Rect(241, 153, 247, 200), _game->_borderColor);
	_surface.hLine(0, 159, 240, _game->_edgeColor);
	_surface.hLine(247, 159, 320, _game->_edgeColor);
	_surface.vLine(240, 159, 200, _game->_edgeColor);
	_surface.vLine(247, 159, 200, _game->_edgeColor);
}

void DrawingSupport::roundFrameCorners(bool skipBottom) {
	// Round the edges of the big outer border
	for (int idx = 1; idx <= 4; ++idx) {
		_surface.drawLine(0, idx, idx, 0, 0);
		_surface.drawLine(319 - idx, 0, 319, idx, 0);
		
		if (!skipBottom) {
			_surface.drawLine(0, 199 - idx, idx, 199, 0);
			_surface.drawLine(319, 199 - idx, 319 - idx, 199, 0);
		}
	}

	_surface.drawPoint(Point(0, 0), 0);
	_surface.drawPoint(Point(0, 5), 0);
	_surface.drawPoint(Point(5, 0), 0);
	_surface.drawPoint(Point(319, 0), 0);
	_surface.drawPoint(Point(314, 0), 0);
	_surface.drawPoint(Point(319, 5), 0);

	if (!skipBottom) {
		_surface.drawPoint(Point(0, 199), 0);
		_surface.drawPoint(Point(0, 194), 0);
		_surface.drawPoint(Point(5, 199), 0);
		_surface.drawPoint(Point(319, 199), 0);
		_surface.drawPoint(Point(319, 194), 0);
		_surface.drawPoint(Point(314, 199), 0);
	}
}

void DrawingSupport::drawRightArrow(const Point &pt) {
	_surface.writeChar(16, pt, _game->_borderColor);
	_surface.drawLine(pt.x, pt.y, pt.x + 7, pt.y + 3, _game->_edgeColor);
	_surface.drawLine(pt.x + 7, pt.y + 3, pt.x, pt.y + 7, _game->_edgeColor);
	_surface.drawLine(pt.x, pt.y + 1, pt.x, pt.y + 6, _game->_borderColor);
}

void DrawingSupport::drawLeftArrow(const Point &pt) {
	_surface.writeChar(17, pt, _game->_borderColor);
	_surface.drawLine(pt.x + 7, pt.y, pt.x, pt.y + 3, _game->_edgeColor);
	_surface.drawLine(pt.x, pt.y + 3, pt.x + 7, pt.y + 7, _game->_edgeColor);
	_surface.drawLine(pt.x + 7, pt.y + 1, pt.x + 7, pt.y + 6, _game->_borderColor);
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
