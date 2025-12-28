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
#include "math/vector3d.h"
#include "phoenixvr/dct_tables.h"
#include "video/4xm_utils.h"

namespace PhoenixVR {

#define CHUNK_VR (0x12fa84ab)
#define CHUNK_STATIC_2D (0xa0b1c400)
#define CHUNK_STATIC_3D (0xa0b1c200)
#define CHUNK_ANIMATION (0xa0b1c201)
#define CHUNK_ANIMATION_BLOCK (0xa0b1c211)

namespace {

template<typename T>
T clip(T a) {
	return a >= 0 ? a <= 255 ? a : 255 : 0;
}

uint32 YCbCr2RGB(const Graphics::PixelFormat &format, int16 y, int16 cb, int16 cr) {
	cr -= 128;
	cb -= 128;

	int r = clip(y + ((cr * 91881 + 32768) >> 16));
	int g = clip(y - ((cb * 22553 + cr * 46801 + 32768) >> 16));
	int b = clip(y + ((cb * 116129 + 32768) >> 16));

	return format.RGBToColor(r, g, b);
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
			v >>= 12;
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
			v >>= 12;
			quantCbCr[i] = v;
		}
	}
};

void unpack(Graphics::Surface &pic, const byte *huff, uint huffSize, const byte *acPtr, uint acSize, const byte *dcPtr, uint dcSize, int quality, const Common::Array<uint> *prefix = nullptr) {
	Quantisation quant(quality);
	auto decoded = Video::FourXM::HuffmanDecoder::unpack(huff, huffSize, 1);
	uint decodedOffset = 0;

	const uint planePitch = prefix ? 8 : pic.w;
	const uint planeSize = prefix ? prefix->size() * 64 : planePitch * pic.h;
	Common::Array<byte> planes(planeSize * 3, 0);

	Video::FourXM::LEByteBitStream acBs(acPtr, acSize, 0), dcBs(dcPtr, dcSize, 0);
	uint channel = 0;
	uint x0 = 0, y0 = 0;
	uint blockIdx = 0;
	while (decodedOffset < decoded.size()) {
		int16 ac[64] = {};
		int8 dc8 = dcBs.readUInt(8);
		auto *iquant = channel ? quant.quantCbCr : quant.quantY;
		ac[0] = iquant[0] * dc8 + 0x80 * 8 * 8;
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
					ac[ac_idx] = iquant[ac_idx] * acBs.readInt(l);
					++idx;
				}
			}
		}

		Video::FourXM::idct(ac);
		auto *dst = prefix ? planes.data() + channel * planeSize + blockIdx * 64 : planes.data() + channel * planeSize + y0 * planePitch + x0;
		const auto *src = ac;
		for (unsigned h = 8; h--; dst += planePitch - 8) {
			for (unsigned w = 8; w--;) {
				int v = *src++;
				v = clip(v);
				*dst++ = v;
			}
		}
		++channel;
		if (channel == 3) {
			++blockIdx;
			channel = 0;
			x0 += 8;
			if (static_cast<int16>(x0) >= pic.w) {
				x0 = 0;
				y0 += 8;
			}
		}
	}
	auto *yPtr = planes.data();
	auto *cbPtr = yPtr + planeSize;
	auto *crPtr = cbPtr + planeSize;

	if (prefix) {
		for (auto dstOffset : *prefix) {
			int dstY = dstOffset / pic.w;
			int dstX = dstOffset % pic.w;
			for (uint by = 0; by < 8; ++by) {
				auto *dstPixel = static_cast<uint32 *>(pic.getBasePtr(dstX, dstY++));
				for (uint bx = 0; bx < 8; ++bx) {
					int16 y = *yPtr++;
					int16 cr = *crPtr++;
					int16 cb = *cbPtr++;

					*dstPixel++ = YCbCr2RGB(pic.format, y, cb, cr);
				}
			}
		}
	} else {
#if 0
		auto &format = pic.format;
		for(int yy = 0; yy < pic.h; ++yy) {
			auto *rows = static_cast<uint32*>(pic.getBasePtr(0, yy));
			for(int xx = 0; xx < pic.w; ++xx) {
				int16 y = *yPtr++;
				int16 cr = *crPtr++;
				int16 cb = *cbPtr++;

				*rows++ = YCbCr2RGB(format, y, cb, cr);
			}
		}
#else
		YUVToRGBMan.convert444(&pic, Graphics::YUVToRGBManager::kScaleFull,
							   yPtr, cbPtr, crPtr, pic.w, pic.h, planePitch, planePitch);
#endif
	}
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
	while (s.pos() < fsize) {
		auto chunkPos = s.pos();
		auto chunkId = s.readUint32LE();
		auto chunkSize = s.readUint32LE();
		bool pic2d = chunkId == CHUNK_STATIC_2D;
		bool pic3d = chunkId == CHUNK_STATIC_3D;
		if (pic2d || pic3d) {
			auto quality = s.readUint32LE();
			auto dataSize = s.readUint32LE();

			Common::Array<byte> vrData(dataSize);
			s.read(vrData.data(), vrData.size());

			auto huffSize = READ_LE_UINT32(vrData.data());
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
			uint acOffset = huffSize + 12;
			auto *acPtr = vrData.data() + acOffset;
			auto dcOffset = READ_LE_UINT32(huff + huffSize);
			auto *dcPtr = acPtr + 4 + dcOffset;
			auto *dcEnd = vrData.data() + vrData.size();
			unpack(*pic, huff, huffSize, acPtr, dcPtr - acPtr, dcPtr, dcEnd - dcPtr, quality);
		} else if (chunkId == CHUNK_ANIMATION) {
			auto name = s.readString(0, 32);
			s.skip(4);
			debug("animation %s", name.c_str());
			while (s.pos() < chunkPos + chunkSize) {
				auto animChunkPos = s.pos();
				auto animChunkId = s.readUint32LE();
				auto animChunkSize = s.readUint32LE();
				assert(animChunkSize >= 8);
				debug("animation chunk %08x %u at %08lx", animChunkId, animChunkSize, animChunkPos);
				if (animChunkId == CHUNK_ANIMATION_BLOCK) {
					auto &blockData = vr._animations[name];
					blockData.resize(animChunkSize - 8);
					s.read(blockData.data(), blockData.size());
				}
				s.seek(animChunkPos + animChunkSize);
			}
		}
		s.seek(chunkPos + chunkSize);
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
		cube.faceIdx = 4;
	}
	if (!isXPositive && absX >= absY && absX >= absZ) {
		maxAxis = absX;
		cx = -y;
		cy = z;
		cube.faceIdx = 5;
	}
	if (isYPositive && absY >= absX && absY >= absZ) {
		maxAxis = absY;
		cx = -x;
		cy = z;
		cube.faceIdx = 3;
	}
	if (!isYPositive && absY >= absX && absY >= absZ) {
		maxAxis = absY;
		cx = x;
		cy = z;
		cube.faceIdx = 1;
	}
	if (isZPositive && absZ >= absX && absZ >= absY) {
		maxAxis = absZ;
		cx = y;
		cy = -x;
		cube.faceIdx = 0;
	}
	if (!isZPositive && absZ >= absX && absZ >= absY) {
		maxAxis = absZ;
		cx = y;
		cy = x;
		cube.faceIdx = 2;
	}

	// Convert range from −1 to 1 to 0 to 1
	cube.y = 0.5f * (cy / maxAxis + 1.0f);
	cube.x = 0.5f * (cx / maxAxis + 1.0f);
	return cube;
}

} // namespace

