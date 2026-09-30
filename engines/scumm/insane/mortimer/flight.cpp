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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/insane/mortimer/flight.h"
#include "scumm/smush/codec37.h"
#include "scumm/smush/rebel/codec_ra2.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"

namespace Scumm {

namespace {

struct Chunk {
	uint32 tag, offset, size;
};

bool nextChunk(const byte *data, uint32 end, uint32 &pos, Chunk &chunk, bool padded = true) {
	if (pos > end || end - pos < 8)
		return false;
	chunk.tag = READ_BE_UINT32(data + pos);
	chunk.size = READ_BE_UINT32(data + pos + 4);
	chunk.offset = pos + 8;
	if (chunk.size > end - chunk.offset)
		return false;
	pos = chunk.offset + chunk.size;
	if (padded && (chunk.size & 1)) {
		if (pos == end)
			return false;
		++pos;
	}
	return true;
}

bool readData(Common::SeekableReadStream &stream, Common::Array<byte> &data) {
	const int64 size = stream.size();
	if (size < 8 || size > 64 * 1024 * 1024 || !stream.seek(0))
		return false;
	data.resize(size);
	return stream.read(data.begin(), data.size()) == data.size();
}

bool isContainer(const Common::Array<byte> &data, uint32 tag) {
	return data.size() >= 8 && READ_BE_UINT32(data.begin()) == tag &&
		READ_BE_UINT32(data.begin() + 4) == data.size() - 8;
}

bool validRows(const byte *src, uint32 size, int height, bool remap) {
	uint32 pos = 0;
	// Remap masks can include additional rows below the FOBJ clipping height.
	for (int y = 0; y < height || (remap && pos + 1 < size); ++y) {
		if (size - pos < 2)
			return false;
		const uint len = READ_LE_UINT16(src + pos);
		pos += 2;
		if (len > size - pos)
			return false;
		const uint32 end = pos + len;
		if (remap) {
			// Skip/run pairs; a final skip without a run is also permitted.
			pos = end;
		} else {
			while (pos < end) {
				const byte code = src[pos++];
				const uint count = (code & 1) ? 1 : (code >> 1) + 1;
				if (count > end - pos)
					return false;
				pos += count;
			}
		}
	}
	// FOBJ includes the encoder's final byte of alignment in its payload size.
	return pos == size || (pos + 1 == size && src[pos] == 0);
}

// Codec37's existing decoder has no input-length argument. Validate the two
// encodings present in the demo flights before passing any data to it.
bool validBackground(const byte *src, uint32 size) {
	if (size < 16 || src[1] != 1 || src[12] != 4)
		return false;
	uint32 pos = 16;
	if (src[0] == 2) {
		if (READ_LE_UINT32(src + 4) != MortimerFlight::kWidth * MortimerFlight::kHeight)
			return false;
		uint remaining = MortimerFlight::kWidth * MortimerFlight::kHeight;
		while (remaining) {
			if (pos == size)
				return false;
			const byte code = src[pos++];
			const uint run = (code >> 1) + 1;
			const uint count = (code & 1) ? 1 : run;
			if (count > size - pos || run > remaining)
				return false;
			pos += count;
			remaining -= run;
		}
	} else if (src[0] == 3) {
		for (uint block = 0; block < MortimerFlight::kWidth * MortimerFlight::kHeight / 16; ++block) {
			if (pos == size)
				return false;
			const byte code = src[pos++];
			const uint count = code == 0xff ? 16 : code == 0xfe ? 4 : code == 0xfd ? 1 : 0;
			if (count > size - pos)
				return false;
			pos += count;
		}
	} else {
		return false;
	}
	return pos <= size;
}

bool validObject(const byte *data, uint32 size, bool sprite = false) {
	if (size < 14)
		return false;
	const int width = READ_LE_INT16(data + 6), height = READ_LE_INT16(data + 8);
	if (width <= 0 || height <= 0 || width > 640 || height > 480)
		return false;
	if (READ_LE_INT16(data + 2) + width > 32767 || READ_LE_INT16(data + 4) + height > 32767)
		return false;
	const byte codec = data[0];
	if (sprite)
		return codec == 1 && validRows(data + 14, size - 14, height, false);
	if (codec == 37)
		return width == MortimerFlight::kWidth && height == MortimerFlight::kHeight &&
			READ_LE_UINT32(data + 2) == 0 && validBackground(data + 14, size - 14);
	if (codec != 1 && codec != 3 && codec != 23)
		return false;
	uint32 offset = 14;
	if (codec == 23) {
		const uint16 mode = READ_LE_UINT16(data + 12);
		if (mode != 0x100 && mode != 0x101)
			return false;
		if (mode == 0x100)
			offset += 256;
	}
	return offset <= size && validRows(data + offset, size - offset, height, codec == 23);
}

bool validPalette(const byte *data, uint32 size) {
	return (size == 2308 && READ_BE_UINT32(data) == 2) ||
		(size == 6 && READ_BE_UINT32(data) == 1);
}

bool validTransition(const byte *data, uint32 size) {
	// IACT 8/46 contains a length-prefixed filename followed by CR/LF/EOF.
	if (size < 22 || READ_LE_UINT16(data + 14) != size - 18 ||
		data[size - 3] != '\r' || data[size - 2] != '\n' || data[size - 1] != 0x1a)
		return false;
	for (uint i = 18; i < size - 3; ++i)
		if (data[i] < 32 || data[i] > 126)
			return false;
	return true;
}

bool decodeSound(const Common::Array<byte> &data, MortimerSound &sound) {
	if (!isContainer(data, MKTAG('S', 'A', 'U', 'D')))
		return false;
	sound.samples.clear();
	uint32 pos = 8;
	while (pos < data.size()) {
		Chunk chunk;
		// SAUD chunks are not word-aligned, unlike the surrounding SMUSH file.
		if (!nextChunk(data.begin(), data.size(), pos, chunk, false))
			return false;
		const byte *src = data.begin() + chunk.offset;
		if (chunk.tag == MKTAG('S', 'D', 'A', 'T')) {
			if (!sound.samples.empty())
				return false;
			sound.samples.assign(src, src + chunk.size);
		} else if (chunk.tag == MKTAG('S', 'H', 'D', 'R') && chunk.size == 4) {
			sound.rate = READ_LE_UINT32(src);
		} else if (chunk.tag == MKTAG('S', 'T', 'R', 'K') && chunk.size >= 14 && src[0] == 6 && src[1] == 12) {
			sound.rate = READ_BE_UINT32(src + 10);
		}
	}
	return !sound.samples.empty() && sound.rate >= 4000 && sound.rate <= 48000;
}

} // End of anonymous namespace

bool loadMortimerSprites(Common::SeekableReadStream &stream, Common::Array<MortimerSprite> &sprites) {
	Common::Array<byte> data;
	sprites.clear();
	if (!readData(stream, data) || !isContainer(data, MKTAG('A', 'N', 'I', 'M')))
		return false;
	uint32 pos = 8;
	while (pos < data.size()) {
		Chunk frame;
		if (!nextChunk(data.begin(), data.size(), pos, frame))
			return false;
		if (frame.tag != MKTAG('F', 'R', 'M', 'E'))
			continue;
		uint32 subpos = frame.offset;
		const uint before = sprites.size();
		while (subpos < frame.offset + frame.size) {
			Chunk chunk;
			if (!nextChunk(data.begin(), frame.offset + frame.size, subpos, chunk))
				return false;
			if (chunk.tag != MKTAG('F', 'O', 'B', 'J'))
				continue;
			const byte *src = data.begin() + chunk.offset;
			if (!validObject(src, chunk.size, true))
				return false;
			MortimerSprite sprite;
			sprite.x = READ_LE_INT16(src + 2);
			sprite.y = READ_LE_INT16(src + 4);
			sprite.width = READ_LE_INT16(src + 6);
			sprite.height = READ_LE_INT16(src + 8);
			sprite.pixels.resize(sprite.width * sprite.height);
			memset(sprite.pixels.begin(), 0, sprite.pixels.size());
			smushDecodeRLEOpaque(sprite.pixels.begin(), src + 14, 0, 0, sprite.width, sprite.height, sprite.width, chunk.size - 14);
			sprites.push_back(sprite);
		}
		if (sprites.size() != before + 1)
			return false;
	}
	return !sprites.empty();
}

bool loadMortimerSound(Common::SeekableReadStream &stream, MortimerSound &sound) {
	Common::Array<byte> data;
	return readData(stream, data) && decodeSound(data, sound);
}

MortimerFlight::MortimerFlight() : _decoder(nullptr), _nextFrame(0), _frameRate(10), _rescuedCount(0), _rescuedSpecies(0) {
	_background.resize(kWidth * kHeight);
	_pixels.resize(kWidth * kHeight);
	memset(_initialPalette, 0, sizeof(_initialPalette));
	reset();
}

MortimerFlight::~MortimerFlight() {
	delete _decoder;
}

bool MortimerFlight::fail(const char *message) {
	_error = message;
	_frames.clear();
	return false;
}

bool MortimerFlight::load(Common::SeekableReadStream &stream) {
	_frames.clear();
	_sounds.clear();
	_error.clear();
	reset();
	if (!readData(stream, _data) || !isContainer(_data, MKTAG('A', 'N', 'I', 'M')))
		return fail("Invalid Mortimer flight ANIM container");

	struct Track {
		uint16 id, nextBlock;
		MortimerSound sound;
		Common::Array<byte> bytes;
	};
	Common::Array<Track> tracks;
	uint32 pos = 8;
	uint expectedFrames = 0;
	while (pos < _data.size()) {
		Chunk chunk;
		if (!nextChunk(_data.begin(), _data.size(), pos, chunk))
			return fail("Truncated Mortimer flight chunk");
		const byte *src = _data.begin() + chunk.offset;
		if (chunk.tag == MKTAG('A', 'H', 'D', 'R')) {
			if (expectedFrames || chunk.size < 778 || READ_LE_UINT16(src) != 2)
				return fail("Unsupported Mortimer flight header");
			expectedFrames = READ_LE_UINT16(src + 2);
			_frameRate = READ_LE_UINT32(src + 774);
			if (!expectedFrames || _frameRate != 10)
				return fail("Unsupported Mortimer flight timing");
			memcpy(_initialPalette, src + 6, sizeof(_initialPalette));
			memcpy(_palette, _initialPalette, sizeof(_palette));
		} else if (chunk.tag == MKTAG('F', 'R', 'M', 'E')) {
			if (!expectedFrames)
				return fail("Mortimer flight has no header");
			uint32 subpos = chunk.offset;
			uint backgrounds = 0;
			while (subpos < chunk.offset + chunk.size) {
				Chunk sub;
				if (!nextChunk(_data.begin(), chunk.offset + chunk.size, subpos, sub))
					return fail("Truncated Mortimer flight frame");
				const byte *body = _data.begin() + sub.offset;
				if (sub.tag == MKTAG('F', 'O', 'B', 'J')) {
					if (!validObject(body, sub.size))
						return fail("Unsupported or truncated Mortimer flight image");
					if (body[0] == 37) {
						++backgrounds;
						if (_frames.empty() && body[14] != 2)
							return fail("Mortimer flight does not start with a keyframe");
					}
				} else if (sub.tag == MKTAG('S', 'K', 'I', 'P')) {
					if (sub.size != 4 || READ_LE_UINT16(body) >= 256 || READ_LE_UINT16(body + 2) >= 256)
						return fail("Invalid Mortimer object toggle");
				} else if (sub.tag == MKTAG('I', 'A', 'C', 'T')) {
					if (sub.size < 8)
						return fail("Truncated Mortimer flight action");
					const uint16 code = READ_LE_UINT16(body);
					if ((code >= 2 && code <= 4 && (sub.size < (code == 4 ? 18U : 12U) || READ_LE_UINT16(body + 8) >= 256)) ||
						(code == 5 && sub.size < 28))
						return fail("Invalid Mortimer flight action");
					if (code == 2 && READ_LE_UINT16(body + 4) == 3 && READ_LE_UINT16(body + 10) >= 256)
						return fail("Invalid Mortimer conditional object");
					if (code == 8 && READ_LE_UINT16(body + 2) == 46 && !validTransition(body, sub.size))
						return fail("Invalid Mortimer flight transition");
				} else if (sub.tag == MKTAG('X', 'P', 'A', 'L')) {
					if (!validPalette(body, sub.size))
						return fail("Invalid Mortimer palette change");
				} else if (sub.tag == MKTAG('P', 'S', 'A', 'D')) {
					if (sub.size < 10)
						return fail("Truncated Mortimer speech block");
					const uint16 id = READ_LE_UINT16(body), block = READ_LE_UINT16(body + 2);
					uint index = 0;
					while (index < tracks.size() && tracks[index].id != id)
						++index;
					if (index == tracks.size()) {
						Track track;
						track.id = id;
						track.nextBlock = 0;
						track.sound.frame = _frames.size();
						track.sound.music = (READ_LE_UINT16(body + 6) & 64) != 0;
						track.sound.volume = MIN(255, body[8] * 2);
						track.sound.balance = body[9] == 128 ? 0 : (int(body[9]) - 64) * 2;
						tracks.push_back(track);
					}
					if (tracks[index].nextBlock++ != block)
						return fail("Missing Mortimer speech block");
					Common::Array<byte> &bytes = tracks[index].bytes;
					const uint oldSize = bytes.size();
					bytes.resize(oldSize + sub.size - 10);
					memcpy(bytes.begin() + oldSize, body + 10, sub.size - 10);
				}
			}
			// The final Big Tree frame contains an opaque RLE image only.
			if (backgrounds > 1 || (_frames.empty() && backgrounds != 1))
				return fail("Invalid Mortimer flight background");
			_frames.push_back({chunk.offset, chunk.size});
		}
	}
	if (_frames.size() != expectedFrames || !expectedFrames)
		return fail("Incomplete Mortimer flight");
	for (uint i = 0; i < tracks.size(); ++i) {
		if (!decodeSound(tracks[i].bytes, tracks[i].sound))
			return fail("Unsupported or incomplete Mortimer speech");
		_sounds.push_back(tracks[i].sound);
	}
	return true;
}

void MortimerFlight::reset() {
	delete _decoder;
	_decoder = new SmushDeltaBlocksDecoder(kWidth, kHeight);
	_nextFrame = _rescuedCount = _rescuedSpecies = 0;
	_nextFlight.clear();
	_weapon = kRevitascope;
	memcpy(_palette, _initialPalette, sizeof(_palette));
	memset(_paletteDelta, 0, sizeof(_paletteDelta));
	memset(_shiftedPalette, 0, sizeof(_shiftedPalette));
	_targets.clear();
	_objects.clear();
	_obstacles.clear();
	memset(_toggles, 0, sizeof(_toggles));
	memset(_background.begin(), 0, _background.size());
	memset(_pixels.begin(), 0, _pixels.size());
	for (uint i = 0; i < 256; ++i)
		_remap[i] = i;
}

MortimerFlight::Target *MortimerFlight::findTarget(uint16 id) {
	for (uint i = 0; i < _targets.size(); ++i)
		if (_targets[i].id == id)
			return &_targets[i];
	return nullptr;
}

void MortimerFlight::handleAction(const byte *data, uint32 size) {
	const uint16 code = READ_LE_UINT16(data), flags = READ_LE_UINT16(data + 2);
	const uint16 mode = READ_LE_UINT16(data + 4), id = READ_LE_UINT16(data + 6);
	if (code >= 2 && code <= 4 && mode == 1) {
		const uint16 toggle = READ_LE_UINT16(data + 8);
		if (id == 2000) {
			// Offscreen control object: the remap SKIPs compare against this
			// false toggle, rather than against the individual stone objects.
			_toggles[toggle] = false;
			return;
		}
		if (id >= 1000) {
			Target *target = findTarget(id - 1000);
			if (target)
				target->remapToggle = toggle;
			// Keep the remapped silhouette for the duration of its NUT effect.
			const uint duration = target && target->hitWeapon == kSlimeGun ? 7 : 11;
			_toggles[toggle] = !target || !target->hit || frame() - target->rescuedAt >= duration;
			return;
		}
		Target *target = findTarget(id);
		if (!target && code == 2) {
			Target fresh;
			fresh.id = id;
			fresh.toggle = toggle;
			fresh.remapToggle = 0xffff;
			fresh.rescuedAt = 0;
			fresh.hitWeapon = kRevitascope;
			fresh.active = fresh.hit = false;
			// Salt belongs to the hornet whose ID is 100 lower. Destroying a
			// hornet also disables its subsequent salt objects (SHELL 407c5f).
			if (id >= 800 && id < 900) {
				const Target *hornet = findTarget(id - 100);
				fresh.hit = hornet && hornet->hit;
			}
			_targets.push_back(fresh);
			target = &_targets.back();
		}
		if (!target)
			return;
		target->toggle = toggle;
		_toggles[toggle] = target->hit;
		if (code == 4) {
			const int x = READ_LE_INT16(data + 10), y = READ_LE_INT16(data + 12);
			const int w = READ_LE_INT16(data + 14), h = READ_LE_INT16(data + 16);
			target->rect = Common::Rect(CLIP<int>(x, 0, kWidth), CLIP<int>(y, 0, kHeight),
				CLIP<int>(x + MAX(0, w), 0, kWidth), CLIP<int>(y + MAX(0, h), 0, kHeight));
			target->active = !target->rect.isEmpty();
		} else if (code == 3) {
			target->active = false;
		}
	} else if (code == 2 && mode == 3) {
		_toggles[READ_LE_UINT16(data + 8)] = !_toggles[READ_LE_UINT16(data + 10)];
	} else if (code == 5 && flags == 14 && size >= 28) {
		Obstacle obstacle;
		for (uint i = 0; i < 4; ++i)
			obstacle.corners[i] = Common::Point(READ_LE_INT16(data + 12 + i * 4), READ_LE_INT16(data + 14 + i * 4));
		_obstacles.push_back(obstacle);
	} else if (code == 6 && (flags == 9 || flags == 10)) {
		// SHELL 407c5f calls 40b844/40b7ce at the scripted weapon changes.
		_weapon = flags == 9 ? kSlimeGun : kRevitascope;
	} else if (code == 8 && flags == 46) {
		_nextFlight = Common::String((const char *)data + 18, size - 21);
		for (uint i = 0; i < _nextFlight.size(); ++i)
			if (_nextFlight[i] == '\\')
				_nextFlight.setChar('/', i);
	}
}

void MortimerFlight::handlePalette(const byte *data, uint32 size) {
	if (size == 2308) {
		for (uint i = 0; i < 768; ++i) {
			_shiftedPalette[i] = _palette[i] << 7;
			_paletteDelta[i] = READ_LE_INT16(data + 4 + i * 2);
		}
		memcpy(_palette, data + 1540, sizeof(_palette));
	} else {
		for (uint i = 0; i < 768; ++i) {
			_shiftedPalette[i] += _paletteDelta[i];
			_palette[i] = CLIP<int32>(_shiftedPalette[i] >> 7, 0, 255);
		}
	}
}

bool MortimerFlight::advance() {
	if (_nextFrame >= _frames.size())
		return false;
	_objects.clear();
	_obstacles.clear();
	for (uint i = 0; i < _targets.size(); ++i)
		_targets[i].active = false;
	const Frame &frame = _frames[_nextFrame++];
	uint32 pos = frame.offset;
	uint16 skipA = 0xffff, skipB = 0;
	while (pos < frame.offset + frame.size) {
		Chunk chunk;
		if (!nextChunk(_data.begin(), frame.offset + frame.size, pos, chunk))
			return fail("Truncated Mortimer flight frame");
		const byte *src = _data.begin() + chunk.offset;
		if (chunk.tag == MKTAG('S', 'K', 'I', 'P')) {
			skipA = READ_LE_UINT16(src);
			skipB = READ_LE_UINT16(src + 2);
			continue;
		}
		if (chunk.tag == MKTAG('I', 'A', 'C', 'T')) {
			handleAction(src, chunk.size);
		} else if (chunk.tag == MKTAG('X', 'P', 'A', 'L')) {
			handlePalette(src, chunk.size);
		} else if (chunk.tag == MKTAG('F', 'O', 'B', 'J')) {
			if (src[0] == 37) {
				_decoder->decode(_background.begin(), src + 14);
			} else {
				if (src[0] == 23 && READ_LE_UINT16(src + 12) == 0x100)
					memcpy(_remap, src + 14, sizeof(_remap));
				_objects.push_back({chunk.offset, chunk.size, skipA, skipB});
			}
		}
		skipA = 0xffff;
		skipB = 0;
	}
	render();
	return true;
}

void MortimerFlight::render() {
	memcpy(_pixels.begin(), _background.begin(), _pixels.size());
	for (uint i = 0; i < _objects.size(); ++i) {
		const Object &object = _objects[i];
		if (object.skipA != 0xffff && (object.skipB ? _toggles[object.skipA] != _toggles[object.skipB] : _toggles[object.skipA]))
			continue;
		const byte *src = _data.begin() + object.offset;
		const int x = READ_LE_INT16(src + 2), y = READ_LE_INT16(src + 4);
		const int width = READ_LE_INT16(src + 6), height = READ_LE_INT16(src + 8);
		Common::Rect clip(x, y, x + width, y + height);
		clip.clip(Common::Rect(kWidth, kHeight));
		if (clip.isEmpty())
			continue;
		// Decode into an object-sized buffer so offscreen sprites cannot write
		// beyond the flight surface. Remap objects must start with its pixels.
		_scratch.resize(width * height);
		memset(_scratch.begin(), 0, _scratch.size());
		if (src[0] == 23) {
			for (int row = clip.top; row < clip.bottom; ++row)
				memcpy(_scratch.begin() + (row - y) * width + clip.left - x,
					_pixels.begin() + row * kWidth + clip.left, clip.width());
			const uint offset = READ_LE_UINT16(src + 12) == 0x100 ? 270 : 14;
			smushDecodeRA2SkipRemap(_scratch.begin(), src + offset, 0, 0, width, height, width, object.size - offset, _remap, 0);
		} else {
			smushDecodeRLEOpaque(_scratch.begin(), src + 14, 0, 0, width, height, width, object.size - 14);
		}
		for (int row = clip.top; row < clip.bottom; ++row) {
			const byte *line = _scratch.begin() + (row - y) * width + clip.left - x;
			byte *dest = _pixels.begin() + row * kWidth + clip.left;
			for (int col = 0; col < clip.width(); ++col) {
				// Mortimer treats zero as transparent in literal runs as well.
				if (src[0] == 1 || src[0] == 23 || line[col])
					dest[col] = line[col];
			}
		}
	}
}

MortimerFlight::ShotResult MortimerFlight::shoot(const Common::Point &point) {
	// Later objects are drawn on top of earlier ones.
	for (int i = int(_targets.size()) - 1; i >= 0; --i) {
		Target &target = _targets[i];
		if (!target.active || target.hit || !target.rect.contains(point))
			continue;
		const uint species = target.id / 100;
		if (!target.id || species > 9 || species == 8 ||
			(species < 7 && _weapon != kRevitascope) || (species == 7 && _weapon != kSlimeGun))
			continue;
		target.hit = true;
		target.rescuedAt = frame();
		target.hitWeapon = _weapon;
		_toggles[target.toggle] = true;
		if (target.remapToggle != 0xffff)
			_toggles[target.remapToggle] = false;
		if (species < 7) {
			++_rescuedCount;
			_rescuedSpecies |= 1 << species;
		} else if (species == 7) {
			Target *salt = findTarget(target.id + 100);
			if (salt) {
				salt->hit = true;
				_toggles[salt->toggle] = true;
			}
		}
		render();
		return species == 9 ? kShellRestored : species == 7 ? kHornetHit : kAnimalRescued;
	}
	return kMiss;
}

bool MortimerFlight::hitsObstacle(const Common::Point &point) const {
	// SHELL 40e390 uses a 50 x 50 footprint in movie coordinates.
	const Common::Rect footprint(point.x - 25, point.y - 25, point.x + 25, point.y + 25);
	for (uint i = 0; i < _targets.size(); ++i) {
		const Target &target = _targets[i];
		if (target.id >= 800 && target.id < 900 && target.active && !target.hit && target.rect.intersects(footprint))
			return true;
	}
	for (uint n = 0; n < _obstacles.size(); ++n) {
		const Obstacle &obstacle = _obstacles[n];
		// Separating axes also handle vertical edges and degenerate quads
		// without the divisions used by the original executable.
		bool separated = false;
		for (uint axis = 0; axis < 6 && !separated; ++axis) {
			int dx = axis == 4 ? 1 : 0, dy = axis == 5 ? 1 : 0;
			if (axis < 4) {
				const Common::Point &a = obstacle.corners[axis], &b = obstacle.corners[(axis + 1) % 4];
				dx = a.y - b.y;
				dy = b.x - a.x;
			}
			int64 minimum = int64(dx) * obstacle.corners[0].x + int64(dy) * obstacle.corners[0].y;
			int64 maximum = minimum;
			for (uint i = 1; i < 4; ++i) {
				const int64 projection = int64(dx) * obstacle.corners[i].x + int64(dy) * obstacle.corners[i].y;
				minimum = MIN(minimum, projection);
				maximum = MAX(maximum, projection);
			}
			const int64 center = int64(dx) * point.x + int64(dy) * point.y;
			const int64 radius = 25 * (int64(ABS(dx)) + ABS(dy));
			separated = center + radius < minimum || center - radius > maximum;
		}
		if (!separated)
			return true;
	}
	return false;
}

} // End of namespace Scumm
