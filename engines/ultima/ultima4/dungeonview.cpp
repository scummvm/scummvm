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

#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/dungeonview.h"
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imagemgr.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/tileanim.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/error.h"
#include "ultima/shared/std/misc.h"

namespace Ultima {
namespace Ultima4 {

DungeonView::DungeonView(int x, int y, int columns, int rows) : TileView(x, y, rows, columns)
, screen3dDungeonViewEnabled(true)
{
}


DungeonView * DungeonView::_instance(NULL);
DungeonView * DungeonView::getInstance()
{
	if (!_instance) 	{
		_instance = new DungeonView(BORDER_WIDTH, BORDER_HEIGHT, VIEWPORT_W, VIEWPORT_H);
	}
	return _instance;
}

void DungeonView::display(Context * c, TileView *view)
{
	int x,y;

    /* 1st-person perspective */
    if (screen3dDungeonViewEnabled) {
        //Note: This shouldn't go above 4, unless we check opaque tiles each step of the way.
        const int farthest_non_wall_tile_visibility = 4;

    	Std::vector<MapTile> tiles;

        screenEraseMapArea();
        if (c->_party->getTorchDuration() > 0) {
            for (y = 3; y >= 0; y--) {
                DungeonGraphicType type;

                //FIXME: Maybe this should be in a loop
				tiles = getTiles(y, -1);
                type = tilesToGraphic(tiles);
				drawWall(-1, y, (Direction)c->_saveGame->_orientation, type);

				tiles = getTiles(y, 1);
                type = tilesToGraphic(tiles);
                drawWall(1, y, (Direction)c->_saveGame->_orientation, type);

                tiles = getTiles(y, 0);
                type = tilesToGraphic(tiles);
                drawWall(0, y, (Direction)c->_saveGame->_orientation, type);

                //This only checks that the tile at y==3 is opaque
                if (y == 3 && !tiles.front().getTileType()->isOpaque())
               	{
               		for (int y_obj = farthest_non_wall_tile_visibility; y_obj > y; y_obj--)
               		{
               		Std::vector<MapTile> distant_tiles = getTiles(y_obj     , 0);
               		DungeonGraphicType distant_type = tilesToGraphic(distant_tiles);

					if ((distant_type == DNGGRAPHIC_DNGTILE) || (distant_type == DNGGRAPHIC_BASETILE))
						drawTile(c->_location->_map->_tileset->get(distant_tiles.front().getId()),0, y_obj, Direction(c->_saveGame->_orientation));
               		}
               	}
				if ((type == DNGGRAPHIC_DNGTILE) || (type == DNGGRAPHIC_BASETILE))
					drawTile(c->_location->_map->_tileset->get(tiles.front().getId()), 0, y, Direction(c->_saveGame->_orientation));
            }
        }
    }

    /* 3rd-person perspective */
    else {
    	Std::vector<MapTile> tiles;

        static MapTile black = c->_location->_map->_tileset->getByName("black")->getId();
        static MapTile avatar = c->_location->_map->_tileset->getByName("avatar")->getId();

        for (y = 0; y < VIEWPORT_H; y++) {
            for (x = 0; x < VIEWPORT_W; x++) {
                    tiles = getTiles((VIEWPORT_H / 2) - y, x - (VIEWPORT_W / 2));

				/* Only show blackness if there is no light */
				if (c->_party->getTorchDuration() <= 0)
					view->drawTile(black, false, x, y);
				else if (x == VIEWPORT_W/2 && y == VIEWPORT_H/2)
					view->drawTile(avatar, false, x, y);
				else
					view->drawTile(tiles, false, x, y);
            }
        }
    }
}

void DungeonView::drawInDungeon(Tile *tile, int x_offset, int distance, Direction orientation, bool tiledWall) {
	Image *scaled;

  	const static int nscale_vga[] = { 12, 8, 4, 2, 1};
    const static int nscale_ega[] = { 8, 4, 2, 1, 0};

	const int lscale_vga[] = { 22, 18, 10, 4, 1};
	const int lscale_ega[] = { 22, 14, 6, 3, 1};

    const int * lscale;
    const int * nscale;
    int offset_multiplier = 0;
    int offset_adj = 0;
    if (settings._videoType != "EGA")
    {
    	lscale = & lscale_vga[0];
    	nscale = & nscale_vga[0];
    	offset_multiplier = 1;
    	offset_adj = 2;
    }
    else
    {
    	lscale = & lscale_ega[0];
    	nscale = & nscale_ega[0];
    	offset_adj = 1;
    	offset_multiplier = 4;
    }

    const int *dscale = tiledWall ? lscale : nscale;

    //Clear scratchpad and set a background color
    _animated->initializeToBackgroundColor();
    //Put tile on animated scratchpad
    if (tile->getAnim()) {
        MapTile mt = tile->getId();
        tile->getAnim()->draw(_animated, tile, mt, orientation);
    }
    else
    {
        tile->getImage()->drawOn(_animated, 0, 0);
    }
    _animated->makeBackgroundColorTransparent();
    //This process involving the background color is only required for drawing in the dungeon.
    //It will not play well with semi-transparent graphics.

    /* scale is based on distance; 1 means half size, 2 regular, 4 means scale by 2x, etc. */
    if (dscale[distance] == 0)
		return;
    else if (dscale[distance] == 1)
        scaled = screenScaleDown(_animated, 2);
    else
    {
        scaled = screenScale(_animated, dscale[distance] / 2, 1, 0);
    }

    if (tiledWall) {
    	int i_x = SCALED((VIEWPORT_W * _tileWidth  / 2) + this->_x) - (scaled->width() / 2);
    	int i_y = SCALED((VIEWPORT_H * _tileHeight / 2) + this->_y) - (scaled->height() / 2);
    	int f_x = i_x + scaled->width();
    	int f_y = i_y + scaled->height();
    	int d_x = _animated->width();
    	int d_y = _animated->height();

    	for (int x = i_x; x < f_x; x+=d_x)
    		for (int y = i_y; y < f_y; y+=d_y)
    			_animated->drawSubRectOn(this->_screen,
    					x,
    					y,
    					0,
    					0,
    					f_x - x,
    					f_y - y
    			);
    }
    else {
    	int y_offset = MAX(0,(dscale[distance] - offset_adj) * offset_multiplier);
    	int x = SCALED((VIEWPORT_W * _tileWidth / 2) + this->_x) - (scaled->width() / 2);
    	int y = SCALED((VIEWPORT_H * _tileHeight / 2) + this->_y + y_offset) - (scaled->height() / 8);

		scaled->drawSubRectOn(	this->_screen,
								x,
								y,
								0,
								0,
								SCALED(_tileWidth * VIEWPORT_W + this->_x) - x ,
								SCALED(_tileHeight * VIEWPORT_H + this->_y) - y );
    }

    delete scaled;
}

int DungeonView::graphicIndex(int xoffset, int distance, Direction orientation, DungeonGraphicType type) {
    int index;

    index = 0;

    if (type == DNGGRAPHIC_LADDERUP && xoffset == 0)
        return 48 +
        (distance * 2) +
        (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH) ? 1 : 0);

