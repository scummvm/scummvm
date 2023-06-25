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
	_transDuration = 0;
	_transType = kTransNone;
	_transArea = 0;
	_transChunkSize = 0;
	_tempo = 0;

	_scoreCachedTempo = 0;
	_scoreCachedPaletteId = CastMemberID(0, 0);

	_sound1 = CastMemberID(0, 0);
	_sound2 = CastMemberID(0, 0);
	_soundType1 = 0;
	_soundType2 = 0;

	_actionId = CastMemberID(0, 0);
	_skipFrameFlag = 0;
	_blend = 0;

	_colorTempo = 0;
	_colorSound1 = 0;
	_colorSound2 = 0;
	_colorScript = 0;
	_colorTrans = 0;

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
	_actionId = frame._actionId;
	_transArea = frame._transArea;
	_transDuration = frame._transDuration;
	_transType = frame._transType;
	_transChunkSize = frame._transChunkSize;
	_tempo = frame._tempo;
	_scoreCachedTempo = frame._scoreCachedTempo;
	_sound1 = frame._sound1;
	_sound2 = frame._sound2;
	_soundType1 = frame._soundType1;
	_soundType2 = frame._soundType2;
	_skipFrameFlag = frame._skipFrameFlag;
	_blend = frame._blend;

	_colorTempo = frame._colorTempo;
	_colorSound1 = frame._colorSound1;
	_colorSound2 = frame._colorSound2;
	_colorScript = frame._colorScript;
	_colorTrans = frame._colorTrans;

	_palette = frame._palette;

	_score = frame._score;

	debugC(1, kDebugLoading, "Frame. action: %s transType: %d transDuration: %d", _actionId.asString().c_str(), _transType, _transDuration);

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

enum {
	kMainChannelSizeD2 = 32,
	kSprChannelSizeD2 = 16
};

void Frame::readChannelD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (offset >= kMainChannelSizeD2) {
		if (size <= kSprChannelSizeD2)
			readSpriteD2(stream, offset, size);
		else {
			// read > 1 sprites channel
			while (size > kSprChannelSizeD2) {
				byte spritePosition = (offset - kMainChannelSizeD2) / kSprChannelSizeD2;
				uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD2 + kMainChannelSizeD2;
				uint16 needSize = nextStart - offset;
				readSpriteD2(stream, offset, needSize);
				offset += needSize;
				size -= needSize;
			}
			readSpriteD2(stream, offset, size);
		}
	} else {
		readMainChannelsD2(stream, offset, size);
	}
}

