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

MapLoader::MapLoader() {
	_mapLoaded = false;
}

bool MapLoader::loadMap(Common::SeekableReadStream *stream, int32 length) {

	if (_mapLoaded) {
		return false;
	}

	// Load MSM data into mapHeader
	mapHeader = new MSMHeader;
	stream->read(mapHeader->name, 32);
	mapHeader->width = stream->readUint16LE();
	mapHeader->height = stream->readUint16LE();
	mapHeader->background = stream->readUint32LE();
	mapHeader->foreground = stream->readUint32LE();
	mapHeader->iconNum = stream->readUint16LE();
	mapHeader->iconList = stream->readUint32LE();
	mapHeader->infoNum = stream->readUint16LE();
	mapHeader->infoList = stream->readUint32LE();

	/*
		TODO: Set the InMapName in hdb.cpp
	*/

	warning("STUB: MAPLOADER: LOAD MAP INCOMPLETE");
	return false;
}

int MapLoader::loadTiles() {
	warning("STUB: MAPLOADER: LOAD TILES");
	return 0;
}

}