    if (type == DNGGRAPHIC_LADDERDOWN && xoffset == 0)
        return 56 +
        (distance * 2) +
        (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH) ? 1 : 0);

    if (type == DNGGRAPHIC_LADDERUPDOWN && xoffset == 0)
        return 64 +
        (distance * 2) +
        (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH) ? 1 : 0);

    /* FIXME */
    if (type != DNGGRAPHIC_WALL && type != DNGGRAPHIC_DOOR)
        return -1;

    if (type == DNGGRAPHIC_DOOR)
        index += 24;

    index += (xoffset + 1) * 2;

    index += distance * 6;

    if (DIR_IN_MASK(orientation, MASK_DIR_SOUTH | MASK_DIR_NORTH))
        index++;

    return index;
}

void DungeonView::drawTile(Tile *tile, int x_offset, int distance, Direction orientation) {
    // Draw the tile to the screen
	DungeonViewer.drawInDungeon(tile, x_offset, distance, orientation, tile->isTiledInDungeon());
}

Std::vector<MapTile> DungeonView::getTiles(int fwd, int side) {
    MapCoords coords = g_context->_location->_coords;

    switch (g_context->_saveGame->_orientation) {
    case DIR_WEST:
        coords.x -= fwd;
        coords.y -= side;
        break;

    case DIR_NORTH:
        coords.x += side;
        coords.y -= fwd;
        break;

    case DIR_EAST:
        coords.x += fwd;
        coords.y += side;
        break;

    case DIR_SOUTH:
        coords.x -= side;
        coords.y += fwd;
        break;

    case DIR_ADVANCE:
    case DIR_RETREAT:
    default:
        ASSERT(0, "Invalid dungeon orientation");
    }

    // Wrap the coordinates if necessary
    coords.wrap(g_context->_location->_map);

    bool focus;
    return g_context->_location->tilesAt(coords, focus);
}

