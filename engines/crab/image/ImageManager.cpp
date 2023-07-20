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
#include "crab/crab.h"
#include "crab/image/ImageManager.h"
#include "crab/XMLDoc.h"

namespace Crab {

using namespace pyrodactyl::image;

//------------------------------------------------------------------------
// Purpose: Load assets here.
//------------------------------------------------------------------------
void ImageManager::loadMap(const Common::String &filename, const MapID &mapid) {
	for (auto it = _map[mapid].begin(); it != _map[mapid].end(); ++it)
		it->_value.deleteImage();

	_map[mapid].clear();
	XMLDoc imageList(filename);
	if (imageList.ready()) {
		rapidxml::xml_node<char> *node = imageList.doc()->first_node("res");
		for (auto n = node->first_node("image"); n != NULL; n = n->next_sibling("image")) {
			ImageKey key;
			if (loadImgKey(key, "name", n)) {
				// Load different images depending on image quality setting
				// Check if there is a low quality image specified for the asset id
				// if yes, load it  - if no, just load the higher quality one

				bool valid = false;
				Common::String path;

				if (!g_engine->_screenSettings->_quality)
					valid = loadStr(path, "path_low", n, false);

				if (!valid)
					valid = loadStr(path, "path", n, false);

				if (valid)
					_map[mapid][key].load(path);
				else
					error("ImageManager::loadMap : Unable to load image id %u from %s!", key, path.c_str());
#if 0
					fprintf(stderr, "Unable to load image id %u from %s! SDL Error: %s\n", key, path.c_str(), SDL_GetError());
#endif
			}
		}

		if (nodeValid("mouse", node, false)) {
			using namespace pyrodactyl::input;
			g_engine->_mouse->quit();
			g_engine->_mouse->load(node->first_node("mouse"));
		}
	}
}

bool ImageManager::init() {
	// First, delete everything that exists
	quit();

	// Load common assets
	loadMap(g_engine->_filePath->_common, MAP_COMMON);

	// Load main menu assets
	loadMap(g_engine->_filePath->_currentR, MAP_CURRENT);

	_invalidImg = _map[MAP_COMMON][0];

	return true;
}

//------------------------------------------------------------------------
// Purpose: Add texture to image map
//------------------------------------------------------------------------
void ImageManager::addTexture(const ImageKey &id, Graphics::Surface *surface, int mapindex) {
	if (_map[mapindex].contains(id))
		freeTexture(id, mapindex);

	_map[mapindex][id].load(surface);
#if 0
	SDL_FreeSurface(surface);
#endif
}

//------------------------------------------------------------------------
// Purpose: Get texture for a particular id
//------------------------------------------------------------------------
void ImageManager::getTexture(const ImageKey &id, Image &data) {
	if (_map[MAP_CURRENT].contains(id))
		data = _map[MAP_CURRENT][id];
	else if (_map[MAP_COMMON].contains(id))
		data = _map[MAP_COMMON][id];
	else
		data = _invalidImg;
}

Image &ImageManager::getTexture(const ImageKey &id) {
	if (_map[MAP_CURRENT].contains(id))
		return _map[MAP_CURRENT][id];
	else if (_map[MAP_COMMON].contains(id))
		return _map[MAP_COMMON][id];

	return _invalidImg;
}

bool ImageManager::validTexture(const ImageKey &id) {
	if (id != 0 && (_map[MAP_CURRENT].contains(id) || _map[MAP_COMMON].contains(id)))
		return true;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ImageManager::draw(const int &x, const int &y, const ImageKey &id, Common::Rect *clip,
						const TextureFlipType &flip) {
	getTexture(id).draw(x, y, clip, flip);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ImageManager::draw(const int &x, const int &y, const ImageKey &id, Rect *clip,
						const TextureFlipType &flip) {
	getTexture(id).draw(x, y, clip, flip);
}

//------------------------------------------------------------------------
// Purpose: Dim the screen by drawing a 128 alpha black rectangle over it
//------------------------------------------------------------------------
void ImageManager::dimScreen() {
	warning("STUB: ImageManger::DimScreen()");

#if 0
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 128);
	SDL_RenderFillRect(gRenderer, NULL);
#endif
}

void ImageManager::blackScreen() {
	warning("STUB: ImageManger::blackScreen()");

#if 0
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
	SDL_RenderFillRect(gRenderer, NULL);
#endif
}
//------------------------------------------------------------------------
// Purpose: free resources
//------------------------------------------------------------------------
void ImageManager::quit() {
	for (int i = 0; i < MAP_TOTAL; i++) {
		for (auto it = _map[i].begin(); it != _map[i].end(); ++it)
			it->_value.deleteImage();

		_map[i].clear();
	}
}

} // End of namespace Crab