void Frame::readMainChannelsD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 finishPosition = offset + size;
	byte unk[6];

	while (offset < finishPosition) {
		switch (offset) {
		case 0: // Sound/Tempo/Transition
			_actionId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			offset++;
			break;
		case 1:
			// type: 0x17 for sounds (sound is cast id), 0x16 for MIDI (sound is cmd id)
			_soundType1 = stream.readByte();
			offset++;
			break;
		case 2: {
				// 0x80 is whole stage (vs changed area), rest is duration in 1/4ths of a second
				uint8 transFlags = stream.readByte();
				if (transFlags & 0x80)
					_transArea = 1;
				else
					_transArea = 0;
				_transDuration = (transFlags & 0x7f) * 250; // Duration is in 1/4 secs
				offset++;
			}
			break;
		case 3:
			_transChunkSize = stream.readByte();
			offset++;
			break;
		case 4:
			_tempo = stream.readByte();
			offset++;
			break;
		case 5:
			_transType = static_cast<TransitionType>(stream.readByte());
			offset++;
			break;
		case 6:
			_sound1 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			offset += 2;
			break;
		case 8:
			_sound2 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			offset += 2;
			break;
		case 10:
			_soundType2 = stream.readByte();
			offset++;
			break;
		case 11:
			_skipFrameFlag = stream.readByte();
			offset++;
			break;
		case 12:
			_blend = stream.readByte();
			offset++;
			break;
		case 13:
			if (_vm->getPlatform() == Common::kPlatformWindows) {
				_sound2 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
				_soundType2 = stream.readByte();
			} else {
				stream.read(unk, 3);
				debugC(8, kDebugLoading, "Frame::readChannels(): STUB: unk1: %02x %02x %02x", unk[0], unk[1], unk[2]);
			}
			offset += 3;
			break;
		case 16: {
				// palette
				int16 paletteId = stream.readSint16();
				if (paletteId == 0) {
					_palette.paletteId = CastMemberID(0, 0);
				} else if (paletteId < 0) {
					_palette.paletteId = CastMemberID(paletteId, -1);
				} else {
					_palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
				}

				// loop points for color cycling
				_palette.firstColor = g_director->transformColor(stream.readByte() ^ 0x80);
				_palette.lastColor = g_director->transformColor(stream.readByte() ^ 0x80);
				_palette.flags = stream.readByte();
				_palette.colorCycling = (_palette.flags & 0x80) != 0;
				_palette.normal = (_palette.flags & 0x60) == 0x00;
				_palette.fadeToBlack = (_palette.flags & 0x60) == 0x60;
				_palette.fadeToWhite = (_palette.flags & 0x60) == 0x40;
				_palette.autoReverse = (_palette.flags & 0x10) != 0;
				_palette.overTime = (_palette.flags & 0x04) != 0;
				_palette.speed = stream.readByte();
				_palette.frameCount = stream.readUint16();
				_palette.cycleCount = stream.readUint16();

				stream.read(unk, 6);

				debugC(8, kDebugLoading, "Frame::readChannels(): STUB: unk1: %02x %02x %02x %02x %02x %02x", unk[0],
					unk[1], unk[2], unk[3], unk[4], unk[5]);
			}
			offset += 16;
			break;
		case 32:
			break;
		default:
			error("Frame::readMainChannelsD2(): Miscomputed field position: %d", offset);
			break;
		}
	}

	_transChunkSize = CLIP<byte>(_transChunkSize, 0, 128);
	_transDuration = CLIP<uint16>(_transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::readSpriteD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD2) / kSprChannelSizeD2;
	uint16 spriteStart = spritePosition * kSprChannelSizeD2 + kMainChannelSizeD2;

	uint16 fieldPosition = offset - spriteStart;
	uint16 finishPosition = fieldPosition + size;

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

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case 0:
			sprite._scriptId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			fieldPosition++;
			break;
		case 1:
			sprite._spriteType = (SpriteType)stream.readByte();
			fieldPosition++;

			sprite._enabled = sprite._spriteType != kInactiveSprite;
			break;
		case 2:
			// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
			sprite._foreColor = _vm->transformColor((128 + stream.readByte()) & 0xff);
			fieldPosition++;
			break;
		case 3:
			// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
			sprite._backColor = _vm->transformColor((128 + stream.readByte()) & 0xff);
			fieldPosition++;
			break;
		case 4:
			sprite._thickness = stream.readByte();
			fieldPosition++;
			break;
		case 5:
			sprite._inkData = stream.readByte();
			fieldPosition++;

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
			fieldPosition += 2;
			break;
		case 8:
			sprite._startPoint.y = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 10:
			sprite._startPoint.x = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 12:
			sprite._height = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 14:
			sprite._width = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 16:
			// end of channel, go to next sprite channel
			readSpriteD2(stream, spriteStart + kSprChannelSizeD2, finishPosition - fieldPosition);
			fieldPosition = finishPosition;
			break;
		default:
			error("Frame::readSpriteD2(): Miscomputed field position: %d", fieldPosition);
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

enum {
	kMainChannelSizeD4 = 40,
	kSprChannelSizeD4 = 20
};

void Frame::readChannelD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	// 40 bytes header
	if (offset >= kMainChannelSizeD4) {
		if (size <= kSprChannelSizeD4)
			readSpriteD4(stream, offset, size);
		else {
			// read > 1 sprites channel
			while (size > kSprChannelSizeD4) {
				byte spritePosition = (offset - kMainChannelSizeD4) / kSprChannelSizeD4;
				uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD4 + kMainChannelSizeD4;
				uint16 needSize = nextStart - offset;
				readSpriteD4(stream, offset, needSize);
				offset += needSize;
				size -= needSize;
			}
			readSpriteD4(stream, offset, size);
		}
	} else {
		readMainChannelsD4(stream, offset, size);
	}
}

