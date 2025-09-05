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
	debugC(6, kDebugLoading, "Frame::readChannel(..., offset=%d, size=%d, version=%x (v%d))", offset, size, version, humanVersion(version));

	if (version < kFileVer400) {
		readChannelD2(stream, offset, size);
	} else if (version >= kFileVer400 && version < kFileVer500) {
		readChannelD4(stream, offset, size);
	} else if (version >= kFileVer500 && version < kFileVer600) {
		readChannelD5(stream, offset, size);
	} else if (version >= kFileVer600 && version < kFileVer700) {
		readChannelD6(stream, offset, size);
	} else if (version >= kFileVer700 && version < kFileVer1100) {
		readChannelD7(stream, offset, size);
	} else {
		error("Frame::readChannel(): Unsupported Director version: %d", version);
	}
}

void Frame::writeMainChannels(Common::SeekableWriteStream *writeStream, uint16 version) {
	debugC(6, kDebugLoading, "Frame::writeChannel: writing main channels for version v%d (%d)", humanVersion(version), version);

	if (version >= kFileVer400 && version < kFileVer500) {
		writeMainChannelsD4(writeStream);
	} else if (version >= kFileVer500 && version < kFileVer600) {
		writeMainChannelsD5(writeStream);
	} else if (version >= kFileVer600 && version < kFileVer700) {
		writeMainChannelsD6(writeStream);
	} else if (version >= kFileVer700 && version < kFileVer1100) {
		writeMainChannelsD7(writeStream);
	} else {
		warning("Frame::writeChannel(): Unsupported Director version: %d", version);
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
		// Sound/Tempo/Transition
		case 0:
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
			if (_mainChannels.tempo && _mainChannels.tempo <= 120)
				_mainChannels.scoreCachedTempo = _mainChannels.tempo;
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
			if (unk[0])
				debugC(8, kDebugLoading, "Frame::readMainChannelsD2(): STUB: unk1: 0x%02x", unk[0]);
			break;
		case 14:
			stream.read(unk, 2);
			if (unk[0] || unk[1])
				debugC(8, kDebugLoading, "Frame::readMainChannelsD2(): STUB: unk2: 0x%02x 0x%02x", unk[0], unk[1]);
			break;

		// Palette
		case 16: {
				int16 paletteId = stream.readSint16();
				if (paletteId == 0) {
					_mainChannels.palette.paletteId = CastMemberID(0, 0);
				} else if (paletteId < 0) {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, -1);
				} else {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
				}
				if (!_mainChannels.palette.paletteId.isNull())
					_mainChannels.scoreCachedPaletteId = _mainChannels.palette.paletteId;
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

			if (unk[0] || unk[1] || unk[2] || unk[3] || unk[4] || unk[5])
				debugC(8, kDebugLoading, "Frame::readMainChannelsD2(): STUB: unk1: %02x %02x %02x %02x %02x %02x", unk[0],
					unk[1], unk[2], unk[3], unk[4], unk[5]);
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("Frame::readMainChannelsD2(): Miscomputed field position: %" PRId64, stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD2(): Read %" PRId64 " extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::readSpriteD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD2) / kSprChannelSizeD2;
	uint16 spriteStart = spritePosition * kSprChannelSizeD2 + kMainChannelSizeD2;

	uint16 fieldPosition = offset - spriteStart;

	debugC(5, kDebugLoading, "Frame::readSpriteD2(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);
	if (debugChannelSet(8, kDebugLoading)) {
		stream.hexdump(size);
	}

	Sprite &sprite = *_sprites[spritePosition + 1];

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD2(stream, sprite, initPos - fieldPosition, finishPosition);

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD2(): Read %" PRId64 " extra bytes", stream.pos() - finishPosition);
	}
}

void readSpriteDataD2(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition) {
	while (stream.pos() < finishPosition) {
		switch (stream.pos() - startPosition) {
		case 0:
			sprite._scriptId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			break;
		case 1:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._spriteType = (SpriteType)stream.readByte();
				sprite._enabled = sprite._spriteType != kInactiveSprite;
			}
			break;
		case 2:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
				sprite._foreColor = g_director->transformColor(stream.readByte() ^ 0x80);
			}
			break;
		case 3:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
				sprite._backColor = g_director->transformColor(stream.readByte() ^ 0x80);
			}
			break;
		case 4:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._thickness = stream.readByte() & 0x7f;
			}
			break;
		case 5:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._inkData = stream.readByte();

				sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
				sprite._trails = sprite._inkData & 0x40 ? true : false;
				sprite._stretch = sprite._inkData & 0x80 ? true : false;
			}
			break;
		case 6:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				if (sprite.isQDShape()) {
					sprite._pattern = stream.readUint16();
					sprite._castId = CastMemberID(0, 0);
				} else {
					sprite._pattern = 0;
					sprite._castId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
				}
			}
			break;
		case 8:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._startPoint.y = (int16)stream.readUint16();
			}
			break;
		case 10:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._startPoint.x = (int16)stream.readUint16();
			}
			break;
		case 12:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._height = (int16)stream.readUint16();
			}
			break;
		case 14:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._width = (int16)stream.readUint16();
			}
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD2(): Miscomputed field position: %" PRId64, stream.pos() - startPosition);
		}
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
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
		// Sound/Tempo/Transition
		case 0:
			unk1 = stream.readByte();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk1: %d 0x%x", unk1, unk1);
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
			if (_mainChannels.tempo && _mainChannels.tempo <= 120)
				_mainChannels.scoreCachedTempo = _mainChannels.tempo;
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

		// Palette, 20 bytes
		case 20: {
				int16 paletteId = stream.readSint16();
				if (paletteId == 0) {
					_mainChannels.palette.paletteId = CastMemberID(0, 0);
				} else if (paletteId < 0) {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, -1);
				} else {
					_mainChannels.palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
				}
				if (!_mainChannels.palette.paletteId.isNull())
					_mainChannels.scoreCachedPaletteId = _mainChannels.palette.paletteId;
			}
			break;
		case 22:
			// loop points for color cycling
			_mainChannels.palette.firstColor = g_director->transformColor(stream.readByte() ^ 0x80); // 22
			_mainChannels.palette.lastColor = g_director->transformColor(stream.readByte() ^ 0x80); // 23
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
			error("Frame::readMainChannelsD4(): Miscomputed field position: %" PRId64, stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD4(): Read %" PRId64 " extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::writeMainChannelsD4(Common::SeekableWriteStream *writeStream) {
	writeStream->writeByte(0);		// Unknown: Sound/Tempo/Transition	// 0

	writeStream->writeByte(_mainChannels.soundType1);		// 1
	writeStream->writeByte((_mainChannels.transArea ? 0x80 : 0x00) | ((_mainChannels.transDuration / 250) & 0x7F));	// 2
	writeStream->writeByte(_mainChannels.transChunkSize);	// 3
	writeStream->writeByte(_mainChannels.tempo);			// 4
	writeStream->writeByte(_mainChannels.transType);		// 5

	writeStream->writeUint16BE(_mainChannels.sound1.member);		// 6, 7
	writeStream->writeUint16BE(_mainChannels.sound2.member);		// 8, 9

	writeStream->writeByte(_mainChannels.soundType2);			// 10
	writeStream->writeByte(_mainChannels.skipFrameFlag);		// 11
	writeStream->writeByte(_mainChannels.blend);				// 12
	writeStream->writeByte(_mainChannels.colorTempo);			// 13
	writeStream->writeByte(_mainChannels.colorSound1);			// 14
	writeStream->writeByte(_mainChannels.colorSound2);			// 15
	writeStream->writeUint16BE(_mainChannels.actionId.member);	// 16, 17
	writeStream->writeByte(_mainChannels.colorScript);			// 18
	writeStream->writeByte(_mainChannels.colorTrans);			// 19

	// palette
	writeStream->writeSint16BE(_mainChannels.palette.paletteId.member);		// 20, 21
	writeStream->writeByte(_mainChannels.palette.firstColor ^ 0x80);		// 22
	writeStream->writeByte(_mainChannels.palette.lastColor ^ 0x80);			// 23
	writeStream->writeByte(_mainChannels.palette.flags);					// 24
	writeStream->writeByte(_mainChannels.palette.speed);					// 25
	writeStream->writeUint16BE(_mainChannels.palette.frameCount);			// 26, 27
	writeStream->writeUint16BE(_mainChannels.palette.cycleCount);			// 28, 29
	writeStream->writeByte(_mainChannels.palette.fade);						// 30
	writeStream->writeByte(_mainChannels.palette.delay);					// 31
	writeStream->writeByte(_mainChannels.palette.style);					// 32

	writeStream->writeByte(0);			// Unknown	// 33
	writeStream->writeUint16BE(0);		// Unknown	// 34, 35
	writeStream->writeUint16BE(0);		// Unknown	// 36, 37

	writeStream->writeByte(_mainChannels.palette.colorCode);	// 38
	writeStream->writeByte(0);			// Unknown	// 39
}

void Frame::readSpriteD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD4) / kSprChannelSizeD4;
	uint16 spriteStart = spritePosition * kSprChannelSizeD4 + kMainChannelSizeD4;

	uint16 fieldPosition = offset - spriteStart;

	debugC(5, kDebugLoading, "Frame::readSpriteD4(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);
	if (debugChannelSet(8, kDebugLoading)) {
		stream.hexdump(size);
	}

	Sprite &sprite = *_sprites[spritePosition + 1];

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD4(stream, sprite, initPos - fieldPosition, finishPosition);

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD4(): Read %" PRId64 " extra bytes", stream.pos() - finishPosition);
	}
}

