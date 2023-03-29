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

#include "crab/level/level.h"

namespace Crab {

using namespace TMX;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::anim;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Used to sort background sprites
//------------------------------------------------------------------------
bool CompSpriteLayer(const Sprite &a, const Sprite &b) {
	return (a.layer < b.layer);
}

//------------------------------------------------------------------------
// Purpose: Load the level
//------------------------------------------------------------------------
void Level::Load(const std::string &filename, pyrodactyl::event::Info &info,
				 pyrodactyl::event::TriggerSet &game_over, const int &player_x, const int &player_y) {
	Reset();
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("level");
		if (NodeValid(node)) {
			std::string vis;
			LoadStr(vis, "map", node, false);
			if (vis == "false")
				showmap.Set(false);
			else
				showmap.Set(true);

			if (NodeValid("preview", node))
				LoadStr(preview_path, "path", node->first_node("preview"));

			if (NodeValid("music", node)) {
				LoadNum(music.id, "id", node->first_node("music"));
				pyrodactyl::music::gMusicManager.PlayMusic(music.id);
			}

			if (NodeValid("map", node)) {
				rapidxml::xml_node<char> *mapnode = node->first_node("map");
				std::string path, tmxfile;
				LoadStr(path, "path", mapnode);
				LoadStr(tmxfile, "file", mapnode);

				terrain.Load(path, tmxfile);

				// Remember to load the terrain data before constructing the pathfinding grid
				pathfindingGrid.SetupNodes(terrain);

				terrain.grid = &pathfindingGrid;

				if (NodeValid("loc", mapnode))
					map_loc.Load(mapnode->first_node("loc"));

				if (NodeValid("clip", mapnode)) {
					rapidxml::xml_node<char> *clipnode = mapnode->first_node("clip");
					LoadNum(map_clip.id, "id", clipnode);
					map_clip.rect.Load(clipnode);
				}
			}

			if (NodeValid("sprites", node)) {
				rapidxml::xml_node<char> *spritenode = node->first_node("sprites");
				int count = 0;
				for (auto n = spritenode->first_node(); n != NULL; n = n->next_sibling(), ++count) {
					Sprite s;
					s.Load(n, anim_set);

					std::string str = n->name();
					if (str == "player") {
						player_index = objects.size();
						if (player_x != -1 && player_y != -1) {
							s.X(player_x);
							s.Y(player_y);
						}
					}

					objects.push_back(s);
				}
			}

			if (NodeValid("background", node)) {
				rapidxml::xml_node<char> *spritenode = node->first_node("background");
				for (auto n = spritenode->first_node(); n != NULL; n = n->next_sibling()) {
					Sprite s;
					s.Load(n, anim_set);
					background.push_back(s);
				}

				std::sort(background.begin(), background.end(), CompSpriteLayer);
			}

			if (NodeValid("fly", node)) {
				rapidxml::xml_node<char> *spritenode = node->first_node("fly");
				for (auto n = spritenode->first_node(); n != NULL; n = n->next_sibling()) {
					Sprite s;
					s.Load(n, anim_set);

					// Set the timer target for the first time
					s.ai_data.walk.timer.Target(sc_default.fly.delay_min + (gRandom.Num() % sc_default.fly.delay_max));

					fly.push_back(s);
				}
			}

			if (NodeValid("movement", node)) {
				rapidxml::xml_node<char> *movnode = node->first_node("movement");
				for (auto n = movnode->first_node("set"); n != NULL; n = n->next_sibling("set"))
					move_set.push_back(n);
			}

			if (NodeValid("popup", node, false))
				pop.Load(node->first_node("popup"));

			game_over.Clear();
			if (NodeValid("game_over", node, false))
				game_over.Load(node->first_node("game_over"));
			else if (player_index < objects.size()) {
				// The default lose condition is the death of the player
				using namespace pyrodactyl::event;
				Trigger t;
				t.type = TRIG_STAT;
				t.target = STATNAME_HEALTH;
				t.subject = objects[player_index].ID();
				t.operation = "<";
				t.val = "1";
				game_over.Add(t);
			}

			CalcProperties(info);
		}
	}

	SetCamera();
}

//------------------------------------------------------------------------
// Purpose: Build an index of all animation files, called once at start
//------------------------------------------------------------------------
void Level::LoadMoves(const std::string &filename) {
	XMLDoc mov_list(filename);
	if (mov_list.ready()) {
		rapidxml::xml_node<char> *node = mov_list.Doc()->first_node("movelist");
		for (auto n = node->first_node("set"); n != NULL; n = n->next_sibling("set")) {
			unsigned int pos = anim_set.size();

			LoadNum(pos, "id", n);
			if (pos >= anim_set.size())
				anim_set.resize(pos + 1);

			// See if there is an alternate moveset for low quality setting
			// If no, just load the regular one
			if (!gScreenSettings.quality) {
				if (!LoadStr(anim_set[pos], "path_low", n))
					LoadStr(anim_set[pos], "path", n);
			} else
				LoadStr(anim_set[pos], "path", n);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Load the default sprite constant parameters
//------------------------------------------------------------------------
void Level::LoadConst(const std::string &filename) {
	XMLDoc doc(filename);
	if (doc.ready()) {
		rapidxml::xml_node<char> *node = doc.Doc()->first_node("constant");
		if (NodeValid(node))
			sc_default.Load(node);
	}
}

//------------------------------------------------------------------------
// Purpose: Save all sprite positions to save file
//------------------------------------------------------------------------
void Level::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("player_index", gStrPool.Get(player_index)));

	for (auto &i : objects) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "sprite");
		i.SaveState(doc, child);
		root->append_node(child);
	}
}

//------------------------------------------------------------------------
// Purpose: Load all sprite positions from save file
//------------------------------------------------------------------------
void Level::LoadState(rapidxml::xml_node<char> *node) {
	LoadNum(player_index, "player_index", node);

	auto i = objects.begin();
	for (auto *n = node->first_node("sprite"); n != NULL && i != objects.end(); n = n->next_sibling("sprite"), ++i)
		i->LoadState(n);
}

} // End of namespace Crab