DungeonGraphicType DungeonView::tilesToGraphic(const Std::vector<MapTile> &tiles) {
    MapTile tile = tiles.front();

    static const MapTile corridor = g_context->_location->_map->_tileset->getByName("brick_floor")->getId();
    static const MapTile up_ladder = g_context->_location->_map->_tileset->getByName("up_ladder")->getId();
    static const MapTile down_ladder = g_context->_location->_map->_tileset->getByName("down_ladder")->getId();
    static const MapTile updown_ladder = g_context->_location->_map->_tileset->getByName("up_down_ladder")->getId();

    /*
     * check if the dungeon tile has an annotation or object on top
     * (always displayed as a tile, unless a ladder)
     */
    if (tiles.size() > 1) {
        if (tile._id == up_ladder._id)
            return DNGGRAPHIC_LADDERUP;
        else if (tile._id == down_ladder._id)
            return DNGGRAPHIC_LADDERDOWN;
        else if (tile._id == updown_ladder._id)
            return DNGGRAPHIC_LADDERUPDOWN;
        else if (tile._id == corridor._id)
            return DNGGRAPHIC_NONE;
        else
            return DNGGRAPHIC_BASETILE;
    }

    /*
     * if not an annotation or object, then the tile is a dungeon
     * token
     */
    Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
    DungeonToken token = dungeon->tokenForTile(tile);

    switch (token) {
    case DUNGEON_TRAP:
    case DUNGEON_CORRIDOR:
        return DNGGRAPHIC_NONE;
    case DUNGEON_WALL:
    case DUNGEON_SECRET_DOOR:
        return DNGGRAPHIC_WALL;
    case DUNGEON_ROOM:
    case DUNGEON_DOOR:
        return DNGGRAPHIC_DOOR;
    case DUNGEON_LADDER_UP:
        return DNGGRAPHIC_LADDERUP;
    case DUNGEON_LADDER_DOWN:
        return DNGGRAPHIC_LADDERDOWN;
    case DUNGEON_LADDER_UPDOWN:
        return DNGGRAPHIC_LADDERUPDOWN;

    default:
        return DNGGRAPHIC_DNGTILE;
    }
}

