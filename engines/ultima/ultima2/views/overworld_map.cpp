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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "ultima/ultima2/data/data.h"
#include "ultima/ultima2/gfx/pic_decoder.h"
#include "ultima/ultima2/views/overworld_map.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

constexpr int TILE_WIDTH = 16;
constexpr int TILE_HEIGHT = 16;
constexpr int VIEWPORT_WIDTH = 20;
constexpr int VIEWPORT_HEIGHT = 10;
constexpr int PLAYER_VIEWPORT_X = 9;
constexpr int PLAYER_VIEWPORT_Y = 5;

OverworldMap::OverworldMap() : Map("OverworldMap") {
	Data::loadTiles(_tiles);
	Data::loadAttackSprite(_attackSprite);
}

OverworldMap::~OverworldMap() {
	for (int i = 0; i < Data::TILE_COUNT; ++i)
		_tiles[i].free();
	_attackSprite.free();
}

bool OverworldMap::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_GAMEPLAY);
	g_system->getPaletteManager()->setPalette(Graphics::Palette(Data::CGA_PALETTE1, 4));
	return Map::msgFocus(msg);
}

bool OverworldMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Map::msgUnfocus(msg);
}

bool OverworldMap::msgAttackTile(const AttackTileMessage &msg) {
	Data::Savegame &sg = _G(savegame);
	int mapLeft = sg._mapX - PLAYER_VIEWPORT_X;
	int mapTop = sg._mapY - PLAYER_VIEWPORT_Y;
	int ox = msg._x - mapLeft;
	int oy = msg._y - mapTop;

	if (ox < 0 || ox >= VIEWPORT_WIDTH || oy < 0 || oy >= VIEWPORT_HEIGHT)
		return true;

	Common::Point pt(ox * TILE_WIDTH, oy * TILE_HEIGHT);
	auto s = getSurface();

	// XOR-flash the hit indicator on, then off again - matches the
	// original's xorSpriteDrawCenter-twice-with-a-pause-between pattern
	s.xorBlitFrom(_attackSprite, pt);
	g_engine->updateScreen();
	g_engine->pauseMillis(80);

	s.xorBlitFrom(_attackSprite, pt);
	g_engine->updateScreen();

	return true;
}

void OverworldMap::draw() {
	auto s = getSurface();
	s.clear();

	Data::Savegame &sg = _G(savegame);
	Data::MapMonsters &monsters = _G(map)._monsters;
	int mapLeft = sg._mapX - PLAYER_VIEWPORT_X;
	int mapTop = sg._mapY - PLAYER_VIEWPORT_Y;

	for (int oy = 0; oy < VIEWPORT_HEIGHT; ++oy) {
		for (int ox = 0; ox < VIEWPORT_WIDTH; ++ox) {
			int x = (mapLeft + ox + Data::MAP_WIDTH) % Data::MAP_WIDTH;
			int y = (mapTop + oy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;
			Data::TileId tileId = _G(map).tileAt(x, y);

			for (int slot = 0; slot < Data::MAP_MONSTER_COUNT; ++slot) {
				if (monsters.isActive(slot) && monsters._mapX[slot] == x && monsters._mapY[slot] == y) {
					tileId = monsters.tileType(slot);
					break;
				}
			}

			if (ox == PLAYER_VIEWPORT_X && oy == PLAYER_VIEWPORT_Y)
				tileId = sg.playerTile();

			s.blitFrom(_tiles[tileId], Common::Point(ox * TILE_WIDTH, oy * TILE_HEIGHT));
		}
	}
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
