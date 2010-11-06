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
#define shapeSize 24

enum overlayState_t {UNDEF, FG, BG};                // Overlay state
struct rect_t {                                     // Rectangle used in Display list
	int16 x;                                        // Position in dib
	int16 y;                                        // Position in dib
	int16 dx;                                       // width
	int16 dy;                                       // height
};

class Screen {
public:
	Screen(HugoEngine *vm);
	virtual ~Screen();

	virtual void loadFont(int16 fontId) = 0;

	int16    fontHeight();
	int16    stringLength(const char *s);

	void     displayBackground();
	void     displayFrame(int sx, int sy, seq_t *seq, bool foreFl);
	void     displayList(dupdate_t update, ...);
	void     displayRect(int16 x, int16 y, int16 dx, int16 dy);
	void     drawRectangle(bool filledFl, uint16 x1, uint16 y1, uint16 x2, uint16 y2, int color);
	void     drawShape(int x, int y, int color1, int color2);
	void     drawStatusText();
	void     freePalette();
	void     initDisplay();
	void     initNewScreenDisplay();
	void     loadPalette(Common::File &in);
	void     moveImage(image_pt srcImage, uint16 x1, uint16 y1, uint16 dx, uint16 dy, uint16 width1, image_pt dstImage, uint16 x2, uint16 y2, uint16 width2);
	void     remapPal(uint16 oldIndex, uint16 newIndex);
	void     restorePal(Common::SeekableReadStream *f);
	void     savePal(Common::WriteStream *f);
	void     setBackgroundColor(long color);
	void     shadowStr(int16 sx, int16 sy, const char *s, byte color);
	void     userHelp();
	void     writeChr(int sx, int sy, byte color, char *local_fontdata);
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

	// Fonts used in dib (non-GDI)
	byte  _fnt;                                     // Current font number
	byte  _fontdata[NUM_FONTS][FONTSIZE];           // Font data
	byte *_font[NUM_FONTS][FONT_LEN];               // Ptrs to each char
	byte *_palette;

	byte  _paletteSize;

private:
	viewdib_t _frontBuffer;
	viewdib_t _backBuffer;
	viewdib_t _GUIBuffer;                           // User interface images
	viewdib_t _backBufferBackup;                    // Backup _backBuffer during inventory
	icondib_t _iconBuffer;                          // Inventory icon DIB

	void createPal();
	overlayState_t findOvl(seq_t *seq_p, image_pt dst_p, uint16 y);
	void merge(rect_t *rectA, rect_t *rectB);
	int16 mergeLists(rect_t *list, rect_t *blist, int16 len, int16 blen, int16 bmax);
	int16 center(const char *s);
};

class Screen_v1d : public Screen {
public:
	Screen_v1d(HugoEngine *vm);
	~Screen_v1d();

	void loadFont(int16 fontId);
};

class Screen_v1w : public Screen {
public:
	Screen_v1w(HugoEngine *vm);
	~Screen_v1w();

	void loadFont(int16 fontId);
};

} // End of namespace Hugo

#endif //HUGO_DISPLAY_H
