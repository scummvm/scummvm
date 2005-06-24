/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/list.h"
#include "common/map.h"

#include "kyra/kyra.h"

namespace Kyra {

// standard Package format for Kyrandia games
class PAKFile {
	struct PakChunk {
		const char* _name;
		const uint8* _data;
		uint32 _size;
	};

public:

	PAKFile(const Common::String& file);
	~PAKFile();

	const uint8* getFile(const char* file);
	uint32 getFileSize(const char* file);

	bool isValid(void) {return (_buffer != 0);}
	bool isOpen(void) {return _open;}
private:
	bool _open;
	uint8* _buffer; // the whole file
	Common::List<PakChunk*> _files; // the entries
};

// some resource types
class Palette;
class CPSImage;
class Font;
class Movie;
class VMContext;

// out resource manager
class Resourcemanager {
	typedef Common::String string;

public:

	Resourcemanager(KyraEngine* engine);
	virtual ~Resourcemanager();

	uint8* fileData(const char* file, uint32* size);

	Palette* loadPalette(const char* file);
	CPSImage* loadImage(const char* file);
	Font* loadFont(const char* file);
	Movie* loadMovie(const char* file);
	VMContext* loadScript(const char* file);

protected:
	KyraEngine* _engine;

	Common::List<PAKFile*> _pakfiles;

};

class Palette {

public:
		
	Palette(uint8* data, uint32 size);
	~Palette() { delete [] _palette; }

	uint8* getData(void) { return _palette; }

protected:

	uint8* _palette;

};

class CPSImage {

public:

	CPSImage(uint8* buffer, uint32 size);
	~CPSImage();

	Palette* palette(void) { return _ownPalette; }
	bool hasPalette(void) { return (_ownPalette != 0); }

	// if col == -1 then no transparany
	void transparency(int16 col) { _transparency = col; }

	void drawToPlane(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 x, uint16 y);
	void drawToPlane(uint8* plane, uint16 planepitch, uint16 planeheight, uint16 x, uint16 y,
			uint16 srcx, uint16 srcy, uint16 srcwidth, uint16 srcheight);

	// only for testing :)
	uint8 getColor(uint16 x, uint16 y) { return _image[y * _width + x]; }
		
	uint8& operator[](uint16 index) { if (index > _width * _height) return _image[0]; return _image[index]; }

protected:

	struct CPSHeader {
		uint16 _filesize;
		uint16 _format;
		uint16 _imagesize;
		uint32 _pal;
	} _cpsHeader;

	Palette* _ownPalette;
	uint8* _image;

	uint16 _width, _height;
	int16 _transparency;
};

class Font {

public:

	Font(uint8* buffer, uint32 size);
	~Font();

	uint32 getStringWidth(const char* string, char terminator = '\0');
	void drawStringToPlane(const char* string,
				uint8* plane, uint16 planewidth, uint16 planeheight,
				uint16 x, uint16 y, uint8 color);

protected:

	void drawCharToPlane(const uint8* c, uint8 color, uint8 width, uint8 height,
				uint8* plane, uint16 planewidth, uint16 planeheight, uint16 x, uint16 y);
	const uint8* getChar(char c, uint8* width, uint8* height, uint8* heightadd);
	const char* getNextWord(const char* string, uint32* size);

	void preRenderAllChars(uint16 offsetTableOffset);

	uint8* _buffer;
	uint16* _offsetTable;
	uint8* _charWidth;
	uint16* _charHeight;
	uint8* _charBits;

	// the chars I call 'prerendered' aren't really prerendered
	// they are only 'decoded'
	struct PreRenderedChar {
		uint8* c;
		uint8 width, height, heightadd;
	};

	Common::Map<uint8, PreRenderedChar> _preRenderedChars; // our prerendered chars :)

	// INFO:
	// _magic1 = 0x0500
	// _magic2 = 0x000e
	// _magic3 = 0x0014
#pragma START_PACK_STRUCTS
	struct FontHeader {
		uint16 _size;
		uint16 _magic1, _magic2, _magic3;
		uint16 _charWidthOffset, _charBitsOffset, _charHeightOffset;
		uint16 _version;
		uint16 _countChars;
		uint8 _width, _height;
	} GCC_PACK _fontHeader;
#pragma END_PACK_STRUCTS
};
} // end of namespace Kyra

#endif
