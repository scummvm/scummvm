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

#ifndef GRAPHICS_THUMBNAIL_H
#define GRAPHICS_THUMBNAIL_H

#include "common/scummsys.h"

namespace Common{
class SeekableReadStream;
class WriteStream;
}

namespace Graphics {

/**
 * @defgroup graphics_thumbnail Thumbnails
 * @ingroup graphics
 *
 * @brief API for managing screen thumbnails used for save games.
 *
 * @{
 */

struct Surface;
class ManagedSurface;

/**
 * Checks for presence of the thumbnail save header.
 * Seeks automatically back to start position after check.
 *
 * @param in	stream to check for header
 */
bool checkThumbnailHeader(Common::SeekableReadStream &in);

/**
 * Skips a thumbnail, if present.
 *
 * @param in	stream to process
 */
bool skipThumbnail(Common::SeekableReadStream &in);

/**
 * Loads a thumbnail from the given input stream.
 */
bool loadThumbnail(Common::SeekableReadStream &in, Graphics::Surface *&thumbnail, bool skipThumbnail = false);

/**
 * Loads a thumbnail from the given input stream.
 */
bool loadThumbnail(Common::SeekableReadStream &in, Graphics::ManagedSurface *&thumbnail, bool skipThumbnail = false);

/**
 * Creates a thumbnail from screen contents.
 */
bool createThumbnail(Graphics::Surface &thumb);

/**
 * Saves a thumbnail to the given write stream.
 * Automatically creates a thumbnail from screen contents.
 */
bool saveThumbnail(Common::WriteStream &out);

/**
 * Saves a (given) thumbnail to the given write stream.
 */
bool saveThumbnail(Common::WriteStream &out, const Graphics::Surface &thumb);

/**
 * Grabs framebuffer into surface
 *
 * @param surf	a surface
 * @return		false if a error occurred
 */
bool createScreenShot(Graphics::Surface &surf);

/** @} */
} // End of namespace Graphics

#endif
