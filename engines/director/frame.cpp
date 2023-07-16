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

#include "common/memstream.h"
#include "common/substream.h"

#include "director/director.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/movie.h"
#include "director/sprite.h"
#include "director/util.h"

namespace Director {

Frame::Frame(Score *score, int numChannels) {
	_score = score;
	_vm = score->getMovie()->getVM();
	_numChannels = numChannels;

	reset();
}

void Frame::reset() {
	// Reset main channels
	_mainChannels = MainChannels();

	_sprites.resize(_numChannels + 1);

	for (uint16 i = 0; i < _sprites.size(); i++) {
		if (_sprites[i])
			delete _sprites[i];

		Sprite *sp = new Sprite(this);
		_sprites[i] = sp;
	}
}

Frame::Frame(const Frame &frame) {
	_vm = frame._vm;
	_numChannels = frame._numChannels;
	_mainChannels.actionId = frame._mainChannels.actionId;
	_mainChannels.transArea = frame._mainChannels.transArea;
	_mainChannels.transDuration = frame._mainChannels.transDuration;
	_mainChannels.transType = frame._mainChannels.transType;
	_mainChannels.transChunkSize = frame._mainChannels.transChunkSize;
	_mainChannels.tempo = frame._mainChannels.tempo;
	_mainChannels.scoreCachedTempo = frame._mainChannels.scoreCachedTempo;
	_mainChannels.sound1 = frame._mainChannels.sound1;
	_mainChannels.sound2 = frame._mainChannels.sound2;
	_mainChannels.soundType1 = frame._mainChannels.soundType1;
	_mainChannels.soundType2 = frame._mainChannels.soundType2;
	_mainChannels.skipFrameFlag = frame._mainChannels.skipFrameFlag;
	_mainChannels.blend = frame._mainChannels.blend;

	_mainChannels.colorTempo = frame._mainChannels.colorTempo;
	_mainChannels.colorSound1 = frame._mainChannels.colorSound1;
	_mainChannels.colorSound2 = frame._mainChannels.colorSound2;
	_mainChannels.colorScript = frame._mainChannels.colorScript;
	_mainChannels.colorTrans = frame._mainChannels.colorTrans;

	_mainChannels.palette = frame._mainChannels.palette;

	_score = frame._score;

	debugC(1, kDebugLoading, "Frame. action: %s transType: %d transDuration: %d", _mainChannels.actionId.asString().c_str(), _mainChannels.transType, _mainChannels.transDuration);

	_sprites.resize(_numChannels + 1);

	for (uint16 i = 0; i <= _numChannels; i++) {
		_sprites[i] = new Sprite(*frame._sprites[i]);
	}
}

Frame::~Frame() {
	for (uint16 i = 0; i < _sprites.size(); i++)
		delete _sprites[i];
}

void Frame::readChannel(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size, uint16 version) {
	debugC(6, kDebugLoading, "Frame::readChannel(..., offset=%d, size=%d, version=%x)", offset, size, version);

	if (version < kFileVer400) {
		readChannelD2(stream, offset, size);
	} else if (version >= kFileVer400 && version < kFileVer500) {
		readChannelD4(stream, offset, size);
	} else if (version >= kFileVer500 && version < kFileVer600) {
		readChannelD5(stream, offset, size);
	} else if (version >= kFileVer600 && version < kFileVer700) {
		readChannelD6(stream, offset, size);
	} else {
		error("Frame::readChannel(): Unsupported Director version: %d", version);
	}
}

/**************************
 *
 * D2 Loading
 *
 **************************/

void Frame::readChannelD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (offset < kMainChannelSizeD2) {
		uint16 needSize = MIN(size, (uint16)(kMainChannelSizeD2 - offset));
		readMainChannelsD2(stream, offset, needSize);
		size -= needSize;
		offset += needSize;
	}

	if (offset >= kMainChannelSizeD2) {
		byte spritePosition = (offset - kMainChannelSizeD2) / kSprChannelSizeD2;
		uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD2 + kMainChannelSizeD2;

		while (size > 0) {
			uint16 needSize = MIN((uint16)(nextStart - offset), size);
			readSpriteD2(stream, offset, needSize);
			offset += needSize;
			size -= needSize;
			nextStart += kSprChannelSizeD2;
		}
	}
}

