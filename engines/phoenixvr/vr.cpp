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
#include "common/bitstream.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "image/bmp.h"
#include "math/vector3d.h"
#include "phoenixvr/angle.h"
#include "phoenixvr/dct_tables.h"
#include "phoenixvr/phoenixvr.h"
#include "phoenixvr/region_set.h"
#include "video/4xm_utils.h"

namespace PhoenixVR {

#define CHUNK_VR (0x12fa84ab)
#define CHUNK_STATIC_2D (0xa0b1c400)
#define CHUNK_STATIC_3D (0xa0b1c200)
#define CHUNK_ANIMATION (0xa0b1c201)
#define CHUNK_ANIMATION_BLOCK (0xa0b1c211)
#define CHUNK_ANIMATION_RESTART (0xa0b1c221)

namespace {

template<typename T>
T clip(T a) {
	return a >= 0 ? a <= 255 ? a : 255 : 0;
}

unsigned reverseBits(unsigned value, unsigned n) {
	unsigned r = 0;
	while (n--) {
		r <<= 1;
		if (value & 1)
			r |= 1;
		value >>= 1;
	}
	return r;
}

uint32 YCbCr2RGB(const Graphics::PixelFormat &format, int16 y, int16 cb, int16 cr) {
	int r = clip(y + ((cr * 91881 + 32768) >> 16));
	int g = clip(y - ((cb * 22553 + cr * 46801 + 32768) >> 16));
	int b = clip(y + ((cb * 116129 + 32768) >> 16));

	return format.RGBToColor(r, g, b);
}

struct Quantisation {
	int quantY[64];
	int quantCbCr[64];

	Quantisation(int quality) {
		int q;
		assert(quality != 0);
		if (quality < 0) {
			q = 5000;
		} else if (quality > 100) {
			quality = 100;
			q = 2 * (100 - quality);
		} else if (quality >= 50) {
			q = 2 * (100 - quality);
		} else {
			q = 5000 / quality;
		}
		for (uint i = 0; i != 64; ++i) {
			auto v = (QY[i] * q + 50) / 100;
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
			auto v = (QUV[i] * q + 50) / 100;
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

void unpack(Graphics::Surface &pic, const byte *huff, uint huffSize, const byte *acPtr, uint acSize, const byte *dcPtr, uint dcSize, int quality, const Common::Array<uint32> *prefix = nullptr) {
	Quantisation quant(quality);

	uint huffOffset = 0;
	auto huffDecoder = Video::FourXM::loadStatistics<Common::Huffman<Common::BitStreamMemory8MSB>>(huff, huffOffset);
	Common::BitStreamMemoryStream huffMs(huff + huffOffset, huffSize - huffOffset);
	Common::BitStreamMemory8MSB huffBs(&huffMs);

	Common::BitStreamMemoryStream acMs(acPtr, acSize);
	Common::BitStreamMemoryStream dcMs(dcPtr, dcSize);
	Common::BitStreamMemory8MSB acBs(&acMs), dcBs(&dcMs);

	const auto dstPitch = pic.pitch / pic.format.bytesPerPixel - 8;
	unsigned numBlocks = prefix ? prefix->size() : ((pic.w + 7) / 8) * ((pic.h + 7) / 8);
	for (uint blockIdx = 0; blockIdx < numBlocks; ++blockIdx) {
		int16 block[3][64] = {};
		for (unsigned channel = 0; channel != 3; ++channel) {
			int16 *ac = block[channel];
			int8 dc8 = static_cast<int8>(reverseBits(dcBs.getBits<8>(), 8));
			auto *iquant = channel ? quant.quantCbCr : quant.quantY;
			ac[0] = iquant[0] * dc8;
			for (uint idx = 1; idx < 64;) {
				auto b = huffDecoder.getSymbol(huffBs);
				assert(b < 0x100);
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
						auto k = Video::FourXM::readInt(reverseBits(acBs.getBits(l), l), l);
						ac[ac_idx] = iquant[ac_idx] * k;
						++idx;
					}
				}
			}

			Video::FourXM::idct(ac, 4);
		}
		int dstY;
		int dstX;
		if (prefix) {
			auto dstOffset = (*prefix)[blockIdx];
			dstY = dstOffset / pic.w;
			dstX = dstOffset % pic.w;
		} else {
			auto dstOffset = blockIdx << 3;
			dstX = dstOffset % pic.w;
			dstY = (dstOffset / pic.w) << 3;
		}

		switch (pic.format.bytesPerPixel) {
		case 4: {
			auto *dstPixel = static_cast<uint32 *>(pic.getBasePtr(dstX, dstY));
			uint srcIdx = 0;
			for (uint by = 0; by < 8; ++by, dstPixel += dstPitch) {
				for (uint bx = 0; bx < 8; ++bx, ++srcIdx) {
					int16 y = block[0][srcIdx];
					int16 cb = block[1][srcIdx];
					int16 cr = block[2][srcIdx];

					*dstPixel++ = YCbCr2RGB(pic.format, y + 128, cb, cr);
				}
			}
		} break;
		case 2: {
			auto *dstPixel = static_cast<uint16 *>(pic.getBasePtr(dstX, dstY));
			uint srcIdx = 0;
			for (uint by = 0; by < 8; ++by, dstPixel += dstPitch) {
				for (uint bx = 0; bx < 8; ++bx, ++srcIdx) {
					int16 y = block[0][srcIdx];
					int16 cb = block[1][srcIdx];
					int16 cr = block[2][srcIdx];

					*dstPixel++ = YCbCr2RGB(pic.format, y + 128, cb, cr);
				}
			}
		} break;
		}
	}
}
} // namespace

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
			vr._pic.reset(new Graphics::ManagedSurface());
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
			unpack(*pic->surfacePtr(), huff, huffSize, acPtr, dcPtr - acPtr, dcPtr, dcEnd - dcPtr, quality);
		} else if (chunkId == CHUNK_ANIMATION) {
			Animation animation;
			animation.name = s.readString(0, 32);
			auto numFrames = s.readUint32LE();
			animation.frames.reserve(numFrames);
			debug("animation %s, frames: %u", animation.name.c_str(), numFrames);
			while (s.pos() < chunkPos + chunkSize) {
				auto animChunkPos = s.pos();
				auto animChunkId = s.readUint32LE();
				auto animChunkSize = s.readUint32LE();
				debug("animation frame at %08x: %08x %u", (uint32)animChunkPos, animChunkId, animChunkSize);
				assert(animChunkSize >= 8);
				Animation::Frame frame;
				if (animChunkId == CHUNK_ANIMATION_BLOCK) {
					frame.blockData.resize(animChunkSize - 8);
					s.read(frame.blockData.data(), frame.blockData.size());
				} else if (animChunkId == CHUNK_ANIMATION_RESTART) {
					assert(animChunkSize - 8 == 4);
					byte buf[4] = {};
					s.read(buf, sizeof(buf));
					frame.restartAtFrame = READ_LE_INT32(buf);
					debug("animation loop, frame: %d", frame.restartAtFrame);
				} else {
					Common::Array<byte> buf(animChunkSize - 8);
					s.read(buf.data(), buf.size());
					warning("unknown frame type");
				}
				animation.frames.push_back(Common::move(frame));
				s.seek(animChunkPos + animChunkSize);
			}
			vr._animations.push_back(Common::move(animation));
		}
		s.seek(chunkPos + chunkSize);
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

