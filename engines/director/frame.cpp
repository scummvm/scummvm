/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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
	_transDuration = 0;
	_transType = kTransNone;
	_transArea = 0;
	_transChunkSize = 0;
	_tempo = 0;

	_numChannels = numChannels;

	_sound1 = 0;
	_sound2 = 0;
	_soundType1 = 0;
	_soundType2 = 0;

	_actionId = 0;
	_skipFrameFlag = 0;
	_blend = 0;

	_colorTempo = 0;
	_colorSound1 = 0;
	_colorSound2 = 0;
	_colorScript = 0;
	_colorTrans = 0;

	_sprites.resize(_numChannels + 1);

	for (uint16 i = 0; i < _sprites.size(); i++) {
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

	debugC(1, kDebugLoading, "Frame. action: %d transType: %d transDuration: %d", _actionId, _transType, _transDuration);

	_sprites.resize(_numChannels + 1);

	for (uint16 i = 0; i <= _numChannels; i++) {
		_sprites[i] = new Sprite(*frame._sprites[i]);
	}
}

Frame::~Frame() {
	for (uint16 i = 0; i < _sprites.size(); i++)
		delete _sprites[i];
}

void Frame::readChannel(Common::SeekableReadStreamEndian &stream, uint16 offset, uint16 size) {
	if (offset >= 32) {
		if (size <= 16)
			readSprite(stream, offset, size);
		else {
			// read > 1 sprites channel
			while (size > 16) {
				byte spritePosition = (offset - 32) / 16;
				uint16 nextStart = (spritePosition + 1) * 16 + 32;
				uint16 needSize = nextStart - offset;
				readSprite(stream, offset, needSize);
				offset += needSize;
				size -= needSize;
			}
			readSprite(stream, offset, size);
		}
	} else {
		readMainChannels(stream, offset, size);
	}
}

