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

#include "common/system.h"
#include "graphics/font.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "image/bmp.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/frame.h"
#include "director/images.h"
#include "director/archive.h"
#include "director/score.h"
#include "director/sprite.h"

namespace Director {

Frame::Frame(DirectorEngine *vm) {
	_vm = vm;
	_transDuration = 0;
	_transType = kTransNone;
	_transArea = 0;
	_transChunkSize = 0;
	_tempo = 0;

	_sound1 = 0;
	_sound2 = 0;
	_soundType1 = 0;
	_soundType2 = 0;

	_actionId = 0;
	_skipFrameFlag = 0;
	_blend = 0;

	_palette = NULL;

	_sprites.resize(CHANNEL_COUNT + 1);

	for (uint16 i = 0; i < _sprites.size(); i++) {
		Sprite *sp = new Sprite();
		_sprites[i] = sp;
	}
}

Frame::Frame(const Frame &frame) {
	_vm = frame._vm;
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
	_palette = new PaletteInfo();

	debugC(1, kDebugLoading, "Frame. action: %d transType: %d transDuration: %d", _actionId, _transType, _transDuration);

	_sprites.resize(CHANNEL_COUNT + 1);

	for (uint16 i = 0; i < CHANNEL_COUNT + 1; i++) {
		_sprites[i] = new Sprite(*frame._sprites[i]);
	}
}

Frame::~Frame() {
	delete _palette;
}

void Frame::readChannel(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size) {
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
	_actionId = stream->readByte();
	_soundType1 = stream->readByte(); // type: 0x17 for sounds (sound is cast id), 0x16 for MIDI (sound is cmd id)
	uint8 transFlags = stream->readByte(); // 0x80 is whole stage (vs changed area), rest is duration in 1/4ths of a second

	if (transFlags & 0x80)
		_transArea = 1;
	else
		_transArea = 0;
	_transDuration = transFlags & 0x7f;

	_transChunkSize = stream->readByte();
	_tempo = stream->readByte();
	_transType = static_cast<TransitionType>(stream->readByte());
	_sound1 = stream->readUint16();
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		_sound2 = stream->readUint16();
		_soundType2 = stream->readByte();
	} else {
		byte unk[3];
		stream->read(unk, 3);
		warning("unk1: %x unk2: %x unk3: %x", unk[0], unk[1], unk[2]);
	}
	_skipFrameFlag = stream->readByte();
	_blend = stream->readByte();

	if (_vm->getPlatform() != Common::kPlatformMacintosh) {
		_sound2 = stream->readUint16();
		_soundType2 = stream->readByte();
	}

	uint16 palette = stream->readUint16();

	if (palette) {
		warning("STUB: Palette info");
	}

	debugC(kDebugLoading, 8, "%d %d %d %d %d %d %d %d %d %d %d", _actionId, _soundType1, _transDuration, _transChunkSize, _tempo, _transType, _sound1, _skipFrameFlag, _blend, _sound2, _soundType2);

	_palette = new PaletteInfo();
	_palette->firstColor = stream->readByte(); // for cycles. note: these start at 0x80 (for pal entry 0)!
	_palette->lastColor = stream->readByte();
	_palette->flags = stream->readByte();
	_palette->speed = stream->readByte();
	_palette->frameCount = stream->readUint16();

	_palette->cycleCount = stream->readUint16();

