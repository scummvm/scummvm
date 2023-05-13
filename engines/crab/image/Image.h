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

#ifndef CRAB_IMAGE_H
#define CRAB_IMAGE_H

#include "image/png.h"
#include "crab/ScreenSettings.h"
#include "crab/filesystem.h"
#include "crab/Rectangle.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace image {
// Image data used in Asset Manager
class Image {
	// The dimensions of the image
	int w, h;
	int id;
	TextureFlipType _flip;

	// The actual hardware texture
	//SDL_Texture *texture;
	Graphics::ManagedSurface *texture;

public:
	Image() : texture(nullptr), w(0), h(0) {}
	~Image() {}

	// Set color modulation
	void Color(const uint8 &r, const uint8 &g, const uint8 &b) {
#if 0
		SDL_SetTextureColorMod(texture, r, g, b);
#endif
		warning("Setting color modulation for texture: %d %d %d", r, g, b);
	}

	// Set blending
#if 0
	void BlendMode(const SDL_BlendMode &mode) { SDL_SetTextureBlendMode(texture, mode); }
#endif

	// Set alpha modulation
	int Alpha(const uint8 &alpha) {
#if 0
		return SDL_SetTextureAlphaMod(texture, alpha);
#endif
		warning("Setting alpha modulation for texture: %d ", alpha);
		return 0;
	}

	// Get alpha modulation
	uint8 Alpha() {
		uint8 res = 255;
#if 0
		SDL_GetTextureAlphaMod(texture, &res);
#endif
		return res;
	}

	// Load the image
	bool Load(const Common::String &path);
	bool Load(rapidxml::xml_node<char> *node, const char *name);
	bool Load(Graphics::Surface *surface);
	bool Load(Graphics::ManagedSurface *surface);
	bool Load(const Image &image, Rect *clip, const TextureFlipType &flip);


	// Draw the openGL texture
	void Draw(const int &x, const int &y, Common::Rect *clip = NULL, const TextureFlipType &flip = FLIP_NONE);
	void Draw(const int &x, const int &y, Rect *clip, const TextureFlipType &flip = FLIP_NONE);
	void FastDraw(const int &x, const int &y);

	// Delete the openGL texture
	void Delete();

	int W() { return w; }
	int H() { return h; }
	bool Valid() { return texture != nullptr; }
};
} // End of namespace image
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_IMAGE_H
