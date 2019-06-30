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
#include "graphics/macgui/mactext.h"
#include "graphics/primitives.h"
#include "image/bmp.h"

#include "director/director.h"
#include "director/cachedmactext.h"
#include "director/cast.h"
#include "director/frame.h"
#include "director/images.h"
#include "director/archive.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

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
	byte unk[16];

	if (_vm->getVersion() < 4) {
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
	} else if (_vm->getVersion() < 5) {
		stream->read(unk, 16);
		_actionId = stream->readUint16();
		stream->read(unk, 5);
	} else {
		stream->read(unk, 16);
		stream->read(unk, 16);
		stream->read(unk, 10);
	}


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

		if (_vm->getVersion() <= 4) {
			sprite._scriptId = stream->readByte();
			sprite._spriteType = stream->readByte();
			sprite._enabled = sprite._spriteType != 0;
			sprite._x2 = stream->readUint16();

			sprite._flags = stream->readUint16();
			sprite._ink = static_cast<InkType>(sprite._flags & 0x3f);

			if (sprite._flags & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			sprite._lineSize = (sprite._flags >> 8) & 0x03;

			sprite._castId = stream->readUint16();
			sprite._startPoint.y = stream->readUint16();
			sprite._startPoint.x = stream->readUint16();
			sprite._height = stream->readUint16();
			sprite._width = stream->readUint16();

			if (_vm->getPlatform() == Common::kPlatformMacintosh && _vm->getVersion() >= 4) {
				sprite._scriptId = stream->readUint16();
				sprite._flags2 = stream->readByte(); // 0x40 editable, 0x80 moveable
				sprite._unk2 = stream->readByte();

				if (_vm->getVersion() >= 5)
					sprite._unk3 = stream->readUint32();
			}
		} else {
			stream->readUint16();
			sprite._scriptId = stream->readByte();
			sprite._spriteType = stream->readByte();
			sprite._enabled = sprite._spriteType != 0;
			sprite._castId = stream->readUint16();
			stream->readUint32();
			sprite._flags = stream->readUint16();
			sprite._startPoint.y = stream->readUint16();
			sprite._startPoint.x = stream->readUint16();
			sprite._height = stream->readUint16();
			sprite._width = stream->readUint16();
			stream->readUint16();
			stream->readUint16();

		}

		if (sprite._castId) {
			debugC(kDebugLoading, 4, "CH: %-3d castId: %03d(%s) (e:%d) [%x,%x, flags:%04x, %dx%d@%d,%d linesize: %d] script: %d",
				i + 1, sprite._castId, numToCastNum(sprite._castId), sprite._enabled, sprite._x1, sprite._x2, sprite._flags,
				sprite._width, sprite._height, sprite._startPoint.x, sprite._startPoint.y,
				sprite._lineSize, sprite._scriptId);
		} else {
			debugC(kDebugLoading, 4, "CH: %-3d castId: 000", i + 1);
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
	stream.skip(8); // unknown
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
	_drawRects.clear();
	renderSprites(*score->_surface, false);
	renderSprites(*score->_trailSurface, true);

	if (_transType != 0)
		// TODO Handle changing area case
		playTransition(score);

	if (_sound1 != 0 || _sound2 != 0) {
		playSoundChannel();
	}

	g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, score->_surface->getBounds().width(), score->_surface->getBounds().height());
}

