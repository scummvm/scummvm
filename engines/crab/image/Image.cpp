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

#include "crab/crab.h"
#include "crab/image/Image.h"
#include "image/png.h"

using ImageDecoder = Image::PNGDecoder;

namespace Crab {

using namespace pyrodactyl::image;
//------------------------------------------------------------------------
// Purpose: Load a texture
//------------------------------------------------------------------------
bool Image::Load(Graphics::Surface *surface) {
	Delete();

	texture->create(surface->w, surface->h, surface->format);
	texture->copyFrom(*surface);
	w = surface->w;
	h = surface->h;
	return true;
#if 0
	// Create texture from surface pixels
	texture = SDL_CreateTextureFromSurface(gRenderer, surface);
	if (texture == NULL) {
		fprintf(stderr, "Unable to create texture from SDL_Surface! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Get image dimensions
	w = surface->w;
	h = surface->h;
	return true;
#endif
}

bool Image::Load(Graphics::ManagedSurface *surface) {
	Delete();
	texture = new Graphics::ManagedSurface(*surface);
	w = surface->w;
	h = surface->h;
	return true;
#if 0
	// Create texture from surface pixels
	texture = SDL_CreateTextureFromSurface(gRenderer, surface);
	if (texture == NULL) {
		fprintf(stderr, "Unable to create texture from SDL_Surface! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Get image dimensions
	w = surface->w;
	h = surface->h;
	return true;
#endif
}

bool Image::Load(const Common::String &path) {
	// Get rid of preexisting texture
	Delete();

	// Load image at specified path
	Common::File file;
	ImageDecoder decoder;

	if (FileOpen(path, &file) && decoder.loadStream(file)) {
		texture = new Graphics::ManagedSurface(decoder.getSurface());
		w = texture->w;
		h = texture->h;

		file.close();

		warning("Image::Load() Image Texture(%s): w: %d h: %d", path.c_str(), w, h);

	}

	return texture != nullptr;

#if 0
	SDL_Surface *loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL) {
		fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	} else {
		// Create texture from surface pixels
		texture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);

		if (texture == NULL)
			fprintf(stderr, "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		else {
			// Get image dimensions
			w = loadedSurface->w;
			h = loadedSurface->h;
		}

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	// Return success
	return texture != NULL;
#endif
}

bool Image::Load(rapidxml::xml_node<char> *node, const char *name) {
	if (node->first_attribute(name) != NULL)
		return Load(node->first_attribute(name)->value());

	return false;
}

//------------------------------------------------------------------------
// Purpose: Draw a texture to the screen without cropping
//------------------------------------------------------------------------
void Image::Draw(const int &x, const int &y, Common::Rect *clip, const TextureFlipType &flip) {

	Common::Rect srcRect;
	if (clip != NULL) {
		srcRect = *clip;
	} else {
		srcRect = Common::Rect{static_cast<int16>(x), static_cast<int16>(y), static_cast<int16>(w + x), static_cast<int16>(h + y)};
	}

	Common::Rect destRect {static_cast<int16>(x), static_cast<int16>(y), static_cast<int16>(w + x), static_cast<int16>(h + y)};
	if (clip != NULL) {
		destRect.right = clip->right;
		destRect.bottom = clip->bottom;
	}

	// TODO: Do proper clipping
	g_engine->_screen->blitFrom(*texture, Common::Point(static_cast<int16>(x), static_cast<int16>(y)));
	//g_engine->_renderSurface->blitFrom(*texture, Common::Point(x, y));
	//g_engine->_renderSurface->copyRectToSurface(texture->getPixels(), texture->pitch, x, y, texture->w, texture->h);
#if 0
	// Set rendering space and render to screen
	SDL_Rect renderQuad = {x, y, w, h};

	// Set clip rendering dimensions
	if (clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	switch (flip) {
	case FLIP_NONE:
		// Render to screen
		SDL_RenderCopy(gRenderer, texture, clip, &renderQuad);
		break;
	case FLIP_X:
		// Bottom right <-> Bottom left
		// Top right <-> Top left
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 0, NULL, SDL_FLIP_HORIZONTAL);
		break;
	case FLIP_Y:
		// Top right <-> Bottom right
		// Top left <-> Bottom left
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 0, NULL, SDL_FLIP_VERTICAL);
		break;
	case FLIP_XY:
		// Top right <-> Bottom left
		// Top left <-> Bottom right
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 180, NULL, SDL_FLIP_NONE);
		break;
	case FLIP_D:
		// Top left <-> Bottom right
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 270, NULL, SDL_FLIP_HORIZONTAL);
		break;
	case FLIP_DX:
		// Rotate 90 degrees clockwise
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 90, NULL, SDL_FLIP_NONE);
		break;
	case FLIP_DY:
		// Rotate 270 degrees clockwise
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 270, NULL, SDL_FLIP_NONE);
		break;
	case FLIP_XYD:
		// This is just flipping across the diagonal, instead of the anti-diagonal
		// Top right <-> Bottom left
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 270, NULL, SDL_FLIP_VERTICAL);
		break;
	default:
		break;
	}
#endif

}

void Image::Draw(const int &x, const int &y, Rect *clip, const TextureFlipType &flip) {
	Common::Rect srcRect {0, 0, static_cast<int16>(w + 0), static_cast<int16>(h + 0)};
	Common::Rect destRect {static_cast<int16>(x), static_cast<int16>(y), static_cast<int16>(w + x), static_cast<int16>(h + y)};

	if (clip) {
		srcRect = {static_cast<int16>(clip->x), static_cast<int16>(clip->y), static_cast<int16>(clip->x + clip->w), static_cast<int16>(clip->y + clip->h)};
		destRect.right = static_cast<int16>(clip->w + x);
		destRect.bottom = static_cast<int16>(clip->h + y);
	}

	Graphics::Surface *s = new Graphics::Surface();
	s->copyFrom(texture->getSubArea(srcRect));

	switch(flip) {
		case FLIP_NONE:
		break;

		case FLIP_X:
		s->flipHorizontal(Common::Rect(s->w, s->h));
		break;

		default:
		warning("Flipped texture: %d", flip);
	}

	g_engine->_screen->blitFrom(s, Common::Rect(s->w, s->h), destRect);
	//g_engine->_renderSurface->blitFrom(s, Common::Rect(s->w, s->h), destRect);
}

//------------------------------------------------------------------------
// Purpose: Delete texture data
//------------------------------------------------------------------------
void Image::Delete() {
	if (texture != nullptr) {
		texture->free();
		delete texture;
		texture = nullptr;
		w = 0;
		h = 0;
	}
}

} // End of namespace Crab
