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

#include "cryo/defs.h"
#include "cryo/cryo.h"
#include "cryo/platdefs.h"
#include "cryo/cryolib.h"
#include "cryo/eden.h"
#include "cryo/sound.h"

namespace Cryo {

// Original name: noclipax
void EdenGame::drawSprite(int16 index, int16 x, int16 y, bool withBlack, bool onSubtitle) {
	uint16 width = (!onSubtitle) ? 640 : _subtitlesXWidth;
	byte *pix = _bankData;
	byte *buf = (!onSubtitle) ? _mainViewBuf : _subtitlesViewBuf;
	byte *scr = buf + x + y * width;
	if (_curBankNum != 117 && (!_noPalette || withBlack || onSubtitle)) {
		if (READ_LE_UINT16(pix) > 2)
			readPalette(pix + 2);
	}
	pix += READ_LE_UINT16(pix);
	pix += READ_LE_UINT16(pix + index * 2);
	//  int16   height:9
	//  int16   pad:6;
	//  int16   flag:1;
	byte h0 = *pix++;
	byte h1 = *pix++;
	int16 w = ((h1 & 1) << 8) | h0;
	int16 h = *pix++;
	byte mode = *pix++;
	if (mode != 0xFF && mode != 0xFE)
		return;
	if (y + h > 200 && !onSubtitle)
		h -= (y + h - 200);
	if (h1 & 0x80) {
		// compressed
		for (; h-- > 0;) {
			for (int16 ww = w; ww > 0;) {
				byte c = *pix++;
				if (c >= 0x80) {
					if (c == 0x80) {
						byte fill = *pix++;
						if (fill == 0 && !withBlack) {
							scr += 128 + 1;
							ww -= 128 + 1;
						}
						else {
							byte runVal;
							*scr++ = fill;  //TODO: wha?
							*scr++ = fill;
							ww -= 128 + 1;
							for (runVal = 127; runVal--;)
								*scr++ = fill;
						}
					}
					else {
						byte fill = *pix++;
						byte runVal = 255 - c + 2;
						ww -= runVal;
						if (fill == 0 && !withBlack)
							scr += runVal;
						else
							for (; runVal--;)
								*scr++ = fill;
					}
				}
				else {
					byte runVal = c + 1;
					ww -= runVal;
					for (; runVal--;) {
						byte p = *pix++;
						if (p == 0 && !withBlack)
							scr++;
						else
							*scr++ = p;
					}
				}
			}
			scr += width - w;
		}
	}
	else {
		// uncompressed
		for (; h--;) {
			for (int16 ww = w; ww--;) {
				byte p = *pix++;
				if (p == 0 && !withBlack)
					scr++;
				else
					*scr++ = p;
			}
			scr += width - w;
		}
	}
}

void EdenGame::sundcurs(int16 x, int16 y) {
	byte *keep = _cursKeepBuf;
	x = CLIP<int16>(x - 4, 0, 640 - 48);
	y = CLIP<int16>(y - 4, 0, 200 - 48);
	_cursKeepPos = Common::Point(x, y);
	byte *scr = _mainViewBuf + _cursKeepPos.x + _cursKeepPos.y * 640;
	for (int16 h = 48; h--;) {
		for (int16 w = 48; w--;)
			*keep++ = *scr++;
		scr += 640 - 48;
	}
	_cursorSaved = true;
}

void EdenGame::rundcurs() {
	byte *keep = _cursKeepBuf;
	byte *scr = _mainViewBuf + _cursKeepPos.x + _cursKeepPos.y * 640;
	if (!_cursorSaved || (_cursKeepPos == Common::Point(-1, -1)))  //TODO ...
		return;

	for (int16 h = 48; h--;) {
		for (int16 w = 48; w--;)
			*scr++ = *keep++;
		scr += 640 - 48;
	}
}

void EdenGame::getglow(int16 x, int16 y, int16 w, int16 h) {
	byte *scr = _mainViewBuf + x + y * 640;
	byte *gl = _glowBuffer;
	_glowX = x;
	_glowY = y;
	_glowW = w;
	_glowH = h;
	for (; h--;) {
		for (int16 ww = w; ww--;)
			*gl++ = *scr++;
		scr += 640 - w;
	}
}

void EdenGame::unglow() {
	byte *gl = _glowBuffer;
	byte *scr = _mainViewBuf + _glowX + _glowY * 640;
	if (_glowX < 0 || _glowY < 0)   //TODO: move it up
		return;
	for (; _glowH--;) {
		for (int16 ww = _glowW; ww--;)
			*scr++ = *gl++;
		scr += 640 - _glowW;
	}
}

void EdenGame::glow(int16 index) {
	// byte pixbase;
	byte *pix = _bankData;

	index += 9;
	pix += READ_LE_UINT16(pix);
	pix += READ_LE_UINT16(pix + index * 2);
	//  int16   height:9
	//  int16   pad:6;
	//  int16   flag:1;
	byte h0 = *pix++;
	byte h1 = *pix++;
	int16 w = ((h1 & 1) << 8) | h0;
	int16 h = *pix++;
	byte mode = *pix++;
	if (mode != 0xFF && mode != 0xFE)
		return;

	int16 x = _cursorPosX + _scrollPos - 38;
	int16 y = _cursorPosY - 28;
	int16 ex = _globals->_frescoeWidth + 320;

	if (x + w <= 0 || x >= ex || y + h <= 0 || y >= 176)
		return;

	int16 dx;
	if (x < 0) {
		dx = -x;
		x = 0;
	}
	else if (x + w > ex)
		dx = x + w - ex;
	else
		dx = 0;

	int16 dy = 0;
	if (y < 16) {
		dy = 16 - y;
		y = 16;
	}
	else if (y + h > 175)
		dy = y + h - 175;

	int16 pstride = dx;
	int16 sstride = 640 - (w - dx);
	if (y == 16)
		pix += w * dy;
	if (x == 0)
		pix += dx;

	byte *scr = _mainViewBuf + x + y * 640;

	w -= dx;
	h -= dy;

	getglow(x, y, w, h);

	for (; h--;) {
		for (int16 ww = w; ww--;) {
			byte p = *pix++;
			if (p == 0)
				scr++;
			else
				*scr++ += p << 4;
		}
		pix += pstride;
		scr += sstride;
	}
}

// Original name : blackbars
void EdenGame::drawBlackBars() {
	byte *scr = _mainViewBuf;
	for (int16 y = 0; y < 16; y++) {
		for (int16 x = 0; x < 640; x++)
			*scr++ = 0;
	}

	scr += 640 * (200 - 16 - 24);
	for (int16 y = 0; y < 24; y++) {
		for (int16 x = 0; x < 640; x++)
			*scr++ = 0;
	}
}

// Original name: bars_out
void EdenGame::hideBars() {
	if (_showBlackBars)
		return;

	display();
	_underTopBarScreenRect.left = _scrollPos;
	_underTopBarScreenRect.right = _scrollPos + 320 - 1;
	CLBlitter_CopyViewRect(_mainView, _underBarsView, &_underTopBarScreenRect, &_underTopBarBackupRect);
	_underBottomBarScreenRect.left = _underTopBarScreenRect.left;
	_underBottomBarScreenRect.right = _underTopBarScreenRect.right;
	CLBlitter_CopyViewRect(_mainView, _underBarsView, &_underBottomBarScreenRect, &_underBottomBarBackupRect);
	int16 r19 = 14;   // TODO - init in decl?
	int16 r20 = 176;
	int16 r25 = 14;
	int16 r24 = 21;
	_underTopBarScreenRect.left = 0;
	_underTopBarScreenRect.right = 320 - 1;
	_underTopBarBackupRect.left = _scrollPos;
	_underTopBarBackupRect.right = _scrollPos + 320 - 1;
	unsigned int *scr40, *scr41, *scr42;
	while (r24 > 0) {
		if (r25 > 0) {
			_underTopBarScreenRect.top = 16 - r25;
			_underTopBarScreenRect.bottom = 16 - 1;
			_underTopBarBackupRect.top = 0;
			_underTopBarBackupRect.bottom = r25 - 1;
			CLBlitter_CopyViewRect(_underBarsView, _mainView, &_underTopBarScreenRect, &_underTopBarBackupRect);
			scr40 = ((unsigned int *)_mainViewBuf) + r19 * 640 / 4;
			scr41 = scr40 + 640 / 4;
			for (int i = 0; i < 320; i += 4) {
				*scr40++ = 0;
				*scr41++ = 0;
			}
		}
		_underTopBarScreenRect.top = 16;
		_underTopBarScreenRect.bottom = r24 + 16 - 1;
		_underTopBarBackupRect.top = 200 - r24;
		_underTopBarBackupRect.bottom = 200 - 1;
		CLBlitter_CopyViewRect(_underBarsView, _mainView, &_underTopBarScreenRect, &_underTopBarBackupRect);
		scr40 = ((unsigned int *)_mainViewBuf) + r20 * 640 / 4;
		scr41 = scr40 + 640 / 4;
		scr42 = scr41 + 640 / 4;
		for (int i = 0; i < 320; i += 4) {
			*scr40++ = 0;
			*scr41++ = 0;
			*scr42++ = 0;
		}
		r19 -= 2;
		r20 += 3;
		r25 -= 2;
		r24 -= 3;
		display();
	}
	scr40 = (unsigned int *)_mainViewBuf;
	scr41 = scr40 + 640 / 4;
	for (int i = 0; i < 320; i += 4) {
		*scr40++ = 0;
		*scr41++ = 0;
	}
	scr40 = ((unsigned int *)_mainViewBuf) + r20 * 640 / 4;
	scr41 = scr40 + 640 / 4;
	scr42 = scr41 + 640 / 4;
	for (int i = 0; i < 320; i += 4) {
		*scr40++ = 0;
		*scr41++ = 0;
		*scr42++ = 0;
	}
	display();
	initRects();
	_showBlackBars = true;
}

// Original name: bars_in
void EdenGame::showBars() {
	if (!_showBlackBars)
		return;

	drawBlackBars();
	int16 r29 = 2;
	int16 r28 = 2;
	_underTopBarScreenRect.left = 0;
	_underTopBarScreenRect.right = 320 - 1;
	_underTopBarBackupRect.left = _scrollPos;
	_underTopBarBackupRect.right = _scrollPos + 320 - 1;
	while (r28 < 24) {
		if (r29 <= 16) {
			_underTopBarScreenRect.top = 16 - r29;
			_underTopBarScreenRect.bottom = 16 - 1;
			_underTopBarBackupRect.top = 0;
			_underTopBarBackupRect.bottom = r29 - 1;
			CLBlitter_CopyViewRect(_underBarsView, _mainView, &_underTopBarScreenRect, &_underTopBarBackupRect);
		}
		_underTopBarScreenRect.top = 16;
		_underTopBarScreenRect.bottom = 16 + r28;
		_underTopBarBackupRect.top = 200 - 1 - r28;
		_underTopBarBackupRect.bottom = 200 - 1;
		CLBlitter_CopyViewRect(_underBarsView, _mainView, &_underTopBarScreenRect, &_underTopBarBackupRect);
		r29 += 2;
		r28 += 3;
		display();
	}
	initRects();
	_showBlackBars = false;
}

// Original name: af_image
void EdenGame::displayImage() {
	byte *img = _imageDesc + 200;

	int16 count = READ_LE_UINT16(img);
	if (!count)
		return;

	byte *img_start = img;
	byte *curimg = _imageDesc;

	img += 2;
	count *= 3;
	while (count--)
		*curimg++ = *img++;
	img = img_start;
	count = READ_LE_UINT16(img);
	img += 2;
	/////// draw it
	while (count--) {
		uint16 index = *img++;
		uint16 x = *img++ + _gameIcons[0].sx;
		uint16 y = *img++ + _gameIcons[0].sy;
		byte *pix = _bankData;
		byte *scr = _mainViewBuf + x + y * 640;
		index--;
		if (READ_LE_UINT16(pix) > 2)
			readPalette(pix + 2);
		pix += READ_LE_UINT16(pix);
		pix += READ_LE_UINT16(pix + index * 2);
		//  int16   height:9
		//  int16   pad:6;
		//  int16   flag:1;
		byte h0 = *pix++;
		byte h1 = *pix++;
		int16 w = ((h1 & 1) << 8) | h0;
		int16 h = *pix++;
		byte mode = *pix++;
		if (mode != 0xFF && mode != 0xFE)
			continue;   //TODO: enclosing block?
		if (h1 & 0x80) {
			// compressed
			for (; h-- > 0;) {
				for (int16 ww = w; ww > 0;) {
					byte c = *pix++;
					if (c >= 0x80) {
						if (c == 0x80) {
							byte fill = *pix++;
							if (fill == 0) {
								scr += 128 + 1;
								ww -= 128 + 1;
							}
							else {
								byte runVal;
								*scr++ = fill;  //TODO: wha?
								*scr++ = fill;
								ww -= 128 + 1;
								for (runVal = 127; runVal--;)
									*scr++ = fill;
							}
						}
						else {
							byte fill = *pix++;
							byte runVal = 255 - c + 2;
							ww -= runVal;
							if (fill == 0)
								scr += runVal;
							else {
								for (; runVal--;)
									*scr++ = fill;
							}
						}
					}
					else {
						byte runVal = c + 1;
						ww -= runVal;
						for (; runVal--;) {
							byte p = *pix++;
							if (p == 0)
								scr++;
							else
								*scr++ = p;
						}
					}
				}
				scr += 640 - w;
			}
		}
		else {
			// uncompressed
			for (; h--;) {
				for (int16 ww = w; ww--;) {
					byte p = *pix++;
					if (p == 0)
						scr++;
					else
						*scr++ = p;
				}
				scr += 640 - w;
			}
		}
	}
}

// Original name: af_subtitle
void EdenGame::displaySubtitles() {
	byte *src = _subtitlesViewBuf;
	byte *dst = _mainViewBuf;
	int16 y;
	if (_globals->_displayFlags & DisplayFlags::dfFlag2) {
		y = 174;
		if ((_globals->_drawFlags & DrawFlags::drDrawMenu) && _numTextLines == 1)
			y = 167;
		dst += 640 * (y - _numTextLines * FONT_HEIGHT) + _subtitlesXScrMargin;
	}
	else {
		y = 174;
		dst += 640 * (y - _numTextLines * FONT_HEIGHT) + _scrollPos + _subtitlesXScrMargin;
	}
	if (_animationActive && !_personTalking)
		return;
	saveUnderSubtitles(y);
	for (int16 h = 0; h < _numTextLines * FONT_HEIGHT + 1; h++) {
		for (int16 w = 0; w < _subtitlesXWidth; w++) {
			byte c = *src++;
			if (c)
				*dst = c;
			dst++;
		}
		dst += 640 - _subtitlesXWidth;
	}
}

// Original name afsalle1
void EdenGame::displaySingleRoom(Room *room) {
	byte *ptr = (byte *)getElem(_placeRawBuf, room->_id - 1);
	ptr++;
	for (;;) {
		byte b0 = *ptr++;
		byte b1 = *ptr++;
		int16 index = (b1 << 8) | b0;
		if (index == -1)
			break;
		if (index > 0) {
			int16 x = *ptr++ | (((b1 & 0x2) >> 1) << 8);      //TODO: check me
			int16 y = *ptr++;
			ptr++;
			index &= 0x1FF;
			if (!(_globals->_displayFlags & 0x80)) {
				if (index == 1 || _globals->_varF7)
					drawSprite(index - 1, x, y, true);
			}
			_globals->_varF7 = 0;
			continue;
		}
		if (b1 & 0x40) {
			if (b1 & 0x20) {
				bool addIcon = false;
				Icon *icon = _globals->_nextRoomIcon;
				if (b0 < 4) {
					if (_globals->_roomPtr->_exits[b0])
						addIcon = true;
				}
				else if (b0 > 229) {
					if (_globals->_partyOutside & (1 << (b0 - 230)))
						addIcon = true;
				}
				else if (b0 >= 100) {
					debug("add object %d", b0 - 100);
					if (isObjectHere(b0 - 100)) {
						addIcon = true;
						_globals->_varF7 = 1;
					}
				}
				else
					addIcon = true;
				if (addIcon) {
					icon->_actionId = b0;
					icon->_objectId = b0;
					icon->_cursorId = _actionCursors[b0];
					int16 x = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 y = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 ex = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 ey = READ_LE_UINT16(ptr);
					ptr += 2;
					x += _globals->_roomBaseX;
					ex += _globals->_roomBaseX;
					debug("add hotspot at %3d:%3d - %3d:%3d, action = %d", x, y, ex, ey, b0);

					if (_vm->_showHotspots) {
						for (int iii = x; iii < ex; iii++)
							_mainViewBuf[y * 640 + iii] = _mainViewBuf[ey * 640 + iii] = (iii % 2) ? 0 : 255;
						for (int iii = y; iii < ey; iii++)
							_mainViewBuf[iii * 640 + x] = _mainViewBuf[iii * 640 + ex] = (iii % 2) ? 0 : 255;
					}

					icon->sx = x;
					icon->sy = y;
					icon->ex = ex;
					icon->ey = ey;
					_globals->_nextRoomIcon = ++icon;
					icon->sx = -1;
				}
				else
					ptr += 8;
			}
			else
				ptr += 8;
		}
		else
			ptr += 8;
	}
}

// Original name: afsalle
void EdenGame::displayRoom() {
	Room *room = _globals->_roomPtr;
	_globals->_displayFlags = DisplayFlags::dfFlag1;
	_globals->_roomBaseX = 0;
	_globals->_roomBackgroundBankNum = room->_backgroundBankNum;
	if (room->_flags & RoomFlags::rf08) {
		_globals->_displayFlags |= DisplayFlags::dfFlag80;
		if (room->_flags & RoomFlags::rfPanable) {
			// Scrollable room on 2 screens
			_globals->_displayFlags |= DisplayFlags::dfPanable;
			_globals->_varF4 = 0;
			rundcurs();
			saveFriezes();
			useBank(room->_bank - 1);
			drawSprite(0, 0, 16, true);
			useBank(room->_bank);
			drawSprite(0, 320, 16, true);
			displaySingleRoom(room);
			_globals->_roomBaseX = 320;
			displaySingleRoom(room + 1);
		}
		else
			displaySingleRoom(room);
	}
	else {
		//TODO: roomImgBank is garbage here!
		debug("displayRoom: room 0x%X using bank %d", _globals->_roomNum, _globals->_roomImgBank);
		useBank(_globals->_roomImgBank);
		displaySingleRoom(room);
		assert(_vm->_screenView->_pitch == 320);
	}
}

void EdenGame::openWindow() {
	_underBarsView = new View(320, 40);
	_underBarsView->_normal._width = 320;

	_view2 = new View(32, 32);
	_view2Buf = _view2->_bufferPtr;

	_subtitlesView = new View(_subtitlesXWidth, 60);
	_subtitlesViewBuf = _subtitlesView->_bufferPtr;

	_underSubtitlesView = new View(_subtitlesXWidth, 60);
	_underSubtitlesViewBuf = _underSubtitlesView->_bufferPtr;

	_mainView = new View(640, 200);
	_mainView->_normal._width = 320;
	CLBlitter_FillView(_mainView, 0xFFFFFFFF);
	_mainView->setSrcZoomValues(0, 0);
	_mainView->setDisplayZoomValues(640, 400);
	_mainView->centerIn(_vm->_screenView);
	_mainViewBuf = _mainView->_bufferPtr;

	_mouseCenterX = _mainView->_normal._dstLeft + _mainView->_normal._width / 2;
	_mouseCenterY = _mainView->_normal._dstTop + _mainView->_normal._height / 2;
	_vm->setMousePosition(_mouseCenterX, _mouseCenterY);
	_vm->hideMouse();

	_cursorPosX = 320 / 2;
	_cursorPosY = 200 / 2;
}

// Original name: effet1
void EdenGame::displayEffect1() {
	blackRect32();
	setSrcRect(0, 0, 16 - 1, 4 - 1);
	int y = _mainView->_normal._dstTop;
	for (int16 i = 16; i <= 96; i += 4) {
		for (int x = _mainView->_normal._dstLeft; x < _mainView->_normal._dstLeft + 320; x += 16) {
			setDestRect(x, y + i, x + 16 - 1, y + i + 4 - 1);
			CLBlitter_CopyViewRect(_view2, _vm->_screenView, &rect_src, &rect_dst);
			setDestRect(x, y + 192 - i, x + 16 - 1, y + 192 - i + 4 - 1);
			CLBlitter_CopyViewRect(_view2, _vm->_screenView, &rect_src, &rect_dst);
		}
		CLBlitter_UpdateScreen();
		wait(1);
	}
	CLPalette_Send2Screen(_globalPalette, 0, 256);
	_mainView->_normal._height = 2;
	_mainView->_zoom._height = 4;
	int16 ny = _mainView->_normal._dstTop;
	int16 dy = _mainView->_zoom._dstTop;
	for (int16 i = 0; i < 100; i += 2) {
		_mainView->_normal._srcTop = 99 - i;
		_mainView->_zoom._srcTop = 99 - i;
		_mainView->_normal._dstTop = 99 - i + ny;
		_mainView->_zoom._dstTop = (99 - i) * 2 + dy;
		CLBlitter_CopyView2Screen(_mainView);
		_mainView->_normal._srcTop = 100 + i;
		_mainView->_zoom._srcTop = 100 + i;
		_mainView->_normal._dstTop = 100 + i + ny;
		_mainView->_zoom._dstTop = (100 + i) * 2 + dy;
		CLBlitter_CopyView2Screen(_mainView);
		CLBlitter_UpdateScreen();
		wait(1);
	}
	_mainView->_normal._height = 200;
	_mainView->_zoom._height = 400;
	_mainView->_normal._srcTop = 0;
	_mainView->_zoom._srcTop = 0;
	_mainView->_normal._dstTop = ny;
	_mainView->_zoom._dstTop = dy;
	_globals->_varF1 = 0;
}

// Original name: effet2
void EdenGame::displayEffect2() {
	static int16 pattern1[] = { 0, 1, 2, 3, 7, 11, 15, 14, 13, 12, 8, 4, 5, 6, 10, 9 };
	static int16 pattern2[] = { 0, 15, 1, 14, 2, 13, 3, 12, 7, 8, 11, 4, 5, 10, 6, 9 };
	static int16 pattern3[] = { 0, 2, 5, 7, 8, 10, 13, 15, 1, 3, 4, 6, 9, 11, 12, 14 };
	static int16 pattern4[] = { 0, 3, 15, 12, 1, 7, 14, 8, 2, 11, 13, 4, 5, 6, 10, 9 };

	static int eff2pat = 0;
	if (_globals->_var103 == 69) {
		displayEffect4();
		return;
	}
	switch (++eff2pat) {
	case 1:
		colimacon(pattern1);
		break;
	case 2:
		colimacon(pattern2);
		break;
	case 3:
		colimacon(pattern3);
		break;
	case 4:
		colimacon(pattern4);
		eff2pat = 0;
		break;
	}
}

// Original name: effet3
void EdenGame::displayEffect3() {
	CLPalette_GetLastPalette(oldPalette);
	for (uint16 i = 0; i < 6; i++) {
		for (uint16 c = 0; c < 256; c++) {
			newColor.r = oldPalette[c].r >> i;
			newColor.g = oldPalette[c].g >> i;
			newColor.b = oldPalette[c].b >> i;
			CLPalette_SetRGBColor(newPalette, c, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 256);
		wait(1);
	}
	CLBlitter_CopyView2Screen(_mainView);
	for (uint16 i = 0; i < 6; i++) {
		for (uint16 c = 0; c < 256; c++) {
			newColor.r = _globalPalette[c].r >> (5 - i);
			newColor.g = _globalPalette[c].g >> (5 - i);
			newColor.b = _globalPalette[c].b >> (5 - i);
			CLPalette_SetRGBColor(newPalette, c, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 256);
		wait(1);
	}
}

// Original name: effet4
void EdenGame::displayEffect4() {
	byte *scr, *pix, *r24, *r25, *r30, c;
	int16 r17, r23, r16, r18, r19, r22, r27, r31;
	CLPalette_Send2Screen(_globalPalette, 0, 256);

	int16 ww = _vm->_screenView->_pitch;
	int16 x = _mainView->_normal._dstLeft;
	int16 y = _mainView->_normal._dstTop;
	for (int16 i = 32; i > 0; i -= 2) {
		scr = _vm->_screenView->_bufferPtr;
		scr += (y + 16) * ww + x;
		pix = _mainView->_bufferPtr + 16 * 640;
		r17 = 320 / i;
		r23 = 320 - 320 / i * i;  //TODO: 320 % i ?
		r16 = 160 / i;
		r18 = 160 - 160 / i * i;  //TODO: 160 % i ?
		for (r19 = r16; r19 > 0; r19--) {
			r24 = scr;
			r25 = pix;
			for (r22 = r17; r22 > 0; r22--) {
				c = *r25;
				r25 += i;
				r30 = r24;
				for (r27 = i; r27 > 0; r27--) {
					for (r31 = i; r31 > 0; r31--)
						*r30++ = c;
					r30 += ww - i;
				}
				r24 += i;
			}
			if (r23) {
				c = *r25;
				r30 = r24;
				for (r27 = i; r27 > 0; r27--) {
					for (r31 = r23; r31 > 0; r31--)
						*r30++ = c;
					r30 += ww - r23;
				}
			}
			scr += i * ww;
			pix += i * 640;
		}
		if (r18) {
			r24 = scr;
			r25 = pix;
			for (r22 = r17; r22 > 0; r22--) {
				c = *r25;
				r25 += i;
				r30 = r24;
				for (r27 = r18; r27 > 0; r27--) {
					for (r31 = i; r31 > 0; r31--)
						*r30++ = c;
					r30 += ww - i;
				}
				r24 += i;
			}
			if (r23) {
				c = *r25;
				r30 = r24;
				for (r27 = r18; r27 > 0; r27--) {
					for (r31 = r23; r31 > 0; r31--)
						*r30++ = c;
					r30 += ww - r23;
				}
			}
		}
		CLBlitter_UpdateScreen();
		wait(3);
	}
	CLBlitter_CopyView2Screen(_mainView);
}

void EdenGame::clearScreen() {
	int16 ww = _vm->_screenView->_pitch;
	int16 x = _mainView->_normal._dstLeft;
	int16 y = _mainView->_normal._dstTop;
	byte *scr = _vm->_screenView->_bufferPtr;
	scr += (y + 16) * ww + x;
	for (int16 yy = 0; yy < 160; yy++) {
		for (int16 xx = 0; xx < 320; xx++)
			*scr++ = 0;
		scr += ww - 320;
	}
	CLBlitter_UpdateScreen();
}

void EdenGame::colimacon(int16 pattern[16]) {
	int16 p, r27, r25;

	int16 ww = _vm->_screenView->_pitch;
	int16 x = _mainView->_normal._dstLeft;
	int16 y = _mainView->_normal._dstTop;
	byte *scr = _vm->_screenView->_bufferPtr;
	scr += (y + 16) * ww + x;
	for (int16 i = 0; i < 16; i++) {
		p = pattern[i];
		r27 = p % 4 + p / 4 * ww;
		for (int16 j = 0; j < 320 * 160 / 16; j++)
			scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] = 0;
		CLBlitter_UpdateScreen();
		wait(1);
	}
	CLPalette_Send2Screen(_globalPalette, 0, 256);
	byte *pix = _mainView->_bufferPtr;
	x = _mainView->_normal._dstLeft;
	y = _mainView->_normal._dstTop;
	pix += 640 * 16;
	scr = _vm->_screenView->_bufferPtr;
	scr += (y + 16) * ww + x;
	for (int16 i = 0; i < 16; i++) {
		p = pattern[i];
		r25 = p % 4 + p / 4 * 640;
		r27 = p % 4 + p / 4 * ww;
		for (int16 j = 0; j < 320 * 160 / 16; j++)
			scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] =
			pix[j / (320 / 4) * 640 * 4 + j % (320 / 4) * 4 + r25];
		CLBlitter_UpdateScreen();
		wait(1);
	}
}

void EdenGame::fadeToBlack(int delay) {
	CLPalette_GetLastPalette(oldPalette);
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 256; j++) {
			newColor.r = oldPalette[j].r >> i;
			newColor.g = oldPalette[j].g >> i;
			newColor.b = oldPalette[j].b >> i;
			CLPalette_SetRGBColor(newPalette, j, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 256);
		wait(delay);
	}
}