void Frame::playSoundChannel() {
	debug(0, "STUB: playSoundChannel(), Sound1 %d Sound2 %d", _sound1, _sound2);
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
				processQuitEvent();

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
				processQuitEvent();

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
				processQuitEvent();

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
				processQuitEvent();

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
				processQuitEvent();

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
				processQuitEvent();

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
				processQuitEvent();

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
				processQuitEvent();

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

			CastType castType = kCastTypeNull;
			if (_vm->getVersion() < 4) {
				debugC(1, kDebugImages, "Channel: %d type: %d", i, _sprites[i]->_spriteType);
				switch (_sprites[i]->_spriteType) {
				case 1:
					castType = kCastBitmap;
					break;
				case 2:
				case 12: // this is actually a mouse-over shape? I don't think it's a real button.
				case 16: // Face kit D3
					castType = kCastShape;
					break;
				case 7:
					castType = kCastText;
					break;
				}
			} else {
				if (!_vm->getCurrentScore()->_castTypes.contains(_sprites[i]->_castId)) {
					if (!_vm->getSharedCastTypes()->contains(_sprites[i]->_castId)) {
						warning("Cast id %d not found", _sprites[i]->_castId);
						continue;
					} else {
						warning("Getting cast id %d from shared cast", _sprites[i]->_castId);
						castType = _vm->getSharedCastTypes()->getVal(_sprites[i]->_castId);
					}
				} else {
					castType = _vm->getCurrentScore()->_castTypes[_sprites[i]->_castId];
				}
			}

			// this needs precedence to be hit first... D3 does something really tricky with cast IDs for shapes.
			// I don't like this implementation 100% as the 'cast' above might not actually hit a member and be null?
			if (castType == kCastShape) {
				renderShape(surface, i);
			} else if (castType == kCastText || castType == kCastRTE) {
				renderText(surface, i, NULL);
			} else if (castType == kCastButton) {
				renderButton(surface, i);
			} else {
				if (!_sprites[i]->_bitmapCast) {
					warning("No cast ID for sprite %d", i);
					continue;
				}

				uint32 regX = _sprites[i]->_bitmapCast->regX;
				uint32 regY = _sprites[i]->_bitmapCast->regY;
				uint32 rectLeft = _sprites[i]->_bitmapCast->initialRect.left;
				uint32 rectTop = _sprites[i]->_bitmapCast->initialRect.top;

				int x = _sprites[i]->_startPoint.x - regX + rectLeft;
				int y = _sprites[i]->_startPoint.y - regY + rectTop;
				int height = _sprites[i]->_height;
				int width = _vm->getVersion() > 4 ? _sprites[i]->_bitmapCast->initialRect.width() : _sprites[i]->_width;

				Common::Rect drawRect(x, y, x + width, y + height);
				addDrawRect(i, drawRect);
				inkBasedBlit(surface, *(_sprites[i]->_bitmapCast->surface), i, drawRect);
			}
		}
	}
}

void Frame::addDrawRect(uint16 spriteId, Common::Rect &rect) {
	FrameEntity *fi = new FrameEntity();
	fi->spriteId = spriteId;
	fi->rect = rect;
	_drawRects.push_back(fi);
}

