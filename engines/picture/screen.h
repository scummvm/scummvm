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

#ifndef PICTURE_SCREEN_H
#define PICTURE_SCREEN_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/list.h"
#include "common/array.h"

#include "graphics/surface.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Picture {

struct DrawRequest {
	int16 x, y;
	int16 resIndex;
	uint16 flags;
	int16 baseColor;
	int8 scaling;
};

struct SpriteDrawItem {
	int16 x, y;
	int16 width, height;
	int16 origWidth, origHeight;
	int16 resIndex;
	uint32 offset;
	int16 xdelta, ydelta;
	uint16 flags;
	int16 skipX, yerror;
	int16 ybottom;
	int16 baseColor;
};

struct SpriteFrameEntry {
	int16 y, x, h, w;
	uint32 offset;
	SpriteFrameEntry() {
	}
	SpriteFrameEntry(byte *data) {
		y = READ_LE_UINT16(data + 0);
		x = READ_LE_UINT16(data + 2);
		h = READ_LE_UINT16(data + 4);
		w = READ_LE_UINT16(data + 6);
		offset = READ_LE_UINT32(data + 8);
	}
};

class Font {
public:
	Font(byte *fontData) : _fontData(fontData) {
	}
	~Font() {
	}
	int16 getSpacing() const {
		return _fontData[1];
	}
	int16 getHeight() const {
		return _fontData[2];
	}
	int16 getWidth() const {
		return _fontData[3];
	}
	int16 getCharWidth(byte ch) const {
		return _fontData[4 + (ch - 0x21)];
	}
	byte *getCharData(byte ch) const {
		return _fontData + 0x298 + READ_LE_UINT16(&_fontData[0xE0 + (ch - 0x21) * 2]);
	}
protected:
	byte *_fontData;
};

//*BEGIN*TEST*CODE********************************************************************************************

struct PixelPacket {
	byte count;
	byte pixel;
};

enum SpriteReaderStatus {
	kSrsPixelsLeft,
	kSrsEndOfLine,
	kSrsEndOfSprite
};

class SpriteFilter {
public:
	SpriteFilter(SpriteDrawItem *sprite) : _sprite(sprite) {
	}
	virtual SpriteReaderStatus readPacket(PixelPacket &packet) = 0;
protected:
	SpriteDrawItem *_sprite;
};

class SpriteReader : public SpriteFilter {
public:
	SpriteReader(byte *source, SpriteDrawItem *sprite) : SpriteFilter(sprite), _source(source) {
		_curWidth = _sprite->origWidth;
		_curHeight = _sprite->origHeight;
	}
	SpriteReaderStatus readPacket(PixelPacket &packet) {
		if (_sprite->flags & 0x40) {
			// shadow sprite
			packet.count = _source[0] & 0x7F;
			if (_source[0] & 0x80)
				packet.pixel = 1;
			else
				packet.pixel = 0;
			_source++;
		} else if (_sprite->flags & 0x10) {
			// 256-color sprite
			packet.pixel = *_source++;
			packet.count = *_source++;
		} else {
			// 16-color sprite
			packet.count = _source[0] & 0x0F;
			packet.pixel = (_source[0] & 0xF0) >> 4;
			_source++;
		}
		_curWidth -= packet.count;
		if (_curWidth <= 0) {
			_curHeight--;
			if (_curHeight == 0) {
				return kSrsEndOfSprite;
			} else {
				_curWidth = _sprite->origWidth;
				return kSrsEndOfLine;
			}
		} else {
			return kSrsPixelsLeft;
		}
	}
	byte *getSource() {
		return _source;
	}
	void setSource(byte *source) {
		_source = source;
		_curHeight++;
	}
protected:
	byte *_source;
	int16 _curWidth, _curHeight;
};

class SpriteFilterScaleDown : public SpriteFilter {
public:
	SpriteFilterScaleDown(SpriteDrawItem *sprite, SpriteReader *reader) : SpriteFilter(sprite), _reader(reader) {
		_height = _sprite->height;
		_yerror = _sprite->yerror;
		_origHeight = _sprite->origHeight;
		_scalerStatus = 0;
	}
	SpriteReaderStatus readPacket(PixelPacket &packet) {
		SpriteReaderStatus status;
		if (_scalerStatus == 0) {
			_xerror = _sprite->xdelta;
			_yerror -= 100;
			while (_yerror <= 0) {
				do {
					status = _reader->readPacket(packet);
				} while (status == kSrsPixelsLeft);
				_yerror += _sprite->ydelta - 100;
			}
			if (status == kSrsEndOfSprite)
				return kSrsEndOfSprite;
			_scalerStatus = 1;
		}
		if (_scalerStatus == 1) {
			status = _reader->readPacket(packet);
			byte updcount = packet.count;
			while (updcount--) {
				_xerror -= 100;
				if (_xerror <= 0) {
					if (packet.count > 0)
						packet.count--;
					_xerror += _sprite->xdelta;
				}
			}
			if (status == kSrsEndOfLine) {
				if (--_height == 0)
					return kSrsEndOfSprite;
				_scalerStatus = 0;
				return kSrsEndOfLine;
			}
		}
		return kSrsPixelsLeft;
	}
protected:
	SpriteReader *_reader;
	int16 _xerror, _yerror;
	int16 _height;
	int16 _origHeight;
	int _scalerStatus;
};

