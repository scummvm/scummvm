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

#include "graphics/image-archive.h"
#include "graphics/surface.h"

#include "common/archive.h"
#include "common/compression/unzip.h"

#include "image/png.h"

namespace Graphics {

ImageArchive::~ImageArchive() {
	reset();

	delete _imageArchive;
}

void ImageArchive::reset() {
#ifdef USE_PNG
	for (auto &i : _imageCache)
		delete i._value;
	_imageCache.clear();
#endif
}

bool ImageArchive::setImageArchive(const Common::Path &fname) {
	reset();

	delete _imageArchive;
	_imageArchive = Common::makeZipArchive(fname);

	if (!_imageArchive)
		warning("ImageArchive::setImageArchive(): Could not find %s. Images will not be rendered", fname.toString().c_str());
	return _imageArchive != nullptr;
}

const Surface *ImageArchive::getImageSurface(const Common::Path &fname, int w, int h) {
#ifndef USE_PNG
	warning("ImageArchive::getImageSurface(): PNG support not compiled. Cannot load file %s", fname.toString().c_str());

	return nullptr;
#else
	if (_imageCache.contains(fname)) {
		// TODO: Handle the case where a previously loaded image hasn't been scaled.
		const Surface *surf = _imageCache[fname];
		if (surf && surf->w == w && surf->h == h)
			return surf;
	}

	if (!_imageArchive) {
		warning("ImageArchive::getImageSurface(): Image Archive was not loaded. Use setImageArchive()");
		return _imageCache[fname];
	}

	Common::SeekableReadStream *stream = _imageArchive->createReadStreamForMember(fname);

	if (!stream) {
		warning("ImageArchive::getImageSurface(): Cannot open file %s", fname.toString().c_str());
		return _imageCache[fname];
	}

	Image::PNGDecoder decoder;
	if (!decoder.loadStream(*stream)) {
		warning("ImageArchive::getImageSurface(): Cannot load file %s", fname.toString().c_str());

		return _imageCache[fname];
	}

	if (_imageCache.contains(fname)) {
		delete _imageCache[fname];
		_imageCache.erase(fname);
	}

	const Graphics::Surface *surf = decoder.getSurface();
	_imageCache[fname] = surf->scale(w ? w : surf->w, h ? h : surf->h, true);
	return _imageCache[fname];
#endif // USE_PNG
}

} // End of namespace Graphics