void Frame::readMainChannelsD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readMainChannelsD4(): 40 byte header");
		stream.hexdump(kMainChannelSizeD4);
	}

	uint16 finishPosition = offset + size;
	int unk1;

	while (offset < finishPosition) {
		switch (offset) {
		case 0:
			// Sound/Tempo/Transition
			unk1 = stream.readByte();
			if (unk1) {
				warning("Frame::readMainChannelsD4(): STUB: unk1: %d 0x%x", unk1, unk1);
			}
			offset++;
			break;
		case 1:
			_soundType1 = stream.readByte(); // type: 0x17 for sounds (sound is cast id), 0x16 for MIDI (sound is cmd id)
			offset++;
			break;
		case 2: {
				uint8 transFlags = stream.readByte();
				if (transFlags & 0x80)  // 0x80 is whole stage (vs changed area), rest is duration in 1/4ths of a second
					_transArea = 1;
				else
					_transArea = 0;
				_transDuration = (transFlags & 0x7f) * 250; // Duration is in 1/4 secs
			}

			offset++;
			break;
		case 3:
			_transChunkSize = stream.readByte();
			offset++;
			break;
		case 4:
			_tempo = stream.readByte();
			offset++;
			break;
		case 5:
			_transType = static_cast<TransitionType>(stream.readByte());
			offset++;
			break;
		case 6:
			_sound1 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			offset += 2;
			break;
		case 8:
			_sound2 = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			offset += 2;
			break;
		case 10:
			_soundType2 = stream.readByte();
			offset++;
			break;
		case 11:
			_skipFrameFlag = stream.readByte();
			offset++;
			break;
		case 12:
			_blend = stream.readByte();
			offset++;
			break;
		case 13:
			_colorTempo = stream.readByte();
			offset++;
			break;
		case 14:
			_colorSound1 = stream.readByte();
			offset++;
			break;
		case 15:
			_colorSound2 = stream.readByte();
			offset++;
			break;
		case 16:
			_actionId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			offset += 2;
			break;
		case 18:
			_colorScript = stream.readByte();
			offset++;
			break;
		case 19:
			_colorTrans = stream.readByte();
			offset++;
			break;
		case 20: {
				// palette, 13 bytes
				int16 paletteId = stream.readSint16();
				if (paletteId == 0) {
					_palette.paletteId = CastMemberID(0, 0);
				} else if (paletteId < 0) {
					_palette.paletteId = CastMemberID(paletteId, -1);
				} else {
					_palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
				}
				// loop points for color cycling
				_palette.firstColor = g_director->transformColor(stream.readByte() + 0x80); // 22
				_palette.lastColor = g_director->transformColor(stream.readByte() + 0x80); // 23
				_palette.flags = stream.readByte(); // 24
				_palette.colorCycling = (_palette.flags & 0x80) != 0;
				_palette.normal = (_palette.flags & 0x60) == 0x00;
				_palette.fadeToBlack = (_palette.flags & 0x60) == 0x60;
				_palette.fadeToWhite = (_palette.flags & 0x60) == 0x40;
				_palette.autoReverse = (_palette.flags & 0x10) != 0;
				_palette.overTime = (_palette.flags & 0x04) != 0;
				_palette.speed = stream.readByte(); // 25
				_palette.frameCount = stream.readUint16(); // 26
				_palette.cycleCount = stream.readUint16(); // 28
				_palette.fade = stream.readByte(); // 30
				_palette.delay = stream.readByte(); // 31
				_palette.style = stream.readByte(); // 32
			}
			offset += 13;
			break;
		case 33:
			unk1 = stream.readByte();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk2: %d 0x%x", unk1, unk1);
			offset += 1;
			break;
		case 34:
			unk1 = stream.readUint16();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk3: %d 0x%x", unk1, unk1);
			offset += 2;
			break;
		case 36:
			unk1 = stream.readUint16();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk4: %d 0x%x", unk1, unk1);
			offset += 2;
			break;
		case 38:
			_palette.colorCode = stream.readByte();
			offset++;
			break;
		case 39:
			unk1 = stream.readUint16();
			if (unk1)
				warning("Frame::readMainChannelsD4(): STUB: unk5: %d 0x%x", unk1, unk1);
			offset += 2;
			break;
		case 40:
			break;
		default:
			error("Frame::readMainChannelsD4(): Miscomputed field position: %d", offset);
			break;
		}
	}

	_transChunkSize = CLIP<byte>(_transChunkSize, 0, 128);
	_transDuration = CLIP<uint16>(_transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::readSpriteD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD4) / kSprChannelSizeD4;
	uint16 spriteStart = spritePosition * kSprChannelSizeD4 + kMainChannelSizeD4;

	uint16 fieldPosition = offset - spriteStart;
	uint16 finishPosition = fieldPosition + size;

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

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case 0:
			sprite._scriptId = CastMemberID(stream.readByte(), DEFAULT_CAST_LIB);
			fieldPosition++;
			break;
		case 1:
			sprite._spriteType = (SpriteType)stream.readByte();
			fieldPosition++;

			sprite._enabled = sprite._spriteType != kInactiveSprite;
			break;
		case 2:
			sprite._foreColor = _vm->transformColor((uint8)stream.readByte());
			fieldPosition++;
			break;
		case 3:
			sprite._backColor = _vm->transformColor((uint8)stream.readByte());
			fieldPosition++;
			break;
		case 4:
			sprite._thickness = stream.readByte();
			fieldPosition++;
			break;
		case 5:
			sprite._inkData = stream.readByte();
			fieldPosition++;

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
			fieldPosition += 2;
			break;
		case 8:
			sprite._startPoint.y = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 10:
			sprite._startPoint.x = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 12:
			sprite._height = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 14:
			sprite._width = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 16:
			sprite._scriptId = CastMemberID(stream.readUint16(), DEFAULT_CAST_LIB);
			fieldPosition += 2;
			break;
		case 18:
			// & 0x0f scorecolor
			// 0x10 forecolor is rgb
			// 0x20 bgcolor is rgb
			// 0x40 editable
			// 0x80 moveable
			sprite._colorcode = stream.readByte();
			fieldPosition++;

			sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			break;
		case 19:
			sprite._blendAmount = stream.readByte();
			fieldPosition++;
			break;
		case 20:
			// end of channel, go to next sprite channel
			readSpriteD4(stream, spriteStart + kSprChannelSizeD4, finishPosition - fieldPosition);
			fieldPosition = finishPosition;
			break;
		default:
			error("Frame::readSpriteD4(): Miscomputed field position: %d", fieldPosition);
		}
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

