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
#include "common/substream.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/primitives.h"

#include "director/director.h"
#include "director/cachedmactext.h"
#include "director/cast.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/util.h"

namespace Director {

Frame::Frame(DirectorEngine *vm, int numChannels) {
	_vm = vm;
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

	_palette = NULL;

	_sprites.resize(_numChannels + 1);

	for (uint16 i = 0; i < _sprites.size(); i++) {
		Sprite *sp = new Sprite();
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
	_palette = new PaletteInfo();

	debugC(1, kDebugLoading, "Frame. action: %d transType: %d transDuration: %d", _actionId, _transType, _transDuration);

	_sprites.resize(_numChannels + 1);

	for (uint16 i = 0; i <= _numChannels; i++) {
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
			if (unk[0] != 0 || unk[1] != 0 || unk[2] != 0)
				warning("Frame::readChannels(): unk1: %x unk2: %x unk3: %x", unk[0], unk[1], unk[2]);
		}
		_skipFrameFlag = stream->readByte();
		_blend = stream->readByte();

		if (_vm->getPlatform() != Common::kPlatformMacintosh) {
			_sound2 = stream->readUint16();
			_soundType2 = stream->readByte();
		}

		uint16 palette = stream->readUint16();

		if (palette) {
			warning("Frame::readChannels(): STUB: Palette info");
		}

		debugC(8, kDebugLoading, "Frame::readChannels(): %d %d %d %d %d %d %d %d %d %d %d", _actionId, _soundType1, _transDuration, _transChunkSize, _tempo, _transType, _sound1, _skipFrameFlag, _blend, _sound2, _soundType2);

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

	for (int i = 0; i < _numChannels; i++) {
		Sprite &sprite = *_sprites[i + 1];

		if (_vm->getVersion() <= 4) {
			sprite._scriptId = stream->readByte();
			sprite._spriteType = stream->readByte();
			sprite._enabled = sprite._spriteType != 0;
			if (_vm->getVersion() >= 4) {
				sprite._foreColor = 0xff - (uint8)stream->readByte();
				sprite._backColor = 0xff - (uint8)stream->readByte();
			} else {
				sprite._foreColor = (127 - stream->readByte()) & 0xff; // -128 -> 0, 127 -> 256
				sprite._backColor = (127 - stream->readByte()) & 0xff;
			}

			sprite._flags = stream->readUint16();
			sprite._ink = static_cast<InkType>(sprite._flags & 0x3f);

			if (sprite._flags & 0x40)
				sprite._trails = 1;
			else
				sprite._trails = 0;

			sprite._lineSize = ((sprite._flags >> 8) & 0x07);

			sprite._castId = stream->readUint16();
			sprite._startPoint.y = stream->readUint16();
			sprite._startPoint.x = stream->readUint16();
			sprite._height = stream->readUint16();
			sprite._width = stream->readUint16();

			if (_vm->getPlatform() == Common::kPlatformMacintosh && _vm->getVersion() >= 4) {
				sprite._scriptId = stream->readUint16();
				sprite._flags2 = stream->readByte(); // 0x40 editable, 0x80 moveable
				sprite._unk2 = stream->readByte();
				sprite._moveable = ((sprite._flags2 & 0x80) == 0x80);

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
			debugC(4, kDebugLoading, "CH: %-3d castId: %03d(%s) [flags:%04x [ink: %x trails: %d line: %d], %dx%d@%d,%d type: %d fg: %d bg: %d] script: %d, flags2: %x, unk2: %x, unk3: %x",
				i + 1, sprite._castId, numToCastNum(sprite._castId), sprite._flags,
				sprite._ink, sprite._trails, sprite._lineSize, sprite._width, sprite._height,
				sprite._startPoint.x, sprite._startPoint.y,
				sprite._spriteType, sprite._foreColor, sprite._backColor, sprite._scriptId, sprite._flags2, sprite._unk2, sprite._unk3);
		} else {
			debugC(4, kDebugLoading, "CH: %-3d castId: 000", i + 1);
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
	warning("Frame::readSprite(): %03d(%d)[%x,%x,%04x,%d/%d/%d/%d]", sprite._castId, sprite._enabled, x1, x2, sprite._flags, sprite._startPoint.x, sprite._startPoint.y, sprite._width, sprite._height);

}

void Frame::prepareFrame(Score *score) {
	_drawRects.clear();
	renderSprites(*score->_surface, false);
	renderSprites(*score->_trailSurface, true);

	score->renderZoomBox();

	_vm->_wm->draw();

	if (_transType != 0)
		// TODO Handle changing area case
		playTransition(score);

	if (_sound1 != 0 || _sound2 != 0) {
		playSoundChannel();
	}

	if (_vm->getCurrentScore()->haveZoomBox())
		score->_backSurface->copyFrom(*score->_surface);

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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height()); // transition
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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height()); // transition
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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, score->_movieRect.height() - stepSize * i, r.width(), r.height()); // transition
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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height()); // transition
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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height()); // transition
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

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;
	default:
		warning("Frame::playTransition(): Unhandled transition type %d %d %d", _transType, duration, _transChunkSize);
		break;

	}
}

