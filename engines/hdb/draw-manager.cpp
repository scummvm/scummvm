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

#include "hdb/hdb.h"

namespace HDB {

DrawMan::DrawMan() {
	_tLookupArray = NULL;
	_systemInit = false;
}

bool DrawMan::init() {

	// Read total number of tiles in game
	_numTiles = g_hdb->_fileMan->getCount("t32_", TYPE_TILE32);
	if (!_numTiles) {
		return false;
	}

	// Setup Tile Lookup Array
	_tLookupArray = new TileLookup[_numTiles];
	Common::Array<const char *> tileData = *g_hdb->_fileMan->findFiles("t32_", TYPE_TILE32);

	int index = 0, skyIndex = 0;
	for (; index < _numTiles; index++) {
		_tLookupArray[index].filename = tileData[index];
		_tLookupArray[index].tData = NULL;
		_tLookupArray[index].skyIndex = 0;
		_tLookupArray[index].animIndex = index;
		// Check if the loaded Tile is a Sky Tile
		if (((Common::String)tileData[index]).contains("sky") && (skyIndex < kMaxSkies)) {
			_tLookupArray[index].skyIndex = skyIndex + 1;
			_skyTiles[skyIndex] = index;
			skyIndex++;
		}
	}

	/*
		TODO: Add Animating Tile Info
	*/

	// Init Sky Data
	_currentSky = 0;

	/*
		TODO: Setup Gamma Table
	*/

	/*
		TODO: Load Mouse Pointer and Display Cursor
	*/

	/*
		TODO: Load all 4 levels of star colors and the snowflake
	*/

	_systemInit = true;
	return true;
}

Picture::~Picture() {
	_surface.free();
}

Graphics::Surface Picture::load(Common::SeekableReadStream *stream) {
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();
	stream->read(_name, 64);
	
	Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);

	debug(8, "Picture: _width: %d, _height: %d", _width, _height);
	debug(8, "Picture: _name: %s", _name);

	_surface.create(_width, _height, format);
	stream->readUint32LE(); // Skip Win32 Surface

	uint16 *ptr;

	for (uint y = 0; y < _height; y++) {
		ptr = (uint16 *) _surface.getBasePtr(0, y);
		for (uint x = 0; x < _width; x++) {
			*ptr = TO_LE_16(stream->readUint16LE());
			ptr++;
		}
	}

	return _surface;
}

Tile::~Tile() {
	_surface.free();
}

Graphics::Surface Tile::load(Common::SeekableReadStream *stream) {
	_flags = stream->readUint32LE();
	stream->read(_name, 64);

	Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);

	_surface.create(32, 32, format);
	stream->readUint32LE(); // Skip Win32 Surface

	uint16 *ptr;

	for (uint y = 0; y < 32; y++) {
		ptr = (uint16 *)_surface.getBasePtr(0, y);
		for (uint x = 0; x < 32; x++) {
			*ptr = TO_LE_16(stream->readUint16LE());
			ptr++;
		}
	}

	return _surface;
}

}
