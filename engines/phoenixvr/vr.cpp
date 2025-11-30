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

#include "phoenixvr/vr.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"
#include "image/bmp.h"
#include "phoenixvr/dct.h"
#include "phoenixvr/dct_tables.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

#define CHUNK_VR (0x12fa84ab)
#define CHUNK_STATIC_2D (0xa0b1c400)
#define CHUNK_STATIC_3D (0xa0b1c200)

namespace {

template<typename T>
T clip(T a) {
	return a >= 0 ? a <= 255 ? a : 255 : 0;
}

struct Quantisation {
	int quantY[64];
	int quantCbCr[64];

	Quantisation(int quality) {
		for (uint i = 0; i != 64; ++i) {
			auto v = (QY[i] * quality + 50) / 100;
			if (v > 255)
				v = 255;
			else if (v < 8) {
				v = 8;
			}
			v *= Q[i];
			v >>= 13;
			quantY[i] = v;
		}
		for (uint i = 0; i != 64; ++i) {
			auto v = (QUV[i] * quality + 50) / 100;
			if (v > 255)
				v = 255;
			else if (v < 8) {
				v = 8;
			}
			v *= Q[i];
			v >>= 13;
			quantCbCr[i] = v;
		}
	}
};

struct HuffChar {
	short next;
	short falseIdx;
	short trueIdx;
};

class BitStream {
	const byte *_data;
	uint _bytePos;
	byte _bitMask;

public:
	BitStream(const byte *data, uint bytePos) : _data(data), _bytePos(bytePos), _bitMask(0x80) {}

	uint getBytePos() const {
		return _bytePos;
	}

	bool readBit() {
		bool bit = _data[_bytePos] & _bitMask;
		_bitMask >>= 1;
		if (_bitMask == 0) {
			_bitMask = 128;
			++_bytePos;
		}
		return bit;
	}

	int readUInt(byte n) {
		int value = 0;
		for (int i = 0; i != n; ++i) {
			if (readBit())
				value |= 1 << i;
		}
		return value;
	}

	int readInt(byte n) {
		int value = readUInt(n);
		if ((value & (1 << (n - 1))) == 0)
			value += 1 - (1 << n);
		return value;
	}

