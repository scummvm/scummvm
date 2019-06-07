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
	_tileSkyStars = getTileIndex("t32_sky_stars");
	_tileSkyStarsLeft = getTileIndex("t32_sky_stars_left_slow");
	_tileSkyClouds = getTileIndex("t32_sky_stars_stationary"); // Not completely sure about this filename.
	_skyClouds = NULL;

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

Tile *DrawMan::getTile(int index) {

	if (index < 0 || index > _numTiles) {
		return NULL;
	}
	if (_tLookupArray[index].skyIndex) {
		// We don't draw Sky Tiles, so return NULL
		return NULL;
	}

	if (_tLookupArray[index].tData == NULL) {
		Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(_tLookupArray[index].filename, TYPE_TILE32);
		Tile *tile = new Tile;
		tile->load(stream);
		_tLookupArray[index].tData = tile;
	}

	return _tLookupArray[index].tData;
}

int DrawMan::getTileIndex(const char *name) {
	for (int i = 0; i < _numTiles; i++) {
		if (_tLookupArray[i].filename == name) {
			return i;
		}
	}
	return -1;
}

Picture *DrawMan::getPicture(const char *name) {
	Common::SeekableReadStream *stream = g_hdb->_fileMan->findFirstData(name, TYPE_PIC);
	Picture *picture = new Picture;
	picture->load(stream);
	return picture;
}

int DrawMan::isSky(int index) {
	if (!index) {
		return 0;
	}

	for (int i = 0; i < kMaxSkies; i++) {
		if(_skyTiles[i] == index) {
			return index + 1; // The skyTiles are indexed from 1. 0 => No Sky tile
		}
	}

	return 0;
}

void DrawMan::setSky(int skyIndex) {
	int tileIndex = _skyTiles[skyIndex - 1];
	_currentSky = skyIndex;

	// Clear memory used by last sky
	if (tileIndex != _tileSkyClouds && _skyClouds) {
		delete _skyClouds;
		_skyClouds = NULL;
	}

	// Setup current sky
	if (tileIndex == _tileSkyStars) {
		setup3DStars();
		return;
	} else if (skyIndex == _tileSkyStarsLeft) {
		setup3DStarsLeft();
		return;
	} else if (skyIndex == _tileSkyStars) {
		warning("STUB: DRAWMAN::setSky: getPicture( CLOUDY_SKIES )");
		return;
	}
}

void DrawMan::setup3DStars() {
	for (int i = 0; i < kNum3DStars; i++) {
		_stars3D[i].x = g_hdb->_rnd->getRandomNumber(kScreenWidth) % kScreenWidth;
		_stars3D[i].y = g_hdb->_rnd->getRandomNumber(kScreenHeight) % kScreenHeight;
		_stars3D[i].speed = (g_hdb->_rnd->getRandomNumber(256) % 256);
		_stars3D[i].speed >>= 1;
		_stars3D[i].color = _stars3D[i].speed / 64;
	}
}

void DrawMan::setup3DStarsLeft() {
	for (int i = 0; i < kNum3DStars; i++) {
		_stars3DSlow[i].x = g_hdb->_rnd->getRandomNumber(kScreenWidth) % kScreenWidth;
		_stars3DSlow[i].y = g_hdb->_rnd->getRandomNumber(kScreenHeight) % kScreenHeight;
		_stars3DSlow[i].speed = ((double) (1 + (g_hdb->_rnd->getRandomNumber(5) % 5))) / 6.0;
		_stars3DSlow[i].color = (int) (_stars3DSlow[i].speed * 4.00);
	}
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
