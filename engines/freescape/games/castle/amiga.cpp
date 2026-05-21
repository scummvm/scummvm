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

#include "common/file.h"
#include "common/memstream.h"
#include "graphics/cursorman.h"

#include "audio/mods/protracker.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

byte kAmigaCastlePalette[16][3] = {
	{0x00, 0x00, 0x00},
	{0x44, 0x44, 0x44},
	{0x66, 0x66, 0x66},
	{0x88, 0x88, 0x88},
	{0xaa, 0xaa, 0xaa},
	{0xcc, 0xcc, 0xcc},
	{0x00, 0x00, 0x88},
	{0x66, 0xaa, 0x00},
	{0x88, 0xcc, 0x00},
	{0xcc, 0xee, 0x00},
	{0xee, 0xee, 0x66},
	{0x44, 0x88, 0x00},
	{0xee, 0xaa, 0x00},
	{0xcc, 0x44, 0x00},
	{0x88, 0x44, 0x00},
	{0xee, 0xee, 0xee},
};

byte kAmigaCastleRiddlePalette[16][3] = {
	{0x00, 0x00, 0x00},
	{0x44, 0x44, 0x44},
	{0x66, 0x66, 0x66},
	{0x88, 0x88, 0x88},
	{0xaa, 0xaa, 0xaa},
	{0xcc, 0x44, 0x00},
	{0xee, 0xaa, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0xaa, 0x88, 0x00},
	{0xaa, 0x66, 0x00},
	{0x88, 0x44, 0x00},
	{0xee, 0xcc, 0x66},
};

class CastleAmigaIntroPlayer {
public:
	CastleAmigaIntroPlayer(CastleEngine *engine, const Common::Array<byte> &introText)
	    : _engine(engine), _data(introText) {
		_screen[0].resize(kScreenBytes);
		_screen[1].resize(kScreenBytes);
		reset();
	}

	bool run(bool &selectedPrincess) {
		selectedPrincess = false;

		clearDrawBuffer();
		drawBaseScreen();
		drawStaticLogo();
		drawStaticForeground();
		fadePalette(0x1dc6);
		pageFlip();
		displayFrame(false, false);
		fadePalette(0x1de6);

		int key = 0;
		while (!_aborted) {
			clearDrawBuffer();
			drawBaseScreen();
			drawStaticForeground();
			drawAnimatedObject();
			drawStaticLogo();
			key = displayFrame(false, true);
			if (key || _phaseDone)
				break;
			pageFlip();
			displayFrame(false, false);
			updateAnimation();
			_frameCounter++;
		}

		if (_aborted)
			return false;

		_phaseDone = 0;
		_renderMode = 2;
		_motionPtr = 0x2054;
		updateAnimation();

		clearDrawBuffer();
		drawBaseScreen();
		drawMovingCharacters();
		drawOverlay();
		drawLanguageText();
		fadePalette(0x1dc6);
		pageFlip();
		displayFrame(false, false);
		displayFrame(false, false);
		fadePalette(0x1e06);

		setSelectionMouseEnabled(true);
		while (!_aborted) {
			clearDrawBuffer();
			drawBaseScreen();
			drawMovingCharacters();
			drawOverlay();
			if (_choiceState == 0)
				drawLanguageText();

			key = displayFrame(true, false);
			if (key == 1 || key == 2)
				acceptSelection(key);

			if (_phaseDone == 2 && _frameCounter > 5)
				break;

			pageFlip();
			displayFrame(false, false);
			updateAnimation();
			_frameCounter++;
		}
		setSelectionMouseEnabled(false);

		if (_aborted)
			return false;

		selectedPrincess = (_choice == 2);

		_renderMode = 3;
		_choiceState = 0;
		_phaseDone = 0;
		clearDrawBuffer();
		_motionPtr = 0x2256;
		updateAnimation();
		drawBaseScreen();
		drawStaticForeground();
		drawAnimatedObject();
		drawStaticLogo();
		fadePalette(0x1dc6);
		pageFlip();
		displayFrame(false, false);
		displayFrame(false, false);
		fadePalette(0x1de6);

		int guard = 0;
		while (!_aborted && !_phaseDone && guard++ < 600) {
			clearDrawBuffer();
			drawBaseScreen();
			drawStaticForeground();
			drawAnimatedObject();
			drawStaticLogo();
			displayFrame(false, false);
			pageFlip();
			displayFrame(false, false);
			updateAnimation();
			_frameCounter++;
		}

		return !_aborted;
	}

private:
	static const int kWidth = 320;
	static const int kHeight = 200;
	static const int kPlaneBytes = 0x1f40;
	static const int kRowBytes = 40;
	static const int kScreenBytes = 0x9c40;
	static const int kVbiDelayMillis = 20;

	CastleEngine *_engine;
	const Common::Array<byte> &_data;
	Common::Array<byte> _screen[2];
	uint16 _palette[32];
	int _displayBuffer;
	int _drawBuffer;
	bool _pageFlag;
	bool _aborted;

	int32 _scrollA;
	int32 _scrollB;
	int16 _sprite1Frame;
	int16 _sprite1X;
	int16 _sprite1Y;
	int16 _sprite2Frame;
	int16 _sprite2X;
	int16 _sprite2Y;
	int16 _choiceState;
	int16 _frameCounter;
	int16 _selectionLift;
	int16 _selectionX;
	int16 _selectionY;
	int16 _objectFrame;
	int16 _objectX;
	int16 _objectY;
	int16 _phaseDone;
	int16 _renderMode;
	int16 _arrowFrame;
	int16 _choice;
	uint32 _character1Ptr;
	uint32 _character2Ptr;
	uint32 _motionPtr;

	void reset() {
		memset(_screen[0].data(), 0, kScreenBytes);
		memset(_screen[1].data(), 0, kScreenBytes);
		memset(_palette, 0, sizeof(_palette));

		_displayBuffer = 0;
		_drawBuffer = 1;
		_pageFlag = false;
		_aborted = false;
		_scrollA = 0;
		_scrollB = 0;
		_sprite1Frame = 0;
		_sprite1X = 40;
		_sprite1Y = 75;
		_sprite2Frame = 0;
		_sprite2X = 124;
		_sprite2Y = 86;
		_choiceState = 0;
		_frameCounter = 0;
		_selectionLift = 0;
		_selectionX = 0;
		_selectionY = -250;
		_objectFrame = 0;
		_objectX = 160;
		_objectY = 100;
		_phaseDone = 0;
		_renderMode = 1;
		_arrowFrame = 0;
		_choice = 1;
		_character1Ptr = READ_BE_UINT32(_data.data() + 0x1e26);
		_character2Ptr = READ_BE_UINT32(_data.data() + 0x1eb8);
		_motionPtr = READ_BE_UINT32(_data.data() + 0x1f4a);
	}

	static int16 highWord(int32 value) {
		return (int16)((uint32)value >> 16);
	}

	uint16 screenWord(int offset) const {
		if (offset < 0 || offset + 1 >= kScreenBytes)
			return 0;
		const Common::Array<byte> &buf = _screen[_drawBuffer];
		return (buf[offset] << 8) | buf[offset + 1];
	}

	void setScreenWord(int offset, uint16 value) {
		if (offset < 0 || offset + 1 >= kScreenBytes)
			return;
		Common::Array<byte> &buf = _screen[_drawBuffer];
		buf[offset] = value >> 8;
		buf[offset + 1] = value & 0xff;
	}

	void setScreenLong(int offset, uint32 value) {
		if (offset < 0 || offset + 3 >= kScreenBytes)
			return;
		Common::Array<byte> &buf = _screen[_drawBuffer];
		WRITE_BE_UINT32(&buf[offset], value);
	}

