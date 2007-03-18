/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "parallaction/defs.h"
#include "common/stream.h"


namespace Parallaction {



#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define SCREEN_SIZE 		SCREEN_WIDTH*SCREEN_HEIGHT

#define SCREENMASK_WIDTH	SCREEN_WIDTH/4
#define SCREENPATH_WIDTH	SCREEN_WIDTH/8

#define PALETTE_COLORS		32
#define PALETTE_SIZE		PALETTE_COLORS*3

#include "common/pack-start.h"	// START STRUCT PACKING

struct PaletteFxRange {

	uint16	_timer;
	uint16	_step;
	uint16	_flags;
	byte	_first;
	byte	_last;

};

#include "common/pack-end.h"	// END STRUCT PACKING

#define NUM_BUFFERS 6

class Parallaction;

struct DoorData;
struct GetData;

class Gfx {

public:
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
	void displayBalloonString(uint16 x, uint16 y, const char *text, byte color);
	void displayString(uint16 x, uint16 y, const char *text);
	bool displayWrappedString(char *text, uint16 x, uint16 y, uint16 maxwidth, byte color);
	uint16 getStringWidth(const char *text);
	void getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height);

	// cnv management
	void makeCnvFromString(StaticCnv *cnv, char *text);
	void freeCnv(Cnv *cnv);
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
	void maskClearRectangle(const Common::Rect& r);
	void maskOpNot(uint16 x, uint16 y, uint16 unused);

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

	// palette
	void animatePalette(byte *palette);
	void setPalette(byte *palette);
	void getBlackPalette(byte *palette);
	void palUnk0(byte *palette);
	void buildBWPalette(byte *palette);
	void quickFadePalette(byte *palette);
	void fadePalette(byte *palette);

	// init
	Gfx(Parallaction* vm);
	virtual ~Gfx();

	void setMousePointer(int16 index);

	void setFont(const char* name);

public:
	Common::Point		_labelPosition[2];
	static bool 		_proportionalFont;

	uint16				_bgLayers[4];
	PaletteFxRange		_palettefx[6];
	byte				_palette[PALETTE_SIZE];

protected:
	Parallaction*		_vm;

	static byte *		_buffers[NUM_BUFFERS];

	static byte			_mouseArrow[256];
	StaticCnv			*_mouseComposedArrow;

	Cnv					*_font;


protected:

	//
	//	maps a character for representation
	//
	byte mapChar(byte c);

	void flatBlit(const Common::Rect& r, byte *data, Gfx::Buffers buffer);
	void blit(const Common::Rect& r, uint16 z, byte *data, Gfx::Buffers buffer);


	void initBuffers();
	void initMouse(uint16 arg_0);


};


} // Parallaction


#endif

