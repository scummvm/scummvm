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

namespace Director {

Score::Score(Common::SeekableReadStream &stream) {

	uint32 size = stream.readUint32BE();
	size -= 4;
	uint16 channelSize;
	uint16 channelOffset;

	Frame* initial = new Frame();
	frames.push_back(initial);
	
	while (size != 0) {
		uint16 frameSize = stream.readUint16BE();
		size -= frameSize;
		frameSize -= 2;
		Frame* frame = new Frame(*frames.back());
		while(frameSize != 0) {
			channelSize = stream.readByte() * 2;
			channelOffset = stream.readByte() * 2;
			frame->readChannel(stream, channelOffset, channelSize);
			frameSize -= channelSize + 2;
		}
		frames.push_back(frame);
		debug("*************FRAME %d****************", frames.size() - 1);
		for (byte i = 0; i < CHANNEL_COUNT; i++) {
			debug("Channel %d, enabled %d, castId %d, X %d, Y %d, width %d, height %d",
				i, frame->sprites[i]->enabled, 
				frame->sprites[i]->castId, 
				frame->sprites[i]->startPoint.x, 
				frame->sprites[i]->startPoint.y, 
				frame->sprites[i]->width, 
				frame->sprites[i]->height);
		}
	}
	//remove initial frame
	frames.remove_at(0);
}

Frame::Frame() {
	transFlags = 0;
	transChunkSize = 0;
	tempo = 0;
	
	sound1 = 0;
	sound2 = 0;
	soundType1 = 0;
	soundType2 = 0;
	
	actionId = 0;
	skipFrameFlag = 0;
	blend = 0;

	sprites.resize(CHANNEL_COUNT);
	for (uint16 i = 0; i < sprites.size(); i++) {
		Sprite* sp = new Sprite();
		sprites[i] = sp;
	}
}

Frame::Frame(const Frame& frame) {
	actionId = frame.actionId;
	transFlags = frame.transFlags;
	transType = frame.transType;
	tempo = frame.tempo;
	sound1 = frame.sound1;
	sound2 = frame.sound2;
	soundType1 = frame.soundType1;
	soundType2 = frame.soundType2;
	skipFrameFlag = frame.skipFrameFlag;
	blend = frame.blend;

	sprites.resize(CHANNEL_COUNT);
	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		sprites[i] = new Sprite(*frame.sprites[i]);;
	}
}

Frame::~Frame() {
	for (uint16 i = 0; i < sprites.size(); i++) {
		delete sprites[i];
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
			actionId = stream.readByte();
			offset++;
			break;
		case kSoundType1Position:
			soundType1 = stream.readByte();
			offset++;
			break;
		case kTransFlagsPosition:
			transFlags = stream.readByte();
			offset++;
			break;
		case kTransChunkSizePosition:
			transChunkSize = stream.readByte();
			offset++;
			break;
		case kTempoPosition:
			tempo = stream.readByte();
			offset++;
			break;
		case kTransTypePosition:
			transType = stream.readByte();
			offset++;
			break;
		case kSound1Position:
			sound1 = stream.readUint16BE();
			offset+=2;
			break;
		case kSkipFrameFlagsPosition:
			skipFrameFlag = stream.readByte();
			offset++;
			break;
		case kBlendPosition:
			blend = stream.readByte();
			offset++;
			break;
		case kSound2Position:
			sound2 = stream.readUint16BE();
			offset += 2;
			break;
		case kSound2TypePosition:
			soundType2 = stream.readByte();
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

	Sprite& sprite = *sprites[spritePosition];

	while (fieldPosition < finishPosition) {
		switch (fieldPosition) {
		case kSpritePositionUnk1:
			/*byte x1 = */ stream.readByte();
			fieldPosition++;
			break;
		case kSpritePositionEnabled:
			sprite.enabled = (stream.readByte() != 0);
			fieldPosition++;
			break;
		case kSpritePositionUnk2:
			/*byte x2 = */ stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionFlags:
			sprite.flags = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionCastId:
			sprite.castId = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionY:
			sprite.startPoint.y = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionX:
			sprite.startPoint.x = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionWidth:
			sprite.width = stream.readUint16BE();
			fieldPosition += 2;
			break;
		case kSpritePositionHeight:
			sprite.height = stream.readUint16BE();
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

Sprite::Sprite() { 
	enabled = false;
	width = 0;
	flags = 0;
	height = 0;
	castId = 0;
	castId = 0;
}

Sprite::Sprite(const Sprite& sprite) {
	enabled = sprite.enabled;
	castId = sprite.castId;
	flags = sprite.flags;
	width = sprite.width;
	height = sprite.height;
	startPoint.x = sprite.startPoint.x;
	startPoint.y = sprite.startPoint.y;
}

} //End of namespace Director