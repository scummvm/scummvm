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

#include "crab/TMX/TMXMap.h"
#include "crab/text/TextManager.h"
#include "crab/collision.h"

namespace Crab {

using namespace TMX;
using namespace pyrodactyl::image;

bool PropCompare(const MapLayer &l1, const MapLayer &l2) {
	return l1.pos.y + l1.pos.h < l2.pos.y + l2.pos.h;
}

TMXMap::TMXMap() {
	tile_rows = 0;
	tile_cols = 0;
	path_rows = 0;
	path_cols = 0;

	w = 0;
	h = 0;
	sprite_layer = 0;
	grid = nullptr;

	movementCosts.no_walk = 0;
	movementCosts.open = 0;
	movementCosts.stairs = 0;
}

//------------------------------------------------------------------------
// Purpose: Load stuff via a .tmx file set to xml storage (no compression)
//------------------------------------------------------------------------

void TMXMap::Load(const std::string &path, std::string filename) {
	XMLDoc conf(path + filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("map");
		if (NodeValid(node)) {
			LoadNum(tile_rows, "width", node);
			LoadNum(tile_cols, "height", node);
			LoadNum(tile_size.x, "tilewidth", node);
			LoadNum(tile_size.y, "tileheight", node);

			// Pathfinding info load
			path_size.x = 0;
			path_size.y = 0;

			LoadNum(path_size.x, "pathwidth", node);
			LoadNum(path_size.y, "pathheight", node);

			// Load costs...Not sure if this is right. (SZ)
			LoadNum(movementCosts.open, "opencost", node);
			LoadNum(movementCosts.no_walk, "nowalkcost", node);
			LoadNum(movementCosts.stairs, "stairscost", node);

			// if(path_size.x == 0)
			//	path_size.x = tile_size.x;
			// if(path_size.y == 0)
			//	path_size.y = tile_size.y;
			// Testing
			if (path_size.x == 0)
				path_size.x = 40;
			if (path_size.y == 0)
				path_size.y = 40;

			w = tile_rows * tile_size.x;
			h = tile_cols * tile_size.y;

			path_rows = (int)ceil((float)w / (float)path_size.x + .5f); // Adding .5 before casting in order to round up (SZ)
			path_cols = (int)ceil((float)h / (float)path_size.y + .5f);

			gImageManager.tileset.Load(path, node);

			// Reset the layer at which sprites are drawn
			sprite_layer = 0;
			unsigned int layer_count = 0;

			// We need to cycle through all tile and object layers in order to
			// see the level at which the sprites will be drawn
			for (auto groupnode = node->first_node(); groupnode != NULL; groupnode = groupnode->next_sibling()) {
				// Store the name for easy comparison
				std::string name = groupnode->name();

				if (name == "layer" || name == "imagelayer") // Is this a tile or an image layer
				{
					MapLayer l;
					l.Load(path, groupnode);
					l.pos.x *= tile_size.x;
					l.pos.y *= tile_size.y;
					l.pos.w *= tile_size.x;
					l.pos.h *= tile_size.y;

					if (l.type == LAYER_PROP)
						prop.push_back(l);
					else
						layer.push_back(l);

					layer_count++;
				} else if (name == "objectgroup") // Is this an object layer
				{
					std::string group_name;
					LoadStr(group_name, "name", groupnode);
					if (group_name == "exit") {
						for (auto n = groupnode->first_node("object"); n != NULL; n = n->next_sibling("object")) {
							pyrodactyl::level::Exit le(n);
							area_exit.push_back(le);
						}
					} else if (group_name == "walk") {
						auto n = groupnode->first_node("object");
						if (n != NULL)
							area_walk.Load(n, true, "x", "y", "width", "height");
					} else if (group_name == "no_walk") {
						for (auto n = groupnode->first_node("object"); n != NULL; n = n->next_sibling("object")) {
							Shape s;
							s.Load(n);
							area_nowalk.push_back(s);
						}
					} else if (group_name == "trigger") {
						for (auto n = groupnode->first_node("object"); n != NULL; n = n->next_sibling("object")) {
							Shape s;
							s.Load(n);

							unsigned int pos = area_trig.size();
							LoadNum(pos, "name", n);

							if (area_trig.size() <= pos)
								area_trig.resize(pos + 1);

							area_trig.at(pos) = s;
						}
					} else if (group_name == "stairs") {
						for (auto n = groupnode->first_node("object"); n != NULL; n = n->next_sibling("object")) {
							pyrodactyl::level::Stairs s;
							s.Load(n);
							area_stairs.push_back(s);
						}
					} else if (group_name == "music") {
						for (auto n = groupnode->first_node("object"); n != NULL; n = n->next_sibling("object")) {
							pyrodactyl::level::MusicArea ma;
							ma.Load(n);
							area_music.push_back(ma);
						}
					} else if (group_name == "sprites")
						sprite_layer = layer_count;
				}
			}

			// Sort the props in the level according to y axis
			std::sort(prop.begin(), prop.end(), PropCompare);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Use this if you want to load a poly line from tmx
//------------------------------------------------------------------------
// void TMXMap::LoadPath(rapidxml::xml_node<char> *node)
//{
//	int pos = 0;
//	for (auto n = node->first_node("object"); n != NULL; n = n->next_sibling("object"), ++pos)
//	{
//		Vector2i start;
//		start.Load(n);
//
//		rapidxml::xml_node<char> *linenode = n->first_node("polyline");
//		if (linenode != NULL)
//		{
//			std::string points, x, y;
//			LoadStr(points, "points", linenode);
//
//			path.resize(pos + 1);
//			bool comma = false;
//			for (auto i = points.begin(); i != points.end(); ++i)
//			{
//				if (*i == ',') comma = true;
//				else if (*i == ' ')
//				{
//					path.at(pos).push_back(GetPoint(start, x, y));
//					comma = false;
//					x.clear();
//					y.clear();
//				}
//				else if (comma) y.push_back(*i);
//				else x.push_back(*i);
//			}
//			path.at(pos).push_back(GetPoint(start, x, y));
//		}
//	}
//}
//------------------------------------------------------------------------
// Purpose: Convert point from string to vector
//------------------------------------------------------------------------
// const Vector2i TMXMap::GetPoint(const Vector2i &ref, std::string &x, std::string &y)
//{
//	Vector2i v;
//	v.x = ref.x + StringToNumber<int>(x);
//	v.y = ref.y + StringToNumber<int>(y);
//	return v;
//}

//------------------------------------------------------------------------
// Purpose: Clear all data from the level
//------------------------------------------------------------------------
void TMXMap::Reset() {
	gImageManager.tileset.Reset();
	layer.clear();

	area_nowalk.clear();
	area_exit.clear();
	area_trig.clear();
	area_stairs.clear();
	area_music.clear();

	prop.clear();
	sprite_layer = 0;
}

//------------------------------------------------------------------------
// Purpose: Draw functions
//------------------------------------------------------------------------
void TMXMap::DrawDebug(const Rect &camera) {
	using namespace pyrodactyl::text;

	for (auto i = area_trig.begin(); i != area_trig.end(); ++i)
		i->Draw(-camera.x, -camera.y, 0, 0, 254, 254);

	for (auto i = area_exit.begin(); i != area_exit.end(); ++i)
		i->dim.Draw(-camera.x, -camera.y, 0, 254, 254, 254);

	for (auto i = prop.begin(); i != prop.end(); ++i)
		i->pos.Draw(-camera.x, -camera.y, 254, 0, 254, 254);

	for (auto i = area_nowalk.begin(); i != area_nowalk.end(); ++i)
		i->Draw(-camera.x, -camera.y, 254, 0, 0, 254);

	for (auto i = area_music.begin(); i != area_music.end(); ++i)
		i->Draw(-camera.x, -camera.y, 254, 254, 0, 254);

	for (auto i = area_stairs.begin(); i != area_stairs.end(); ++i) {
		i->Draw(-camera.x, -camera.y, 0, 254, 0, 254);
		gTextManager.Draw(i->rect.x - camera.x + 100, i->rect.y - camera.y, NumberToString(i->modifier.x), 0);
		gTextManager.Draw(i->rect.x - camera.x + 200, i->rect.y - camera.y, NumberToString(i->modifier.y), 0);
	}

	// Draw the pathfinding grid (SZ)
	/*for(int x = 0; x < grid->GetDimensions().x; ++x)
	{
	for(int y = 0; y < grid->GetDimensions().y; ++y)
	{
	if(grid->GetNodeAtCoords(x, y)->GetMovementCost() < 0.0f)
	grid->GetNodeAtCoords(x, y)->GetRect().Draw(-camera.x, -camera.y, 0, 0, 0, 254);
	else
	grid->GetNodeAtCoords(x, y)->GetRect().Draw(-camera.x, -camera.y, 200, 200, 0, 254);
	}
	}*/

	for (auto i = layer.begin(); i != layer.end(); ++i)
		i->pos.Draw(-camera.x, -camera.y, 254, 216, 0);

	area_walk.Draw(-camera.x, -camera.y, 254, 254, 254, 254);
	// gTextManager.Draw(0, 200, NumberToString(sprite_layer), 0);

	// Use this if you want to draw poly lines in debug
	/*bool start = true;
	Vector2i prev;
	for (auto i = path.begin(); i != path.end(); ++i)
	{
	for (auto j = i->begin(); j != i->end(); ++j)
	{
	if (start)
	{
	prev = *j;
	start = false;
	}

	DrawLine(prev.x - camera.x, prev.y - camera.y, j->x - camera.x, j->y - camera.y, 0, 0, 0, 254);

	gTextManager.Draw(j->x - camera.x + 100, j->y - camera.y, NumberToString(j->x), 0);
	gTextManager.Draw(j->x - camera.x + 200, j->y - camera.y, NumberToString(j->y), 0);

	prev = *j;
	}
	}*/
}

//------------------------------------------------------------------------
// Purpose: Collision functions
//------------------------------------------------------------------------
void TMXMap::CollideWithNoWalk(const Rect bounding_box, std::list<CollisionData> &colliders) {
	CollisionData res;
	for (auto i = area_nowalk.begin(); i != area_nowalk.end(); ++i) {
		res = i->Collide(bounding_box);
		if (res.intersect)
			colliders.push_back(res);
	}
}

bool TMXMap::InsideNoWalk(const Vector2i &pos) {
	for (auto i = area_nowalk.begin(); i != area_nowalk.end(); ++i)
		if (i->Contains(pos))
			return true;

	return false;
}

bool TMXMap::InsideWalk(const Rect bounding_box) {
	if (area_walk.Contains(bounding_box))
		return true;

	return false;
}

bool TMXMap::InsideWalk(const Vector2i &pos) {
	if (area_walk.Contains(pos))
		return true;

	return false;
}

bool TMXMap::CollideWithTrigger(const Rect rect, int index) {
	if (area_trig.size() > index)
		return area_trig.at(index).Collide(rect).intersect;

	return false;
}

void TMXMap::CollideWithTrigger(const Rect rect, std::vector<int> &collision_table) {
	int index = 0;
	collision_table.clear();

	for (auto i = area_trig.begin(); i != area_trig.end(); ++i, ++index)
		if (i->Collide(rect).intersect)
			collision_table.push_back(index);
}

bool TMXMap::CollideWithExit(const Rect rect, LevelResult &res) {
	for (auto i = area_exit.begin(); i != area_exit.end(); ++i)
		if (i->dim.Collide(rect).intersect) {
			res.val = i->name;
			res.x = i->entry.x;
			res.y = i->entry.y;
			return true;
		}

	return false;
}

bool TMXMap::CollideWithStairs(const Rect rect, Vector2f &vel_mod) {
	for (auto i = area_stairs.begin(); i != area_stairs.end(); ++i) {
		if (i->Collide(rect).intersect) {
			vel_mod = i->modifier;
			return true;
		}
	}

	// We are not colliding with any stairs, reset the modifier
	vel_mod.x = 1.0f;
	vel_mod.y = 1.0f;
	return false;
}

bool TMXMap::CollideWithMusic(const Rect rect, pyrodactyl::level::MusicInfo &music) {
	for (auto i = area_music.begin(); i != area_music.end(); ++i) {
		if (i->Collide(rect).intersect) {
			music.id = i->id;
			music.track = i->track;
			music.loops = i->loops;
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Path finding functions
//------------------------------------------------------------------------

} // End of namespace Crab
