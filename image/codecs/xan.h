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

#ifndef IMAGE_CODECS_XAN_H
#define IMAGE_CODECS_XAN_H

#include "image/codecs/codec.h"

namespace Image {

/**
 * Xan image decoder. (fourcc Xxan)
 *
 * Used by Crusader: No Regret AVI files
 *
 * This code was created based on the multimedia wiki:
 * https://wiki.multimedia.cx/index.php/Origin_Xan_Codec
 * and ffmpeg's libavcodec/xxan.c.
 * The ffmpeg code is LGPL2 licensed and Copyright (C) 2011
 * Konstantin Shishkov based on work by Mike Melanson.
 *
 * A similar format is used in Wing Commander III (although not in an AVI
 * container) and IV.
 */
class XanDecoder : public Codec {
public:
	XanDecoder (int width, int height, int bitsPerPixel);
	~XanDecoder();

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override;

private:
	void decodeFrameType0(Common::SeekableReadStream &stream);
	void decodeFrameType1(Common::SeekableReadStream &stream);

	/** Decompress the huffman table for base luma data */
	void decompressLuma(Common::SeekableReadStream &stream);

	bool decodeChroma(Common::SeekableReadStream &stream, int chroma_off);

	/** convert the internally expanded YUV to the output RGBA surface */
	void convertYUVtoRGBSurface();

	/** A buffer to hold the final frame in RGBA */
	Graphics::Surface _surface;

	/** Dest surface width and height */
	int _width, _height;

	/** If true, decode chroma vals in Wing Commander 4 style (false = No Regret style) */
	bool _wc4Mode;

	/** A buffer to hold scratch data.  Either chroma data in progress, or
	 * decompressed delta luma values (5-bit).  Interpretation depends on frame type. */
	uint8 *_scratchbuf;
	/** A buffer to hold expanded/interpolated absolute luma values (6-bit) from the values in _scratchbuf.
	 * These still need to be multiplied out to make 8-bit values. */
	uint8 *_lumabuf;
	/** a buffer for uncompressed and multiplied out "y" values (of yuv) */
	uint8 *_ybuf;
	/** a buffer for uncompressed "u" values (of yuv) */
	uint8 *_ubuf;
	/** a buffer for uncompressed "v" values (of yuv) */
	uint8 *_vbuf;
};

} // End of namespace Image

#endif
