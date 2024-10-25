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

#include "crab/Rectangle.h"
#include "crab/rapidxml/rapidxml.hpp"

namespace Graphics {
class ManagedSurface;
struct Surface;
} // End of namespace Graphics

namespace Common {
struct Rect;
} // End of namespace Common

namespace Crab {

namespace pyrodactyl {
namespace image {
// Image data used in Asset Manager
class Image {
	// The dimensions of the image
	int _w, _h;

	enum ImageRotateDegrees {
		kImageRotateBy90,
		kImageRotateBy180,
		kImageRotateBy270
	};

public:
	Graphics::ManagedSurface *_texture;

	Image() : _texture(nullptr), _w(0), _h(0) {}
	~Image() {}

	Graphics::Surface* rotate(const Graphics::ManagedSurface &src, ImageRotateDegrees rotation);

	// Set color modulation
	void color(const uint8 &r, const uint8 &g, const uint8 &b) {
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
	int alpha(const uint8 &alpha) {
#if 0
		return SDL_SetTextureAlphaMod(texture, alpha);
#endif
		debug(5, "Setting alpha modulation for texture: %d ", alpha);
		return 0;
	}

	// Get alpha modulation
	uint8 alpha() {
		uint8 res = 255;
#if 0
		SDL_GetTextureAlphaMod(texture, &res);
#endif
		return res;
	}

	// Load the image
	bool load(const Common::Path &path);
	bool load(rapidxml::xml_node<char> *node, const char *name);
	bool load(Graphics::ManagedSurface *surface);


	// Draw the texture
	void draw(const int &x, const int &y, Common::Rect *clip = nullptr, const TextureFlipType &flip = FLIP_NONE);
	void draw(const int &x, const int &y, Rect *clip, const TextureFlipType &flip = FLIP_NONE, Graphics::ManagedSurface *surf = nullptr);
	void fastDraw(const int &x, const int &y, Rect *clip = nullptr);

	// Delete the texture
	void deleteImage();

	int w() {
		return _w;
	}

	int h() {
		return _h;
	}

	bool valid() {
		return _texture != nullptr;
	}
};
} // End of namespace image
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_IMAGE_H
