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

#ifndef ULTIMA4_DUNGEONVIEW_H
#define ULTIMA4_DUNGEONVIEW_H

#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/dungeon.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/graphics/tileset.h"
#include "ultima/ultima4/direction.h"
#include "ultima/ultima4/graphics/tileview.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/location.h"

namespace Ultima {
namespace Ultima4 {

typedef enum {
    DNGGRAPHIC_NONE,
    DNGGRAPHIC_WALL,
    DNGGRAPHIC_LADDERUP,
    DNGGRAPHIC_LADDERDOWN,
    DNGGRAPHIC_LADDERUPDOWN,
    DNGGRAPHIC_DOOR,
    DNGGRAPHIC_DNGTILE,
    DNGGRAPHIC_BASETILE
} DungeonGraphicType;

Std::vector<MapTile> dungeonViewGetTiles(int fwd, int side);
DungeonGraphicType dungeonViewTilesToGraphic(const Std::vector<MapTile> &tiles);

#define DungeonViewer (*DungeonView::getInstance())

/**
 * @todo
 * <ul>
 *      <li>move the rest of the dungeon drawing logic here from screen_sdl</li>
 * </ul>
 */
class DungeonView : public TileView {
private:
    DungeonView(int x, int y, int columns, int rows);
    bool screen3dDungeonViewEnabled;
public:
    static DungeonView *_instance;
    static DungeonView *getInstance();

    void drawInDungeon(Tile *tile, int x_offset, int distance, Direction orientation, bool tiled);
    int graphicIndex(int xoffset, int distance, Direction orientation, DungeonGraphicType type);
    void drawTile(Tile *tile, int x_offset, int distance, Direction orientation);
    void drawWall(int xoffset, int distance, Direction orientation, DungeonGraphicType type);

    void display(Context * c, TileView *view);
    DungeonGraphicType tilesToGraphic(const Std::vector<MapTile> &tiles);

    bool toggle3DDungeonView(){return screen3dDungeonViewEnabled=!screen3dDungeonViewEnabled;}

    Std::vector<MapTile> getTiles(int fwd, int side);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
