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

#ifndef ACCESS_SCREEN_H
#define ACCESS_SCREEN_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/stream.h"
#include "access/asurface.h"

namespace Access {

class AccessEngine;

#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class Screen: public ASurface {
private:
	AccessEngine *_vm;
	byte _tempPalette[PALETTE_SIZE];
	byte _rawPalette[PALETTE_SIZE];
	int _vesaCurrentWin;
	int _currentPanel;
	Common::Point _msVirtualOffset;
	Common::Point _virtualOffsetsTable[4];
	bool _hideFlag;
	Common::Rect _lastBounds;
	int _leftSkip, _rightSkip;
	int _topSkip, _bottomSkip;
	int _clipWidth, _clipHeight;

	void updatePalette();

	bool clip(Common::Rect &r);
public:
	int _vesaMode;
	bool _loadPalFlag;
	bool _scrollFlag;
	int _scrollThreshold;
	int _startColor, _numColors;
	Common::Point _vWindowSize;
	int _scrollX, _scrollY;
	int _scrollCol, _scrollRow;
	byte _manPal[0x60];
	byte _scaleTable1[256];
	byte _scaleTable2[256];
public:
	Screen(AccessEngine *vm);

	void setDisplayScan();

	void setPanel(int num);

	/**
	 * Update the underlying screen
	 */
	void updateScreen();

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
	void clearScreen() { clearBuffer(); }

	/**
	 * Set the initial palette
	 */
	void setInitialPalettte();

	void loadPalette(Common::SeekableReadStream *stream);

	void loadPalette(int fileNum, int subfile);

	void setPalette();

	/**
	 * Copy a buffer to the screen
	 */
	void copyBuffer(const byte *data);

	void plotImage(const byte *pData, int idx, const Common::Point &pt);

	void checkScroll();

	void copyBF1BF2();

	void copyBF2Vid();

	void plotList();

	void copyBlocks();

	void copyRects();

	void setBufferScan();

	void setScaleTable(int scale);
};

} // End of namespace Access

#endif /* ACCESS_SCREEN_H */
