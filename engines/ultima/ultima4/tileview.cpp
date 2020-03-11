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
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imagemgr.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/tile.h"
#include "ultima/ultima4/tileanim.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/tileview.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/error.h"

namespace Ultima {
namespace Ultima4 {

TileView::TileView(int x, int y, int columns, int rows) : View(x, y, columns * TILE_WIDTH, rows * TILE_HEIGHT) {
    this->columns = columns;
    this->rows = rows;
    this->tileWidth = TILE_WIDTH;
    this->tileHeight = TILE_HEIGHT;
    this->tileset = Tileset::get("base");
    animated = Image::create(SCALED(tileWidth), SCALED(tileHeight), false, Image::HARDWARE);
}

TileView::TileView(int x, int y, int columns, int rows, const Common::String &tileset) :
		View(x, y, columns * TILE_WIDTH, rows * TILE_HEIGHT) {
    this->columns = columns;
    this->rows = rows;
    this->tileWidth = TILE_WIDTH;
    this->tileHeight = TILE_HEIGHT;
    this->tileset = Tileset::get(tileset);
    animated = Image::create(SCALED(tileWidth), SCALED(tileHeight), false, Image::HARDWARE);
}

TileView::~TileView() {
    delete animated;
}

void TileView::reinit() {
    View::reinit();
    tileset = Tileset::get("base");

    //Scratchpad needs to be re-inited if we rescale...
    if (animated)
    {
    	delete animated;
    	animated = NULL;
    }
    animated = Image::create(SCALED(tileWidth), SCALED(tileHeight), false, Image::HARDWARE);
}

void TileView::loadTile(MapTile &mapTile)
{
	//This attempts to preload tiles in advance
    Tile *tile = tileset->get(mapTile.id);
    if (tile)
    {
    	tile->getImage();
    }
    //But may fail if the tiles don't exist directly in the expected imagesets
}

void TileView::drawTile(MapTile &mapTile, bool focus, int x, int y) {
    Tile *tile = tileset->get(mapTile.id);
    Image *image = tile->getImage();

    ASSERT(x < columns, "x value of %d out of range", x);
    ASSERT(y < rows, "y value of %d out of range", y);

    //Blank scratch pad
	animated->fillRect(0,0,SCALED(tileWidth),SCALED(tileHeight),0,0,0, 255);
	//Draw blackness on the tile.
	animated->drawSubRect(SCALED(x * tileWidth + this->x),
						  SCALED(y * tileHeight + this->y),
						  0,
						  0,
						  SCALED(tileWidth),
						  SCALED(tileHeight));

    // draw the tile to the screen
    if (tile->getAnim()) {
        // First, create our animated version of the tile
#ifdef IOS
        animated->clearImageContents();
#endif
        tile->getAnim()->draw(animated, tile, mapTile, DIR_NONE);

        // Then draw it to the screen
        animated->drawSubRect(SCALED(x * tileWidth + this->x),
                              SCALED(y * tileHeight + this->y),
                              0, 
                              0, 
                              SCALED(tileWidth), 
                              SCALED(tileHeight));
    }
    else {
        image->drawSubRect(SCALED(x * tileWidth + this->x), 
                           SCALED(y * tileHeight + this->y),
                           0,
                           SCALED(tileHeight * mapTile.frame),
                           SCALED(tileWidth),
                           SCALED(tileHeight));
    }

    // draw the focus around the tile if it has the focus
    if (focus)
        drawFocus(x, y);
}

void TileView::drawTile(Std::vector<MapTile> &tiles, bool focus, int x, int y) {
	ASSERT(x < columns, "x value of %d out of range", x);
	ASSERT(y < rows, "y value of %d out of range", y);

	animated->fillRect(0,0,SCALED(tileWidth),SCALED(tileHeight),0,0,0, 255);
	animated->drawSubRect(SCALED(x * tileWidth + this->x),
						  SCALED(y * tileHeight + this->y),
						  0,
						  0,
						  SCALED(tileWidth),
						  SCALED(tileHeight));

	//int layer = 0;

	for (Std::vector<MapTile>::reverse_iterator t = tiles.rbegin(); t != tiles.rend(); ++t) {
		MapTile &frontTile = *t;
		Tile *frontTileType = tileset->get(frontTile.id);

		if (!frontTileType)
		{
			//TODO, this leads to an error. It happens after graphics mode changes.
			return;
		}

		Image *image = frontTileType->getImage();


		// draw the tile to the screen
		if (frontTileType->getAnim()) {
			// First, create our animated version of the tile
			frontTileType->getAnim()->draw(animated, frontTileType, frontTile, DIR_NONE);
		}
		else {
            if (!image)
                return; //This is a problem //FIXME, error message it. 
			image->drawSubRectOn(animated,
								0, 0,
								0, SCALED(tileHeight * frontTile.frame),
								SCALED(tileWidth),  SCALED(tileHeight));
		}

		// Then draw it to the screen
		animated->drawSubRect(SCALED(x * tileWidth + this->x),
							  SCALED(y * tileHeight + this->y),
							  0,
							  0,
							  SCALED(tileWidth),
							  SCALED(tileHeight));
	}


	// draw the focus around the tile if it has the focus
	if (focus)
        drawFocus(x, y);
}

/**
 * Draw a focus rectangle around the tile
 */
void TileView::drawFocus(int x, int y) {
    ASSERT(x < columns, "x value of %d out of range", x);
    ASSERT(y < rows, "y value of %d out of range", y);

    /*
     * draw the focus rectangle around the tile
     */
    if ((screenCurrentCycle * 4 / SCR_CYCLE_PER_SECOND) % 2) {
        /* left edge */
        screen->fillRect(SCALED(x * tileWidth + this->x),
                         SCALED(y * tileHeight + this->y),
                         SCALED(2),
                         SCALED(tileHeight),
                         0xff, 0xff, 0xff);

        /* top edge */
        screen->fillRect(SCALED(x * tileWidth + this->x),
                         SCALED(y * tileHeight + this->y),
                         SCALED(tileWidth),
                         SCALED(2),
                         0xff, 0xff, 0xff);

        /* right edge */
        screen->fillRect(SCALED((x + 1) * tileWidth + this->x - 2),
                         SCALED(y * tileHeight + this->y),
                         SCALED(2),
                         SCALED(tileHeight),
                         0xff, 0xff, 0xff);

        /* bottom edge */
        screen->fillRect(SCALED(x * tileWidth + this->x),
                         SCALED((y + 1) * tileHeight + this->y - 2),
                         SCALED(tileWidth),
                         SCALED(2),
                         0xff, 0xff, 0xff);
    }
}

void TileView::setTileset(Tileset *tileset) {
    this->tileset = tileset;
}

} // End of namespace Ultima4
} // End of namespace Ultima
