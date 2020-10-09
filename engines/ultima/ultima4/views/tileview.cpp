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
#include "ultima/ultima4/views/tileview.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

TileView::TileView(int x, int y, int columns, int rows) :
		View(x, y, columns * TILE_WIDTH, rows * TILE_HEIGHT) {
	_columns = columns;
	_rows = rows;
	_tileWidth = TILE_WIDTH;
	_tileHeight = TILE_HEIGHT;
	_tileSet = g_tileSets->get("base");
	_animated = Image::create(SCALED(_tileWidth), SCALED(_tileHeight), false, Image::HARDWARE);
	_dest = nullptr;
}

TileView::TileView(int x, int y, int columns, int rows, const Common::String &tileset) :
		View(x, y, columns * TILE_WIDTH, rows * TILE_HEIGHT) {
	_columns = columns;
	_rows = rows;
	_tileWidth = TILE_WIDTH;
	_tileHeight = TILE_HEIGHT;
	_tileSet = g_tileSets->get(tileset);
	_animated = Image::create(SCALED(_tileWidth), SCALED(_tileHeight), false, Image::HARDWARE);
	_dest = nullptr;
}

TileView::~TileView() {
	delete _animated;
}

void TileView::reinit() {
	View::reinit();
	_tileSet = g_tileSets->get("base");

	// Scratchpad needs to be re-inited if we rescale...
	if (_animated) {
		delete _animated;
		_animated = nullptr;
	}
	_animated = Image::create(SCALED(_tileWidth), SCALED(_tileHeight), false, Image::HARDWARE);
}

void TileView::loadTile(MapTile &mapTile) {
	// This attempts to preload tiles in advance
	Tile *tile = _tileSet->get(mapTile._id);
	if (tile) {
		tile->getImage();
	}
	// But may fail if the tiles don't exist directly in the expected imagesets
}

void TileView::drawTile(MapTile &mapTile, bool focus, int x, int y) {
	Tile *tile = _tileSet->get(mapTile._id);
	Image *image = tile->getImage();

	assertMsg(x < _columns, "x value of %d out of range", x);
	assertMsg(y < _rows, "y value of %d out of range", y);

	// Blank scratch pad
	_animated->fillRect(0, 0, SCALED(_tileWidth), SCALED(_tileHeight), 0, 0, 0, 255);

	// Draw blackness on the tile.
	_animated->drawSubRectOn(_dest, SCALED(x * _tileWidth + _bounds.left),
	    SCALED(y * _tileHeight + _bounds.top), 0, 0,
	    SCALED(_tileWidth), SCALED(_tileHeight));

	// Draw the tile to the screen
	if (tile->getAnim()) {
		// First, create our animated version of the tile
#ifdef IOS_ULTIMA4
		animated->clearImageContents();
#endif
		tile->getAnim()->draw(_animated, tile, mapTile, DIR_NONE);

		// Then draw it to the screen
		_animated->drawSubRectOn(_dest, SCALED(x * _tileWidth + _bounds.left),
			SCALED(y * _tileHeight + _bounds.top), 0, 0,
			SCALED(_tileWidth), SCALED(_tileHeight));
	} else {
		image->drawSubRectOn(_dest, SCALED(x * _tileWidth + _bounds.left),
			SCALED(y * _tileHeight + _bounds.top),
			0, SCALED(_tileHeight * mapTile._frame),
			SCALED(_tileWidth), SCALED(_tileHeight));
	}

	// Draw the focus around the tile if it has the focus
	if (focus)
		drawFocus(x, y);
}

void TileView::drawTile(Std::vector<MapTile> &tiles, bool focus, int x, int y) {
	assertMsg(x < _columns, "x value of %d out of range", x);
	assertMsg(y < _rows, "y value of %d out of range", y);

	// Clear tile contents
	_animated->fillRect(0, 0, SCALED(_tileWidth), SCALED(_tileHeight), 0, 0, 0, 255);
	_animated->drawSubRectOn(_dest,
		SCALED(x * _tileWidth + _bounds.left), SCALED(y * _tileHeight + _bounds.top),
		0, 0,
		SCALED(_tileWidth), SCALED(_tileHeight)
	);

	// Iterate through rendering each of the needed tiles
 	for (Std::vector<MapTile>::reverse_iterator t = tiles.rbegin(); t != tiles.rend(); ++t) {
		MapTile &frontTile = *t;
		Tile *frontTileType = _tileSet->get(frontTile._id);

		if (!frontTileType) {
			// TODO: This leads to an error. It happens after graphics mode changes.
			return;
		}

		// Get the image for the tile
		Image *image = frontTileType->getImage();

		// Draw the tile to the screen
		if (frontTileType->getAnim()) {
			// First, create our animated version of the tile
			frontTileType->getAnim()->draw(_animated, frontTileType, frontTile, DIR_NONE);
		} else {
			if (!image)
				// FIXME: This is a problem, error message it.
				return;
			image->drawSubRectOn(_animated, 0, 0,
				0, SCALED(_tileHeight * frontTile._frame),
				SCALED(_tileWidth),  SCALED(_tileHeight)
			);
		}

		// Then draw it to the screen
		_animated->drawSubRectOn(_dest, SCALED(x * _tileWidth + _bounds.left),
			SCALED(y * _tileHeight + _bounds.top), 0, 0,
				SCALED(_tileWidth), SCALED(_tileHeight)
		);
	}

	// Draw the focus around the tile if it has the focus
	if (focus)
		drawFocus(x, y);
}

void TileView::drawFocus(int x, int y) {
	assertMsg(x < _columns, "x value of %d out of range", x);
	assertMsg(y < _rows, "y value of %d out of range", y);

	// Draw the focus rectangle around the tile
	if ((g_screen->_currentCycle * 4 / SCR_CYCLE_PER_SECOND) % 2) {
		// left edge
		_screen->fillRect(SCALED(x * _tileWidth + _bounds.left),
			SCALED(y * _tileHeight + _bounds.top),
			SCALED(2), SCALED(_tileHeight), 0xff, 0xff, 0xff);

		// top edge
		_screen->fillRect(SCALED(x * _tileWidth + _bounds.left),
			SCALED(y * _tileHeight + _bounds.top),
			SCALED(_tileWidth), SCALED(2),
			0xff, 0xff, 0xff);

		// Right edge
		_screen->fillRect(SCALED((x + 1) * _tileWidth + _bounds.left - 2),
		    SCALED(y * _tileHeight + _bounds.top),
			SCALED(2), SCALED(_tileHeight),
		    0xff, 0xff, 0xff);

		// Bottom edge
		_screen->fillRect(SCALED(x * _tileWidth + _bounds.left),
			SCALED((y + 1) * _tileHeight + _bounds.top - 2),
			SCALED(_tileWidth), SCALED(2),
			0xff, 0xff, 0xff);
	}
}

void TileView::setTileset(Tileset *tileset) {
	this->_tileSet = tileset;
}

} // End of namespace Ultima4
} // End of namespace Ultima