// Original name: fadetoblack128
void EdenGame::fadeToBlackLowPalette(int delay) {
	CLPalette_GetLastPalette(oldPalette);
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 129; j++) { //CHECKME: Should be 128?
			newColor.r = oldPalette[j].r >> i;
			newColor.g = oldPalette[j].g >> i;
			newColor.b = oldPalette[j].b >> i;
			CLPalette_SetRGBColor(newPalette, j, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 128);
		wait(delay);
	}
}

// Original name: fadefromblack128
void EdenGame::fadeFromBlackLowPalette(int delay) {
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 129; j++) { //CHECKME: Should be 128?
			newColor.r = _globalPalette[j].r >> (5 - i);
			newColor.g = _globalPalette[j].g >> (5 - i);
			newColor.b = _globalPalette[j].b >> (5 - i);
			CLPalette_SetRGBColor(newPalette, j, &newColor);
		}
		CLPalette_Send2Screen(newPalette, 0, 128);
		wait(delay);
	}
}

// Original name: rectanglenoir32
void EdenGame::blackRect32() {
	// blacken 32x32 rectangle
	int *pix = (int *)_view2Buf;
	for (int16 i = 0; i < 32; i++) {
		pix[0] = 0;
		pix[1] = 0;
		pix[2] = 0;
		pix[3] = 0;
		pix[4] = 0;
		pix[5] = 0;
		pix[6] = 0;
		pix[7] = 0;
		pix += 32 / 4;
	}
}

