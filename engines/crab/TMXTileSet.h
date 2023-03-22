#pragma once

#include "Image.h"
#include "TMXLayer.h"
#include "common_header.h"

namespace TMX {
struct TileSet {
	// The name of the tileset
	std::string name;

	// The location of the tileset image on the disk
	std::string loc;

	// The first gid of the tileset
	int first_gid;

	// Dimensions of tiles
	int tile_w, tile_h;

	// Number of rows and columns of tiles
	int total_rows, total_cols;

	// The image used by the tileset
	pyrodactyl::image::Image img;

	// Stuff used to store temporary data

	// The rectangle used to store clip info
	Rect clip;

	void Init() {
		first_gid = 1;
		tile_w = 1;
		tile_h = 1;
		total_rows = 1;
		total_cols = 1;
	}

	TileSet() { Init(); }
	TileSet(const std::string &path, rapidxml::xml_node<char> *node) {
		Init();
		Load(path, node);
	}

	void Load(const std::string &path, rapidxml::xml_node<char> *node);
	void Draw(const Vector2i &pos, const TileInfo &tile);
};

class TileSetGroup {
	std::vector<TileSet> tileset;

	// The latest tile position
	Vector2i v;

	// The area that we have to draw
	Vector2i start, finish;

public:
	TileSetGroup() {}

	void Reset();

	void Load(const std::string &path, rapidxml::xml_node<char> *node);
	void Draw(MapLayer &layer, const Rect &camera, const Vector2i &tile_size, const Rect &player_pos);
};
}; // End of namespace TMX