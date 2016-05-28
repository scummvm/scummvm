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

void Score::loadConfig(Common::SeekableReadStream &stream) {
	/*uint16 unk1 = */ stream.readUint16BE();
	/*ver1 = */ stream.readUint16BE();
	_movieRect = readRect(stream);

	_castArrayStart = stream.readUint16BE();
	_castArrayEnd = stream.readUint16BE();
	_currentFrameRate = stream.readByte();
	stream.skip(9);
	/*uint16 stageColor = */ stream.readUint16BE();
}

void Score::readVersion(uint32 rid) {
	_versionMinor = rid & 0xffff;
	_versionMajor = rid >> 16;
	debug("%d.%d", _versionMajor, _versionMinor);
}

void Score::loadCastData(Common::SeekableReadStream &stream) {
	for (uint16 id = _castArrayStart; id < _castArrayEnd; id++) {
		byte size = stream.readByte();
		if (size == 0)
			continue;

		uint8 castType = stream.readByte();
		switch (castType) {
		case kCastBitmap:
			_casts[id] = getBitmapCast(stream);
			_casts[id]->type = kCastBitmap;
			break;
		case kCastText:
			_casts[id] = getTextCast(stream);
			_casts[id]->type = kCastText;
			break;
		case kCastShape:
			_casts[id] = getShapeCast(stream);
			_casts[id]->type = kCastShape;
			break;
		case kCastButton:
			_casts[id] = getButtonCast(stream);
			_casts[id]->type = kCastButton;
			break;
		default:
			warning("Unhandled cast type: %d", castType);
			stream.skip(size - 1);
			break;
		}
	}
	//Set cast pointers to sprites
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			byte castId = _frames[i]->_sprites[j]->_castId;
			if (_casts.contains(castId))
				_frames[i]->_sprites[j]->_cast = _casts.find(castId)->_value;
		}
	}
}

BitmapCast *Score::getBitmapCast(Common::SeekableReadStream &stream) {
	BitmapCast *cast = new BitmapCast();
	/*byte flags = */ stream.readByte();
	/*uint16 someFlaggyThing = */ stream.readUint16BE();
	cast->initialRect = readRect(stream);
	cast->boundingRect = readRect(stream);
	cast->regY = stream.readUint16BE();
	cast->regX = stream.readUint16BE();
	/*uint16 unk1 =*/ stream.readUint16BE();
	/*uint16 unk2 =*/ stream.readUint16BE();
	return cast;
}

TextCast *Score::getTextCast(Common::SeekableReadStream &stream) {
	TextCast *cast = new TextCast();
	/*byte flags =*/ stream.readByte();
	cast->borderSize = stream.readByte();
	cast->gutterSize = stream.readByte();
	cast->boxShadow = stream.readByte();
	cast->textType = stream.readByte();
	cast->textAlign = stream.readUint16BE();
	stream.skip(6); //palinfo
	/*uint32 unk1 = */ stream.readUint32BE();
	cast->initialRect = readRect(stream);
	cast->textShadow = stream.readByte();
	cast->textFlags = stream.readByte();
	/*uint16 unk2 =*/ stream.readUint16BE();
	return cast;
}

ShapeCast *Score::getShapeCast(Common::SeekableReadStream &stream) {
	ShapeCast *cast = new ShapeCast();
	/*byte flags = */ stream.readByte();
	/*unk1 = */ stream.readByte();
	cast->shapeType = stream.readByte();
	cast->initialRect = readRect(stream);
	cast->pattern = stream.readUint16BE();
	cast->fgCol = stream.readByte();
	cast->bgCol = stream.readByte();
	cast->fillType = stream.readByte();
	cast->lineThickness = stream.readByte();
	cast->lineDirection = stream.readByte();
	return cast;
}

ButtonCast *Score::getButtonCast(Common::SeekableReadStream &stream) {
	ButtonCast *cast = new ButtonCast();
	/*byte flags =*/ stream.readByte();
	cast->borderSize = stream.readByte();
	cast->gutterSize = stream.readByte();
	cast->boxShadow = stream.readByte();
	cast->textType = stream.readByte();
	cast->textAlign = stream.readUint16BE();
	stream.skip(6); //palinfo
	/*uint32 unk1 = */ stream.readUint32BE();
	cast->initialRect = readRect(stream);
	cast->textShadow = stream.readByte();
	cast->textFlags = stream.readByte();
	/*uint16 unk2 =*/ stream.readUint16BE();
	cast->buttonType = stream.readUint16BE();
	return cast;
}

Common::Rect Score::readRect(Common::SeekableReadStream &stream) {
	Common::Rect *rect = new Common::Rect();
	rect->top = stream.readUint16BE();
	rect->left = stream.readUint16BE();
	rect->bottom = stream.readUint16BE();
	rect->right = stream.readUint16BE();
	return *rect;
}

void Score::play() {
	initGraphics(800, 600, true);

	_currentFrame = 0;
	_stopPlay = false;
	_nextFrameTime = 0;
	while (!_stopPlay) {
		display();
		processEvents();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void Score::display() {
	if (g_system->getMillis() < _nextFrameTime)
		return;

	_frames[_currentFrame]->display();
	_currentFrame++;
	byte tempo = _frames[_currentFrame]->_tempo;
	if (tempo) {
		if (tempo > 161) {
			//Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;
		} else {
			//FPS
			_nextFrameTime = g_system->getMillis() + (float)tempo / 60 * 1000;
			_currentFrameRate = tempo;
		}
	}
	_nextFrameTime = g_system->getMillis() + (float)_currentFrameRate / 60 * 1000;
}

void Score::processEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT)
			_stopPlay = true;
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
			//TODO check cast type
			uint32 imgId = 1024 + _sprites[i]->_castId;
			img.loadStream(*riff.getResource(MKTAG('D', 'I', 'B', ' '), imgId));
			uint32 regX = static_cast<BitmapCast *>(_sprites[i]->_cast)->regX;
			uint32 regY = static_cast<BitmapCast *>(_sprites[i]->_cast)->regY;
			uint32 rectLeft = static_cast<BitmapCast *>(_sprites[i]->_cast)->initialRect.left;
			uint32 rectTop = static_cast<BitmapCast *>(_sprites[i]->_cast)->initialRect.top;

			int x = _sprites[i]->_startPoint.x - regX + rectLeft;
			int y = _sprites[i]->_startPoint.y - regY + rectTop;
			int height = _sprites[i]->_height;
			int width = _sprites[i]->_width;
			if (x < 0) {
				width += x;
				x = 0;
			}
			if (y < 0) {
				height += y;
				y = 0;
			}
			g_system->copyRectToScreen(img.getSurface()->getPixels(), img.getSurface()->pitch, x, y, height, width);
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
