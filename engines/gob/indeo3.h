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

/* Intel Indeo 3 decompressor, derived from ffmpeg.
 *
 * Original copyright note:
 * Intel Indeo 3 (IV31, IV32, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef GOB_INDEO3_H
#define GOB_INDEO3_H

namespace Gob {

class PaletteLUT {
public:
	enum PaletteFormat {
		kPaletteRGB,
		kPaletteYUV
	};

	inline static void YUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b) {
		r = CLIP<int>(y + ((1357 * (v - 128)) >> 10), 0, 255);
		g = CLIP<int>(y - (( 691 * (v - 128)) >> 10) - ((333 * (u - 128)) >> 10), 0, 255);
		b = CLIP<int>(y + ((1715 * (u - 128)) >> 10), 0, 255);
	}
	inline static void RGB2YUV(byte r, byte g, byte b, byte &y, byte &u, byte &v) {
		y = CLIP<int>( ((r * 306) >> 10) + ((g * 601) >> 10) + ((b * 117) >> 10)      , 0, 255);
		u = CLIP<int>(-((r * 172) >> 10) - ((g * 340) >> 10) + ((b * 512) >> 10) + 128, 0, 255);
		v = CLIP<int>( ((r * 512) >> 10) - ((g * 429) >> 10) - ((b *  83) >> 10) + 128, 0, 255);
	}

	PaletteLUT(byte depth, PaletteFormat format);
	~PaletteLUT();

	void setPalette(const byte *palette, PaletteFormat format, byte depth);

	void buildNext();

	void getEntry(byte index, byte &c1, byte &c2, byte &c3) const;
	byte findNearest(byte c1, byte c2, byte c3);
	byte findNearest(byte c1, byte c2, byte c3, byte &nC1, byte &nC2, byte &nC3);

private:
	byte _depth1, _depth2;
	byte _shift;

	int _dim1, _dim2, _dim3;

	PaletteFormat _format;
	byte _lutPal[768];
	byte _realPal[768];

	int _got;
	byte *_gots;
	byte *_lut;

	void build(int d1);
	inline int getIndex(byte c1, byte c2, byte c3) const;
	inline void plotEntry(int x, int y, int z, byte e, byte *filled, int &free);
};

// The Sierra-2-4A ("Filter Light") dithering algorithm
class SierraLight {
public:
	SierraLight(int16 width, int16 height, PaletteLUT *palLUT);
	~SierraLight();

	void newFrame();
	void nextLine();
	byte dither(byte c1, byte c2, byte c3, uint32 x);

protected:
	int16 _width, _height;

	PaletteLUT *_palLUT;

	int32 *_errorBuf;
	int32 *_errors[2];
	int _curLine;

	inline void getErrors(uint32 x, int32 &eC1, int32 &eC2, int32 &eC3);
	inline void addErrors(uint32 x, int32 eC1, int32 eC2, int32 eC3);
};

class Indeo3 {
public:
	enum DitherAlgorithm {
		kDitherNone = 0,
		kDitherSierraLight
	};

	Indeo3(int16 width, int16 height, PaletteLUT *palLUT);
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

	PaletteLUT *_palLUT;

	DitherAlgorithm _dither;
	SierraLight *_ditherSL;

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

} // End of namespace Gob

#endif // GOB_INDEO3_H
