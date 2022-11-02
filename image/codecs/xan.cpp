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
 */

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

#include "image/codecs/xan.h"

#include "common/stream.h"
#include "common/bitstream.h"
#include "common/memstream.h"
#include "common/util.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"

namespace Image {

static const int SCRATCH_SPARE = 256;

XanDecoder::XanDecoder(int width, int height, int bitsPerPixel) : Codec(),
		_width(width), _height(height), _wc4Mode(false) {
	assert(bitsPerPixel == 16);
	if (bitsPerPixel != 16)
		error("XanDecoder: BPP must be 16 not %d", bitsPerPixel);
	if (width % 2)
		error("XanDecoder: width must be even, not %d", width);
	_surface.create(_width, _height, getPixelFormat());
	_scratchbuf = new uint8[_width * _height + SCRATCH_SPARE]();
	_lumabuf = new uint8[_width * _height]();
	_ybuf = new uint8[_width * _height]();
	_ubuf = new uint8[_width * _height / 2]();
	_vbuf = new uint8[_width * _height / 2]();
}

XanDecoder::~XanDecoder() {
	_surface.free();
	delete [] _scratchbuf;
	delete [] _lumabuf;
	delete [] _ybuf;
	delete [] _ubuf;
	delete [] _vbuf;
}

const Graphics::Surface *XanDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	uint32 frametype = stream.readUint32LE();
	if (frametype > 1) {
		error("Xan frametype should be 0 or 1, got %d", frametype);
	}

	if (frametype == 0) {
		decodeFrameType0(stream);
	} else {
		decodeFrameType1(stream);
	}

	return &_surface;
}

// An unoptimized version of the one from libavutil, but works fine
static void _memcpy_backptr(uint8 *dst, int back, int cnt) {
	if (cnt <= 0 || back <= 0)
		return;

	const uint8 *src = &dst[-back];
	if (back == 1) {
		uint8 val = *src;
		memset(dst, val, cnt);
	} else {
		int blocklen = back;
		while (cnt > blocklen) {
			memcpy(dst, src, blocklen);
			dst += blocklen;
			cnt -= blocklen;
			blocklen <<= 1;
		}
		memcpy(dst, src, cnt);
	}
}

// Unpack using the WC3 algorithm
static int _unpack(Common::SeekableReadStream &stream, uint8 *dest, const int dest_len) {
	const uint8 *orig_dest = dest;
	const uint8 *dest_end = dest + dest_len;

	memset(dest, 0, dest_len);

	while (dest < dest_end) {
		if (stream.eos())
			return -1;

		const uint8 opcode = stream.readByte();

		if (opcode < 0xe0) {
			int readsize, copysize, back;
			if ((opcode & 0x80) == 0) {
				readsize = opcode & 3;
				back     = ((opcode & 0x60) << 3) + stream.readByte() + 1;
				copysize = ((opcode & 0x1c) >> 2) + 3;
			} else if ((opcode & 0x40) == 0) {
				const uint8 b = stream.readByte();
				readsize = b >> 6;
				back     = ((b & 0x3f) << 8) + stream.readByte() + 1;
				copysize = (opcode & 0x3f) + 4;
			} else {
				readsize = opcode & 3;
				back     = ((opcode & 0x10) << 12) + stream.readUint16BE() + 1;
				copysize = ((opcode & 0x0c) <<  6) + stream.readByte() + 5;
				if (readsize + copysize > dest_end - dest)
					break;
			}
			if (dest + readsize + copysize > dest_end ||
				dest - orig_dest + readsize < back)
				return -1;
			stream.read(dest, readsize);
			dest += readsize;
			_memcpy_backptr(dest, back, copysize);
			dest += copysize;
		} else {
			bool finish = (opcode >= 0xfc);

			int readsize = finish ? opcode & 3 : ((opcode & 0x1f) * 4) + 4;
			if (dest + readsize > dest_end)
				return -1;
			stream.read(dest, readsize);
			dest += readsize;
			if (finish)
				break;
		}
	}

	return dest - orig_dest;
}


