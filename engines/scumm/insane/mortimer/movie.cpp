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

#include "scumm/insane/mortimer/movie.h"
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

// Codec37's existing decoder has no input-length argument. Validate the
// encodings present in the demo movies before passing any data to it.
bool validBackground(const byte *src, uint32 size, int width, int height) {
	if (size < 16 || src[1] != 1 || (src[12] & ~6))
		return false;
	uint32 pos = 16;
	if (src[0] == 2) {
		if (READ_LE_UINT32(src + 4) != uint(width * height))
			return false;
		uint remaining = uint(width * height);
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
	} else if (src[0] == 3 || src[0] == 4) {
		const uint blocks = uint(width * height) / 16;
		for (uint block = 0; block < blocks; ++block) {
			if (pos == size)
				return false;
			const byte code = src[pos++];
			if (src[0] == 4 && code == 0) {
				if (pos == size)
					return false;
				const uint run = src[pos++] + 1;
				if (run > blocks - block)
					return false;
				block += run - 1;
				continue;
			}
			const uint count = code == 0xff ? 16 : (src[12] & 4) && code == 0xfe ? 4 : (src[12] & 4) && code == 0xfd ? 1 : 0;
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
		return !(width & 3) && !(height & 3) && validBackground(data + 14, size - 14, width, height);
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

bool loadMortimerSprites(Common::SeekableReadStream &stream, Common::Array<MortimerSprite> &sprites, byte *palette) {
	Common::Array<byte> data;
	sprites.clear();
	if (!readData(stream, data) || !isContainer(data, MKTAG('A', 'N', 'I', 'M')))
		return false;
	uint32 pos = 8;
	while (pos < data.size()) {
		Chunk frame;
		if (!nextChunk(data.begin(), data.size(), pos, frame))
			return false;
		if (palette && frame.tag == MKTAG('A', 'H', 'D', 'R')) {
			if (frame.size < 774)
				return false;
			memcpy(palette, data.begin() + frame.offset + 6, 768);
		}
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

MortimerMovie::MortimerMovie() : _decoder(nullptr), _decoderWidth(0), _decoderHeight(0),
	_doubled(true), _initialDoubled(true), _storeNext(false), _nextFrame(0), _frameRate(10) {
	_pixels.resize(kScreenWidth * kScreenHeight);
	_previous.resize(_pixels.size());
	memset(_initialPalette, 0, sizeof(_initialPalette));
	reset();
}

MortimerMovie::~MortimerMovie() {
	delete _decoder;
}

bool MortimerMovie::fail(const char *message) {
	_error = message;
	_frames.clear();
	return false;
}

bool MortimerMovie::load(Common::SeekableReadStream &stream, bool doubled) {
	_initialDoubled = doubled;
	_frames.clear();
	_sounds.clear();
	_error.clear();
	reset();
	if (!readData(stream, _data) || !isContainer(_data, MKTAG('A', 'N', 'I', 'M')))
		return fail("Invalid Mortimer movie ANIM container");

	struct Track {
		uint16 id, flags, nextBlock;
		MortimerSound sound;
		Common::Array<byte> bytes;
	};
	Common::Array<Track> tracks;
	uint32 pos = 8;
	uint expectedFrames = 0;
	int decoderWidth = 0, decoderHeight = 0;
	while (pos < _data.size()) {
		Chunk chunk;
		if (!nextChunk(_data.begin(), _data.size(), pos, chunk))
			return fail("Truncated Mortimer movie chunk");
		const byte *src = _data.begin() + chunk.offset;
		if (chunk.tag == MKTAG('A', 'H', 'D', 'R')) {
			if (expectedFrames || chunk.size < 778 || READ_LE_UINT16(src) != 2)
				return fail("Unsupported Mortimer movie header");
			expectedFrames = READ_LE_UINT16(src + 2);
			_frameRate = READ_LE_UINT32(src + 774);
			if (!expectedFrames || _frameRate != 10)
				return fail("Unsupported Mortimer movie timing");
			memcpy(_initialPalette, src + 6, sizeof(_initialPalette));
			memcpy(_palette, _initialPalette, sizeof(_palette));
		} else if (chunk.tag == MKTAG('F', 'R', 'M', 'E')) {
			if (!expectedFrames)
				return fail("Mortimer movie has no header");
			uint32 subpos = chunk.offset;
			uint backgrounds = 0;
			while (subpos < chunk.offset + chunk.size) {
				Chunk sub;
				if (!nextChunk(_data.begin(), chunk.offset + chunk.size, subpos, sub))
					return fail("Truncated Mortimer movie frame");
				const byte *body = _data.begin() + sub.offset;
				if (sub.tag == MKTAG('F', 'O', 'B', 'J')) {
					if (!validObject(body, sub.size))
						return fail("Unsupported or truncated Mortimer movie image");
					if (body[0] == 37) {
						++backgrounds;
						const int width = READ_LE_UINT16(body + 6), height = READ_LE_UINT16(body + 8);
						if ((width != decoderWidth || height != decoderHeight) && body[14] != 2)
							return fail("Mortimer movie changes dimensions without a keyframe");
						decoderWidth = width;
						decoderHeight = height;
					}
				} else if (sub.tag == MKTAG('S', 'K', 'I', 'P')) {
					if (sub.size != 4 || READ_LE_UINT16(body) >= 256 || READ_LE_UINT16(body + 2) >= 256)
						return fail("Invalid Mortimer object toggle");
				} else if (sub.tag == MKTAG('I', 'A', 'C', 'T')) {
					if (sub.size < 8)
						return fail("Truncated Mortimer movie action");
					const uint16 code = READ_LE_UINT16(body);
					if ((code >= 2 && code <= 4 && (sub.size < (code == 4 ? 18U : 12U) || READ_LE_UINT16(body + 8) >= 256)) ||
						(code == 5 && sub.size < 28))
						return fail("Invalid Mortimer movie action");
					if (code == 2 && READ_LE_UINT16(body + 4) == 3 && READ_LE_UINT16(body + 10) >= 256)
						return fail("Invalid Mortimer conditional object");
					if (code == 8 && READ_LE_UINT16(body + 2) == 46 && !validTransition(body, sub.size))
						return fail("Invalid Mortimer movie transition");
				} else if (sub.tag == MKTAG('X', 'P', 'A', 'L')) {
					if (!validPalette(body, sub.size))
						return fail("Invalid Mortimer palette change");
				} else if (sub.tag == MKTAG('N', 'P', 'A', 'L')) {
					if (sub.size != 768)
						return fail("Invalid Mortimer palette");
				} else if (sub.tag == MKTAG('T', 'R', 'E', 'S')) {
					if (sub.size != 18)
						return fail("Invalid Mortimer caption");
				} else if (sub.tag == MKTAG('S', 'T', 'O', 'R') || sub.tag == MKTAG('F', 'T', 'C', 'H')) {
					if (sub.size != (sub.tag == MKTAG('S', 'T', 'O', 'R') ? 4U : 6U))
						return fail("Invalid Mortimer background command");
				} else if (sub.tag == MKTAG('P', 'S', 'A', 'D')) {
					if (sub.size < 10)
						return fail("Truncated Mortimer audio block");
					const uint16 id = READ_LE_UINT16(body), block = READ_LE_UINT16(body + 2);
					const uint16 flags = READ_LE_UINT16(body + 6) & 0xc0;
					uint index = 0;
					while (index < tracks.size() && (tracks[index].id != id || tracks[index].flags != flags))
						++index;
					if (index == tracks.size()) {
						Track track;
						track.id = id;
						track.flags = flags;
						track.nextBlock = 0;
						track.sound.frame = _frames.size();
						track.sound.music = (flags & 64) != 0;
						track.sound.speech = (flags & 128) != 0;
						track.sound.volume = MIN(255, body[8] * 2);
						track.sound.balance = body[9] == 128 ? 0 : (int(body[9]) - 64) * 2;
						tracks.push_back(track);
					}
					if (tracks[index].nextBlock++ != block)
						return fail("Missing Mortimer audio block");
					Common::Array<byte> &bytes = tracks[index].bytes;
					const uint oldSize = bytes.size();
					bytes.resize(oldSize + sub.size - 10);
					memcpy(bytes.begin() + oldSize, body + 10, sub.size - 10);
				}
			}
			if (backgrounds > 1)
				return fail("Invalid Mortimer movie background");
			_frames.push_back({chunk.offset, chunk.size});
		}
	}
	if (_frames.size() != expectedFrames || !expectedFrames)
		return fail("Incomplete Mortimer movie");
	for (uint i = 0; i < tracks.size(); ++i) {
		if (!decodeSound(tracks[i].bytes, tracks[i].sound))
			return fail("Unsupported or incomplete Mortimer audio");
		_sounds.push_back(tracks[i].sound);
	}
	return true;
}

void MortimerMovie::reset() {
	delete _decoder;
	_decoder = nullptr;
	_decoderWidth = _decoderHeight = 0;
	_nextFrame = 0;
	_doubled = _initialDoubled;
	_storeNext = false;
	_storedObject = Object();
	_objects.clear();
	_captions.clear();
	_background.clear();
	memcpy(_palette, _initialPalette, sizeof(_palette));
	memset(_paletteDelta, 0, sizeof(_paletteDelta));
	memset(_shiftedPalette, 0, sizeof(_shiftedPalette));
	memset(_pixels.begin(), 0, _pixels.size());
	memset(_previous.begin(), 0, _previous.size());
	for (uint i = 0; i < 256; ++i)
		_remap[i] = i;
}

void MortimerMovie::setBackground(const byte *pixels) {
	_background.resize(_pixels.size());
	memcpy(_background.begin(), pixels, _background.size());
}

void MortimerMovie::handleAction(const byte *data, uint32 size) {
	if (READ_LE_UINT16(data) == 6) {
		const uint16 flags = READ_LE_UINT16(data + 2);
		if (flags == 7 || flags == 38)
			_doubled = flags == 38;
	}
}

int MortimerMovie::objectScale(byte codec) const {
	// SHELL 401392/402430 double codec37 and codec3 images.
	// Codec1 artwork uses native screen coordinates.
	return _doubled && codec != 1 ? 2 : 1;
}

void MortimerMovie::handlePalette(const byte *data, uint32 size) {
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

bool MortimerMovie::decodeObject(Object &object) {
	const byte *src = _data.begin() + object.offset;
	object.codec = src[0];
	object.x = READ_LE_INT16(src + 2);
	object.y = READ_LE_INT16(src + 4);
	object.width = READ_LE_UINT16(src + 6);
	object.height = READ_LE_UINT16(src + 8);
	object.scale = objectScale(object.codec);
	object.transparent = object.codec == 1 || object.codec == 3 ||
		(object.codec == 37 && object.scale == 1 && (src[26] & 2));
	if (object.codec == 23) {
		if (READ_LE_UINT16(src + 12) == 0x100)
			memcpy(_remap, src + 14, sizeof(_remap));
		memcpy(object.remap, _remap, sizeof(_remap));
		return true;
	}
	object.pixels.resize(object.width * object.height);
	if (object.codec == 37) {
		if (_decoderWidth != object.width || _decoderHeight != object.height) {
			if (src[14] != 2)
				return fail("Mortimer movie changes dimensions without a keyframe");
			delete _decoder;
			_decoderWidth = object.width;
			_decoderHeight = object.height;
			_decoder = new SmushDeltaBlocksDecoder(_decoderWidth, _decoderHeight);
		}
		_decoder->decode(object.pixels.begin(), src + 14);
	} else {
		memset(object.pixels.begin(), 0, object.pixels.size());
		smushDecodeRLEOpaque(object.pixels.begin(), src + 14, 0, 0,
			object.width, object.height, object.width, object.size - 14);
	}
	return true;
}

bool MortimerMovie::advance() {
	if (_nextFrame >= _frames.size())
		return false;
	memcpy(_previous.begin(), _pixels.begin(), _pixels.size());
	_objects.clear();
	_captions.clear();
	const Frame &current = _frames[_nextFrame++];
	uint32 pos = current.offset;
	uint16 skipA = 0xffff, skipB = 0;
	while (pos < current.offset + current.size) {
		Chunk chunk;
		if (!nextChunk(_data.begin(), current.offset + current.size, pos, chunk))
			return fail("Truncated Mortimer movie frame");
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
		} else if (chunk.tag == MKTAG('N', 'P', 'A', 'L')) {
			memcpy(_palette, src, sizeof(_palette));
		} else if (chunk.tag == MKTAG('T', 'R', 'E', 'S')) {
			_captions.push_back({READ_LE_INT16(src), READ_LE_INT16(src + 2), READ_LE_UINT16(src + 16)});
		} else if (chunk.tag == MKTAG('S', 'T', 'O', 'R')) {
			_storeNext = true;
		} else if (chunk.tag == MKTAG('F', 'T', 'C', 'H')) {
			if (!_storedObject.size)
				return fail("Mortimer movie fetches an unstored background");
			Object object = _storedObject;
			object.x += READ_LE_INT16(src + 2);
			object.y += READ_LE_INT16(src + 4);
			object.scale = objectScale(object.codec);
			if (object.codec == 37)
				object.transparent = object.scale == 1 && (_data[object.offset + 26] & 2);
			object.skipA = skipA;
			object.skipB = skipB;
			_objects.push_back(object);
		} else if (chunk.tag == MKTAG('F', 'O', 'B', 'J')) {
			Object object;
			object.offset = chunk.offset;
			object.size = chunk.size;
			object.skipA = skipA;
			object.skipB = skipB;
			if (!decodeObject(object))
				return false;
			if (_storeNext) {
				_storedObject = object;
				_storeNext = false;
			}
			_objects.push_back(object);
		}
		skipA = 0xffff;
		skipB = 0;
	}
	render();
	return true;
}

void MortimerMovie::render() {
	// Transparent cutscenes compose over the scene beneath the movie, not
	// over their own previous overlays (notably the rising interview panel).
	// Frames containing only palette or audio commands retain the last image.
	const byte *base = !_background.empty() && !_objects.empty() ? _background.begin() : _previous.begin();
	memcpy(_pixels.begin(), base, _pixels.size());
	for (uint i = 0; i < _objects.size(); ++i) {
		const Object &object = _objects[i];
		if (skipObject(object.skipA, object.skipB))
			continue;
		const int x = object.x * object.scale, y = object.y * object.scale;
		Common::Rect clip(CLIP<int>(x, 0, kScreenWidth), CLIP<int>(y, 0, kScreenHeight),
			CLIP<int>(x + object.width * object.scale, 0, kScreenWidth),
			CLIP<int>(y + object.height * object.scale, 0, kScreenHeight));
		if (clip.isEmpty())
			continue;
		const byte *pixels = object.pixels.begin();
		if (object.codec == 23) {
			_scratch.resize(object.width * object.height);
			memset(_scratch.begin(), 0, _scratch.size());
			for (int row = clip.top; row < clip.bottom; ++row)
				for (int col = clip.left; col < clip.right; ++col)
					_scratch[(row - y) / object.scale * object.width + (col - x) / object.scale] = _pixels[row * kScreenWidth + col];
			const byte *src = _data.begin() + object.offset;
			const uint offset = READ_LE_UINT16(src + 12) == 0x100 ? 270 : 14;
			smushDecodeRA2SkipRemap(_scratch.begin(), src + offset, 0, 0, object.width,
				object.height, object.width, object.size - offset, object.remap, 0);
			pixels = _scratch.begin();
		}
		for (int row = clip.top; row < clip.bottom; ++row) {
			const byte *line = pixels + (row - y) / object.scale * object.width;
			byte *dest = _pixels.begin() + row * kScreenWidth;
			for (int col = clip.left; col < clip.right; ++col) {
				const byte color = line[(col - x) / object.scale];
				// SHELL 410160/401b20 preserve the destination for every
				// zero pixel in both RLE codecs, including literal runs.
				if (!object.transparent || color)
					dest[col] = color;
			}
		}
	}
}

} // End of namespace Scumm