void Frame::renderShape(Graphics::ManagedSurface &surface, uint16 spriteId) {
	Common::Rect shapeRect = Common::Rect(_sprites[spriteId]->_startPoint.x,
		_sprites[spriteId]->_startPoint.y,
		_sprites[spriteId]->_startPoint.x + _sprites[spriteId]->_width,
		_sprites[spriteId]->_startPoint.y + _sprites[spriteId]->_height);

	Graphics::ManagedSurface tmpSurface;
	tmpSurface.create(shapeRect.width(), shapeRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	if (_vm->getVersion() <= 3 && _sprites[spriteId]->_spriteType == 0x0c) {
		tmpSurface.fillRect(Common::Rect(shapeRect.width(), shapeRect.height()), (_vm->getCurrentScore()->_currentMouseDownSpriteId == spriteId ? 0 : 0xff));
		//tmpSurface.frameRect(Common::Rect(shapeRect.width(), shapeRect.height()), 0);
		// TODO: don't override, work out how to display correctly.
		_sprites[spriteId]->_ink = kInkTypeReverse;
	} else {
		// No minus one on the pattern here! MacPlotData will do that for us!
		Graphics::MacPlotData pd(&tmpSurface, &_vm->getPatterns(), _sprites[spriteId]->_castId, 1, _sprites[spriteId]->_backColor);
		Common::Rect fillRect(shapeRect.width(), shapeRect.height());
		Graphics::drawFilledRect(fillRect, _sprites[spriteId]->_foreColor, Graphics::macDrawPixel, &pd);
	}

	if (_sprites[spriteId]->_lineSize > 0) {
		for (int rr = 0; rr < (_sprites[spriteId]->_lineSize - 1); rr++)
			tmpSurface.frameRect(Common::Rect(rr, rr, shapeRect.width() - (rr * 2), shapeRect.height() - (rr * 2)), 0);
	}

	addDrawRect(spriteId, shapeRect);
	inkBasedBlit(surface, tmpSurface, spriteId, shapeRect);
}

void Frame::renderButton(Graphics::ManagedSurface &surface, uint16 spriteId) {
	uint16 castId = _sprites[spriteId]->_castId;
	ButtonCast *button = _vm->getCurrentScore()->_loadedButtons->getVal(castId);

	uint32 rectLeft = button->initialRect.left;
	uint32 rectTop = button->initialRect.top;

	int x = _sprites[spriteId]->_startPoint.x + rectLeft;
	int y = _sprites[spriteId]->_startPoint.y + rectTop;
	int height = button->initialRect.height();
	int width = button->initialRect.width() + 3;

	Common::Rect textRect(0, 0, width, height);
	// pass the rect of the button into the label.
	renderText(surface, spriteId, &textRect);

	// TODO: review all cases to confirm if we should use text height.
	// height = textRect.height();

	Common::Rect _rect;

	switch (button->buttonType) {
	case kTypeCheckBox:
		// Magic numbers: checkbox square need to move left about 5px from text and 12px side size (D4)
		_rect = Common::Rect(x - 17, y, x + 12, y + 12);
		surface.frameRect(_rect, 0);
		addDrawRect(spriteId, _rect);
		break;
	case kTypeButton: {
			_rect = Common::Rect(x, y, x + width, y + height + 3);
			Graphics::MacPlotData pd(&surface, &_vm->getMacWindowManager()->getPatterns(), Graphics::MacGUIConstants::kPatternSolid, 1, Graphics::kColorWhite);
			Graphics::drawRoundRect(_rect, 4, 0, false, Graphics::macDrawPixel, &pd);
			addDrawRect(spriteId, _rect);
		}
		break;
	case kTypeRadio:
		warning("STUB: renderButton: kTypeRadio");
		break;
	}
}

void Frame::inkBasedBlit(Graphics::ManagedSurface &targetSurface, const Graphics::Surface &spriteSurface, uint16 spriteId, Common::Rect drawRect) {
	switch (_sprites[spriteId]->_ink) {
	case kInkTypeCopy:
		targetSurface.blitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top));
		break;
	case kInkTypeTransparent:
		// FIXME: is it always white (last entry in pallette)?
		targetSurface.transBlitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top), _vm->getPaletteColorCount() - 1);
		break;
	case kInkTypeBackgndTrans:
		drawBackgndTransSprite(targetSurface, spriteSurface, drawRect);
		break;
	case kInkTypeMatte:
		drawMatteSprite(targetSurface, spriteSurface, drawRect);
		break;
	case kInkTypeGhost:
		drawGhostSprite(targetSurface, spriteSurface, drawRect);
		break;
	case kInkTypeReverse:
		drawReverseSprite(targetSurface, spriteSurface, drawRect);
		break;
	default:
		warning("Unhandled ink type %d", _sprites[spriteId]->_ink);
		targetSurface.blitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top));
		break;
	}
}