/**************************
 *
 * D5 Loading
 *
 **************************/

enum {
	kMainChannelSizeD5 = 48,
	kSprChannelSizeD5 = 24
};

void Frame::readChannelD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	// 48 bytes header
	if (offset >= kMainChannelSizeD5) {
		if (size <= kSprChannelSizeD5)
			readSpriteD5(stream, offset, size);
		else {
			// read > 1 sprites channel
			while (size > kSprChannelSizeD5) {
				byte spritePosition = (offset - kMainChannelSizeD5) / kSprChannelSizeD5;
				uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD5 + kMainChannelSizeD5;
				uint16 needSize = nextStart - offset;
				readSpriteD5(stream, offset, needSize);
				offset += needSize;
				size -= needSize;
			}
			readSpriteD5(stream, offset, size);
		}
	} else {
		readMainChannelsD5(stream, offset, size);
	}
}

void Frame::readMainChannelsD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (debugChannelSet(8, kDebugLoading)) {
		debugC(8, kDebugLoading, "Frame::readMainChannelsD5(): 40 byte header");
		stream.hexdump(kMainChannelSizeD4);
	}

	uint16 finishPosition = offset + size;
	byte unk[5];

	while (offset < finishPosition) {
		switch (offset) {
		case 0: {
				// Sound/Tempo/Transition
				uint16 actionCastLib = stream.readUint16();
				uint16 actionId = stream.readUint16();
				_actionId = CastMemberID(actionId, actionCastLib);
			}
			offset += 4;
			break;
		case 4: {
				uint16 sound1CastLib = stream.readUint16();
				uint16 sound1Id = stream.readUint16();
				_sound1 = CastMemberID(sound1Id, sound1CastLib);
			}
			offset += 4;
			break;
		case 8: {
				uint16 sound2CastLib = stream.readUint16();
				uint16 sound2Id = stream.readUint16();
				_sound2 = CastMemberID(sound2Id, sound2CastLib);
			}
			offset += 4;
			break;
		case 12: {
				uint16 transCastLib = stream.readUint16();
				uint16 transId = stream.readUint16();
				_trans = CastMemberID(transId, transCastLib);
			}
			offset += 4;
			break;
		case 16:
			stream.read(unk, 5);
			warning("Frame::readMainChannelsD5(): STUB: unk1: 0x%x 0x%x 0x%x 0x%x 0x%x", unk[0], unk[1], unk[2], unk[3], unk[4]);
			offset += 5;
			break;
		case 21:
			_tempo = stream.readByte();
			offset++;
			break;
		case 22:
			stream.read(unk, 2);
			warning("Frame::readMainChannelsD5(): STUB: unk2: 0x%x 0x%x", unk[0], unk[1]);
			offset += 2;
			break;
		case 24: {
				// palette, 14 (26?) bytes
				int16 paletteCastLib = stream.readSint16();
				int16 paletteId = stream.readSint16(); // 26
				_palette.paletteId = CastMemberID(paletteId, paletteCastLib);

				_palette.speed = stream.readByte(); // 28
				_palette.flags = stream.readByte(); // 29
				_palette.colorCycling = (_palette.flags & 0x80) != 0;
				_palette.normal = (_palette.flags & 0x60) == 0x00;
				_palette.fadeToBlack = (_palette.flags & 0x60) == 0x60;
				_palette.fadeToWhite = (_palette.flags & 0x60) == 0x40;
				_palette.autoReverse = (_palette.flags & 0x10) != 0;
				_palette.overTime = (_palette.flags & 0x04) != 0;
				_palette.firstColor = g_director->transformColor(stream.readByte() + 0x80); // 30
				_palette.lastColor = g_director->transformColor(stream.readByte() + 0x80); // 31
				_palette.frameCount = stream.readUint16(); // 32
				_palette.cycleCount = stream.readUint16(); // 34
				stream.read(unk, 12);
			}
			offset += 14 + 12;
			break;
		case 48:
			break;
		default:
			error("Frame::readMainChannelsD5(): Miscomputed field position: %d", offset);
			break;
		}
	}

	_transChunkSize = CLIP<byte>(_transChunkSize, 0, 128);
	_transDuration = CLIP<uint16>(_transDuration, 0, 32000);  // restrict to 32 secs
}