class SpriteFilterScaleUp : public SpriteFilter {
public:
	SpriteFilterScaleUp(SpriteDrawItem *sprite, SpriteReader *reader) : SpriteFilter(sprite), _reader(reader) {
		_height = _sprite->height;
		_yerror = _sprite->yerror;
		_origHeight = _sprite->origHeight;
		_scalerStatus = 0;
	}
	SpriteReaderStatus readPacket(PixelPacket &packet) {
		SpriteReaderStatus status;
		if (_scalerStatus == 0) {
			_xerror = _sprite->xdelta;
			_sourcep = _reader->getSource();
			_scalerStatus = 1;
		}
		if (_scalerStatus == 1) {
			status = _reader->readPacket(packet);
			byte updcount = packet.count;
			while (updcount--) {
				_xerror -= 100;
				if (_xerror <= 0) {
					packet.count++;
					_xerror += _sprite->xdelta;
				}
			}
			if (status == kSrsEndOfLine) {
				if (--_height == 0)
					return kSrsEndOfSprite;
				_yerror -= 100;
				if (_yerror <= 0) {
					_reader->setSource(_sourcep);
					_yerror += _sprite->ydelta + 100;
				}
				_scalerStatus = 0;
				return kSrsEndOfLine;
			}
		}
		return kSrsPixelsLeft;
	}
protected:
	SpriteReader *_reader;
	byte *_sourcep;
	int16 _xerror, _yerror;
	int16 _height;
	int16 _origHeight;
	int _scalerStatus;
};

//*END*TEST*CODE**********************************************************************************************

struct TextRect {
	int16 x, y;
	int16 width, length;
};

struct TalkTextItem {
	int16 duration;
	int16 slotIndex;
	int16 slotOffset;
	int16 fontNum;
	byte color;
	byte lineCount;
	TextRect lines[15];
};

class Screen {
public:
	Screen(PictureEngine *vm);
	~Screen();

	void unpackRle(byte *source, byte *dest, uint16 width, uint16 height);
	
	void loadMouseCursor(uint resIndex);
	
	void drawGuiImage(int16 x, int16 y, uint resIndex);
	
	void startShakeScreen(int16 shakeCounter);
	void stopShakeScreen();
	void updateShakeScreen();
	
	// Sprite list
	void addStaticSprite(byte *spriteItem);
	void addAnimatedSprite(int16 x, int16 y, int16 fragmentId, byte *data, int16 *spriteArray, bool loop, int mode);
	void clearSprites();

	// Sprite drawing
	void drawSprite(SpriteDrawItem *sprite);
	void drawSpriteCore(byte *dest, SpriteFilter &reader, SpriteDrawItem *sprite);
	void drawSprites();

	// Verb line
	void updateVerbLine(int16 slotIndex, int16 slotOffset);
	
	// Talk text
	void updateTalkText(int16 slotIndex, int16 slotOffset);
	void addTalkTextRect(Font &font, int16 x, int16 &y, int16 length, int16 width, TalkTextItem *item);
	void drawTalkTextItems();
	int16 getTalkTextDuration();
	void finishTextDrawItems();

	// Font/text
	void registerFont(uint fontIndex, uint resIndex);
	void printText(byte *textData);
	void preprocessText(uint fontResIndex, int maxWidth, int &width, byte *&sourceString, byte *&destString, byte &len);
	void drawStringEx(int16 x, int16 y, byte fontColor1, byte fontColor2, uint fontResIndex);

	int16 drawString(int16 x, int16 y, byte color, uint fontResIndex, byte *text, int len = -1, int16 *ywobble = NULL, bool outline = false);
	void drawChar(const Font &font, byte *dest, int16 x, int16 y, byte ch, byte color, bool outline);

	void saveState(Common::WriteStream *out);
	void loadState(Common::ReadStream *in);

//protected:
public:

	struct VerbLineItem {
		int16 slotIndex;
		int16 slotOffset;
	};
	
	struct Rect {
		int16 x, y, width, height;
	};

	PictureEngine *_vm;

	byte *_frontScreen, *_backScreen;
	
	Common::List<SpriteDrawItem> _spriteDrawList;

	uint _fontResIndexArray[10];
	byte _fontColor1, _fontColor2;

	// TODO: Remove this _tempXXX stuff
	byte _tempString[100];
	byte _tempStringLen1, _tempStringLen2;

	// Screen shaking
	bool _shakeActive;
	int16 _shakeCounterInit, _shakeCounter;
	int _shakePos;

	// Verb line
	int16 _verbLineNum;
	VerbLineItem _verbLineItems[8];
	int16 _verbLineX, _verbLineY, _verbLineWidth;
	int16 _verbLineCount;
	
	// Talk text
	int16 _talkTextX, _talkTextY;
	int16 _talkTextMaxWidth;
	byte _talkTextFontColor;
	int16 _talkTextItemNum;
	TalkTextItem _talkTextItems[5];

	void addDrawRequest(const DrawRequest &drawRequest);

};

} // End of namespace Picture

#endif /* PICTURE_SCREEN_H */
