#include "phoenixvr/vr.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"
#include "image/bmp.h"
#include "phoenixvr/dct.h"

namespace PhoenixVR {

#define CHUNK_VR (0x12fa84ab)
#define CHUNK_STATIC_2D (0xa0b1c400)
#define CHUNK_STATIC_3D (0xa0b1c200)

namespace {

template<typename T>
T clip(T a) {
	return a >= 0 ? a <= 255 ? a : 255 : 0;
}

const byte ZIGZAG[] = {
	0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4,
	5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7,
	14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22,
	15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39,
	46, 53, 60, 61, 54, 47, 55, 62, 63};

// looks like standard JPEG quantisation matrix
const char QY[] = {
	16,
	11,
	10,
	16,
	24,
	40,
	51,
	61,
	12,
	12,
	14,
	19,
	26,
	58,
	60,
	55,
	14,
	13,
	16,
	24,
	40,
	57,
	69,
	56,
	14,
	17,
	22,
	29,
	51,
	87,
	80,
	62,
	18,
	22,
	37,
	56,
	68,
	109,
	103,
	77,
	24,
	35,
	55,
	64,
	81,
	104,
	113,
	92,
	49,
	64,
	78,
	87,
	103,
	121,
	120,
	101,
	72,
	92,
	95,
	98,
	112,
	100,
	103,
	99,
};

const char QUV[] = {
	17,
	18,
	24,
	47,
	99,
	99,
	99,
	99,
	18,
	21,
	26,
	66,
	99,
	99,
	99,
	99,
	24,
	26,
	56,
	99,
	99,
	99,
	99,
	99,
	47,
	66,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
	99,
};

const uint16 Q[] = {
	0x4000,
	0x58C5,
	0x539F,
	0x4B42,
	0x4000,
	0x3249,
	0x22A3,
	0x11A8,
	0x58C5,
	0x7B21,
	0x73FC,
	0x6862,
	0x58C5,
	0x45BF,
	0x300B,
	0x187E,
	0x539F,
	0x73FC,
	0x6D41,
	0x6254,
	0x539F,
	0x41B3,
	0x2D41,
	0x1712,
	0x4B42,
	0x6862,
	0x6254,
	0x587E,
	0x4B42,
	0x3B21,
	0x28BA,
	0x14C3,
	0x4000,
	0x58C5,
	0x539F,
	0x4B42,
	0x4000,
	0x3249,
	0x22A3,
	0x11A8,
	0x3249,
	0x45BF,
	0x41B3,
	0x3B21,
	0x3249,
	0x2782,
	0x1B37,
	0x0DE0,
	0x22A3,
	0x300B,
	0x2D41,
	0x28BA,
	0x22A3,
	0x1B37,
	0x12BF,
	0x98E,
	0x11A8,
	0x187E,
	0x1712,
	0x14C3,
	0x11A8,
	0x0DE0,
	0x98E,
	0x4DF,
};

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

void VR::render(Graphics::Screen *screen) {
	if (!_pic) {
		screen->clear();
		return;
	}

	if (!_vr) {
		Common::Point dst(0, 0);
		Common::Rect src(_pic->getRect());
		Common::Rect::getBlitRect(dst, src, screen->getBounds());
		screen->copyRectToSurface(*_pic, dst.x, dst.y, src);
	}
}

} // namespace PhoenixVR
