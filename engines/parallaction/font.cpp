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

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/stream.h"

#include "parallaction/parallaction.h"

namespace Parallaction {


extern byte _amigaTopazFont[];

class DosFont : public Font {

protected:
	// drawing properties
	byte		*_cp;

	Cnv			*_data;
	byte		_pitch;
	uint32		_bufPitch;

protected:
	virtual uint16 drawChar(char c) = 0;
	virtual uint16 width(byte c) = 0;
	virtual uint16 height() = 0;

	byte mapChar(byte c) {
		if (c == 0xA5) return 0x5F;
		if (c == 0xDF) return 0x60;

		if (c > 0x7F) return c - 0x7F;

		return c - 0x20;
	}

public:
	DosFont(Cnv *cnv) : _data(cnv), _pitch(cnv->_width) {
	}

	~DosFont() {
		if (_data)
			delete _data;
	}

	void setData() {

	}

	uint32 getStringWidth(const char *s) {
		uint32 len = 0;

		while (*s) {
			byte c = mapChar(*s);
			len += width(c);
			s++;
		}

		return len;
	}

	void drawString(byte* buffer, uint32 pitch, const char *s) {
		if (s == NULL)
			return;

		_bufPitch = pitch;

		_cp = buffer;
		while (*s) {
			byte c = mapChar(*s);
			_cp += drawChar(c);
			s++;
		}
	}
};

class DosDialogueFont : public DosFont {

private:
	static const byte 	_glyphWidths[126];

protected:
	uint16 width(byte c) {
		return _glyphWidths[c];
	}

	uint16 height() {
		return _data->_height;
	}

public:
	DosDialogueFont(Cnv *cnv) : DosFont(cnv) {
	}

protected:
	uint16 drawChar(char c) {

		byte *src = _data->getFramePtr(c);
		byte *dst = _cp;
		uint16 w = width(c);

		for (uint16 j = 0; j < height(); j++) {
			for (uint16 k = 0; k < w; k++) {

				if (!*src)
					*dst = _color;

				dst++;
				src++;
			}

			src += (_pitch - w);
			dst += (_bufPitch - w);
		}

		return w;

	}

};

const byte DosDialogueFont::_glyphWidths[126] = {
  0x04, 0x03, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x04, 0x04, 0x06, 0x06, 0x03, 0x05, 0x03, 0x05,
  0x06, 0x06, 0x06, 0x06, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x03, 0x03, 0x05, 0x04, 0x05, 0x05,
  0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x07, 0x05, 0x06, 0x05, 0x08, 0x07,
  0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x03, 0x04, 0x05, 0x05, 0x06, 0x06, 0x05,
  0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x06, 0x07, 0x05, 0x05, 0x05, 0x05, 0x02, 0x05, 0x05, 0x07,
  0x08, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04,
  0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x04, 0x06, 0x05, 0x05, 0x05, 0x05
};


class DosMonospacedFont : public DosFont {

protected:
	uint16	_width;

protected:
	uint16 width(byte c) {
		return _width;
	}

	uint16 height() {
		return _data->_height;
	}


public:
	DosMonospacedFont(Cnv *cnv) : DosFont(cnv) {
		_width = 8;
	}

};

class DosMenuFont : public DosMonospacedFont {

public:
	DosMenuFont(Cnv *cnv) : DosMonospacedFont(cnv) {
	}

protected:
	uint16 drawChar(char c) {

		byte *src = _data->getFramePtr(c);
		byte *dst = _cp;

		for (uint16 i = 0; i < height(); i++) {
			for (uint16 j = 0; j < _width; j++) {
				if (*src)
					*dst = *src;
				src++;
				dst++;
			}

			dst += (_bufPitch - _width);
			src += (_pitch - _width);
		}

		return _width;
	}

};


class DosLabelFont : public DosMonospacedFont {

public:
	DosLabelFont(Cnv *cnv) : DosMonospacedFont(cnv) {
	}

protected:
	uint16 drawChar(char c) {

		byte *src = _data->getFramePtr(c);
		byte *dst = _cp;

		for (uint16 i = 0; i < height(); i++) {
			memcpy(dst, src, _width);
			dst += _bufPitch;
			src += _pitch;
		}

		return _width;
	}

};

class AmigaFont : public Font {

#include "common/pack-start.h"
	struct CharLoc {
		uint16	_offset;
		uint16	_length;
	};

	struct AmigaDiskFont {
		uint16	_ySize;
		byte	_style;
		byte	_flags;
		uint16	_xSize;
		uint16	_baseline;
		uint16	_boldSmear;
		uint16	_accessors;	// unused
		byte	_loChar;
		byte	_hiChar;
		uint32	_charData;
		uint16	_modulo;
		uint32	_charLoc;
		uint32	_charSpace;
		uint32	_charKern;
	};
#include "common/pack-end.h"

	AmigaDiskFont	*_font;
	uint32		_dataSize;
	byte		*_data;
	byte		*_charData;
	CharLoc		*_charLoc;
	uint16		*_charSpace;
	uint16		*_charKern;

