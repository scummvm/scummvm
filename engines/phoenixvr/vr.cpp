#include "phoenixvr/vr.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"
#include "math/dct.h"

namespace PhoenixVR {

#define CHUNK_VR (0x12fa84ab)
#define CHUNK_STATIC (0xa0b1c400)

namespace {
const byte ZIGZAG[] = {
	0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4,
	5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7,
	14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22,
	15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39,
	46, 53, 60, 61, 54, 47, 55, 62, 63};

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

void unpack640x480(Graphics::Surface &pic, const byte *huff, uint huffSize, const byte *coeff, uint dataSize) {
	auto decoded = unpackHuffman(huff, huffSize);
	uint decodedOffset = 0;
	static const Math::DCT dct(6, Math::DCT::DCT_III);

	static constexpr uint planePitch = 640;
	static constexpr uint planeSize = planePitch * 480;
	Common::Array<byte> planes(planeSize * 3, 0);

	BitStream bs(coeff, 0);
	uint channel = 0;
	uint x0 = 0, y0 = 0;
	while (decodedOffset < decoded.size()) {
		float ac[64] = {};
		ac[0] = 1.0f * bs.readUInt(8);
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
					ac[ZIGZAG[idx]] = 1024 * bs.readInt(l);
					++idx;
				}
			}
		}
		dct.calc(ac);
		Common::String str;
		for (uint i = 0; i != 64; ++i)
			str += Common::String::format("%g ", ac[i]);
		debug("decoded block %s", str.c_str());

		debug("block at %d,%d %d", x0, y0, channel);
		auto *dst = planes.data() + channel++ * planeSize + y0 * planePitch + x0;
		if (channel == 3) {
			channel = 0;
			x0 += 8;
			if (x0 >= 640) {
				x0 = 0;
				y0 += 8;
			}
		}
		const auto *src = ac;
		str.clear();
		for (unsigned h = 8; h--; dst += planePitch - 8) {
			for (unsigned w = 8; w--;) {
				int v = *src++;
				if (v < 0)
					v = 0;
				if (v > 255)
					v = 255;
				str += Common::String::format("%d ", v);
				*dst++ = v;
			}
		}
		debug("decoded block %s", str.c_str());
	}
	auto *y = planes.data();
	auto *u = y + planeSize;
	auto *v = u + planeSize;
	YUVToRGBMan.convert444(&pic, Graphics::YUVToRGBManager::kScaleFull, y, u, v, 640, 480, planePitch, planePitch);
}
} // namespace

Graphics::Surface *VR::load640x480(const Graphics::PixelFormat &format, Common::SeekableReadStream &s) {
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
		if (chunkId == CHUNK_STATIC) {
			auto unk0 = s.readUint32LE();
			auto dataSize = s.readUint32LE();
			auto coefOffset = s.readUint32LE();
			auto unpHuffSize = s.readUint32LE();
			debug("static picture header %u packed data size: %u ac/dc offset: %08x unpacked huff size: %u", unk0, dataSize, coefOffset, unpHuffSize);
			Common::Array<byte> staticData(chunkSize - 16 - 8);
			s.read(staticData.data(), staticData.size());
			Graphics::Surface *pic = new Graphics::Surface();
			pic->create(640, 480, format);
			unpack640x480(*pic, staticData.data(), unpHuffSize, staticData.data() + coefOffset, staticData.size());
			return pic;
		}
		s.skip(chunkSize - 8);
	}
	return nullptr;
}

} // namespace PhoenixVR
