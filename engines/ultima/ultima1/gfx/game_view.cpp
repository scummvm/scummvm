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

#include "ultima/ultima1/gfx/game_view.h"
#include "ultima/shared/gfx/info.h"
#include "ultima/shared/gfx/status.h"
#include "ultima/shared/gfx/viewport_dungeon.h"
#include "ultima/shared/gfx/viewport_map.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {

GameView::GameView(TreeItem *parent) : Shared::Gfx::VisualContainer("GameView", Common::Rect(0, 0, 320, 200), parent) {
	_info = new Shared::Info(this);
	_status = new Shared::Status(this);
	_viewportDungeon = new Shared::ViewportDungeon(this);
	_viewportMap = new Shared::ViewportMap(this);
}

GameView::~GameView() {
	delete _info;
	delete _status;
	delete _viewportDungeon;
	delete _viewportMap;
}

void GameView::draw() {
	drawFrame();
	Shared::Gfx::VisualContainer::draw();
}

void GameView::drawFrame() {
	Shared::Gfx::VisualSurface s = getSurface();
	Ultima1Game *game = static_cast<Ultima1Game *>(getRoot());

	// Big border around the screen
	s.fillRect(Common::Rect(0, 0, 317, 7), game->_borderColor);
	s.fillRect(Common::Rect(0, 6, 7, 200), game->_borderColor);
	s.fillRect(Common::Rect(313, 7, 320, 200), game->_borderColor);
	s.fillRect(Common::Rect(0, 193, 320, 200), game->_borderColor);

	// Thin line on edge of big border
	s.vLine(7, 7, 192, game->_edgeColor);
	s.vLine(312, 7, 192, game->_edgeColor);
	s.hLine(7, 7, 312, game->_edgeColor);
	s.hLine(7, 192, 312, game->_edgeColor);

	// Round the edges of the big outer border
	for (int idx = 1; idx <= 4; ++idx) {
		s.drawLine(idx, 0, 0, idx, 0);
		s.drawLine(319 - idx, 0, 319, idx, 0);
		s.drawLine(0, 199 - idx, idx, 199, 0);
		s.drawLine(319, 199 - idx, 319 - idx, 199, 0);
	}

	s.drawPoint(Common::Point(0, 0), 0);
	s.drawPoint(Common::Point(0, 5), 0);
	s.drawPoint(Common::Point(5, 0), 0);
	s.drawPoint(Common::Point(319, 0), 0);
	s.drawPoint(Common::Point(314, 0), 0);
	s.drawPoint(Common::Point(319, 5), 0);
	s.drawPoint(Common::Point(0, 199), 0);
	s.drawPoint(Common::Point(0, 194), 0);
	s.drawPoint(Common::Point(5, 199), 0);
	s.drawPoint(Common::Point(319, 199), 0);
	s.drawPoint(Common::Point(319, 194), 0);
	s.drawPoint(Common::Point(314, 199), 0);
}

} // End of namespace Shared
} // End of namespace Ultima