void Frame::renderSprites(Graphics::ManagedSurface &surface, bool renderTrail) {
	for (uint16 i = 0; i <= _numChannels; i++) {
		if (!_sprites[i]->_enabled)
			continue;

		if ((_sprites[i]->_trails == 0 && renderTrail) || (_sprites[i]->_trails == 1 && !renderTrail))
			continue;

		CastType castType = kCastTypeNull;
		if (_vm->getVersion() < 4) {
			debugC(1, kDebugImages, "Frame::renderSprites(): Channel: %d type: %d", i, _sprites[i]->_spriteType);
			switch (_sprites[i]->_spriteType) {
			case kBitmapSprite:
				castType = kCastBitmap;
				break;
			case kRectangleSprite:
			case kRoundedRectangleSprite:
			case kOvalSprite:
			case kLineTopBottomSprite:
			case kLineBottomTopSprite:
			case kOutlinedRectangleSprite:	// this is actually a mouse-over shape? I don't think it's a real button.
			case kOutlinedRoundedRectangleSprite:
			case kOutlinedOvalSprite:
			case kCastMemberSprite: 		// Face kit D3
				castType = kCastShape;
				break;
			case kTextSprite:
				castType = kCastText;
				break;
			default:
				warning("Frame::renderSprites(): Unhandled sprite type %d", _sprites[i]->_spriteType);
				break;
			}
		} else {
			if (!_vm->getCurrentScore()->_loadedCast->contains(_sprites[i]->_castId)) {
				if (!_vm->getSharedScore() || !_vm->getSharedScore()->_loadedCast->contains(_sprites[i]->_castId)) {
					debugC(1, kDebugImages, "Frame::renderSprites(): Cast id %d not found", _sprites[i]->_castId);
					continue;
				} else {
					debugC(1, kDebugImages, "Frame::renderSprites(): Getting cast id %d from shared cast", _sprites[i]->_castId);
					castType = _vm->getSharedScore()->_loadedCast->getVal(_sprites[i]->_castId)->_type;
				}
			} else {
				castType = _vm->getCurrentScore()->_loadedCast->getVal(_sprites[i]->_castId)->_type;
			}
		}

		// this needs precedence to be hit first... D3 does something really tricky with cast IDs for shapes.
		// I don't like this implementation 100% as the 'cast' above might not actually hit a member and be null?
		debugC(1, kDebugImages, "Frame::renderSprites(): Channel: %d castType: %d", i, castType);

		if (castType == kCastShape) {
			renderShape(surface, i);
		} else if (castType == kCastText || castType == kCastRTE) {
			renderText(surface, i, NULL);
		} else if (castType == kCastButton) {
			renderButton(surface, i);
		} else {
			if (!_sprites[i]->_cast || _sprites[i]->_cast->_type != kCastBitmap) {
				warning("Frame::renderSprites(): No cast ID for sprite %d", i);
				continue;
			}
			if (_sprites[i]->_cast->_surface == nullptr) {
				warning("Frame::renderSprites(): No cast surface for sprite %d", i);
				continue;
			}
			InkType ink;
			if (i == _vm->getCurrentScore()->_currentMouseDownSpriteId)
				ink = kInkTypeReverse;
			else
				ink = _sprites[i]->_ink;

			BitmapCast *bc = (BitmapCast *)_sprites[i]->_cast;

			int32 regX = bc->_regX;
			int32 regY = bc->_regY;
			int32 rectLeft = bc->_initialRect.left;
			int32 rectTop = bc->_initialRect.top;

			int x = _sprites[i]->_startPoint.x - regX + rectLeft;
			int y = _sprites[i]->_startPoint.y - regY + rectTop;
			int height = _sprites[i]->_height;
			int width = _vm->getVersion() > 4 ? bc->_initialRect.width() : _sprites[i]->_width;
			Common::Rect drawRect(x, y, x + width, y + height);
			addDrawRect(i, drawRect);
			inkBasedBlit(surface, *(bc->_surface), ink, drawRect);
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
	Sprite *sp = _sprites[spriteId];

	InkType ink = sp->_ink;
	byte spriteType = sp->_spriteType;
	byte foreColor = sp->_foreColor;
	byte backColor = sp->_backColor;
	int lineSize = sp->_lineSize;
	if (spriteType == kCastMemberSprite && sp->_cast != NULL) {
		switch (sp->_cast->_type) {
		case kCastShape:
			{
				ShapeCast *sc = (ShapeCast *)sp->_cast;
				switch (sc->_shapeType) {
				case kShapeRectangle:
					spriteType = sc->_fillType ? kRectangleSprite : kOutlinedRectangleSprite;
					break;
				case kShapeRoundRect:
					spriteType = sc->_fillType ? kRoundedRectangleSprite : kOutlinedRoundedRectangleSprite;
					break;
				case kShapeOval:
					spriteType = sc->_fillType ? kOvalSprite : kOutlinedOvalSprite;
					break;
				case kShapeLine:
					spriteType = sc->_lineDirection == 6 ? kLineBottomTopSprite : kLineTopBottomSprite;
					break;
				default:
					break;
				}
				foreColor = sc->_fgCol;
				backColor = sc->_bgCol;
				lineSize = sc->_lineThickness;
				ink = sc->_ink;
				// shapes should be rendered with transparency by default
				if (ink == kInkTypeCopy) {
					ink = kInkTypeTransparent;
				}
			}
			break;
		default:
			warning("Frame::renderShape(): Unhandled cast type: %d", sp->_cast->_type);
			break;
		}
	}

	// for outlined shapes, line thickness of 1 means invisible.
	lineSize -= 1;

	Common::Rect shapeRect = Common::Rect(sp->_startPoint.x,
		sp->_startPoint.y,
		sp->_startPoint.x + sp->_width,
		sp->_startPoint.y + sp->_height);

	Graphics::ManagedSurface tmpSurface;
	tmpSurface.create(shapeRect.width(), shapeRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	tmpSurface.clear(255);


	// Draw fill
	Common::Rect fillRect((int)shapeRect.width(), (int)shapeRect.height());
	Graphics::MacPlotData plotFill(&tmpSurface, &_vm->getPatterns(), sp->getPattern(), -shapeRect.left, -shapeRect.top, 1, backColor);
	switch (spriteType) {
	case kRectangleSprite:
		Graphics::drawFilledRect(fillRect, foreColor, Graphics::macDrawPixel, &plotFill);
		break;
	case kRoundedRectangleSprite:
		Graphics::drawRoundRect(fillRect, 12, foreColor, true, Graphics::macDrawPixel, &plotFill);
		break;
	case kOvalSprite:
		Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, foreColor, true, Graphics::macDrawPixel, &plotFill);
		break;
	case kCastMemberSprite: 		// Face kit D3
		Graphics::drawFilledRect(fillRect, foreColor, Graphics::macDrawPixel, &plotFill);
		break;
	default:
		break;
	}

	// Draw stroke
	Common::Rect strokeRect(MAX((int)shapeRect.width() - lineSize, 0), MAX((int)shapeRect.height() - lineSize, 0));
	Graphics::MacPlotData plotStroke(&tmpSurface, &_vm->getPatterns(), 1, -shapeRect.left, -shapeRect.top, lineSize, backColor);
	switch (spriteType) {
	case kLineTopBottomSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, foreColor, Graphics::macDrawPixel, &plotStroke);
		break;
	case kLineBottomTopSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.bottom, strokeRect.right, strokeRect.top, foreColor, Graphics::macDrawPixel, &plotStroke);
		break;
	case kRectangleSprite:
		// fall through
	case kOutlinedRectangleSprite:	// this is actually a mouse-over shape? I don't think it's a real button.
		Graphics::drawRect(strokeRect, foreColor, Graphics::macDrawPixel, &plotStroke);
		//tmpSurface.fillRect(Common::Rect(shapeRect.width(), shapeRect.height()), (_vm->getCurrentScore()->_currentMouseDownSpriteId == spriteId ? 0 : 0xff));
		break;
	case kRoundedRectangleSprite:
		// fall through
	case kOutlinedRoundedRectangleSprite:
		Graphics::drawRoundRect(strokeRect, 12, foreColor, false, Graphics::macDrawPixel, &plotStroke);
		break;
	case kOvalSprite:
		// fall through
	case kOutlinedOvalSprite:
		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, foreColor, false, Graphics::macDrawPixel, &plotStroke);
		break;
	default:
		break;
	}

	addDrawRect(spriteId, shapeRect);
	inkBasedBlit(surface, tmpSurface, ink, shapeRect);

}