const struct {
    const char *subimage;
    int ega_x2, ega_y2;
    int vga_x2, vga_y2;
    const char *subimage2;
} dngGraphicInfo[] = {
    { "dung0_lft_ew" },
    { "dung0_lft_ns" },
    { "dung0_mid_ew" },
    { "dung0_mid_ns" },
    { "dung0_rgt_ew" },
    { "dung0_rgt_ns" },

    { "dung1_lft_ew", 0, 32, 0, 8, "dung1_xxx_ew" },
    { "dung1_lft_ns", 0, 32, 0, 8, "dung1_xxx_ns" },
    { "dung1_mid_ew" },
    { "dung1_mid_ns" },
    { "dung1_rgt_ew", 144, 32, 160, 8, "dung1_xxx_ew" },
    { "dung1_rgt_ns", 144, 32, 160, 8, "dung1_xxx_ns" },

    { "dung2_lft_ew", 0, 64, 0, 48, "dung2_xxx_ew" },
    { "dung2_lft_ns", 0, 64, 0, 48, "dung2_xxx_ns" },
    { "dung2_mid_ew" },
    { "dung2_mid_ns" },
    { "dung2_rgt_ew", 112, 64, 128, 48, "dung2_xxx_ew" },
    { "dung2_rgt_ns", 112, 64, 128, 48, "dung2_xxx_ns" },

    { "dung3_lft_ew", 0, 80, 48, 72, "dung3_xxx_ew" },
    { "dung3_lft_ns", 0, 80, 48, 72, "dung3_xxx_ns" },
    { "dung3_mid_ew" },
    { "dung3_mid_ns" },
    { "dung3_rgt_ew", 96, 80, 104, 72, "dung3_xxx_ew" },
    { "dung3_rgt_ns", 96, 80, 104, 72, "dung3_xxx_ns" },

    { "dung0_lft_ew_door" },
    { "dung0_lft_ns_door" },
    { "dung0_mid_ew_door" },
    { "dung0_mid_ns_door" },
    { "dung0_rgt_ew_door" },
    { "dung0_rgt_ns_door" },

    { "dung1_lft_ew_door", 0, 32, 0, 8, "dung1_xxx_ew" },
    { "dung1_lft_ns_door", 0, 32, 0, 8, "dung1_xxx_ns" },
    { "dung1_mid_ew_door" },
    { "dung1_mid_ns_door" },
    { "dung1_rgt_ew_door", 144, 32, 160, 8, "dung1_xxx_ew" },
    { "dung1_rgt_ns_door", 144, 32, 160, 8, "dung1_xxx_ns" },

    { "dung2_lft_ew_door", 0, 64, 0, 48, "dung2_xxx_ew" },
    { "dung2_lft_ns_door", 0, 64, 0, 48, "dung2_xxx_ns" },
    { "dung2_mid_ew_door" },
    { "dung2_mid_ns_door" },
    { "dung2_rgt_ew_door", 112, 64, 128, 48, "dung2_xxx_ew" },
    { "dung2_rgt_ns_door", 112, 64, 128, 48, "dung2_xxx_ns" },

    { "dung3_lft_ew_door", 0, 80, 48, 72, "dung3_xxx_ew" },
    { "dung3_lft_ns_door", 0, 80, 48, 72, "dung3_xxx_ns" },
    { "dung3_mid_ew_door" },
    { "dung3_mid_ns_door" },
    { "dung3_rgt_ew_door", 96, 80, 104, 72, "dung3_xxx_ew" },
    { "dung3_rgt_ns_door", 96, 80, 104, 72, "dung3_xxx_ns" },

    { "dung0_ladderup" },
    { "dung0_ladderup_side" },
    { "dung1_ladderup" },
    { "dung1_ladderup_side" },
    { "dung2_ladderup" },
    { "dung2_ladderup_side" },
    { "dung3_ladderup" },
    { "dung3_ladderup_side" },

    { "dung0_ladderdown" },
    { "dung0_ladderdown_side" },
    { "dung1_ladderdown" },
    { "dung1_ladderdown_side" },
    { "dung2_ladderdown" },
    { "dung2_ladderdown_side" },
    { "dung3_ladderdown" },
    { "dung3_ladderdown_side" },

    { "dung0_ladderupdown" },
    { "dung0_ladderupdown_side" },
    { "dung1_ladderupdown" },
    { "dung1_ladderupdown_side" },
    { "dung2_ladderupdown" },
    { "dung2_ladderupdown_side" },
    { "dung3_ladderupdown" },
    { "dung3_ladderupdown_side" },
};

void DungeonView::drawWall(int xoffset, int distance, Direction orientation, DungeonGraphicType type) {
    int index;

    index = graphicIndex(xoffset, distance, orientation, type);
    if (index == -1 || distance >= 4)
        return;

    int x = 0, y = 0;
    SubImage *subimage = imageMgr->getSubImage(dngGraphicInfo[index].subimage);
    if (subimage) {
        x = subimage->x;
        y = subimage->y;
    }

    screenDrawImage(dngGraphicInfo[index].subimage, (BORDER_WIDTH + x) * settings._scale,
                    (BORDER_HEIGHT + y) * settings._scale);

    if (dngGraphicInfo[index].subimage2 != NULL) {
        // FIXME: subimage2 is a horrible hack, needs to be cleaned up
        if (settings._videoType == "EGA")
            screenDrawImage(dngGraphicInfo[index].subimage2,
                            (8 + dngGraphicInfo[index].ega_x2) * settings._scale,
                            (8 + dngGraphicInfo[index].ega_y2) * settings._scale);
        else
            screenDrawImage(dngGraphicInfo[index].subimage2,
                            (8 + dngGraphicInfo[index].vga_x2) * settings._scale,
                            (8 + dngGraphicInfo[index].vga_y2) * settings._scale);
    }
}

} // End of namespace Ultima4
} // End of namespace Ultima