void Frame::readSpriteD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD5) / kSprChannelSizeD5;
	uint16 spriteStart = spritePosition * kSprChannelSizeD5 + kMainChannelSizeD5;

	uint16 fieldPosition = offset - spriteStart;
	uint16 finishPosition = fieldPosition + size;

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

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case 0:
			sprite._spriteType = (SpriteType)stream.readByte();
			fieldPosition++;
			break;
		case 1:
			sprite._inkData = stream.readByte();
			fieldPosition++;

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
			fieldPosition += 4;
			break;
		case 6: {
				uint16 scriptCastLib = stream.readUint16();
				uint16 scriptMemberID = stream.readUint16();
				sprite._scriptId = CastMemberID(scriptMemberID, scriptCastLib);
			}
			fieldPosition += 4;
			break;
		case 10:
			sprite._foreColor = _vm->transformColor((uint8)stream.readByte());
			fieldPosition++;
			break;
		case 11:
			sprite._backColor = _vm->transformColor((uint8)stream.readByte());
			fieldPosition++;
			break;
		case 12:
			sprite._startPoint.y = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 14:
			sprite._startPoint.x = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 16:
			sprite._height = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 18:
			sprite._width = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 20:
			// & 0x0f scorecolor
			// 0x10 forecolor is rgb
			// 0x20 bgcolor is rgb
			// 0x40 editable
			// 0x80 moveable
			sprite._colorcode = stream.readByte();
			fieldPosition++;

			sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			break;
		case 21:
			sprite._blendAmount = stream.readByte();
			fieldPosition++;
			break;
		case 22:
			sprite._thickness = stream.readByte();
			fieldPosition++;
			break;
		case 23:
			(void)stream.readByte(); // unused
			fieldPosition++;
			break;
		case 24:
			// end of channel, go to next sprite channel
			readSpriteD4(stream, spriteStart + kSprChannelSizeD5, finishPosition - fieldPosition);
			fieldPosition = finishPosition;
			break;
		default:
			error("Frame::readSpriteD5(): Miscomputed field position: %d", fieldPosition);
		}
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

/**************************
 *
 * D6 Loading
 *
 **************************/

enum {
	kMainChannelSizeD6 = 48,
	kSprChannelSizeD6 = 24
};

void Frame::readChannelD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	// 48 bytes header
	if (offset >= kMainChannelSizeD6) {
		if (size <= kSprChannelSizeD6)
			readSpriteD6(stream, offset, size);
		else {
			// read > 1 sprites channel
			while (size > kSprChannelSizeD6) {
				byte spritePosition = (offset - kMainChannelSizeD6) / kSprChannelSizeD6;
				uint16 nextStart = (spritePosition + 1) * kSprChannelSizeD6 + kMainChannelSizeD6;
				uint16 needSize = nextStart - offset;
				readSpriteD6(stream, offset, needSize);
				offset += needSize;
				size -= needSize;
			}
			readSpriteD6(stream, offset, size);
		}
	} else {
		readMainChannelsD6(stream, offset, size);
	}
}

void Frame::readMainChannelsD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	error("Frame::readMainChannelsD6(): Miscomputed field position: %d", offset);
}

void Frame::readSpriteD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - kMainChannelSizeD6) / kSprChannelSizeD6;
	uint16 spriteStart = spritePosition * kSprChannelSizeD6 + kMainChannelSizeD6;

	uint16 fieldPosition = offset - spriteStart;
	uint16 finishPosition = fieldPosition + size;

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

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case 0:
			sprite._spriteType = (SpriteType)stream.readByte();
			fieldPosition++;
			break;
		case 1:
			sprite._inkData = stream.readByte();
			fieldPosition++;

			sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
			if (sprite._inkData & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			break;
		case 2:
			sprite._foreColor = _vm->transformColor((uint8)stream.readByte());
			fieldPosition++;
			break;
		case 3:
			sprite._backColor = _vm->transformColor((uint8)stream.readByte());
			fieldPosition++;
			break;
		case 4: {
				uint16 castLib = stream.readUint16();
				uint16 memberID = stream.readUint16();
				sprite._castId = CastMemberID(memberID, castLib);
			}
			fieldPosition += 4;
			break;
		case 8: {
				uint16 scriptCastLib = stream.readUint16();
				uint16 scriptMemberID = stream.readUint16();
				sprite._scriptId = CastMemberID(scriptMemberID, scriptCastLib);
			}
			fieldPosition += 4;
			break;
		case 12:
			sprite._startPoint.y = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 14:
			sprite._startPoint.x = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 16:
			sprite._height = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 18:
			sprite._width = (int16)stream.readUint16();
			fieldPosition += 2;
			break;
		case 20:
			// & 0x0f scorecolor
			// 0x10 forecolor is rgb
			// 0x20 bgcolor is rgb
			// 0x40 editable
			// 0x80 moveable
			sprite._colorcode = stream.readByte();
			fieldPosition++;

			sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
			break;
		case 21:
			sprite._blendAmount = stream.readByte();
			fieldPosition++;
			break;
		case 22:
			sprite._thickness = stream.readByte();
			fieldPosition++;
			break;
		case 23:
			(void)stream.readByte(); // unused
			fieldPosition++;
			break;
		case 24:
			// end of channel, go to next sprite channel
			readSpriteD4(stream, spriteStart + kSprChannelSizeD5, finishPosition - fieldPosition);
			fieldPosition = finishPosition;
			break;
		default:
			error("Frame::readSpriteD6(): Miscomputed field position: %d", fieldPosition);
		}
	}

	// Sometimes removed sprites leave garbage in the channel
	// We set it to zero, so then could skip
	if (sprite._width <= 0 || sprite._height <= 0)
		sprite._width = sprite._height = 0;
}