void readSpriteDataD4(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition) {
	debugC(8, kDebugLoading, "stream.pos(): %0x, startPosition: %d, finishPosition: %d", (int)stream.pos(), startPosition, finishPosition);
	while (stream.pos() < finishPosition) {
		switch (stream.pos() - startPosition) {
		case 0:
			sprite._scriptId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			break;
		case 1:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._spriteType = (SpriteType)stream.readByte();
				sprite._enabled = sprite._spriteType != kInactiveSprite;
			}
			break;
		case 2:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._foreColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 3:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._backColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 4:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._thickness = stream.readByte() & 0x7f;
			}
			break;
		case 5:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._inkData = stream.readByte();

				sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
				sprite._trails = sprite._inkData & 0x40 ? true : false;
				sprite._stretch = sprite._inkData & 0x80 ? true : false;
			}
			break;
		case 6:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				if (sprite.isQDShape()) {
					sprite._pattern = stream.readUint16();
				} else {
					sprite._castId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
				}
			}
			break;
		case 8:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._startPoint.y = (int16)stream.readUint16();
			}
			break;
		case 10:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._startPoint.x = (int16)stream.readUint16();
			}
			break;
		case 12:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._height = (int16)stream.readUint16();
			}
			break;
		case 14:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._width = (int16)stream.readUint16();
			}
			break;
		case 16:
			sprite._scriptId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			break;
		case 18:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				// & 0x0f scorecolor
				// 0x10 forecolor is rgb
				// 0x20 bgcolor is rgb
				// 0x40 editable
				// 0x80 moveable
				sprite._colorcode = stream.readByte();

				sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			}
			break;
		case 19:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._blendAmount = stream.readByte();
			}
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD4(): Miscomputed field position: %" PRId64, stream.pos() - startPosition);
		}
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