void Frame::readMainChannelsD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readMainChannelsD2(): %d byte header", size);
		stream.hexdump(size);
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;
	byte unk[6];

	while (stream.pos() < finishPosition) {
		switch (stream.pos() - initPos + offset) {
		case 0: // Sound/Tempo/Transition
			_mainChannels.actionId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			break;
		case 1:
			// type: 0x17 for sounds (sound is cast id), 0x16 for MIDI (sound is cmd id)
			_mainChannels.soundType1 = stream.readByte();
			break;
		case 2: {
				// 0x80 is whole stage (vs changed area), rest is duration in 1/4ths of a second
				uint8 transFlags = stream.readByte();
				if (transFlags & 0x80)
					_mainChannels.transArea = 1;
				else
					_mainChannels.transArea = 0;
				_mainChannels.transDuration = (transFlags & 0x7f) * 250; // Duration is in 1/4 secs
			}
			break;
		case 3:
			_mainChannels.transChunkSize = stream.readByte();
			break;
		case 4:
			_mainChannels.tempo = stream.readByte();
			break;
		case 5:
			_mainChannels.transType = static_cast<TransitionType>(stream.readByte());
			break;
		case 6:
			_mainChannels.sound1 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			break;
		case 8:
			_mainChannels.sound2 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			break;
		case 10:
			_mainChannels.soundType2 = stream.readByte();
			break;
		case 11:
			_mainChannels.skipFrameFlag = stream.readByte();
			break;
		case 12:
			_mainChannels.blend = stream.readByte();
			break;
		case 13:
			stream.read(unk, 1);
			debugC(8, kDebugLoading, "Frame::readMainChannelsD2(): STUB: unk1: 0x%02x", unk[0]);
			break;
		case 14:
			stream.read(unk, 2);
			debugC(8, kDebugLoading, "Frame::readMainChannelsD2(): STUB: unk2: 0x%02x 0x%02x", unk[0], unk[1]);
			break;
		case 16: {
				// palette
				int16 paletteId = stream.readSint16();
				if (paletteId == 0) {
					_mainChannels.palette.paletteId = CastMemberID(0, 0);
				} else if (paletteId < 0) {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, -1);
				} else {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
				}
			}
			break;
		case 18:
			// loop points for color cycling
			_mainChannels.palette.firstColor = g_director->transformColor(stream.readByte() ^ 0x80);
			_mainChannels.palette.lastColor = g_director->transformColor(stream.readByte() ^ 0x80);
			break;
		case 20:
			_mainChannels.palette.flags = stream.readByte();
			_mainChannels.palette.colorCycling = (_mainChannels.palette.flags & 0x80) != 0;
			_mainChannels.palette.normal = (_mainChannels.palette.flags & 0x60) == 0x00;
			_mainChannels.palette.fadeToBlack = (_mainChannels.palette.flags & 0x60) == 0x60;
			_mainChannels.palette.fadeToWhite = (_mainChannels.palette.flags & 0x60) == 0x40;
			_mainChannels.palette.autoReverse = (_mainChannels.palette.flags & 0x10) != 0;
			_mainChannels.palette.overTime = (_mainChannels.palette.flags & 0x04) != 0;
			_mainChannels.palette.speed = stream.readByte();
			break;
		case 22:
			_mainChannels.palette.frameCount = stream.readUint16();
			break;
		case 24:
			_mainChannels.palette.cycleCount = stream.readUint16();
			break;
		case 26:
			stream.read(unk, 6);

			debugC(8, kDebugLoading, "Frame::readMainChannelsD2(): STUB: unk1: %02x %02x %02x %02x %02x %02x", unk[0],
				unk[1], unk[2], unk[3], unk[4], unk[5]);
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("Frame::readMainChannelsD2(): Miscomputed field position: %ld", stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD2(): Read %ld extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::readSpriteD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD2) / kSprChannelSizeD2;
	uint16 spriteStart = spritePosition * kSprChannelSizeD2 + kMainChannelSizeD2;

	uint16 fieldPosition = offset - spriteStart;

	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readSpriteD2(): channel %d, 16 bytes", spritePosition);
		stream.hexdump(kSprChannelSizeD2);
	}

	debugC(3, kDebugLoading, "Frame::readSpriteD2(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);

	Sprite &sprite = *_sprites[spritePosition + 1];

	if (sprite._puppet || sprite._autoPuppet) {
		stream.skip(size);
		return;
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD2(stream, sprite, initPos - fieldPosition, finishPosition);

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD2(): Read %ld extra bytes", stream.pos() - finishPosition);
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

void readSpriteDataD2(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition) {
	while (stream.pos() < finishPosition) {
		switch (stream.pos() - startPosition) {
		case 0:
			sprite._scriptId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			break;
		case 1:
			sprite._spriteType = (SpriteType)stream.readByte();

			sprite._enabled = sprite._spriteType != kInactiveSprite;
			break;
		case 2:
			// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
			sprite._foreColor = g_director->transformColor((128 + stream.readByte()) & 0xff);
			break;
		case 3:
			// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
			sprite._backColor = g_director->transformColor((128 + stream.readByte()) & 0xff);
			break;
		case 4:
			sprite._thickness = stream.readByte();
			break;
		case 5:
			sprite._inkData = stream.readByte();

			sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
			if (sprite._inkData & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			break;
		case 6:
			if (sprite.isQDShape()) {
				sprite._pattern = stream.readUint16();
			} else {
				sprite._castId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			}
			break;
		case 8:
			sprite._startPoint.y = (int16)stream.readUint16();
			break;
		case 10:
			sprite._startPoint.x = (int16)stream.readUint16();
			break;
		case 12:
			sprite._height = (int16)stream.readUint16();
			break;
		case 14:
			sprite._width = (int16)stream.readUint16();
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD2(): Miscomputed field position: %ld", stream.pos() - startPosition);
		}
	}

}


/**************************
 *
 * D4 Loading
 *
 **************************/

void Frame::readChannelD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	// 40 bytes header
	if (offset < kMainChannelSizeD4) {
		uint16 needSize = MIN(size, (uint16)(kMainChannelSizeD4 - offset));
		readMainChannelsD4(stream, offset, needSize);
		size -= needSize;
		offset += needSize;
	}

	if (offset >= kMainChannelSizeD4) {
		byte spritePosition = (offset - kMainChannelSizeD4) / kSprChannelSizeD4;
		uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD4 + kMainChannelSizeD4;

		while (size > 0) {
			uint16 needSize = MIN((uint16)(nextStart - offset), size);
			readSpriteD4(stream, offset, needSize);
			offset += needSize;
			size -= needSize;
			nextStart += kSprChannelSizeD4;
		}
	}
}

void Frame::readMainChannelsD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readMainChannelsD4(): %d byte header", size);
		stream.hexdump(size);
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;
	int unk1;

	while (stream.pos() < finishPosition) {
		switch (stream.pos() - initPos + offset) {
		case 0:
			// Sound/Tempo/Transition
			unk1 = stream.readByte();
			if (unk1) {
				warning("Frame::readMainChannelsD4(): STUB: unk1: %d 0x%x", unk1, unk1);
			}
			break;
		case 1:
			_mainChannels.soundType1 = stream.readByte(); // type: 0x17 for sounds (sound is cast id), 0x16 for MIDI (sound is cmd id)
			break;
		case 2: {
				uint8 transFlags = stream.readByte();
				if (transFlags & 0x80)  // 0x80 is whole stage (vs changed area), rest is duration in 1/4ths of a second
					_mainChannels.transArea = 1;
				else
					_mainChannels.transArea = 0;
				_mainChannels.transDuration = (transFlags & 0x7f) * 250; // Duration is in 1/4 secs
			}
			break;
		case 3:
			_mainChannels.transChunkSize = stream.readByte();
			break;
		case 4:
			_mainChannels.tempo = stream.readByte();
			break;
		case 5:
			_mainChannels.transType = static_cast<TransitionType>(stream.readByte());
			break;
		case 6:
			_mainChannels.sound1 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			break;
		case 8:
			_mainChannels.sound2 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			break;
		case 10:
			_mainChannels.soundType2 = stream.readByte();
			break;
		case 11:
			_mainChannels.skipFrameFlag = stream.readByte();
			break;
		case 12:
			_mainChannels.blend = stream.readByte();
			break;
		case 13:
			_mainChannels.colorTempo = stream.readByte();
			break;
		case 14:
			_mainChannels.colorSound1 = stream.readByte();
			break;
		case 15:
			_mainChannels.colorSound2 = stream.readByte();
			break;
		case 16:
			_mainChannels.actionId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			break;
		case 18:
			_mainChannels.colorScript = stream.readByte();
			break;
		case 19:
			_mainChannels.colorTrans = stream.readByte();
			break;
		case 20: {
				// palette, 13 bytes
				int16 paletteId = stream.readSint16();
				if (paletteId == 0) {
					_mainChannels.palette.paletteId = CastMemberID(0, 0);
				} else if (paletteId < 0) {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, -1);
				} else {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
				}
			}
			break;
		case 22:
			// loop points for color cycling
			_mainChannels.palette.firstColor = g_director->transformColor(stream.readByte() + 0x80); // 22
			_mainChannels.palette.lastColor = g_director->transformColor(stream.readByte() + 0x80); // 23
			break;
		case 24:
			_mainChannels.palette.flags = stream.readByte(); // 24
			_mainChannels.palette.colorCycling = (_mainChannels.palette.flags & 0x80) != 0;
			_mainChannels.palette.normal = (_mainChannels.palette.flags & 0x60) == 0x00;
			_mainChannels.palette.fadeToBlack = (_mainChannels.palette.flags & 0x60) == 0x60;
			_mainChannels.palette.fadeToWhite = (_mainChannels.palette.flags & 0x60) == 0x40;
			_mainChannels.palette.autoReverse = (_mainChannels.palette.flags & 0x10) != 0;
			_mainChannels.palette.overTime = (_mainChannels.palette.flags & 0x04) != 0;
			_mainChannels.palette.speed = stream.readByte(); // 25
			break;
		case 26:
			_mainChannels.palette.frameCount = stream.readUint16(); // 26
			break;
		case 28:
			_mainChannels.palette.cycleCount = stream.readUint16(); // 28
			break;
		case 30:
			_mainChannels.palette.fade = stream.readByte(); // 30
			_mainChannels.palette.delay = stream.readByte(); // 31
			_mainChannels.palette.style = stream.readByte(); // 32
			break;
		case 33:
			unk1 = stream.readByte();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk2: %d 0x%x", unk1, unk1);
			break;
		case 34:
			unk1 = stream.readUint16();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk3: %d 0x%x", unk1, unk1);
			break;
		case 36:
			unk1 = stream.readUint16();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk4: %d 0x%x", unk1, unk1);
			break;
		case 38:
			_mainChannels.palette.colorCode = stream.readByte();
			break;
		case 39:
			unk1 = stream.readByte();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk5: 0x%02x", unk1);
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("Frame::readMainChannelsD4(): Miscomputed field position: %ld", stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD4(): Read %ld extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::readSpriteD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD4) / kSprChannelSizeD4;
	uint16 spriteStart = spritePosition * kSprChannelSizeD4 + kMainChannelSizeD4;

	uint16 fieldPosition = offset - spriteStart;

	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readSpriteD4(): channel %d, 20 bytes", spritePosition);
		stream.hexdump(kSprChannelSizeD4);
	}

	debugC(3, kDebugLoading, "Frame::readSpriteD4(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);

	Sprite &sprite = *_sprites[spritePosition + 1];

	if (sprite._puppet || sprite._autoPuppet) {
		stream.skip(size);
		return;
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD4(stream, sprite, initPos - fieldPosition, finishPosition);

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD4(): Read %ld extra bytes", stream.pos() - finishPosition);
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

void readSpriteDataD4(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition) {
	while (stream.pos() < finishPosition) {
		switch (stream.pos() - startPosition) {
		case 0:
			sprite._scriptId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			break;
		case 1:
			sprite._spriteType = (SpriteType)stream.readByte();

			sprite._enabled = sprite._spriteType != kInactiveSprite;
			break;
		case 2:
			sprite._foreColor = g_director->transformColor((uint8)stream.readByte());
			break;
		case 3:
			sprite._backColor = g_director->transformColor((uint8)stream.readByte());
			break;
		case 4:
			sprite._thickness = stream.readByte();
			break;
		case 5:
			sprite._inkData = stream.readByte();

			sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
			if (sprite._inkData & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			break;
		case 6:
			if (sprite.isQDShape()) {
				sprite._pattern = stream.readUint16();
			} else {
				sprite._castId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			}
			break;
		case 8:
			sprite._startPoint.y = (int16)stream.readUint16();
			break;
		case 10:
			sprite._startPoint.x = (int16)stream.readUint16();
			break;
		case 12:
			sprite._height = (int16)stream.readUint16();
			break;
		case 14:
			sprite._width = (int16)stream.readUint16();
			break;
		case 16:
			sprite._scriptId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			break;
		case 18:
			// & 0x0f scorecolor
			// 0x10 forecolor is rgb
			// 0x20 bgcolor is rgb
			// 0x40 editable
			// 0x80 moveable
			sprite._colorcode = stream.readByte();

			sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			break;
		case 19:
			sprite._blendAmount = stream.readByte();
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD4(): Miscomputed field position: %ld", stream.pos() - startPosition);
		}
	}
}

/**************************
 *
 * D5 Loading
 *
 **************************/

void Frame::readChannelD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	// 48 bytes header
	if (offset < kMainChannelSizeD5) {
		uint16 needSize = MIN(size, (uint16)(kMainChannelSizeD5 - offset));
		readMainChannelsD5(stream, offset, needSize);
		size -= needSize;
		offset += needSize;
	}

	if (offset >= kMainChannelSizeD5) {
		byte spritePosition = (offset - kMainChannelSizeD5) / kSprChannelSizeD5;
		uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD5 + kMainChannelSizeD5;

		while (size > 0) {
			uint16 needSize = MIN((uint16)(nextStart - offset), size);
			readSpriteD5(stream, offset, needSize);
			offset += needSize;
			size -= needSize;
			nextStart += kSprChannelSizeD5;
		}
	}
}

void Frame::readMainChannelsD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readMainChannelsD5(): %d byte header", size);
		stream.hexdump(size);
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;
	byte unk[12];

	while (stream.pos() < finishPosition) {
		switch (stream.pos() - initPos + offset) {
		case 0: {
				// Sound/Tempo/Transition
				uint16 actionCastLib = stream.readUint16();
				uint16 actionId = stream.readUint16();
				_mainChannels.actionId = CastMemberID(actionId, actionCastLib);
			}
			break;
		case 2: {
				uint16 actionId = stream.readUint16();
				_mainChannels.actionId = CastMemberID(actionId, _mainChannels.actionId.castLib);  // Inherit castLinb from previous frame
			}
			break;
		case 4: {
				uint16 sound1CastLib = stream.readUint16();
				uint16 sound1Id = stream.readUint16();
				_mainChannels.sound1 = CastMemberID(sound1Id, sound1CastLib);
			}
			break;
		case 6: {
				uint16 sound1Id = stream.readUint16();
				_mainChannels.sound1 = CastMemberID(sound1Id, _mainChannels.sound1.castLib);	// Inherit castLinb from previous frame
			}
			break;
		case 8: {
				uint16 sound2CastLib = stream.readUint16();
				uint16 sound2Id = stream.readUint16();
				_mainChannels.sound2 = CastMemberID(sound2Id, sound2CastLib);
			}
			break;
		case 10: {
				uint16 sound2Id = stream.readUint16();
				_mainChannels.sound2 = CastMemberID(sound2Id, _mainChannels.sound2.castLib);	// Inherit castLinb from previous frame
			}
			break;
		case 12: {
				uint16 transCastLib = stream.readUint16();
				uint16 transId = stream.readUint16();
				_mainChannels.trans = CastMemberID(transId, transCastLib);
			}
			break;
		case 16:
			stream.read(unk, 4);
			warning("Frame::readMainChannelsD5(): STUB: unk1: 0x%02x 0x%02x 0x%02x 0x%02x", unk[0], unk[1], unk[2], unk[3]);
			break;
		case 20:
			stream.read(unk, 1);
			warning("Frame::readMainChannelsD5(): STUB: unk2: 0x%02x", unk[0]);
			break;
		case 21:
			_mainChannels.tempo = stream.readByte();
			break;
		case 22:
			stream.read(unk, 2);
			warning("Frame::readMainChannelsD5(): STUB: unk3: 0x%02x 0x%02x", unk[0], unk[1]);
			break;
		case 24: {
				int16 paletteCastLib = stream.readSint16();
				int16 paletteId = stream.readSint16(); // 26
				_mainChannels.palette.paletteId = CastMemberID(paletteId, paletteCastLib);
			}
			break;
		case 28:
			_mainChannels.palette.speed = stream.readByte(); // 28
			_mainChannels.palette.flags = stream.readByte(); // 29
			_mainChannels.palette.colorCycling = (_mainChannels.palette.flags & 0x80) != 0;
			_mainChannels.palette.normal = (_mainChannels.palette.flags & 0x60) == 0x00;
			_mainChannels.palette.fadeToBlack = (_mainChannels.palette.flags & 0x60) == 0x60;
			_mainChannels.palette.fadeToWhite = (_mainChannels.palette.flags & 0x60) == 0x40;
			_mainChannels.palette.autoReverse = (_mainChannels.palette.flags & 0x10) != 0;
			_mainChannels.palette.overTime = (_mainChannels.palette.flags & 0x04) != 0;
			break;
		case 30:
			_mainChannels.palette.firstColor = g_director->transformColor(stream.readByte() + 0x80); // 30
			_mainChannels.palette.lastColor = g_director->transformColor(stream.readByte() + 0x80); // 31
			break;
		case 32:
			_mainChannels.palette.frameCount = stream.readUint16(); // 32
			break;
		case 34:
			_mainChannels.palette.cycleCount = stream.readUint16(); // 34
			break;
		case 36: {
				stream.read(unk, 12);

				Common::String s;
				for (int i = 0; i < 12; i++)
					s += Common::String::format("0x%02x ", unk[i]);

				warning("Frame::readMainChannelsD5(): STUB: unk4: %s", s.c_str());
			}
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("Frame::readMainChannelsD5(): Miscomputed field position: %ld", stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD5(): Read %ld extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::readSpriteD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD5) / kSprChannelSizeD5;
	uint16 spriteStart = spritePosition * kSprChannelSizeD5 + kMainChannelSizeD5;

	uint16 fieldPosition = offset - spriteStart;

	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readSpriteD5(): channel %d, 20 bytes", spritePosition);
		stream.hexdump(kSprChannelSizeD4);
	}

	debugC(3, kDebugLoading, "Frame::readSpriteD5(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);

	Sprite &sprite = *_sprites[spritePosition + 1];

	if (sprite._puppet || sprite._autoPuppet) {
		stream.skip(size);
		return;
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD5(stream, sprite, initPos - fieldPosition, finishPosition);

	if (fieldPosition > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD5(): Read %ld extra bytes", stream.pos() - finishPosition);
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

void readSpriteDataD5(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition) {
	while (stream.pos() < finishPosition) {
		switch (stream.pos() - startPosition) {
		case 0:
			sprite._spriteType = (SpriteType)stream.readByte();
			break;
		case 1:
			sprite._inkData = stream.readByte();

			sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
			if (sprite._inkData & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			break;
		case 2: {
				uint16 castLib = stream.readUint16();
				uint16 memberID = stream.readUint16();
				sprite._castId = CastMemberID(memberID, castLib);
			}
			break;
		case 4: {
				uint16 memberID = stream.readUint16();
				sprite._castId = CastMemberID(memberID, sprite._castId.castLib);  // Inherit castLinb from previous frame
			}
			break;
		case 6: {
				uint16 scriptCastLib = stream.readUint16();
				uint16 scriptMemberID = stream.readUint16();
				sprite._scriptId = CastMemberID(scriptMemberID, scriptCastLib);
			}
			break;
		case 8: {
				uint16 scriptMemberID = stream.readUint16();
				sprite._scriptId = CastMemberID(scriptMemberID, sprite._scriptId.castLib);  // Inherit castLinb from previous frame
			}
			break;
		case 10:
			sprite._foreColor = g_director->transformColor((uint8)stream.readByte());
			break;
		case 11:
			sprite._backColor = g_director->transformColor((uint8)stream.readByte());
			break;
		case 12:
			sprite._startPoint.y = (int16)stream.readUint16();
			break;
		case 14:
			sprite._startPoint.x = (int16)stream.readUint16();
			break;
		case 16:
			sprite._height = (int16)stream.readUint16();
			break;
		case 18:
			sprite._width = (int16)stream.readUint16();
			break;
		case 20:
			// & 0x0f scorecolor
			// 0x10 forecolor is rgb
			// 0x20 bgcolor is rgb
			// 0x40 editable
			// 0x80 moveable
			sprite._colorcode = stream.readByte();

			sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			break;
		case 21:
			sprite._blendAmount = stream.readByte();
			break;
		case 22:
			sprite._thickness = stream.readByte();
			break;
		case 23:
			(void)stream.readByte(); // unused
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD5(): Miscomputed field position: %ld", stream.pos() - startPosition);
		}
	}

}

/**************************
 *
 * D6 Loading
 *
 **************************/

void Frame::readChannelD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	// 48 bytes header
	if (offset < kMainChannelSizeD6) {
		uint16 needSize = MIN(size, (uint16)(kMainChannelSizeD6 - offset));
		readMainChannelsD6(stream, offset, needSize);
		size -= needSize;
		offset += needSize;
	}

	if (offset >= kMainChannelSizeD6) {
		byte spritePosition = (offset - kMainChannelSizeD6) / kSprChannelSizeD6;
		uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD6 + kMainChannelSizeD6;

		while (size > 0) {
			uint16 needSize = MIN((uint16)(nextStart - offset), size);
			readSpriteD6(stream, offset, needSize);
			offset += needSize;
			size -= needSize;
			nextStart += kSprChannelSizeD6;
		}
	}
}

void Frame::readMainChannelsD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readMainChannelsD6(): %d byte header", size);
		stream.hexdump(size);
	}
	error("Frame::readMainChannelsD6(): Miscomputed field position: %d", offset);
}

void Frame::readSpriteD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD6) / kSprChannelSizeD6;
	uint16 spriteStart = spritePosition * kSprChannelSizeD6 + kMainChannelSizeD6;

	uint16 fieldPosition = offset - spriteStart;

	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readSpriteD6(): channel %d, 20 bytes", spritePosition);
		stream.hexdump(kSprChannelSizeD6);
	}

	debugC(3, kDebugLoading, "Frame::readSpriteD6(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);

	Sprite &sprite = *_sprites[spritePosition + 1];

	if (sprite._puppet || sprite._autoPuppet) {
		stream.skip(size);
		return;
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD6(stream, sprite, initPos - fieldPosition, finishPosition);

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD6(): Read %ld extra bytes", stream.pos() - finishPosition);
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

void readSpriteDataD6(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition) {
	while (stream.pos() < finishPosition) {
		switch (stream.pos() - startPosition) {
		case 0:
			sprite._spriteType = (SpriteType)stream.readByte();
			break;
		case 1:
			sprite._inkData = stream.readByte();

			sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
			if (sprite._inkData & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			break;
		case 2:
			sprite._foreColor = g_director->transformColor((uint8)stream.readByte());
			break;
		case 3:
			sprite._backColor = g_director->transformColor((uint8)stream.readByte());
			break;
		case 4: {
				uint16 castLib = stream.readUint16();
				uint16 memberID = stream.readUint16();
				sprite._castId = CastMemberID(memberID, castLib);
			}
			break;
		case 8: {
				uint16 scriptCastLib = stream.readUint16();
				uint16 scriptMemberID = stream.readUint16();
				sprite._scriptId = CastMemberID(scriptMemberID, scriptCastLib);
			}
			break;
		case 12:
			sprite._startPoint.y = (int16)stream.readUint16();
			break;
		case 14:
			sprite._startPoint.x = (int16)stream.readUint16();
			break;
		case 16:
			sprite._height = (int16)stream.readUint16();
			break;
		case 18:
			sprite._width = (int16)stream.readUint16();
			break;
		case 20:
			// & 0x0f scorecolor
			// 0x10 forecolor is rgb
			// 0x20 bgcolor is rgb
			// 0x40 editable
			// 0x80 moveable
			sprite._colorcode = stream.readByte();

			sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			break;
		case 21:
			sprite._blendAmount = stream.readByte();
			break;
		case 22:
			sprite._thickness = stream.readByte();
			break;
		case 23:
			(void)stream.readByte(); // unused
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD6(): Miscomputed field position: %ld", stream.pos() - startPosition);
		}
	}
}

Common::String Frame::formatChannelInfo() {
	Common::String result;
	result += Common::String::format("TMPO:   tempo: %d, skipFrameFlag: %d, blend: %d\n",
		_mainChannels.tempo, _mainChannels.skipFrameFlag, _mainChannels.blend);
	if (_mainChannels.palette.paletteId.isNull()) {
		result += Common::String::format("PAL:    paletteId: %s, firstColor: %d, lastColor: %d, flags: %d, cycleCount: %d, speed: %d, frameCount: %d, fade: %d, delay: %d, style: %d\n",
			_mainChannels.palette.paletteId.asString().c_str(), _mainChannels.palette.firstColor, _mainChannels.palette.lastColor, _mainChannels.palette.flags,
			_mainChannels.palette.cycleCount, _mainChannels.palette.speed, _mainChannels.palette.frameCount,
			_mainChannels.palette.fade, _mainChannels.palette.delay, _mainChannels.palette.style);
	} else {
		result += Common::String::format("PAL:    paletteId: 000\n");
	}
	result += Common::String::format("TRAN:   transType: %d, transDuration: %d, transChunkSize: %d\n",
		_mainChannels.transType, _mainChannels.transDuration, _mainChannels.transChunkSize);
	result += Common::String::format("SND: 1  sound1: %d, soundType1: %d\n", _mainChannels.sound1.member, _mainChannels.soundType1);
	result += Common::String::format("SND: 2  sound2: %d, soundType2: %d\n", _mainChannels.sound2.member, _mainChannels.soundType2);
	result += Common::String::format("LSCR:   actionId: %d\n", _mainChannels.actionId.member);

	for (int i = 0; i < _numChannels; i++) {
		Sprite &sprite = *_sprites[i + 1];
		if (sprite._castId.member) {
			result += Common::String::format("CH: %-3d castId: %s, [inkData: 0x%02x [ink: %d, trails: %d, line: %d], %dx%d@%d,%d type: %d (%s) fg: %d bg: %d], script: %s, colorcode: 0x%x, blendAmount: 0x%x, unk3: 0x%x\n",
				i + 1, sprite._castId.asString().c_str(), sprite._inkData,
				sprite._ink, sprite._trails, sprite._thickness, sprite._width, sprite._height,
				sprite._startPoint.x, sprite._startPoint.y,
				sprite._spriteType, spriteType2str(sprite._spriteType), sprite._foreColor,
				sprite._backColor, sprite._scriptId.asString().c_str(), sprite._colorcode,
				sprite._blendAmount, sprite._unk3);
		} else {
			result += Common::String::format("CH: %-3d castId: 000\n", i + 1);
		}
	}

	return result;
}

} // End of namespace Director