void Frame::readChannels(Common::SeekableReadStreamEndian *stream, uint16 version) {
	byte unk[24];

	if (version < kFileVer400) {
		// Sound/Tempo/Transition
		_actionId = CastMemberID(stream->readByte(), DEFAULT_CAST_LIB);
		_soundType1 = stream->readByte(); // type: 0x17 for sounds (sound is cast id), 0x16 for MIDI (sound is cmd id)
		uint8 transFlags = stream->readByte(); // 0x80 is whole stage (vs changed area), rest is duration in 1/4ths of a second

		if (transFlags & 0x80)
			_transArea = 1;
		else
			_transArea = 0;
		_transDuration = (transFlags & 0x7f) * 250; // Duration is in 1/4 secs

		_transChunkSize = stream->readByte();
		_tempo = stream->readByte();
		_transType = static_cast<TransitionType>(stream->readByte());
		_sound1 = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);
		_sound2 = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);
		_soundType2 = stream->readByte();

		_skipFrameFlag = stream->readByte();
		_blend = stream->readByte();

		if (_vm->getPlatform() == Common::kPlatformWindows) {
			_sound2 = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);
			_soundType2 = stream->readByte();
		} else {
			stream->read(unk, 3);
			debugC(8, kDebugLoading, "Frame::readChannels(): STUB: unk1: %02x %02x %02x", unk[0], unk[1], unk[2]);
		}

		// palette
		int16 paletteId = stream->readSint16();
		if (paletteId == 0) {
			_palette.paletteId = CastMemberID(0, 0);
		} else if (paletteId < 0) {
			_palette.paletteId = CastMemberID(paletteId, -1);
		} else {
			_palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
		}
		// loop points for color cycling
		_palette.firstColor = g_director->transformColor(stream->readByte() ^ 0x80);
		_palette.lastColor = g_director->transformColor(stream->readByte() ^ 0x80);
		_palette.flags = stream->readByte();
		_palette.colorCycling = (_palette.flags & 0x80) != 0;
		_palette.normal = (_palette.flags & 0x60) == 0x00;
		_palette.fadeToBlack = (_palette.flags & 0x60) == 0x60;
		_palette.fadeToWhite = (_palette.flags & 0x60) == 0x40;
		_palette.autoReverse = (_palette.flags & 0x10) != 0;
		_palette.overTime = (_palette.flags & 0x04) != 0;
		_palette.speed = stream->readByte();
		_palette.frameCount = stream->readUint16();
		_palette.cycleCount = stream->readUint16();

		stream->read(unk, 6);

		debugC(8, kDebugLoading, "Frame::readChannels(): STUB: unk1: %02x %02x %02x %02x %02x %02x", unk[0],
			unk[1], unk[2], unk[3], unk[4], unk[5]);
	} else if (version >= kFileVer400 && version < kFileVer500) {
		if (debugChannelSet(8, kDebugLoading)) {
			debugC(8, kDebugLoading, "Frame::readChannels(): 40 byte header");
			stream->hexdump(40);
		}
		// Sound/Tempo/Transition
		int unk1 = stream->readByte();
		if (unk1) {
			warning("Frame::readChannels(): STUB: unk1: %d 0x%x", unk1, unk1);
		}
		_soundType1 = stream->readByte(); // type: 0x17 for sounds (sound is cast id), 0x16 for MIDI (sound is cmd id)
		uint8 transFlags = stream->readByte(); // 0x80 is whole stage (vs changed area), rest is duration in 1/4ths of a second

		if (transFlags & 0x80)
			_transArea = 1;
		else
			_transArea = 0;
		_transDuration = (transFlags & 0x7f) * 250; // Duration is 1/4 secs

		_transChunkSize = stream->readByte();
		_tempo = stream->readByte();
		_transType = static_cast<TransitionType>(stream->readByte());
		_sound1 = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);

		_sound2 = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);
		_soundType2 = stream->readByte();

		_skipFrameFlag = stream->readByte();
		_blend = stream->readByte();

		_colorTempo = stream->readByte();
		_colorSound1 = stream->readByte();
		_colorSound2 = stream->readByte();

		_actionId = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);

		_colorScript = stream->readByte();
		_colorTrans = stream->readByte();

		// palette
		int16 paletteId = stream->readSint16();
		if (paletteId == 0) {
			_palette.paletteId = CastMemberID(0, 0);
		} else if (paletteId < 0) {
			_palette.paletteId = CastMemberID(paletteId, -1);
		} else {
			_palette.paletteId = CastMemberID(paletteId, DEFAULT_CAST_LIB);
		}
		// loop points for color cycling
		_palette.firstColor = g_director->transformColor(stream->readByte() + 0x80);
		_palette.lastColor = g_director->transformColor(stream->readByte() + 0x80);
		_palette.flags = stream->readByte();
		_palette.colorCycling = (_palette.flags & 0x80) != 0;
		_palette.normal = (_palette.flags & 0x60) == 0x00;
		_palette.fadeToBlack = (_palette.flags & 0x60) == 0x60;
		_palette.fadeToWhite = (_palette.flags & 0x60) == 0x40;
		_palette.autoReverse = (_palette.flags & 0x10) != 0;
		_palette.overTime = (_palette.flags & 0x04) != 0;
		_palette.speed = stream->readByte();
		_palette.frameCount = stream->readUint16();
		_palette.cycleCount = stream->readUint16();
		_palette.fade = stream->readByte();
		_palette.delay = stream->readByte();
		_palette.style = stream->readByte();


		unk1 = stream->readByte();
		if (unk1)
			warning("Frame::readChannels(): STUB: unk2: %d 0x%x", unk1, unk1);
		unk1 = stream->readUint16();
		if (unk1)
			warning("Frame::readChannels(): STUB: unk3: %d 0x%x", unk1, unk1);
		unk1 = stream->readUint16();
		if (unk1)
			warning("Frame::readChannels(): STUB: unk4: %d 0x%x", unk1, unk1);

		_palette.colorCode = stream->readByte();

		unk1 = stream->readByte();
		if (unk1)
			warning("Frame::readChannels(): STUB: unk5: %d 0x%x", unk1, unk1);

	} else if (version >= kFileVer500 && version < kFileVer600) {
		if (debugChannelSet(8, kDebugLoading)) {
			debugC(8, kDebugLoading, "Frame::readChannels(): 48 byte header");
			stream->hexdump(48);
		}
		// Sound/Tempo/Transition channel
		uint16 actionCastLib = stream->readUint16();
		uint16 actionId = stream->readUint16();
		_actionId = CastMemberID(actionId, actionCastLib);
		uint16 sound1CastLib = stream->readUint16();
		uint16 sound1Id = stream->readUint16();
		_sound1 = CastMemberID(sound1Id, sound1CastLib);
		uint16 sound2CastLib = stream->readUint16();
		uint16 sound2Id = stream->readUint16();
		_sound2 = CastMemberID(sound2Id, sound2CastLib);
		uint16 transCastLib = stream->readUint16();
		uint16 transId = stream->readUint16();
		_trans = CastMemberID(transId, transCastLib);

		stream->read(unk, 5);

		_tempo = stream->readByte();

		stream->read(unk, 2);

		// palette
		int16 paletteCastLib = stream->readSint16();
		int16 paletteId = stream->readSint16();
		_palette.paletteId = CastMemberID(paletteId, paletteCastLib);
		_palette.speed = stream->readByte();
		_palette.flags = stream->readByte();
		_palette.colorCycling = (_palette.flags & 0x80) != 0;
		_palette.normal = (_palette.flags & 0x60) == 0x00;
		_palette.fadeToBlack = (_palette.flags & 0x60) == 0x60;
		_palette.fadeToWhite = (_palette.flags & 0x60) == 0x40;
		_palette.autoReverse = (_palette.flags & 0x10) != 0;
		_palette.overTime = (_palette.flags & 0x04) != 0;
		_palette.firstColor = g_director->transformColor(stream->readByte() + 0x80);
		_palette.lastColor = g_director->transformColor(stream->readByte() + 0x80);
		_palette.frameCount = stream->readUint16();
		_palette.cycleCount = stream->readUint16();
		stream->read(unk, 12);
	}

	_transChunkSize = CLIP<byte>(_transChunkSize, 0, 128);
	_transDuration = CLIP<uint16>(_transDuration, 0, 32000);  // restrict to 32 secs

	for (int i = 0; i < _numChannels; i++) {
		Sprite &sprite = *_sprites[i + 1];

		if (version < kFileVer500) {
			if (debugChannelSet(8, kDebugLoading)) {
				debugC(8, kDebugLoading, "Frame::readChannels(): channel %d, 22 bytes", i);
				stream->hexdump(22);
			}
			sprite._scriptId = CastMemberID(stream->readByte(), DEFAULT_CAST_LIB);
			sprite._spriteType = (SpriteType)stream->readByte();
			sprite._enabled = sprite._spriteType != kInactiveSprite;
			if (version >= kFileVer400) {
				sprite._foreColor = _vm->transformColor((uint8)stream->readByte());
				sprite._backColor = _vm->transformColor((uint8)stream->readByte());
			} else {
				// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
				sprite._foreColor = _vm->transformColor((128 + stream->readByte()) & 0xff);
				sprite._backColor = _vm->transformColor((128 + stream->readByte()) & 0xff);
			}

			sprite._thickness = stream->readByte();
			sprite._inkData = stream->readByte();

			if (sprite.isQDShape()) {
				sprite._pattern = stream->readUint16();
			} else {
				sprite._castId = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);
			}

			sprite._startPoint.y = (int16)stream->readUint16();
			sprite._startPoint.x = (int16)stream->readUint16();

			sprite._height = (int16)stream->readUint16();
			sprite._width = (int16)stream->readUint16();

			if (version >= kFileVer400) {
				sprite._scriptId = CastMemberID(stream->readUint16(), DEFAULT_CAST_LIB);
				// & 0x0f scorecolor
				// 0x10 forecolor is rgb
				// 0x20 bgcolor is rgb
				// 0x40 editable
				// 0x80 moveable
				sprite._colorcode = stream->readByte();
				sprite._blendAmount = stream->readByte();
			}
		} else if (version >= kFileVer500 && version < kFileVer600) {
			sprite._spriteType = (SpriteType)stream->readByte();
			sprite._inkData = stream->readByte();

			uint16 castLib = stream->readUint16();
			uint16 memberID = stream->readUint16();
			sprite._castId = CastMemberID(memberID, castLib);

			uint16 scriptCastLib = stream->readUint16();
			uint16 scriptMemberID = stream->readUint16();
			sprite._scriptId = CastMemberID(scriptMemberID, scriptCastLib);

			sprite._foreColor = _vm->transformColor((uint8)stream->readByte());
			sprite._backColor = _vm->transformColor((uint8)stream->readByte());

			sprite._startPoint.y = (int16)stream->readUint16();
			sprite._startPoint.x = (int16)stream->readUint16();

			sprite._height = (int16)stream->readUint16();
			sprite._width = (int16)stream->readUint16();

			sprite._colorcode = stream->readByte();
			sprite._blendAmount = stream->readByte();
			sprite._thickness = stream->readByte();
			stream->readByte();	// unused
		} else if (version >= kFileVer600 && version < kFileVer700) {
			sprite._spriteType = (SpriteType)stream->readByte();
			sprite._inkData = stream->readByte();

			sprite._foreColor = _vm->transformColor((uint8)stream->readByte());
			sprite._backColor = _vm->transformColor((uint8)stream->readByte());

			uint16 castLib = stream->readUint16();
			uint16 memberID = stream->readUint16();
			sprite._castId = CastMemberID(memberID, castLib);

			/* uint32 spriteId = */stream->readUint32();

			sprite._startPoint.y = (int16)stream->readUint16();
			sprite._startPoint.x = (int16)stream->readUint16();

			sprite._height = (int16)stream->readUint16();
			sprite._width = (int16)stream->readUint16();

			sprite._colorcode = stream->readByte();
			sprite._blendAmount = stream->readByte();
			sprite._thickness = stream->readByte();
			stream->readByte();	// unused
		}

		// Sometimes removed sprites leave garbage in the channel
		// We set it to zero, so then could skip
		if (sprite._width <= 0 || sprite._height <= 0)
			sprite._width = sprite._height = 0;

		sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);
		sprite._editable = ((sprite._colorcode & 0x40) == 0x40);
		sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);

		if (sprite._inkData & 0x40)
			sprite._trails = 1;
		else
			sprite._trails = 0;

		sprite._moveable = ((sprite._colorcode & 0x80) == 0x80);
	}

	if (debugChannelSet(4, kDebugLoading)) {
		debugC(4, kDebugLoading, "%s", formatChannelInfo().c_str());
	}
}