	void andScreenWord(int offset, uint16 value) {
		setScreenWord(offset, screenWord(offset) & value);
	}

	void orScreenWord(int offset, uint16 value) {
		setScreenWord(offset, screenWord(offset) | value);
	}

	static uint16 shiftedWord(uint32 value, int shift) {
		return (uint16)((value << shift) >> 16);
	}

	void clearDrawBuffer() {
		memset(_screen[_drawBuffer].data(), 0, kScreenBytes);
	}

	void drawBaseScreen() {
		int dst = 38 * kRowBytes;
		dst = drawScrollingPlane(0x247c, _scrollA, 57, dst);
		dst = drawScrollingFourPlane(0x2d64, _scrollB, 40, dst);
		dst = drawStaticFourPlane(0x4664, 33, dst);
		drawFillBands(dst);
	}

	int drawScrollingPlane(int src, int32 scroll, int rows, int dst) {
		int d7 = highWord(scroll);
		int srcOffset = ((d7 >> 3) & 0x3e);
		int shift = d7 & 0xf;
		int skip = -(((srcOffset >> 1) - 0x12));
		src += srcOffset;

		for (int row = 0; row < rows; row++) {
			int rowSrc = src + 0x28;
			int run = skip;
			int curSrc = src;
			for (int col = 0; col < 20; col++) {
				uint32 val;
				if (run >= 0) {
					val = READ_BE_UINT32(_data.data() + curSrc);
					run--;
				} else {
					run = 0xff;
					val = ((uint32)READ_BE_UINT16(_data.data() + curSrc) << 16) | READ_BE_UINT16(_data.data() + curSrc - 0x26);
					curSrc -= 0x28;
				}
				uint16 word = shiftedWord(val, shift);
				setScreenWord(dst + kPlaneBytes, 0);
				setScreenWord(dst + kPlaneBytes * 2, 0);
				setScreenWord(dst + kPlaneBytes * 3, 0);
				setScreenWord(dst + kPlaneBytes * 4, 0);
				setScreenWord(dst, word);
				curSrc += 2;
				dst += 2;
			}
			src = rowSrc;
		}
		return dst;
	}

	int drawScrollingFourPlane(int src, int32 scroll, int rows, int dst) {
		int d7 = highWord(scroll);
		int srcOffset = ((d7 >> 3) & 0x3e);
		int shift = d7 & 0xf;
		int skip = -(((srcOffset >> 1) - 0x12));
		src += srcOffset;

		for (int row = 0; row < rows; row++) {
			int rowSrc = src + 0xa0;
			int run = skip;
			int curSrc = src;
			for (int col = 0; col < 20; col++) {
				uint32 p0, p1, p2, p3;
				if (run >= 0) {
					p0 = READ_BE_UINT32(_data.data() + curSrc);
					p1 = READ_BE_UINT32(_data.data() + curSrc + 0x28);
					p2 = READ_BE_UINT32(_data.data() + curSrc + 0x50);
					p3 = READ_BE_UINT32(_data.data() + curSrc + 0x78);
					run--;
				} else {
					run = 0xff;
					p0 = ((uint32)READ_BE_UINT16(_data.data() + curSrc) << 16) | READ_BE_UINT16(_data.data() + curSrc - 0x26);
					p1 = ((uint32)READ_BE_UINT16(_data.data() + curSrc + 0x28) << 16) | READ_BE_UINT16(_data.data() + curSrc + 0x02);
					p2 = ((uint32)READ_BE_UINT16(_data.data() + curSrc + 0x50) << 16) | READ_BE_UINT16(_data.data() + curSrc + 0x2a);
					p3 = ((uint32)READ_BE_UINT16(_data.data() + curSrc + 0x78) << 16) | READ_BE_UINT16(_data.data() + curSrc + 0x52);
					curSrc -= 0x28;
				}
				setScreenLong(dst, p0 << shift);
				setScreenLong(dst + kPlaneBytes, p1 << shift);
				setScreenLong(dst + kPlaneBytes * 2, p2 << shift);
				setScreenLong(dst + kPlaneBytes * 3, p3 << shift);
				curSrc += 2;
				dst += 2;
			}
			src = rowSrc;
		}
		return dst;
	}

	int drawStaticFourPlane(int src, int rows, int dst) {
		for (int row = 0; row < rows; row++) {
			for (int col = 0; col < 20; col++) {
				setScreenWord(dst, READ_BE_UINT16(_data.data() + src));
				setScreenWord(dst + kPlaneBytes, READ_BE_UINT16(_data.data() + src + 0x28));
				setScreenWord(dst + kPlaneBytes * 2, READ_BE_UINT16(_data.data() + src + 0x50));
				setScreenWord(dst + kPlaneBytes * 3, READ_BE_UINT16(_data.data() + src + 0x78));
				src += 2;
				dst += 2;
			}
			src += 0x78;
		}
		return dst;
	}

	void drawFillBands(int dst) {
		int src = 0x5ba4;
		for (int band = 0; band < 13; band++) {
			int count = READ_BE_UINT16(_data.data() + src);
			src += 2;
			uint16 p0 = READ_BE_UINT16(_data.data() + src);
			uint16 p1 = READ_BE_UINT16(_data.data() + src + 2);
			uint16 p2 = READ_BE_UINT16(_data.data() + src + 4);
			src += 6;
			for (int repeat = 0; repeat <= count; repeat++) {
				for (int col = 0; col < 20; col++) {
					setScreenWord(dst, p0);
					setScreenWord(dst + kPlaneBytes, p1);
					setScreenWord(dst + kPlaneBytes * 2, p2);
					setScreenWord(dst + kPlaneBytes * 3, 0);
					dst += 2;
				}
			}
		}
	}

	void drawOverlay() {
		int src = 0x5c0c;
		int dst = 0x1a18;
		for (int row = 0; row < 30; row++) {
			for (int col = 0; col < 20; col++) {
				uint16 p0 = READ_BE_UINT16(_data.data() + src);
				uint16 p1 = READ_BE_UINT16(_data.data() + src + 0x28);
				uint16 p2 = READ_BE_UINT16(_data.data() + src + 0x50);
				uint16 p3 = READ_BE_UINT16(_data.data() + src + 0x78);
				uint16 mask = ~(p0 | p1 | p2 | p3);
				andScreenWord(dst, mask);
				orScreenWord(dst, p0);
				andScreenWord(dst + kPlaneBytes, mask);
				orScreenWord(dst + kPlaneBytes, p1);
				andScreenWord(dst + kPlaneBytes * 2, mask);
				orScreenWord(dst + kPlaneBytes * 2, p2);
				andScreenWord(dst + kPlaneBytes * 3, mask);
				orScreenWord(dst + kPlaneBytes * 3, p3);
				src += 2;
				dst += 2;
			}
			src += 0x78;
		}
		for (int i = 0; i < 60; i++) {
			setScreenWord(dst, 0);
			setScreenWord(dst + kPlaneBytes, 0);
			setScreenWord(dst + kPlaneBytes * 2, 0);
			setScreenWord(dst + kPlaneBytes * 3, 0);
			dst += 2;
		}
	}

