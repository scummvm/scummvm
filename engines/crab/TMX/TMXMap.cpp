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

bool propCompare(const MapLayer &l1, const MapLayer &l2) {
	return l1._pos.y + l1._pos.h < l2._pos.y + l2._pos.h;
}

TMXMap::TMXMap() {
	_tileRows = 0;
	_tileCols = 0;
	_pathRows = 0;
	_pathCols = 0;

	_w = 0;
	_h = 0;
	_spriteLayer = 0;
	_grid = nullptr;

	_movementCosts._noWalk = 0;
	_movementCosts._open = 0;
	_movementCosts._stairs = 0;
}

//------------------------------------------------------------------------
// Purpose: Load stuff via a .tmx file set to xml storage (no compression)
//------------------------------------------------------------------------

void TMXMap::load(const Common::String &path, Common::String filename) {
	XMLDoc conf((path + filename));
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("map");
		if (nodeValid(node)) {
			loadNum(_tileRows, "width", node);
			loadNum(_tileCols, "height", node);
			loadNum(_tileSize.x, "tilewidth", node);
			loadNum(_tileSize.y, "tileheight", node);

			// Pathfinding info load
			_pathSize.x = 0;
			_pathSize.y = 0;

			loadNum(_pathSize.x, "pathwidth", node);
			loadNum(_pathSize.y, "pathheight", node);

			// Load costs...Not sure if this is right. (SZ)
			loadNum(_movementCosts._open, "opencost", node);
			loadNum(_movementCosts._noWalk, "nowalkcost", node);
			loadNum(_movementCosts._stairs, "stairscost", node);

			// if(path_size.x == 0)
			//	path_size.x = tile_size.x;
			// if(path_size.y == 0)
			//	path_size.y = tile_size.y;
			// Testing
			if (_pathSize.x == 0)
				_pathSize.x = 40;
			if (_pathSize.y == 0)
				_pathSize.y = 40;

			_w = _tileRows * _tileSize.x;
			_h = _tileCols * _tileSize.y;

			_pathRows = (int)ceil((float)_w / (float)_pathSize.x + .5f); // Adding .5 before casting in order to round up (SZ)
			_pathCols = (int)ceil((float)_h / (float)_pathSize.y + .5f);

			g_engine->_imageManager->_tileset.load(path, node);

			// Reset the layer at which sprites are drawn
			_spriteLayer = 0;
			uint layerCount = 0;

			// We need to cycle through all tile and object layers in order to
			// see the level at which the sprites will be drawn
			for (auto groupnode = node->first_node(); groupnode != nullptr; groupnode = groupnode->next_sibling()) {
				// Store the name for easy comparison
				Common::String name = groupnode->name();

				if (name == "layer" || name == "imagelayer") { // Is this a tile or an image layer
					MapLayer l;
					l.load(path, groupnode);
					l._pos.x *= _tileSize.x;
					l._pos.y *= _tileSize.y;
					l._pos.w *= _tileSize.x;
					l._pos.h *= _tileSize.y;

					if (l._type == LAYER_PROP)
						_prop.push_back(l);
					else
						_layer.push_back(l);

					layerCount++;
				} else if (name == "objectgroup") { // Is this an object layer
					Common::String groupName;
					loadStr(groupName, "name", groupnode);
					if (groupName == "exit") {
						for (auto n = groupnode->first_node("object"); n != nullptr; n = n->next_sibling("object")) {
							pyrodactyl::level::Exit le(n);
							_areaExit.push_back(le);
						}
					} else if (groupName == "walk") {
						auto n = groupnode->first_node("object");
						if (n != nullptr)
							_areaWalk.load(n, true, "x", "y", "width", "height");
					} else if (groupName == "no_walk") {
						for (auto n = groupnode->first_node("object"); n != nullptr; n = n->next_sibling("object")) {
							Shape s;
							s.load(n);
							_areaNowalk.push_back(s);
						}
					} else if (groupName == "trigger") {
						for (auto n = groupnode->first_node("object"); n != nullptr; n = n->next_sibling("object")) {
							Shape s;
							s.load(n);

							uint pos = _areaTrig.size();
							loadNum(pos, "name", n);

							if (_areaTrig.size() <= pos)
								_areaTrig.resize(pos + 1);

							_areaTrig[pos] = s;
						}
					} else if (groupName == "stairs") {
						for (auto n = groupnode->first_node("object"); n != nullptr; n = n->next_sibling("object")) {
							pyrodactyl::level::Stairs s;
							s.load(n);
							_areaStairs.push_back(s);
						}
					} else if (groupName == "music") {
						for (auto n = groupnode->first_node("object"); n != nullptr; n = n->next_sibling("object")) {
							pyrodactyl::level::MusicArea ma;
							ma.load(n);
							_areaMusic.push_back(ma);
						}
					} else if (groupName == "sprites")
						_spriteLayer = layerCount;
				}
			}

			// Sort the props in the level according to y axis
			Common::sort(_prop.begin(), _prop.end(), propCompare);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Use this if you want to load a poly line from tmx
//------------------------------------------------------------------------
// void TMXMap::LoadPath(rapidxml::xml_node<char> *node)
//{
//	int pos = 0;
//	for (auto n = node->first_node("object"); n != nullptr; n = n->next_sibling("object"), ++pos)
//	{
//		Vector2i start;
//		start.load(n);
//
//		rapidxml::xml_node<char> *linenode = n->first_node("polyline");
//		if (linenode != nullptr)
//		{
//			Common::String points, x, y;
//			loadStr(points, "points", linenode);
//
//			path.resize(pos + 1);
//			bool comma = false;
//			for (auto i = points.begin(); i != points.end(); ++i)
//			{
//				if (*i == ',') comma = true;
//				else if (*i == ' ')
//				{
//					path[pos].push_back(GetPoint(start, x, y));
//					comma = false;
//					x.clear();
//					y.clear();
//				}
//				else if (comma) y.push_back(*i);
//				else x.push_back(*i);
//			}
//			path[pos].push_back(GetPoint(start, x, y));
//		}
//	}
//}
//------------------------------------------------------------------------
// Purpose: Convert point from string to vector
//------------------------------------------------------------------------
// const Vector2i TMXMap::GetPoint(const Vector2i &ref, Common::String &x, Common::String &y)
//{
//	Vector2i v;
//	v.x = ref.x + StringToNumber<int>(x);
//	v.y = ref.y + StringToNumber<int>(y);
//	return v;
//}

//------------------------------------------------------------------------
// Purpose: Clear all data from the level
//------------------------------------------------------------------------
void TMXMap::reset() {
	g_engine->_imageManager->_tileset.reset();
	_layer.clear();

	_areaNowalk.clear();
	_areaExit.clear();
	_areaTrig.clear();
	_areaStairs.clear();
	_areaMusic.clear();

	_prop.clear();
	_spriteLayer = 0;
}

//------------------------------------------------------------------------
// Purpose: Draw functions
//------------------------------------------------------------------------
void TMXMap::drawDebug(const Rect &camera) {
	using namespace pyrodactyl::text;

	for (auto i = _areaTrig.begin(); i != _areaTrig.end(); ++i)
		i->draw(-camera.x, -camera.y, 0, 0, 254, 254);

	for (auto i = _areaExit.begin(); i != _areaExit.end(); ++i)
		i->_dim.draw(-camera.x, -camera.y, 0, 254, 254, 254);

	for (auto i = _prop.begin(); i != _prop.end(); ++i)
		i->_pos.draw(-camera.x, -camera.y, 254, 0, 254, 254);

	for (auto i = _areaNowalk.begin(); i != _areaNowalk.end(); ++i)
		i->draw(-camera.x, -camera.y, 254, 0, 0, 254);

	for (auto i = _areaMusic.begin(); i != _areaMusic.end(); ++i)
		i->draw(-camera.x, -camera.y, 254, 254, 0, 254);

	for (auto i = _areaStairs.begin(); i != _areaStairs.end(); ++i) {
		i->draw(-camera.x, -camera.y, 0, 254, 0, 254);
	}

	// Draw the pathfinding grid (SZ)
	for(int x = 0; x < _grid->getDimensions().x; ++x) {
		for(int y = 0; y < _grid->getDimensions().y; ++y) {
			if(_grid->getNodeAtCoords(x, y)->getMovementCost() < 0.0f)
				_grid->getNodeAtCoords(x, y)->getRect().draw(-camera.x, -camera.y, 0, 0, 0, 254);
		}
	}

	for (auto i = _layer.begin(); i != _layer.end(); ++i)
		i->_pos.draw(-camera.x, -camera.y, 254, 216, 0);

	_areaWalk.draw(-camera.x, -camera.y, 254, 254, 254, 254);
}

//------------------------------------------------------------------------
// Purpose: Collision functions
//------------------------------------------------------------------------
void TMXMap::collideWithNoWalk(const Rect boundingBox, Common::List<CollisionData> &colliders) {
	CollisionData res;
	for (auto i = _areaNowalk.begin(); i != _areaNowalk.end(); ++i) {
		res = i->collide(boundingBox);
		if (res._intersect)
			colliders.push_back(res);
	}
}

bool TMXMap::insideNoWalk(const Vector2i &pos) {
	for (auto i = _areaNowalk.begin(); i != _areaNowalk.end(); ++i)
		if (i->contains(pos))
			return true;

	return false;
}

bool TMXMap::insideWalk(const Rect &boundingBox) {
	if (_areaWalk.contains(boundingBox))
		return true;

	return false;
}

bool TMXMap::insideWalk(const Vector2i &pos) {
	if (_areaWalk.contains(pos))
		return true;

	return false;
}

bool TMXMap::collideWithTrigger(const Rect rect, int index) {
	if (_areaTrig.size() > (uint)index)
		return _areaTrig[index].collide(rect)._intersect;

	return false;
}

void TMXMap::collideWithTrigger(const Rect rect, Common::Array<int> &collisionTable) {
	int index = 0;
	collisionTable.clear();

	for (auto i = _areaTrig.begin(); i != _areaTrig.end(); ++i, ++index)
		if (i->collide(rect)._intersect)
			collisionTable.push_back(index);
}

bool TMXMap::collideWithExit(const Rect rect, LevelResult &res) {
	for (auto i = _areaExit.begin(); i != _areaExit.end(); ++i)
		if (i->_dim.collide(rect)._intersect) {
			res._val = i->_name;
			res._x = i->_entry.x;
			res._y = i->_entry.y;
			return true;
		}

	return false;
}

bool TMXMap::collideWithStairs(const Rect rect, Vector2f &velMod) {
	for (auto i = _areaStairs.begin(); i != _areaStairs.end(); ++i) {
		if (i->collide(rect)._intersect) {
			velMod = i->_modifier;
			return true;
		}
	}

	// We are not colliding with any stairs, reset the modifier
	velMod.x = 1.0f;
	velMod.y = 1.0f;
	return false;
}

bool TMXMap::collideWithMusic(const Rect rect, pyrodactyl::level::MusicInfo &music) {
	for (auto i = _areaMusic.begin(); i != _areaMusic.end(); ++i) {
		if (i->collide(rect)._intersect) {
			music._id = i->_id;
			music._track = i->_track;
			music._loops = i->_loops;
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Path finding functions
//------------------------------------------------------------------------

} // End of namespace Crab
