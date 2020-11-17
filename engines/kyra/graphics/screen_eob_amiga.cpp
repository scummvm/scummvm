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


#ifdef ENABLE_EOB

#include "kyra/resource/resource.h"

#include "common/memstream.h"
#include "common/translation.h"

#include "gui/error.h"

namespace Kyra {

static uint32 _decodeFrameAmiga_x = 0;

bool decodeFrameAmiga_readNextBit(const uint8 *&data, uint32 &code, uint32 &chk) {
	_decodeFrameAmiga_x = code & 1;
	code >>= 1;
	if (code)
		return _decodeFrameAmiga_x;

	data -= 4;
	code = READ_BE_UINT32(data);
	chk ^= code;
	_decodeFrameAmiga_x = code & 1;
	code = (code >> 1) | (1 << 31);

	return _decodeFrameAmiga_x;
}

uint32 decodeFrameAmiga_readBits(const uint8 *&data, uint32 &code, uint32 &chk, int count) {
	uint32 res = 0;
	while (count--) {
		decodeFrameAmiga_readNextBit(data, code, chk);
		uint32 bt1 = _decodeFrameAmiga_x;
		_decodeFrameAmiga_x = res >> 31;
		res = (res << 1) | bt1;
	}
	return res;
}

void Screen_EoB::loadSpecialAmigaCPS(const char *fileName, int destPage, bool isGraphics) {
	uint32 fileSize = 0;
	const uint8 *file = _vm->resource()->fileData(fileName, &fileSize);

	if (!file)
		error("Screen_EoB::loadSpecialAmigaCPS(): Failed to load file '%s'", file);

	uint32 inSize = READ_BE_UINT32(file);
	const uint8 *pos = file;

	// Check whether the file starts with the actual compression header.
	// If this is not the case, there should a palette before the header.
	// Unlike normal CPS files these files never have more than one palette.
	if (((inSize + 15) & ~3) != ((fileSize + 3) & ~3)) {
		Common::MemoryReadStream in(pos, 64);
		_palettes[0]->loadAmigaPalette(in, 0, 32);
		pos += 64;
	}

	inSize = READ_BE_UINT32(pos);
	uint32 outSize = READ_BE_UINT32(pos + 4);
	uint32 chk = READ_BE_UINT32(pos + 8);

	pos = pos + 8 + inSize;
	uint8 *dstStart = _pagePtrs[destPage];
	uint8 *dst = dstStart + outSize;

	uint32 val = READ_BE_UINT32(pos);
	_decodeFrameAmiga_x = 0;
	chk ^= val;

	while (dst > dstStart) {
		int para = -1;
		int para2 = 0;

		if (decodeFrameAmiga_readNextBit(pos, val, chk)) {
			uint32 code = decodeFrameAmiga_readBits(pos, val, chk, 2);

			if (code == 3) {
				para = para2 = 8;
			} else {
				int cnt = 0;
				if (code < 2) {
					cnt = 3 + code;
					para2 = 9 + code;
				} else {
					cnt = decodeFrameAmiga_readBits(pos, val, chk, 8) + 1;
					para2 = 12;
				}

				code = decodeFrameAmiga_readBits(pos, val, chk, para2);
				while (cnt--) {
					dst--;
					*dst = dst[code & 0xFFFF];
				}
			}
		} else {
			if (decodeFrameAmiga_readNextBit(pos, val, chk)) {
				uint32 code = decodeFrameAmiga_readBits(pos, val, chk, 8);
				dst--;
				*dst = dst[code & 0xFFFF];
				dst--;
				*dst = dst[code & 0xFFFF];

			} else {
				para = 3;
			}
		}

		if (para > 0) {
			uint32 code = decodeFrameAmiga_readBits(pos, val, chk, para);
			uint32 cnt = (code & 0xFFFF) + para2 + 1;

			while (cnt--) {
				for (int i = 0; i < 8; ++i) {
					decodeFrameAmiga_readNextBit(pos, val, chk);
					uint32 bt1 = _decodeFrameAmiga_x;
					_decodeFrameAmiga_x = code >> 31;
					code = (code << 1) | bt1;
				}
				*(--dst) = code & 0xFF;
			}
		}
	}

	delete[] file;

	if (chk)
		error("Screen_EoB::loadSpecialAmigaCPS(): Checksum error");

	if (isGraphics)
		convertAmigaGfx(_pagePtrs[destPage], 320, 200);
}

void Screen_EoB::setDualPalettes(Palette &top, Palette &bottom) {
	// The original supports simultaneous fading of both palettes, but doesn't make any use of that
	// feature. The fade rate is always set to 0. So I see no need to implement that.
	_palettes[0]->copy(top, 0, 32, 0);
	_palettes[0]->copy(bottom, 0, 32, 32);
	setScreenPalette(*_palettes[0]);
	enableDualPaletteMode(120);
}

AmigaDOSFont::AmigaDOSFont(Resource *res, bool needsLocalizedFont) : _res(res), _needsLocalizedFont(needsLocalizedFont), _width(0), _height(0), _first(0), _last(0), _content(0), _numElements(0), _selectedElement(0), _maxPathLen(256) {
	assert(_res);
}

bool AmigaDOSFont::load(Common::SeekableReadStream &file) {
	unload();

	uint16 id = file.readUint16BE();
	// We only support type 0x0f00, since this is the only type used for EOB
	if (id != 0x0f00)
		return false;

	_numElements = file.readUint16BE();
	_content = new FontContent[_numElements];
	char *cfile = new char[_maxPathLen];

	for (int i = 0; i < _numElements; ++i) {
		file.read(cfile, _maxPathLen);
		_content[i].height = file.readUint16BE();
		_content[i].style = file.readByte();
		_content[i].flags = file.readByte();
		_content[i].contentFile = cfile;

		for (int ii = 0; ii < i; ++ii) {
			if (_content[ii].contentFile == _content[i].contentFile && _content[ii].data.get())
				_content[i].data = _content[ii].data;
		}

		if (!_content[i].data.get()) {
			TextFont *contentData = loadContentFile(cfile);
			if (contentData) {
				_content[i].data = Common::SharedPtr<TextFont>(contentData);
			} else {
				unload();
				return false;
			}
		}

		if (!(_content[i].flags & 0x40) && (_content[i].height != _content[i].data->height)) {
			warning("Amiga DOS Font construction / scaling not implemented.");
		}
	}

	delete[] cfile;

	selectMode(0);

	return true;
}

int AmigaDOSFont::getCharWidth(uint16 c) const {
	if (c < _first || c > _last)
		return 0;
	c -= _first;

	int width = _content[_selectedElement].data->spacing ? _content[_selectedElement].data->spacing[c] : _content[_selectedElement].data->width;

	/*if (_content[_selectedElement].data->kerning)
		width += _content[_selectedElement].data->kerning[c];*/

	return width;
}

void AmigaDOSFont::drawChar(uint16 c, byte *dst, int pitch, int) const {
	if (c < _first || c > _last || !dst)
		return;

	static const uint16 table[] = {
		0x8000, 0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00,
		0xff80, 0xffc0, 0xffe0, 0xfff0, 0xfff8, 0xfffc, 0xfffe, 0xffff
	};

	c -= _first;

	int w = _content[_selectedElement].data->spacing ? _content[_selectedElement].data->spacing[c] : _content[_selectedElement].data->width;
	int xbits = _content[_selectedElement].data->location[c * 2 + 1];
	int h = _content[_selectedElement].data->height;

	uint16 bitPos = _content[_selectedElement].data->location[c * 2] & 0x0F;
	uint16 mod = _content[_selectedElement].data->modulo;
	const uint8 *data = _content[_selectedElement].data->bitmap + ((_content[_selectedElement].data->location[c * 2] >> 3) & ~1);
	uint32 xbt_mask = xbits ? table[(xbits - 1) & 0x0F] << 16 : 0;

	for (int y = 0; y < h; ++y) {
		uint32 mask = 0x80000000;
		uint32 bits = (READ_BE_UINT32(data) << bitPos) & xbt_mask;
		data += mod;

		for (int x = 0; x < w; ++x) {
			if (bits & mask) {
				if (_colorMap[1])
					*dst = _colorMap[1];
			} else {
				if (_colorMap[0])
					*dst = _colorMap[0];
			}
			mask >>= 1;
			dst++;
		}
		dst += (pitch - w);
	}
}

uint8 AmigaDOSFont::_errorDialogDisplayed = 0;

void AmigaDOSFont::errorDialog(int index) {
	if (_errorDialogDisplayed & (1 << index))
		return;
	_errorDialogDisplayed |= (1 << index);

	// I've made rather elaborate dialogs here, since the Amiga font file handling is quite prone to cause problems for users.
	// This will hopefully prevent unnecessary forum posts and bug reports.
	if (index == 0) {
		::GUI::displayErrorDialog(_(
			"This AMIGA version requires the following font files:\n\nEOBF6.FONT\nEOBF6/6\nEOBF8.FONT\nEOBF8/8\n\n"
			"If you used the orginal installer for the installation these files\nshould be located in the AmigaDOS system 'Fonts/' folder.\n"
			"Please copy them into the EOB game data directory.\n"
		));

		error("Failed to load font files.");
	} else if (index == 1) {
		::GUI::displayErrorDialog(_(
			"This AMIGA version requires the following font files:\n\nEOBF6.FONT\nEOBF6/6\nEOBF8.FONT\nEOBF8/8\n\n"
			"This is a localized (non-English) version of EOB II which uses language specific characters\n"
			"contained only in the specific font files that came with your game. You cannot use the font\n"
			"files from the English version or from any EOB I game which seems to be what you are doing.\n\n"
			"The game will continue, but the language specific characters will not be displayed.\n"
			"Please copy the correct font files into your EOB II game data directory.\n\n"
		));
	}
}

void AmigaDOSFont::unload() {
	delete[] _content;
}

AmigaDOSFont::TextFont *AmigaDOSFont::loadContentFile(const Common::String fileName) {
	Common::SeekableReadStreamEndian *str = _res->createEndianAwareReadStream(fileName);

	if (!str && fileName.contains('/')) {
		// These content files are usually located in sub directories (i. e. the eobf8.font
		// has a sub dir named 'eobf8' with a file '8' in it). In case someone put the content
		// files directly in the game directory we still try to open it.
		Common::String fileNameAlt = fileName;
		while (fileNameAlt.firstChar() != '/')
			fileNameAlt.deleteChar(0);
		fileNameAlt.deleteChar(0);

		str = _res->createEndianAwareReadStream(fileNameAlt);

		if (!str) {
			// Someone might even have copied the floppy disks to the game directory with the
			// full sub directory structure. So we also try that...
			fileNameAlt = "fonts/";
			fileNameAlt += fileName;

			str = _res->createEndianAwareReadStream(fileNameAlt);
		}

		if (!str)
			errorDialog(0);
	}

	uint32 hunkId = str->readUint32();
	// Except for some sanity checks we skip all of the Amiga hunk file magic
	if (hunkId != 0x03f3)
		return 0;
	str->seek(20, SEEK_CUR);

	uint32 hunkType = str->readUint32();
	if (hunkType != 0x3E9)
		return 0;
	uint32 dataSize = str->readUint32() * 4;
	int32 hunkStartPos = str->pos();

	str->seek(34, SEEK_CUR);
	TextFont *fnt = new TextFont();
	int32 fntStartPos = str->pos();
	str->seek(44, SEEK_CUR);
	fnt->height = str->readUint16();
	str->seek(2, SEEK_CUR);
	fnt->width = str->readUint16();
	fnt->baseLine = str->readUint16();
	str->seek(4, SEEK_CUR);
	fnt->firstChar = str->readByte();
	fnt->lastChar = str->readByte();

	if (_needsLocalizedFont && fnt->lastChar <= 127)
		errorDialog(1);

	str->seek(18, SEEK_CUR);
	int32 curPos = str->pos();
	uint32 bufferSize = dataSize - (curPos - fntStartPos);
	uint8 *buffer = new uint8[bufferSize];
	str->read(buffer, bufferSize);

	str->seek(curPos - 18, SEEK_SET);
	uint32 offset = str->readUint32();
	fnt->bitmap = offset ? buffer + offset - (curPos - hunkStartPos) : 0;
	fnt->modulo = str->readUint16();

	offset = str->readUint32();
	uint16 *loc = (uint16*)(offset ? buffer + offset - (curPos - hunkStartPos) : 0);
	for (int i = 0; i <= (fnt->lastChar - fnt->firstChar) * 2 + 1; ++i)
		loc[i] = READ_BE_UINT16(&loc[i]);
	fnt->location = loc;

	offset = str->readUint32();
	int16 *idat = offset ? (int16*)(buffer + offset - (curPos - hunkStartPos)) : 0;
	if (idat) {
		for (int i = 0; i <= (fnt->lastChar - fnt->firstChar) * 2 + 1; ++i)
			idat[i] = (int16)READ_BE_UINT16(&idat[i]);
	}
	fnt->spacing = idat;

	offset = str->readUint32();
	// This warning will only show up if someone tries to use this code elsewhere. It cannot happen with EOB fonts.
	if (offset)
		warning("Trying to load an AmigaDOS font with kerning data. This is not implemented. Font Rendering will not be accurate.");
	idat = offset ? (int16*)(buffer + offset - (curPos - hunkStartPos)) : 0;
	if (idat) {
		for (int i = 0; i <= (fnt->lastChar - fnt->firstChar) * 2 + 1; ++i)
			idat[i] = (int16)READ_BE_UINT16(&idat[i]);
	}
	fnt->kerning = idat;

	fnt->data = buffer;

	delete str;

	return fnt;
}

void AmigaDOSFont::selectMode(int mode) {
	if (mode < 0 || mode > _numElements - 1)
		return;

	_selectedElement = mode;

	_width = _content[mode].data->width;
	_height = _content[mode].data->height;
	_first = _content[mode].data->firstChar;
	_last = _content[mode].data->lastChar;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
