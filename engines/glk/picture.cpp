/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/picture.h"
#include "glk/glk.h"
#include "glk/raw_decoder.h"
#include "glk/screen.h"
#include "common/file.h"
#include "image/jpeg.h"
#include "image/png.h"

namespace Glk {

Pictures::Pictures() : _refCount(0) {
	Common::File f;
	if (f.open("apal")) {
		while (f.pos() < f.size())
			_adaptivePics.push_back(
				Common::String::format("%u", f.readUint32BE()));
	}
}

void Pictures::clear() {
	for (uint idx = 0; idx < _store.size(); ++idx) {
		if (_store[idx]._picture)
			_store[idx]._picture->decrement();
		if (_store[idx]._scaled)
			_store[idx]._scaled->decrement();
	}

	_store.clear();
}

void Pictures::increment() {
	++_refCount;
}

void Pictures::decrement() {
	if (_refCount > 0 && --_refCount == 0)
		clear();
}

PictureEntry *Pictures::search(const Common::String &name) {
	Picture *pic;

	for (uint idx = 0; idx < _store.size(); ++idx) {
		pic = _store[idx]._picture;

		if (pic && pic->_name.equalsIgnoreCase(name))
			return &_store[idx];
	}

	return nullptr;
}

void Pictures::storeOriginal(Picture *pic) {
	PictureEntry newPic;
	newPic._picture = pic;

	_store.push_back(newPic);
}

void Pictures::storeScaled(Picture *pic) {
	PictureEntry *entry = search(pic->_name);
	if (!entry)
		return;

	delete entry->_scaled;
	entry->_scaled = pic;
}

void Pictures::store(Picture *pic) {
	if (!pic)
		return;

	if (!pic->_scaled)
		storeOriginal(pic);
	else
		storeScaled(pic);
}

Picture *Pictures::retrieve(const Common::String &name, bool scaled) {
	Picture *pic;

	for (uint idx = 0; idx < _store.size(); ++idx) {
		pic = scaled ? _store[idx]._scaled : _store[idx]._picture;

		if (pic && pic->_name.equalsIgnoreCase(name))
			return pic;
	}

	return nullptr;
}

Picture *Pictures::load(const Common::String &name) {
	::Image::PNGDecoder png;
	::Image::JPEGDecoder jpg;
	Graphics::Surface rectImg;
	RawDecoder raw;
	const Graphics::Surface *img;
	const byte *palette = nullptr;
	uint palCount = 0;
	int transColor = -1;
	Picture *pic;

	// Check if the picture is already in the store
	pic = retrieve(name, false);
	if (pic)
		return pic;

	Common::File f;
	if ((name.hasSuffixIgnoreCase(".png") && f.open(name))
		|| f.open(Common::String::format("pic%s.png", name.c_str()))
		|| f.open(Common::String::format("%s.png", name.c_str()))
	) {
		png.setKeepTransparencyPaletted(true);
		png.loadStream(f);
		img = png.getSurface();
		palette = png.getPalette();
		palCount = png.getPaletteColorCount();
		transColor = png.getTransparentColor();
	} else if (
		((name.hasSuffixIgnoreCase(".jpg") || name.hasSuffixIgnoreCase(".jpeg")) && f.open(name))
		|| f.open(Common::String::format("pic%s.jpg", name.c_str()))
		|| f.open(Common::String::format("pic%s.jpeg", name.c_str()))
		|| f.open(Common::String::format("%s.jpg", name.c_str()))
	) {
		jpg.setOutputPixelFormat(g_system->getScreenFormat());
		jpg.loadStream(f);
		img = jpg.getSurface();
	} else if ((name.hasSuffixIgnoreCase(".raw") && f.open(name)) ||
			f.open(Common::String::format("pic%s.raw", name.c_str()))) {
		raw.loadStream(f);
		img = raw.getSurface();
		palette = raw.getPalette();
		palCount = raw.getPaletteColorCount();
		transColor = raw.getTransparentColor();
	} else if (f.open(Common::String::format("pic%s.rect", name.c_str()))) {
		rectImg.w = f.readUint32BE();
		rectImg.h = f.readUint32BE();
		img = &rectImg;
	} else {
		// No such picture
		return nullptr;
	}

	// Also check if it's going to be an adaptive pic
	bool isAdaptive = false;
	for (uint idx = 0; idx < _adaptivePics.size() && !isAdaptive; ++idx)
		isAdaptive = _adaptivePics[idx].equalsIgnoreCase(name);

	if (isAdaptive) {
		// It is, so used previously saved palette
		assert(!_savedPalette.empty());
		palette = &_savedPalette[0];
		palCount = _savedPalette.size() / 3;
	} else if (palette) {
		// It's a picture with a valid palette, so save a copy of it for later
		_savedPalette.resize(palCount * 3);
		Common::copy(palette, palette + palCount * 3, &_savedPalette[0]);
	}

	// Create new picture based on the image
	pic = new Picture(img->w, img->h, g_system->getScreenFormat());
	pic->_refCount = 1;
    pic->_name = name;
    pic->_scaled = false;
	if (transColor != -1 || (!palette && img->format.aBits() > 0))
		pic->clear(pic->getTransparentColor());

	if (!img->getPixels()) {
		// Area definition without any content
	} else if (!palette) {
		pic->blitFrom(*img);
	} else {
		uint pal[256];
		for (uint idx = 0; idx < palCount; ++idx)
			pal[idx] = pic->format.RGBToColor(palette[idx * 3],
				palette[idx * 3 + 1], palette[idx * 3 + 2]);
		
		const byte *srcP = (const byte *)img->getPixels();
		byte *destP = (byte *)pic->getPixels();
		for (int idx = 0; idx < img->w * img->h; ++idx, srcP++, destP += pic->format.bytesPerPixel) {
			if ((int)*srcP != transColor) {
				uint val = (*srcP >= palCount) ? 0 : pal[*srcP];
				if (pic->format.bytesPerPixel == 2)
					WRITE_LE_UINT16(destP, val);
				else
					WRITE_LE_UINT32(destP, val);
			}
		}
	}

    store(pic);
    return pic;
}

Picture *Pictures::scale(Picture *src, size_t sx, size_t sy) {
	// Check for the presence of an already scaled version of that size
	Picture *dst = retrieve(src->_name, true);
	if (dst && dst->w == sx && dst->h == sy)
		return dst;

	// Create a new picture of the destination size and rescale the source picture
	dst = new Picture(sx, sy, src->format);
	dst->_name = src->_name;
	dst->_scaled = true;
	dst->transBlitFrom(*src, src->getBounds(), dst->getBounds(), (uint)0x8888);

	storeScaled(dst);
	return dst;
}

/*--------------------------------------------------------------------------*/

Picture::Picture(int width, int height, const Graphics::PixelFormat &fmt) :
		Graphics::ManagedSurface(width, height, fmt), _refCount(0), _scaled(false) {

	// Default transparent color chosen at random
	_transColor = format.RGBToColor(0x77, 0x77, 0x77);
}

void Picture::increment() {
	++_refCount;
}

void Picture::decrement() {
	if (_refCount > 0 && --_refCount == 0) {
		// No longer any references to this picture, so remove it
		delete this;
	}
}

void Picture::drawPicture(const Common::Point &destPos, const Common::Rect &box) {
	Graphics::ManagedSurface s(*g_vm->_screen, box);
	Common::Point pt(destPos.x - box.left, destPos.y - box.top);

	s.transBlitFrom(*this, pt, _transColor);
}

} // End of namespace Glk