void EdenGame::setSrcRect(int16 sx, int16 sy, int16 ex, int16 ey) {
	rect_src = Common::Rect(sx, sy, ex, ey);
}

void EdenGame::setDestRect(int16 sx, int16 sy, int16 ex, int16 ey) {
	rect_dst = Common::Rect(sx, sy, ex, ey);
}

void EdenGame::effetpix() {
	uint16 r25, r18, r31, r30;  //TODO: change to xx/yy

	uint16 ww = _vm->_screenView->_pitch;
	r25 = ww * 80;
	r18 = 640 * 80;
	byte *pix = _mainView->_bufferPtr + 16 * 640;
	int x = _mainView->_normal._dstLeft;
	int y = _mainView->_normal._dstTop;
	byte *scr = _vm->_screenView->_bufferPtr;
	scr += (y + 16) * ww + x;
	int16 r20 = 0x4400;   //TODO
	int16 r27 = 1;
	int16 r26 = 0;
	do {
		char r8 = r27 & 1;
		r27 >>= 1;
		if (r8)
			r27 ^= r20;
		if (r27 < 320 * 80) {
			r31 = r27 / 320;
			r30 = r27 % 320;
			scr[r31 * ww + r30] = 0;
			scr[r31 * ww + r25 + r30] = 0;
			if (++r26 == 960) {
				CLBlitter_UpdateScreen();
				wait(1);
				r26 = 0;
			}
		}
	} while (r27 != 1);
	CLPalette_Send2Screen(_globalPalette, 0, 256);
	r20 = 0x4400;
	r27 = 1;
	r26 = 0;
	do {
		char r8 = r27 & 1;
		r27 >>= 1;
		if (r8)
			r27 ^= r20;
		if (r27 < 320 * 80) {
			r31 = r27 / 320;
			r30 = r27 % 320;
			byte p0 = pix[r31 * 640 + r30];
			byte p1 = pix[r31 * 640 + r18 + r30];
			scr[r31 * ww + r30] = p0;
			scr[r31 * ww + r25 + r30] = p1;
			if (++r26 == 960) {
				CLBlitter_UpdateScreen();
				wait(1);
				r26 = 0;
			}
		}
	} while (r27 != 1);
	assert(_vm->_screenView->_pitch == 320);
}