void Frame::readChannels(Common::ReadStreamEndian *stream) {
	byte unk[48];

	if (_vm->getVersion() < 400) {
		// Sound/Tempo/Transition
		_actionId = stream->readByte();
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
		_sound1 = stream->readUint16();

		if (_vm->getPlatform() == Common::kPlatformMacintosh) {
			_sound2 = stream->readUint16();
			_soundType2 = stream->readByte();
		} else {
			stream->read(unk, 3);
			if (unk[0] != 0 || unk[1] != 0 || unk[2] != 0)
				warning("Frame::readChannels(): unk1: %x unk2: %x unk3: %x", unk[0], unk[1], unk[2]);
		}
		_skipFrameFlag = stream->readByte();
		_blend = stream->readByte();

		if (_vm->getPlatform() != Common::kPlatformMacintosh) {
			_sound2 = stream->readUint16();
			_soundType2 = stream->readByte();
		}

		// palette
		if (_vm->getPlatform() == Common::kPlatformWindows) {
			_palette.paletteId = stream->readUint16();
			_palette.firstColor = stream->readByte(); // for cycles. note: these start at 0x80 (for pal entry 0)!
			_palette.lastColor = stream->readByte();
			_palette.flags = stream->readByte();
			_palette.speed = stream->readByte();
			_palette.frameCount = stream->readUint16();
			_palette.cycleCount = stream->readUint16();

			stream->read(unk, 6);
		} else {
			stream->read(unk, 4);

			_palette.paletteId = stream->readByte();
			_palette.firstColor = stream->readByte(); // for cycles. note: these start at 0x80 (for pal entry 0)!
			_palette.lastColor = stream->readByte();
			_palette.flags = stream->readByte();
			_palette.cycleCount = stream->readByte();
			_palette.speed = stream->readByte();
			_palette.frameCount = stream->readByte();
			_palette.cycleLength = stream->readByte();

			stream->read(unk, 4);
		}

		debugC(8, kDebugLoading, "Frame::readChannels(): %d %d %d %d %d %d %d %d %d %d %d", _actionId, _soundType1, _transDuration, _transChunkSize, _tempo, _transType, _sound1, _skipFrameFlag, _blend, _sound2, _soundType2);

		if (_vm->getPlatform() == Common::kPlatformMacintosh)
			stream->read(unk, 3);
	} else if (_vm->getVersion() >= 400 && _vm->getVersion() < 500) {
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
		_sound1 = stream->readUint16();

		_sound2 = stream->readUint16();
		_soundType2 = stream->readByte();

		_skipFrameFlag = stream->readByte();
		_blend = stream->readByte();

		_colorTempo = stream->readByte();
		_colorSound1 = stream->readByte();
		_colorSound2 = stream->readByte();

		_actionId = stream->readUint16();

		_colorScript = stream->readByte();
		_colorTrans = stream->readByte();

		// palette
		_palette.paletteId = stream->readSint16();
		_palette.firstColor = stream->readByte(); // for cycles. note: these start at 0x80 (for pal entry 0)!
		_palette.lastColor = stream->readByte();
		_palette.flags = stream->readByte();
		_palette.speed = stream->readByte();
		_palette.frameCount = stream->readUint16();
		_palette.cycleCount = stream->readUint16();
		_palette.fade = stream->readByte();
		_palette.delay = stream->readByte();
		_palette.style = stream->readByte();

		stream->readByte();
		stream->readUint16();
		stream->readUint16();

		_palette.colorCode = stream->readByte();

		stream->readByte();

		debugC(8, kDebugLoading, "Frame::readChannels(): %d %d %d %d %d %d %d %d %d %d %d", _actionId, _soundType1, _transDuration, _transChunkSize, _tempo, _transType, _sound1, _skipFrameFlag, _blend, _sound2, _soundType2);
	} else if (_vm->getVersion() >= 500 && _vm->getVersion() < 600) {
		// Sound/Tempo/Transition channel
		stream->read(unk, 24);

		// palette
		stream->read(unk, 24);
	} else {
		// Sound[2]
		// palette
		// Transition
		// Tempo
		// Script
	}

	_transChunkSize = CLIP<byte>(_transChunkSize, 0, 128);
	_transDuration = CLIP<uint16>(_transDuration, 0, 32000);  // restrict to 32 secs

	for (int i = 0; i < _numChannels; i++) {
		Sprite &sprite = *_sprites[i + 1];

		if (_vm->getVersion() < 500) {
			sprite._scriptId = stream->readByte();
			sprite._spriteType = (SpriteType)stream->readByte();
			sprite._enabled = sprite._spriteType != kInactiveSprite;
			if (_vm->getVersion() >= 400) {
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
				sprite._castId = stream->readUint16();
			}

			sprite._startPoint.y = (int16)stream->readUint16();
			sprite._startPoint.x = (int16)stream->readUint16();

			sprite._height = (int16)stream->readUint16();
			sprite._width = (int16)stream->readUint16();

			if (_vm->getVersion() >= 400) {
				sprite._scriptId = stream->readUint16();
				// & 0x0f scorecolor
				// 0x10 forecolor is rgb
				// 0x20 bgcolor is rgb
				// 0x40 editable
				// 0x80 moveable
				sprite._colorcode = stream->readByte();
				sprite._blendAmount = stream->readByte();
			}
		} else if (_vm->getVersion() >= 500 && _vm->getVersion() < 600) {
			sprite._spriteType = (SpriteType)stream->readByte();
			sprite._inkData = stream->readByte();

			sprite._castIndex = stream->readUint16();
			sprite._castId = stream->readUint16();

			sprite._scriptCastIndex = stream->readUint16();
			sprite._scriptId = stream->readUint16();

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
		} else if (_vm->getVersion() >= 600 && _vm->getVersion() < 700) {
			sprite._spriteType = (SpriteType)stream->readByte();
			sprite._inkData = stream->readByte();

			sprite._foreColor = _vm->transformColor((uint8)stream->readByte());
			sprite._backColor = _vm->transformColor((uint8)stream->readByte());

			sprite._castIndex = stream->readUint16();
			sprite._castId = stream->readUint16();

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

		if (sprite._castId) {
			debugC(4, kDebugLoading, "CH: %-3d castId: %03d(%s) [flags:%04x [ink: %x trails: %d line: %d], %dx%d@%d,%d type: %d fg: %d bg: %d] script: %d, flags2: %x, unk2: %x, unk3: %x",
				i + 1, sprite._castId, numToCastNum(sprite._castId), sprite._inkData,
				sprite._ink, sprite._trails, sprite._thickness, sprite._width, sprite._height,
				sprite._startPoint.x, sprite._startPoint.y,
				sprite._spriteType, sprite._foreColor, sprite._backColor, sprite._scriptId, sprite._colorcode, sprite._blendAmount, sprite._unk3);
		} else {
			debugC(4, kDebugLoading, "CH: %-3d castId: 000", i + 1);
		}
	}
}

void Frame::readMainChannels(Common::SeekableReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 finishPosition = offset + size;

	while (offset < finishPosition) {
		switch(offset) {
		case kScriptIdPosition:
			_actionId = stream.readByte();
			offset++;
			break;
		case kSoundType1Position:
			_soundType1 = stream.readByte();
			offset++;
			break;
		case kTransFlagsPosition: {
				uint8 transFlags = stream.readByte();
				if (transFlags & 0x80)
					_transArea = 1;
				else
					_transArea = 0;
				_transDuration = (transFlags & 0x7f) * 250; // Duration is in 1/4 secs
				offset++;
			}
			break;
		case kTransChunkSizePosition:
			_transChunkSize = stream.readByte();
			offset++;
			break;
		case kTempoPosition:
			_tempo = stream.readByte();
			offset++;
			break;
		case kTransTypePosition:
			_transType = static_cast<TransitionType>(stream.readByte());
			offset++;
			break;
		case kSound1Position:
			_sound1 = stream.readUint16();
			offset+=2;
			break;
		case kSkipFrameFlagsPosition:
			_skipFrameFlag = stream.readByte();
			offset++;
			break;
		case kBlendPosition:
			_blend = stream.readByte();
			offset++;
			break;
		case kSound2Position:
			_sound2 = stream.readUint16();
			offset += 2;
			break;
		case kSound2TypePosition:
			_soundType2 = stream.readByte();
			offset += 1;
			break;
		case kPalettePosition:
			if (stream.readUint16())
				readPaletteInfo(stream);
			offset += 16;
			break;
		default:
			offset++;
			stream.readByte();
			debugC(1, kDebugLoading, "Frame::readMainChannels: Field Position %d, Finish Position %d", offset, finishPosition);
			break;
		}
	}

	debugC(1, kDebugLoading, "Frame::readChannels(): %d %d %d %d %d %d %d %d %d %d %d", _actionId, _soundType1, _transDuration, _transChunkSize, _tempo, _transType, _sound1, _skipFrameFlag, _blend, _sound2, _soundType2);
}

void Frame::readPaletteInfo(Common::SeekableReadStreamEndian &stream) {
	_palette.firstColor = stream.readByte();
	_palette.lastColor = stream.readByte();
	_palette.flags = stream.readByte();
	_palette.speed = stream.readByte();
	_palette.frameCount = stream.readUint16();
	stream.skip(8); // unknown
}

void Frame::readSprite(Common::SeekableReadStreamEndian &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - 32) / 16;
	uint16 spriteStart = spritePosition * 16 + 32;

	uint16 fieldPosition = offset - spriteStart;
	uint16 finishPosition = fieldPosition + size;

	Sprite &sprite = *_sprites[spritePosition];
	int x1 = 0;
	int x2 = 0;

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case kSpritePositionUnk1:
			x1 = stream.readByte();
			fieldPosition++;
			break;
		case kSpritePositionEnabled:
			sprite._enabled = (stream.readByte() != 0);
			fieldPosition++;
			break;
		case kSpritePositionUnk2:
			x2 = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionFlags:
			sprite._thickness = stream.readByte();
			sprite._inkData = stream.readByte();
			sprite._ink = static_cast<InkType>(sprite._inkData & 0x3f);

			if (sprite._inkData & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			fieldPosition += 2;
			break;
		case kSpritePositionCastId:
			sprite._castId = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionY:
			sprite._startPoint.y = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionX:
			sprite._startPoint.x = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionWidth:
			sprite._width = stream.readUint16();
			fieldPosition += 2;
			break;
		case kSpritePositionHeight:
			sprite._height = stream.readUint16();
			fieldPosition += 2;
			break;
		default:
			// end of channel, go to next sprite channel
			readSprite(stream, spriteStart + 16, finishPosition - fieldPosition);
			fieldPosition = finishPosition;
			break;
		}
	}
	warning("Frame::readSprite(): %03d(%d)[%x,%x,%02x %02x,%d/%d/%d/%d]", sprite._castId, sprite._enabled, x1, x2, sprite._thickness, sprite._inkData, sprite._startPoint.x, sprite._startPoint.y, sprite._width, sprite._height);

}

} // End of namespace Director
