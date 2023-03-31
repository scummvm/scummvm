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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/TMX/TMXTileSet.h"
#include "crab/text/TextManager.h"

namespace Crab {

using namespace TMX;

void TileSet::Load(const std::string &path, rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		LoadNum(first_gid, "firstgid", node);
		LoadStr(name, "name", node);
		LoadNum(tile_w, "tilewidth", node);
		LoadNum(tile_h, "tileheight", node);

		clip.w = tile_w;
		clip.h = tile_h;

		if (NodeValid("image", node)) {
			rapidxml::xml_node<char> *imgnode = node->first_node("image");
			std::string filename;
			LoadStr(filename, "source", imgnode);
			loc = path + filename;

			img.Load(loc.c_str());
			total_rows = img.H() / tile_h;
			total_cols = img.W() / tile_w;
		}
	}

	// Prevent divide by zero errors later
	if (total_cols == 0)
		total_cols = 1;
}

void TileSetGroup::Reset() {
	for (auto i = tileset.begin(); i != tileset.end(); ++i)
		i->img.Delete();

	tileset.clear();
}

void TileSetGroup::Load(const std::string &path, rapidxml::xml_node<char> *node) {
	Reset();
	for (auto n = node->first_node("tileset"); n != NULL; n = n->next_sibling("tileset")) {
		TileSet t;
		t.Load(path, n);
		tileset.push_back(t);
	}
}

void TileSet::Draw(const Vector2i &pos, const TileInfo &tile) {
	warning("STUB: TileSet::Draw()");

#if 0
	if (tile.gid != 0) {
		clip.x = ((tile.gid - first_gid) % total_cols) * tile_w;
		clip.y = ((tile.gid - first_gid) / total_cols) * tile_h;

		img.Draw(pos.x, pos.y, &clip, tile.flip);

		/*if(tile.flip != FLIP_NONE && GameDebug)
		pyrodactyl::text::gTextManager.Draw(pos.x,pos.y,NumberToString(tile.flip),0);*/
	}
#endif
}

void TileSetGroup::Draw(MapLayer &layer, const Rect &camera, const Vector2i &tile_size, const Rect &player_pos) {
	if (layer.type == LAYER_IMAGE)
		layer.img.Draw(-1.0f * camera.x * layer.rate.x, -1.0f * camera.y * layer.rate.y);
	else if (layer.type == LAYER_PARALLAX) {
		// The row and column we start drawing at
		start.x = 0;
		start.y = 0;

		// The row and column we end drawing at
		finish.x = layer.tile.size() - 1;
		finish.y = finish.y = layer.tile.at(0).size() - 1;

		v.x = (start.y * tile_size.x - camera.x) * layer.rate.x;
		v.y = (start.x * tile_size.y - camera.y) * layer.rate.y;

		for (int x = start.x; x < finish.x; ++x) {
			for (int y = start.y; y < finish.y; ++y) {
				for (int i = tileset.size() - 1; i >= 0; --i)
					if (layer.tile[x][y].gid >= tileset[i].first_gid) {
						tileset[i].Draw(v, layer.tile[x][y]);
						break;
					}

				v.x += tile_size.x;
			}

			v.x = (start.y * tile_size.x - camera.x) * layer.rate.x;
			v.y += tile_size.y;
		}
	} else {
		layer.collide = layer.pos.Collide(player_pos);

		// If player is inside the layer bounds, draw normally - else skip drawing
		if (layer.type == LAYER_AUTOHIDE && !layer.collide)
			return;

		// If the player is outside the layer bounds, draw normally - else skip drawing
		if (layer.type == LAYER_AUTOSHOW && layer.collide)
			return;

		// Normal and prop layers are drawn this way

		// The row and column we start drawing at
		start.x = camera.y / tile_size.y;
		start.y = camera.x / tile_size.x;

		// The row and column we end drawing at
		finish.x = (camera.y + camera.h) / tile_size.y + 1;
		finish.y = (camera.x + camera.w) / tile_size.x + 1;

		if (finish.x > layer.tile.size())
			finish.x = layer.tile.size();
		if (finish.y > layer.tile[0].size())
			finish.y = layer.tile.at(0).size();

		v.x = start.y * tile_size.x - camera.x;
		v.y = start.x * tile_size.y - camera.y;

		for (int x = start.x; x < finish.x; ++x) {
			for (int y = start.y; y < finish.y; ++y) {
				for (int i = tileset.size() - 1; i >= 0; --i)
					if (layer.tile[x][y].gid >= tileset[i].first_gid) {
						tileset[i].Draw(v, layer.tile[x][y]);
						break;
					}

				v.x += tile_size.x;
			}

			v.x = start.y * tile_size.x - camera.x;
			v.y += tile_size.y;
		}
	}
}

} // End of namespace Crab