Common::String Frame::formatChannelInfo() {
	Common::String result;
	result += Common::String::format("TMPO:   tempo: %d, skipFrameFlag: %d, blend: %d\n",
		_tempo, _skipFrameFlag, _blend);
	if (_palette.paletteId.isNull()) {
		result += Common::String::format("PAL:    paletteId: %s, firstColor: %d, lastColor: %d, flags: %d, cycleCount: %d, speed: %d, frameCount: %d, fade: %d, delay: %d, style: %d\n",
			_palette.paletteId.asString().c_str(), _palette.firstColor, _palette.lastColor, _palette.flags,
			_palette.cycleCount, _palette.speed, _palette.frameCount,
			_palette.fade, _palette.delay, _palette.style);
	} else {
		result += Common::String::format("PAL:    paletteId: 000\n");
	}
	result += Common::String::format("TRAN:   transType: %d, transDuration: %d, transChunkSize: %d\n",
		_transType, _transDuration, _transChunkSize);
	result += Common::String::format("SND: 1  sound1: %d, soundType1: %d\n", _sound1.member, _soundType1);
	result += Common::String::format("SND: 2  sound2: %d, soundType2: %d\n", _sound2.member, _soundType2);
	result += Common::String::format("LSCR:   actionId: %d\n", _actionId.member);

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
