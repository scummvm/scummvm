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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VIDEO_CODECS_CODEC_H
#define VIDEO_CODECS_CODEC_H

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Common {
class SeekableReadStream;
}

namespace Video {


// FIXME: Why is this class called "Codec" -- it is not meant to encode,
// only to decode, isn't it? So "Decoder" seems like a better name. 
// Or "StreamDecoder"
// If that rename happens, then this directory should be also renamed from
// "video/codecs" to "video/decoders".

// WTF: This class lacks proper documentation, that's really *really* bad.
// See also comments on Audio::Codec class
class Codec {
public:
	Codec() {}
	virtual ~Codec() {}

// FIXME: THe corresponding method Audio::Codec::decodeFrame takes a reference
// to a stream, this takes a pointer. Unify!

// FIXME: The ownership of the input stream as well as the output surface 
// should be specified!
	virtual const Graphics::Surface *decodeImage(Common::SeekableReadStream *stream) = 0;

// FIXME: What are the semantics for this method? What does it return?
// Maybe the pixelformat of the last decoded surface? If so, can it only be called
// right after a decodeImage call? What happens if it is called before?
// 
// How does this value compare to the pixelformat in the decoded surfaces ?
	virtual Graphics::PixelFormat getPixelFormat() const = 0;

// FIXME: If this palette is for the graphics surface, then maybe it should
// be stored in the surface, resp. in the pixelformat?
// FIXME: Is containsPalette necessary, or is it equivalent to "getPalette() != 0" ?
	virtual bool containsPalette() const { return false; }

// FIXME: Who owns the returned palette?
	virtual const byte *getPalette() { return 0; }
	virtual bool hasDirtyPalette() const { return false; }
};

} // End of namespace Video

#endif
