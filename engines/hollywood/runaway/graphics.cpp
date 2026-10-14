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

bool loadRunawaySurface(uint chunk, int width, int height, Graphics::ManagedSurface &surface) {
	Common::Array<byte> pixels;
	return readRunawayGlobalChunk(chunk, pixels) && decodeSurface(pixels, width, height, surface);
}

bool loadRunawaySurface(ChunkArchive &archive, uint chunk, int width, int height, Graphics::ManagedSurface &surface) {
	Common::Array<byte> pixels;
	return archive.readVariableChunk(chunk, pixels) && decodeSurface(pixels, width, height, surface);
}

bool applyRunawayPatch(const Common::Array<byte> &patch, Graphics::ManagedSurface &surface, int yOffset) {
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
		if (!length || x + length > (uint)surface.w || y < 0 || y >= surface.h || length * 2 > patch.size() - cursor)
			return false;
		uint16 *out = (uint16 *)surface.getBasePtr(x, y);
		for (uint pixel = 0; pixel < length; ++pixel)
			out[pixel] = READ_LE_UINT16(patch.data() + cursor + pixel * 2);
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
			if (x + length > 1376 || y >= 600 || !length || length * 2 > _data.size() - cursor)
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
	if (chunk >= 500 || file.size() < 4010 || !file.seek(10 + chunk * 4))
		return false;
	const uint32 offset = file.readUint32LE();
	if (!file.seek(10 + 500 * 4 + chunk * 4))
		return false;
	const uint32 size = file.readUint32LE();
	if (!size || size > 16 * 1024 * 1024 || offset < 4010 || offset > file.size() || size > file.size() - offset ||
			!file.seek(offset))
		return false;
	data.resize(size);
	return file.read(data.data(), size) == size;
}

bool readRunawayGlobalChunk(uint chunk, Common::Array<byte> &data) {
	Common::File file;
	return file.open("RESOURCE.000") && readGlobalChunk(file, chunk, data);
}

bool RunawayActor::load() {
	Common::File file;
	Common::Array<byte> palette;
	// Native chapter-five actor loader at 0x40b590 selects entry 44. Entry 45
	// is the palette, and 46..53 are the eight banks of 19 walking cels.
	if (!file.open("RESOURCE.000") || !readGlobalChunk(file, 44, _descriptors) ||
			_descriptors.size() != kFacings * kFrames * 32 ||
			!readGlobalChunk(file, 45, palette) || palette.size() != 512)
		return false;
	for (uint i = 0; i < 256; ++i)
		_palette[i] = READ_LE_UINT16(palette.data() + i * 2);
	_pixels.resize(kWidth * kHeight);
	_opacity.resize(kWidth * kHeight);
	_shadow.resize(kWidth * kHeight);
	for (uint facing = 0; facing < kFacings; ++facing) {
		if (!readGlobalChunk(file, 46 + facing, _banks[facing]))
			return false;
		for (uint frame = 0; frame < kFrames; ++frame) {
			if (!setFrame(facing, frame))
				return false;
		}
	}
	return setFrame(3, 0);
}

bool RunawayActor::setFrame(uint facing, uint frame) {
	if (facing >= kFacings || frame >= kFrames)
		return false;
	if (_facing == (int)facing && _frame == (int)frame)
		return true;
	const byte *descriptor = _descriptors.data() + (facing * kFrames + frame) * 32;
	const Common::Array<byte> &bank = _banks[facing];
	uint cursor = READ_LE_UINT32(descriptor);
	const uint end = frame + 1 < kFrames ? READ_LE_UINT32(descriptor + 32) : bank.size();
	if (cursor > end || end > bank.size())
		return false;
	const int left = 168 - (int)READ_LE_UINT32(descriptor + 16);
	const int top = 434 - (int)READ_LE_UINT32(descriptor + 20);
	const uint width = READ_LE_UINT32(descriptor + 24), height = READ_LE_UINT32(descriptor + 28);
	if (left < 0 || top < 0 || width > kWidth || height > kHeight ||
			left + width > kWidth || top + height > kHeight)
		return false;
	_bounds = Common::Rect(left, top, left + width, top + height);
	memset(_pixels.data(), 0, _pixels.size());
	memset(_opacity.data(), 0, _opacity.size());
	memset(_shadow.data(), 0, _shadow.size());
	// Packed streams: color spans (x,y,length), then opacity and shadow spans
	// (x,y,literal,length). A nonliteral mask span expands to opaque bytes.
	const uint counts[] = { READ_LE_UINT32(descriptor + 4), READ_LE_UINT32(descriptor + 12),
		READ_LE_UINT32(descriptor + 8) };
	for (uint plane = 0; plane < 3; ++plane) {
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
			if (!length || x + length > kWidth || y >= kHeight || (literal && length > end - cursor))
				return false;
			if (literal) {
				memcpy(destination + y * kWidth + x, bank.data() + cursor, length);
				cursor += length;
			} else {
				memset(destination + y * kWidth + x, 255, length);
			}
		}
	}
	if (cursor != end)
		return false;
	_facing = facing;
	_frame = frame;
	return true;
}

