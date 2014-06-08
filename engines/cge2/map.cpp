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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/map.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

Map::Map(CGE2Engine *vm) :_vm(vm) {}

Map::~Map() {
	_container.clear();
}

int Map::convertCoord(int coord) {
	return (coord + (kMapGrid >> 1)) & kMapMask;
}

void Map::load(int cave) {
	char fname[] = "%.2d.MAP\0";
	Common::String filename = Common::String::format(fname, cave);
	clear();
	if (!_vm->_resman->exist(filename.c_str()))
		return;

	EncryptedStream file(_vm, filename.c_str());
	
	char tmpStr[kLineMax + 1];
	Common::String line;

	for (line = file.readLine(); !file.eos(); line = file.readLine()) {
		if (line.size() == 0)
			continue;

		Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));

		char *currPos = tmpStr;
		currPos = strtok(currPos, " (),");
		int x = atoi(currPos);
		currPos = strtok(nullptr, " (),");
		int y = atoi(currPos);
		_container.push_back(V2D(_vm, convertCoord(x), convertCoord(y)));

		while (true) {
			currPos = strtok(nullptr, " (),");
			if (currPos == nullptr)
				break;
			int x = atoi(currPos);
			currPos = strtok(nullptr, " (),");
			int y = atoi(currPos);
			_container.push_back(V2D(_vm, convertCoord(x), convertCoord(y)));
		}
	}
}

int Map::size() {
	return _container.size();
}

void Map::clear() {
	_container.clear();
}

V2D &Map::operator[](int idx) {
	return _container[idx];
}

} // End of namespace CGE2
