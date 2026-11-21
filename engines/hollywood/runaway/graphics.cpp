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

#include "common/endian.h"
#include "common/file.h"
#include "common/util.h"

#include "hollywood/resource.h"
#include "hollywood/runaway/graphics.h"

namespace Hollywood {

static bool decodeSurface(const Common::Array<byte> &pixels, int width, int height, Graphics::ManagedSurface &surface) {
	if (width <= 0 || height <= 0 || pixels.size() != (uint64)width * height * 2)
		return false;
	surface.create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	for (int y = 0; y < height; ++y) {
		uint16 *out = (uint16 *)surface.getBasePtr(0, y);
		for (int x = 0; x < width; ++x)
			out[x] = READ_LE_UINT16(pixels.data() + (y * width + x) * 2);
	}
	return true;
}

bool readRunawayFile(const Common::Path &path, Common::Array<byte> &data) {
	Common::File file;
	if (!file.open(path) || file.size() <= 0 || file.size() > 32 * 1024 * 1024)
		return false;
	data.resize(file.size());
	return file.read(data.data(), data.size()) == data.size();
}

bool loadRunawaySurface(uint chunk, int width, int height, Graphics::ManagedSurface &surface) {
	Common::Array<byte> pixels;
	return readRunawayGlobalChunk(chunk, pixels) && decodeSurface(pixels, width, height, surface);
}

bool loadRunawaySurface(const Common::Path &path, int width, int height, Graphics::ManagedSurface &surface) {
	Common::Array<byte> pixels;
	return readRunawayFile(path, pixels) && decodeSurface(pixels, width, height, surface);
}

bool loadRunawaySurface(ChunkArchive &archive, uint chunk, int width, int height, Graphics::ManagedSurface &surface) {
	Common::Array<byte> pixels;
	return archive.readVariableChunk(chunk, pixels) && decodeSurface(pixels, width, height, surface);
}

bool loadRunawayMask(ChunkArchive &archive, int width, int height, Common::Array<byte> &mask, Common::Array<byte> &maps, uint chunk) {
	Common::Array<byte> packed;
	if (width <= 0 || height <= 0 || (uint64)width * height > 16 * 1024 * 1024 ||
			!archive.readVariableChunk(chunk, packed) || packed.size() % 3 ||
			!archive.readVariableChunk(chunk + 1, maps) || maps.size() != 1536)
		return false;
	mask.resize(width * height);
	// Each run is (byte value, uint16 length). The first four 256-byte tables
	// map these values to walking zones, hotspots, depth and lighting.
	uint offset = 0;
	for (uint i = 0; i < packed.size(); i += 3) {
		const uint length = READ_LE_UINT16(packed.data() + i + 1);
		if (!length || length > mask.size() - offset)
			return false;
		memset(mask.data() + offset, packed[i], length);
		offset += length;
	}
	return offset == mask.size();
}

byte findRunawayLight(Common::Point position, int width, const Common::Array<byte> &mask, const Common::Array<byte> &maps) {
	if (width <= 0 || mask.size() % width || maps.size() != 1536) {
		return 0;
	}
	const int height = mask.size() / width;
	if (position.x < 0 || position.x >= width || position.y < 0 || position.y >= height) {
		return 0;
	}
	// Match the original fallback for gaps in the lighting map: search the
	// column down/up, then the top row right/left.
	for (int y = position.y; y < height; ++y) {
		const byte light = maps[768 + mask[y * width + position.x]];
		if (light) {
			return light;
		}
	}
	for (int y = position.y - 1; y >= 0; --y) {
		const byte light = maps[768 + mask[y * width + position.x]];
		if (light) {
			return light;
		}
	}
	for (int x = position.x + 1; x < width; ++x) {
		const byte light = maps[768 + mask[x]];
		if (light) {
			return light;
		}
	}
	for (int x = position.x - 1; x >= 0; --x) {
		const byte light = maps[768 + mask[x]];
		if (light) {
			return light;
		}
	}
	return 0;
}

bool applyRunawayPatch(const Common::Array<byte> &patch, Graphics::ManagedSurface &surface, int yOffset, int xOffset) {
	if (patch.size() < 2 || surface.format.bytesPerPixel != 2)
		return false;
	uint cursor = 2;
	for (uint i = 0; i < READ_LE_UINT16(patch.data()); ++i) {
		if (patch.size() - cursor < 6)
			return false;
		const uint x = READ_LE_UINT16(patch.data() + cursor);
		const int y = READ_LE_UINT16(patch.data() + cursor + 2) + yOffset;
		const uint length = READ_LE_UINT16(patch.data() + cursor + 4);
		cursor += 6;
		if (!length || x + length > 32767 || length * 2 > patch.size() - cursor)
			return false;
		const int left = (int)x + xOffset;
		const int first = MAX(0, -left), last = MIN<int>(length, surface.w - left);
		if (y >= 0 && y < surface.h && first < last) {
			uint16 *out = (uint16 *)surface.getBasePtr(left + first, y);
			for (int pixel = first; pixel < last; ++pixel)
				*out++ = READ_LE_UINT16(patch.data() + cursor + pixel * 2);
		}
		cursor += length * 2;
	}
	return cursor == patch.size();
}

bool RunawayAnimation::load(ChunkArchive &archive, uint chunk, uint frames) {
	_frames = 0;
	return archive.readVariableChunk(chunk, _data) && validate(frames);
}

bool RunawayAnimation::load(const Common::Array<byte> &data, uint frames) {
	_data = data;
	return validate(frames);
}

bool RunawayAnimation::validate(uint frames) {
	_frames = 0;
	if (!frames || frames > 0xffff || _data.size() < frames * 14)
		return false;
	uint cursor = frames * 14;
	for (uint frame = 0; frame < frames; ++frame) {
		const byte *descriptor = _data.data() + frame * 14;
		if (READ_LE_UINT32(descriptor) != cursor - frames * 14)
			return false;
		const uint spans = READ_LE_UINT16(descriptor + 12);
		for (uint span = 0; span < spans; ++span) {
			if (_data.size() - cursor < 5)
				return false;
			const byte *run = _data.data() + cursor;
			const uint x = READ_LE_UINT16(run), y = READ_LE_UINT16(run + 2), length = run[4];
			cursor += 5;
			if (x + length > 32767 || y >= 32767 || !length || length * 2 > _data.size() - cursor)
				return false;
			cursor += length * 2;
		}
	}
	if (cursor != _data.size())
		return false;
	_frames = frames;
	return true;
}

void RunawayAnimation::draw(Graphics::ManagedSurface &surface, uint frame, int cameraX, int roomTop) const {
	if (frame >= _frames)
		return;
	const byte *descriptor = _data.data() + frame * 14;
	const byte *run = _data.data() + _frames * 14 + READ_LE_UINT32(descriptor);
	const uint spans = READ_LE_UINT16(descriptor + 12);
	for (uint span = 0; span < spans; ++span) {
		const int x = READ_LE_UINT16(run) - cameraX;
		const int y = READ_LE_UINT16(run + 2) + roomTop;
		const uint length = run[4];
		run += 5;
		if (y >= 0 && y < surface.h) {
			const int first = MAX(0, -x), last = MIN<int>(length, surface.w - x);
			if (first < last) {
				uint16 *out = (uint16 *)surface.getBasePtr(x + first, y);
				for (int i = first; i < last; ++i)
					*out++ = READ_LE_UINT16(run + i * 2);
			}
		}
		run += length * 2;
	}
}

static bool readGlobalChunk(Common::File &file, uint chunk, Common::Array<byte> &data) {
	if (chunk >= 500 || file.size() < 4010 || !file.seek(10))
		return false;
	// The first offset points immediately past both tables, even when the
	// first chunk is empty. Do not mistake retail configuration bytes for offsets.
	uint prefix = 10;
	if (file.readUint32LE() != 4010) {
		prefix = 20;
		if (!file.seek(prefix) || file.readUint32LE() != 4020)
			return false;
	}
	if (!file.seek(prefix + chunk * 4))
		return false;
	const uint32 offset = file.readUint32LE();
	if (!file.seek(prefix + 500 * 4 + chunk * 4))
		return false;
	const uint32 size = file.readUint32LE();
	if (file.err() || !size || size > 16 * 1024 * 1024 || offset < prefix + 4000 || offset > file.size() || size > file.size() - offset ||
			!file.seek(offset))
		return false;
	data.resize(size);
	return file.read(data.data(), size) == size;
}

bool readRunawayGlobalChunk(uint chunk, Common::Array<byte> &data) {
	Common::File file;
	return file.open("RESOURCE.000") && readGlobalChunk(file, chunk, data);
}

bool RunawayActor::loadChapter(byte chapter, bool alternate) {
	if (chapter < 1 || chapter > 6)
		return false;
	if (chapter <= 2)
		return load(alternate ? kChapters12Alternate : kChapters12);
	if (chapter == 3)
		return load(alternate ? kCabinAlternate : kCabin);
	return load(alternate ? kChapters456Alternate : kChapters456);
}

bool RunawayActor::load(Model model) {
	if (_loadedModel == model)
		return setFrame(3, 0);
	_loadedModel = -1;
	Common::File file;
	Common::Array<byte> palette;
	const bool museum = model == kMuseumInterior || model == kMuseumHall;
	const bool interior = model == kMuseumInterior;
	// Each bank contains descriptors, palette, eight walking banks, six
	// head banks and head descriptors (retail loader 0x40eac0).
	uint base = 44;
	switch (model) {
	case kChapters12: base = 10; break;
	case kChapters12Alternate: base = 27; break;
	case kCabinAlternate: base = 61; break;
	case kChapters456: base = 78; break;
	case kChapters456Alternate: base = 95; break;
	default: break;
	}
	_facing = _frame = _headFrame = -1;
	_museum = museum;
	_width = museum ? (interior ? 237 : 238) : 345;
	_height = museum ? 455 : 518;
	_anchorX = museum ? (interior ? 120 : 119) : 168;
	_anchorY = museum ? 403 : 434;
	const char *suffix = interior ? "spi" : "spd";
	if (museum) {
		if (!readRunawayFile(Common::Path(Common::String::format("br1_%s.VAL", suffix)), _descriptors) ||
				!readRunawayFile(Common::Path(Common::String::format("br1_%s.PAL", suffix)), palette) ||
				!readRunawayFile(Common::Path(Common::String::format("ca1_%s.VAL", suffix)), _headDescriptors))
			return false;
	} else if (!file.open("RESOURCE.000") || !readGlobalChunk(file, base, _descriptors) ||
			!readGlobalChunk(file, base + 1, palette) || !readGlobalChunk(file, base + 16, _headDescriptors)) {
		return false;
	}
	if (_descriptors.size() != kFacings * kFrames * 32 || palette.size() != 512 ||
			_headDescriptors.size() != kHeadFacings * kHeadFrames * 32)
		return false;
	for (uint i = 0; i < 256; ++i)
		_palette[i] = READ_LE_UINT16(palette.data() + i * 2);
	_pixels.resize(_width * _height);
	_opacity.resize(_width * _height);
	_shadow.resize(_width * _height);
	for (uint facing = 0; facing < kFacings; ++facing) {
		if (museum ? !readRunawayFile(Common::Path(Common::String::format("br1_%s%u.SPR", suffix, facing)), _banks[facing]) :
				!readGlobalChunk(file, base + 2 + facing, _banks[facing]))
			return false;
		for (uint frame = 0; frame < kFrames; ++frame) {
			if (!setFrame(facing, frame))
				return false;
		}
	}
	const byte headFacings[kHeadFacings] = {0, 1, 2, 4, 5, 7};
	for (uint bank = 0; bank < kHeadFacings; ++bank) {
		if (museum ? !readRunawayFile(Common::Path(Common::String::format("ca1_%s%u.SPR", suffix, bank)), _headBanks[bank]) :
				!readGlobalChunk(file, base + 10 + bank, _headBanks[bank]))
			return false;
		for (uint frame = 0; frame < kHeadFrames; ++frame) {
			if (!setFrame(headFacings[bank], 0, frame))
				return false;
		}
	}
	if (!setFrame(3, 0))
		return false;
	_loadedModel = model;
	return true;
}

bool RunawayActor::setFrame(uint facing, uint frame, int headFrame) {
	if (facing >= kFacings || frame >= kFrames || headFrame < -1 || headFrame >= kHeadFrames)
		return false;
	// Head cels replace the top of the standing body; walking cels already
	// contain their own moving head and shoulders.
	if (frame)
		headFrame = -1;
	if (_facing == (int)facing && _frame == (int)frame && _headFrame == headFrame)
		return true;
	const byte *descriptor = _descriptors.data() + (facing * kFrames + frame) * 32;
	const Common::Array<byte> &bank = _banks[facing];
	const uint end = frame + 1 < kFrames ? READ_LE_UINT32(descriptor + 32) : bank.size();
	if (!decodeBodyFrame(bank, descriptor, end))
		return false;
	if (headFrame >= 0) {
		// Native 0x428560 maps eight body directions onto six head banks and
		// clears the old head before compositing color and opacity. Keep the
		// body's shadow, then scale/light/occlude the completed actor together.
		const byte headFacings[kFacings] = {0, 1, 2, 2, 3, 4, 4, 5};
		const uint headFacing = headFacings[facing];
		const Common::Array<byte> &headBank = _headBanks[headFacing];
		const byte *head = _headDescriptors.data() + (headFacing * kHeadFrames + headFrame) * 32;
		const uint headEnd = headFrame + 1 < kHeadFrames ? READ_LE_UINT32(head + 32) : headBank.size();
		const uint clearHeight = _museum ? kHeadHeight - head[8] : kHeadHeight;
		if (clearHeight > kHeadHeight)
			return false;
		memset(_pixels.data(), 0, _width * clearHeight);
		memset(_opacity.data(), 0, _width * clearHeight);
		if (!decodeFrame(headBank, head, headEnd, true))
			return false;
	}
	_facing = facing;
	_frame = frame;
	_headFrame = headFrame;
	return true;
}

bool RunawayActor::loadAction(uint chunk, uint frames) {
	// RESOURCE.001 has 216 banks, each preceded by 100 frame descriptors.
	// The size table counts only the sprite data after those descriptors.
	Common::File file;
	if (_loadedModel < 0 || _museum || chunk >= 216 || !frames || frames > 100 || !file.open("RESOURCE.001")) {
		return false;
	}
	file.seek(chunk * 4);
	const uint32 offset = file.readUint32LE();
	file.seek((216 + chunk) * 4);
	const uint32 size = file.readUint32LE();
	if (!offset || !size || (uint64)offset + 3200 + size > (uint64)file.size()) {
		return false;
	}
	_actionDescriptors.resize(frames * 32);
	_actionBank.resize(size);
	file.seek(offset);
	if (file.read(_actionDescriptors.data(), _actionDescriptors.size()) != _actionDescriptors.size() ||
			!file.seek(offset + 3200) || file.read(_actionBank.data(), size) != size) {
		return false;
	}
	const int facing = _facing;
	for (uint frame = 0; frame < frames; ++frame) {
		if (!setActionFrame(frame)) {
			return false;
		}
	}
	return setFrame(facing, 0);
}

bool RunawayActor::setActionFrame(uint frame) {
	if (frame >= _actionDescriptors.size() / 32) {
		return false;
	}
	const byte *descriptor = _actionDescriptors.data() + frame * 32;
	const uint end = (frame + 1) * 32 < _actionDescriptors.size() ? READ_LE_UINT32(descriptor + 32) : _actionBank.size();
	return decodeBodyFrame(_actionBank, descriptor, end);
}

bool RunawayActor::decodeBodyFrame(const Common::Array<byte> &bank, const byte *descriptor, uint end) {
	const int left = _anchorX - (int)READ_LE_UINT32(descriptor + 16);
	const int top = _anchorY - (int)READ_LE_UINT32(descriptor + 20);
	const uint width = READ_LE_UINT32(descriptor + 24), height = READ_LE_UINT32(descriptor + 28);
	if (left < 0 || top < 0 || width > _width || height > _height ||
			left + width > _width || top + height > _height) {
		return false;
	}
	_bounds = Common::Rect(left, top, left + width, top + height);
	memset(_pixels.data(), 0, _pixels.size());
	memset(_opacity.data(), 0, _opacity.size());
	memset(_shadow.data(), 0, _shadow.size());
	_facing = _frame = _headFrame = -1;
	return decodeFrame(bank, descriptor, end, false);
}

bool RunawayActor::decodeFrame(const Common::Array<byte> &bank, const byte *descriptor, uint end, bool head) {
	uint cursor = READ_LE_UINT32(descriptor);
	if (cursor > end || end > bank.size())
		return false;
	// Packed streams: color spans (x,y,length), then opacity and shadow spans
	// (x,y,literal,length). A nonliteral mask span expands to opaque bytes.
	// Head descriptors retain a shadow count, but head banks omit that plane.
	const uint counts[] = { READ_LE_UINT32(descriptor + 4), READ_LE_UINT32(descriptor + 12),
		READ_LE_UINT32(descriptor + 8) };
	for (uint plane = 0; plane < (head ? 2U : 3U); ++plane) {
		byte *destination = plane == 0 ? _pixels.data() : plane == 1 ? _opacity.data() : _shadow.data();
		const uint header = plane == 0 ? 6 : 7;
		for (uint span = 0; span < counts[plane]; ++span) {
			if (end - cursor < header)
				return false;
			const byte *run = bank.data() + cursor;
			const uint x = READ_LE_UINT16(run), y = READ_LE_UINT16(run + 2);
			const bool literal = plane == 0 || run[4] != 0;
			const uint length = READ_LE_UINT16(run + header - 2);
			cursor += header;
			if (!length || x + length > _width || y >= (head ? kHeadHeight : _height) ||
					(literal && length > end - cursor))
				return false;
			if (head)
				_bounds.extend(Common::Rect(x, y, x + length, y + 1));
			if (literal) {
				memcpy(destination + y * _width + x, bank.data() + cursor, length);
				cursor += length;
			} else {
				memset(destination + y * _width + x, 255, length);
			}
		}
	}
	return cursor == end;
}

static uint16 blend565(uint16 source, uint16 destination, uint alpha) {
	const uint red = ((source >> 11) * alpha + (destination >> 11) * (255 - alpha)) / 255;
	const uint green = (((source >> 5) & 63) * alpha + ((destination >> 5) & 63) * (255 - alpha)) / 255;
	const uint blue = ((source & 31) * alpha + (destination & 31) * (255 - alpha)) / 255;
	return (red << 11) | (green << 5) | blue;
}

float RunawayActor::perspectiveHeight(Common::Point position) {
	// Native scene callback 0x438970, also used to set footstep volume.
	if (position.x > 1079 && position.y < 542)
		return position.y * 0.98648649f - 93.675674f;
	if (position.x > 1215)
		return position.x * 0.2857143f + 93.85714f;
	return (position.y * 1.47f - 348.04f + position.x * 0.058927f + 362.462616f) * 0.5f;
}

void RunawayActor::draw(Graphics::ManagedSurface &surface, Common::Point position, int cameraX,
		int roomTop, const Common::Array<byte> &mask, const Common::Array<byte> &maps, bool sunlit, bool antialias) const {
	// Scene lighting and occlusion come from the same pixel lookup as hotspots.
	const byte light = MIN<byte>(5, findRunawayLight(position, 1376, mask, maps));
	byte lights[6][3] = { {100,100,100}, {55,50,45}, {55,50,45},
		{85,80,75}, {75,70,65}, {70,65,60} };
	if (sunlit) {
		const byte windowLight[2][3] = {{90, 85, 80}, {80, 75, 70}};
		memcpy(lights[1], windowLight[0], sizeof(windowLight[0]));
		memcpy(lights[2], windowLight[1], sizeof(windowLight[1]));
	}
	RenderParams params;
	params.position = position;
	params.camera = Common::Point(cameraX, kRunawayViewportTop - roomTop);
	params.roomWidth = 1376;
	params.height = perspectiveHeight(position);
	params.antialias = antialias;
	memcpy(params.light, lights[light], sizeof(params.light));
	draw(surface, params, mask, maps);
}

void RunawayActor::draw(Graphics::ManagedSurface &surface, const RenderParams &params,
		const Common::Array<byte> &mask, const Common::Array<byte> &maps) const {
	if (params.roomWidth <= 0 || mask.size() % params.roomWidth || maps.size() != 1536)
		return;
	const int roomHeight = mask.size() / params.roomWidth;
	// Douglasville's distant street positions shrink Brian below quarter size.
	const int scale = CLIP<int>((int)(params.height * 256 / 455), 1, 256);
	const int roomTop = kRunawayViewportTop - params.camera.y;
	const int originX = params.position.x - params.camera.x - _anchorX * scale / 256;
	const int originY = params.position.y + roomTop - _anchorY * scale / 256;
	Common::Rect bounds(originX + _bounds.left * scale / 256, originY + _bounds.top * scale / 256,
		originX + _bounds.right * scale / 256, originY + _bounds.bottom * scale / 256);
	bounds.clip(Common::Rect(0, kRunawayViewportTop, surface.w, MIN<int>(surface.h, kRunawayViewportTop + kRunawayViewportHeight)));
	bounds.clip(Common::Rect(-params.camera.x, roomTop, params.roomWidth - params.camera.x, roomTop + roomHeight));
	if (bounds.isEmpty())
		return;
	uint16 palette[256];
	for (uint i = 0; i < 256; ++i) {
		const uint16 c = _palette[i];
		palette[i] = (((c >> 11) * params.light[0] / 100) << 11) |
			((((c >> 5) & 63) * params.light[1] / 100) << 5) | ((c & 31) * params.light[2] / 100);
	}
	for (int y = bounds.top; y < bounds.bottom; ++y) {
		uint16 *out = (uint16 *)surface.getBasePtr(bounds.left, y);
		const int sourceY = (y - originY) * 256 / scale;
		for (int x = bounds.left; x < bounds.right; ++x, ++out) {
			const byte depth = maps[512 + mask[(y - roomTop) * params.roomWidth + x + params.camera.x]];
			if (depth && (depth >= ARRAYSIZE(params.depthY) || params.position.y <= params.depthY[depth]))
				continue;
			const int sourceX = (x - originX) * 256 / scale;
			const int source = sourceY * _width + sourceX;
			uint color = palette[_pixels[source]], opacity = _opacity[source], shadow = _shadow[source];
			if (params.antialias && scale < 256) {
				// Native 0x426040 averages color, opacity and shadow over
				// each source footprint; 0x426de0 takes a single sample.
				const int endX = MIN<int>(_width, (x - originX + 1) * 256 / scale);
				const int endY = MIN<int>(_height, (y - originY + 1) * 256 / scale);
				uint r = 0, g = 0, b = 0;
				opacity = shadow = 0;
				for (int sy = sourceY; sy < endY; ++sy) {
					for (int sx = sourceX; sx < endX; ++sx) {
						const uint sample = sy * _width + sx;
						const uint16 c = _opacity[sample] ? palette[_pixels[sample]] : 0;
						r += c >> 11;
						g += (c >> 5) & 63;
						b += c & 31;
						opacity += _opacity[sample];
						shadow += _shadow[sample];
					}
				}
				const uint count = (endX - sourceX) * (endY - sourceY);
				color = ((r / count) << 11) | ((g / count) << 5) | (b / count);
				opacity /= count;
				shadow /= count;
			}
			if (shadow)
				*out = blend565(0, *out, shadow / 2);
			if (opacity)
				*out = blend565(color, *out, opacity);
		}
	}
}

} // End of namespace Hollywood
