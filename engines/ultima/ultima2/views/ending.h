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

#ifndef ULTIMA2_VIEWS_ENDING_H
#define ULTIMA2_VIEWS_ENDING_H

#include "ultima/shared/gfx/view.h"
#include "ultima/ultima2/data/map.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

using namespace Shared::Messages;

/**
 * The game's final, non-interactive screen: the victory text over MAPX30,
 * a fixed decorative map reused for this purpose, endlessly panning on a
 * diagonal with the player shown as a rocket. There's no way back from
 * here short of quitting - matches the original, which loops forever
 */
class Ending : public Shared::Gfx::View {
private:
	Data::TileId _tiles[Data::MAP_HEIGHT][Data::MAP_WIDTH] = {};
	Graphics::Surface _tileGfx[Data::TILE_COUNT];
	byte _panX = 0, _panY = 0;
	uint32 _lastStep = 0;

public:
	Ending();
	~Ending() override;

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
