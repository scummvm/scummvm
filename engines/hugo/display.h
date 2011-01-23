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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_DISPLAY_H
#define HUGO_DISPLAY_H

namespace Hugo {
enum overlayState_t {UNDEF, FG, BG};                // Overlay state

static const int kShapeSize = 24;
static const int kFontLength = 128;                 // Number of chars in font
static const int kFontSize = 1200;                  // Max size of font data
static const int kNumFonts = 3;                     // Number of dib fonts
static const int kCenter = -1;                      // Used to center text in x

struct rect_t {                                     // Rectangle used in Display list
	int16 x;                                        // Position in dib
	int16 y;                                        // Position in dib
	int16 dx;                                       // width
	int16 dy;                                       // height
};

/**
 * A black and white Windows-style arrow cursor (12x20).
 * 0 = Black (#000000 in 24-bit RGB).
 * 1 = Transparent.
 * 15 = White (#FFFFFF in 24-bit RGB).
 * This cursor comes from Mohawk engine.
 */

static const byte stdMouseCursor[] = {
	0, 0,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 0,  1,  1,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 0,  1,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 0,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 0,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 0,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 0,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 15, 0,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 15, 15, 0,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 1,
	0, 15, 15, 15, 15, 15, 15, 0,  0,  0,  0, 0,
	0, 15, 15, 15, 0,  15, 15, 0,  1,  1,  1, 1,
	0, 15, 15, 0,  0,  15, 15, 0,  1,  1,  1, 1,
	0, 15, 0,  1,  0,  0,  15, 15, 0,  1,  1, 1,
	0, 0,  1,  1,  1,  0,  15, 15, 0,  1,  1, 1,
	0, 1,  1,  1,  1,  1,  0,  15, 15, 0,  1, 1,
	1, 1,  1,  1,  1,  1,  0,  15, 15, 0,  1, 1,
	1, 1,  1,  1,  1,  1,  1,  0,  15, 15, 0, 1,
	1, 1,  1,  1,  1,  1,  1,  0,  15, 15, 0, 1,
	1, 1,  1,  1,  1,  1,  1,  1,  0,  0,  1, 1
};
static const byte stdMouseCursorHeight = 20;
static const byte stdMouseCursorWidth = 12;

class Screen {
public:
	Screen(HugoEngine *vm);
	virtual ~Screen();

	virtual void loadFont(int16 fontId) = 0;
	virtual void loadFontArr(Common::File &in) = 0;

	int16    fontHeight();
	int16    stringLength(const char *s);

	void     displayBackground();
	void     displayFrame(int sx, int sy, seq_t *seq, bool foreFl);
	void     displayList(dupdate_t update, ...);
	void     displayRect(int16 x, int16 y, int16 dx, int16 dy);
	void     drawRectangle(bool filledFl, int16 x1, int16 y1, int16 x2, int16 y2, int color);
	void     drawShape(int x, int y, int color1, int color2);
	void     drawStatusText();
	void     freeFonts();
	void     freePalette();
	void     hideCursor();
	void     initDisplay();
	void     initNewScreenDisplay();
	void     loadPalette(Common::File &in);
	void     moveImage(image_pt srcImage, int16 x1, int16 y1, int16 dx, int16 dy, int16 width1, image_pt dstImage, int16 x2, int16 y2, int16 width2);
	void     remapPal(uint16 oldIndex, uint16 newIndex);
	void     resetInventoryObjId();
	void     restorePal(Common::SeekableReadStream *f);
	void     savePal(Common::WriteStream *f);
	void     setBackgroundColor(long color);
	void     setCursorPal();
	void     selectInventoryObjId(int16 objId);
	void     shadowStr(int16 sx, int16 sy, const char *s, byte color);
	void     showCursor();
	void     userHelp();
	void     writeStr(int16 sx, int16 sy, const char *s, byte color);

	icondib_t &getIconBuffer() {
		return _iconBuffer;
	}

	viewdib_t &getBackBuffer() {
		return _backBuffer;
	}

	viewdib_t &getBackBufferBackup() {
		return _backBufferBackup;
	}

	viewdib_t &getFrontBuffer() {
		return _frontBuffer;
	}

	viewdib_t &getGUIBuffer() {
		return _GUIBuffer;
	}

protected:
	HugoEngine *_vm;

	static const int kRectListSize = 16;            // Size of add/restore rect lists
	static const int kBlitListSize = kRectListSize * 2; // Size of dirty rect blit list

	inline bool isInX(int16 x, rect_t *rect);
	inline bool isInY(int16 y, rect_t *rect);
	inline bool isOverlaping(rect_t *rectA, rect_t *rectB);

	bool fontLoadedFl[kNumFonts];

	// Fonts used in dib (non-GDI)
	byte *_arrayFont[kNumFonts];
	byte  _fnt;                                     // Current font number
	byte  _fontdata[kNumFonts][kFontSize];          // Font data
	byte *_font[kNumFonts][kFontLength];            // Ptrs to each char
	byte *_mainPalette;
	int16 _arrayFontSize[kNumFonts];

private:
	byte     *_curPalette;
	byte      _iconImage[kInvDx * kInvDy];
	byte      _paletteSize;

	icondib_t _iconBuffer;                          // Inventory icon DIB

	int16 mergeLists(rect_t *list, rect_t *blist, int16 len, int16 blen, int16 bmax);
	int16 center(const char *s);

	overlayState_t findOvl(seq_t *seq_p, image_pt dst_p, uint16 y);

	viewdib_t _frontBuffer;
	viewdib_t _backBuffer;
	viewdib_t _GUIBuffer;                           // User interface images
	viewdib_t _backBufferBackup;                    // Backup _backBuffer during inventory

	void createPal();
	void merge(rect_t *rectA, rect_t *rectB);
	void writeChr(int sx, int sy, byte color, char *local_fontdata);
};

class Screen_v1d : public Screen {
public:
	Screen_v1d(HugoEngine *vm);
	~Screen_v1d();

	void loadFont(int16 fontId);
	void loadFontArr(Common::File &in);
};

class Screen_v1w : public Screen {
public:
	Screen_v1w(HugoEngine *vm);
	~Screen_v1w();

	void loadFont(int16 fontId);
	void loadFontArr(Common::File &in);
};

} // End of namespace Hugo

#endif //HUGO_DISPLAY_H
