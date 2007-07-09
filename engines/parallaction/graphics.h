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

#ifndef PARALLACTION_GRAPHICS_H
#define PARALLACTION_GRAPHICS_H

#include "common/rect.h"
#include "common/stream.h"


#include "parallaction/defs.h"


namespace Parallaction {



#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define SCREEN_SIZE 		SCREEN_WIDTH*SCREEN_HEIGHT

#define SCREENMASK_WIDTH	SCREEN_WIDTH/4
#define SCREENPATH_WIDTH	SCREEN_WIDTH/8

#define BASE_PALETTE_COLORS		32
#define FIRST_BASE_COLOR		0
#define LAST_BASE_COLOR			(FIRST_BASE_COLOR+BASE_PALETTE_COLORS-1)

#define EHB_PALETTE_COLORS		32										// extra half-brite colors for amiga
#define FIRST_EHB_COLOR			(LAST_BASE_COLOR+1)
#define LAST_EHB_COLOR			(FIRST_EHB_COLOR+EHB_PALETTE_COLORS-1)

#define PALETTE_COLORS			(BASE_PALETTE_COLORS+EHB_PALETTE_COLORS)

#define BASE_PALETTE_SIZE		BASE_PALETTE_COLORS*3
#define PALETTE_SIZE			PALETTE_COLORS*3

#define MOUSEARROW_WIDTH		16
#define MOUSEARROW_HEIGHT		16

#define MOUSECOMBO_WIDTH		32	// sizes for cursor + selected inventory item
#define MOUSECOMBO_HEIGHT		32

#include "common/pack-start.h"	// START STRUCT PACKING

struct PaletteFxRange {

	uint16	_timer;
	uint16	_step;
	uint16	_flags;
	byte	_first;
	byte	_last;

} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

class Font {

protected:
	byte _color;


public:
	Font() {}
	virtual ~Font() {}

	virtual void setColor(byte color) {
		_color = color;
	}
	virtual uint32 getStringWidth(const char *s) = 0;
	virtual uint16 height() = 0;

	virtual void drawString(byte* buffer, uint32 pitch, const char *s) = 0;


};

struct StaticCnv {
	uint16	_width; 	//
	uint16	_height;	//
	byte*	_data0; 	// bitmap
	byte*	_data1; 	// unused

	StaticCnv() {
		_width = _height = 0;
		_data0 = _data1 = NULL;
	}
};

struct Cnv {
	uint16	_count; 	// # of frames
	uint16	_width; 	//
	uint16	_height;	//
	byte**	field_8;	// unused
	byte*	_data;

public:
	Cnv() {
		_width = _height = _count = 0;
		_data = NULL;
	}

	Cnv(uint16 numFrames, uint16 width, uint16 height, byte* data) : _count(numFrames), _width(width), _height(height), _data(data) {

	}

	~Cnv() {
		if (_count == 0 || _data == NULL) return;
		free(_data);
	}

	byte* getFramePtr(uint16 index) {
		if (index >= _count)
			return NULL;
		return &_data[index * _width * _height];
	}
};


#define NUM_BUFFERS 4

class Parallaction;

struct DoorData;
struct GetData;

enum Fonts {
	kFontDialogue = 0,
	kFontLabel = 1,
	kFontMenu = 2
};

class Gfx {

public:
	typedef byte Palette[PALETTE_SIZE];

	enum Buffers {
		// bit buffers
		kBitFront,
		kBitBack,
		kBit2,
		// mask buffers
		kMask0
	};

public:

	// dialogue and text
	void drawBalloon(const Common::Rect& r, uint16 arg_8);
	void displayString(uint16 x, uint16 y, const char *text, byte color);
	void displayCenteredString(uint16 y, const char *text);
	bool displayWrappedString(char *text, uint16 x, uint16 y, byte color, uint16 wrapwidth = SCREEN_WIDTH);
	uint16 getStringWidth(const char *text);
	void getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height);

	// cnv management
	void makeCnvFromString(StaticCnv *cnv, char *text);
	void freeStaticCnv(StaticCnv *cnv);
	void backupDoorBackground(DoorData *data, int16 x, int16 y);
	void backupGetBackground(GetData *data, int16 x, int16 y);
	void restoreZoneBackground(const Common::Rect& r, byte *data);

	// location
	void setBackground(byte *background);
	void setMask(byte *mask);
	int16 queryMask(int16 v);
	void intGrottaHackMask();
	void restoreBackground(const Common::Rect& r);

	// intro
	void fillMaskRect(const Common::Rect& r, byte color);
	void plotMaskPixel(uint16 x, uint16 y, byte color);

	// low level
	void swapBuffers();
	void updateScreen();
	void clearScreen(Gfx::Buffers buffer);
	void copyScreen(Gfx::Buffers srcbuffer, Gfx::Buffers dstbuffer);
	void copyRect(Gfx::Buffers dstbuffer, const Common::Rect& r, byte *src, uint16 pitch);
	void grabRect(byte *dst, const Common::Rect& r, Gfx::Buffers srcbuffer, uint16 pitch);
	void floodFill(Gfx::Buffers buffer, const Common::Rect& r, byte color);

	// NOTE: flatBlitCnv used to have an additional unused parameter,
	// that was always the _data1 member of the StaticCnv parameter.
	// DOS version didn't make use of it, but it is probably needed for Amiga stuff.
	void flatBlitCnv(StaticCnv *cnv, int16 x, int16 y, Gfx::Buffers buffer);
	void blitCnv(StaticCnv *cnv, int16 x, int16 y, uint16 z, Gfx::Buffers buffer);
	void flatBlitCnv(Cnv *cnv, uint16 frame, int16 x, int16 y, Gfx::Buffers buffer);


	// palette
	void setPalette(Palette palette, uint32 first = FIRST_BASE_COLOR, uint32 num = BASE_PALETTE_COLORS);
	void setBlackPalette();
	void animatePalette();
	void fadePalette(Palette palette);
	void buildBWPalette(Palette palette);
	void quickFadePalette(Palette palette);

	// amiga specific
	void setHalfbriteMode(bool enable);

	// init
	Gfx(Parallaction* vm);
	virtual ~Gfx();

	void setMousePointer(int16 index);

	void initFonts();
	void setFont(Fonts name);

public:
	Common::Point		_labelPosition[2];
	uint16				_bgLayers[4];
	PaletteFxRange		_palettefx[6];
	Palette				_palette;

protected:
	Parallaction*		_vm;
	static byte *		_buffers[NUM_BUFFERS];
	static byte			_mouseArrow[256];
	StaticCnv			*_mouseComposedArrow;
	Font				*_font;
	Font				*_fonts[3];
	bool				_halfbrite;

protected:
	byte mapChar(byte c);
	void flatBlit(const Common::Rect& r, byte *data, Gfx::Buffers buffer);
	void blit(const Common::Rect& r, uint16 z, byte *data, Gfx::Buffers buffer);
	void initBuffers();
	void initMouse(uint16 arg_0);
};


} // Parallaction


#endif