	void drawMaskedBlock4(int src, int dst, int rows, int words) {
		for (int row = 0; row < rows; row++) {
			for (int col = 0; col < words; col++) {
				uint16 p0 = READ_BE_UINT16(_data.data() + src);
				uint16 p1 = READ_BE_UINT16(_data.data() + src + 2);
				uint16 p2 = READ_BE_UINT16(_data.data() + src + 4);
				uint16 p3 = READ_BE_UINT16(_data.data() + src + 6);
				uint16 mask = ~(p0 | p1 | p2 | p3);
				andScreenWord(dst, mask);
				orScreenWord(dst, p0);
				andScreenWord(dst + kPlaneBytes, mask);
				orScreenWord(dst + kPlaneBytes, p1);
				andScreenWord(dst + kPlaneBytes * 2, mask);
				orScreenWord(dst + kPlaneBytes * 2, p2);
				andScreenWord(dst + kPlaneBytes * 3, mask);
				orScreenWord(dst + kPlaneBytes * 3, p3);
				src += 8;
				dst += 2;
			}
		}
	}

	void drawStaticLogo() {
		drawMaskedBlock4(0x14ff4, 0x820, 0x94, 20);
	}

	void drawStaticForeground() {
		drawMaskedBlock4(0x1ac74, 0, 0x27, 20);
	}

	void drawMovingCharacters() {
		drawLargeSprite4(0x700c, _sprite1Frame, 0xe60, _sprite1X, _sprite1Y, 0x73, 0x20, 0x08, 4);
		drawLargeSprite4(0xd4ac, _sprite2Frame, 0xd00, _sprite2X, _sprite2Y, 0x68, 0x20, 0x08, 4);
		if (_choiceState != 0 && _selectionLift == 0) {
			drawLargeSprite4(0x108ac, 0, 0, _selectionX - 0x20, _selectionY - 0x76, 0x95, 0x40, 0x10, 8);
			return;
		}
		drawAnimatedObject();
	}

	void drawAnimatedObject() {
		bool objectVisible = true;
		if (_objectY < 0 && -_objectY >= 13) {
			_selectionLift = 0;
			objectVisible = false;
		} else {
			drawSprite1(0x12dec + _objectFrame * 0x34, _objectX, _objectY, 13);
		}
		if (objectVisible && _renderMode == 3) {
			int src = 0x136dc;
			if (_choice != 2)
				src += 0x36;
			src += _arrowFrame * 18;
			_arrowFrame++;
			if (_arrowFrame > 2)
				_arrowFrame = 0;
			drawSmallSamePlane(src, _objectX + 8, _objectY + 8);
		}
	}

	void drawLargeSprite4(int base, int frame, int frameSize, int x, int y, int rows, int rowStride, int planeStride, int maxWords) {
		int src = base + frame * frameSize;
		int rowCount = rows;
		int dstY = y;
		if (dstY < 0) {
			int skip = -dstY;
			if (skip >= rowCount)
				return;
			rowCount -= skip;
			src += skip * rowStride;
			dstY = 0;
		}
		if (dstY >= kHeight)
			return;
		if (dstY + rowCount > kHeight)
			rowCount = kHeight - dstY;
		drawSprite4(src, x, dstY, rowCount, rowStride, planeStride, maxWords);
	}

	void drawSprite4(int src, int x, int y, int rows, int rowStride, int planeStride, int maxWords) {
		if (x >= kWidth)
			return;
		int dstBase = y * kRowBytes + ((x >> 3) & 0xfffe);
		int visible = kWidth - x;
		if (visible <= 0)
			return;
		int d0 = visible >> 4;
		if (d0 > maxWords)
			d0 = maxWords;
		int shift = (16 - (x & 0xf)) & 0xf;
		if (shift == 0)
			d0--;
		if (d0 < 0)
			return;

		for (int row = 0; row < rows; row++) {
			int dst = dstBase + row * kRowBytes;
			for (int col = 0; col <= d0; col++) {
				uint32 values[4];
				for (int plane = 0; plane < 4; plane++) {
					int planeSrc = src + plane * planeStride;
					if (shift && col == 0)
						values[plane] = READ_BE_UINT16(_data.data() + planeSrc);
					else {
						values[plane] = READ_BE_UINT32(_data.data() + planeSrc + (shift ? (col - 1) * 2 : col * 2));
						if (col == d0)
							values[plane] &= 0xffff0000;
					}
				}

				uint16 p0 = shiftedWord(values[0], shift);
				uint16 p1 = shiftedWord(values[1], shift);
				uint16 p2 = shiftedWord(values[2], shift);
				uint16 p3 = shiftedWord(values[3], shift);
				uint16 mask = ~(p0 | p1 | p2 | p3);
				andScreenWord(dst, mask);
				orScreenWord(dst, p0);
				andScreenWord(dst + kPlaneBytes, mask);
				orScreenWord(dst + kPlaneBytes, p1);
				andScreenWord(dst + kPlaneBytes * 2, mask);
				orScreenWord(dst + kPlaneBytes * 2, p2);
				andScreenWord(dst + kPlaneBytes * 3, mask);
				orScreenWord(dst + kPlaneBytes * 3, p3);
				dst += 2;
			}
			src += rowStride;
		}
	}

	void drawSprite1(int src, int x, int y, int rows) {
		if (x >= kWidth || y >= kHeight)
			return;
		if (y < 0) {
			int skip = -y;
			if (skip >= rows)
				return;
			rows -= skip;
			src += skip * 4;
			y = 0;
		}
		if (y + rows > kHeight)
			rows = kHeight - y;

		int visible = kWidth - x;
		if (visible <= 0)
			return;
		int d0 = visible >> 4;
		if (d0 > 2)
			d0 = 2;
		int shift = (16 - (x & 0xf)) & 0xf;
		if (shift == 0)
			d0--;
		if (d0 < 0)
			return;

		int dstBase = y * kRowBytes + ((x >> 3) & 0xfffe);
		for (int row = 0; row < rows; row++) {
			int dst = dstBase + row * kRowBytes;
			for (int col = 0; col <= d0; col++) {
				uint32 value;
				if (shift && col == 0)
					value = READ_BE_UINT16(_data.data() + src);
				else {
					value = READ_BE_UINT32(_data.data() + src + (shift ? (col - 1) * 2 : col * 2));
					if (col == d0 && x <= 0x120)
						value &= 0xffff0000;
				}
				uint16 p = shiftedWord(value, shift);
				uint16 mask = ~p;
				if (_renderMode == 2) {
					andScreenWord(dst + kPlaneBytes, mask);
					andScreenWord(dst + kPlaneBytes * 2, mask);
					andScreenWord(dst + kPlaneBytes * 4, mask);
					orScreenWord(dst + kPlaneBytes * 4, p);
					andScreenWord(dst + kPlaneBytes * 3, mask);
					orScreenWord(dst + kPlaneBytes * 3, p);
					andScreenWord(dst, mask);
					orScreenWord(dst, p);
				} else {
					andScreenWord(dst, mask);
					orScreenWord(dst, p);
					andScreenWord(dst + kPlaneBytes, mask);
					orScreenWord(dst + kPlaneBytes, p);
					andScreenWord(dst + kPlaneBytes * 2, mask);
					orScreenWord(dst + kPlaneBytes * 2, p);
					andScreenWord(dst + kPlaneBytes * 3, mask);
					orScreenWord(dst + kPlaneBytes * 3, p);
				}
				dst += 2;
			}
			src += 4;
		}
	}

	void drawSmallSamePlane(int src, int x, int y) {
		int rows = 9;
		if (y < 0) {
			int skip = -y;
			if (skip >= rows)
				return;
			rows -= skip;
			src += skip * 2;
			y = 0;
		}
		if (y >= kHeight || x >= kWidth)
			return;
		if (y + rows > kHeight)
			rows = kHeight - y;

		int dstBase = y * kRowBytes + ((x >> 3) & 0xfffe);
		int shift = (16 - (x & 0xf)) & 0x1f;
		int words = (kWidth - x) >> 4;
		if (words > 1)
			words = 1;
		if (words < 0)
			return;

		for (int row = 0; row < rows; row++) {
			uint32 value = (uint32)READ_BE_UINT16(_data.data() + src) << shift;
			uint16 right = value & 0xffff;
			uint16 left = (value >> 16) & 0xffff;
			int dst = dstBase + row * kRowBytes;
			if (words != 0)
				orSamePlanes(dst + 2, right);
			orSamePlanes(dst, left);
			src += 2;
		}
	}