////// film.c
// Original name: showfilm
void EdenGame::showMovie(char arg1) {
	_vm->_video->readHeader();
	if (_globals->_curVideoNum == 92) {
		// _hnmContext->_header._unusedFlag2 = 0; CHECKME: Useless?
		_hnmSoundChannel->setVolumeLeft(0);
		_hnmSoundChannel->setVolumeRight(0);
	}

	if (_vm->_video->getVersion() != 4)
		return;

	bool playing = true;
	_vm->_video->allocMemory();
	_hnmView = new View(_vm->_video->_header._width, _vm->_video->_header._height);
	_hnmView->setSrcZoomValues(0, 0);
	_hnmView->setDisplayZoomValues(_vm->_video->_header._width * 2, _vm->_video->_header._height * 2);
	_hnmView->centerIn(_vm->_screenView);
	_hnmViewBuf = _hnmView->_bufferPtr;
	if (arg1) {
		_hnmView->_normal._height = 160;
		_hnmView->_zoom._height = 320;   //TODO: width??
		_hnmView->_normal._dstTop = _mainView->_normal._dstTop + 16;
		_hnmView->_zoom._dstTop = _mainView->_zoom._dstTop + 32;
	}
	_vm->_video->setFinalBuffer(_hnmView->_bufferPtr);
	do {
		_hnmFrameNum = _vm->_video->getFrameNum();
		_vm->_video->waitLoop();
		playing = _vm->_video->nextElement();
		if (_specialTextMode)
			handleHNMSubtitles();
		else
			musicspy();
		CLBlitter_CopyView2Screen(_hnmView);
		assert(_vm->_screenView->_pitch == 320);
		_vm->pollEvents();

		if (arg1) {
			if (_vm->isMouseButtonDown()) {
				if (!_mouseHeld) {
					_mouseHeld = true;
					_videoCanceledFlag = true;
				}
			}
			else
				_mouseHeld = false;
		}
	} while (playing && !_videoCanceledFlag);
	delete _hnmView;
	_vm->_video->deallocMemory();
}