	byte unk[11];
	stream->read(unk, 6);

	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		if (_vm->getVersion() < 4) {
			stream->read(unk, 3);
		} else {
			stream->read(unk, 11);
			//Common::hexdump(unk, 11);

			if (_vm->getVersion() >= 5) {
				stream->read(unk, 7);
				//Common::hexdump(unk, 7);
			}
		}
	}

	for (int i = 0; i < CHANNEL_COUNT; i++) {
		Sprite &sprite = *_sprites[i + 1];

		sprite._x1 = stream->readByte();
		sprite._enabled = (stream->readByte() != 0);
		sprite._x2 = stream->readUint16();

		sprite._flags = stream->readUint16();
		sprite._ink = static_cast<InkType>(sprite._flags & 0x3f);

		if (sprite._flags & 0x40)
			sprite._trails = 1;
		else
			sprite._trails = 0;

		sprite._castId = stream->readUint16();
		sprite._startPoint.y = stream->readUint16();
		sprite._startPoint.x = stream->readUint16();
		sprite._height = stream->readUint16();
		sprite._width = stream->readUint16();

		debugC(kDebugLoading, 8, "%03d(%d)[%x,%x,%04x,%d/%d/%d/%d]", sprite._castId, sprite._enabled, sprite._x1, sprite._x2, sprite._flags, sprite._startPoint.x, sprite._startPoint.y, sprite._width, sprite._height);

		if (_vm->getPlatform() == Common::kPlatformMacintosh && _vm->getVersion() >= 4) {
			sprite._scriptId = stream->readUint16();
			sprite._flags2 = stream->readByte(); // 0x40 editable, 0x80 moveable
			sprite._unk2 = stream->readByte();

			if (_vm->getVersion() >= 5)
				sprite._unk3 = stream->readUint32();
		}
	}
}

void Frame::readMainChannels(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size) {
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
				_transDuration = transFlags & 0x7f;
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
		case kPaletePosition:
			if (stream.readUint16())
				readPaletteInfo(stream);
			offset += 16;
			break;
		default:
			offset++;
			stream.readByte();
			debugC(kDebugLoading, "Frame::readMainChannels: Field Position %d, Finish Position %d", offset, finishPosition);
			break;
		}
	}

	warning("%d %d %d %d %d %d %d %d %d %d %d", _actionId, _soundType1, _transDuration, _transChunkSize, _tempo, _transType, _sound1, _skipFrameFlag, _blend, _sound2, _soundType2);
}

void Frame::readPaletteInfo(Common::SeekableSubReadStreamEndian &stream) {
	_palette->firstColor = stream.readByte();
	_palette->lastColor = stream.readByte();
	_palette->flags = stream.readByte();
	_palette->speed = stream.readByte();
	_palette->frameCount = stream.readUint16();
	stream.skip(8); //unknown
}

void Frame::readSprite(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size) {
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
			sprite._flags = stream.readUint16();
			sprite._ink = static_cast<InkType>(sprite._flags & 0x3f);

			if (sprite._flags & 0x40)
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
	warning("%03d(%d)[%x,%x,%04x,%d/%d/%d/%d]", sprite._castId, sprite._enabled, x1, x2, sprite._flags, sprite._startPoint.x, sprite._startPoint.y, sprite._width, sprite._height);

}

void Frame::prepareFrame(Score *score) {
	renderSprites(*score->_surface, false);
	renderSprites(*score->_trailSurface, true);

	if (_transType != 0)
		//T ODO Handle changing area case
		playTransition(score);

	if (_sound1 != 0 || _sound2 != 0) {
		playSoundChannel();
	}

	g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, score->_surface->getBounds().width(), score->_surface->getBounds().height());
}

void Frame::playSoundChannel() {
	debug(0, "Sound1 %d", _sound1);
	debug(0, "Sound2 %d", _sound2);
}

void Frame::playTransition(Score *score) {
	uint16 duration = _transDuration * 250; // _transDuration in 1/4 of sec
	duration = (duration == 0 ? 250 : duration); // director supports transition duration = 0, but animation play like value = 1, idk.

	if (_transChunkSize == 0)
		_transChunkSize = 1; // equal to 1 step

	uint16 stepDuration = duration / _transChunkSize;
	uint16 steps = duration / stepDuration;

	switch (_transType) {
	case kTransCoverDown:
		{
			uint16 stepSize = score->_movieRect.height() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverUp:
		{
			uint16 stepSize = score->_movieRect.height() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverRight: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverLeft: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverUpLeft: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, score->_movieRect.height() - stepSize * i, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverUpRight: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverDownLeft: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	case kTransCoverDownRight: {
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				score->processEvents();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height());
				g_system->updateScreen();
			}
		}
		break;
	default:
		warning("Unhandled transition type %d %d %d", _transType, duration, _transChunkSize);
		break;

	}
}

