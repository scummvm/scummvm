/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "access/access.h"
#include "access/screen.h"
#include "access/resources.h"
#include "access/martian/martian_resources.h"

namespace Access {

#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)

ScreenSave::ScreenSave(){
	_clipWidth = _clipHeight = 0;
	_windowXAdd = _windowYAdd = 0;
	_scrollCol = _scrollRow = 0;
	_screenYOff = 0;
}

Screen::Screen(AccessEngine *vm) : _vm(vm) {
	create(320, 200);
	Common::fill(&_tempPalette[0], &_tempPalette[PALETTE_SIZE], 0);
	Common::fill(&_manPal[0], &_manPal[0x60], 0);
	Common::fill(&_scaleTable1[0], &_scaleTable1[256], 0);
	Common::fill(&_scaleTable2[0], &_scaleTable2[256], 0);
	_savedPaletteCount = 0;
	if (_vm->isCD())
		_vesaMode = 0;
	else
		_vesaMode = 1;

	_vesaCurrentWin = 0;
	_currentPanel = 0;
	_hideFlag = true;
	_startColor = _numColors = 0;
	_windowXAdd = _windowYAdd = 0;
	_screenYOff = 0;
	_screenChangeFlag = false;

	_bufferBytesWide = _vWindowBytesWide = this->w;
	_vWindowLinesTall = this->h;
	_vWindowWidth = _vWindowHeight = 0;
	_clipWidth = _vWindowBytesWide - 1;
	_clipHeight = _vWindowLinesTall - 1;
	_startCycle = 0;
	_cycleStart = 0;
	_endCycle = 0;
	_fadeIn = false;

	for (int i = 0; i < 768; ++i) {
		_rawPalette[i] = 0;
		_savedPalettes[0][i] = 0;
		_savedPalettes[1][i] = 0;
		_tempPalette[i] = 0;
	}
}

void Screen::clearScreen() {
	clearBuffer();
	if (_vesaMode)
		_vm->_clearSummaryFlag = true;
}

void Screen::setDisplayScan() {
	_clipWidth = this->w - 1;
	_clipHeight = this->h - 1;
	_windowXAdd = _windowYAdd = 0;
	_vm->_scrollX = _vm->_scrollY = 0;
	_vm->_scrollCol = _vm->_scrollRow = 0;
	_bufferStart.x = _bufferStart.y = 0;
	_screenYOff = 0;
}

void Screen::setPanel(int num) {
	assert(num < 4);
	_currentPanel = num;
	_msVirtualOffset = _virtualOffsetsTable[num];
}

void Screen::update() {
	if (_vm->_startup >= 0) {
		if (--_vm->_startup == -1)
			_fadeIn = true;
		return;
	}
	markAllDirty();//****DEBUG****
	Graphics::Screen::update();
}

void Screen::setInitialPalettte() {
	Common::copy(&INITIAL_PALETTE[0], &INITIAL_PALETTE[18 * 3], _rawPalette);
	Common::fill(&_rawPalette[18 * 3], &_rawPalette[PALETTE_SIZE], 0);

	g_system->getPaletteManager()->setPalette(INITIAL_PALETTE, 0, 18);
}

void Screen::setManPalette() {
	for (int i = 0; i < 0x42; i++) {
		_rawPalette[672 + i] = VGA_COLOR_TRANS(_manPal[i]);
	}
}

void Screen::setIconPalette() {
	if (_vm->getGameID() == GType_MartianMemorandum) {
		for (int i = 0; i < 0x1B; i++) {
			_rawPalette[741 + i] = VGA_COLOR_TRANS(Martian::ICON_PALETTE[i]);
		}
	}
}

void Screen::loadPalette(int fileNum, int subfile) {
	Resource *res = _vm->_files->loadFile(fileNum, subfile);
	byte *palette = res->data();
	Common::copy(palette, palette + (_numColors * 3), &_rawPalette[_startColor * 3]);
	delete res;
}

void Screen::setPalette() {
	g_system->getPaletteManager()->setPalette(&_rawPalette[0], 0, PALETTE_COUNT);
}

void Screen::loadRawPalette(Common::SeekableReadStream *stream) {
	stream->read(&_rawPalette[0], PALETTE_SIZE);
	for (byte *p = &_rawPalette[0]; p < &_rawPalette[PALETTE_SIZE]; ++p)
		*p = VGA_COLOR_TRANS(*p);
}

void Screen::updatePalette() {
	g_system->getPaletteManager()->setPalette(&_tempPalette[0], 0, PALETTE_COUNT);
	update();
}

void Screen::savePalette() {
	Common::copy(&_rawPalette[0], &_rawPalette[PALETTE_SIZE],
		&_savedPalettes[_savedPaletteCount][0]);

	if (++_savedPaletteCount == 2)
		_savedPaletteCount = 1;
}

void Screen::restorePalette() {
	if (--_savedPaletteCount < 0)
		_savedPaletteCount = 0;

	Common::copy(&_savedPalettes[_savedPaletteCount][0],
		&_savedPalettes[_savedPaletteCount][PALETTE_SIZE], &_rawPalette[0]);
}

void Screen::getPalette(byte *pal) {
	g_system->getPaletteManager()->grabPalette(pal, 0, 256);
}

void Screen::forceFadeOut() {
	const int FADE_AMOUNT = 2;
	bool repeatFlag;
	byte *srcP;
	int count;

	do {
		repeatFlag = false;
		for (srcP = &_tempPalette[0], count = 0; count < PALETTE_SIZE; ++count, ++srcP) {
			int v = *srcP;
			if (v) {
				repeatFlag = true;
				*srcP = MAX((int)*srcP - FADE_AMOUNT, 0);
			}
		}

		updatePalette();
		_vm->_events->pollEventsAndWait();
	} while (repeatFlag && !_vm->shouldQuit());
}

void Screen::forceFadeIn() {
	Common::fill(&_tempPalette[0], &_tempPalette[PALETTE_SIZE], 0);

	const int FADE_AMOUNT = 2;
	bool repeatFlag;
	do {
		repeatFlag = false;
		const byte *srcP = &_rawPalette[0];
		byte *destP = &_tempPalette[0];

		for (int idx = 0; idx < PALETTE_SIZE; ++idx, ++srcP, ++destP) {
			if (*destP != *srcP) {
				repeatFlag = true;
				*destP = MIN((int)*destP + FADE_AMOUNT, (int)*srcP);
			}
		}

		updatePalette();
		_vm->_events->pollEventsAndWait();
	} while (repeatFlag);
}

void Screen::copyBuffer(const byte *data) {
	byte *destP = (byte *)getPixels();
	Common::copy(data, data + (h * w), destP);
	g_system->copyRectToScreen(destP, w, 0, 0, w, h);
}

void Screen::setBufferScan() {
	_clipWidth = _vWindowBytesWide - 1;
	_windowXAdd = (320 - _clipWidth) >> 1;
	_clipHeight = _vWindowLinesTall - 1;
	_windowYAdd = (176 - _clipHeight) >> 1;
}

void Screen::setScaleTable(int scale) {
	int total = 0;
	for (int idx = 0; idx < 256; ++idx) {
		_scaleTable1[idx] = total >> 8;
		_scaleTable2[idx] = total & 0xff;
		total += scale;
	}
}

void Screen::saveScreen() {
	_screenSave._clipWidth = _clipWidth;
	_screenSave._clipHeight = _clipHeight;
	_screenSave._windowXAdd = _windowXAdd;
	_screenSave._windowYAdd = _windowYAdd;
	_screenSave._scroll.x = _vm->_scrollX;
	_screenSave._scroll.y = _vm->_scrollY;
	_screenSave._scrollCol = _vm->_scrollCol;
	_screenSave._scrollRow = _vm->_scrollRow;
	_screenSave._bufferStart.x = _bufferStart.x;
	_screenSave._bufferStart.y = _bufferStart.y;
	_screenSave._screenYOff = _screenYOff;
}

void Screen::restoreScreen() {
	_clipWidth = _screenSave._clipWidth;
	_clipHeight = _screenSave._clipHeight;
	_windowXAdd = _screenSave._windowXAdd;
	_windowYAdd = _screenSave._windowYAdd;
	_vm->_scrollX = _screenSave._scroll.x;
	_vm->_scrollY = _screenSave._scroll.y;
	_vm->_scrollCol = _screenSave._scrollCol;
	_vm->_scrollRow = _screenSave._scrollRow;
	_bufferStart.x = _screenSave._bufferStart.x;
	_bufferStart.y = _screenSave._bufferStart.y;
	_screenYOff = _screenSave._screenYOff;
}

void Screen::copyBlock(BaseSurface *src, const Common::Rect &bounds) {
	Common::Rect destBounds = bounds;
	destBounds.translate(_windowXAdd, _windowYAdd + _screenYOff);

	copyRectToSurface(*src, destBounds.left, destBounds.top, bounds);
	addDirtyRect(destBounds);
}

void Screen::restoreBlock() {
	if (!_savedBounds.isEmpty())
		addDirtyRect(_savedBounds);
	BaseSurface::restoreBlock();
}

void Screen::drawRect() {
	addDirtyRect(Common::Rect(_orgX1, _orgY1, _orgX2, _orgY2));
	BaseSurface::drawRect();
}

void Screen::drawBox() {
	addDirtyRect(Common::Rect(_orgX1, _orgY1, _orgX2, _orgY2));
	BaseSurface::drawBox();
}

void Screen::copyBuffer(Graphics::ManagedSurface *src) {
	addDirtyRect(Common::Rect(0, 0, src->w, src->h));
	BaseSurface::copyBuffer(src);
}

void Screen::setPaletteCycle(int startCycle, int endCycle, int timer) {
	_startCycle = _cycleStart = startCycle;
	_endCycle = endCycle;

	TimerEntry &te = _vm->_timers[6];
	te._timer = te._initTm = timer;
	te._flag++;
}

void Screen::cyclePaletteForward() {
	cyclePaletteBackwards();
}

void Screen::cyclePaletteBackwards() {
	if (!_vm->_timers[6]._flag) {
		_vm->_timers[6]._flag++;
		byte *pStart = &_rawPalette[_cycleStart * 3];
		byte *pEnd = &_rawPalette[_endCycle * 3];

		for (int idx = _startCycle; idx < _endCycle; ++idx) {
			g_system->getPaletteManager()->setPalette(pStart, idx, 1);

			pStart += 3;
			if (pStart == pEnd)
				pStart = &_rawPalette[_cycleStart * 3];
		}

		if (--_cycleStart <= _startCycle)
			_cycleStart = _endCycle - 1;

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void Screen::flashPalette(int count) {
	// No implementation needed in ScummVM
}

} // End of namespace Access