void Frame::renderButton(Graphics::ManagedSurface &surface, uint16 spriteId) {
	uint16 castId = _sprites[spriteId]->_castId;
	ButtonCast *button = (ButtonCast *)_vm->getCurrentScore()->_loadedCast->getVal(castId);

	uint32 rectLeft = button->_initialRect.left;
	uint32 rectTop = button->_initialRect.top;

	int x = _sprites[spriteId]->_startPoint.x + rectLeft;
	int y = _sprites[spriteId]->_startPoint.y + rectTop;
	int height = button->_initialRect.height();
	int width = button->_initialRect.width() + 3;

	bool invert = spriteId == _vm->getCurrentScore()->_currentMouseDownSpriteId;

	// TODO: review all cases to confirm if we should use text height.
	// height = textRect.height();

	Common::Rect _rect;

	Common::Rect textRect(0, 0, width, height);

	// WORKAROUND, HACK
	// Because we're not drawing text with transparency
	// We swap drawing depending on whether the button is
	// inverted or not, to prevent destroying the border
	if (!invert)
		renderText(surface, spriteId, &textRect);

	switch (button->_buttonType) {
	case kTypeCheckBox:
		// Magic numbers: checkbox square need to move left about 5px from text and 12px side size (D4)
		_rect = Common::Rect(x - 17, y, x + 12, y + 12);
		surface.frameRect(_rect, 0);
		addDrawRect(spriteId, _rect);
		break;
	case kTypeButton: {
			_rect = Common::Rect(x, y, x + width, y + height + 3);
			Graphics::MacPlotData pd(&surface, &_vm->getMacWindowManager()->getPatterns(), Graphics::MacGUIConstants::kPatternSolid, 0, 0, 1, invert ? Graphics::kColorBlack : Graphics::kColorWhite);

			Graphics::drawRoundRect(_rect, 4, 0, invert, Graphics::macDrawPixel, &pd);
			addDrawRect(spriteId, _rect);
		}
		break;
	case kTypeRadio:
		warning("STUB: renderButton: kTypeRadio");
		break;
	default:
		warning("renderButton: Unknown buttonType");
		break;
	}

	if (invert)
		renderText(surface, spriteId, &textRect);
}

