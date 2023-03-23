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

// Temporary values go here
TempValue gTemp;

// The global file path object
FilePaths gFilePath;

// Are we in debug mode or not?
bool GameDebug = false;

// A temporary list of strings we use when storing numbers in a file
StringPool gStrPool;

// Generate random numbers using this
//RandomNumberGen gRandom;

FilePaths::FilePaths() : common("res/levels/common.xml"),
						 mod_path("mods/"),
						 mod_ext(".unrmod"),
						 mod_cur("res/default.xml"),
						 mainmenu_l("res/layout/main_menu_l.xml"),
						 mainmenu_r("res/layout/main_menu_r.xml"),
						 sound_effect("res/sounds/effects.xml"),
						 sound_music("res/sounds/music.xml"),
						 font("res/fonts/fonts.xml"),
						 icon("res/gfx/icon.bmp"),
						 save_dir("save/"),
						 save_ext(".unr"),
						 shaders("res/shaders/list.xml"),
						 colors("res/gfx/colors.xml"),
						 current_r("res/layout/main_menu_r.xml") {
	level.clear();
	loaded = false;

#if 0
	char *path = SDL_GetPrefPath("Pyrodactyl", "Unrest");
	appdata = path;
	SDL_free(path);
#endif
	warning("FilePaths::FilePaths() setting appdata directory to game dir");
	appdata = "./";
}

void FilePaths::LoadLevel(const Common::String &filename) {
	level.clear();
	XMLDoc lev_list(filename);
	if (lev_list.ready()) {
		rapidxml::xml_node<char> *node = lev_list.Doc()->first_node("world");

		for (rapidxml::xml_node<char> *n = node->first_node("loc"); n != NULL; n = n->next_sibling("loc")) {
			LevelPath l;
			l.Load(n);

			Common::String id;
			LoadStr(id, "id", n);

			level[id] = l;
		}
	}
}

void FilePaths::Load(const Common::String &filename) {
	XMLDoc settings(filename);
	if (settings.ready()) {
		rapidxml::xml_node<char> *node = settings.Doc()->first_node("paths");

		if (NodeValid(node) && !loaded) {
			if (NodeValid("icon", node)) {
				rapidxml::xml_node<char> *iconode = node->first_node("icon");
				icon = iconode->value();
			}

			if (NodeValid("common", node)) {
				rapidxml::xml_node<char> *commonnode = node->first_node("common");
				common = commonnode->value();
			}

			if (NodeValid("font", node)) {
				rapidxml::xml_node<char> *fontnode = node->first_node("font");
				font = fontnode->value();
			}

			if (NodeValid("shader", node)) {
				rapidxml::xml_node<char> *shadnode = node->first_node("shader");
				shaders = shadnode->value();
			}

			if (NodeValid("color", node)) {
				rapidxml::xml_node<char> *colnode = node->first_node("color");
				colors = colnode->value();
			}

			if (NodeValid("mod", node)) {
				rapidxml::xml_node<char> *modnode = node->first_node("mod");
				LoadStr(mod_path, "path", modnode);
				LoadStr(mod_ext, "ext", modnode);
				LoadStr(mod_cur, "cur", modnode);
			}

			if (NodeValid("main_menu", node)) {
				rapidxml::xml_node<char> *menunode = node->first_node("main_menu");
				LoadStr(mainmenu_l, "l", menunode);
				LoadStr(mainmenu_r, "r", menunode);
				current_r = mainmenu_r;
			}

			if (NodeValid("sound", node)) {
				rapidxml::xml_node<char> *soundnode = node->first_node("sound");
				LoadStr(sound_effect, "effect", soundnode);
				LoadStr(sound_music, "music", soundnode);
			}

			if (NodeValid("save", node)) {
				rapidxml::xml_node<char> *savenode = node->first_node("save");
				LoadStr(save_dir, "dir", savenode);
				LoadStr(save_ext, "ext", savenode);

				Common::String custom_path;
				if (LoadStr(custom_path, "custom", savenode)) {
#if 0
					using namespace boost::filesystem;

					path customdir(custom_path);
					if (exists(customdir) && is_directory(customdir)) {
						appdata = custom_path;
					}
#endif
					warning("In FilePaths::Load(), customPath : %s", custom_path.c_str());
				}
			}

			loaded = true;
		}
	}
}

}