	byte			*_cp;
	uint32		_pitch;

protected:
	uint16 getSpacing(byte c);
	void blitData(byte c);
	uint16 getKerning(byte c);
	uint16 getPixels(byte c);
	uint16 getOffset(byte c);
	uint16 width(byte c);
	uint16 height();

	byte	mapChar(byte c);

public:
	AmigaFont(Common::SeekableReadStream &stream);
	~AmigaFont();

	uint32 getStringWidth(const char *s);
	void drawString(byte *buf, uint32 pitch, const char *s);



};

AmigaFont::AmigaFont(Common::SeekableReadStream &stream) {
	stream.seek(32);	// skips dummy header

	_dataSize = stream.size() - stream.pos();
	_data = (byte*)malloc(_dataSize);
	stream.read(_data, _dataSize);

	_font = (AmigaDiskFont*)(_data + 78);
	_font->_ySize = FROM_BE_16(_font->_ySize);
	_font->_xSize = FROM_BE_16(_font->_xSize);
	_font->_baseline = FROM_BE_16(_font->_baseline);
	_font->_modulo = FROM_BE_16(_font->_modulo);

	_charLoc = (CharLoc*)(_data + FROM_BE_32(_font->_charLoc));
	_charData = _data + FROM_BE_32(_font->_charData);

	_charSpace = 0;
	_charKern = 0;

	if (_font->_charSpace != 0)
		_charSpace = (uint16*)(_data + FROM_BE_32(_font->_charSpace));
	if (_font->_charKern != 0)
		_charKern = (uint16*)(_data + FROM_BE_32(_font->_charKern));

}

AmigaFont::~AmigaFont() {
	if (_data)
		free(_data);
}

uint16 AmigaFont::getSpacing(byte c) {
	return (_charSpace == 0) ? _font->_xSize : FROM_BE_16(_charSpace[c]);
}

uint16 AmigaFont::getKerning(byte c) {
	return (_charKern == 0) ? 0 : FROM_BE_16(_charKern[c]);
}

uint16 AmigaFont::getPixels(byte c) {
	return FROM_BE_16(_charLoc[c]._length);
}

uint16 AmigaFont::getOffset(byte c) {
	return FROM_BE_16(_charLoc[c]._offset);
}

void AmigaFont::blitData(byte c) {

	int num = getPixels(c);
	int bitOffset = getOffset(c);

	byte *d = _cp;
	byte *s = _charData;

	for (int i = 0; i < _font->_ySize; i++) {

		for (int j = bitOffset; j < bitOffset + num; j++) {
			byte *b = s + (j >> 3);
			byte bit = *b & (0x80 >> (j & 7));

			if (bit)
				*d = _color;

			d++;
		}

		s += _font->_modulo;
		d += _pitch - num;
	}

}

uint16 AmigaFont::width(byte c) {
//	printf("kern(%i) = %i, space(%i) = %i\t", c, getKerning(c), c, getSpacing(c));
	return getKerning(c) + getSpacing(c);
}

uint16 AmigaFont::height() {
	return _font->_ySize;
}

byte AmigaFont::mapChar(byte c) {

	if (c < _font->_loChar || c > _font->_hiChar)
		error("character '%c (%x)' not supported by font", c, c);

	return c - _font->_loChar;
}

uint32 AmigaFont::getStringWidth(const char *s) {
	uint32 len = 0;

	while (*s) {
		byte c = mapChar(*s);
		len += width(c);
		s++;
	}

	return len;
}

void AmigaFont::drawString(byte *buffer, uint32 pitch, const char *s) {

	_cp = buffer;
	_pitch = pitch;

	byte c;

	while (*s) {
		c = mapChar(*s);
		_cp += getKerning(c);
		blitData(c);
		_cp += getSpacing(c);
		s++;
	}

}

Font *DosDisk_ns::createFont(const char *name, Cnv* cnv) {
	Font *f = 0;

	if (!scumm_stricmp(name, "comic"))
		f = new DosDialogueFont(cnv);
	else
	if (!scumm_stricmp(name, "topaz"))
		f = new DosLabelFont(cnv);
	else
	if (!scumm_stricmp(name, "slide"))
		f = new DosMenuFont(cnv);
	else
		error("unknown dos font '%s'", name);

	return f;
}

Font *AmigaDisk_ns::createFont(const char *name, Common::SeekableReadStream &stream) {
	// TODO: implement AmigaLabelFont for labels
	return new AmigaFont(stream);
}

void Gfx::initFonts() {

	if (_vm->getPlatform() == Common::kPlatformPC) {
		_fonts[kFontDialogue] = _vm->_disk->loadFont("comic");
		_fonts[kFontLabel] = _vm->_disk->loadFont("topaz");
		_fonts[kFontMenu] = _vm->_disk->loadFont("slide");
	} else {
		_fonts[kFontDialogue] = _vm->_disk->loadFont("comic");

		Common::MemoryReadStream stream(_amigaTopazFont, 2600, false);
		_fonts[kFontLabel] = new AmigaFont(stream);

		_fonts[kFontMenu] = _vm->_disk->loadFont("slide");
	}

}

}
