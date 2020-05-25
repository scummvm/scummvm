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
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "cge2/map.h"

namespace CGE2 {

Map::Map(CGE2Engine *vm) :_vm(vm) {}

Map::~Map() {
	_container.clear();
}

void Map::clear() {
	_container.clear();
}

void Map::load(int scene) {
	clear();

	const char *fname = "%.2d.MAP";
	Common::String fileName = Common::String::format(fname, scene);
	if (!_vm->_resman->exist(fileName.c_str()))
		return;

	EncryptedStream file(_vm, fileName.c_str());

	Common::String line;
	for (line = file.readLine(); !file.eos(); line = file.readLine()) {
		if (line.empty())
			continue;

		char tmpStr[kLineMax + 1];
		Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));

		char *currPos = tmpStr;
		int x = nextNum(currPos);
		while (true) {
			int y = nextNum(nullptr);
			_container.push_back(V2D(_vm, convertCoord(x), convertCoord(y)));
			x = nextNum(nullptr);
			if (x == -1) // We stop if there are no more data left to process in the current line.
				break;
		}
	}
}

int Map::nextNum(char *currPos) {
	currPos = strtok(currPos, " (),");
	if (currPos == nullptr)
		return -1;
	int num = atoi(currPos);
	return num;
}

int Map::convertCoord(int coord) {
	return (coord + (kMapGrid >> 1)) & kMapMask;
}

int Map::size() {
	return _container.size();
}

V2D *Map::getCoord(int idx) {
	return &_container[idx];
}

} // End of namespace CGE2