	void alignToByte() {
		if (_bitMask != 0x80) {
			_bitMask = 128;
			++_bytePos;
		}
	}
};

Common::Array<byte> unpackHuffman(const byte *huff, uint huffSize) {
	HuffChar table[514] = {};
	uint offset = 0;
	uint8 codebyte = huff[offset++];
	do {
		uint8 next = huff[offset++];
		if (codebyte <= next) {
			for (auto idx = codebyte; idx <= next; ++idx) {
				table[idx].next = huff[offset++];
			}
		}
		codebyte = huff[offset++];
	} while (codebyte != 0);
	table[256].next = 1;
	table[513].next = 0x7FFF;

	short startEntry;
	short codeIdx = 257, nIdx = 257;
	while (true) {
		short idx = 0, dstIdx = 0;
		short trueIdx = 513, falseIdx = 513;
		short nextLo = 513, nextHi = 513;
		while (idx < nIdx) {
			auto next = table[dstIdx].next;
			if (next != 0) {
				if (next >= table[nextLo].next) {
					if (next < table[nextHi].next) {
						trueIdx = idx;
						nextHi = dstIdx;
					}
				} else {
					trueIdx = falseIdx;
					nextHi = nextLo;
					falseIdx = idx;
					nextLo = dstIdx;
				}
			}
			++idx;
			++dstIdx;
		}
		if (trueIdx == 513) {
			startEntry = nIdx - 1;
			break;
		}
		table[codeIdx].next = table[falseIdx].next + table[trueIdx].next;
		table[falseIdx].next = table[trueIdx].next = 0;
		table[codeIdx].falseIdx = falseIdx;
		table[codeIdx].trueIdx = trueIdx;
		++codeIdx;
		++nIdx;
	}
	Common::Array<byte> decoded;
	decoded.reserve(huffSize);
	{
		BitStream bs(huff, offset);
		while (true) {
			short value = startEntry;
			while (value > 256) {
				auto bit = bs.readBit();
				if (bit)
					value = table[value].trueIdx;
				else
					value = table[value].falseIdx;
			}
			if (value == 256)
				break;
			decoded.push_back(static_cast<byte>(value));
		}
		bs.alignToByte();
		offset = bs.getBytePos();
	}
	debug("decoded %u bytes at %08x", decoded.size(), offset);
	return decoded;
}

void unpack(Graphics::Surface &pic, const byte *huff, uint huffSize, const byte *acPtr, const byte *dcPtr, int quality) {
	Quantisation quant(quality);
	auto decoded = unpackHuffman(huff, huffSize);
	uint decodedOffset = 0;
	static const DCT2DIII<6> dct;

	const uint planePitch = pic.w;
	const uint planeSize = planePitch * pic.h;
	Common::Array<byte> planes(planeSize * 3, 0);

	BitStream acBs(acPtr, 0), dcBs(dcPtr, 0);
	uint channel = 0;
	uint x0 = 0, y0 = 0;
	while (decodedOffset < decoded.size()) {
		float ac[64] = {};
		int8 dc8 = dcBs.readUInt(8);
		auto *iquant = channel ? quant.quantCbCr : quant.quantY;
		ac[0] = 1.0f * iquant[0] * (int)dc8;
		for (uint idx = 1; idx < 64;) {
			auto b = decoded[decodedOffset++];
			if (b == 0x00) {
				break;
			} else if (b == 0xf0) {
				idx += 16;
			} else {
				auto h = b >> 4;
				auto l = b & 0x0f;
				idx += h;
				if (l && idx < 64) {
					auto ac_idx = ZIGZAG[idx];
					ac[ac_idx] = 1.0f * iquant[ac_idx] * acBs.readInt(l);
					++idx;
				}
			}
		}
		float out[64];
		dct.calc(ac, out);
		auto *dst = planes.data() + channel * planeSize + y0 * planePitch + x0;
		const auto *src = out;
		for (unsigned h = 8; h--; dst += planePitch - 8) {
			for (unsigned w = 8; w--;) {
				int v = *src++ / 4 + 128;
				v = clip(v);
				*dst++ = v;
			}
		}
		++channel;
		if (channel == 3) {
			channel = 0;
			x0 += 8;
			if (static_cast<int16>(x0) >= pic.w) {
				x0 = 0;
				y0 += 8;
			}
		}
	}
	auto *yPtr = planes.data();
	auto *crPtr = yPtr + planeSize;
	auto *cbPtr = crPtr + planeSize;
#if 0
	auto &format = pic.format;
	for(int yy = 0; yy < pic.h; ++yy) {
		auto *rows = static_cast<uint32*>(pic.getBasePtr(0, yy));
		for(int xx = 0; xx < pic.w; ++xx) {
			int16 y = *yPtr++;
			int16 cr = (int16)*crPtr++;
			int16 cb = (int16)*cbPtr++;

			int r = y;
			int g = y;
			int b = y;

			*rows++ = format.RGBToColor(r, g, b);
		}
	}
#else
	YUVToRGBMan.convert444(&pic, Graphics::YUVToRGBManager::kScaleFull,
						   yPtr, crPtr, cbPtr, pic.w, pic.h, planePitch, planePitch);
#endif
}
} // namespace

VR::~VR() {
	if (_pic) {
		_pic->free();
		_pic.reset();
	}
}

VR VR::loadStatic(const Graphics::PixelFormat &format, Common::SeekableReadStream &s) {
	VR vr;
	auto magic = s.readUint32LE();
	if (magic != CHUNK_VR) {
		error("wrong VR magic");
	}
	auto fsize = s.readUint32LE();
	debug("file size = %08x", fsize);
	while (s.pos() < fsize) {
		auto chunkId = s.readUint32LE();
		auto chunkSize = s.readUint32LE();
		debug("chunk %08x %u", chunkId, chunkSize);
		bool pic2d = chunkId == CHUNK_STATIC_2D;
		bool pic3d = chunkId == CHUNK_STATIC_3D;
		if (pic2d || pic3d) {
			auto quality = s.readUint32LE();
			auto dataSize = s.readUint32LE();

			Common::Array<byte> vrData(dataSize);
			s.read(vrData.data(), vrData.size());

			auto huffSize = READ_LE_UINT32(vrData.data());
			auto unpHuffSize = READ_LE_UINT32(vrData.data() + 4);
			debug("static picture header, quality: %u, packed data size: %u, huff size: %08x, unpacked huff size: %u", quality, dataSize, huffSize, unpHuffSize);
			if (vr._pic)
				error("2d/3d picture in the same file");
			vr._pic.reset(new Graphics::Surface());
			auto *pic = vr._pic.get();
			if (pic3d) {
				vr._vr = true;
				pic->create(256, 6144, format);
			} else
				pic->create(640, 480, format);
			auto *huff = vrData.data() + 8;
			auto *acPtr = vrData.data() + huffSize + 12;
			auto dcOffset = READ_LE_UINT32(vrData.data() + huffSize + 8);
			auto *dcPtr = vrData.data() + huffSize + 16 + dcOffset;
			unpack(*pic, huff, unpHuffSize, acPtr, dcPtr, quality);
		}
		s.skip(chunkSize - 8);
	}
	if (vr._pic) {
		Common::DumpFile out;
		if (out.open("static.bmp"))
			Image::writeBMP(out, *vr._pic);
	}
	return vr;
}