void Frame::renderSprites(Graphics::ManagedSurface &surface, bool renderTrail) {
	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		if (_sprites[i]->_enabled) {
			if ((_sprites[i]->_trails == 0 && renderTrail) || (_sprites[i]->_trails == 1 && !renderTrail))
				continue;

			Cast *cast;
			if (!_vm->_currentScore->_casts.contains(_sprites[i]->_castId)) {
				if (!_vm->getSharedCasts()->contains(_sprites[i]->_castId)) {
					warning("Cast id %d not found", _sprites[i]->_castId);
					continue;
				} else {
					warning("Getting cast id %d from shared cast", _sprites[i]->_castId);
					cast = _vm->getSharedCasts()->getVal(_sprites[i]->_castId);
				}
			} else {
				cast = _vm->_currentScore->_casts[_sprites[i]->_castId];
			}

			if (cast->type == kCastText) {
				renderText(surface, i);
				continue;
			}

			Image::ImageDecoder *img = getImageFrom(_sprites[i]->_castId);

			if (!img) {
				warning("Image with id %d not found", _sprites[i]->_castId);
				continue;
			}

			if (!img->getSurface()) {
				warning("Frame::renderSprites: Could not load image %d", _sprites[i]->_castId);
				continue;
			}

			assert(_sprites[i]->_cast);

			uint32 regX = static_cast<BitmapCast *>(_sprites[i]->_cast)->regX;
			uint32 regY = static_cast<BitmapCast *>(_sprites[i]->_cast)->regY;
			uint32 rectLeft = static_cast<BitmapCast *>(_sprites[i]->_cast)->initialRect.left;
			uint32 rectTop = static_cast<BitmapCast *>(_sprites[i]->_cast)->initialRect.top;

			int x = _sprites[i]->_startPoint.x - regX + rectLeft;
			int y = _sprites[i]->_startPoint.y - regY + rectTop;
			int height = _sprites[i]->_height;
			int width = _sprites[i]->_width;

			Common::Rect drawRect = Common::Rect(x, y, x + width, y + height);
			_drawRects.push_back(drawRect);

			switch (_sprites[i]->_ink) {
			case kInkTypeCopy:
				surface.blitFrom(*img->getSurface(), Common::Point(x, y));
				break;
			case kInkTypeTransparent:
				// FIXME: is it always white (last entry in pallette)?
				surface.transBlitFrom(*img->getSurface(), Common::Point(x, y), _vm->getPaletteColorCount() - 1);
				break;
			case kInkTypeBackgndTrans:
				drawBackgndTransSprite(surface, *img->getSurface(), drawRect);
				break;
			case kInkTypeMatte:
				drawMatteSprite(surface, *img->getSurface(), drawRect);
				break;
			case kInkTypeGhost:
				drawGhostSprite(surface, *img->getSurface(), drawRect);
				break;
			case kInkTypeReverse:
				drawReverseSprite(surface, *img->getSurface(), drawRect);
				break;
			default:
				warning("Unhandled ink type %d", _sprites[i]->_ink);
				surface.blitFrom(*img->getSurface(), Common::Point(x, y));
				break;
			}
		}
	}
}

void Frame::renderButton(Graphics::ManagedSurface &surface, uint16 spriteId) {
	renderText(surface, spriteId);

	uint16 castID = _sprites[spriteId]->_castId;
	ButtonCast *button = static_cast<ButtonCast *>(_vm->_currentScore->_casts[castID]);

	uint32 rectLeft = button->initialRect.left;
	uint32 rectTop = button->initialRect.top;

	int x = _sprites[spriteId]->_startPoint.x + rectLeft;
	int y = _sprites[spriteId]->_startPoint.y + rectTop;
	int height = _sprites[spriteId]->_height;
	int width = _sprites[spriteId]->_width;

	switch (button->buttonType) {
	case kTypeCheckBox:
		// Magic numbers: checkbox square need to move left about 5px from text and 12px side size (D4)
		surface.frameRect(Common::Rect(x - 17, y, x + 12, y + 12), 0);
		break;
	case kTypeButton:
		surface.frameRect(Common::Rect(x, y, x + width, y + height), 0);
		break;
	case kTypeRadio:
		warning("STUB: renderButton: kTypeRadio");
		break;
	}
}

