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

#include "common/file.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"
#include "image/png.h"
#include "crab/crab.h"
#include "crab/filesystem.h"
#include "crab/image/Image.h"

using ImageDecoder = Image::PNGDecoder;

namespace Crab {

using namespace pyrodactyl::image;
//------------------------------------------------------------------------
// Purpose: Load a texture
//------------------------------------------------------------------------

bool Image::load(Graphics::ManagedSurface *surface) {
	deleteImage();

	_texture = new Graphics::ManagedSurface();
	_texture->copyFrom(*surface);

	_w = surface->w;
	_h = surface->h;

	return true;
}

bool Image::load(const Common::Path &path) {
	// Get rid of preexisting texture
	deleteImage();

	// Load image at specified path
	Common::File file;
	ImageDecoder decoder;

	if (fileOpen(path, &file) && decoder.loadStream(file)) {
		_texture = new Graphics::ManagedSurface();
		_texture->convertFrom(*decoder.getSurface(), *g_engine->_format);
		_w = _texture->w;
		_h = _texture->h;

		file.close();

		debugC(kDebugFilePath, "Image::load() Image Texture(%s): w: %d h: %d", path.toString(Common::Path::kNativeSeparator).c_str(), _w, _h);
	}

	return _texture != nullptr;
}

bool Image::load(rapidxml::xml_node<char> *node, const char *name) {
	if (node->first_attribute(name) != nullptr)
		return load(node->first_attribute(name)->value());

	return false;
}

//------------------------------------------------------------------------
// Purpose: Draw a texture to the screen without cropping
//------------------------------------------------------------------------
void Image::draw(const int &x, const int &y, Common::Rect *clip, const TextureFlipType &flip) {

	Common::Rect srcRect;
	if (clip != nullptr) {
		srcRect = *clip;
	} else {
		srcRect = Common::Rect(x, y, _w + x, _h + y);
	}

	Common::Rect destRect(x, y, _w + x, _h + y);
	if (clip != nullptr) {
		destRect.right = clip->right;
		destRect.bottom = clip->bottom;
	}

	// TODO: Do proper clipping
	g_engine->_screen->blitFrom(*_texture, Common::Point(static_cast<int16>(x), static_cast<int16>(y)));
}

Graphics::Surface* Image::rotate(const Graphics::ManagedSurface &src, ImageRotateDegrees rotation) {
	assert(!src.empty());
	assert(src.w == src.h);
	assert(src.format.bytesPerPixel == 4);

	Graphics::Surface *dest = new Graphics::Surface();
	dest->create(src.w, src.h, src.format);

	const uint size = src.w;
	const uint32 *s = (const uint32 *)src.getBasePtr(0, 0);

	switch (rotation) {
	case kImageRotateBy90:
		for (uint y = 0; y < size; ++y) {
			for (uint x = 0; x < size; ++x, ++s) {
				*((uint32 *)dest->getBasePtr(size - y - 1, x)) = *s;
			}
		}

		break;

	case kImageRotateBy180: {
		// 180 degrees
		uint32 *d;
		for (uint y = 0; y < size; ++y) {
			const uint32 *e = (const uint32 *)src.getBasePtr(size - 1, y);
			d = (uint32 *)dest->getBasePtr(size - 1, size - y - 1);
			for (; s < e; ++s, --d) {
				*d = *s;
			}
		}

		break;
	}

	case kImageRotateBy270:
		for (uint y = 0; y < size; ++y) {
			for (uint x = 0; x < size; ++x, ++s) {
				*((uint32 *)dest->getBasePtr(y, size - x - 1)) = *s;
			}
		}

		break;
	}

	return dest;
}

void Image::draw(const int &x, const int &y, Rect *clip, const TextureFlipType &flip, Graphics::ManagedSurface *surf) {
	if (surf == nullptr)
		surf = g_engine->_screen;

	Common::Rect srcRect(0, 0, _w, _h);
	Common::Rect destRect(x, y, _w + x, _h + y);

	if (clip) {
		srcRect = Common::Rect(clip->x, clip->y, clip->x + clip->w, clip->y + clip->h);
		destRect.right = static_cast<int16>(clip->w + x);
		destRect.bottom = static_cast<int16>(clip->h + y);
	}

	Graphics::ManagedSurface s;
	s.copyFrom(_texture->getSubArea(srcRect));

	if (s.w < 1 || s.h < 1)
		return;

	Graphics::Surface *rotated_surf = nullptr;

	switch(flip) {
	case FLIP_NONE:
		break;

	case FLIP_X:
		s.surfacePtr()->flipHorizontal(Common::Rect(s.w, s.h));
		break;

	case FLIP_Y:
		s.surfacePtr()->flipVertical(Common::Rect(s.w, s.h));
		break;

	case FLIP_XY:
		s.surfacePtr()->flipHorizontal(Common::Rect(s.w, s.h));
		s.surfacePtr()->flipVertical(Common::Rect(s.w, s.h));
		break;

	case FLIP_D:
		s.surfacePtr()->flipHorizontal(Common::Rect(s.w, s.h));
		rotated_surf = rotate(s, kImageRotateBy270);
		s.copyFrom(*rotated_surf);
		delete rotated_surf;
		break;

	case FLIP_DX:
		rotated_surf = rotate(s, kImageRotateBy90);
		s.copyFrom(*rotated_surf);
		delete rotated_surf;
		break;

	case FLIP_DY:
		rotated_surf = rotate(s, kImageRotateBy270);
		s.copyFrom(*rotated_surf);
		delete rotated_surf;
		break;

	case FLIP_XYD:
		s.surfacePtr()->flipVertical(Common::Rect(s.w, s.h));
		rotated_surf = rotate(s, kImageRotateBy270);
		s.copyFrom(*rotated_surf);
		delete rotated_surf;
		break;

	default:
		warning("Flipped texture: %d", flip);
	}

	surf->blitFrom(s, Common::Rect(s.w, s.h), destRect);
}

void Image::fastDraw(const int &x, const int &y, Rect *clip) {
	Common::Rect destRect(x, y, _w + x, _h + y);
	int in_y = 0, in_x = 0;

	if (clip) {
		destRect.setWidth(clip->w);
		destRect.setHeight(clip->h);
		in_x = clip->x;
		in_y = clip->y;
	}

	const int he = destRect.height();
	const int destW = destRect.width();
	uint32 *out = (uint32*)g_engine->_screen->getBasePtr(destRect.left, destRect.top);
	uint32 *in = (uint32*)_texture->getBasePtr(in_x, in_y);

	const uint32 outPitch = g_engine->_screen->pitch / sizeof(uint32);
	const uint32 inPitch = _texture->pitch / sizeof(uint32);

	for (int y_ = 0; y_ < he; y_++) {
		memcpy(out, in, destW * 4);
		out += outPitch;
		in += inPitch;
	}

	g_engine->_screen->addDirtyRect(destRect);
}


//------------------------------------------------------------------------
// Purpose: Delete texture data
//------------------------------------------------------------------------
void Image::deleteImage() {
	if (_texture != nullptr && _w > 0 && _h > 0) {
		_texture->free();
		delete _texture;
		_texture = nullptr;
		_w = 0;
		_h = 0;
	}
}

} // End of namespace Crab