void VR::playAnimation(const Common::String &name) {
	auto it = _animations.find(name);
	if (it == _animations.end()) {
		debug("no animation %s", name.c_str());
		return;
	}
	if (it->_value.size() == 0)
		return;
	const auto *data = it->_value.data();
	auto prefixSize = READ_LE_UINT32(data);
	Common::Array<uint32> prefixData(prefixSize);
	uint offset = 4;
	for (uint i = 0; i != prefixSize; ++i) {
		prefixData[i] = READ_LE_UINT32(data + offset);
		offset += 4;
	}
	auto quality = READ_LE_UINT32(data + offset);
	offset += 8;
	auto huffSize = READ_LE_UINT32(data + offset);
	offset += 8;
	auto *acPtr = data + offset + huffSize + 4;
	auto dcOffset = READ_LE_UINT32(data + offset + huffSize);
	auto *dcPtr = data + offset + huffSize + 8 + dcOffset;
	auto *dcEnd = data + it->_value.size();
	unpack(*_pic, data + offset, huffSize, acPtr, dcPtr - acPtr, dcPtr, dcEnd - dcPtr, quality, &prefixData);
}

void VR::render(Graphics::Screen *screen, float ax, float ay, float fov) {
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

		// camera pose
		using namespace Math;
		Vector3d forward = Vector3d(
							   cosf(ax) * sinf(-ay),
							   sinf(ax) * sinf(-ay),
							   cosf(-ay))
							   .getNormalized();
		Vector3d right = Vector3d::crossProduct(forward, Vector3d(0, 0, -1)).getNormalized();
		Vector3d up = Vector3d::crossProduct(forward, right); // already normalized

		// camera projection
		float gx = tanf(fov / 2.0f), gy = gx * h / w;
		Vector3d incrementX = right * (2 * gx / w);
		Vector3d incrementY = up * (2 * gy / h);
		Vector3d start = forward - right * gx - up * gy;
		Vector3d line = start;
		for (int dstY = 0; dstY != h; ++dstY, line += incrementY) {
			Vector3d pixel = line;
			for (int dstX = 0; dstX != w; ++dstX, pixel += incrementX) {
				Vector3d ray = pixel.getNormalized();
				auto cube = toCube(ray.x(), ray.y(), ray.z());
				int srcX = static_cast<int>(512 * cube.x);
				int srcY = static_cast<int>(512 * cube.y);
				int tileId = cube.faceIdx * 4;
				tileId += (srcY < 256) ? (srcX < 256 ? 0 : 1) : (srcX < 256 ? 3 : 2);
				srcX &= 0xff;
				srcY &= 0xff;
				srcY += (tileId << 8);
				auto color = _pic->getPixel(srcX, srcY);
				screen->setPixel(dstX, dstY, color);
			}
		}
	}
}

} // namespace PhoenixVR
