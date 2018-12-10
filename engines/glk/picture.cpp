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

void Pictures::clear() {
	for (uint idx = 0; idx < _store.size(); ++idx) {
		_store[idx]._picture->decrement();
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

PictureEntry *Pictures::search(uint id) {
	Picture *pic;

	for (uint idx = 0; idx < _store.size(); ++idx) {
		pic = _store[idx]._picture;

		if (pic && pic->_id == id)
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
	PictureEntry *entry = search(pic->_id);
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

Picture *Pictures::retrieve(uint id, bool scaled) {
	Picture *pic;

	for (uint idx = 0; idx < _store.size(); ++idx) {
		pic = scaled ? _store[idx]._scaled : _store[idx]._picture;

		if (pic && pic->_id == id)
			return pic;
	}

	return nullptr;
}

Picture *Pictures::load(uint32 id) {
	::Image::PNGDecoder png;
	::Image::JPEGDecoder jpg;
	Graphics::Surface rectImg;
	RawDecoder raw;
	const Graphics::Surface *img;
	const byte *palette = nullptr;
	uint palCount = 0;
	Picture *pic;

	// Check if the picture is already in the store
	pic = retrieve(id, false);
	if (pic)
		return pic;

	Common::File f;
	if (f.open(Common::String::format("pic%u.png", id))) {
		png.loadStream(f);
		img = png.getSurface();
		palette = png.getPalette();
		palCount = png.getPaletteColorCount();
	} else if (f.open(Common::String::format("pic%u.jpg", id))) {
		jpg.loadStream(f);
		img = jpg.getSurface();
	} else if (f.open(Common::String::format("pic%u.raw", id))) {
		raw.loadStream(f);
		img = raw.getSurface();
		palette = raw.getPalette();
		palCount = raw.getPaletteColorCount();
	} else if (f.open(Common::String::format("pic%u.rect", id))) {
		rectImg.w = f.readUint16LE();
		rectImg.h = f.readUint16LE();
		img = &rectImg;
	} else {
		// No such picture
		return nullptr;
	}

	pic = new Picture(img->w, img->h, g_system->getScreenFormat());
	pic->_refCount = 1;
    pic->_id = id;
    pic->_scaled = false;

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
			uint val = (*srcP >= palCount) ? 0 : pal[*srcP];
			if (pic->format.bytesPerPixel == 2)
				WRITE_LE_UINT16(destP, val);
			else
				WRITE_LE_UINT32(destP, val);
		}
	}

    store(pic);
    return pic;
}

Picture *Pictures::scale(Picture *src, size_t sx, size_t sy) {
	// Check for the presence of an already scaled version of that size
	Picture *dst = retrieve(src->_id, true);
	if (dst && dst->w == sx && dst->h == sy)
		return dst;

	// Create a new picture of the destination size and rescale the source picture
	dst = new Picture(sx, sy, src->format);
	dst->_id = src->_id;
	dst->_scaled = true;
	dst->transBlitFrom(*src, src->getBounds(), dst->getBounds(), (uint)-1);

	storeScaled(dst);
	return dst;
}

/*--------------------------------------------------------------------------*/

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
	Graphics::Surface s = g_vm->_screen->getSubArea(box);
	s.copyRectToSurface(*this, destPos.x - box.left, destPos.y, getBounds());
}

} // End of namespace Glk