	void orSamePlanes(int dst, uint16 bits) {
		uint16 mask = ~bits;
		for (int plane = 0; plane < 4; plane++) {
			andScreenWord(dst + plane * kPlaneBytes, mask);
			orScreenWord(dst + plane * kPlaneBytes, bits);
		}
	}

	void drawLanguageText() {
		int table = currentLanguageTextTable();

		for (int i = 0; i < 5; i++) {
			int src = READ_BE_UINT32(_data.data() + table);
			int x = (int16)READ_BE_UINT16(_data.data() + table + 4);
			int y = (int16)READ_BE_UINT16(_data.data() + table + 6);
			int width = (int16)READ_BE_UINT16(_data.data() + table + 8) + 1;
			int rows = (int16)READ_BE_UINT16(_data.data() + table + 10) + 1;
			drawLanguageBlock(src, x, y, width, rows);
			table += 12;
		}
	}

	int currentLanguageTextTable() const {
		if (_engine->_language == Common::FR_FRA)
			return 0x1d2e;
		if (_engine->_language == Common::DE_DEU)
			return 0x1d6a;
		return 0x1cf2;
	}

	int selectionSplitX() const {
		int table = currentLanguageTextTable();
		int princeEntry = table + 3 * 12;
		int princessEntry = table + 4 * 12;
		int princeX = (int16)READ_BE_UINT16(_data.data() + princeEntry + 4);
		int princeWords = (int16)READ_BE_UINT16(_data.data() + princeEntry + 8) + 2;
		int princessX = (int16)READ_BE_UINT16(_data.data() + princessEntry + 4);
		if (princeWords <= 0 || princessX <= princeX)
			return kWidth / 2;
		return (princeX + princeWords * 16 + princessX) / 2;
	}

	void drawLanguageBlock(int src, int x, int y, int sourceWords, int rows) {
		if (y >= kHeight || x >= kWidth)
			return;
		int dstBase = y * kRowBytes + ((x >> 3) & 0xfffe);
		int shift = (16 - (x & 0xf)) & 0xf;
		for (int row = 0; row < rows; row++) {
			int dst = dstBase + row * kRowBytes;
			uint16 prev0 = 0;
			uint16 prev1 = 0;
			uint16 prev2 = 0;
			uint16 prev3 = 0;
			for (int col = 0; col <= sourceWords; col++) {
				uint32 p0;
				uint32 p1;
				uint32 p2;
				uint32 p3;
				if (col < sourceWords) {
					uint16 cur0 = READ_BE_UINT16(_data.data() + src);
					uint16 cur1 = READ_BE_UINT16(_data.data() + src + 2);
					uint16 cur2 = READ_BE_UINT16(_data.data() + src + 4);
					uint16 cur3 = READ_BE_UINT16(_data.data() + src + 6);
					if (col == 0) {
						p0 = cur0;
						p1 = cur1;
						p2 = cur2;
						p3 = cur3;
					} else {
						p0 = ((uint32)prev0 << 16) | cur0;
						p1 = ((uint32)prev1 << 16) | cur1;
						p2 = ((uint32)prev2 << 16) | cur2;
						p3 = ((uint32)prev3 << 16) | cur3;
					}
					prev0 = cur0;
					prev1 = cur1;
					prev2 = cur2;
					prev3 = cur3;
					src += 8;
				} else {
					p0 = 0;
					p1 = 0;
					p2 = 0;
					p3 = 0;
				}
				uint16 w0 = shiftedWord(p0, shift);
				uint16 w1 = shiftedWord(p1, shift);
				uint16 w2 = shiftedWord(p2, shift);
				uint16 w3 = shiftedWord(p3, shift);
				uint16 mask = ~(w0 | w1 | w2 | w3);
				andScreenWord(dst, mask);
				orScreenWord(dst, w0);
				andScreenWord(dst + kPlaneBytes, mask);
				orScreenWord(dst + kPlaneBytes, w1);
				andScreenWord(dst + kPlaneBytes * 2, mask);
				orScreenWord(dst + kPlaneBytes * 2, w2);
				andScreenWord(dst + kPlaneBytes * 3, mask);
				orScreenWord(dst + kPlaneBytes * 3, w3);
				dst += 2;
			}
		}
	}

	void fadePalette(int target) {
		for (int threshold = 15; threshold > 0 && !_aborted; threshold--) {
			for (int i = 0; i < 16; i++) {
				uint16 dst = READ_BE_UINT16(_data.data() + target + i * 2);
				uint16 cur = _palette[i];
				int channels[3] = {cur & 0xf, (cur >> 4) & 0xf, (cur >> 8) & 0xf};
				int targets[3] = {dst & 0xf, (dst >> 4) & 0xf, (dst >> 8) & 0xf};
				for (int c = 0; c < 3; c++) {
					int diff = targets[c] - channels[c];
					if (ABS(diff) >= threshold && diff != 0)
						channels[c] += diff > 0 ? 1 : -1;
				}
				_palette[i] = (channels[2] << 8) | (channels[1] << 4) | channels[0];
			}
			displayFrame(false, false, 4);
		}
	}

	void pageFlip() {
		_pageFlag = !_pageFlag;
		if (_pageFlag) {
			_displayBuffer = 0;
			_drawBuffer = 1;
		} else {
			_displayBuffer = 1;
			_drawBuffer = 0;
		}
	}

	void updateAnimation() {
		_scrollA += 0x8000;
		if (highWord(_scrollA) >= 0x13f)
			_scrollA -= 0x13f0000;
		_scrollB += 0x10000;
		if (highWord(_scrollB) >= 0x13f)
			_scrollB -= 0x13f0000;

		if (_choiceState == 1)
			updateCharacterPath(_character1Ptr, _sprite1Frame, _sprite1X, _sprite1Y);
		if (_choiceState == 2)
			updateCharacterPath(_character2Ptr, _sprite2Frame, _sprite2X, _sprite2Y);

		if (_choiceState < 0 && _selectionLift == 0) {
			_selectionX = (_choiceState == -1) ? _sprite1X : _sprite2X;
			_selectionY += 12;
			if (_selectionY > 0x4b)
				_choiceState = -_choiceState;
		}

		if (_renderMode == 1 && _frameCounter <= 10)
			return;

		if (READ_BE_UINT32(_data.data() + _motionPtr) == 0xffffffff) {
			_phaseDone = 1;
			_motionPtr = 0x21b0;
		}
		_objectFrame = (int16)READ_BE_UINT16(_data.data() + _motionPtr);
		_objectX = (int16)READ_BE_UINT16(_data.data() + _motionPtr + 2);
		_objectY = (int16)READ_BE_UINT16(_data.data() + _motionPtr + 4);
		_motionPtr += 6;
		if (_selectionLift != 0 && _renderMode == 2) {
			_objectY -= _selectionLift;
			_selectionLift += 2;
		}
	}

