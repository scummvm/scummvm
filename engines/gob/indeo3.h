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

// A simple octree for quickly finding the best matching palette entry
class PalOctree {
public:
	PalOctree(const byte *palette, byte depth);
	~PalOctree();

	byte findNearest(byte c1, byte c2, byte c3) const;
	byte findNearest(byte c1, byte c2, byte c3, byte &nc1, byte &nc2, byte &nc3) const;

private:
	struct Node {
		byte index;
		byte comp[3];
		struct Node *children[8];

		Node(byte i, byte c1, byte c2, byte c3) : index(i) {
			memset(children, 0, 8 * sizeof(struct Node *));
			comp[0] = c1;
			comp[1] = c2;
			comp[2] = c3;
		}
		~Node() {
			for (int i = 0; i < 8; i++)
				delete children[i];
		}
	};

	Node *_root;
	byte _depth;

	void build(const byte *palette);
};

// The Sierra-2-4A ("Filter Light") dithering algorithm
class SierraLite {
public:
	SierraLite(int16 width, int16 height);
	~SierraLite();

	void setPalTree(const PalOctree *palTree);

	void newFrame();
	void nextLine();
	byte dither(byte c1, byte c2, byte c3, uint32 x);

protected:
	int16 _width, _height;

	const PalOctree *_palTree;

	int32 *_errorBuf;
	int32 *_errors[2];
	int _curLine;

	inline void getErrors(uint32 x, int32 &eC1, int32 &eC2, int32 &eC3);
	inline void addErrors(uint32 x, int32 eC1, int32 eC2, int32 eC3);
};

// Ordered ditherer with a 8x8 mask
class Ordered8x8 {
public:
	inline static byte dither(byte c1, byte c2, byte c3, const PalOctree *palTree, uint32 x, uint32 y) {
		c1 = CLIP<int>(c1 + map[x % 8][y % 8], 0, 255);
		c2 = CLIP<int>(c2 + map[x % 8][y % 8], 0, 255);
		c3 = CLIP<int>(c3 + map[x % 8][y % 8], 0, 255);
		return palTree->findNearest(c1, c2, c3);
	}

private:
	static const int map[8][8];
};

class Indeo3 {
public:
	enum DitherAlgorithm {
		kDitherNone = 0,
		kDitherSierraLite,
		kDitherOrdered8x8
	};

	Indeo3(int16 width, int16 height);
	~Indeo3();

	static bool isIndeo3(byte *data, uint32 dataLen);

	void setPalette(const byte *palette);
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

	PalOctree *_palTree;

	DitherAlgorithm _dither;
	SierraLite *_ditherSL;

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

	inline void YUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b);
	inline void RGB2YUV(byte r, byte g, byte b, byte &y, byte &u, byte &v);
};

} // End of namespace Gob

#endif // GOB_INDEO3_H