bool XanDecoder::decodeChroma(Common::SeekableReadStream &stream, const int chroma_off) {
	if (!chroma_off)
		return 0;

	if (chroma_off + 4 >= stream.size() - stream.pos()) {
		warning("Invalid chroma block position");
		return false;
	}

	stream.seek(chroma_off + 4);

	const int mode = stream.readUint16LE();
	const int table_start = stream.pos();
	// 2 bytes per table entry
	const int table_size = stream.readUint16LE() * 2;

	if (mode > 1) {
		warning("Unexpected chroma mode %d", mode);
		return false;
	}

	if (table_size >= stream.size() - stream.pos()) {
		warning("Invalid chroma block offset");
		return false;
	}

	stream.skip(table_size);
	const int dec_size = _unpack(stream, _scratchbuf, _width * _height);
	if (dec_size < 0) {
		warning("Chroma unpacking failed");
		return false;
	}

	const int pitch = _width / 2;

	uint8 *U = _ubuf;
	uint8 *V = _vbuf;
	const uint8 *src = _scratchbuf;
	const uint8 *src_end = src + dec_size;
	if (mode) {
		// YUV420 frame
		for (int y = 0; y < _height / 2; y++) {
			for (int x = 0; x < pitch; x++) {
				if (src >= src_end)
					return true;
				int toff = *src++ * 2;
				if (toff) {
					if (toff > table_size)
						return false;
					const int pos = stream.pos();
					stream.seek(table_start + toff);
					uint8 uval, vval;
					if (_wc4Mode) {
						uint16 val = stream.readUint16LE();
						uval = (val >> 3) & 0xF8;
						vval = (val >> 8) & 0xF8;
					} else {
						uval = stream.readByte();
						vval = stream.readByte();
					}
					uval = uval | uval >> 5;
					vval = vval | vval >> 5;
					stream.seek(pos);
					U[x] = uval;
					V[x] = vval;
				}
			}
			U += pitch;
			V += pitch;
		}
		if (_height % 1) {
			memcpy(U, U - pitch, pitch);
			memcpy(V, V - pitch, pitch);
		}
	} else {
		// YUV410 frame - expand out U and V components
		uint8 *U2 = U + pitch;
		uint8 *V2 = V + pitch;

		for (int y = 0; y < _height / 4; y++) {
			for (int x = 0; x < pitch; x += 2) {
				if (src >= src_end)
					return true;
				int toff = *src++ * 2;
				if (toff) {
					if (toff > table_size)
						return false;
					const int pos = stream.pos();
					stream.seek(table_start + toff);
					uint8 uval, vval;
					if (_wc4Mode) {
						uint16 val = stream.readUint16LE();
						uval = (val >> 3) & 0xF8;
						vval = (val >> 8) & 0xF8;
					} else {
						uval = stream.readByte();
						vval = stream.readByte();
					}
					uval = uval | uval >> 5;
					vval = vval | vval >> 5;
					stream.seek(pos);
					U[x] = U[x + 1] = U2[x] = U2[x + 1] = uval;
					V[x] = V[x + 1] = V2[x] = V2[x + 1] = vval;
				}
			}
			U  += pitch * 2;
			V  += pitch * 2;
			U2 += pitch * 2;
			V2 += pitch * 2;
		}
		if (_height % 4) {
			int lines = ((_height + 1) / 2) - (_height / 4) * 2;

			memcpy(U, U - lines * pitch, lines * pitch);
			memcpy(V, V - lines * pitch, lines * pitch);
		}
	}

	return true;
}

void XanDecoder::decodeFrameType0(Common::SeekableReadStream &stream) {
	const uint32 chroma_offset = stream.readUint32LE();
	const uint32 refines_offset = stream.readUint32LE();
	const uint32 luma_offset = stream.pos();
	const uint32 nbytes = static_cast<uint32>(stream.size());

	if (chroma_offset > nbytes || refines_offset > nbytes) {
		error("invalid frame type 0 offsets");
	}

	if (!decodeChroma(stream, chroma_offset)) {
		warning("xxan chrome decode failed frame type 0");
		return;
	}

	stream.seek(luma_offset);
	decompressLuma(stream);

	//
	// Expand out the decompressed luma data. For type 0 frames:
	// * luma vals are 5-bit diffs, where
	//   * top row values are diffs on the last value
	//   * and the remaining rows are deltas on the value above.
	// * every second pixel in x is linearly interpolated from its horizontal neighbours.
	// * output values are clipped to 6 bits.
	// * a set of refinements values can adjust luma of interpolated pixels
	//
	const uint8 *lumadecomp = _scratchbuf;
	uint8 *lumarow = _lumabuf;
	int last = *lumadecomp++;
	lumarow[0] = last * 2;
	int x;
	// The top row uses only the left value for prediction
	for (x = 1; x < _surface.w - 1; x += 2) {
		int cur = (last + *lumadecomp++) & 0x1F;
		lumarow[x] = last + cur;
		lumarow[x + 1] = cur * 2;
		last = cur;
	}
	lumarow[x] = last * 2;
	uint8 const *last_lumarow = lumarow;
	lumarow += _surface.w;

	// The remaining rows
	for (int y = 1; y < _surface.h; y++) {
		last = ((last_lumarow[0] / 2) + *lumadecomp++) & 0x1F;
		lumarow[0] = last * 2;
		for (x = 1; x < _surface.w - 1; x += 2) {
			int cur = ((last_lumarow[x + 1] / 2) + *lumadecomp++) & 0x1F;
			lumarow[x] = last + cur;
			lumarow[x + 1] = cur * 2;
			last = cur;
		}
		lumarow[x] = last * 2;
		last_lumarow = lumarow;
		lumarow += _surface.w;
	}

	if (refines_offset) {
		stream.seek(refines_offset + 8);

		int dec_size = _unpack(stream, _scratchbuf, _width * _height / 2);
		if (dec_size < 0) {
			warning("luma refine unpacking failed!");
			dec_size = 0;
		} else {
			dec_size = MIN(dec_size, _width * _height / 2 - 1);
		}

		for (int i = 0; i < dec_size; i++)
			_lumabuf[i * 2 + 1] = (_lumabuf[i * 2 + 1] + _scratchbuf[i] * 2) & 0x3F;
	}

	convertYUVtoRGBSurface();
}