Image::ImageDecoder *Frame::getImageFrom(uint16 spriteId) {
	uint16 imgId = spriteId + 1024;
	Image::ImageDecoder *img = NULL;

	if (_vm->_currentScore->getArchive()->hasResource(MKTAG('D', 'I', 'B', ' '), imgId)) {
		img = new DIBDecoder();
		img->loadStream(*_vm->_currentScore->getArchive()->getResource(MKTAG('D', 'I', 'B', ' '), imgId));
		return img;
	}

	if (_vm->getSharedDIB() != NULL && _vm->getSharedDIB()->contains(imgId)) {
		img = new DIBDecoder();
		img->loadStream(*_vm->getSharedDIB()->getVal(imgId));
		return img;
	}

	if (_vm->_currentScore->getArchive()->hasResource(MKTAG('B', 'I', 'T', 'D'), imgId)) {
		Common::SeekableReadStream *pic = _vm->_currentScore->getArchive()->getResource(MKTAG('B', 'I', 'T', 'D'), imgId);

		if (_vm->getVersion() <= 4) {
			BitmapCast *bc = static_cast<BitmapCast *>(_vm->_currentScore->_casts[spriteId]);
			int w = bc->initialRect.width(), h = bc->initialRect.height();

			debugC(2, kDebugImages, "id: %d, w: %d, h: %d, flags: %x, some: %x, unk1: %d, unk2: %d",
				imgId, w, h, bc->flags, bc->someFlaggyThing, bc->unk1, bc->unk2);
			img = new BITDDecoder(w, h);
		} else {
			img = new Image::BitmapDecoder();
		}

		if (debugChannelSet(8, kDebugLoading)) {
			Common::SeekableReadStream *s = pic;
			byte buf[1024];
			int n = s->read(buf, 1024);
			Common::hexdump(buf, n);
		}

		img->loadStream(*pic);
		return img;
	}

	if (_vm->getSharedBMP() != NULL && _vm->getSharedBMP()->contains(imgId)) {
		img = new Image::BitmapDecoder();
		img->loadStream(*_vm->getSharedBMP()->getVal(imgId));
		return img;
	}

	warning("Image %d not found", spriteId);
	return img;
}


void Frame::renderText(Graphics::ManagedSurface &surface, uint16 spriteID) {
	uint16 castID = _sprites[spriteID]->_castId;

	TextCast *textCast = static_cast<TextCast *>(_vm->_currentScore->_casts[castID]);
	Common::SeekableSubReadStreamEndian *textStream;

	if (_vm->_currentScore->_movieArchive->hasResource(MKTAG('S','T','X','T'), castID + 1024)) {
		textStream = _vm->_currentScore->_movieArchive->getResource(MKTAG('S','T','X','T'), castID + 1024);
	} else {
		textStream = _vm->getSharedSTXT()->getVal(spriteID + 1024);
	}
	uint32 unk1 = textStream->readUint32();
	uint32 strLen = textStream->readUint32();
	uint32 dataLen = textStream->readUint32();
	Common::String text;

	for (uint32 i = 0; i < strLen; i++) {
		byte ch = textStream->readByte();
		if (ch == 0x0d) {
			ch = '\n';
		}
		text += ch;
	}

	debugC(3, kDebugText, "renderText: unk1: %d strLen: %d dataLen: %d textlen: %u", unk1, strLen, dataLen, text.size());
	if (strLen < 200)
		debugC(3, kDebugText, "text: '%s'", text.c_str());

	uint32 rectLeft = static_cast<TextCast *>(_sprites[spriteID]->_cast)->initialRect.left;
	uint32 rectTop = static_cast<TextCast *>(_sprites[spriteID]->_cast)->initialRect.top;

	int x = _sprites[spriteID]->_startPoint.x + rectLeft;
	int y = _sprites[spriteID]->_startPoint.y + rectTop;
	int height = _sprites[spriteID]->_height;
	int width = _sprites[spriteID]->_width;

	Graphics::MacFont macFont(textCast->fontId, textCast->fontSize);

	if (_vm->_currentScore->_fontMap.contains(textCast->fontId)) {
		// Override
		macFont.setName(_vm->_currentScore->_fontMap[textCast->fontId]);
	}

	const Graphics::Font *font = _vm->_wm->_fontMan->getFont(macFont);

	debugC(3, kDebugText, "renderText: x: %d y: %d w: %d h: %d font: '%s'", x, y, width, height, _vm->_wm->_fontMan->getFontName(macFont));

	font->drawString(&surface, text, x, y, width, 0);

	if (textCast->borderSize != kSizeNone) {
		uint16 size = textCast->borderSize;

		// Indent from borders, measured in d4
		x -= 1;
		y -= 4;

		height += 4;
		width += 1;

		while (size) {
			surface.frameRect(Common::Rect(x, y, x + height, y + width), 0);
			x--;
			y--;
			height += 2;
			width += 2;
			size--;
		}
	}

	if (textCast->gutterSize != kSizeNone) {
		x -= 1;
		y -= 4;

		height += 4;
		width += 1;
		uint16 size = textCast->gutterSize;

		surface.frameRect(Common::Rect(x, y, x + height, y + width), 0);

		while (size) {
			surface.drawLine(x + width, y, x + width, y + height, 0);
			surface.drawLine(x, y + height, x + width, y + height, 0);
			x++;
			y++;
			size--;
		}
	}
}