void writeSpriteDataD4(Common::SeekableWriteStream *writeStream, Sprite &sprite) {
	writeStream->writeByte(sprite._scriptId.member);			// 0
	writeStream->writeByte((byte) sprite._spriteType);		// 1
	writeStream->writeByte(sprite._foreColor);				// 2
	writeStream->writeByte(sprite._backColor);				// 3
	writeStream->writeByte(sprite._thickness);				// 4
	writeStream->writeByte(sprite._inkData);				// 5

	if (sprite.isQDShape()) {
		writeStream->writeUint16BE(sprite._pattern);		// 6, 7
	} else {
		writeStream->writeUint16BE(sprite._castId.member);	// 6, 7
	}

	writeStream->writeUint16BE(sprite._startPoint.y);		// 8, 9
	writeStream->writeUint16BE(sprite._startPoint.x);		// 10, 11
	writeStream->writeUint16BE(sprite._height);				// 12, 13
	writeStream->writeUint16BE(sprite._width);				// 14, 15
	writeStream->writeUint16BE(sprite._scriptId.member);	// 16, 17
	writeStream->writeByte(sprite._colorcode);				// 18
	writeStream->writeByte(sprite._blendAmount);			// 19
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
		// Sound/Tempo/Transition
		case 0:
			_mainChannels.actionId.castLib = stream.readUint16();
			break;
		case 2:
			_mainChannels.actionId.member = stream.readUint16();
			break;
		case 4:
			_mainChannels.sound1.castLib = stream.readUint16();
			break;
		case 6:
			_mainChannels.sound1.member = stream.readUint16();
			break;
		case 8:
			_mainChannels.sound2.castLib = stream.readUint16();
			break;
		case 10:
			_mainChannels.sound2.member = stream.readUint16();
			break;
		case 12:
			_mainChannels.trans.castLib = stream.readUint16();
			break;
		case 14:
			_mainChannels.trans.member = stream.readUint16();
			break;
		case 16:
			_mainChannels.colorTempo = stream.readByte();
			break;
		case 17:
			_mainChannels.colorSound1 = stream.readByte();
			break;
		case 18:
			_mainChannels.colorSound2 = stream.readByte();
			break;
		case 19:
			_mainChannels.colorScript = stream.readByte();
			break;
		case 20:
			_mainChannels.colorTrans = stream.readByte();
			break;
		case 21:
			_mainChannels.tempo = stream.readByte();
			if (_mainChannels.tempo && _mainChannels.tempo <= 120)
				_mainChannels.scoreCachedTempo = _mainChannels.tempo;
			break;
		case 22:
			stream.read(unk, 2); // alignment bytes
			if (unk[0] || unk[1])
				warning("Frame::readMainChannelsD5(): STUB: unk4: 0x%02x 0x%02x", unk[0], unk[1]);
			break;

		// Palette
		case 24:
			_mainChannels.palette.paletteId.castLib = stream.readSint16();
			break;
		case 26:
			_mainChannels.palette.paletteId.member = stream.readSint16();
			if (!_mainChannels.palette.paletteId.isNull())
				_mainChannels.scoreCachedPaletteId = _mainChannels.palette.paletteId;
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
			_mainChannels.palette.firstColor = g_director->transformColor(stream.readByte() ^ 0x80); // 30
			_mainChannels.palette.lastColor = g_director->transformColor(stream.readByte() ^ 0x80); // 31
			break;
		case 32:
			_mainChannels.palette.frameCount = stream.readUint16(); // 32
			break;
		case 34:
			_mainChannels.palette.cycleCount = stream.readUint16(); // 34
			break;
		case 36:
			_mainChannels.palette.fade = stream.readByte();
			break;
		case 37:
			_mainChannels.palette.delay = stream.readByte();
			break;
		case 38:
			_mainChannels.palette.style = stream.readByte();
			break;
		case 39:
			_mainChannels.palette.colorCode = stream.readByte();
			break;
		case 40: {
				stream.read(unk, 8); // alignment bytes

				Common::String s;
				for (int i = 0; i < 8; i++)
					s += Common::String::format("0x%02x ", unk[i]);

				warning("Frame::readMainChannelsD5(): STUB: unk7: %s", s.c_str());
			}
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("Frame::readMainChannelsD5(): Miscomputed field position: %" PRId64, stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD5(): Read %" PRId64 "extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::writeMainChannelsD5(Common::SeekableWriteStream *writeStream) {
	writeStream->writeUint16BE(_mainChannels.actionId.castLib);		// 0, 1
	writeStream->writeUint16BE(_mainChannels.actionId.member);		// 2, 3
	writeStream->writeUint16BE(_mainChannels.sound1.castLib);		// 4, 5
	writeStream->writeUint16BE(_mainChannels.sound1.member);		// 6, 7
	writeStream->writeUint16BE(_mainChannels.sound2.castLib);		// 8, 9
	writeStream->writeUint16BE(_mainChannels.sound2.member);		// 10, 11
	writeStream->writeUint16BE(_mainChannels.trans.member);			// 12, 13
	writeStream->writeUint16BE(_mainChannels.trans.member);			// 14, 15

	writeStream->writeByte(_mainChannels.colorTempo);				// 16
	writeStream->writeByte(_mainChannels.colorSound1);				// 17
	writeStream->writeByte(_mainChannels.colorSound2);				// 18
	writeStream->writeByte(_mainChannels.colorScript);				// 19
	writeStream->writeByte(_mainChannels.colorTrans);				// 20

	writeStream->writeByte(_mainChannels.tempo);					// 21
	writeStream->writeUint16BE(0);									// Unknown	// 22, 23

	writeStream->writeSint16BE(_mainChannels.palette.paletteId.castLib);	// 24, 25
	writeStream->writeSint16BE(_mainChannels.palette.paletteId.member);		// 26, 27
	writeStream->writeByte(_mainChannels.palette.speed);				// 28
	writeStream->writeByte(_mainChannels.palette.flags);				// 29
	writeStream->writeByte(_mainChannels.palette.firstColor ^ 0x80);	// 30
	writeStream->writeByte(_mainChannels.palette.lastColor ^ 0x80);		// 31

	writeStream->writeUint16BE(_mainChannels.palette.frameCount);		// 32, 33
	writeStream->writeUint16BE(_mainChannels.palette.cycleCount);		// 34, 35
	writeStream->writeByte(_mainChannels.palette.fade);					// 36
	writeStream->writeByte(_mainChannels.palette.delay);				// 37
	writeStream->writeByte(_mainChannels.palette.style);				// 38
	writeStream->writeByte(_mainChannels.palette.colorCode);			// 39

	writeStream->writeUint64BE(0);		// Unknown 	// 40, 41, 42, 43, 44, 45, 46, 47
}

void Frame::readSpriteD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD5) / kSprChannelSizeD5;
	uint16 spriteStart = spritePosition * kSprChannelSizeD5 + kMainChannelSizeD5;

	uint16 fieldPosition = offset - spriteStart;

	debugC(5, kDebugLoading, "Frame::readSpriteD5(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);
	if (debugChannelSet(8, kDebugLoading)) {
		stream.hexdump(size);
	}

	Sprite &sprite = *_sprites[spritePosition + 1];

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD5(stream, sprite, initPos - fieldPosition, finishPosition);

	if (fieldPosition > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD5(): Read %" PRId64 " extra bytes", stream.pos() - finishPosition);
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
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._spriteType = (SpriteType)stream.readByte();
			}
			break;
		case 1:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._inkData = stream.readByte();

				sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
				sprite._trails = sprite._inkData & 0x40 ? true : false;
				sprite._stretch = sprite._inkData & 0x80 ? true : false;
			}
			break;
		case 2:
			if (sprite._puppet) {
				stream.readSint16();
			} else {
				int castLib = stream.readSint16();
				sprite._castId = CastMemberID(sprite._castId.member, castLib);
			}
			break;
		case 4:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				uint16 memberID = stream.readUint16();
				sprite._castId = CastMemberID(memberID, sprite._castId.castLib);  // Inherit castLib from previous frame
			}
			break;
		case 6: {
				int scriptCastLib = stream.readSint16();
				sprite._scriptId = CastMemberID(sprite._scriptId.member, scriptCastLib);
			}
			break;
		case 8: {
				uint16 scriptMemberID = stream.readUint16();
				sprite._scriptId = CastMemberID(scriptMemberID, sprite._scriptId.castLib);  // Inherit castLib from previous frame
			}
			break;
		case 10:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._foreColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 11:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._backColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 12:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._startPoint.y = (int16)stream.readUint16();
			}
			break;
		case 14:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._startPoint.x = (int16)stream.readUint16();
			}
			break;
		case 16:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._height = (int16)stream.readUint16();
			}
			break;
		case 18:
			if (sprite._puppet) {
				stream.readUint16();
			} else {
				sprite._width = (int16)stream.readUint16();
			}
			break;
		case 20:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				// & 0x0f scorecolor
				// 0x10 forecolor is rgb
				// 0x20 bgcolor is rgb
				// 0x40 editable
				// 0x80 moveable
				sprite._colorcode = stream.readByte();

				sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			}
			break;
		case 21:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._blendAmount = stream.readByte();
			}
			break;
		case 22:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._thickness = stream.readByte() & 0x7f;
			}
			break;
		case 23:
			(void)stream.readByte(); // unused
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD5(): Miscomputed field position: %" PRId64, stream.pos() - startPosition);
		}
	}
}

