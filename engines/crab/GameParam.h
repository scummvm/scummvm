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

#ifndef CRAB_GAMEPARAM_H
#define CRAB_GAMEPARAM_H

#include "common/hashmap.h"
#include "common/rect.h"
#include "common/str.h"
#include "crab/loaders.h"
#include "crab/rapidxml/rapidxml.hpp"

namespace Crab {

// The index for all levels in the game
struct LevelPath {
	// The file paths
	Common::String _layout, _asset;

	// The name of the level
	Common::String _name;

	LevelPath() : _layout(""), _asset(""), _name("") {}

	void load(rapidxml::xml_node<char> *node) {
		loadStr(_name, "name", node);
		loadStr(_layout, "layout", node);
		loadStr(_asset, "res", node);
	}
};

// Stores all layout paths for the game
struct FilePaths {
	// Resources common to all levels and states
	Common::String _common;

	// Mod file location, current mod and their extension
	Common::String _modPath, _modExt, _modCur;

	// Main menu resources
	Common::String _mainmenuL, _mainmenuR;

	// Sounds
	Common::String _soundEffect, _soundMusic;

	// Fonts and window icon file
	Common::String _font, _icon;

	// Save directory and extension
	Common::String _saveDir, _saveExt;

	// The location of the shader index file
	Common::String _shaders;

	// The location of the color index file
	Common::String _colors;

	// The list of levels in the game
	Common::HashMap<Common::String, LevelPath> _level;

	// The file path of the current resource file
	Common::String _currentR;

	// The application data path (where saves and settings are stored)
	Common::String _appdata;

	// Has this been loaded?
	bool _loaded;

	FilePaths();
	void load(const Common::String &filename);
	void loadLevel(const Common::String &filename);
};

// Storage pool used for saving numbers as strings
class StringPool {
	// Store integer strings here
	// std::unordered_map<int, Common::String> pool_i;
	Common::HashMap<int, Common::String> poolI;

	// Store floating point strings here
	struct FloatString {
		float _val;
		Common::String _str;

		FloatString() {
			_val = 0.0f;
		}
	};

	// std::list<FloatString> pool_f;
	Common::List<FloatString> poolF;

public:
	StringPool() {
		poolI.clear();
		poolF.clear();
	}

	const char *Get(const int &num) {
		if (poolI.contains(num) == false)
			poolI[num] = numberToString<int>(num);

		return poolI.getVal(num).c_str();
	}

	const char *FGet(const float &num) {
		for (auto &i : poolF)
			if (i._val == num)
				return i._str.c_str();

		FloatString fs;
		fs._val = num;
		//fs.str = NumberToString<float>(num);
		poolF.push_back(fs);

		auto ret = poolF.back();
		return ret._str.c_str();
	}
};

struct TempValue {
	// Differences between normal mode and iron man mode
	// save button - iron man saves in existing file
	// load button - hidden in iron man
	// exit - saves and exits in iron man
	// quick save and quick load - operate on the iron man file in iron man mode
	bool _ironman;

	// This is the filename a player chose when selecting "new game" + iron man mode
	Common::String _filename;

	// We use this to see whether the player is exiting to main menu or to credits
	bool _credits;

	TempValue() : _filename("No IronMan") {
		_ironman = false;
		_credits = false;
	}
};

// Our global objects

// Whether to draw debug outlines on polygons
extern bool GameDebug;

} // End of namespace Crab

#endif // CRAB_GAMEPARAM_H
