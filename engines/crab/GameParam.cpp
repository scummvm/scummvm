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

#include "crab/GameParam.h"

namespace Crab {
// Are we in debug mode or not?
bool GameDebug = false;

// A temporary list of strings we use when storing numbers in a file
StringPool *gStrPool;

// Generate random numbers using this
//RandomNumberGen gRandom;

FilePaths::FilePaths() : _common("res/levels/common.xml"),
						 _modPath("mods/"),
						 _modExt(".unrmod"),
						 _modCur("res/default.xml"),
						 _mainmenuL("res/layout/main_menu_l.xml"),
						 _mainmenuR("res/layout/main_menu_r.xml"),
						 _soundEffect("res/sounds/effects.xml"),
						 _soundMusic("res/sounds/music.xml"),
						 _font("res/fonts/fonts.xml"),
						 _icon("res/gfx/icon.bmp"),
						 _saveDir("save/"),
						 _saveExt(".unr"),
						 _shaders("res/shaders/list.xml"),
						 _colors("res/gfx/colors.xml"),
						 _currentR("res/layout/main_menu_r.xml") {
	_level.clear();
	_loaded = false;

	warning("FilePaths::FilePaths() setting appdata directory to game dir");
	_appdata = "./";
}

void FilePaths::loadLevel(const Common::String &filename) {
	_level.clear();
	XMLDoc lev_list(filename);
	if (lev_list.ready()) {
		rapidxml::xml_node<char> *node = lev_list.doc()->first_node("world");

		for (rapidxml::xml_node<char> *n = node->first_node("loc"); n != NULL; n = n->next_sibling("loc")) {
			LevelPath l;
			l.load(n);

			Common::String id;
			loadStr(id, "id", n);

			_level[id] = l;
		}
	}
}

void FilePaths::load(const Common::String &filename) {
	XMLDoc settings(filename);
	if (settings.ready()) {
		rapidxml::xml_node<char> *node = settings.doc()->first_node("paths");

		if (nodeValid(node) && !_loaded) {
			if (nodeValid("icon", node)) {
				rapidxml::xml_node<char> *iconode = node->first_node("icon");
				_icon = iconode->value();
			}

			if (nodeValid("common", node)) {
				rapidxml::xml_node<char> *commonnode = node->first_node("common");
				_common = commonnode->value();
			}

			if (nodeValid("font", node)) {
				rapidxml::xml_node<char> *fontnode = node->first_node("font");
				_font = fontnode->value();
			}

			if (nodeValid("shader", node)) {
				rapidxml::xml_node<char> *shadnode = node->first_node("shader");
				_shaders = shadnode->value();
			}

			if (nodeValid("color", node)) {
				rapidxml::xml_node<char> *colnode = node->first_node("color");
				_colors = colnode->value();
			}

			if (nodeValid("mod", node)) {
				rapidxml::xml_node<char> *modnode = node->first_node("mod");
				loadStr(_modPath, "path", modnode);
				loadStr(_modExt, "ext", modnode);
				loadStr(_modCur, "cur", modnode);
			}

			if (nodeValid("main_menu", node)) {
				rapidxml::xml_node<char> *menunode = node->first_node("main_menu");
				loadStr(_mainmenuL, "l", menunode);
				loadStr(_mainmenuR, "r", menunode);
				_currentR = _mainmenuR;
			}

			if (nodeValid("sound", node)) {
				rapidxml::xml_node<char> *soundnode = node->first_node("sound");
				loadStr(_soundEffect, "effect", soundnode);
				loadStr(_soundMusic, "music", soundnode);
			}

			if (nodeValid("save", node)) {
				rapidxml::xml_node<char> *savenode = node->first_node("save");
				loadStr(_saveDir, "dir", savenode);
				loadStr(_saveExt, "ext", savenode);

				Common::String custom_path;
				if (loadStr(custom_path, "custom", savenode)) {
					warning("In FilePaths::load(), customPath : %s", custom_path.c_str());
				}
			}

			_loaded = true;
		}
	}
}

}
