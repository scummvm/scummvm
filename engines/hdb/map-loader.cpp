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

Map::Map() {
	_mapLoaded = false;
}

bool Map::load(Common::SeekableReadStream *stream) {
	if (_mapLoaded) {
		return false;
	}

	// Load MSM data header
	stream->read(_name, 32);
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_backgroundOffset = stream->readUint32LE();
	_foregroundOffset = stream->readUint32LE();
	_iconNum = stream->readUint16LE();
	_iconListOffset = stream->readUint32LE();
	_infoNum = stream->readUint16LE();
	_infoListOffset = stream->readUint32LE();

	// Reading Background
	_background = new uint16[_width * _height];
	stream->seek(_backgroundOffset);
	for (int i = 0; i < _width * _height; i++) {
		_background[i] = stream->readUint16LE();
	}

	// Reading Foreground
	_foreground = new uint16[_width * _height];
	stream->seek(_foregroundOffset);
	for (int i = 0; i < _width * _height; i++) {
		_foreground[i] = stream->readUint16LE();
	}

	// Reading Icon List
	_iconList = new MSMIcon[_iconNum];
	for (uint i = 0; i < _iconNum; i++) {
		_iconList[i].icon = stream->readUint16LE();
		_iconList[i].x = stream->readUint16LE();
		_iconList[i].y = stream->readUint16LE();

		stream->read(_iconList[i].funcInit, 32);
		stream->read(_iconList[i].funcAction, 32);
		stream->read(_iconList[i].funcUse, 32);

		_iconList[i].dir = stream->readUint16LE();
		_iconList[i].level = stream->readUint16LE();
		_iconList[i].value1 = stream->readUint16LE();
		_iconList[i].value2 = stream->readUint16LE();
	}

	/*
		TODO: Add the InfoList when it comes up
	*/

	/*
		TODO: Set the InMapName once its setup
	*/

	_mapExplosions = new char[_width * _height];
	_mapExpBarrels = new char[_width * _height];
	_mapLaserBeams = new char[_width * _height];


	_mapX = _mapY = 0;

	_mapLoaded = true;

	return true;
}

int Map::loadTiles() {

	int tile, temp;
	int skyIndex = 0;

	// Load all tiles
	for (uint j = 0; j < _height; j++) {
		for (uint i = 0; i < _width; i++) {
			tile = _background[j * _width + i];
		}
	}

	return skyIndex;
}
}