	float maxAxis = 0, cy = 0, cx = 0;

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

void VR::Animation::Frame::render(Graphics::Surface &pic) const {
	if (blockData.empty())
		return;
	const auto *data = blockData.data();
	auto prefixSize = READ_LE_UINT32(data);
	if (prefixSize == 0)
		return;
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
	auto *dcEnd = data + blockData.size();
	unpack(pic, data + offset, huffSize, acPtr, dcPtr - acPtr, dcPtr, dcEnd - dcPtr, quality, &prefixData);
}

void VR::playAnimation(const Common::String &name, const Common::String &variable, int value, float speed) {
	auto it = Common::find_if(_animations.begin(), _animations.end(), [&](const Animation &a) { return a.name.compareToIgnoreCase(name) == 0; });
	if (it == _animations.end()) {
		debug("no animation %s", name.c_str());
		return;
	}
	auto &animation = *it;
	animation.active = true;
	animation.frameIndex = 0;
	animation.t = 0;
	animation.speed = speed;
	animation.variable = variable;
	animation.variableValue = value;
	animation.renderNextFrame(*_pic->surfacePtr());
	if (animation.speed == 0)
		g_engine->setVariable(variable, value);
}

void VR::Animation::renderNextFrame(Graphics::Surface &pic) {
	assert(active);
	if (frameIndex >= frames.size()) {
		active = false;
		g_engine->setVariable(variable, variableValue);
	} else {
		auto &frame = frames[frameIndex++];
		frame.render(pic);
		if (frame.restartAtFrame >= 0) {
			frameIndex = frame.restartAtFrame;
			t = 1;
		}
	}
}

void VR::Animation::render(Graphics::Surface &pic, float dt) {
	if (!active)
		return;

	t += speed * dt;
	if (t >= 1) {
		t = fmodf(t, 1);
		renderNextFrame(pic);
	}
}

void VR::render(Graphics::Screen *screen, float ax, float ay, float fov, float dt, RegionSet *regSet) {
	if (!_pic) {
		screen->clear();
		return;
	}

	for (auto &animation : _animations)
		animation.render(*_pic->surfacePtr(), dt);

	if (!_vr) {
		Common::Point dst(0, 0);
		Common::Rect src(_pic->getBounds());
		Common::Rect::getBlitRect(dst, src, screen->getBounds());
		screen->copyRectToSurface(*_pic, dst.x, dst.y, src);
		if (regSet) {
			for (auto &rect : regSet->getRegions())
				screen->drawRoundRect(rect.toRect().toRect(), 4, _pic->format.RGBToColor(255, 255, 255), false);
		}
	} else {
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
		float regX = 0, regY = 0, regDX = 0, regDY = 0;
		if (regSet) {
			regY = ay - fov / 2;
			regDX = fov / w;
			regDY = fov / h;
			if (regY < 0)
				regY += kTau;
		}
		for (int dstY = 0; dstY != h; ++dstY, line += incrementY) {
			if (regSet) {
				regX = ax - fov / 2;
				if (regX < 0)
					regX += kTau;
				if (regX >= kTau)
					regX -= kTau;
			}
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
				if (regSet) {
					regX += regDX;
					if (regX >= kTau)
						regX -= kTau;
					for (auto &reg : regSet->getRegions()) {
						if (reg.contains3D(regX, kTau - regY)) {
							byte r, g, b;
							_pic->format.colorToRGB(color, r, g, b);
							r ^= _rnd.getRandomNumber(31);
							g ^= _rnd.getRandomNumber(31);
							b ^= _rnd.getRandomNumber(31);
							color = screen->format.RGBToColor(r, g, b);
						}
					}
				}
				screen->setPixel(dstX, dstY, color);
			}
			if (regSet) {
				regY += regDY;
				if (regY >= kTau)
					regY -= kTau;
			}
		}
		screen->addDirtyRect(screen->getBounds());
	}
}

} // namespace PhoenixVR