	void updateCharacterPath(uint32 &ptr, int16 &frame, int16 &x, int16 &y) {
		if (READ_BE_UINT32(_data.data() + ptr) & 0x80000000) {
			if (_phaseDone != 2) {
				_phaseDone = 2;
				_frameCounter = 0;
			}
			return;
		}
		frame = (int16)READ_BE_UINT16(_data.data() + ptr);
		x = (int16)READ_BE_UINT16(_data.data() + ptr + 2);
		y = (int16)READ_BE_UINT16(_data.data() + ptr + 4);
		_selectionX = x;
		_selectionY = y;
		ptr += 6;
	}

	void acceptSelection(int key) {
		if (_choiceState != 0)
			return;
		_choice = key;
		_selectionLift = 1;
		_choiceState = (key == 1) ? -2 : -1;
	}

	void setSelectionMouseEnabled(bool enabled) {
		if (enabled) {
			CursorMan.setDefaultArrowCursor();
			g_system->lockMouse(false);
			CursorMan.showMouse(true);
		} else {
			g_system->lockMouse(true);
			CursorMan.showMouse(false);
		}
	}

	uint32 paletteColor(uint16 value) const {
		byte r = ((value >> 8) & 0xf) * 0x11;
		byte g = ((value >> 4) & 0xf) * 0x11;
		byte b = (value & 0xf) * 0x11;
		return _engine->_gfx->_texturePixelFormat.ARGBToColor(0xff, r, g, b);
	}

	int displayFrame(bool acceptSelection, bool acceptAnyKey, int ticks = 1) {
		int key = pumpEvents(acceptSelection, acceptAnyKey);

		Graphics::Surface surface;
		surface.create(kWidth, kHeight, _engine->_gfx->_texturePixelFormat);
		const Common::Array<byte> &buf = _screen[_displayBuffer];
		uint32 colors[32];
		for (int i = 0; i < 32; i++)
			colors[i] = i < 16 ? paletteColor(_palette[i]) : paletteColor(0);

		for (int y = 0; y < kHeight; y++) {
			int row = y * kRowBytes;
			for (int x = 0; x < kWidth; x++) {
				int byteOffset = row + (x >> 3);
				byte bit = 0x80 >> (x & 7);
				byte color = 0;
				for (int plane = 0; plane < 5; plane++) {
					if (buf[byteOffset + plane * kPlaneBytes] & bit)
						color |= 1 << plane;
				}
				surface.setPixel(x, y, colors[color]);
			}
		}

		_engine->_gfx->clear(0, 0, 0, true);
		_engine->drawFullscreenSurface(&surface);
		_engine->_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(kVbiDelayMillis * ticks);
		surface.free();
		return key;
	}

	int pumpEvents(bool acceptSelection, bool acceptAnyKey) {
		int key = 0;
		Common::Event event;
		while (_engine->_eventManager->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				_aborted = true;
				_engine->quitGame();
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_engine->_gfx->computeScreenViewport();
				_engine->_gfx->clear(0, 0, 0, true);
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_1)
					key = 1;
				else if (event.kbd.keycode == Common::KEYCODE_2)
					key = 2;
				else if (acceptAnyKey)
					key = 3;
				break;
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
				if (acceptSelection) {
					int mouseX = kWidth * event.mouse.x / g_system->getWidth();
					key = mouseX < selectionSplitX() ? 1 : 2;
				} else if (acceptAnyKey) {
					key = 3;
				}
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionSelectPrince)
					key = 1;
				else if (event.customType == kActionSelectPrincess)
					key = 2;
				else if (event.customType == kActionSkip && acceptAnyKey)
					key = 3;
				break;
			default:
				break;
			}
		}
		if (!acceptSelection && (key == 1 || key == 2) && acceptAnyKey)
			return key;
		if (!acceptSelection && (key == 1 || key == 2))
			return 0;
		return key;
	}
};

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesVertical(Common::SeekableReadStream *file, int widthInBytes, int height) {
	Graphics::ManagedSurface *surface;
	surface = new Graphics::ManagedSurface();
	surface->create(widthInBytes * 8 / 4, height, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, widthInBytes * 8 / 4, height), 0);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 0);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 1);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 2);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 3);
	return surface;
}

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesInternalVertical(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height, int plane) {
	byte *colors = (byte *)malloc(sizeof(byte) * height * width);
	file->read(colors, height * width);

	for (int i = 0; i < height * width; i++) {
		byte color = colors[i];
		for (int n = 0; n < 8; n++) {
			int y = i / width;
			int x = (i % width) * 8 + (7 - n);

			int bit = ((color >> n) & 0x01) << plane;
			int sample = surface->getPixel(x, y) | bit;
			assert(sample < 16);
			surface->setPixel(x, y, sample);
		}
	}
	free(colors);
	return surface;
}

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesInterleaved(Common::SeekableReadStream *file, int widthInWords, int height) {
	int widthInPixels = widthInWords * 16;
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(widthInPixels, height, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, widthInPixels, height), 0);

	for (int y = 0; y < height; y++) {
		for (int col = 0; col < widthInWords; col++) {
			uint16 planes[4];
			for (int p = 0; p < 4; p++)
				planes[p] = file->readUint16BE();

			for (int bit = 0; bit < 16; bit++) {
				int x = col * 16 + (15 - bit);
				byte color = 0;
				for (int p = 0; p < 4; p++) {
					if (planes[p] & (1 << bit))
						color |= (1 << p);
				}
				surface->setPixel(x, y, color);
			}
		}
	}
	return surface;
}

