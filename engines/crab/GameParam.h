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

#include "XMLDoc.h"
#include "common_header.h"
#include "loaders.h"

// The index for all levels in the game
struct LevelPath {
	// The file paths
	std::string layout, asset;

	// The name of the level
	std::string name;

	LevelPath() : layout(""), asset(""), name("") {}

	void Load(rapidxml::xml_node<char> *node) {
		LoadStr(name, "name", node);
		LoadStr(layout, "layout", node);
		LoadStr(asset, "res", node);
	}
};

// Stores all layout paths for the game
struct FilePaths {
	// Resources common to all levels and states
	std::string common;

	// Mod file location, current mod and their extension
	std::string mod_path, mod_ext, mod_cur;

	// Main menu resources
	std::string mainmenu_l, mainmenu_r;

	// Sounds
	std::string sound_effect, sound_music;

	// Fonts and window icon file
	std::string font, icon;

	// Save directory and extension
	std::string save_dir, save_ext;

	// The location of the shader index file
	std::string shaders;

	// The location of the color index file
	std::string colors;

	// The list of levels in the game
	std::unordered_map<std::string, LevelPath> level;

	// The file path of the current resource file
	std::string current_r;

	// The application data path (where saves and settings are stored)
	std::string appdata;

	// Has this been loaded?
	bool loaded;

	FilePaths();
	void Load(const std::string &filename);
	void LoadLevel(const std::string &filename);
};

// Storage pool used for saving numbers as strings
class StringPool {
	// Store integer strings here
	std::unordered_map<int, std::string> pool_i;

	// Store floating point strings here
	struct FloatString {
		float val;
		std::string str;

		FloatString() { val = 0.0f; }
	};

	std::list<FloatString> pool_f;

public:
	StringPool() {
		pool_i.clear();
		pool_f.clear();
	}

	const char *Get(const int &num) {
		if (pool_i.count(num) == 0)
			pool_i[num] = NumberToString<int>(num);

		return pool_i.at(num).c_str();
	}

	const char *FGet(const float &num) {
		for (auto &i : pool_f)
			if (i.val == num)
				return i.str.c_str();

		FloatString fs;
		fs.val = num;
		fs.str = NumberToString<float>(num);
		pool_f.push_back(fs);

		auto ret = pool_f.rbegin();
		return ret->str.c_str();
	}
};

// Our source of random numbers
class RandomNumberGen {
	std::random_device rd;
	std::default_random_engine dre;

public:
	RandomNumberGen() : rd(), dre(rd()) {}

	int Num() { return dre(); }
};

struct TempValue {
	// Differences between normal mode and iron man mode
	// save button - iron man saves in existing file
	// load button - hidden in iron man
	// exit - saves and exits in iron man
	// quick save and quick load - operate on the iron man file in iron man mode
	bool ironman;

	// This is the filename a player chose when selecting "new game" + iron man mode
	std::string filename;

	// We use this to see whether the player is exiting to main menu or to credits
	bool credits;

	TempValue() : filename("No IronMan") {
		ironman = false;
		credits = false;
	}
};

// Our global objects

// Where we store some temporary data used in between GameState swapping
extern TempValue gTemp;

// Whether to draw debug outlines on polygons
extern bool GameDebug;

// Paths of all files
extern FilePaths gFilePath;

// Strings are stored here to avoid duplicates and invalid values when writing to XML
extern StringPool gStrPool;

// Generate random numbers using this
extern RandomNumberGen gRandom;

#endif // CRAB_GAMEPARAM_H
