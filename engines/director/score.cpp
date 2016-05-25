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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "director/score.h"
#include "common/stream.h"
#include "common/debug.h"

#include "common/system.h"
#include "director/dib.h"
#include "director/resource.h"

#include "graphics/palette.h"
#include "common/events.h"
#include "engines/util.h"

namespace Director {

Score::Score(Common::SeekableReadStream &stream) {

	uint32 size = stream.readUint32BE();
	size -= 4;
	uint16 channelSize;
	uint16 channelOffset;

	Frame *initial = new Frame();
	_frames.push_back(initial);
	
	while (size != 0) {
		uint16 frameSize = stream.readUint16BE();
		size -= frameSize;
		frameSize -= 2;
		Frame *frame = new Frame(*_frames.back());
		while (frameSize != 0) {
			channelSize = stream.readByte() * 2;
			channelOffset = stream.readByte() * 2;
			frame->readChannel(stream, channelOffset, channelSize);
			frameSize -= channelSize + 2;
		}
		_frames.push_back(frame);
	}
	//remove initial frame
	_frames.remove_at(0);
}

void Score::play() {
	initGraphics(800, 800, true);
	uint32 frameId = 0;

	bool stop = false;

	while (frameId != _frames.size() && !stop) {
		Common::Event event;

		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT)
				stop = true;
		}
		_frames[frameId]->display();
		frameId++;
		g_system->updateScreen();
		g_system->delayMillis(200);
	}
}

Frame::Frame() {
	_transFlags = 0;
	_transChunkSize = 0;
	_tempo = 0;
	
	_sound1 = 0;
	_sound2 = 0;
	_soundType1 = 0;
	_soundType2 = 0;
	
	_actionId = 0;
	_skipFrameFlag = 0;
	_blend = 0;

	_sprites.resize(CHANNEL_COUNT);
	for (uint16 i = 0; i < _sprites.size(); i++) {
		Sprite *sp = new Sprite();
		_sprites[i] = sp;
	}
}

Frame::Frame(const Frame &frame) {
	_actionId = frame._actionId;
	_transFlags = frame._transFlags;
	_transType = frame._transType;
	_tempo = frame._tempo;
	_sound1 = frame._sound1;
	_sound2 = frame._sound2;
	_soundType1 = frame._soundType1;
	_soundType2 = frame._soundType2;
	_skipFrameFlag = frame._skipFrameFlag;
	_blend = frame._blend;

	_sprites.resize(CHANNEL_COUNT);
	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		_sprites[i] = new Sprite(*frame._sprites[i]);
	}
}

Frame::~Frame() {
	for (uint16 i = 0; i < _sprites.size(); i++) {
		delete _sprites[i];
	}
}

void Frame::readChannel(Common::SeekableReadStream &stream, uint16 offset, uint16 size) {
	if (offset >= 32) {
		if (size <= 16)
			readSprite(stream, offset, size);
		else {
			//read > 1 sprites channel
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

void Frame::readMainChannels(Common::SeekableReadStream &stream, uint16 offset, uint16 size) {
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
		case kTransFlagsPosition:
			_transFlags = stream.readByte();
			offset++;
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
			_transType = stream.readByte();
			offset++;
			break;
		case kSound1Position:
			_sound1 = stream.readUint16BE();
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
			_sound2 = stream.readUint16BE();
			offset += 2;
			break;
		case kSound2TypePosition:
			_soundType2 = stream.readByte();
			offset += 1;
			break;
		case kPaletePosition:
			//TODO palette channel
			stream.skip(16);
			offset += 16;
		default:
			offset++;
			stream.readByte();
			debug("Field Position %d, Finish Position %d", offset, finishPosition);
			break;
		}
	}
}

void Frame::readSprite(Common::SeekableReadStream &stream, uint16 offset, uint16 size) {
	uint16 spritePosition = (offset - 32) / 16;
	uint16 spriteStart = spritePosition * 16 + 32;

	uint16 fieldPosition = offset - spriteStart;
	uint16 finishPosition = fieldPosition + size;

	Sprite &sprite = *_sprites[spritePosition];

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case kSpritePositionUnk1:
			/*byte x1 = */ stream.readByte();
			fieldPosition++;
			break;
		case kSpritePositionEnabled:
			sprite._enabled = (stream.readByte() != 0);
			fieldPosition++;
			break;
		case kSpritePositionUnk2:
			/*byte x2 = */ stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionFlags:
			sprite._flags = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionCastId:
			sprite._castId = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionY:
			sprite._startPoint.y = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionX:
			sprite._startPoint.x = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionWidth:
			sprite._width = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionHeight:
			sprite._height = stream.readUint16BE();
			fieldPosition += 2;
			break;
		default:
			//end cycle, go to next sprite channel
			readSprite(stream, spriteStart + 16, finishPosition - fieldPosition);
			fieldPosition = finishPosition;
			break;
		}
	}
}

void Frame::display() {
	//FIXME
	RIFFArchive riff;
	riff.openFile("bookshelf_example.mmm");

	DIBDecoder palette;
	Common::SeekableReadStream *pal = riff.getResource(MKTAG('C', 'L', 'U', 'T'), 1025);
	palette.loadPalette(*pal);
	g_system->getPaletteManager()->setPalette(palette.getPalette(), 0, 255);

	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		if (_sprites[i]->_enabled) {
			DIBDecoder img;
			uint32 castId = 1024 + _sprites[i]->_castId;
			img.loadStream(*riff.getResource(MKTAG('D', 'I', 'B', ' '), castId));
			g_system->copyRectToScreen(img.getSurface()->getPixels(), img.getSurface()->pitch,
				_sprites[i]->_startPoint.x,
				_sprites[i]->_startPoint.y,
				_sprites[i]->_height,
				_sprites[i]->_width);
		}
	}
}

Sprite::Sprite() { 
	_enabled = false;
	_width = 0;
	_flags = 0;
	_height = 0;
	_castId = 0;
	_castId = 0;
}

Sprite::Sprite(const Sprite &sprite) {
	_enabled = sprite._enabled;
	_castId = sprite._castId;
	_flags = sprite._flags;
	_width = sprite._width;
	_height = sprite._height;
	_startPoint.x = sprite._startPoint.x;
	_startPoint.y = sprite._startPoint.y;
}

} //End of namespace Director