void Frame::drawBackgndTransSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	uint8 skipColor = _vm->getPaletteColorCount() - 1; // FIXME is it always white (last entry in pallette) ?

	for (int ii = 0; ii < sprite.h; ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(0, ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if (*src != skipColor)
				*dst = *src;

			src++;
			dst++;
		}
	}
}

void Frame::drawGhostSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < sprite.h; ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(0, ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0) && (*src != skipColor))
				*dst = (_vm->getPaletteColorCount() - 1) - *src; // Oposite color

			src++;
			dst++;
		}
	}
}

void Frame::drawReverseSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < sprite.h; ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(0, ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < drawRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0))
				*dst = (_vm->getPaletteColorCount() - 1) - *src;
			else if (*src != skipColor)
				*dst = *src;
			src++;
			dst++;
		}
	}
}

void Frame::drawMatteSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels are transparent
	Graphics::Surface tmp;
	tmp.copyFrom(sprite);

	// Searching white color in the corners
	int whiteColor = -1;

	for (int corner = 0; corner < 4; corner++) {
		int x = (corner & 0x1) ? tmp.w - 1 : 0;
		int y = (corner & 0x2) ? tmp.h - 1 : 0;

		byte color = *(byte *)tmp.getBasePtr(x, y);

		if (_vm->getPalette()[color * 3 + 0] == 0xff &&
			_vm->getPalette()[color * 3 + 1] == 0xff &&
			_vm->getPalette()[color * 3 + 2] == 0xff) {
			whiteColor = color;
			break;
		}
	}

	if (whiteColor == -1) {
		debugC(1, kDebugImages, "No white color for Matte image");

		for (int yy = 0; yy < tmp.h; yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(0, yy);
			byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < drawRect.width(); xx++, src++, dst++)
				*dst = *src;
		}
	} else {
		Graphics::FloodFill ff(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			ff.addSeed(0, yy);
			ff.addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			ff.addSeed(xx, 0);
			ff.addSeed(xx, tmp.h - 1);
		}
		ff.fillMask();

		for (int yy = 0; yy < tmp.h; yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(0, yy);
			const byte *mask = (const byte *)ff.getMask()->getBasePtr(0, yy);
			byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < drawRect.width(); xx++, src++, dst++, mask++)
				if (*mask == 0)
					*dst = *src;
		}
	}

	tmp.free();
}

uint16 Frame::getSpriteIDFromPos(Common::Point pos) {
	// Find first from top to bottom
	for (uint16 i = _drawRects.size() - 1; i > 0; i--) {
		if (_drawRects[i].contains(pos))
			return i;
	}

	return 0;
}

} // End of namespace Director