void XanDecoder::decodeFrameType1(Common::SeekableReadStream &stream) {
	const uint32 chroma_offset = stream.readUint32LE();
	const uint32 refines_offset = stream.readUint32LE();
	const uint32 refine2_offset = stream.readUint32LE();
	const uint32 luma_offset = stream.pos();
	const uint32 nbytes = static_cast<uint32>(stream.size());

	if (chroma_offset > nbytes || refines_offset > nbytes || refine2_offset > nbytes) {
		error("invalid frame type 1 offset");
	}

	if (!decodeChroma(stream, chroma_offset)) {
		warning("xxan chrome decode failed frame type 1");
		return;
	}

	stream.seek(luma_offset);
	decompressLuma(stream);

	//
	// Expand out the decompressed luma data. For type 1 frames:
	// * luma vals are 5-bit diffs on the previous frame's values
	// * every second pixel in x is linearly interpolated from its horizontal neighbours.
	// * output values are clipped to 6 bits.
	// * a set of refinements values can adjust luma of interpolated pixels
	//
	const uint8 *lumadecomp = _scratchbuf;
	uint8 *lumarow = _lumabuf;
	for (int y = 0; y < _surface.h; y++) {
		int x;
		int last = (lumarow[0] + (*lumadecomp++ * 2)) & 0x3F;
		lumarow[0] = last;
		for (x = 1; x < _surface.w - 1; x += 2) {
			int cur = (lumarow[x + 1] + (*lumadecomp++ * 2)) & 0x3F;
			lumarow[x] = (last + cur) / 2;
			lumarow[x + 1] = cur;
			last = cur;
		}
		lumarow[x] = last;
		lumarow += _surface.w;
	}

	if (refines_offset) {
		stream.seek(refines_offset + 8);

		int dec_size = _unpack(stream, _scratchbuf, _width * _height / 2);

		if (dec_size < 0) {
			warning("luma refine unpacking failed!");
			dec_size = 0;
		} else {
			dec_size = MIN(dec_size, _width * _height / 2 - 1);
		}

		int dec2_size = 0;
		uint8 *scratch2 = _scratchbuf + _width * _height / 2;
		if (refine2_offset) {
			stream.seek(refine2_offset + 8);
			dec2_size = _unpack(stream, scratch2, _width * _height / 2);
			if (dec2_size < 0) {
				warning("luma refine2 unpacking failed!");
				dec2_size = 0;
			} else {
				dec2_size = MIN(dec_size, _width * _height / 2 - 1);
			}
		}

		for (int i = 0; i < dec_size; i++) {
			int adjust = _scratchbuf[i] * 2;
			if (dec2_size)
				adjust += scratch2[i] * 2;
			_lumabuf[i * 2 + 1] = (_lumabuf[i * 2 + 1] + adjust) & 0x3F;
		}
	}

	convertYUVtoRGBSurface();
}

void XanDecoder::decompressLuma(Common::SeekableReadStream &stream) {
	const int32 startpos = stream.pos();
	const int nsymbols = stream.readByte();
	const int eofsymbol = stream.readByte();

	const int root = nsymbols + eofsymbol;
	const uint8 *lumaend = _scratchbuf + _surface.w * _surface.h;

	stream.skip(nsymbols * 2);

	uint8 *luma = _scratchbuf;
	int node = root;
	int bits = stream.readByte();
	int mask = 0x80;
	while (!stream.eos()) {
		const int bit = ((bits & mask) ? 1 : 0);
		mask >>= 1;

		const int32 nextbitspos = stream.pos();
		stream.seek(startpos + node * 2 + bit - eofsymbol * 2);
		node = stream.readByte();
		stream.seek(nextbitspos);

		if (node == eofsymbol)
			break;
		if (node < eofsymbol) {
			*luma++ = node;
			if (luma >= lumaend)
				break;
			node = root;
		}
		if (!mask) {
			if (stream.eos())
				break;
			bits = stream.readByte();
			mask = 0x80;
		}
	}
}

void XanDecoder::convertYUVtoRGBSurface() {
	// Expand luma from 6-bit to 8-bit.
	for (int i = 0; i < _width * _height; i++)
		_ybuf[i] = _lumabuf[i] << 2 | _lumabuf[i] >> 4;

	YUVToRGBMan.convert420(&_surface, Graphics::YUVToRGBManager::kScaleFull,
						   _ybuf, _ubuf, _vbuf, _width, (_height / 2) * 2, _width, _width / 2);
}

Graphics::PixelFormat XanDecoder::getPixelFormat() const {
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
}

} // End of namespace Image
