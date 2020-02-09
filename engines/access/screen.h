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

#ifndef ACCESS_SCREEN_H
#define ACCESS_SCREEN_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/stream.h"
#include "graphics/screen.h"
#include "access/asurface.h"

namespace Access {

class AccessEngine;

struct ScreenSave {
	int _clipWidth;
	int _clipHeight;
	int _windowXAdd;
	int _windowYAdd;
	Common::Point _scroll;
	int _scrollCol;
	int _scrollRow;
	Common::Point _bufferStart;
	int _screenYOff;
};

class Screen : public BaseSurface {
private:
	AccessEngine *_vm;
	byte _tempPalette[PALETTE_SIZE];
	byte _rawPalette[PALETTE_SIZE];
	byte _savedPalettes[2][PALETTE_SIZE];
	int _savedPaletteCount;
	int _vesaCurrentWin;
	int _currentPanel;
	Common::Point _msVirtualOffset;
	Common::Point _virtualOffsetsTable[4];
	bool _hideFlag;
	ScreenSave _screenSave;
	int _startCycle;
	int _cycleStart;
	int _endCycle;
	Common::List<Common::Rect> _dirtyRects;

	void updatePalette();
public:
	int _vesaMode;
	int _startColor, _numColors;
	Common::Point _bufferStart;
	int _windowXAdd, _windowYAdd;
	int _screenYOff;
	byte _manPal[0x60];
	byte _scaleTable1[256];
	byte _scaleTable2[256];
	int _vWindowWidth;
	int _vWindowHeight;
	int _vWindowBytesWide;
	int _bufferBytesWide;
	int _vWindowLinesTall;
	bool _screenChangeFlag;
	bool _fadeIn;
public:
	/**
	 * Updates the screen
	 */
	void update() override;

	void copyBlock(BaseSurface *src, const Common::Rect &bounds) override;

	void restoreBlock() override;

	void drawRect() override;

	void drawBox() override;

	void copyBuffer(Graphics::ManagedSurface *src) override;
public:
	Screen(AccessEngine *vm);

	~Screen() override {}

	void setDisplayScan();

	void setPanel(int num);

	/**
	 * Fade out screen
	 */
	void forceFadeOut();

	/**
	 * Fade in screen
	 */
	void forceFadeIn();

	void fadeOut() { forceFadeOut(); }
	void fadeIn() { forceFadeIn(); }
	void clearScreen();

	/**
	 * Set the initial palette
	 */
	void setInitialPalettte();

	/**
	 * Set icon palette
	 */
	void setIconPalette();

	/**
	 * Set Tex palette (Martian Memorandum)
	 */
	void setManPalette();

	void loadPalette(int fileNum, int subfile);

	void setPalette();

	void loadRawPalette(Common::SeekableReadStream *stream);

	void savePalette();

	void restorePalette();

	void getPalette(byte *pal);

	void flashPalette(int count);

	/**
	 * Copy a buffer to the screen
	 */
	void copyBuffer(const byte *data);

	void setBufferScan();

	void setScaleTable(int scale);

	/**
	 * Save all the screen display state variables
	 */
	void saveScreen();

	/**
	 * Restores previously saved screen display state variables
	 */
	void restoreScreen();

	void setPaletteCycle(int startCycle, int endCycle, int timer);

	void cyclePaletteForward();

	void cyclePaletteBackwards();
};

} // End of namespace Access

#endif /* ACCESS_SCREEN_H */