void Frame::renderText(Graphics::ManagedSurface &surface, uint16 spriteId, Common::Rect *textRect) {
	TextCast *textCast = (TextCast*)_sprites[spriteId]->_cast;

	int x = _sprites[spriteId]->_startPoint.x; // +rectLeft;
	int y = _sprites[spriteId]->_startPoint.y; // +rectTop;
	int height = textCast->_initialRect.height(); //_sprites[spriteId]->_height;
	int width;

	if (_vm->getVersion() >= 4) {
		if (textRect == NULL)
			width = textCast->_initialRect.right;
		else {
			width = textRect->width();
		}
	} else {
		width = textCast->_initialRect.width(); //_sprites[spriteId]->_width;
	}

	if (_vm->getCurrentScore()->_fontMap.contains(textCast->_fontId)) {
		// We need to make sure that the Shared Cast fonts have been loaded in?
		// might need a mapping table here of our own.
		// textCast->fontId = _vm->_wm->_fontMan->getFontIdByName(_vm->getCurrentScore()->_fontMap[textCast->fontId]);
	}

	if (width == 0 || height == 0) {
		warning("Frame::renderText(): Requested to draw on an empty surface: %d x %d", width, height);
		return;
	}

	Graphics::MacFont *macFont = new Graphics::MacFont(textCast->_fontId, textCast->_fontSize, textCast->_textSlant);

	debugC(3, kDebugText, "renderText: sprite: %d x: %d y: %d w: %d h: %d font: '%s' text: '%s'", spriteId, x, y, width, height, _vm->_wm->_fontMan->getFontName(*macFont).c_str(), Common::toPrintable(textCast->_ftext).c_str());

	uint16 boxShadow = (uint16)textCast->_boxShadow;
	uint16 borderSize = (uint16)textCast->_borderSize;
	if (textRect != NULL)
		borderSize = 0;
	uint16 padding = (uint16)textCast->_gutterSize;
	uint16 textShadow = (uint16)textCast->_textShadow;

	//uint32 rectLeft = textCast->initialRect.left;
	//uint32 rectTop = textCast->initialRect.top;

	textCast->_cachedMacText->clip(width);
	const Graphics::ManagedSurface *textSurface = textCast->_cachedMacText->getSurface();

	if (!textSurface)
		return;

	height = textSurface->h;
	if (textRect != NULL) {
		// TODO: this offset could be due to incorrect fonts loaded!
		textRect->bottom = height + textCast->_cachedMacText->getLineCount();
	}

	uint16 textX = 0, textY = 0;

	if (textRect == NULL) {
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

		if (textCast->_textAlign == kTextAlignRight)
			textX -= 1;

		if (textShadow > 0)
			textX--;
	} else {
		x++;
		if (width % 2 != 0)
			x++;
		y += 2;
	}

	switch (textCast->_textAlign) {
	case kTextAlignLeft:
	default:
		break;
	case kTextAlignCenter:
		textX = (width / 2) - (textSurface->w / 2) + (padding / 2) + borderSize;
		break;
	case kTextAlignRight:
		textX = width - (textSurface->w + 1) + (borderSize * 2) - (textShadow * 2) - (padding);
		break;
	}

	Graphics::ManagedSurface textWithFeatures(width + (borderSize * 2) + boxShadow + textShadow, height + borderSize + boxShadow + textShadow);
	textWithFeatures.fillRect(Common::Rect(textWithFeatures.w, textWithFeatures.h), 255 - _vm->getCurrentScore()->getStageColor());

	if (textRect == NULL && boxShadow > 0) {
		textWithFeatures.fillRect(Common::Rect(boxShadow, boxShadow, textWithFeatures.w + boxShadow, textWithFeatures.h), 0);
	}

	if (textRect == NULL && borderSize != kSizeNone) {
		for (int bb = 0; bb < borderSize; bb++) {
			Common::Rect borderRect(bb, bb, textWithFeatures.w - bb - boxShadow - textShadow, textWithFeatures.h - bb - boxShadow - textShadow);
			textWithFeatures.fillRect(borderRect, 0xff);
			textWithFeatures.frameRect(borderRect, 0);
		}
	}

	if (textShadow > 0)
		textWithFeatures.transBlitFrom(textSurface->rawSurface(), Common::Point(textX + textShadow, textY + textShadow), 0xff);

	textWithFeatures.transBlitFrom(textSurface->rawSurface(), Common::Point(textX, textY), 0xff);

	InkType ink = _sprites[spriteId]->_ink;

	if (spriteId == _vm->getCurrentScore()->_currentMouseDownSpriteId)
		ink = kInkTypeReverse;

	inkBasedBlit(surface, textWithFeatures, ink, Common::Rect(x, y, x + width, y + height));
}

