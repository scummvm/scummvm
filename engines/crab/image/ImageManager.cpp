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
// Purpose:  Contains the image manager class - used to manage in-game images
//=============================================================================
#include "crab/image/ImageManager.h"
#include "crab/XMLDoc.h"

namespace Crab {

using namespace pyrodactyl::image;

// Stuff we use throughout the game
namespace pyrodactyl {
namespace image {
ImageManager gImageManager;
}
} // End of namespace pyrodactyl

//------------------------------------------------------------------------
// Purpose: Load assets here.
//------------------------------------------------------------------------
void ImageManager::LoadMap(const Common::String &filename, const MapID &mapid) {
	for (auto it = map[mapid].begin(); it != map[mapid].end(); ++it)
		it->_value.Delete();

	map[mapid].clear();
	XMLDoc image_list(filename);
	if (image_list.ready()) {
		rapidxml::xml_node<char> *node = image_list.Doc()->first_node("res");
		for (auto n = node->first_node("image"); n != NULL; n = n->next_sibling("image")) {
			ImageKey key;
			if (LoadImgKey(key, "name", n)) {
				// Load different images depending on image quality setting
				// Check if there is a low quality image specified for the asset id
				// if yes, load it  - if no, just load the higher quality one

				bool valid = false;
				Common::String path;

				if (!gScreenSettings.quality)
					valid = LoadStr(path, "path_low", n, false);

				if (!valid)
					valid = LoadStr(path, "path", n, false);

				if (valid)
					map[mapid][key].Load(path);
				else
					error("ImageManager::LoadMap : Unable to load image id %u from %s!", key, path.c_str());
#if 0
					fprintf(stderr, "Unable to load image id %u from %s! SDL Error: %s\n", key, path.c_str(), SDL_GetError());
#endif
			}
		}

		if (NodeValid("mouse", node, false)) {
			using namespace pyrodactyl::input;
			gMouse.Quit();
			gMouse.Load(node->first_node("mouse"));
		}
	}
}

bool ImageManager::Init() {
	// First, delete everything that exists
	Quit();

	// Load common assets
	LoadMap(gFilePath.common, MAP_COMMON);

	// Load main menu assets
	LoadMap(gFilePath.current_r, MAP_CURRENT);

	invalid_img = map[MAP_COMMON][0];

	return true;
}

//------------------------------------------------------------------------
// Purpose: Add texture to image map
//------------------------------------------------------------------------
void ImageManager::AddTexture(const ImageKey &id, Graphics::Surface *surface, int mapindex) {
	if (map[mapindex].contains(id))
		FreeTexture(id, mapindex);

	map[mapindex][id].Load(surface);
#if 0
	SDL_FreeSurface(surface);
#endif
}

//------------------------------------------------------------------------
// Purpose: Get texture for a particular id
//------------------------------------------------------------------------
void ImageManager::GetTexture(const ImageKey &id, Image &data) {
	if (map[MAP_CURRENT].contains(id))
		data = map[MAP_CURRENT][id];
	else if (map[MAP_COMMON].contains(id))
		data = map[MAP_COMMON][id];
	else
		data = invalid_img;
}

Image &ImageManager::GetTexture(const ImageKey &id) {
	if (map[MAP_CURRENT].contains(id))
		return map[MAP_CURRENT][id];
	else if (map[MAP_COMMON].contains(id))
		return map[MAP_COMMON][id];

	return invalid_img;
}

bool ImageManager::ValidTexture(const ImageKey &id) {
	if (id != 0 && (map[MAP_CURRENT].contains(id) || map[MAP_COMMON].contains(id)))
		return true;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ImageManager::Draw(const int &x, const int &y, const ImageKey &id, Common::Rect *clip,
						const TextureFlipType &flip) {
	GetTexture(id).Draw(x, y, clip, flip);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ImageManager::Draw(const int &x, const int &y, const ImageKey &id, Rect *clip,
						const TextureFlipType &flip) {
	GetTexture(id).Draw(x, y, clip, flip);
}

//------------------------------------------------------------------------
// Purpose: Dim the screen by drawing a 128 alpha black rectangle over it
//------------------------------------------------------------------------
void ImageManager::DimScreen() {
	warning("STUB: ImageManger::DimScreen()");

#if 0
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 128);
	SDL_RenderFillRect(gRenderer, NULL);
#endif
}

void ImageManager::BlackScreen() {
	warning("STUB: ImageManger::BlackScreen()");

#if 0
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
	SDL_RenderFillRect(gRenderer, NULL);
#endif
}
//------------------------------------------------------------------------
// Purpose: free resources
//------------------------------------------------------------------------
void ImageManager::Quit() {
	for (int i = 0; i < MAP_TOTAL; i++) {
		for (auto it = map[i].begin(); it != map[i].end(); ++it)
			it->_value.Delete();

		map[i].clear();
	}

	invalid_img.Delete();
}

} // End of namespace Crab
