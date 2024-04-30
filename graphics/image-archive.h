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

#ifndef GRAPHICS_IMAGE_ARCHIVE_H
#define GRAPHICS_IMAGE_ARCHIVE_H

#include "common/hashmap.h"
#include "common/path.h"

namespace Common {
class Archive;
}

namespace Graphics {
struct Surface;

/**
 * Helper class for loading PNG images from zip files.
 */
class ImageArchive {
public:
	~ImageArchive();

	/* Frees all previously loaded images. */
	void reset();

	/* Open a new zip archive, after closing the previous one. */
	bool setImageArchive(const Common::Path &fname);

	/* Retrieve an image from the cache, or load it from the archive if it hasn't been loaded previously. */
	const Surface *getImageSurface(const Common::Path &fname) {
		return getImageSurface(fname, 0, 0);
	}
	const Surface *getImageSurface(const Common::Path &fname, int w, int h);

private:
#ifdef USE_PNG
	Common::HashMap<Common::Path, Surface *, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> _imageCache;
#endif
	Common::Archive *_imageArchive = nullptr;
};

} // End of namespace Graphics

#endif