void writeSpriteDataD5(Common::SeekableWriteStream *writeStream, Sprite &sprite) {
	writeStream->writeByte(sprite._spriteType);				// 0
	writeStream->writeByte(sprite._inkData);				// 1
	writeStream->writeSint16BE(sprite._castId.castLib);		// 2, 3
	writeStream->writeUint16BE(sprite._castId.member);		// 4, 5
	writeStream->writeSint16BE(sprite._scriptId.castLib);	// 6, 7
	writeStream->writeUint16BE(sprite._scriptId.member);	// 8, 9
	writeStream->writeByte(sprite._foreColor);				// 10
	writeStream->writeByte(sprite._backColor);				// 11
	writeStream->writeUint16BE(sprite._startPoint.y);		// 12, 13
	writeStream->writeUint16BE(sprite._startPoint.x);		// 14, 15
	writeStream->writeUint16BE(sprite._height);				// 16, 17
	writeStream->writeUint16BE(sprite._width);				// 18, 19
	writeStream->writeByte(sprite._colorcode);				// 20
	writeStream->writeByte(sprite._blendAmount);			// 21
	writeStream->writeByte(sprite._thickness);				// 22
	writeStream->writeByte(0);								// 23, unused
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

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;
	byte unk[16];

	while (stream.pos() < finishPosition) {
		switch (stream.pos() - initPos + offset) {
		// Script
		case 0+0:
			_mainChannels.actionId.castLib = stream.readUint16();
			break;
		case 0+2:
			_mainChannels.actionId.member = stream.readUint16();
			break;
		case 0+4:
			_mainChannels.scriptSpriteListIdx = stream.readUint32();
			break;
		case 0+6:
			_mainChannels.scriptSpriteListIdx = stream.readUint16();
			break;
		case 0+8:
			_mainChannels.colorScript = stream.readByte();
			break;
		case 0+9:
			stream.read(unk, 15); // alignment bytes
			hexdumpIfNotZero(unk, 15, "Frame::readMainChannelsD6(): script.unk: ");
			break;

		// Tempo
		case 24+0:
			_mainChannels.tempoSpriteListIdx = stream.readUint32();
			break;
		case 24+2:
			_mainChannels.tempoSpriteListIdx = stream.readUint16();
			break;
		case 24+4:
			_mainChannels.tempoD6Flags = stream.readUint16();
			break;
		case 24+6:
			_mainChannels.tempo = stream.readByte();
			if (_mainChannels.tempo && _mainChannels.tempo <= 120)
				_mainChannels.scoreCachedTempo = _mainChannels.tempo;
			break;
		case 24+7:
			_mainChannels.colorTempo = stream.readByte();
			break;
		case 24+8:
			stream.read(unk, 16); // alignment bytes
			hexdumpIfNotZero(unk, 16, "Frame::readMainChannelsD6(): tempo.unk: ");
			break;


		// Transition
		case 48+0:
			_mainChannels.trans.castLib = stream.readUint16();
			break;
		case 48+2:
			_mainChannels.trans.member = stream.readUint16();
			break;
		case 48+4:
			_mainChannels.transSpriteListIdx = stream.readUint32();
			break;
		case 48+6:
			_mainChannels.transSpriteListIdx = stream.readUint16();
			break;
		case 48+8:
			_mainChannels.colorTrans = stream.readByte();
			break;
		case 48+9:
			stream.read(unk, 15); // alignment bytes
			hexdumpIfNotZero(unk, 15, "Frame::readMainChannelsD6(): trans.unk: ");
			break;

		// Sound2
		case 72+0:
			_mainChannels.sound2.castLib = stream.readUint16();
			break;
		case 72+2:
			_mainChannels.sound2.member = stream.readUint16();
			break;
		case 72+4:
			_mainChannels.sound2SpriteListIdx = stream.readUint32();
			break;
		case 72+6:
			_mainChannels.sound2SpriteListIdx = stream.readUint16();
			break;
		case 72+8:
			_mainChannels.colorSound2 = stream.readByte();
			break;
		case 72+9:
			stream.read(unk, 15); // alignment bytes
			hexdumpIfNotZero(unk, 15, "Frame::readMainChannelsD6(): sound2.unk: ");
			break;

		// Sound1
		case 96+0:
			_mainChannels.sound1.castLib = stream.readUint16();
			break;
		case 96+2:
			_mainChannels.sound1.member = stream.readUint16();
			break;
		case 96+4:
			_mainChannels.sound1SpriteListIdx = stream.readUint32();
			break;
		case 96+6:
			_mainChannels.sound1SpriteListIdx = stream.readUint16();
			break;
		case 96+8:
			_mainChannels.colorSound1 = stream.readByte();
			break;
		case 96+9:
			stream.read(unk, 15); // alignment bytes
			hexdumpIfNotZero(unk, 15, "Frame::readMainChannelsD6(): sound1.unk: ");
			break;

		// Palette
		case 120+0:
			_mainChannels.palette.paletteId.castLib = stream.readSint16();
			break;
		case 120+2:
			_mainChannels.palette.paletteId.member = stream.readSint16();
			if (!_mainChannels.palette.paletteId.isNull())
				_mainChannels.scoreCachedPaletteId = _mainChannels.palette.paletteId;
			break;
		case 120+4:
			_mainChannels.palette.speed = stream.readByte(); // 52
			_mainChannels.palette.flags = stream.readByte(); // 53
			_mainChannels.palette.colorCycling = (_mainChannels.palette.flags & 0x80) != 0;
			_mainChannels.palette.normal = (_mainChannels.palette.flags & 0x60) == 0x00;
			_mainChannels.palette.fadeToBlack = (_mainChannels.palette.flags & 0x60) == 0x60;
			_mainChannels.palette.fadeToWhite = (_mainChannels.palette.flags & 0x60) == 0x40;
			_mainChannels.palette.autoReverse = (_mainChannels.palette.flags & 0x10) != 0;
			_mainChannels.palette.overTime = (_mainChannels.palette.flags & 0x04) != 0;
			break;
		case 120+6:
			_mainChannels.palette.firstColor = g_director->transformColor(stream.readByte() ^ 0x80); // 51
			_mainChannels.palette.lastColor = g_director->transformColor(stream.readByte() ^ 0x80); // 52
			break;
		case 120+8:
			_mainChannels.palette.frameCount = stream.readUint16(); // 53
			break;
		case 120+10:
			_mainChannels.palette.cycleCount = stream.readUint16(); // 55
			break;
		case 120+12:
			_mainChannels.palette.fade = stream.readByte();
			break;
		case 120+13:
			_mainChannels.palette.delay = stream.readByte();
			break;
		case 120+14:
			_mainChannels.palette.style = stream.readByte();
			break;
		case 120+15:
			_mainChannels.palette.colorCode = stream.readByte();
			break;
		case 120+16:
			_mainChannels.palette.spriteListIdx = stream.readUint32();
			break;
		case 120+18:
			_mainChannels.palette.spriteListIdx = stream.readUint16();
			break;
		case 120+20:
			stream.read(unk, 4); // alignment bytes
			hexdumpIfNotZero(unk, 4, "Frame::readMainChannelsD6(): palette.unk: ");
			break;

		// 144 bytes (24 * 6)

		default:
			// This means that a `case` label has to be split at this position
			error("Frame::readMainChannelsD6(): Miscomputed field position: %" PRId64, stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD6(): Read %" PRId64 "extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

static void writePadding(Common::SeekableWriteStream *writeStream, int size) {
	for (int i = 0; i < size; i++) {
		writeStream->writeByte(0);
	}
}

void Frame::writeMainChannelsD6(Common::SeekableWriteStream *writeStream) {
	// Script
	writeStream->writeUint32BE(_mainChannels.actionId.castLib);				// 0+0
	writeStream->writeUint16BE(_mainChannels.actionId.member);				// 0+2
	writeStream->writeUint32BE(_mainChannels.scriptSpriteListIdx);			// 0+4
	writeStream->writeByte(_mainChannels.colorScript);						// 0+8
	writePadding(writeStream, 15);											// 0+9

	// Tempo
	writeStream->writeUint32BE(_mainChannels.tempoSpriteListIdx);			// 24+0
	writeStream->writeUint16BE(_mainChannels.tempoD6Flags);					// 24+4
	writeStream->writeByte(_mainChannels.tempo);							// 24+6
	writeStream->writeByte(_mainChannels.colorTempo);						// 24+7
	writePadding(writeStream, 16);											// 24+8

	// Transition
	writeStream->writeUint16BE(_mainChannels.trans.castLib);				// 48+0
	writeStream->writeUint16BE(_mainChannels.trans.member);					// 48+2
	writeStream->writeUint32BE(_mainChannels.transSpriteListIdx);			// 48+4
	writeStream->writeByte(_mainChannels.colorTrans);						// 48+8
	writePadding(writeStream, 15);											// 48+9

	// Sound2
	writeStream->writeUint16BE(_mainChannels.sound2.castLib);				// 72+0
	writeStream->writeUint16BE(_mainChannels.sound2.member);				// 72+2
	writeStream->writeUint32BE(_mainChannels.sound2SpriteListIdx);			// 72+4
	writeStream->writeByte(_mainChannels.colorSound2);						// 72+8
	writePadding(writeStream, 15);											// 72+9

	// Sound1
	writeStream->writeUint16BE(_mainChannels.sound1.castLib);				// 96+0
	writeStream->writeUint16BE(_mainChannels.sound1.member);				// 96+2
	writeStream->writeUint32BE(_mainChannels.sound1SpriteListIdx);			// 96+4
	writeStream->writeByte(_mainChannels.colorSound1);						// 96+8
	writePadding(writeStream, 15);											// 96+9-23

	// Palette
	writeStream->writeUint16BE(_mainChannels.palette.paletteId.castLib);	// 120+0
	writeStream->writeUint16BE(_mainChannels.palette.paletteId.member);		// 120+2
	writeStream->writeByte(_mainChannels.palette.speed);					// 120+4
	writeStream->writeByte(_mainChannels.palette.flags);					// 120+5
	writeStream->writeByte(_mainChannels.palette.firstColor);				// 120+6
	writeStream->writeByte(_mainChannels.palette.lastColor);				// 120+7
	writeStream->writeUint16BE(_mainChannels.palette.frameCount);			// 120+8
	writeStream->writeUint16BE(_mainChannels.palette.cycleCount);			// 120+10
	writeStream->writeByte(_mainChannels.palette.fade);						// 120+12
	writeStream->writeByte(_mainChannels.palette.delay);					// 120+13
	writeStream->writeByte(_mainChannels.palette.style);					// 120+14
	writeStream->writeByte(_mainChannels.palette.colorCode);				// 120+15
	writeStream->writeUint32BE(_mainChannels.palette.spriteListIdx);		// 120+16
	writePadding(writeStream, 4);											// 120+20
}

void Frame::readSpriteD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD6) / kSprChannelSizeD6;
	uint16 spriteStart = spritePosition * kSprChannelSizeD6 + kMainChannelSizeD6;

	uint16 fieldPosition = offset - spriteStart;

	debugC(5, kDebugLoading, "Frame::readSpriteD6(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);
	if (debugChannelSet(8, kDebugLoading)) {
		stream.hexdump(size);
	}

	Sprite &sprite = *_sprites[spritePosition + 1];

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD6(stream, sprite, initPos - fieldPosition, finishPosition);

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD6(): Read %" PRId64 " extra bytes", stream.pos() - finishPosition);
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
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._spriteType = (SpriteType)stream.readByte();
			}
			break;
		case 1:
			if (sprite._puppet || sprite.getAutoPuppet(kAPInk)) {
				stream.readByte();
			} else {
				sprite._inkData = stream.readByte();

				sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
				sprite._trails = sprite._inkData & 0x40 ? true : false;
				sprite._stretch = sprite._inkData & 0x80 ? true : false;
			}
			break;
		case 2:
			if (sprite._puppet || sprite.getAutoPuppet(kAPForeColor)) {
				stream.readByte();
			} else {
				sprite._foreColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 3:
			if (sprite._puppet || sprite.getAutoPuppet(kAPBackColor)) {
				stream.readByte();
			} else {
				sprite._backColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 4:
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readSint16();
			} else {
				int castLib = stream.readSint16();
				sprite._castId = CastMemberID(sprite._castId.member, castLib);
			}
			break;
		case 6:
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readUint16();
			} else {
				uint16 memberID = stream.readUint16();
				sprite._castId = CastMemberID(memberID, sprite._castId.castLib);  // Inherit castLib from previous frame
			}
			break;
		case 8:
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readUint32();
			} else {
				sprite._spriteListIdx = stream.readUint32();
			}
			break;
		case 10: // This field could be optimized
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readUint16();
			} else {
				sprite._spriteListIdx = stream.readUint16();
			}
			break;
		case 12:
			if (sprite._puppet || sprite.getAutoPuppet(kAPLoc)) {
				stream.readUint16();
			} else {
				sprite._startPoint.y = (int16)stream.readUint16();
			}
			break;
		case 14:
			if (sprite._puppet || sprite.getAutoPuppet(kAPLoc)) {
				stream.readUint16();
			} else {
				sprite._startPoint.x = (int16)stream.readUint16();
			}
			break;
		case 16:
			if (sprite._puppet || sprite.getAutoPuppet(kAPHeight)) {
				stream.readUint16();
			} else {
				sprite._height = (int16)stream.readUint16();
			}
			break;
		case 18:
			if (sprite._puppet || sprite.getAutoPuppet(kAPWidth)) {
				stream.readUint16();
			} else {
				sprite._width = (int16)stream.readUint16();
			}
			break;
		case 20:
			if (sprite._puppet || sprite.getAutoPuppet(kAPMoveable)) {
				stream.readByte();
			} else {
				// & 0x0f scorecolor
				// 0x10 forecolor is rgb
				// 0x20 bgcolor is rgb
				// 0x40 editable
				// 0x80 moveable
				sprite._colorcode = stream.readByte();

				sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			}
			break;
		case 21:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._blendAmount = stream.readByte();
			}
			break;
		case 22:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._thickness = stream.readByte() & 0x7f;
			}
			break;
		case 23:
			(void)stream.readByte(); // unused
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD6(): Miscomputed field position: %" PRId64, stream.pos() - startPosition);
		}
	}
}

