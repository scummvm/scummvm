/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_GFX_IMAGELOADER_H
#define ULTIMA4_GFX_IMAGELOADER_H

#include "ultima/shared/std/containers.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Ultima {
namespace Ultima4 {

class Image;
class ImageLoader;
class U4FILE;

class ImageLoaders {
private:
	Common::HashMap<Common::String, ImageLoader *> _loaderMap;
public:
	ImageLoaders();
	~ImageLoaders();

	/**
	 * This class method returns the registered concrete subclass
	 * appropriate for loading images of a type given by fileType.
	 */
	ImageLoader *getLoader(const Common::String &fileType);
};

/**
 * The generic image loader interface.  Image loaders should override
 * the load method to load an image from a U4FILE and register
 * themselves with registerLoader.  By convention, the type parameter
 * of load and registerLoader is the standard mime type of the image
 * file (e.g. image/png) or an xu4 specific mime type
 * (e.g. image/x-u4...).
 */
class ImageLoader {
protected:
	/**
	 * Fill in the image pixel data from an uncompressed string of bytes.
	 */
	static void setFromRawData(Image *image, int width, int height, int bpp, const byte *rawData);

	/**
	 * Sets the image from a source image
	 */
	static void setFromSurface(Image *image, const Graphics::ManagedSurface &src);
public:
	ImageLoader() {}
	virtual ~ImageLoader() {}
	virtual Image *load(Common::SeekableReadStream &stream, int width, int height, int bpp) = 0;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