void EdenGame::playHNM(int16 num) {
	perso_t *perso = nullptr;
	int16 oldDialogType = -1;
	_globals->_curVideoNum = num;
	if (num != 2001 && num != 2012 && num != 98 && num != 171) {
		byte oldMusicType = _globals->_newMusicType;
		_globals->_newMusicType = MusicType::mtEvent;
		musique();
		musicspy();
		_globals->_newMusicType = oldMusicType;
	}
	_globals->_videoSubtitleIndex = 1;
	if (_specialTextMode) {
		perso = _globals->_characterPtr;
		oldDialogType = _globals->_dialogType;
		preloadDialogs(num);
		fademusica0(1);
		_musicChannel->stop();
	}
	_showVideoSubtitle = false;
	_videoCanceledFlag = false;
	loadHnm(num);
	_vm->_video->reset();
	if (_needToFade) {
		fadeToBlack(4);
		clearScreen();
		_needToFade = false;
	}
	if (num == 2012 || num == 98 || num == 171)
		showMovie(0);
	else
		showMovie(1);
	_cursKeepPos = Common::Point(-1, -1);
	if (_specialTextMode) {
		_musicFadeFlag = 3;
		musicspy();
		_globals->_characterPtr = perso;
		_globals->_dialogType = oldDialogType;
		_specialTextMode = false;
	}
	if (_videoCanceledFlag)
		_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_globals->_curVideoNum == 167)
		_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_globals->_curVideoNum == 104)
		_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_globals->_curVideoNum == 102)
		_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_globals->_curVideoNum == 77)
		_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_globals->_curVideoNum == 149)
		_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
}

