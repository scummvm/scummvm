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
	return (a._layer < b._layer);
}

//------------------------------------------------------------------------
// Purpose: Load the level
//------------------------------------------------------------------------
void Level::load(const Common::String &filename, pyrodactyl::event::Info &info,
				 pyrodactyl::event::TriggerSet &gameOver, const int &playerX, const int &playerY) {
	reset();
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("level");
		if (nodeValid(node)) {
			Common::String vis;
			loadStr(vis, "map", node, false);
			if (vis == "false")
				_showmap.set(false);
			else
				_showmap.set(true);

			if (nodeValid("preview", node))
				loadStr(_previewPath, "path", node->first_node("preview"));

			if (nodeValid("music", node)) {
				loadNum(_music._id, "id", node->first_node("music"));
				g_engine->_musicManager->playMusic(_music._id);
			}

			if (nodeValid("map", node)) {
				rapidxml::xml_node<char> *mapnode = node->first_node("map");
				Common::String path, tmxfile;
				loadStr(path, "path", mapnode);
				loadStr(tmxfile, "file", mapnode);

				_terrain.load(path, tmxfile);

				// Remember to load the terrain data before constructing the pathfinding grid
				_pathfindingGrid.SetupNodes(_terrain);

				_terrain._grid = &_pathfindingGrid;

				if (nodeValid("loc", mapnode))
					_mapLoc.load(mapnode->first_node("loc"));

				if (nodeValid("clip", mapnode)) {
					rapidxml::xml_node<char> *clipnode = mapnode->first_node("clip");
					loadNum(_mapClip._id, "id", clipnode);
					_mapClip._rect.load(clipnode);
				}
			}

			if (nodeValid("sprites", node)) {
				rapidxml::xml_node<char> *spritenode = node->first_node("sprites");
				int count = 0;
				for (auto n = spritenode->first_node(); n != NULL; n = n->next_sibling(), ++count) {
					Sprite s;
					s.load(n, _animSet);

					Common::String str = n->name();
					if (str == "player") {
						_playerIndex = _objects.size();
						if (playerX != -1 && playerY != -1) {
							s.x(playerX);
							s.y(playerY);
						}
					}

					_objects.push_back(s);
				}
			}

			if (nodeValid("background", node)) {
				rapidxml::xml_node<char> *spritenode = node->first_node("background");
				for (auto n = spritenode->first_node(); n != NULL; n = n->next_sibling()) {
					Sprite s;
					s.load(n, _animSet);
					_background.push_back(s);
				}

				Common::sort(_background.begin(), _background.end(), CompSpriteLayer);
			}

			if (nodeValid("fly", node)) {
				rapidxml::xml_node<char> *spritenode = node->first_node("fly");
				for (auto n = spritenode->first_node(); n != NULL; n = n->next_sibling()) {
					Sprite s;
					s.load(n, _animSet);

					// Set the timer target for the first time
					//s.ai_data.walk.timer.Target(sc_default.fly.delay_min + (gRandom.Num() % sc_default.fly.delay_max));
					s._aiData._walk._timer.Target(_scDefault._fly._delayMax);

					_fly.push_back(s);
				}
			}

			if (nodeValid("movement", node)) {
				rapidxml::xml_node<char> *movnode = node->first_node("movement");
				for (auto n = movnode->first_node("set"); n != NULL; n = n->next_sibling("set"))
					_moveSet.push_back(n);
			}

			if (nodeValid("popup", node, false))
				_pop.load(node->first_node("popup"));

			gameOver.clear();
			if (nodeValid("game_over", node, false))
				gameOver.load(node->first_node("game_over"));
			else if (_playerIndex < _objects.size()) {
				// The default lose condition is the death of the player
				using namespace pyrodactyl::event;
				Trigger t;
				t._type = TRIG_STAT;
				t._target = STATNAME_HEALTH;
				t._subject = _objects[_playerIndex].id();
				t._operation = "<";
				t._val = "1";
				gameOver.add(t);
			}

			calcProperties(info);
		}
	}

	setCamera();
}

//------------------------------------------------------------------------
// Purpose: Build an index of all animation files, called once at start
//------------------------------------------------------------------------
void Level::loadMoves(const Common::String &filename) {
	XMLDoc movList(filename);
	if (movList.ready()) {
		rapidxml::xml_node<char> *node = movList.doc()->first_node("movelist");
		for (auto n = node->first_node("set"); n != NULL; n = n->next_sibling("set")) {
			unsigned int pos = _animSet.size();

			loadNum(pos, "id", n);
			if (pos >= _animSet.size())
				_animSet.resize(pos + 1);

			// See if there is an alternate moveset for low quality setting
			// If no, just load the regular one
			if (!g_engine->_screenSettings->quality) {
				if (!loadStr(_animSet[pos], "path_low", n))
					loadStr(_animSet[pos], "path", n);
			} else
				loadStr(_animSet[pos], "path", n);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Load the default sprite constant parameters
//------------------------------------------------------------------------
void Level::loadConst(const Common::String &filename) {
	XMLDoc doc(filename);
	if (doc.ready()) {
		rapidxml::xml_node<char> *node = doc.doc()->first_node("constant");
		if (nodeValid(node))
			_scDefault.load(node);
	}
}

//------------------------------------------------------------------------
// Purpose: Save all sprite positions to save file
//------------------------------------------------------------------------
void Level::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("player_index", gStrPool->Get(_playerIndex)));

	for (auto &i : _objects) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "sprite");
		i.saveState(doc, child);
		root->append_node(child);
	}
}

//------------------------------------------------------------------------
// Purpose: Load all sprite positions from save file
//------------------------------------------------------------------------
void Level::loadState(rapidxml::xml_node<char> *node) {
	loadNum(_playerIndex, "player_index", node);

	auto i = _objects.begin();
	for (auto *n = node->first_node("sprite"); n != NULL && i != _objects.end(); n = n->next_sibling("sprite"), ++i)
		i->loadState(n);
}

} // End of namespace Crab