void CastleEngine::loadAssetsAmigaDemo() {
	Common::File file;
	file.open("x");
	if (!file.isOpen())
		error("Failed to open 'x' file");

	_viewArea = Common::Rect(40, 29, 280, 154);
	loadMessagesVariableSize(&file, 0x8bb2, 178);
	loadRiddles(&file, 0x96c8 - 2 - 19 * 2, 19);


	file.seek(0x11eec);
	Common::Array<Graphics::ManagedSurface *> chars;
	Common::Array<Graphics::ManagedSurface *> charsRiddle;
	for (int i = 0; i < 90; i++) {
		Graphics::ManagedSurface *img = loadFrameFromPlanes(&file, 8, 8);
		Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
		imgRiddle->copyFrom(*img);

		chars.push_back(img);
		chars[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

		charsRiddle.push_back(imgRiddle);
		charsRiddle[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	}

	_font = Font(chars);
	_font.setCharWidth(9);

	_fontRiddle = Font(charsRiddle);
	_fontRiddle.setCharWidth(9);

	load8bitBinary(&file, 0x162a6, 16);
	for (int i = 0; i < 3; i++) {
		debugC(1, kFreescapeDebugParser, "Continue to parse area index %d at offset %x", _areaMap.size() + i + 1, (int)file.pos());
		Area *newArea = load8bitArea(&file, 16);
		if (newArea) {
			if (!_areaMap.contains(newArea->getAreaID()))
				_areaMap[newArea->getAreaID()] = newArea;
			else
				error("Repeated area ID: %d", newArea->getAreaID());
		} else {
			error("Invalid area %d?", i);
		}
	}

	loadPalettes(&file, 0x151a6);

	// COLOR15 cycling table (mem $8B78, file 0x8B94): 14 entries of 12-bit Amiga RGB + 0xFFFF end.
	// From assembly: interrupt handler at $12BA cycles $DFF19E through this table every 4 frames.
	file.seek(0x8b94);
	while (true) {
		uint16 val = file.readUint16BE();
		if (val == 0xFFFF) break;
		_gfx->_colorCyclingTable.push_back(val);
	}

	file.seek(0x2be96); // Area 255
	_areaMap[255] = load8bitArea(&file, 16);

	file.seek(0x2cf28 + 0x28 - 0x2 + 0x28);
	_border = loadFrameFromPlanesVertical(&file, 160, 200);
	_border->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Mountains skybox panorama used by FUN_3E38.
	// Source at memory 0x406A (file 0x4086), 63 words x 22 rows, interleaved 4-plane.
	file.seek(0x4086);
	_background = loadFrameFromPlanesInterleaved(&file, 63, 22);
	_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Menu image used by drawInfoMenu:
	// Assembly at ~0x1AE0 copies from mem 0x350CA in a 14-word x 116-row loop.
	// File offset = mem + 0x1C header => 0x350E6.
	file.seek(0x350e6);
	_menu = loadFrameFromPlanesInterleaved(&file, 14, 116);
	_menu->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Additional 224x54 menu-related block (memory 0x36B9A, file 0x36BB6).
	// Kept as a separate parsed surface for future use.
	file.seek(0x36bb6);
	_menuButtons = loadFrameFromPlanesInterleaved(&file, 14, 54);
	_menuButtons->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file.seek(0x38952); // Spirit meter indicator background (memory 0x38936)
	_spiritsMeterIndicatorBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 5, 10);
	_spiritsMeterIndicatorBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file.seek(0x38ae2); // Spirit meter indicator (memory 0x38AC6)
	_spiritsMeterIndicatorFrame = loadFrameFromPlanesInterleaved(&file, 1, 10);
	_spiritsMeterIndicatorFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Strength weight sprites (file 0x395F2, 1 word x 14 rows x 4 frames)
	file.seek(0x395f2);
	for (int i = 0; i < 4; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 1, 14);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_strenghtWeightsFrames.push_back(frame);
	}

	// Strength background with bar (file 0x397B2, 5 words x 20 rows)
	//file.seek(0x397b2);
	//_strenghtBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 5, 4);
	//_strenghtBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Eye icon sprites (memory 0x3C096, 12 frames, 16x7 each, interleaved 4-plane)
	// Used for strength/compass display at screen (224, 164). Header at 0x3C08E.
	// TODO: load as separate eye icon member, not _keysBorderFrames
	file.seek(0x3c0b2);
	for (int i = 0; i < 12; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 1, 7);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_keysBorderFrames.push_back(frame);
	}

	// Crawl/Walk/Run + Sound indicators (memory 0x3838A, file 0x383A6, 5 frames, 48x12)
	// Header (6 bytes) + mask (3 words = 6 bytes) + graphics.
	// From assembly: frames 0-2 = crawl/walk/run at (96,118), frames 3-4 = sound off/on at (96,103)
	file.seek(0x383a6 + 6 + 6); // skip header + mask
	{
		_menuCrawlIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuCrawlIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuWalkIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuWalkIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuRunIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuRunIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuFxOffIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuFxOffIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuFxOnIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuFxOnIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
	}

	// Mouse pointer from paired sprites at mem $22E/$276 (file 0x24A/0x292).
	// SPR0 at $22E + SPR1 at $276 form the diagonal arrow cursor.
	// Each: 2 control words + 16 data rows (p0,p1 interleaved) + end marker = 72 bytes.
	// SPR0 contributes color bits 0-1, SPR1 contributes bits 2-3 (4-bit combined).
	{
		_cursorW = 16;
		_cursorH = 16;
		_cursorData = new byte[16 * 16];
		memset(_cursorData, 0, 16 * 16);
		// Read SPR0 (bits 0-1)
		file.seek(0x24A + 4); // skip control
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_cursorData[row * 16 + bit] = c;
			}
		}
		// Overlay SPR1 (bits 2-3)
		file.seek(0x292 + 4); // skip control
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_cursorData[row * 16 + bit] |= (c << 2);
			}
		}
	}

	// Crosshair pointer from paired sprites at mem $19E/$1E6 (file 0x1BA/0x202).
	// Used outside the view area. Same format as diagonal arrow.
	{
		_crosshairData = new byte[16 * 16];
		memset(_crosshairData, 0, 16 * 16);
		file.seek(0x1BA + 4);
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_crosshairData[row * 16 + bit] = c;
			}
		}
		file.seek(0x202 + 4);
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_crosshairData[row * 16 + bit] |= (c << 2);
			}
		}
	}

	// Flag animation (memory 0x3C340, 5 frames, 32x11 each, interleaved 4-plane)
	file.seek(0x3c35c);
	for (int i = 0; i < 5; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 2, 11);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_flagFrames.push_back(frame);
	}

	// Riddle mask (memory 0x3C6DA, file 0x3C6F6): 16 words, one per 16-pixel column.
	// Applied per-pixel: frame_pixel = (mask_bit == 1) ? frame_pixel : 0.
	// Same mask for every row. Trims the frame edges for proper compositing.
	file.seek(0x3c6f6);
	uint16 riddleMask[16];
	for (int i = 0; i < 16; i++)
		riddleMask[i] = file.readUint16BE();

	// Riddle frames (memory 0x3C6FA: top 20 rows + bg 1 row + bottom 8 rows, 256px wide)
	file.seek(0x3c716);
	_riddleTopFrame = loadFrameFromPlanesInterleaved(&file, 16, 20);
	_riddleBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 16, 1);
	_riddleBottomFrame = loadFrameFromPlanesInterleaved(&file, 16, 8);

	// Apply mask to CLUT8 frames before palette conversion
	Graphics::ManagedSurface *riddleFrames[] = {_riddleTopFrame, _riddleBackgroundFrame, _riddleBottomFrame};
	for (int f = 0; f < 3; f++) {
		Graphics::ManagedSurface *frame = riddleFrames[f];
		for (int y = 0; y < frame->h; y++) {
			for (int x = 0; x < frame->w; x++) {
				int col = x / 16;
				int bit = 15 - (x % 16);
				if (!(riddleMask[col] & (1 << bit)))
					frame->setPixel(x, y, 0);
			}
		}
	}

	_riddleTopFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBottomFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);

	// Castle gate (game over background frame)
	// Pixel data: 43 rows × 96 bytes (16 columns × 3 words) at file 0x39AE2
	// Mask data: 43 rows × 32 bytes (16 words) at file 0x3AB02
	// FUN_2CCA tiles 24 top rows + 19 bottom rows into a 256×120 gate image
	{
		static const int kTopRows = 24;
		static const int kBottomRows = 19;
		static const int kTotalSrcRows = kTopRows + kBottomRows;
		static const int kColumnsPerRow = 16;
		static const int kPixelBytesPerRow = kColumnsPerRow * 6; // 3 words × 2 bytes
		static const int kMaskBytesPerRow = kColumnsPerRow * 2;  // 1 word × 2 bytes
		static const int kGateWidth = 256;
		static const int kGateHeight = 120;

		byte pixelData[kTotalSrcRows * kPixelBytesPerRow];
		byte maskData[kTotalSrcRows * kMaskBytesPerRow];

		file.seek(0x39AE2);
		file.read(pixelData, sizeof(pixelData));
		file.seek(0x3AB02);
		file.read(maskData, sizeof(maskData));

		uint32 keyColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x24, 0xA5);
		uint32 paletteColors[8];
		for (int i = 0; i < 8; i++)
			paletteColors[i] = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
				kAmigaCastlePalette[i][0], kAmigaCastlePalette[i][1], kAmigaCastlePalette[i][2]);

		_gameOverBackgroundFrame = new Graphics::ManagedSurface();
		_gameOverBackgroundFrame->create(kGateWidth, kGateHeight, _gfx->_texturePixelFormat);
		_gameOverBackgroundFrame->fillRect(Common::Rect(0, 0, kGateWidth, kGateHeight), keyColor);

		// Build row mapping: FUN_2CCA tiling for N=120
		// 5 tail rows from top portion (rows 19-23), then 4×24 full top blocks, then 19 bottom rows
		int destRow = 0;
		// Tail of top portion
		for (int r = kTopRows - 5; r < kTopRows; r++) {
			int srcRow = r;
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
				int pOff = srcRow * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
		// 4 full repetitions of top portion (24 rows each)
		for (int block = 0; block < 4; block++) {
			for (int r = 0; r < kTopRows; r++) {
				int srcRow = r;
				for (int col = 0; col < kColumnsPerRow; col++) {
					uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
					int pOff = srcRow * kPixelBytesPerRow + col * 6;
					uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
					uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
					uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
					for (int bit = 15; bit >= 0; bit--) {
						if (!(mask & (1 << bit))) {
							int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
							_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
						}
					}
				}
				destRow++;
			}
		}
		// Bottom portion (19 rows)
		for (int r = 0; r < kBottomRows; r++) {
			int srcRow = kTopRows + r;
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
				int pOff = srcRow * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
	}

	// Load synthesized sound effects from command table
	// Table at file offset 0x1469E (memory 0x14682), 30 entries
	loadSoundsAmigaDemo(&file, 0x1469E, 30);

	// Load embedded ProTracker module for background music
	// Module is at file offset 0x3D5A6 (memory 0x3D58A), ~86260 bytes
	static const int kModOffset = 0x3D5A6;
	file.seek(0, SEEK_END);
	int fileSize = file.pos();
	int modSize = fileSize - kModOffset;
	if (modSize > 0) {
		file.seek(kModOffset);
		_modData.resize(modSize);
		file.read(_modData.data(), modSize);
	}

	file.close();

	_areaMap[2]->_groundColor = 1;
	for (auto &it : _areaMap)
		it._value->addStructure(_areaMap[255]);
}