namespace {

struct Cube {
	float x;
	float y;
	int faceIdx;
};

Cube toCube(float x, float y, float z) {
	Cube cube;

	float absX = fabs(x);
	float absY = fabs(y);
	float absZ = fabs(z);

	bool isXPositive = x > 0;
	bool isYPositive = y > 0;
	bool isZPositive = z > 0;

	float maxAxis, cy, cx;

	if (isXPositive && absX >= absY && absX >= absZ) {
		maxAxis = absX;
		cx = y;
		cy = z;
		cube.faceIdx = 1;
	}
	if (!isXPositive && absX >= absY && absX >= absZ) {
		maxAxis = absX;
		cx = -y;
		cy = z;
		cube.faceIdx = 3;
	}
	if (isYPositive && absY >= absX && absY >= absZ) {
		maxAxis = absY;
		cx = -x;
		cy = z;
		cube.faceIdx = 4;
	}
	if (!isYPositive && absY >= absX && absY >= absZ) {
		maxAxis = absY;
		cx = x;
		cy = z;
		cube.faceIdx = 5;
	}
	if (isZPositive && absZ >= absX && absZ >= absY) {
		maxAxis = absZ;
		cx = -x;
		cy = -y;
		cube.faceIdx = 0;
	}
	if (!isZPositive && absZ >= absX && absZ >= absY) {
		maxAxis = absZ;
		cx = -x;
		cy = y;
		cube.faceIdx = 2;
	}

	// Convert range from −1 to 1 to 0 to 1
	cube.y = 0.5f * (cy / maxAxis + 1.0f);
	cube.x = 0.5f * (cx / maxAxis + 1.0f);
	return cube;
}

struct Projection {
	struct Point {
		float angle;
		float sine, cosine;
	};
	Common::Array<Point> points;

	Projection(uint num, float start, float fov) : points(num) {
		const auto da = fov / float(num);
		float a = fmodf(start - fov / 2 + 2 * M_PI, 2 * M_PI);
		float a2 = fmodf(start + 2 * M_PI, 2 * M_PI);
		for (uint i = 0; i != num; ++i) {
			points[i] = {a2, sin(a), cos(a)};
			a += da;
			a2 += da;
			a = fmodf(a, M_PI * 2);
			a2 = fmodf(a2, M_PI * 2);
		}
	}
};
} // namespace

void VR::render(Graphics::Screen *screen, float ax, float ay) {
	if (!_pic) {
		screen->clear();
		return;
	}

	if (!_vr) {
		Common::Point dst(0, 0);
		Common::Rect src(_pic->getRect());
		Common::Rect::getBlitRect(dst, src, screen->getBounds());
		screen->copyRectToSurface(*_pic, dst.x, dst.y, src);
	} else {
		screen->clear();
		auto w = g_system->getWidth();
		auto h = g_system->getHeight();

		static const float kFOV = (90 / 180.0f) * M_PI;
		Projection projH(w, ax, kFOV);
		Projection projV(h, ay - M_PI_2, kFOV);
		for (int dstY = 0; dstY != h; ++dstY) {
			auto &pv = projV.points[dstY];
			for (int dstX = 0; dstX != w; ++dstX) {
				auto &ph = projH.points[dstX];
				float x = ph.cosine * pv.sine;
				float y = ph.sine * pv.sine;
				float z = pv.cosine;
				auto cube = toCube(x, y, z);
				int srcX = static_cast<int>(512 * cube.x);
				int srcY = static_cast<int>(512 * cube.y);
				int tileId = cube.faceIdx * 4;
				tileId += (srcY < 256) ? (srcX < 256 ? 0 : 1) : (srcX < 256 ? 3 : 2);
				srcX &= 0xff;
				srcY &= 0xff;
				srcY += (tileId << 8);
				auto color = _pic->getPixel(srcX, srcY);
#if 0
				auto n = g_engine->numCursors();
				for(uint i = 0; i != n; ++i) {
					auto *src = g_engine->getCursorRect(i);
					if (src && src->contains(RectF::transform(ph.angle, pv.angle))) {
						uint8 r, g, b;
						_pic->format.colorToRGB(color, r, g, b);
						r += 32;
						g += 32;
						b += 32;
						color = _pic->format.RGBToColor(r, g, b);
						break;
					}
				}
#endif
				screen->setPixel(dstX, dstY, color);
			}
		}
	}
}

} // namespace PhoenixVR
