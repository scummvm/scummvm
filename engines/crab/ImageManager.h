//=============================================================================
// Author:   Arvind
// Purpose:  Contains the image manager class - used to manage in-game assets
//=============================================================================
#pragma once

#include "GameParam.h"
#include "Image.h"
#include "LoadingScreen.h"
#include "TMXTileSet.h"
#include "common_header.h"
#include "cursor.h"
#include "loaders.h"

// We use this object as the key for all image assets
typedef unsigned int ImageKey;

// Since we use unsigned int as a key for images, our LoadImgKey function is LoadNum
#define LoadImgKey LoadNum

namespace pyrodactyl {
namespace image {
// We store images here
typedef std::unordered_map<ImageKey, Image> TextureMap;

// Two image maps are used in the game - current (changes with level) and common
enum MapID { MAP_CURRENT,
			 MAP_COMMON,
			 MAP_TOTAL };

class ImageManager {
	// Assets are stored in images
	// Common is stuff used everywhere - this is only loaded once
	TextureMap map[MAP_TOTAL];

	// The default image for all invalid image names
	Image invalid_img;

public:
	// The tile sets used in the level
	TMX::TileSetGroup tileset;

	// This image is used to notify player about changes to quests and inventory
	ImageKey notify;

	ImageManager() { notify = 0; }
	~ImageManager() {}

	void Quit();

	bool Init();
	void LoadPaths();

	// image related stuff

	// Load all images specified in an xml file in a map
	void LoadMap(const std::string &filename, const MapID &mapid = MAP_CURRENT);

	void AddTexture(const ImageKey &id, SDL_Surface *surface, int mapindex = MAP_COMMON);
	void FreeTexture(const ImageKey &id, int mapindex = MAP_COMMON) { map[mapindex][id].Delete(); }
	void GetTexture(const ImageKey &id, Image &data);
	Image &GetTexture(const ImageKey &id);
	bool ValidTexture(const ImageKey &id);

	void Draw(const int &x, const int &y, const ImageKey &id,
			  Rect *clip = NULL, const TextureFlipType &flip = FLIP_NONE);

	void DimScreen();
	void BlackScreen();

	// Draw the notification icon
	void NotifyDraw(const int &x, const int &y) {
		auto *k = &GetTexture(notify);
		Draw(x - k->W() / 2, y - k->H() / 2, notify);
	}
};

extern ImageManager gImageManager;
} // End of namespace image
} // End of namespace pyrodactyl