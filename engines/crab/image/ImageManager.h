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

//=============================================================================
// Author:   Arvind
// Purpose:  Contains the image manager class - used to manage in-game assets
//=============================================================================
#ifndef CRAB_IMAGEMANAGER_H
#define CRAB_IMAGEMANAGER_H

#include "crab/GameParam.h"
#include "crab/image/Image.h"
//#include "crab/LoadingScreen.h"
#include "crab/TMX/TMXTileSet.h"
#include "crab/common_header.h"
#include "crab/input/cursor.h"
#include "crab/loaders.h"

namespace Crab {

// We use this object as the key for all image assets
typedef unsigned int ImageKey;

// Since we use unsigned int as a key for images, our LoadImgKey function is LoadNum
#define LoadImgKey LoadNum

namespace pyrodactyl {
namespace image {
// We store images here
typedef Common::HashMap<ImageKey, Image> TextureMap;

// Two image maps are used in the game - current (changes with level) and common
enum MapID {
	MAP_CURRENT,
	MAP_COMMON,
	MAP_TOTAL
};

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
	void LoadMap(const Common::String &filename, const MapID &mapid = MAP_CURRENT);

	void AddTexture(const ImageKey &id, Graphics::Surface *surface, int mapindex = MAP_COMMON);
	void FreeTexture(const ImageKey &id, int mapindex = MAP_COMMON) { map[mapindex][id].Delete(); }
	void GetTexture(const ImageKey &id, Image &data);
	Image &GetTexture(const ImageKey &id);
	bool ValidTexture(const ImageKey &id);

	void Draw(const int &x, const int &y, const ImageKey &id,
			  Common::Rect *clip = NULL, const TextureFlipType &flip = FLIP_NONE);

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

} // End of namespace Crab

#endif // CRAB_IMAGEMANAGER_H