void Frame::inkBasedBlit(Graphics::ManagedSurface &targetSurface, const Graphics::Surface &spriteSurface, InkType ink, Common::Rect drawRect) {
	// drawRect could be bigger than the spriteSurface. Clip it
	Common::Rect t(spriteSurface.w, spriteSurface.h);
	t.moveTo(drawRect.left, drawRect.top);
	drawRect.clip(t);

	switch (ink) {
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
		warning("Frame::inkBasedBlit(): Unhandled ink type %d", ink);
		targetSurface.blitFrom(spriteSurface, Common::Point(drawRect.left, drawRect.top));
		break;
	}
}

void Frame::drawBackgndTransSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	uint8 skipColor = _vm->getPaletteColorCount() - 1; // FIXME is it always white (last entry in pallette) ?
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!target.clip(srcRect, drawRect))
		return; // Out of screen

	for (int ii = 0; ii < srcRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < srcRect.width(); j++) {
			if (*src != skipColor)
				*dst = *src;

			src++;
			dst++;
		}
	}
}

void Frame::drawGhostSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!target.clip(srcRect, drawRect))
		return; // Out of screen

	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < srcRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < srcRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0) && (*src != skipColor))
				*dst = (_vm->getPaletteColorCount() - 1) - *src; // Oposite color

			src++;
			dst++;
		}
	}
}