void CastleEngine::loadAssetsAmigaFullGame() {
	Common::File file;
	file.open("x");
	if (!file.isOpen())
		error("Failed to open 'x' file");

	_viewArea = Common::Rect(40, 29, 280, 154);
	loadMessagesVariableSize(&file, 0x99ac, 178);
	loadRiddles(&file, 0xa476, 19);

	file.seek(0x11540);
	Common::Array<Graphics::ManagedSurface *> chars;
	Common::Array<Graphics::ManagedSurface *> charsRiddle;
	for (int i = 0; i < 90; i++) {
		Graphics::ManagedSurface *img = loadFrameFromPlanes(&file, 8, 8);
		Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
		imgRiddle->copyFrom(*img);

		chars.push_back(img);
		chars[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

		charsRiddle.push_back(imgRiddle);
		charsRiddle[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	}

	_font = Font(chars);
	_font.setCharWidth(9);

	_fontRiddle = Font(charsRiddle);
	_fontRiddle.setCharWidth(9);

	load8bitBinary(&file, 0x158fa, 16);
	for (int i = 0; i < 3; i++) {
		debugC(1, kFreescapeDebugParser, "Continue to parse area index %d at offset %x", _areaMap.size() + i + 1, (int)file.pos());
		Area *newArea = load8bitArea(&file, 16);
		if (newArea) {
			if (!_areaMap.contains(newArea->getAreaID()))
				_areaMap[newArea->getAreaID()] = newArea;
			else
				error("Repeated area ID: %d", newArea->getAreaID());
		} else {
			error("Invalid area %d?", i);
		}
	}

	loadPalettes(&file, 0x147fa);

	// COLOR15 cycling table: same format as the demo, terminated by 0xFFFF.
	file.seek(0x998e);
	while (true) {
		uint16 val = file.readUint16BE();
		if (val == 0xFFFF) break;
		_gfx->_colorCyclingTable.push_back(val);
	}

	file.seek(0x2b4ea); // Area 255
	_areaMap[255] = load8bitArea(&file, 16);

	// Border NEO image (demo loaded at 0x2cf28 + 0x28 - 0x2 + 0x28 = 0x2cf76)
	file.seek(0x2c5ca);
	_border = loadFrameFromPlanesVertical(&file, 160, 200);
	_border->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// End-game throne picture. The original executable opens "W" during the
	// escaped ending and displays the first 114 rows as a 16-word-wide
	// interleaved Amiga bitplane image.
	Common::File endGameFile;
	if (endGameFile.open("w")) {
		_endGameBackgroundFrame = loadFrameFromPlanesInterleaved(&endGameFile, 16, 114);
		_endGameBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		endGameFile.close();
	}

	// Mountains panorama (63 words × 22 rows × 4 planes, interleaved).
	file.seek(0x49c8);
	_background = loadFrameFromPlanesInterleaved(&file, 63, 22);
	_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Info menu image (14 words × 116 rows).
	file.seek(0x3473a);
	_menu = loadFrameFromPlanesInterleaved(&file, 14, 116);
	_menu->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Additional 224×54 menu-related block.
	file.seek(0x3620a);
	_menuButtons = loadFrameFromPlanesInterleaved(&file, 14, 54);
	_menuButtons->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file.seek(0x37fa6); // Spirit meter indicator background
	_spiritsMeterIndicatorBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 5, 10);
	_spiritsMeterIndicatorBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file.seek(0x38136); // Spirit meter indicator
	_spiritsMeterIndicatorFrame = loadFrameFromPlanesInterleaved(&file, 1, 10);
	_spiritsMeterIndicatorFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Strength weight sprites: 4 frames × 1 word × 14 rows.
	file.seek(0x38c46);
	for (int i = 0; i < 4; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 1, 14);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_strenghtWeightsFrames.push_back(frame);
	}

	// Eye icon sprites: 12 frames × 1 word × 7 rows. Header at 0x3b6fe.
	file.seek(0x3b706);
	for (int i = 0; i < 12; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 1, 7);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_keysBorderFrames.push_back(frame);
	}

	// Crawl/Walk/Run + Sound indicators: 5 frames × 3 words × 12 rows,
	// preceded by a 6-byte header and a 6-byte mask (skipped here).
	file.seek(0x379fa + 6 + 6);
	{
		_menuCrawlIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuCrawlIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuWalkIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuWalkIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuRunIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuRunIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuFxOffIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuFxOffIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuFxOnIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuFxOnIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
	}

	// Mouse pointer and crosshair sprites (paired SPR0/SPR1). These live
	// in the TEXT section at the very top of the binary and are at the
	// same file offsets as the demo (no shift).
	{
		_cursorW = 16;
		_cursorH = 16;
		_cursorData = new byte[16 * 16];
		memset(_cursorData, 0, 16 * 16);
		file.seek(0x24A + 4); // SPR0 (bits 0-1)
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_cursorData[row * 16 + bit] = c;
			}
		}
		file.seek(0x292 + 4); // SPR1 (bits 2-3)
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_cursorData[row * 16 + bit] |= (c << 2);
			}
		}
	}
	{
		_crosshairData = new byte[16 * 16];
		memset(_crosshairData, 0, 16 * 16);
		file.seek(0x1BA + 4);
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_crosshairData[row * 16 + bit] = c;
			}
		}
		file.seek(0x202 + 4);
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_crosshairData[row * 16 + bit] |= (c << 2);
			}
		}
	}

	// Flag animation: 5 frames × 2 words × 11 rows.
	file.seek(0x3b9b0);
	for (int i = 0; i < 5; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 2, 11);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_flagFrames.push_back(frame);
	}

	// Riddle mask + frames (see demo loader for layout details).
	file.seek(0x3bd4a);
	uint16 riddleMask[16];
	for (int i = 0; i < 16; i++)
		riddleMask[i] = file.readUint16BE();

	file.seek(0x3bd6a);
	_riddleTopFrame = loadFrameFromPlanesInterleaved(&file, 16, 20);
	_riddleBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 16, 1);
	_riddleBottomFrame = loadFrameFromPlanesInterleaved(&file, 16, 8);

	Graphics::ManagedSurface *riddleFrames[] = {_riddleTopFrame, _riddleBackgroundFrame, _riddleBottomFrame};
	for (int f = 0; f < 3; f++) {
		Graphics::ManagedSurface *frame = riddleFrames[f];
		for (int y = 0; y < frame->h; y++) {
			for (int x = 0; x < frame->w; x++) {
				int col = x / 16;
				int bit = 15 - (x % 16);
				if (!(riddleMask[col] & (1 << bit)))
					frame->setPixel(x, y, 0);
			}
		}
	}

	_riddleTopFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBottomFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);

	// Castle gate tiles (24 top rows + 19 bottom rows, see demo loader).
	{
		static const int kTopRows = 24;
		static const int kBottomRows = 19;
		static const int kTotalSrcRows = kTopRows + kBottomRows;
		static const int kColumnsPerRow = 16;
		static const int kPixelBytesPerRow = kColumnsPerRow * 6;
		static const int kMaskBytesPerRow = kColumnsPerRow * 2;
		static const int kGateWidth = 256;
		static const int kGateHeight = 120;

		byte pixelData[kTotalSrcRows * kPixelBytesPerRow];
		byte maskData[kTotalSrcRows * kMaskBytesPerRow];

		file.seek(0x39136);
		file.read(pixelData, sizeof(pixelData));
		file.seek(0x3a156);
		file.read(maskData, sizeof(maskData));

		uint32 keyColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x24, 0xA5);
		uint32 paletteColors[8];
		for (int i = 0; i < 8; i++)
			paletteColors[i] = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
				kAmigaCastlePalette[i][0], kAmigaCastlePalette[i][1], kAmigaCastlePalette[i][2]);

		_gameOverBackgroundFrame = new Graphics::ManagedSurface();
		_gameOverBackgroundFrame->create(kGateWidth, kGateHeight, _gfx->_texturePixelFormat);
		_gameOverBackgroundFrame->fillRect(Common::Rect(0, 0, kGateWidth, kGateHeight), keyColor);

		int destRow = 0;
		for (int r = kTopRows - 5; r < kTopRows; r++) {
			int srcRow = r;
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
				int pOff = srcRow * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
		for (int block = 0; block < 4; block++) {
			for (int r = 0; r < kTopRows; r++) {
				int srcRow = r;
				for (int col = 0; col < kColumnsPerRow; col++) {
					uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
					int pOff = srcRow * kPixelBytesPerRow + col * 6;
					uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
					uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
					uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
					for (int bit = 15; bit >= 0; bit--) {
						if (!(mask & (1 << bit))) {
							int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
							_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
						}
					}
				}
				destRow++;
			}
		}
		for (int r = 0; r < kBottomRows; r++) {
			int srcRow = kTopRows + r;
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
				int pOff = srcRow * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
	}

	// Sound effects command table (30 entries). Pass the full-game MOD
	// offset so DMA sample extraction reads from the right place — the
	// demo's 0x3D5A6 hardcoded default is wrong for the full binary.
	loadSoundsAmigaDemo(&file, 0x13cf2, 30, 0x3cbfa);

	// Embedded ProTracker module for background music.
	static const int kModOffset = 0x3cbfa;
	file.seek(0, SEEK_END);
	int fileSize = file.pos();
	int modSize = fileSize - kModOffset;
	if (modSize > 0) {
		file.seek(kModOffset);
		_modData.resize(modSize);
		file.read(_modData.data(), modSize);
	}

	file.close();

	_areaMap[2]->_groundColor = 1;
	for (auto &it : _areaMap)
		it._value->addStructure(_areaMap[255]);
}

