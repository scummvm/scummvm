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
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/palette.h"
#include "picture/resource.h"
#include "picture/screen.h"
#include "picture/script.h"
#include "picture/segmap.h"

namespace Picture {

Screen::Screen(PictureEngine *vm) : _vm(vm) {

	_frontScreen = new byte[268800];
	_backScreen = new byte[870400];

	memset(_fontResIndexArray, 0, sizeof(_fontResIndexArray));
	_fontColor1 = 0;
	_fontColor2 = 0;

	// Screen shaking
	_shakeActive = false;
	_shakeCounterInit = 0;
	_shakeCounter = 0;
	_shakePos = 0;

	// Verb line
	_verbLineNum = 0;
	memset(_verbLineItems, 0, sizeof(_verbLineItems));
	_verbLineX = 160;
	_verbLineY = 2;
	_verbLineWidth = 20;
	_verbLineCount = 0;

	// Talk text
	_talkTextItemNum = 0;
	memset(_talkTextItems, 0, sizeof(_talkTextItems));
	_talkTextX = 0;//TODO correct init values
	_talkTextY = 0;
	_talkTextFontColor = 0;
	_talkTextMaxWidth = 520;

}

Screen::~Screen() {

	delete[] _frontScreen;
	delete[] _backScreen;

}

void Screen::unpackRle(byte *source, byte *dest, uint16 width, uint16 height) {
	int32 size = width * height;
	while (size > 0) {
		byte a = *source++;
		byte b = *source++;
		if (a == 0) {
			dest += b;
			size -= b;
		} else {
			b = ((b << 4) & 0xF0) | ((b >> 4) & 0x0F);
			memset(dest, b, a);
			dest += a;
			size -= a;
		}
	}
}

void Screen::loadMouseCursor(uint resIndex) {
	byte mouseCursor[16 * 16], *mouseCursorP = mouseCursor;
	byte *cursorData = _vm->_res->load(resIndex);
	for (int i = 0; i < 32; i++) {
		byte pixel;
		byte mask1 = *cursorData++;
		byte mask2 = *cursorData++;
		for (int j = 0; j < 8; j++) {
			pixel = 0xE5;
			if ((mask2 & 0x80) == 0)
				pixel = 0xE0;
			mask2 <<= 1;
			if ((mask1 & 0x80) == 0)
				pixel = 0;
			mask1 <<= 1;
			*mouseCursorP++ = pixel;
		}
	}
	//CursorMan.replaceCursor((const byte*)mouseCursor, 16, 16, 0, 0, 0);
	// FIXME: Where's the cursor hotspot? Using 8, 8 seems good enough for now.
	CursorMan.replaceCursor((const byte*)mouseCursor, 16, 16, 8, 8, 0);
}

void Screen::drawGuiImage(int16 x, int16 y, uint resIndex) {

	byte *imageData = _vm->_res->load(resIndex);
	int16 headerSize = READ_LE_UINT16(imageData);
	int16 width = imageData[2];
	int16 height = imageData[3];
	int16 workWidth = width, workHeight = height;
	imageData += headerSize;
	
	byte *dest = _frontScreen + x + (y + _vm->_cameraHeight) * 640;

	debug(0, "Screen::drawGuiImage() x = %d; y = %d; w = %d; h = %d; resIndex = %d", x, y, width, height, resIndex);
	
	while (workHeight > 0) {
		int count = 1;
		byte pixel = *imageData++;
		if (pixel & 0x80) {
			pixel &= 0x7F;
			count = *imageData++;
			count += 2;
		}
		pixel = pixel + 0xE0;
		while (count-- && workHeight > 0) {
			*dest++ = pixel;
			workWidth--;
			if (workWidth == 0) {
				workHeight--;
				dest += 640 - width;
				workWidth = width;
			}
		}
	}
	
}

void Screen::startShakeScreen(int16 shakeCounter) {
	_shakeActive = true;
	_shakeCounterInit = shakeCounter;
	_shakeCounter = shakeCounter;
	_shakePos = 0;
}

void Screen::stopShakeScreen() {
	_shakeActive = false;
	_vm->_system->setShakePos(0);
}

void Screen::updateShakeScreen() {
	if (_shakeActive) {
		_shakeCounter--;
		if (_shakeCounter == 0) {
			_shakeCounter = _shakeCounterInit;
			_shakePos ^= 8;
			_vm->_system->setShakePos(_shakePos);
		}
	}
}

void Screen::addStaticSprite(byte *spriteItem) {

	DrawRequest drawRequest;
	memset(&drawRequest, 0, sizeof(drawRequest));

	drawRequest.y = READ_LE_UINT16(spriteItem + 0);
	drawRequest.x = READ_LE_UINT16(spriteItem + 2);
	int16 fragmentId = READ_LE_UINT16(spriteItem + 4);
	drawRequest.baseColor = _vm->_palette->findFragment(fragmentId) & 0xFF;
	drawRequest.resIndex = READ_LE_UINT16(spriteItem + 6);
	drawRequest.flags = READ_LE_UINT16(spriteItem + 8);
	drawRequest.scaling = 0;

	debug(0, "Screen::addStaticSprite() x = %d; y = %d; baseColor = %d; resIndex = %d; flags = %04X", drawRequest.x, drawRequest.y, drawRequest.baseColor, drawRequest.resIndex, drawRequest.flags);

	addDrawRequest(drawRequest);

}

void Screen::addAnimatedSprite(int16 x, int16 y, int16 fragmentId, byte *data, int16 *spriteArray, bool loop, int mode) {

	debug(0, "Screen::addAnimatedSprite(%d, %d, %d)", x, y, fragmentId);

	DrawRequest drawRequest;
	memset(&drawRequest, 0, sizeof(drawRequest));

	drawRequest.x = x;
	drawRequest.y = y;
	drawRequest.baseColor = _vm->_palette->findFragment(fragmentId) & 0xFF;

	if (mode == 1) {
		drawRequest.scaling = _vm->_segmap->getScalingAtPoint(drawRequest.x, drawRequest.y);
	} else if (mode == 2) {
		drawRequest.scaling = 0;
	}

	int16 count = spriteArray[0];

	debug(0, "count = %d", count);

	for (int16 index = 1; index <= count; index++) {

		byte *spriteItem = data + spriteArray[index];

		uint16 loopNum = READ_LE_UINT16(spriteItem + 0) & 0x7FFF;
		uint16 loopCount = READ_LE_UINT16(spriteItem + 2);
		uint16 frameNum = READ_LE_UINT16(spriteItem + 4);
		uint16 frameCount = READ_LE_UINT16(spriteItem + 6);
		drawRequest.resIndex = READ_LE_UINT16(spriteItem + 8);
		drawRequest.flags = READ_LE_UINT16(spriteItem + 10 + loopNum * 2);

		debug(0, "Screen::addAnimatedSprite(%d of %d) loopNum = %d; loopCount = %d; frameNum = %d; frameCount = %d; resIndex = %d; flags = %04X, mode = %d",
			index, count, loopNum, loopCount, frameNum, frameCount, drawRequest.resIndex, drawRequest.flags, mode);

		addDrawRequest(drawRequest);

		frameNum++;
		if (frameNum == frameCount) {
			frameNum = 0;
			loopNum++;
			if (loopNum == loopCount) {
				if (loop) {
					loopNum = 0;
				} else {
					loopNum--;
				}
			}
		} else {
			loopNum |= 0x8000;
		}
		
		WRITE_LE_UINT16(spriteItem + 0, loopNum);
		WRITE_LE_UINT16(spriteItem + 4, frameNum);

	}

}

void Screen::clearSprites() {

	_spriteDrawList.clear();
	// TODO

}

void Screen::addDrawRequest(const DrawRequest &drawRequest) {

	int16 scaleValueX, scaleValueY;
	int16 xoffs, yoffs;
	byte *spriteData;
	int16 frameNum;

	SpriteDrawItem sprite;
	memset(&sprite, 0, sizeof(SpriteDrawItem));

	if (drawRequest.flags == 0xFFFF)
		return;

	sprite.flags = 0;
	sprite.baseColor = drawRequest.baseColor;
	sprite.x = drawRequest.x;
	sprite.y = drawRequest.y;
	sprite.ybottom = drawRequest.y;
	sprite.resIndex = drawRequest.resIndex;
	
	spriteData = _vm->_res->load(drawRequest.resIndex);
	
	if (drawRequest.flags & 0x1000) {
		sprite.flags |= 4;
	}

	if (drawRequest.flags & 0x2000) {
		sprite.flags |= 0x10;
	}
	
	if (drawRequest.flags & 0x4000) {
		sprite.flags |= 0x40;
	}

	frameNum = drawRequest.flags & 0x0FFF;

	// First initialize the sprite item with the values from the sprite resource

	SpriteFrameEntry spriteFrameEntry(spriteData + frameNum * 12);
	
	if (spriteFrameEntry.w == 0 || spriteFrameEntry.h == 0)
		return;
	
	sprite.offset = spriteFrameEntry.offset;

	sprite.width = spriteFrameEntry.w;
	sprite.height = spriteFrameEntry.h;

	sprite.origWidth = spriteFrameEntry.w;
	sprite.origHeight = spriteFrameEntry.h;

	if (drawRequest.flags & 0x1000) {
		xoffs = spriteFrameEntry.w - spriteFrameEntry.x;
	} else {
		xoffs = spriteFrameEntry.x;
	}

	yoffs = spriteFrameEntry.y;

	// If the sprite should be scaled we need to initialize some values now

	if (drawRequest.scaling != 0) {

		byte scaleValue = ABS(drawRequest.scaling);

		scaleValueX = scaleValue * sprite.origWidth;
		sprite.xdelta = (10000 * sprite.origWidth) / scaleValueX;
		scaleValueX /= 100;

		scaleValueY = scaleValue * sprite.origHeight;
		sprite.ydelta = (10000 * sprite.origHeight) / scaleValueY;
		scaleValueY /= 100;

		if (drawRequest.scaling > 0) {
			sprite.flags |= 2;
			sprite.width = sprite.origWidth + scaleValueX;
			sprite.height = sprite.origHeight + scaleValueY;
			xoffs += (xoffs * scaleValue) / 100;
			yoffs += (yoffs * scaleValue) / 100;
		} else {
			sprite.flags |= 1;
			sprite.width = sprite.origWidth - scaleValueX;
			sprite.height = sprite.origHeight - 1 - scaleValueY;
			if (sprite.width <= 0 || sprite.height <= 0)
				return;
			xoffs -= (xoffs * scaleValue) / 100;
			yoffs -= (yoffs * scaleValue) / 100;
		}
		
	}
	
	sprite.x -= xoffs;
	sprite.y -= yoffs;

	sprite.yerror = sprite.ydelta;

	// Now we check if the sprite needs to be clipped

	// Clip Y
	if (sprite.y - _vm->_cameraY < 0) {

		int16 clipHeight = ABS(sprite.y - _vm->_cameraY);
		int16 chopHeight, skipHeight, lineWidth;
		byte *spriteFrameData;

		sprite.height -= clipHeight;
		if (sprite.height <= 0)
			return;
		
		sprite.y = _vm->_cameraY;

		// If the sprite is scaled
		if (sprite.flags & 3) {
			chopHeight = sprite.ydelta;
			skipHeight = clipHeight;
			if ((sprite.flags & 2) == 0) {
				do {
					chopHeight -= 100;
					if (chopHeight <= 0) {
						skipHeight++;
						chopHeight += sprite.ydelta;
					} else {
						clipHeight--;
					}
				} while (clipHeight > 0);
			} else {
				do {
					chopHeight -= 100;
					if (chopHeight < 0) {
						skipHeight--;
						chopHeight += sprite.ydelta + 100;
					}
					clipHeight--;
				} while (clipHeight > 0);
			}
			sprite.yerror = chopHeight;
		}
		
		spriteFrameData = spriteData + sprite.offset;
		
		// Now the sprite's offset is adjusted to point to the starting line
		if ((sprite.flags & 0x10) == 0) {
			while (clipHeight--) {
				lineWidth = 0;
				while (lineWidth </*CHECKME was != */ sprite.origWidth) {
					sprite.offset++;
					lineWidth += (*spriteFrameData++) & 0x0F;
				}
			}
		} else {
			lineWidth = 0;
			while (clipHeight--) {
				while (lineWidth < sprite.origWidth) {
					sprite.offset += 2;
					spriteFrameData++;
					lineWidth += *spriteFrameData++;
				}
			}
		}

	}

	if (sprite.y + sprite.height - _vm->_cameraY - _vm->_cameraHeight > 0)
		sprite.height -= sprite.y + sprite.height - _vm->_cameraY - _vm->_cameraHeight;
	if (sprite.height <= 0)
		return;

	sprite.skipX = 0;

	if (drawRequest.flags & 0x1000) {
		// Left border
		if (sprite.x - _vm->_cameraX < 0) {
			sprite.width -= ABS(sprite.x - _vm->_cameraX);
			sprite.x = _vm->_cameraX;
		}
		// Right border
		if (sprite.x + sprite.width - _vm->_cameraX - 640 > 0) {
			sprite.flags |= 8;
			sprite.skipX = sprite.x + sprite.width - _vm->_cameraX - 640;
			sprite.width -= sprite.skipX;
		}
	} else {
		// Left border
		if (sprite.x - _vm->_cameraX < 0) {
			sprite.flags |= 8;
			sprite.skipX = ABS(sprite.x - _vm->_cameraX);
			sprite.width -= sprite.skipX;
			sprite.x = _vm->_cameraX;
		}
		// Right border
		if (sprite.x + sprite.width - _vm->_cameraX - 640 > 0) {
			sprite.flags |= 8;
			sprite.width -= sprite.x + sprite.width - _vm->_cameraX - 640;
		}
	}

	if (sprite.width <= 0)
		return;

	// Add sprite sorted by priority
	Common::List<SpriteDrawItem>::iterator iter = _spriteDrawList.begin();
	while (iter != _spriteDrawList.end() && (*iter).ybottom <= sprite.ybottom) {
		iter++;
	}
	_spriteDrawList.insert(iter, sprite);
	
}

void Screen::drawSprite(SpriteDrawItem *sprite) {

	debug(0, "Screen::drawSprite() x = %d; y = %d; flags = %04X; resIndex = %d; offset = %08X; drawX = %d; drawY = %d",
		sprite->x, sprite->y, sprite->flags, sprite->resIndex, sprite->offset,
		sprite->x - _vm->_cameraX, sprite->y - _vm->_cameraY);
	debug(0, "Screen::drawSprite() width = %d; height = %d; origWidth = %d; origHeight = %d",
		sprite->width, sprite->height, sprite->origWidth, sprite->origHeight);

	byte *source = _vm->_res->load(sprite->resIndex) + sprite->offset;
	byte *dest = _frontScreen + (sprite->x - _vm->_cameraX) + (sprite->y - _vm->_cameraY) * 640;

	SpriteReader spriteReader(source, sprite);

	if (sprite->flags & 0x40) {
		// TODO: Shadow sprites
		if (sprite->flags & 1) {
			SpriteFilterScaleDown spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else if (sprite->flags & 2) {
			SpriteFilterScaleUp spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else {
			drawSpriteCore(dest, spriteReader, sprite);
		}
	} else if (sprite->flags & 0x10) {
		// 256 color sprite
		drawSpriteCore(dest, spriteReader, sprite);
	} else {
		// 16 color sprite
		if (sprite->flags & 1) {
			SpriteFilterScaleDown spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else if (sprite->flags & 2) {
			SpriteFilterScaleUp spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else {
			drawSpriteCore(dest, spriteReader, sprite);
		}
	}

	debug(0, "Screen::drawSprite() ok");

}

void Screen::drawSpriteCore(byte *dest, SpriteFilter &reader, SpriteDrawItem *sprite) {

	int16 destInc;

	if (sprite->flags & 4) {
		destInc = -1;
		dest += sprite->width;
	} else {
		destInc = 1;
	}

	SpriteReaderStatus status;
	PixelPacket packet;
	
	byte *destp = dest;
	int16 skipX = sprite->skipX;

	int16 w = sprite->width;
	int16 h = sprite->height;

	do {
		status = reader.readPacket(packet);

		if (skipX > 0) {
			while (skipX > 0) {
				skipX -= packet.count;
				if (skipX < 0) {
					packet.count = ABS(skipX);
					break;
				}
				status = reader.readPacket(packet);
			}
		}
		
		if (w - packet.count < 0)
			packet.count = w;

		w -= packet.count;

		if (((sprite->flags & 0x40) && (packet.pixel != 0)) ||
			((sprite->flags & 0x10) && (packet.pixel != 0xFF)) ||
			!(sprite->flags & 0x10) && (packet.pixel != 0))
		{
			if (sprite->flags & 0x40) {
				while (packet.count--) {
					*dest = _vm->_palette->getColorTransPixel(*dest);
					dest += destInc;
				}
			} else {
				if (sprite->flags & 0x10) {
					packet.pixel = ((packet.pixel << 4) & 0xF0) | ((packet.pixel >> 4) & 0x0F);
				} else {
					packet.pixel += sprite->baseColor - 1;
				}
				while (packet.count--) {
					*dest = packet.pixel;
					dest += destInc;
				}
			}
		} else {
			dest += packet.count * destInc;
		}

		if (status == kSrsEndOfLine || w <= 0) {
			if (w <= 0) {
				while (status == kSrsPixelsLeft) {
					status = reader.readPacket(packet);
				}
			}
			dest = destp + 640;
			destp = dest;
			skipX = sprite->skipX;
			w = sprite->width;
			h--;
		}

	} while (status != kSrsEndOfSprite && h > 0);

}

void Screen::drawSprites() {
	for (Common::List<SpriteDrawItem>::iterator iter = _spriteDrawList.begin(); iter != _spriteDrawList.end(); iter++) {
		SpriteDrawItem *sprite = &(*iter);
		drawSprite(sprite);
		_vm->_segmap->restoreMasksBySprite(sprite);
	}
}

void Screen::updateVerbLine(int16 slotIndex, int16 slotOffset) {

	debug(0, "Screen::updateVerbLine() _verbLineNum = %d; _verbLineX = %d; _verbLineY = %d; _verbLineWidth = %d; _verbLineCount = %d",
		_verbLineNum, _verbLineX, _verbLineY, _verbLineWidth, _verbLineCount);

	Font font(_vm->_res->load(_fontResIndexArray[0]));

	_verbLineItems[_verbLineNum].slotIndex = slotIndex;
	_verbLineItems[_verbLineNum].slotOffset = slotOffset;

	// First clear the line
	int16 y = _verbLineY;
	for (int16 i = 0; i < _verbLineCount; i++) {
		byte *dest = _frontScreen + _verbLineX - _verbLineWidth / 2 + (y - 1 + _vm->_cameraHeight) * 640;
		for (int16 j = 0; j < 20; j++) {
			memset(dest, 0xE0, _verbLineWidth);
			dest += 640;
		}
		y += 18;
	}

	int width = 0;
	byte *sourceString;
	byte *destString;
	byte len;

	_tempStringLen1 = 0;
	destString = _tempString;
	y = _verbLineY;
	
	memset(_tempString, 0, sizeof(_tempString));
	
	for (int16 i = 0; i <= _verbLineNum; i++) {
		sourceString = _vm->_script->getSlotData(_verbLineItems[i].slotIndex) + _verbLineItems[i].slotOffset;
		preprocessText(_fontResIndexArray[0], _verbLineWidth, width, sourceString, destString, len);
		_tempStringLen1 += len;
	}

	if (_verbLineCount != 1) {
		int16 charWidth;
		if (*sourceString < 0xF0) {
			while (*sourceString > 0x20 && *sourceString < 0xF0 && len > 0/*CHECKME, len check added*/) {
				byte ch = *sourceString--;
				_tempStringLen1--;
				len--;
				charWidth = font.getCharWidth(ch) + font.getSpacing() - 1;
				width -= charWidth;
			}
			width += charWidth;
			sourceString++;
			_tempStringLen1 -= len;
			_tempStringLen2 = len + 1;
			
			drawStringEx(_verbLineX - 1 - (width / 2), y, 0xF9, 0xFF, _fontResIndexArray[0]);

			destString = _tempString;
			width = 0;
			preprocessText(_fontResIndexArray[0], _verbLineWidth, width, sourceString, destString, len);
			
			_tempStringLen1 += len;
			y += 9;
		}
		y += 9;
	}
	
	_tempStringLen1 -= len;
	_tempStringLen2 = len;

	drawStringEx(_verbLineX - 1 - (width / 2), y, 0xF9, 0xFF, _fontResIndexArray[0]);

}

void Screen::updateTalkText(int16 slotIndex, int16 slotOffset) {

	int16 x, y, maxWidth, width, length;
	byte durationModifier = 1;
	byte *textData = _vm->_script->getSlotData(slotIndex) + slotOffset;

	TalkTextItem *item = &_talkTextItems[_talkTextItemNum];

	item->fontNum = 0;
	item->color = _talkTextFontColor;

	x = CLIP<int16>(_talkTextX - _vm->_cameraX, 120, _talkTextMaxWidth);
	y = CLIP<int16>(_talkTextY - _vm->_cameraY, 4, _vm->_cameraHeight - 16);

	maxWidth = 624 - ABS(x - 320) * 2;

	while (1) {
		if (*textData == 0x0A) {
			x = CLIP<int16>(textData[3], 120, _talkTextMaxWidth);
			y = CLIP<int16>(READ_LE_UINT16(&textData[1]), 4, _vm->_cameraHeight - 16);
			maxWidth = 624 - ABS(x - 320) * 2;
			textData += 4;
		} else if (*textData == 0x14) {
			item->color = textData[1];
			textData += 2;
		} else if (*textData == 0x19) {
			durationModifier = textData[1];
			textData += 2;
		} else if (*textData < 0x0A) {
			item->fontNum = textData[0];
			textData += 1;
		} else
			break;
	}
	
	item->slotIndex = slotIndex;
	item->slotOffset = textData - _vm->_script->getSlotData(slotIndex);

	width = 0;
	length = 0;

	item->duration = 0;
	item->rectCount = 0;

	Font font(_vm->_res->load(_fontResIndexArray[item->fontNum]));
	int16 wordLength, wordWidth;

	while (*textData < 0xF0) {
		if (*textData == 0x1E) {
			textData++;
			addTalkTextRect(font, x, y, length, width, item);
			// CHECKME?
			width = 0;
			length = 0;
		} else {
			wordLength = 0;
			wordWidth = 0;
			while (*textData >= 0x20 && *textData < 0xF0) {
				byte ch = *textData++;
				wordLength++;
				if (ch == 0x20) {
					wordWidth += font.getWidth();
					break;
				} else {
					wordWidth += font.getCharWidth(ch) + font.getSpacing() - 1;
				}
			}
			
			debug(0, "## width = %d; wordWidth = %d; width + wordWidth = %d; maxWidth + font.getWidth() = %d",
				width, wordWidth, width + wordWidth, maxWidth + font.getWidth());
			
			if (width + wordWidth > maxWidth + font.getWidth()) {
				addTalkTextRect(font, x, y, length, width, item);
				width = wordWidth;
				length = wordLength;
			} else {
				width += wordWidth;
				length += wordLength;
			}
		}
	}
	
	addTalkTextRect(font, x, y, length, width, item);

	debug(0, "## item->rectCount = %d", item->rectCount);

	int16 textDurationMultiplier = item->duration + 8;
	// TODO: Check sound/text flags
	if (*textData == 0xFE) {
		//textDurationMultiplier += 100;
	}
	item->duration = 4 * textDurationMultiplier * durationModifier;

}

void Screen::addTalkTextRect(Font &font, int16 x, int16 &y, int16 length, int16 width, TalkTextItem *item) {

	if (width > 0) {
		TextRect *textRect = &item->rects[item->rectCount];
		width = width + 1 - font.getSpacing();
		textRect->width = width;
		item->duration += length;
		textRect->length = length;
		textRect->y = y;
		textRect->x = CLIP<int16>(x - width / 2, 0, 640);
		item->rectCount++;
	}
	
	y += font.getHeight() - 1;

}

void Screen::drawTalkTextItems() {

	for (int16 i = 0; i <= _talkTextItemNum; i++) {
		TalkTextItem *item = &_talkTextItems[i];
		byte *text = _vm->_script->getSlotData(item->slotIndex) + item->slotOffset;

		if (item->fontNum == -1 || item->duration == 0)
			continue;

		item->duration -= _vm->_counter01;
		if (item->duration < 0)
			item->duration = 0;

		for (byte j = 0; j < item->rectCount; j++) {
			drawString(item->rects[j].x, item->rects[j].y, item->color, _fontResIndexArray[item->fontNum],
				text, item->rects[j].length, NULL, true);
			text += item->rects[j].length;
		}
		
	}

}

int16 Screen::getTalkTextDuration() {
	return _talkTextItems[_talkTextItemNum].duration;
}

void Screen::finishTextDrawItems() {
	for (int16 i = 0; i <= _talkTextItemNum; i++) {
		_talkTextItems[i].duration = 0;
	}
}

void Screen::registerFont(uint fontIndex, uint resIndex) {
	_fontResIndexArray[fontIndex] = resIndex;
}

void Screen::printText(byte *textData) {

	int16 x = 0, y = 0;

	// Really strange stuff.
	for (int i = 30; i >= 0; i--) {
		if (textData[i] >= 0xF0)
			break;
		if (i == 0)
			return;
	}

	do {
	
		if (*textData == 0x0A) {
			// Set text position
			y = textData[1];
			x = READ_LE_UINT32(textData + 2);
			textData += 4;
		} else if (*textData == 0x0B) {
			// Inc text position
			y += textData[1]; // CHECKME: Maybe these are signed?
			x += textData[2];
			textData += 3;
		} else {
			byte *destString = _tempString;
			int width = 0;
			_tempStringLen1 = 0;
			preprocessText(_fontResIndexArray[1], 640, width, textData, destString, _tempStringLen2);
			drawStringEx(x - width / 2, y, _fontColor1, _fontColor2, _fontResIndexArray[1]);
		}
	
	} while (*textData != 0xFF);

}

void Screen::preprocessText(uint fontResIndex, int maxWidth, int &width, byte *&sourceString, byte *&destString, byte &len) {

	Font font(_vm->_res->load(fontResIndex));

	len = 0;
	while (*sourceString >= 0x20 && *sourceString < 0xF0) {
		byte ch = *sourceString;
		byte charWidth;
		if (ch <= 0x20)
			charWidth = font.getWidth();
		else
			charWidth = font.getCharWidth(ch) + font.getSpacing() - 1;
		if (width + charWidth >= maxWidth)
			break;
		len++;
		width += charWidth;
		*destString++ = *sourceString++;
	}
}

void Screen::drawStringEx(int16 x, int16 y, byte fontColor1, byte fontColor2, uint fontResIndex) {

	debug(0, "Screen::drawStringEx(%d, %d, %d, %d, %d) _tempStringLen1 = %d; _tempStringLen2 = %d", x, y, fontColor1, fontColor2, fontResIndex, _tempStringLen1, _tempStringLen2);

	int16 ywobble = 1;

	x = drawString(x + 1, y + _vm->_cameraHeight, fontColor1, fontResIndex, _tempString, _tempStringLen1, &ywobble, false);
	x = drawString(x, y + _vm->_cameraHeight, fontColor2, fontResIndex, _tempString + _tempStringLen1, _tempStringLen2, &ywobble, false);

}

int16 Screen::drawString(int16 x, int16 y, byte color, uint fontResIndex, byte *text, int len, int16 *ywobble, bool outline) {

	debug(0, "Screen::drawString(%d, %d, %d, %d)", x, y, color, fontResIndex);

	Font font(_vm->_res->load(fontResIndex));

	if (len == -1)
		len = strlen((char*)text);

	int16 yadd = 0;
	if (ywobble)
		yadd = *ywobble;

 	while (len--) {
		byte ch = *text++;
		if (ch <= 0x20) {
			x += font.getWidth();
		} else {
			drawChar(font, _frontScreen, x, y - yadd, ch, color, outline);
			x += font.getCharWidth(ch) + font.getSpacing() - 1;
			yadd = -yadd;
		}
	}

	if (ywobble)
		*ywobble = yadd;

	return x;

}

void Screen::drawChar(const Font &font, byte *dest, int16 x, int16 y, byte ch, byte color, bool outline) {

	int16 charWidth, charHeight;
	byte *charData;

	dest += x + (y * 640);

	charWidth = font.getCharWidth(ch);
	charHeight = font.getHeight() - 2;
	charData = font.getCharData(ch);

	while (charHeight--) {
		byte lineWidth = charWidth;
		while (lineWidth > 0) {
			byte count = charData[0] & 0x0F;
			byte flags = charData[0] & 0xF0;
			charData++;
			if ((flags & 0x80) == 0) {
 				if (flags & 0x10) {
					memset(dest, color, count);
				} else if (outline) {
					memset(dest, 0, count);
				}
			}
			dest += count;
			lineWidth -= count;
		}
		dest += 640 - charWidth;
	}

}

void Screen::saveState(Common::WriteStream *out) {

	// Save verb line
	out->writeUint16LE(_verbLineNum);
	out->writeUint16LE(_verbLineX);
	out->writeUint16LE(_verbLineY);
	out->writeUint16LE(_verbLineWidth);
	out->writeUint16LE(_verbLineCount);
	for (int i = 0; i < 8; i++) {
		out->writeUint16LE(_verbLineItems[i].slotIndex);
		out->writeUint16LE(_verbLineItems[i].slotOffset);
	}

	// Save talk text items
	out->writeUint16LE(_talkTextX);
	out->writeUint16LE(_talkTextY);
	out->writeUint16LE(_talkTextMaxWidth);
	out->writeByte(_talkTextFontColor);
	out->writeUint16LE(_talkTextItemNum);
	for (int i = 0; i < 5; i++) {
		out->writeUint16LE(_talkTextItems[i].duration);
		out->writeUint16LE(_talkTextItems[i].slotIndex);
		out->writeUint16LE(_talkTextItems[i].slotOffset);
		out->writeUint16LE(_talkTextItems[i].fontNum);
		out->writeByte(_talkTextItems[i].color);
		out->writeByte(_talkTextItems[i].rectCount);
		for (int j = 0; j < _talkTextItems[i].rectCount; j++) {
			out->writeUint16LE(_talkTextItems[i].rects[j].x);
			out->writeUint16LE(_talkTextItems[i].rects[j].y);
			out->writeUint16LE(_talkTextItems[i].rects[j].width);
			out->writeUint16LE(_talkTextItems[i].rects[j].length);
		}
	}

	// Save GUI bitmap
	{
		byte *gui = _frontScreen + _vm->_cameraHeight * 640;
		for (int i = 0; i < _vm->_guiHeight; i++) {
			out->write(gui, 640);
			gui += 640;
		}
	}

	// Save fonts
	for (int i = 0; i < 10; i++)
		out->writeUint32LE(_fontResIndexArray[i]);
	out->writeByte(_fontColor1);
	out->writeByte(_fontColor2);

}

void Screen::loadState(Common::ReadStream *in) {

	// Load verb line
	_verbLineNum = in->readUint16LE();
	_verbLineX = in->readUint16LE();
	_verbLineY = in->readUint16LE();
	_verbLineWidth = in->readUint16LE();
	_verbLineCount = in->readUint16LE();
	for (int i = 0; i < 8; i++) {
		_verbLineItems[i].slotIndex = in->readUint16LE();
		_verbLineItems[i].slotOffset = in->readUint16LE();
	}
	
	// Load talk text items
	_talkTextX = in->readUint16LE();
	_talkTextY = in->readUint16LE();
	_talkTextMaxWidth = in->readUint16LE();
	_talkTextFontColor = in->readByte();
	_talkTextItemNum = in->readUint16LE();
	for (int i = 0; i < 5; i++) {
		_talkTextItems[i].duration = in->readUint16LE();
		_talkTextItems[i].slotIndex = in->readUint16LE();
		_talkTextItems[i].slotOffset = in->readUint16LE();
		_talkTextItems[i].fontNum = in->readUint16LE();
		_talkTextItems[i].color = in->readByte();
		_talkTextItems[i].rectCount = in->readByte();
		for (int j = 0; j < _talkTextItems[i].rectCount; j++) {
			_talkTextItems[i].rects[j].x = in->readUint16LE();
			_talkTextItems[i].rects[j].y = in->readUint16LE();
			_talkTextItems[i].rects[j].width = in->readUint16LE();
			_talkTextItems[i].rects[j].length = in->readUint16LE();
		}
	}
	
	// Load GUI bitmap
	{
		byte *gui = _frontScreen + _vm->_cameraHeight * 640;
		for (int i = 0; i < _vm->_guiHeight; i++) {
			in->read(gui, 640);
			gui += 640;
		}
	}

	// Load fonts
	for (int i = 0; i < 10; i++)
		_fontResIndexArray[i] = in->readUint32LE();
	_fontColor1 = in->readByte();
	_fontColor2 = in->readByte();

}


/*
void Screen::update() {
}
*/

} // End of namespace Picture
