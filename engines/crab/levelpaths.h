/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef LEVELPATHS_H
#define LEVELPATHS_H

#include <string>

namespace Crab {

class LevelPaths {
	Common::String layout, asset;

public:
	LevelPaths() {
		layout = "";
		asset = "";
	}

	LevelPaths(const char *Layout, const char *Asset) {
		layout = Layout;
		asset = Asset;
	}

	const char *LayoutPath() { return layout.c_str(); }
	void LayoutPath(const char *str) { layout = str; }

	const char *AssetPath() { return asset.c_str(); }
	void AssetPath(const char *str) { asset = str; }
};

} // End of namespace Crab

#endif // LEVELPATHS_H