bool CastleEngine::playAmigaIntro() {
	Common::File introFile;
	if (!introFile.open("intro"))
		return false;

	introFile.seek(0, SEEK_END);
	int introFileSize = introFile.pos();

	Common::Array<byte> introText;
	if (introFileSize > 0x1c) {
		introFile.seek(0);
		uint16 magic = introFile.readUint16BE();
		uint32 textSize = introFile.readUint32BE();
		if (magic == 0x601a) {
			if (textSize == 0 || textSize + 0x1c > (uint32)introFileSize)
				return false;
			introText.resize(textSize);
			introFile.seek(0x1c);
			introFile.read(introText.data(), textSize);
		}
	}

	if (introText.empty()) {
		if (introFileSize <= 0)
			return false;
		introText.resize(introFileSize);
		introFile.seek(0);
		introFile.read(introText.data(), introFileSize);
	}
	introFile.close();

	if (introText.size() < 0x1c00)
		return false;

	Common::Array<byte> introMusic;
	Common::File introMusicFile;
	if (introMusicFile.open("musicdat")) {
		introMusicFile.seek(0, SEEK_END);
		int introMusicSize = introMusicFile.pos();
		introMusicFile.seek(0);
		introMusic.resize(introMusicSize);
		if (introMusicSize > 0)
			introMusicFile.read(introMusic.data(), introMusicSize);
		introMusicFile.close();
	}

	Audio::SoundHandle introMusicHandle;
	if (!introMusic.empty()) {
		Common::MemoryReadStream modStream(introMusic.data(), introMusic.size());
		Audio::AudioStream *musicStream = Audio::makeProtrackerStream(&modStream);
		if (musicStream)
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &introMusicHandle, musicStream);
	}

	bool selectedPrincess = false;
	CastleAmigaIntroPlayer player(this, introText);
	bool played = player.run(selectedPrincess);
	if (played)
		_selectedPrincess = selectedPrincess;

	if (_mixer->isSoundHandleActive(introMusicHandle))
		_mixer->stopHandle(introMusicHandle);

	_gfx->clear(0, 0, 0, true);
	return played;
}

void CastleEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	drawLiftingGate(surface);
	drawDroppingGate(surface);

	uint8 r, g, b;
	_gfx->readFromPalette(15, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	Common::Rect backRect(97, 181, 232, 190);
	surface->fillRect(backRect, black);

	Common::String message;
	int deadline = -1;
	getLatestMessages(message, deadline);
	if (deadline > 0 && deadline <= _countdown) {
		drawStringInSurface(message, 97, 182, front, black, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_gameStateControl != kFreescapeGameStateEnd) {
			if (ghostInArea())
				drawStringInSurface(_ghostInAreaMessage, 97, 182, front, black, surface);
			else
				drawStringInSurface(_currentArea->_name, 97, 182, front, black, surface);
		}
	}

	// TODO: Draw collected keys - key sprites location in binary still unknown

	// Draw flag animation at (288, 5)
	if (!_flagFrames.empty()) {
		int flagFrameIndex = (_ticks / 10) % _flagFrames.size();
		surface->copyRectToSurface(*_flagFrames[flagFrameIndex], 288, 5,
			Common::Rect(0, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));
	}

	// Draw energy meter (strength) - background placed at (0, 154) to match border
	drawEnergyMeter(surface, Common::Point(40, 158));

	// Draw spirit meter
	if (_spiritsMeterIndicatorBackgroundFrame)
		surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorBackgroundFrame, 128, 160,
			Common::Rect(0, 0, _spiritsMeterIndicatorBackgroundFrame->w, _spiritsMeterIndicatorBackgroundFrame->h));

	if (_spiritsMeterIndicatorFrame) {
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 128 + _spiritsMeterPosition, 160,
			Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h), black);
	}
}

} // End of namespace Freescape