void writeSpriteDataD6(Common::SeekableWriteStream *writeStream, Sprite &sprite) {
	writeStream->writeByte(sprite._spriteType);				// 0
	writeStream->writeByte(sprite._inkData);				// 1
	writeStream->writeByte(sprite._foreColor);				// 2
	writeStream->writeByte(sprite._backColor);				// 3
	writeStream->writeSint16BE(sprite._castId.castLib);		// 4, 5
	writeStream->writeUint16BE(sprite._castId.member);		// 6, 7
	writeStream->writeUint32BE(sprite._spriteListIdx);		// 8, 9, 10, 11
	writeStream->writeUint16BE(sprite._startPoint.y);		// 12, 13
	writeStream->writeUint16BE(sprite._startPoint.x);		// 14, 15
	writeStream->writeUint16BE(sprite._height);				// 16, 17
	writeStream->writeUint16BE(sprite._width);				// 18, 19
	writeStream->writeByte(sprite._colorcode);				// 20
	writeStream->writeByte(sprite._blendAmount);			// 21
	writeStream->writeByte(sprite._thickness);				// 22
	writeStream->writeByte(0);								// 23, unused
}

/**************************
 *
 * D7 Loading
 *
 **************************/

void Frame::readChannelD7(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	// 48 bytes header
	if (offset < kMainChannelSizeD7) {
		uint16 needSize = MIN(size, (uint16)(kMainChannelSizeD7 - offset));
		readMainChannelsD7(stream, offset, needSize);
		size -= needSize;
		offset += needSize;
	}

	if (offset >= kMainChannelSizeD7) {
		byte spritePosition = (offset - kMainChannelSizeD7) / kSprChannelSizeD7;
		uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD7 + kMainChannelSizeD7;

		while (size > 0) {
			uint16 needSize = MIN((uint16)(nextStart - offset), size);
			readSpriteD7(stream, offset, needSize);
			offset += needSize;
			size -= needSize;
			nextStart += kSprChannelSizeD7;
		}
	}
}