void Frame::drawReverseSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect) {
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!target.clip(srcRect, drawRect))
		return; // Out of screen

	uint8 skipColor = _vm->getPaletteColorCount() - 1;
	for (int ii = 0; ii < srcRect.height(); ii++) {
		const byte *src = (const byte *)sprite.getBasePtr(srcRect.left, srcRect.top + ii);
		byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + ii);

		for (int j = 0; j < srcRect.width(); j++) {
			if ((getSpriteIDFromPos(Common::Point(drawRect.left + j, drawRect.top + ii)) != 0)) {
				if (*src != skipColor) {
					*dst = 0xff - *src;
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
	Common::Rect srcRect(sprite.w, sprite.h);

	if (!target.clip(srcRect, drawRect))
		return; // Out of screen

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
		debugC(1, kDebugImages, "Frame::drawMatteSprite(): No white color for Matte image");

		for (int yy = 0; yy < srcRect.height(); yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(srcRect.left, srcRect.top + yy);
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

		for (int yy = 0; yy < srcRect.height(); yy++) {
			const byte *src = (const byte *)tmp.getBasePtr(srcRect.left, srcRect.top + yy);
			const byte *mask = (const byte *)ff.getMask()->getBasePtr(srcRect.left, srcRect.top + yy);
			byte *dst = (byte *)target.getBasePtr(drawRect.left, drawRect.top + yy);

			for (int xx = 0; xx < srcRect.width(); xx++, src++, dst++, mask++)
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

Common::Rect *Frame::getSpriteRect(uint16 spriteId) {
	for (int dr = _drawRects.size() - 1; dr >= 0; dr--)
		if (_drawRects[dr]->spriteId == spriteId)
			return &_drawRects[dr]->rect;

	return nullptr;
}

} // End of namespace Director