static uint16 blend565(uint16 source, uint16 destination, uint alpha) {
	const uint red = ((source >> 11) * alpha + (destination >> 11) * (255 - alpha)) / 255;
	const uint green = (((source >> 5) & 63) * alpha + ((destination >> 5) & 63) * (255 - alpha)) / 255;
	const uint blue = ((source & 31) * alpha + (destination & 31) * (255 - alpha)) / 255;
	return (red << 11) | (green << 5) | blue;
}

void RunawayActor::draw(Graphics::ManagedSurface &surface, Common::Point position, int cameraX,
		int roomTop, const Common::Array<byte> &mask, const Common::Array<byte> &maps, bool sunlit) const {
	// Perspective planes from the demo's scene callback at 0x438970. Actor
	// coordinates are anchored at (168,434) in a nominal 455-pixel-high image.
	float height;
	if (position.x > 1079 && position.y < 542)
		height = position.y * 0.98648649f - 93.675674f;
	else if (position.x > 1215)
		height = position.x * 0.2857143f + 93.85714f;
	else
		height = (position.y * 1.47f - 348.04f + position.x * 0.058927f + 362.462616f) * 0.5f;
	const int scale = CLIP<int>((int)(height * 256 / 455), 64, 256);
	const int originX = position.x - cameraX - 168 * scale / 256;
	const int originY = position.y + roomTop - 434 * scale / 256;
	const int x0 = MAX(0, originX + _bounds.left * scale / 256);
	const int y0 = MAX(roomTop, originY + _bounds.top * scale / 256);
	const int x1 = MIN<int>(surface.w, originX + _bounds.right * scale / 256);
	const int y1 = MIN<int>(roomTop + 600, originY + _bounds.bottom * scale / 256);
	if (x0 >= x1 || y0 >= y1)
		return;
	// Scene lighting and occlusion come from the same pixel lookup as hotspots.
	const byte light = MIN<byte>(5, maps[768 + mask[position.y * 1376 + position.x]]);
	byte lights[6][3] = { {100,100,100}, {55,50,45}, {55,50,45},
		{85,80,75}, {75,70,65}, {70,65,60} };
	if (sunlit) {
		const byte windowLight[2][3] = {{90, 85, 80}, {80, 75, 70}};
		memcpy(lights[1], windowLight, sizeof(windowLight));
	}
	uint16 palette[256];
	for (uint i = 0; i < 256; ++i) {
		const uint16 c = _palette[i];
		palette[i] = (((c >> 11) * lights[light][0] / 100) << 11) |
			((((c >> 5) & 63) * lights[light][1] / 100) << 5) | ((c & 31) * lights[light][2] / 100);
	}
	for (int y = y0; y < y1; ++y) {
		uint16 *out = (uint16 *)surface.getBasePtr(x0, y);
		const int sourceY = (y - originY) * 256 / scale;
		for (int x = x0; x < x1; ++x, ++out) {
			if (maps[512 + mask[(y - roomTop) * 1376 + x + cameraX]])
				continue;
			const int source = sourceY * kWidth + (x - originX) * 256 / scale;
			if (_shadow[source])
				*out = blend565(0, *out, _shadow[source] / 2);
			if (_opacity[source])
				*out = blend565(palette[_pixels[source]], *out, _opacity[source]);
		}
	}
}

} // End of namespace Hollywood
