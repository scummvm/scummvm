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

#ifndef MADS_CORE_IMAGE_H
#define MADS_CORE_IMAGE_H

#include "common/stream.h"

namespace MADS {

struct Image {
	int16  flags;         /* refers to the index of the corresponding frame in the anim struct */
	/* ^^ only true for images which have been anim_loaded...            */
	/* anim_view dumps these images into lists using flags to tell       */
	/* when they should be put; the flags then become actual flags       */
	/* like IMAGE_INCOMING, IMAGE_STATIC, or about to be erased, or ...  */
	byte segment_id;    /* i'm a part of this segment                                        */
	byte series_id;     /* this is my series                                                 */
	int16  sprite_id;     /* sprite within the series                                          */
	int16  x, y;
	byte depth;
	byte scale;

	static constexpr size_t SIZE = 2 + 1 + 1 + 2 + 2 + 2 + 1 + 1;
	void load(Common::SeekableReadStream *src);

	bool equals(const Image &rhs) const {
		return series_id == rhs.series_id && sprite_id == rhs.sprite_id &&
			x == rhs.x && y == rhs.y && depth == rhs.depth && scale == rhs.scale;
	}
};

typedef Image *ImagePtr;

} // namespace MADS

#endif