void Frame::readMainChannelsD7(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readMainChannelsD7(): %d byte header", size);
		stream.hexdump(size);
	}

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;
	byte unk[40];

	while (stream.pos() < finishPosition) {
		switch (stream.pos() - initPos + offset) {
		// Script
		case 0+0:
			_mainChannels.actionId.castLib = stream.readUint16();
			break;
		case 0+2:
			_mainChannels.actionId.member = stream.readUint16();
			break;
		case 0+4:
			_mainChannels.scriptSpriteListIdx = stream.readUint32();
			break;
		case 0+6:
			_mainChannels.scriptSpriteListIdx = stream.readUint16();
			break;
		case 0+8:
			_mainChannels.colorScript = stream.readByte();
			break;
		case 0+9:
			stream.read(unk, 39); // alignment bytes
			hexdumpIfNotZero(unk, 39, "Frame::readMainChannelsD7(): script.unk: ");
			break;

		// Tempo
		case 48+0:
			_mainChannels.tempoSpriteListIdx = stream.readUint32();
			break;
		case 48+2:
			_mainChannels.tempoSpriteListIdx = stream.readUint16();
			break;
		case 48+4:
			_mainChannels.tempoD6Flags = stream.readUint16();
			break;
		case 48+6:
			_mainChannels.tempo = stream.readByte();
			if (_mainChannels.tempo && _mainChannels.tempo <= 120)
				_mainChannels.scoreCachedTempo = _mainChannels.tempo;
			break;
		case 48+7:
			_mainChannels.colorTempo = stream.readByte();
			break;
		case 48+8:
			stream.read(unk, 40); // alignment bytes
			hexdumpIfNotZero(unk, 40, "Frame::readMainChannelsD7(): tempo.unk: ");
			break;

		// Transition
		case 96+0:
			_mainChannels.trans.castLib = stream.readUint16();
			break;
		case 96+2:
			_mainChannels.trans.member = stream.readUint16();
			break;
		case 96+4:
			_mainChannels.transSpriteListIdx = stream.readUint32();
			break;
		case 96+6:
			_mainChannels.transSpriteListIdx = stream.readUint16();
			break;
		case 96+8:
			_mainChannels.colorTrans = stream.readByte();
			break;
		case 96+9:
			stream.read(unk, 39); // alignment bytes
			hexdumpIfNotZero(unk, 39, "Frame::readMainChannelsD7(): trans.unk: ");
			break;

		// Sound2
		case 144+0:
			_mainChannels.sound2.castLib = stream.readUint16();
			break;
		case 144+2:
			_mainChannels.sound2.member = stream.readUint16();
			break;
		case 144+4:
			_mainChannels.sound2SpriteListIdx = stream.readUint32();
			break;
		case 144+6:
			_mainChannels.sound2SpriteListIdx = stream.readUint16();
			break;
		case 144+8:
			_mainChannels.colorSound2 = stream.readByte();
			break;
		case 144+9:
			stream.read(unk, 39); // alignment bytes
			hexdumpIfNotZero(unk, 39, "Frame::readMainChannelsD7(): sound2.unk: ");
			break;

		// Sound1
		case 192+0:
			_mainChannels.sound1.castLib = stream.readUint16();
			break;
		case 192+2:
			_mainChannels.sound1.member = stream.readUint16();
			break;
		case 192+4:
			_mainChannels.sound1SpriteListIdx = stream.readUint32();
			break;
		case 192+6:
			_mainChannels.sound1SpriteListIdx = stream.readUint16();
			break;
		case 192+8:
			_mainChannels.colorSound1 = stream.readByte();
			break;
		case 192+9:
			stream.read(unk, 39); // alignment bytes
			hexdumpIfNotZero(unk, 39, "Frame::readMainChannelsD7(): sound1.unk: ");
			break;

		// Palette
		case 240+0:
			_mainChannels.palette.paletteId.castLib = stream.readSint16();
			break;
		case 240+2:
			_mainChannels.palette.paletteId.member = stream.readSint16();
			if (!_mainChannels.palette.paletteId.isNull())
				_mainChannels.scoreCachedPaletteId = _mainChannels.palette.paletteId;
			break;
		case 240+4:
			_mainChannels.palette.speed = stream.readByte(); // 52
			_mainChannels.palette.flags = stream.readByte(); // 53
			_mainChannels.palette.colorCycling = (_mainChannels.palette.flags & 0x80) != 0;
			_mainChannels.palette.normal = (_mainChannels.palette.flags & 0x60) == 0x00;
			_mainChannels.palette.fadeToBlack = (_mainChannels.palette.flags & 0x60) == 0x60;
			_mainChannels.palette.fadeToWhite = (_mainChannels.palette.flags & 0x60) == 0x40;
			_mainChannels.palette.autoReverse = (_mainChannels.palette.flags & 0x10) != 0;
			_mainChannels.palette.overTime = (_mainChannels.palette.flags & 0x04) != 0;
			break;
		case 240+6:
			_mainChannels.palette.firstColor = g_director->transformColor(stream.readByte() ^ 0x80); // 51
			_mainChannels.palette.lastColor = g_director->transformColor(stream.readByte() ^ 0x80); // 52
			break;
		case 240+8:
			_mainChannels.palette.frameCount = stream.readUint16(); // 53
			break;
		case 240+10:
			_mainChannels.palette.cycleCount = stream.readUint16(); // 55
			break;
		case 240+12:
			_mainChannels.palette.fade = stream.readByte();
			break;
		case 240+13:
			_mainChannels.palette.delay = stream.readByte();
			break;
		case 240+14:
			_mainChannels.palette.style = stream.readByte();
			break;
		case 240+15:
			_mainChannels.palette.colorCode = stream.readByte();
			break;
		case 240+16:
			_mainChannels.palette.spriteListIdx = stream.readUint32();
			break;
		case 240+18:
			_mainChannels.palette.spriteListIdx = stream.readUint16();
			break;
		case 240+20:
			stream.read(unk, 28); // alignment bytes
			hexdumpIfNotZero(unk, 28, "Frame::readMainChannelsD7(): palette.unk: ");
			break;

		// 288 bytes (48 * 6)

		default:
			// This means that a `case` label has to be split at this position
			error("Frame::readMainChannelsD7(): Miscomputed field position: %" PRId64, stream.pos() - initPos + offset);
			break;
		}
	}

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readMainChannelsD7(): Read %" PRId64 "extra bytes", stream.pos() - finishPosition);
	}

	_mainChannels.transChunkSize = CLIP<byte>(_mainChannels.transChunkSize, 0, 128);
	_mainChannels.transDuration = CLIP<uint16>(_mainChannels.transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::writeMainChannelsD7(Common::SeekableWriteStream *writeStream) {
	// Script
	writeStream->writeUint32BE(_mainChannels.actionId.castLib);				// 0+0
	writeStream->writeUint16BE(_mainChannels.actionId.member);				// 0+2
	writeStream->writeUint32BE(_mainChannels.scriptSpriteListIdx);			// 0+4
	writeStream->writeByte(_mainChannels.colorScript);						// 0+8
	writePadding(writeStream, 39);											// 0+9

	// Tempo
	writeStream->writeUint32BE(_mainChannels.tempoSpriteListIdx);			// 48+0
	writeStream->writeUint16BE(_mainChannels.tempoD6Flags);					// 48+4
	writeStream->writeByte(_mainChannels.tempo);							// 48+6
	writeStream->writeByte(_mainChannels.colorTempo);						// 48+7
	writePadding(writeStream, 40);											// 48+8

	// Transition
	writeStream->writeUint16BE(_mainChannels.trans.castLib);				// 96+0
	writeStream->writeUint16BE(_mainChannels.trans.member);					// 96+2
	writeStream->writeUint32BE(_mainChannels.transSpriteListIdx);			// 96+4
	writeStream->writeByte(_mainChannels.colorTrans);						// 96+8
	writePadding(writeStream, 39);											// 96+9

	// Sound2
	writeStream->writeUint16BE(_mainChannels.sound2.castLib);				// 144+0
	writeStream->writeUint16BE(_mainChannels.sound2.member);				// 144+2
	writeStream->writeUint32BE(_mainChannels.sound2SpriteListIdx);			// 144+4
	writeStream->writeByte(_mainChannels.colorSound2);						// 144+8
	writePadding(writeStream, 39);											// 144+9

	// Sound1
	writeStream->writeUint16BE(_mainChannels.sound1.castLib);				// 192+0
	writeStream->writeUint16BE(_mainChannels.sound1.member);				// 192+2
	writeStream->writeUint32BE(_mainChannels.sound1SpriteListIdx);			// 192+4
	writeStream->writeByte(_mainChannels.colorSound1);						// 192+8
	writePadding(writeStream, 39);											// 192+9-23

	// Palette
	writeStream->writeUint16BE(_mainChannels.palette.paletteId.castLib);	// 240+0
	writeStream->writeUint16BE(_mainChannels.palette.paletteId.member);		// 240+2
	writeStream->writeByte(_mainChannels.palette.speed);					// 240+4
	writeStream->writeByte(_mainChannels.palette.flags);					// 240+5
	writeStream->writeByte(_mainChannels.palette.firstColor);				// 240+6
	writeStream->writeByte(_mainChannels.palette.lastColor);				// 240+7
	writeStream->writeUint16BE(_mainChannels.palette.frameCount);			// 240+8
	writeStream->writeUint16BE(_mainChannels.palette.cycleCount);			// 240+10
	writeStream->writeByte(_mainChannels.palette.fade);						// 240+12
	writeStream->writeByte(_mainChannels.palette.delay);					// 240+13
	writeStream->writeByte(_mainChannels.palette.style);					// 240+14
	writeStream->writeByte(_mainChannels.palette.colorCode);				// 240+15
	writeStream->writeUint32BE(_mainChannels.palette.spriteListIdx);		// 240+16
	writePadding(writeStream, 28);											// 240+20
}

void Frame::readSpriteD7(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD7) / kSprChannelSizeD7;
	uint16 spriteStart = spritePosition * kSprChannelSizeD7 + kMainChannelSizeD7;

	uint16 fieldPosition = offset - spriteStart;

	debugC(5, kDebugLoading, "Frame::readSpriteD7(): sprite: %d offset: %d size: %d, field: %d", spritePosition, offset, size, fieldPosition);
	if (debugChannelSet(8, kDebugLoading)) {
		stream.hexdump(size);
	}

	Sprite &sprite = *_sprites[spritePosition + 1];

	uint32 initPos = stream.pos();
	uint32 finishPosition = initPos + size;

	readSpriteDataD7(stream, sprite, initPos - fieldPosition, finishPosition);

	if (stream.pos() > finishPosition) {
		// This means that the relevant `case` label reads too many bytes and must be split
		error("Frame::readSpriteD7(): Read %" PRId64 " extra bytes", stream.pos() - finishPosition);
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

void readSpriteDataD7(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition) {
	byte unk[12];

	while (stream.pos() < finishPosition) {
		switch (stream.pos() - startPosition) {
		case 0:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._spriteType = (SpriteType)stream.readByte();
			}
			break;
		case 1:
			if (sprite._puppet || sprite.getAutoPuppet(kAPInk)) {
				stream.readByte();
			} else {
				sprite._inkData = stream.readByte();

				sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
				sprite._trails = sprite._inkData & 0x40 ? true : false;
				sprite._stretch = sprite._inkData & 0x80 ? true : false;
			}
			break;
		case 2:
			if (sprite._puppet || sprite.getAutoPuppet(kAPForeColor)) {
				stream.readByte();
			} else {
				sprite._foreColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 3:
			if (sprite._puppet || sprite.getAutoPuppet(kAPBackColor)) {
				stream.readByte();
			} else {
				sprite._backColor = g_director->transformColor((uint8)stream.readByte());
			}
			break;
		case 4:
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readSint16();
			} else {
				int castLib = stream.readSint16();
				sprite._castId = CastMemberID(sprite._castId.member, castLib);
			}
			break;
		case 6:
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readUint16();
			} else {
				uint16 memberID = stream.readUint16();
				sprite._castId = CastMemberID(memberID, sprite._castId.castLib);  // Inherit castLib from previous frame
			}
			break;
		case 8:
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readUint32();
			} else {
				sprite._spriteListIdx = stream.readUint32();
			}
			break;
		case 10: // This field could be optimized
			if (sprite._puppet || sprite.getAutoPuppet(kAPCast)) {
				stream.readUint16();
			} else {
				sprite._spriteListIdx = stream.readUint16();
			}
			break;
		case 12:
			if (sprite._puppet || sprite.getAutoPuppet(kAPLoc)) {
				stream.readUint16();
			} else {
				sprite._startPoint.y = (int16)stream.readUint16();
			}
			break;
		case 14:
			if (sprite._puppet || sprite.getAutoPuppet(kAPLoc)) {
				stream.readUint16();
			} else {
				sprite._startPoint.x = (int16)stream.readUint16();
			}
			break;
		case 16:
			if (sprite._puppet || sprite.getAutoPuppet(kAPHeight)) {
				stream.readUint16();
			} else {
				sprite._height = (int16)stream.readUint16();
			}
			break;
		case 18:
			if (sprite._puppet || sprite.getAutoPuppet(kAPWidth)) {
				stream.readUint16();
			} else {
				sprite._width = (int16)stream.readUint16();
			}
			break;
		case 20:
			if (sprite._puppet || sprite.getAutoPuppet(kAPMoveable)) {
				stream.readByte();
			} else {
				// & 0x0f scorecolor
				// 0x10 forecolor is rgb
				// 0x20 bgcolor is rgb
				// 0x40 editable
				// 0x80 moveable
				sprite._colorcode = stream.readByte();

				sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
				sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			}
			break;
		case 21:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._blendAmount = stream.readByte();
			}
			break;
		case 22:
			if (sprite._puppet) {
				stream.readByte();
			} else {
				sprite._thickness = stream.readByte() & 0x7f;
			}
			break;
		case 23:
			sprite._flags = stream.readByte();
			break;
		case 24:
			if (sprite._puppet || sprite.getAutoPuppet(kAPForeColor)) {
				stream.readByte();
			} else {
				sprite._fgColorG = (uint8)stream.readByte();
			}
			break;
		case 25:
			if (sprite._puppet || sprite.getAutoPuppet(kAPBackColor)) {
				stream.readByte();
			} else {
				sprite._bgColorG = (uint8)stream.readByte();
			}
			break;
		case 26:
			if (sprite._puppet || sprite.getAutoPuppet(kAPForeColor)) {
				stream.readByte();
			} else {
				sprite._fgColorB = (uint8)stream.readByte();
			}
			break;
		case 27:
			if (sprite._puppet || sprite.getAutoPuppet(kAPBackColor)) {
				stream.readByte();
			} else {
				sprite._bgColorB = (uint8)stream.readByte();
			}
			break;
		case 28:
			sprite._angleRot = stream.readUint32();
			break;
		case 30:	// half of the field
			sprite._angleRot = stream.readUint16();
			break;
		case 32:
			sprite._angleSkew = stream.readUint32();
			break;
		case 36:
			stream.read(unk, 12); // alignment bytes
			hexdumpIfNotZero(unk, 12, "Frame::readSpriteDataD7(): sprite.unk: ");
			break;
		default:
			// This means that a `case` label has to be split at this position
			error("readSpriteDataD7(): Miscomputed field position: %" PRId64, stream.pos() - startPosition);
		}
	}
}

