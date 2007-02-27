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

extern byte _palette[];

#define NUM_BUFFERS 6

class Parallaction;

class Graphics {

public:
	enum Buffers {
		// bit buffers
		kBitFront,
		kBitBack,
		kBit2,
		kBit3,
		// mask buffers
		kMask0,
		// path buffers
		kPath0
	};

public:

	// dialogue and text
	void drawBalloon(int16 left, int16 top, uint16 width, uint16 height, uint16 arg_8);
	void displayBalloonString(uint16 x, uint16 y, const char *text, byte color);
	void displayString(uint16 x, uint16 y, const char *text);
	bool displayWrappedString(char *text, uint16 x, uint16 y, uint16 maxwidth, byte color);
	uint16 getStringWidth(const char *text);
	void getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height);

	// cnv management
	void makeCnvFromString(StaticCnv *cnv, char *text);
	void loadExternalCnv(const char *filename, Cnv *cnv);
	void loadExternalStaticCnv(const char *filename, StaticCnv *cnv);
	void loadCnv(const char *filename, Cnv *cnv);
	void loadStaticCnv(const char *filename, StaticCnv *cnv);
	void freeCnv(Cnv *cnv);
	void freeStaticCnv(StaticCnv *cnv);
	void backupCnvBackground(StaticCnv *cnv, int16 x, int16 y);
	void backupCnvBackgroundTransparent(StaticCnv *cnv, int16 x, int16 y);
	void restoreCnvBackground(StaticCnv *cnv, int16 x, int16 y);

	// location
	void loadBackground(const char *filename, Graphics::Buffers buffer);
	void loadMaskAndPath(const char *filename);
	uint16 queryPath(uint16 x, uint16 y);
	int16 queryMask(int16 v);
	void intGrottaHackMask();
	void restoreBackground(int16 left, int16 top, uint16 width, uint16 height);
	void backupBackgroundMask(Graphics::Buffers mask);
	void restoreBackgroundMask(Graphics::Buffers mask);
	void backupBackgroundPath(Graphics::Buffers path);
	void restoreBackgroundPath(Graphics::Buffers path);

	// intro
	void maskClearRectangle(uint16 left, uint16 top, uint16 right, uint16 bottom, Graphics::Buffers mask);
	void maskOpNot(uint16 x, uint16 y, uint16 unused, Graphics::Buffers mask);

	// low level
	void swapBuffers();
	void updateScreen();
	void clearScreen(Graphics::Buffers buffer);
	void copyScreen(Graphics::Buffers srcbuffer, Graphics::Buffers dstbuffer);
	void copyRect(Graphics::Buffers srcbuffer, uint16 sx, uint16 sy, Graphics::Buffers dstbuffer, uint16 dx, uint16 dy, uint16 w, uint16 h);
	void copyRect(Graphics::Buffers dstbuffer, uint16 x, uint16 y, uint16 w, uint16 h, byte *src, uint16 pitch);
	void grabRect(Graphics::Buffers srcbuffer, byte *dst, uint16 x, uint16 y, uint16 w, uint16 h, uint16 pitch);
	void drawBorder(Graphics::Buffers buffer, uint16 x, uint16 y, uint16 w, uint16 h, byte color);
	void floodFill(byte color, uint16 left, uint16 top, uint16 right, uint16 bottom, Graphics::Buffers buffer);
	void flatBlitCnv(StaticCnv *cnv, int16 x, int16 y, Graphics::Buffers buffer, byte *unused);
	void blitCnv(StaticCnv *cnv, int16 x, int16 y, uint16 z, Graphics::Buffers buffer, Graphics::Buffers mask);

	// palette
	void animatePalette(byte *palette);
	void setPalette(byte *palette);
	void getBlackPalette(byte *palette);
	void palUnk0(byte *palette);
	void buildBWPalette(byte *palette);
	void quickFadePalette(byte *palette);
	void fadePalette(byte *palette);

	// init
	Graphics(Parallaction* vm);
	virtual ~Graphics();

	void setMousePointer(int16 index);

	void setFont(const char* name);

public:
	static Point		_labelPosition[2];
	static bool 		_proportionalFont;

protected:
	Parallaction*		_vm;

	static byte *		_buffers[NUM_BUFFERS];

	static byte			_mouseArrow[256];
	static StaticCnv	_mouseComposedArrow;

	static Cnv			_font;


protected:
	//
	//	decompress a graphics block (size is *target* size)
	//
	//	returns amount of byte read
	//
	uint16 decompressChunk(byte *src, byte *dst, uint16 size);

	//
	//	maps a character for representation
	//
	byte mapChar(byte c);

	void flatBlit(uint16 w, uint16 h, int16 x, int16 y, byte *data, Graphics::Buffers buffer);
	void blit(uint16 w, uint16 h, int16 x, int16 y, uint16 z, byte *data, Graphics::Buffers buffer, Graphics::Buffers mask);


	void initBuffers();
	void initMouse(uint16 arg_0);


};


} // Parallaction


#endif

