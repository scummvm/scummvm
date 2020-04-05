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

#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/map/tileanim.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/map/tileview.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/core/error.h"

namespace Ultima {
namespace Ultima4 {

TileView::TileView(int x, int y, int columns, int rows) : View(x, y, columns * TILE_WIDTH, rows * TILE_HEIGHT) {
	this->_columns = columns;
	this->_rows = rows;
	this->_tileWidth = TILE_WIDTH;
	this->_tileHeight = TILE_HEIGHT;
	this->_tileset = Tileset::get("base");
	_animated = Image::create(SCALED(_tileWidth), SCALED(_tileHeight), false, Image::HARDWARE);
}

TileView::TileView(int x, int y, int columns, int rows, const Common::String &tileset) :
	View(x, y, columns * TILE_WIDTH, rows * TILE_HEIGHT) {
	this->_columns = columns;
	this->_rows = rows;
	this->_tileWidth = TILE_WIDTH;
	this->_tileHeight = TILE_HEIGHT;
	this->_tileset = Tileset::get(tileset);
	_animated = Image::create(SCALED(_tileWidth), SCALED(_tileHeight), false, Image::HARDWARE);
}

TileView::~TileView() {
	delete _animated;
}

void TileView::reinit() {
	View::reinit();
	_tileset = Tileset::get("base");

	//Scratchpad needs to be re-inited if we rescale...
	if (_animated) {
		delete _animated;
		_animated = NULL;
	}
	_animated = Image::create(SCALED(_tileWidth), SCALED(_tileHeight), false, Image::HARDWARE);
}

void TileView::loadTile(MapTile &mapTile) {
	//This attempts to preload tiles in advance
	Tile *tile = _tileset->get(mapTile._id);
	if (tile) {
		tile->getImage();
	}
	//But may fail if the tiles don't exist directly in the expected imagesets
}

void TileView::drawTile(MapTile &mapTile, bool focus, int x, int y) {
	Tile *tile = _tileset->get(mapTile._id);
	Image *image = tile->getImage();

	ASSERT(x < _columns, "x value of %d out of range", x);
	ASSERT(y < _rows, "y value of %d out of range", y);

	//Blank scratch pad
	_animated->fillRect(0, 0, SCALED(_tileWidth), SCALED(_tileHeight), 0, 0, 0, 255);
	//Draw blackness on the tile.
	_animated->drawSubRect(SCALED(x * _tileWidth + this->_x),
	                       SCALED(y * _tileHeight + this->_y),
	                       0,
	                       0,
	                       SCALED(_tileWidth),
	                       SCALED(_tileHeight));

	// draw the tile to the screen
	if (tile->getAnim()) {
		// First, create our animated version of the tile
#ifdef IOS
		animated->clearImageContents();
#endif
		tile->getAnim()->draw(_animated, tile, mapTile, DIR_NONE);

		// Then draw it to the screen
		_animated->drawSubRect(SCALED(x * _tileWidth + this->_x),
		                       SCALED(y * _tileHeight + this->_y),
		                       0,
		                       0,
		                       SCALED(_tileWidth),
		                       SCALED(_tileHeight));
	} else {
		image->drawSubRect(SCALED(x * _tileWidth + this->_x),
		                   SCALED(y * _tileHeight + this->_y),
		                   0,
		                   SCALED(_tileHeight * mapTile._frame),
		                   SCALED(_tileWidth),
		                   SCALED(_tileHeight));
	}

	// draw the focus around the tile if it has the focus
	if (focus)
		drawFocus(x, y);
}

void TileView::drawTile(Std::vector<MapTile> &tiles, bool focus, int x, int y) {
	ASSERT(x < _columns, "x value of %d out of range", x);
	ASSERT(y < _rows, "y value of %d out of range", y);

	_animated->fillRect(0, 0, SCALED(_tileWidth), SCALED(_tileHeight), 0, 0, 0, 255);
	_animated->drawSubRect(SCALED(x * _tileWidth + this->_x),
	                       SCALED(y * _tileHeight + this->_y),
	                       0,
	                       0,
	                       SCALED(_tileWidth),
	                       SCALED(_tileHeight));

	//int layer = 0;

	for (Std::vector<MapTile>::reverse_iterator t = tiles.rbegin(); t != tiles.rend(); ++t) {
		MapTile &frontTile = *t;
		Tile *frontTileType = _tileset->get(frontTile._id);

		if (!frontTileType) {
			//TODO, this leads to an error. It happens after graphics mode changes.
			return;
		}

		Image *image = frontTileType->getImage();


		// draw the tile to the screen
		if (frontTileType->getAnim()) {
			// First, create our animated version of the tile
			frontTileType->getAnim()->draw(_animated, frontTileType, frontTile, DIR_NONE);
		} else {
			if (!image)
				return; //This is a problem //FIXME, error message it.
			image->drawSubRectOn(_animated,
			                     0, 0,
			                     0, SCALED(_tileHeight * frontTile._frame),
			                     SCALED(_tileWidth),  SCALED(_tileHeight));
		}

		// Then draw it to the screen
		_animated->drawSubRect(SCALED(x * _tileWidth + this->_x),
		                       SCALED(y * _tileHeight + this->_y),
		                       0,
		                       0,
		                       SCALED(_tileWidth),
		                       SCALED(_tileHeight));
	}


	// draw the focus around the tile if it has the focus
	if (focus)
		drawFocus(x, y);
}

void TileView::drawFocus(int x, int y) {
	ASSERT(x < _columns, "x value of %d out of range", x);
	ASSERT(y < _rows, "y value of %d out of range", y);

	/*
	 * draw the focus rectangle around the tile
	 */
	if ((screenCurrentCycle * 4 / SCR_CYCLE_PER_SECOND) % 2) {
		/* left edge */
		_screen->fillRect(SCALED(x * _tileWidth + this->_x),
		                  SCALED(y * _tileHeight + this->_y),
		                  SCALED(2),
		                  SCALED(_tileHeight),
		                  0xff, 0xff, 0xff);

		/* top edge */
		_screen->fillRect(SCALED(x * _tileWidth + this->_x),
		                  SCALED(y * _tileHeight + this->_y),
		                  SCALED(_tileWidth),
		                  SCALED(2),
		                  0xff, 0xff, 0xff);

		/* right edge */
		_screen->fillRect(SCALED((x + 1) * _tileWidth + this->_x - 2),
		                  SCALED(y * _tileHeight + this->_y),
		                  SCALED(2),
		                  SCALED(_tileHeight),
		                  0xff, 0xff, 0xff);

		/* bottom edge */
		_screen->fillRect(SCALED(x * _tileWidth + this->_x),
		                  SCALED((y + 1) * _tileHeight + this->_y - 2),
		                  SCALED(_tileWidth),
		                  SCALED(2),
		                  0xff, 0xff, 0xff);
	}
}

void TileView::setTileset(Tileset *tileset) {
	this->_tileset = tileset;
}

} // End of namespace Ultima4
} // End of namespace Ultima
