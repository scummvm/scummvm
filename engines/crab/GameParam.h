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

#include "crab/XMLDoc.h"
#include "crab/common_header.h"
#include "crab/loaders.h"

namespace Crab {

// The index for all levels in the game
struct LevelPath {
	// The file paths
	Common::String layout, asset;

	// The name of the level
	Common::String name;

	LevelPath() : layout(""), asset(""), name("") {}

	void load(rapidxml::xml_node<char> *node) {
		loadStr(name, "name", node);
		loadStr(layout, "layout", node);
		loadStr(asset, "res", node);
	}
};

// Stores all layout paths for the game
struct FilePaths {
	// Resources common to all levels and states
	Common::String common;

	// Mod file location, current mod and their extension
	Common::String mod_path, mod_ext, mod_cur;

	// Main menu resources
	Common::String mainmenu_l, mainmenu_r;

	// Sounds
	Common::String sound_effect, sound_music;

	// Fonts and window icon file
	Common::String font, icon;

	// Save directory and extension
	Common::String save_dir, save_ext;

	// The location of the shader index file
	Common::String shaders;

	// The location of the color index file
	Common::String colors;

	// The list of levels in the game
	Common::HashMap<Common::String, LevelPath> level;

	// The file path of the current resource file
	Common::String current_r;

	// The application data path (where saves and settings are stored)
	Common::String appdata;

	// Has this been loaded?
	bool loaded;

	FilePaths();
	void load(const Common::String &filename);
	void LoadLevel(const Common::String &filename);
};

// Storage pool used for saving numbers as strings
class StringPool {
	// Store integer strings here
	// std::unordered_map<int, Common::String> pool_i;
	Common::HashMap<int, Common::String> pool_i;

	// Store floating point strings here
	struct FloatString {
		float val;
		Common::String str;

		FloatString() { val = 0.0f; }
	};

	// std::list<FloatString> pool_f;
	Common::List<FloatString> pool_f;

public:
	StringPool() {
		pool_i.clear();
		pool_f.clear();
	}

	const char *Get(const int &num) {
#if 0
		if (pool_i.count(num) == 0)
			pool_i[num] = NumberToString<int>(num);

		return pool_i[num].c_str();
#endif
		if (pool_i.contains(num) == false)
			pool_i[num] = NumberToString<int>(num);

		return pool_i.getVal(num).c_str();
	}

	const char *FGet(const float &num) {
		for (auto &i : pool_f)
			if (i.val == num)
				return i.str.c_str();

		FloatString fs;
		fs.val = num;
		//fs.str = NumberToString<float>(num);
		pool_f.push_back(fs);

		auto ret = pool_f.back();
		return ret.str.c_str();
	}
};

// Our source of random numbers
#if 0
class RandomNumberGen {
	std::random_device rd;
	std::default_random_engine dre;

public:
	RandomNumberGen() : rd(), dre(rd()) {}

	int Num() { return dre(); }
};
#endif

struct TempValue {
	// Differences between normal mode and iron man mode
	// save button - iron man saves in existing file
	// load button - hidden in iron man
	// exit - saves and exits in iron man
	// quick save and quick load - operate on the iron man file in iron man mode
	bool ironman;

	// This is the filename a player chose when selecting "new game" + iron man mode
	Common::String filename;

	// We use this to see whether the player is exiting to main menu or to credits
	bool credits;

	TempValue() : filename("No IronMan") {
		ironman = false;
		credits = false;
	}
};

// Our global objects

// Whether to draw debug outlines on polygons
extern bool GameDebug;

// Strings are stored here to avoid duplicates and invalid values when writing to XML
extern StringPool *gStrPool;

// Generate random numbers using this
//extern RandomNumberGen gRandom;

}

#endif // CRAB_GAMEPARAM_H