void Frame::renderText(Graphics::ManagedSurface &surface, uint16 spriteId, Common::Rect *textSize) {
	TextCast *textCast = _sprites[spriteId]->_buttonCast != nullptr ? (TextCast*)_sprites[spriteId]->_buttonCast : _sprites[spriteId]->_textCast;


	int x = _sprites[spriteId]->_startPoint.x; // +rectLeft;
	int y = _sprites[spriteId]->_startPoint.y; // +rectTop;
	int height = textCast->initialRect.height(); //_sprites[spriteId]->_height;
	int width;

	if (_vm->getVersion() >= 4) {
		if (textSize == NULL)
			width = textCast->initialRect.right;
		else {
			width = textSize->width();
		}
	} else {
		width = textCast->initialRect.width(); //_sprites[spriteId]->_width;
	}

	if (_vm->getCurrentScore()->_fontMap.contains(textCast->fontId)) {
		// We need to make sure that the Shared Cast fonts have been loaded in?
		// might need a mapping table here of our own.
		// textCast->fontId = _vm->_wm->_fontMan->getFontIdByName(_vm->getCurrentScore()->_fontMap[textCast->fontId]);
	}

	if (width == 0 || height == 0) {
		warning("renderText: Requested to draw on an empty surface: %d x %d", width, height);
		return;
	}

	Graphics::MacFont *macFont = new Graphics::MacFont(textCast->fontId, textCast->fontSize, textCast->textSlant);

	debugC(3, kDebugText, "renderText: x: %d y: %d w: %d h: %d font: '%s'", x, y, width, height, _vm->_wm->_fontMan->getFontName(*macFont).c_str());

	uint16 boxShadow = (uint16)textCast->boxShadow;
	uint16 borderSize = (uint16)textCast->borderSize;
	if (textSize != NULL)
		borderSize = 0;
	uint16 padding = (uint16)textCast->gutterSize;
	uint16 textShadow = (uint16)textCast->textShadow;

	//uint32 rectLeft = textCast->initialRect.left;
	//uint32 rectTop = textCast->initialRect.top;

	textCast->cachedMacText->clip(width);
	textCast->cachedMacText->setWm(_vm->_wm); // TODO this is not a good place to do it
	const Graphics::ManagedSurface *textSurface = textCast->cachedMacText->getSurface();

	height = textSurface->h;
	if (textSize != NULL) {
		// TODO: this offset could be due to incorrect fonts loaded!
		textSize->bottom = height + textCast->cachedMacText->getLineCount();
	}

	uint16 textX = 0, textY = 0;

	if (textSize == NULL) {
		if (borderSize > 0) {
			if (_vm->getVersion() <= 3)
				height++;
			else
				height += borderSize;

			textX += (borderSize + 1);
			textY += borderSize;
		} else {
			x += 1;
		}

		if (padding > 0) {
			width += padding * 2;
			height += padding;
			textY += padding / 2;
		}

		if (textCast->textAlign == kTextAlignRight)
			textX -= 1;

		if (textShadow > 0)
			textX--;
	} else {
		x++;
		if (width % 2 != 0)
			x++;
		y += 2;
	}

	switch (textCast->textAlign) {
	case kTextAlignLeft:
		break;
	case kTextAlignCenter:
		textX = (width / 2) - (textSurface->w / 2) + (padding / 2) + borderSize;
		break;
	case kTextAlignRight:
		textX = width - (textSurface->w + 1) + (borderSize * 2) - (textShadow * 2) - (padding);
		break;
	}

	Graphics::ManagedSurface textWithFeatures(width + (borderSize * 2) + boxShadow + textShadow, height + borderSize + boxShadow + textShadow);
	textWithFeatures.fillRect(Common::Rect(textWithFeatures.w, textWithFeatures.h), 0xff);

	if (textSize == NULL && boxShadow > 0) {
		textWithFeatures.fillRect(Common::Rect(boxShadow, boxShadow, textWithFeatures.w + boxShadow, textWithFeatures.h), 0);
	}

	if (textSize == NULL && borderSize != kSizeNone) {
		for (int bb = 0; bb < borderSize; bb++) {
			Common::Rect borderRect(bb, bb, textWithFeatures.w - bb - boxShadow - textShadow, textWithFeatures.h - bb - boxShadow - textShadow);
			textWithFeatures.fillRect(borderRect, 0xff);
			textWithFeatures.frameRect(borderRect, 0);
		}
	}

	if (textShadow > 0)
		textWithFeatures.transBlitFrom(textSurface->rawSurface(), Common::Point(textX + textShadow, textY + textShadow), 0xff);

	textWithFeatures.transBlitFrom(textSurface->rawSurface(), Common::Point(textX, textY), 0xff);

	inkBasedBlit(surface, textWithFeatures, spriteId, Common::Rect(x, y, x + width, y + height));
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
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0)) {
				if (*src != skipColor) {
					*dst = (*dst == *src ? (*src == 0 ? 0xff : 0) : *src);
				}
			} else if (*src != skipColor) {
				*dst = *src;
			}
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
	// Find first from front to back
	for (int dr = _drawRects.size() - 1; dr >= 0; dr--)
		if (_drawRects[dr]->rect.contains(pos))
			return _drawRects[dr]->spriteId;

	return 0;
}

bool Frame::checkSpriteIntersection(uint16 spriteId, Common::Point pos) {
	// Find first from front to back
	for (int dr = _drawRects.size() - 1; dr >= 0; dr--)
		if (_drawRects[dr]->spriteId == spriteId && _drawRects[dr]->rect.contains(pos))
			return true;

	return false;
}

} // End of namespace Director