// Original name bullehnm
void EdenGame::handleHNMSubtitles() {
#define SUB_LINE(start, end) \
	(start), (end) | 0x8000

	static uint16 kFramesVid170[] = {
		SUB_LINE(68, 120),
		SUB_LINE(123, 196),
		SUB_LINE(199, 274),
		SUB_LINE(276, 370),
		SUB_LINE(799, 885),
		SUB_LINE(888, 940),
		SUB_LINE(947, 1000),
		SUB_LINE(1319, 1378),
		SUB_LINE(1380, 1440),
		SUB_LINE(1854, 1898),
		SUB_LINE(1900, 1960),
		SUB_LINE(2116, 2184),
		SUB_LINE(2186, 2252),
		SUB_LINE(2254, 2320),
		SUB_LINE(3038, 3094),
		SUB_LINE(3096, 3160),
		0xFFFF
	};

	static uint16 kFramesVid83[] = {
		SUB_LINE(99, 155),
		SUB_LINE(157, 256),
		0xFFFF
	};

	static uint16 kFramesVid88[] = {
		SUB_LINE(106, 173),
		SUB_LINE(175, 244),
		SUB_LINE(246, 350),
		SUB_LINE(352, 467),
		0xFFFF
	};

	static uint16 kFramesVid89[] = {
		SUB_LINE(126, 176),
		SUB_LINE(178, 267),
		SUB_LINE(269, 342),
		SUB_LINE(344, 398),
		SUB_LINE(400, 458),
		SUB_LINE(460, 558),
		0xFFFF
	};

	static uint16 kFramesVid94[] = {
		SUB_LINE(101, 213),
		SUB_LINE(215, 353),
		SUB_LINE(355, 455),
		SUB_LINE(457, 518),
		SUB_LINE(520, 660),
		SUB_LINE(662, 768),
		0xFFFF
	};

#undef SUB_LINE

	uint16 *frames;
	perso_t *perso;
	switch (_globals->_curVideoNum) {
	case 170:
		frames = kFramesVid170;
		perso = &_persons[PER_UNKN_156];
		break;
	case 83:
		frames = kFramesVid83;
		perso = &_persons[PER_MORKUS];
		break;
	case 88:
		frames = kFramesVid88;
		perso = &_persons[PER_MORKUS];
		break;
	case 89:
		frames = kFramesVid89;
		perso = &_persons[PER_MORKUS];
		break;
	case 94:
		frames = kFramesVid94;
		perso = &_persons[PER_MORKUS];
		break;
	default:
		return;
	}
	uint16 *frames_start = frames;
	uint16 frame;
	while ((frame = *frames++) != 0xFFFF) {
		if ((frame & ~0x8000) == _hnmFrameNum)
			break;
	}
	if (frame == 0xFFFF) {
		if (_showVideoSubtitle)
			displayHNMSubtitle();
		return;
	}
	if (frame & 0x8000)
		_showVideoSubtitle = false;
	else {
		_globals->_videoSubtitleIndex = (frames - frames_start) / 2 + 1;
		_globals->_characterPtr = perso;
		_globals->_dialogType = DialogType::dtInspect;
		int16 num = (perso->_id << 3) | _globals->_dialogType;
		dialoscansvmas((Dialog *)getElem(_gameDialogs, num));
		_showVideoSubtitle = true;
	}
	if (_showVideoSubtitle)
		displayHNMSubtitle();
}

}   // namespace Cryo