void writeSpriteDataD7(Common::SeekableWriteStream *writeStream, Sprite &sprite) {
	writeStream->writeByte(sprite._spriteType);				// 0
	writeStream->writeByte(sprite._inkData);				// 1
	writeStream->writeByte(sprite._foreColor);				// 2
	writeStream->writeByte(sprite._backColor);				// 3
	writeStream->writeSint16BE(sprite._castId.castLib);		// 4, 5
	writeStream->writeUint16BE(sprite._castId.member);		// 6, 7
	writeStream->writeUint32BE(sprite._spriteListIdx);		// 8, 9, 10, 11
	writeStream->writeUint16BE(sprite._startPoint.y);		// 12, 13
	writeStream->writeUint16BE(sprite._startPoint.x);		// 14, 15
	writeStream->writeUint16BE(sprite._height);				// 16, 17
	writeStream->writeUint16BE(sprite._width);				// 18, 19
	writeStream->writeByte(sprite._colorcode);				// 20
	writeStream->writeByte(sprite._blendAmount);			// 21
	writeStream->writeByte(sprite._thickness);				// 22
	writeStream->writeByte(sprite._flags);					// 23
	writeStream->writeByte(sprite._fgColorG);				// 24
	writeStream->writeByte(sprite._bgColorG);				// 25
	writeStream->writeByte(sprite._fgColorB);				// 26
	writeStream->writeByte(sprite._bgColorB);				// 27
	writeStream->writeUint32BE(sprite._angleRot);			// 28
	writeStream->writeUint32BE(sprite._angleSkew);			// 32
	writePadding(writeStream, 12);							// 36-47
}

