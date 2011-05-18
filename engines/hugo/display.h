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
enum overlayState_t {kOvlUndef, kOvlForeground, kOvlBackground}; // Overlay state

static const int kCenter = -1;                      // Used to center text in x


class Screen {
public:
	struct rect_t {                                 // Rectangle used in Display list
		int16 x;                                    // Position in dib
		int16 y;                                    // Position in dib
		int16 dx;                                   // width
		int16 dy;                                   // height
	};

	Screen(HugoEngine *vm);
	virtual ~Screen();

	virtual void loadFont(int16 fontId) = 0;
	virtual void loadFontArr(Common::ReadStream &in) = 0;

	int16    fontHeight() const;
	int16    stringLength(const char *s) const;

	void     displayBackground();
	void     displayFrame(const int sx, const int sy, seq_t *seq, const bool foreFl);
	void     displayList(dupdate_t update, ...);
	void     displayRect(const int16 x, const int16 y, const int16 dx, const int16 dy);
	void     drawBoundaries();
	void     drawRectangle(const bool filledFl, const int16 x1, const int16 y1, const int16 x2, const int16 y2, const int color);
	void     drawShape(const int x, const int y, const int color1, const int color2);
	void     drawStatusText();
	void     freeScreen();
	void     hideCursor();
	void     initDisplay();
	void     initNewScreenDisplay();
	void     loadPalette(Common::ReadStream &in);
	void     moveImage(image_pt srcImage, const int16 x1, const int16 y1, const int16 dx, int16 dy, const int16 width1, image_pt dstImage, const int16 x2, const int16 y2, const int16 width2);
	void     remapPal(uint16 oldIndex, uint16 newIndex);
	void     resetInventoryObjId();
	void     restorePal(Common::ReadStream *f);
	void     savePal(Common::WriteStream *f) const;
	void     setBackgroundColor(const uint16 color);
	void     setCursorPal();
	void     selectInventoryObjId(const int16 objId);
	void     shadowStr(int16 sx, const int16 sy, const char *s, const byte color);
	void     showCursor();
	void     userHelp() const;
	void     writeStr(int16 sx, const int16 sy, const char *s, const byte color);

	icondib_t &getIconBuffer();
	viewdib_t &getBackBuffer();
	viewdib_t &getBackBufferBackup();
	viewdib_t &getFrontBuffer();
	viewdib_t &getGUIBuffer();

protected:
	HugoEngine *_vm;

	static const int kRectListSize = 16;            // Size of add/restore rect lists
	static const int kBlitListSize = kRectListSize * 2; // Size of dirty rect blit list
	static const int kShapeSize = 24;
	static const int kFontLength = 128;             // Number of chars in font
	static const int kFontSize = 1200;              // Max size of font data
	static const int kNumFonts = 3;                 // Number of dib fonts
	static const byte stdMouseCursorHeight = 20;
	static const byte stdMouseCursorWidth = 12;

	bool fontLoadedFl[kNumFonts];

	// Fonts used in dib (non-GDI)
	byte *_arrayFont[kNumFonts];
	byte  _fnt;                                     // Current font number
	byte  _fontdata[kNumFonts][kFontSize];          // Font data
	byte *_font[kNumFonts][kFontLength];            // Ptrs to each char
	byte *_mainPalette;
	int16 _arrayFontSize[kNumFonts];

	viewdib_t _frontBuffer;

	inline bool isInX(const int16 x, const rect_t *rect) const;
	inline bool isInY(const int16 y, const rect_t *rect) const;
	inline bool isOverlapping(const rect_t *rectA, const rect_t *rectB) const;

	virtual overlayState_t findOvl(seq_t *seq_p, image_pt dst_p, uint16 y) = 0;

private:
	byte     *_curPalette;
	byte      _iconImage[kInvDx * kInvDy];
	byte      _paletteSize;

	icondib_t _iconBuffer;                          // Inventory icon DIB

	int16 mergeLists(rect_t *list, rect_t *blist, const int16 len, int16 blen);
	int16 center(const char *s) const;

	viewdib_t _backBuffer;
	viewdib_t _GUIBuffer;                              // User interface images
	viewdib_t _backBufferBackup;                       // Backup _backBuffer during inventory

	// Formerly static variables used by displayList()
	int16  _dlAddIndex, _dlRestoreIndex;               // Index into add/restore lists
	rect_t _dlRestoreList[kRectListSize];              // The restore list
	rect_t _dlAddList[kRectListSize];                  // The add list
	rect_t _dlBlistList[kBlitListSize];                // The blit list
	//

	void createPal();
	void merge(const rect_t *rectA, rect_t *rectB);
	void writeChr(const int sx, const int sy, const byte color, const char *local_fontdata);
};

class Screen_v1d : public Screen {
public:
	Screen_v1d(HugoEngine *vm);
	~Screen_v1d();

	void loadFont(int16 fontId);
	void loadFontArr(Common::ReadStream &in);
protected:
	overlayState_t findOvl(seq_t *seq_p, image_pt dst_p, uint16 y);
};

class Screen_v1w : public Screen {
public:
	Screen_v1w(HugoEngine *vm);
	~Screen_v1w();

	void loadFont(int16 fontId);
	void loadFontArr(Common::ReadStream &in);
protected:
	overlayState_t findOvl(seq_t *seq_p, image_pt dst_p, uint16 y);
};

} // End of namespace Hugo

#endif //HUGO_DISPLAY_H
