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
 * $URL$
 * $Id$
 *
 */

#include "common/scummsys.h"

#ifdef USE_INDEO3

/* Intel Indeo 3 decompressor, derived from ffmpeg.
 *
 * Original copyright note:
 * Intel Indeo 3 (IV31, IV32, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef GRAPHICS_VIDEO_INDEO3_H
#define GRAPHICS_VIDEO_INDEO3_H

#include "common/stream.h"

namespace Graphics {
	class PaletteLUT;
	class SierraLight;
}

namespace Graphics {

class Indeo3 {
public:
	enum DitherAlgorithm {
		kDitherNone = 0,
		kDitherSierraLight
	};

	Indeo3(int16 width, int16 height, Graphics::PaletteLUT *palLUT);
	~Indeo3();

	static bool isIndeo3(byte *data, uint32 dataLen);

	void setDither(DitherAlgorithm dither);

	bool decompressFrame(byte *inData, uint32 dataLen,
			byte *outData, uint16 width, uint16 height);

private:
	static const int _corrector_type_0[24];
	static const int _corrector_type_2[8];
	static const uint32 correction[];
	static const uint32 correctionloworder[];
	static const uint32 correctionhighorder[];

	struct YUVBufs {
		byte *Ybuf;
		byte *Ubuf;
		byte *Vbuf;
		byte *the_buf;
		uint32 the_buf_size;
		uint16 y_w, y_h;
		uint16 uv_w, uv_h;
	};

	int16 _width;
	int16 _height;
	YUVBufs _iv_frame[2];
	YUVBufs *_cur_frame;
	YUVBufs *_ref_frame;

	byte *_ModPred;
	uint16 *_corrector_type;

	Graphics::PaletteLUT *_palLUT;

	DitherAlgorithm _dither;
	Graphics::SierraLight *_ditherSL;

	struct BlitState {
		uint32 curX, curY;
		uint16 widthY,  widthUV;
		uint16 heightY, heightUV;
		uint16 uwidthUV,  uwidthOut;
		uint16 uheightUV, uheightOut;
		uint16 scaleWYUV, scaleWYOut;
		uint16 scaleHYUV, scaleHYOut;
		uint16 lineWidthOut, lineHeightOut;
		byte *bufY, *bufU, *bufV, *bufOut;
	};

	void buildModPred();
	void allocFrames();

	void decodeChunk(byte *cur, byte *ref, int width, int height,
			const byte *buf1, uint32 fflags2, const byte *hdr,
			const byte *buf2, int min_width_160);

	void blitFrame(BlitState &s);

	void blitLine(BlitState &s);
	void blitLineDither(BlitState &s);
};

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_INDEO3_H

#endif // USE_INDEO3
