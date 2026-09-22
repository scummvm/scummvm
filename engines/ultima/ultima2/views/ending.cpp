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

#include "common/file.h"
#include "common/system.h"
#include "ultima/ultima2/views/ending.h"
#include "ultima/ultima2/data/map.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

namespace {

constexpr int TILE_WIDTH = 16;
constexpr int TILE_HEIGHT = 16;
constexpr int VIEWPORT_WIDTH = 20;
constexpr int VIEWPORT_HEIGHT = 10;
constexpr int PLAYER_VIEWPORT_X = 9;
constexpr int PLAYER_VIEWPORT_Y = 5;

// The original's pace here is CPU-dependent; picked to look right
constexpr uint32 STEP_DELAY = 90;

} // namespace

Ending::Ending() : View("Ending") {
	setBounds(TextRect(0, 0, 39, 24));
}

Ending::~Ending() {
	for (int i = 0; i < Data::TILE_COUNT; ++i)
		_tileGfx[i].free();
}

bool Ending::msgFocus(const FocusMessage &msg) {
	Data::loadTiles(_tileGfx);

	// A fixed background, reused from the same file that's the real
	// planet map of this era/type combination - not loaded as a live map
	Common::File f;
	if (f.open(Data::mapFilename(3, 0).c_str())) {
		for (int y = 0; y < Data::MAP_HEIGHT; ++y) {
			for (int x = 0; x < Data::MAP_WIDTH; ++x)
				_tiles[y][x] = (Data::TileId)(f.readByte() / 4);
		}
	}

	_panX = _panY = 0;
	_lastStep = g_system->getMillis();

	g_engine->baseView()->findView("Commands")->send(Shared::Messages::GameMessage("TEXT",
		"\n\nYOU HAVE SAVED THE UNIVERSE,\nAND COMPLETED ULTIMA ][! SEEK\n"
		"NOW TO CONQUER WICKED EXODUS,\nFOUND IN ULTIMA ]I[-D ]II[-P!\n"));

	return View::msgFocus(msg);
}

void Ending::draw() {
	auto s = getSurface();
	s.clear();

	int mapLeft = _panX - PLAYER_VIEWPORT_X;
	int mapTop = _panY - PLAYER_VIEWPORT_Y;

	for (int oy = 0; oy < VIEWPORT_HEIGHT; ++oy) {
		for (int ox = 0; ox < VIEWPORT_WIDTH; ++ox) {
			int x = (mapLeft + ox + Data::MAP_WIDTH) % Data::MAP_WIDTH;
			int y = (mapTop + oy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;
			Data::TileId tileId = _tiles[y][x];

			if (ox == PLAYER_VIEWPORT_X && oy == PLAYER_VIEWPORT_Y)
				tileId = Data::TILE_ROCKET;

			s.blitFrom(_tileGfx[tileId], Common::Point(ox * TILE_WIDTH, oy * TILE_HEIGHT));
		}
	}
}

bool Ending::tick() {
	uint32 now = g_system->getMillis();
	if (now - _lastStep >= STEP_DELAY) {
		_lastStep = now;

		_panY = (_panY + 1) & 0x3F;
		if ((_panY & 3) == 0)
			_panX = (_panX + 1) & 0x3F;

		redraw();
	}

	return View::tick();
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