/**************************
 *
 * Utility Functions
 *
 **************************/

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
	result += Common::String::format("LSCR:   actionId: %s\n", _mainChannels.actionId.asString().c_str());

	for (int i = 0; i < _numChannels; i++) {
		Sprite &sprite = *_sprites[i + 1];
		if (sprite._castId.member) {
			result += Common::String::format("CH: %-3d castId: %s, [inkData: 0x%02x [ink: %d, trails: %d, stretch: %d, line: %d], %dx%d@%d,%d type: %d (%s) fg: %d bg: %d], script: %s, colorcode: 0x%x, blendAmount: 0x%x, blend: 0x%x, unk3: 0x%x\n",
				i + 1, sprite._castId.asString().c_str(), sprite._inkData,
				sprite._ink, sprite._trails, sprite._stretch, sprite._thickness, sprite._width, sprite._height,
				sprite._startPoint.x, sprite._startPoint.y,
				sprite._spriteType, spriteType2str(sprite._spriteType), sprite._foreColor,
				sprite._backColor, sprite._scriptId.asString().c_str(), sprite._colorcode,
				sprite._blendAmount, sprite._blend, sprite._unk3);
		} else {
			result += Common::String::format("CH: %-3d castId: 000\n", i + 1);
		}
	}

	return result;
}

} // End of namespace Director
