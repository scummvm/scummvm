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

#include "sherlock/tattoo/tattoo_map.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Tattoo {

void MapEntry::clear() {
	_iconNum = -1;
	_description = "";
}

/*-------------------------------------------------------------------------*/

TattooMap::TattooMap(SherlockEngine *vm) : Map(vm) {
	loadData();
}

int TattooMap::show() {
	// TODO
	return 61;
}

void TattooMap::loadData() {
	Resources &res = *_vm->_res;
	char c;

	Common::SeekableReadStream *stream = res.load("map.txt");

	_data.resize(100);
	for (uint idx = 0; idx < _data.size(); ++idx)
		_data[idx].clear();

	do
	{
		// Find the start of the number
		do {
			c = stream->readByte();
			if (stream->pos() >= stream->size())
				return;
		} while (c < '0' || c > '9');

		// Get the scene number
		Common::String locStr;
		locStr += c;
		while ((c = stream->readByte()) != '.')
			locStr += c;
		MapEntry &mapEntry = _data[atoi(locStr.c_str()) - 1];

		// Get the location name
		while (stream->readByte() != '"')
			;

		while ((c = stream->readByte()) != '"')
			mapEntry._description += c;

		// Find the ( specifying the (X,Y) position of the Icon
		while (stream->readByte() != '(')
			;

		// Get the X Position of the icon
		Common::String numStr;
		while ((c = stream->readByte()) != ',')
			numStr += c;
		mapEntry.x = atoi(numStr.c_str());

		// Get the Y position of the icon
		numStr = "";
		while ((c = stream->readByte()) != ')')
			numStr += c;
		mapEntry.y = atoi(numStr.c_str());

		// Find and get the location's icon number
		while (stream->readByte() != '#')
			;

		Common::String iconStr;
		while (stream->pos() < stream->size() && (c = stream->readByte()) != '\r')
			iconStr += c;

		mapEntry._iconNum = atoi(iconStr.c_str()) - 1;
	} while (stream->pos() < stream->size());
}

} // End of namespace Tattoo

} // End of namespace Sherlock
