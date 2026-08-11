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

#include "cryo/cryo.h"
#include "cryo/platdefs.h"
#include "cryo/cryolib.h"
#include "cryo/eden.h"
#include "cryo/eden_graphics.h"

#include "graphics/blit.h"
#include "video/hnm_decoder.h"

namespace Cryo {

EdenGraphics::EdenGraphics(EdenGame *game) : _game(game) {
	_glowH = _glowW = _glowY = _glowX = 0;
	_showVideoSubtitle = false;
	_showBlackBars = false;
	_mainView = nullptr;
	_mainViewBuf = nullptr;
	_hnmView = nullptr;
	_view2 = nullptr;
	_view2Buf = nullptr;
	_curCharacterRect = nullptr;
	_subtitlesViewBuf = nullptr;
	_underSubtitlesView = nullptr;
	_subtitlesView = nullptr;
	_underBarsView = nullptr;
	_needToFade = false;
	_eff2pat = 0;

	_savedUnderSubtitles = false;
	_underSubtitlesViewBuf = nullptr;
	_hnmViewBuf = nullptr;
	_hnmFrameNum = 0;
	_videoCanceledFlag = false;

	for (int i = 0; i < 256; ++i) {
		_globalPalette[i].a = _globalPalette[i].r = _globalPalette[i].g = _globalPalette[i].b = 0;
		_oldPalette[i].a = _oldPalette[i].r = _oldPalette[i].g = _oldPalette[i].b = 0;
		_newPalette[i].a = _newPalette[i].r = _newPalette[i].g = _newPalette[i].b = 0;
	}

	_newColor.r = _newColor.g = _newColor.b = 0;
}

EdenGraphics::~EdenGraphics() {
	delete _underBarsView;
	delete _view2;
	delete _subtitlesView;
	delete _underSubtitlesView;
	delete _mainView;
}

void EdenGraphics::SendPalette2Screen(int16 value) {
	CLPalette_Send2Screen(_globalPalette, 0, value);
}

void EdenGraphics::setFade(bool value) {
	_needToFade = value;
}

bool EdenGraphics::getFade() {
	return _needToFade;
}

void EdenGraphics::setPaletteColor(byte *buffer) {
	for (int i = 0; i < 256; i++) {
		color3_t color;
		color.r = buffer[i * 3] << 8;
		color.g = buffer[i * 3 + 1] << 8;
		color.b = buffer[i * 3 + 2] << 8;
		CLPalette_SetRGBColor(_globalPalette, i, &color);
	}
	SendPalette2Screen(256);
}

void EdenGraphics::readPalette(byte *ptr) {
	bool doit = true;
	color3_t pal_entry;
	while (doit) {
		uint16 idx = *ptr++;
		if (idx != 0xFF) {
			uint16 cnt = *ptr++;
			while (cnt--) {
				if (idx == 0) {
					pal_entry.r = 0;
					pal_entry.g = 0;
					pal_entry.b = 0;
					ptr += 3;
				} else {
					pal_entry.r = *ptr++ << 10;
					pal_entry.g = *ptr++ << 10;
					pal_entry.b = *ptr++ << 10;
				}
				CLPalette_SetRGBColor(_globalPalette, idx, &pal_entry);
				idx++;
			}
		} else
			doit = false;
	}
}

// Original name: noclipax
void EdenGraphics::drawSprite(int16 index, int16 x, int16 y, bool withBlack, bool onSubtitle) {
	uint16 width = (!onSubtitle) ? 640 : _subtitlesXWidth;
	byte *pix = _game->getBankData();
	byte *buf = (!onSubtitle) ? _mainViewBuf : _subtitlesViewBuf;
	byte *scr = buf + x + y * width;
	if (_game->getCurBankNum() != 117 && (!_game->getNoPalette() || withBlack || onSubtitle)) {
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

void EdenGraphics::setCursKeepPos(int16 x, int16 y) {
	_cursKeepPos = Common::Point(-1, -1);
}

byte *EdenGraphics::getSubtitlesViewBuf() {
	return _subtitlesViewBuf;
}

View *EdenGraphics::getSubtitlesView() {
	return _subtitlesView;
}

void EdenGraphics::loadMouthRectFromCurChar() {
	_rect_src.left   = _curCharacterRect->left;
	_rect_src.top    = _curCharacterRect->top;
	_rect_src.right  = _curCharacterRect->right;
	_rect_src.bottom = _curCharacterRect->bottom;
	_rect_dst.left   = _curCharacterRect->left  + 320;
	_rect_dst.top    = _curCharacterRect->top;
	_rect_dst.right  = _curCharacterRect->right + 320;
	_rect_dst.bottom = _curCharacterRect->bottom;
}

void EdenGraphics::paneltobuf() {
	setSrcRect(0, 16, 320 - 1, 169 - 1);
	setDestRect(320, 16, 640 - 1, 169 - 1);
	CLBlitter_CopyViewRect(getMainView(), getMainView(), &_rect_src, &_rect_dst);
}

void EdenGraphics::cursbuftopanel() {
	setSrcRect(434, 40, 525 - 1, 111 - 1);
	setDestRect(114, 40, 205 - 1, 111 - 1);
	CLBlitter_CopyViewRect(getMainView(), getMainView(), &_rect_src, &_rect_dst);
}

void EdenGraphics::langbuftopanel() {
	setSrcRect(328, 42, 407 - 1, 97 - 1);
	setDestRect(8, 42,  87 - 1, 97 - 1);
	CLBlitter_CopyViewRect(getMainView(), getMainView(), &_rect_src, &_rect_dst);
}

// Original name: sauvefondbouche
void EdenGraphics::saveMouthBackground() {
	loadMouthRectFromCurChar();
	CLBlitter_CopyViewRect(getMainView(), getMainView(), &_rect_src, &_rect_dst);
}

// Original name: restaurefondbouche
void EdenGraphics::restoreMouthBackground() {
	loadMouthRectFromCurChar();
	CLBlitter_CopyViewRect(getMainView(), getMainView(), &_rect_dst, &_rect_src);
}

void EdenGraphics::setGlowX(int16 value) {
	_glowX = value;
}

View *EdenGraphics::getMainView() {
	return _mainView;
}

void EdenGraphics::setGlowY(int16 value) {
	_glowY = value;
}

void EdenGraphics::setCurCharRect(Common::Rect * charRect) {
	_curCharacterRect = charRect;
}

void EdenGraphics::sundcurs(int16 x, int16 y) {
	byte *keep = _game->getCurKeepBuf();
	x = CLIP<int16>(x - 4, 0, 640 - 48);
	y = CLIP<int16>(y - 4, 0, 200 - 48);
	_cursKeepPos = Common::Point(x, y);
	byte *scr = _mainViewBuf + _cursKeepPos.x + _cursKeepPos.y * 640;
	for (int16 h = 48; h--;) {
		for (int16 w = 48; w--;)
			*keep++ = *scr++;
		scr += 640 - 48;
	}
	_game->setCursorSaved(true);
}

void EdenGraphics::rundcurs() {
	byte *keep = _game->getCurKeepBuf();
	byte *scr = _mainViewBuf + _cursKeepPos.x + _cursKeepPos.y * 640;
	if (!_game->getCursorSaved() || (_cursKeepPos == Common::Point(-1, -1)))  //TODO ...
		return;

	for (int16 h = 48; h--;) {
		for (int16 w = 48; w--;)
			*scr++ = *keep++;
		scr += 640 - 48;
	}
}

byte * EdenGraphics::getHnmViewBuf() {
	return _hnmViewBuf;
}

void EdenGraphics::getglow(int16 x, int16 y, int16 w, int16 h) {
	byte *scr = _mainViewBuf + x + y * 640;
	byte *gl = _game->getGlowBuffer();
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

void EdenGraphics::unglow() {
	byte *gl = _game->getGlowBuffer();
	byte *scr = _mainViewBuf + _glowX + _glowY * 640;
	if (_glowX < 0 || _glowY < 0)   //TODO: move it up
		return;
	for (; _glowH--;) {
		for (int16 ww = _glowW; ww--;)
			*scr++ = *gl++;
		scr += 640 - _glowW;
	}
}

void EdenGraphics::glow(int16 index) {
	// byte pixbase;
	byte *pix = _game->getBankData();

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

	int16 x = _game->getCurPosX() + _game->getScrollPos() - 38;
	int16 y = _game->getCurPosY() - 28;
	int16 ex = _game->_globals->_frescoeWidth + 320;

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
void EdenGraphics::drawBlackBars() {
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

// Original name: restaurefriseshaut
void EdenGraphics::restoreTopFrieze() {
	_underTopBarScreenRect.left = _game->getScrollPos();
	_underTopBarScreenRect.right = _game->getScrollPos() + 320 - 1;
	CLBlitter_CopyViewRect(_underBarsView, getMainView(), &_underTopBarBackupRect, &_underTopBarScreenRect);
}

// Original name: bars_out
void EdenGraphics::hideBars() {
	if (_showBlackBars)
		return;

	_game->display();
	_underTopBarScreenRect.left = _game->getScrollPos();
	_underTopBarScreenRect.right = _game->getScrollPos() + 320 - 1;
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
	_underTopBarBackupRect.left = _game->getScrollPos();
	_underTopBarBackupRect.right = _game->getScrollPos() + 320 - 1;
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
		_game->display();
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
	_game->display();
	initRects();
	_showBlackBars = true;
}

void EdenGraphics::initRects() {
	_underTopBarScreenRect = Common::Rect(0, 0, 320 - 1, 16 - 1);
	_underTopBarBackupRect = Common::Rect(0, 0, 320 - 1, 16 - 1);
	_underBottomBarScreenRect = Common::Rect(0, 176, 320 - 1, 200 - 1);  //TODO: original bug? this cause crash in copyrect (this, underBottomBarBackupRect)
	_underBottomBarBackupRect = Common::Rect(0, 16, 320 - 1, 40 - 1);
}

// Original name: bars_in
void EdenGraphics::showBars() {
	if (!_showBlackBars)
		return;

	drawBlackBars();
	int16 r29 = 2;
	int16 r28 = 2;
	_underTopBarScreenRect.left = 0;
	_underTopBarScreenRect.right = 320 - 1;
	_underTopBarBackupRect.left = _game->getScrollPos();
	_underTopBarBackupRect.right = _game->getScrollPos() + 320 - 1;
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
		_game->display();
	}
	initRects();
	_showBlackBars = false;
}

// Original name: af_image
void EdenGraphics::displayImage() {
	byte *img = _game->getImageDesc() + 200;

	int16 count = READ_LE_UINT16(img);
	if (!count)
		return;

	byte *img_start = img;
	byte *curimg = _game->getImageDesc();

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
		uint16 x = *img++ + _game->getGameIconX(0);
		uint16 y = *img++ + _game->getGameIconY(0);
		byte *pix = _game->getBankData();
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
void EdenGraphics::displaySubtitles() {
	byte *src = _subtitlesViewBuf;
	byte *dst = _mainViewBuf;
	int16 y;
	if (_game->_globals->_displayFlags & DisplayFlags::dfFlag2) {
		y = 174;
		if ((_game->_globals->_drawFlags & DrawFlags::drDrawMenu) && _game->getNumTextLines() == 1)
			y = 167;
		dst += 640 * (y - _game->getNumTextLines() * FONT_HEIGHT) + _subtitlesXScrMargin;
	}
	else {
		y = 174;
		dst += 640 * (y - _game->getNumTextLines() * FONT_HEIGHT) + _game->getScrollPos() + _subtitlesXScrMargin;
	}
	if (_game->animationIsActive() && !_game->personIsTalking())
		return;
	saveUnderSubtitles(y);
	for (int16 h = 0; h < _game->getNumTextLines() * FONT_HEIGHT + 1; h++) {
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
void EdenGraphics::displaySingleRoom(Room *room) {
	byte *ptr = (byte *)getElem(_game->getPlaceRawBuf(), room->_id - 1);
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
			if (!(_game->_globals->_displayFlags & 0x80)) {
				if (index == 1 || _game->_globals->_varF7)
					drawSprite(index - 1, x, y, true);
			}
			_game->_globals->_varF7 = 0;
			continue;
		}
		if (b1 & 0x40) {
			if (b1 & 0x20) {
				bool addIcon = false;
				Icon *icon = _game->_globals->_nextRoomIcon;
				if (b0 < 4) {
					if (_game->_globals->_roomPtr->_exits[b0])
						addIcon = true;
				}
				else if (b0 > 229) {
					if (_game->_globals->_partyOutside & (1 << (b0 - 230)))
						addIcon = true;
				}
				else if (b0 >= 100) {
					debug("add object %d", b0 - 100);
					if (_game->isObjectHere(b0 - 100)) {
						addIcon = true;
						_game->_globals->_varF7 = 1;
					}
				}
				else
					addIcon = true;
				if (addIcon) {
					icon->_actionId = b0;
					icon->_objectId = b0;
					icon->_cursorId = _game->getActionCursor(b0);
					int16 x = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 y = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 ex = READ_LE_UINT16(ptr);
					ptr += 2;
					int16 ey = READ_LE_UINT16(ptr);
					ptr += 2;
					x += _game->_globals->_roomBaseX;
					ex += _game->_globals->_roomBaseX;
					debug("add hotspot at %3d:%3d - %3d:%3d, action = %d", x, y, ex, ey, b0);

					if (_game->_vm->_showHotspots) {
						for (int iii = x; iii < ex; iii++)
							_mainViewBuf[y * 640 + iii] = _mainViewBuf[ey * 640 + iii] = (iii % 2) ? 0 : 255;
						for (int iii = y; iii < ey; iii++)
							_mainViewBuf[iii * 640 + x] = _mainViewBuf[iii * 640 + ex] = (iii % 2) ? 0 : 255;
					}

					icon->sx = x;
					icon->sy = y;
					icon->ex = ex;
					icon->ey = ey;
					_game->_globals->_nextRoomIcon = ++icon;
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

// Original name: restaurefrisesbas
void EdenGraphics::restoreBottomFrieze() {
	_underBottomBarScreenRect.left = _game->getScrollPos();
	_underBottomBarScreenRect.right = _game->getScrollPos() + 320 - 1;
	CLBlitter_CopyViewRect(_underBarsView, getMainView(), &_underBottomBarBackupRect, &_underBottomBarScreenRect);
}

// Original name: sauvefriseshaut
void EdenGraphics::saveTopFrieze(int16 x) { // Save top bar
	_underTopBarScreenRect = Common::Rect(x, 0, x + 320 - 1, 15);
	_underTopBarBackupRect = Common::Rect(0, 0, 320 - 1, 15);
	CLBlitter_CopyViewRect(getMainView(), _underBarsView, &_underTopBarScreenRect, &_underTopBarBackupRect);
}

// Original name: sauvefrisesbas
void EdenGraphics::saveBottomFrieze() {         // Save bottom bar
	_underBottomBarScreenRect.left = 0;
	_underBottomBarScreenRect.right = 320 - 1;
	CLBlitter_CopyViewRect(getMainView(), _underBarsView, &_underBottomBarScreenRect, &_underBottomBarBackupRect);
}

// Original name: afsalle
void EdenGraphics::displayRoom() {
	Room *room = _game->_globals->_roomPtr;
	_game->_globals->_displayFlags = DisplayFlags::dfFlag1;
	_game->_globals->_roomBaseX = 0;
	_game->_globals->_roomBackgroundBankNum = room->_backgroundBankNum;
	if (room->_flags & RoomFlags::rf08) {
		_game->_globals->_displayFlags |= DisplayFlags::dfFlag80;
		if (room->_flags & RoomFlags::rfPanable) {
			// Scrollable room on 2 screens
			_game->_globals->_displayFlags |= DisplayFlags::dfPanable;
			_game->_globals->_varF4 = 0;
			rundcurs();
			_game->saveFriezes();
			_game->useBank(room->_bank - 1);
			drawSprite(0, 0, 16, true);
			_game->useBank(room->_bank);
			drawSprite(0, 320, 16, true);
			displaySingleRoom(room);
			_game->_globals->_roomBaseX = 320;
			displaySingleRoom(room + 1);
		}
		else
			displaySingleRoom(room);
	}
	else {
		//TODO: roomImgBank is garbage here!
		debug("displayRoom: room 0x%X using bank %d", _game->_globals->_roomNum, _game->_globals->_roomImgBank);
		_game->useBank(_game->_globals->_roomImgBank);
		displaySingleRoom(room);
		assert(_game->_vm->_screenView->_pitch == 320);
	}
}

View *EdenGraphics::getUnderBarsView() {
	return _underBarsView;
}

void EdenGraphics::openWindow() {
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
	_mainView->centerIn(_game->_vm->_screenView);
	_mainViewBuf = _mainView->_bufferPtr;

	_game->setMouseCenterX(_mainView->_normal._dstLeft + _mainView->_normal._width  / 2);
	_game->setMouseCenterY(_mainView->_normal._dstTop  + _mainView->_normal._height / 2);
	_game->_vm->setMousePosition(_game->getMouseCenterX(), _game->getMouseCenterY());
	_game->_vm->hideMouse();

	_game->setCurPosX(320 / 2);
	_game->setCurPosY(200 / 2);
}

// Original name: effet1
void EdenGraphics::displayEffect1() {
	blackRect32();
	setSrcRect(0, 0, 16 - 1, 4 - 1);
	int y = _mainView->_normal._dstTop;
	for (int16 i = 16; i <= 96; i += 4) {
		for (int x = _mainView->_normal._dstLeft; x < _mainView->_normal._dstLeft + 320; x += 16) {
			setDestRect(x, y + i, x + 16 - 1, y + i + 4 - 1);
			CLBlitter_CopyViewRect(_view2, _game->_vm->_screenView, &_rect_src, &_rect_dst);
			setDestRect(x, y + 192 - i, x + 16 - 1, y + 192 - i + 4 - 1);
			CLBlitter_CopyViewRect(_view2, _game->_vm->_screenView, &_rect_src, &_rect_dst);
		}
		CLBlitter_UpdateScreen();
		_game->wait(1);
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
		_game->wait(1);
	}
	_mainView->_normal._height = 200;
	_mainView->_zoom._height = 400;
	_mainView->_normal._srcTop = 0;
	_mainView->_zoom._srcTop = 0;
	_mainView->_normal._dstTop = ny;
	_mainView->_zoom._dstTop = dy;
	_game->_globals->_varF1 = 0;
}

// Original name: effet2
void EdenGraphics::displayEffect2() {
	static const int16 pattern1[] = { 0, 1, 2, 3, 7, 11, 15, 14, 13, 12, 8, 4, 5, 6, 10, 9 };
	static const int16 pattern2[] = { 0, 15, 1, 14, 2, 13, 3, 12, 7, 8, 11, 4, 5, 10, 6, 9 };
	static const int16 pattern3[] = { 0, 2, 5, 7, 8, 10, 13, 15, 1, 3, 4, 6, 9, 11, 12, 14 };
	static const int16 pattern4[] = { 0, 3, 15, 12, 1, 7, 14, 8, 2, 11, 13, 4, 5, 6, 10, 9 };

	if (_game->_globals->_var103 == 69) {
		displayEffect4();
		return;
	}
	switch (++_eff2pat) {
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
	default:
		colimacon(pattern4);
		_eff2pat = 0;
		break;
	}
}

// Original name: effet3
void EdenGraphics::displayEffect3() {
	CLPalette_GetLastPalette(_oldPalette);
	for (uint16 i = 0; i < 6; i++) {
		for (uint16 c = 0; c < 256; c++) {
			_newColor.r = _oldPalette[c].r >> i;
			_newColor.g = _oldPalette[c].g >> i;
			_newColor.b = _oldPalette[c].b >> i;
			CLPalette_SetRGBColor(_newPalette, c, &_newColor);
		}
		CLPalette_Send2Screen(_newPalette, 0, 256);
		_game->wait(1);
	}
	CLBlitter_CopyView2Screen(_mainView);
	for (uint16 i = 0; i < 6; i++) {
		for (uint16 c = 0; c < 256; c++) {
			_newColor.r = _globalPalette[c].r >> (5 - i);
			_newColor.g = _globalPalette[c].g >> (5 - i);
			_newColor.b = _globalPalette[c].b >> (5 - i);
			CLPalette_SetRGBColor(_newPalette, c, &_newColor);
		}
		CLPalette_Send2Screen(_newPalette, 0, 256);
		_game->wait(1);
	}
}

// Original name: effet4
void EdenGraphics::displayEffect4() {
	byte *scr, *pix, *r24, *r25, *r30, c;
	int16 r17, r23, r16, r18, r19, r22, r27, r31;
	CLPalette_Send2Screen(_globalPalette, 0, 256);

	int16 ww = _game->_vm->_screenView->_pitch;
	int16 x = _mainView->_normal._dstLeft;
	int16 y = _mainView->_normal._dstTop;
	for (int16 i = 32; i > 0; i -= 2) {
		scr = _game->_vm->_screenView->_bufferPtr;
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
		_game->wait(3);
	}
	CLBlitter_CopyView2Screen(_mainView);
}

void EdenGraphics::clearScreen() {
	int16 ww = _game->_vm->_screenView->_pitch;
	int16 x = _mainView->_normal._dstLeft;
	int16 y = _mainView->_normal._dstTop;
	byte *scr = _game->_vm->_screenView->_bufferPtr;
	scr += (y + 16) * ww + x;
	for (int16 yy = 0; yy < 160; yy++) {
		for (int16 xx = 0; xx < 320; xx++)
			*scr++ = 0;
		scr += ww - 320;
	}
	CLBlitter_UpdateScreen();
}

void EdenGraphics::colimacon(const int16 pattern[]) {
	int16 p, r27, r25;

	int16 ww = _game->_vm->_screenView->_pitch;
	int16 x = _mainView->_normal._dstLeft;
	int16 y = _mainView->_normal._dstTop;
	byte *scr = _game->_vm->_screenView->_bufferPtr;
	scr += (y + 16) * ww + x;
	for (int16 i = 0; i < 16; i++) {
		p = pattern[i];
		r27 = p % 4 + p / 4 * ww;
		for (int16 j = 0; j < 320 * 160 / 16; j++)
			scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] = 0;
		CLBlitter_UpdateScreen();
		_game->wait(1);
	}
	CLPalette_Send2Screen(_globalPalette, 0, 256);
	byte *pix = _mainView->_bufferPtr;
	x = _mainView->_normal._dstLeft;
	y = _mainView->_normal._dstTop;
	pix += 640 * 16;
	scr = _game->_vm->_screenView->_bufferPtr;
	scr += (y + 16) * ww + x;
	for (int16 i = 0; i < 16; i++) {
		p = pattern[i];
		r25 = p % 4 + p / 4 * 640;
		r27 = p % 4 + p / 4 * ww;
		for (int16 j = 0; j < 320 * 160 / 16; j++)
			scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] =
			pix[j / (320 / 4) * 640 * 4 + j % (320 / 4) * 4 + r25];
		CLBlitter_UpdateScreen();
		_game->wait(1);
	}
}

void EdenGraphics::fadeToBlack(int delay) {
	CLPalette_GetLastPalette(_oldPalette);
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 256; j++) {
			_newColor.r = _oldPalette[j].r >> i;
			_newColor.g = _oldPalette[j].g >> i;
			_newColor.b = _oldPalette[j].b >> i;
			CLPalette_SetRGBColor(_newPalette, j, &_newColor);
		}
		CLPalette_Send2Screen(_newPalette, 0, 256);
		_game->wait(delay);
	}
}

// Original name: fadetoblack128
void EdenGraphics::fadeToBlackLowPalette(int delay) {
	CLPalette_GetLastPalette(_oldPalette);
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 129; j++) { //CHECKME: Should be 128?
			_newColor.r = _oldPalette[j].r >> i;
			_newColor.g = _oldPalette[j].g >> i;
			_newColor.b = _oldPalette[j].b >> i;
			CLPalette_SetRGBColor(_newPalette, j, &_newColor);
		}
		CLPalette_Send2Screen(_newPalette, 0, 128);
		_game->wait(delay);
	}
}

// Original name: fadefromblack128
void EdenGraphics::fadeFromBlackLowPalette(int delay) {
	for (int16 i = 0; i < 6; i++) {
		for (int16 j = 0; j < 129; j++) { //CHECKME: Should be 128?
			_newColor.r = _globalPalette[j].r >> (5 - i);
			_newColor.g = _globalPalette[j].g >> (5 - i);
			_newColor.b = _globalPalette[j].b >> (5 - i);
			CLPalette_SetRGBColor(_newPalette, j, &_newColor);
		}
		CLPalette_Send2Screen(_newPalette, 0, 128);
		_game->wait(delay);
	}
}

// Original name: rectanglenoir32
void EdenGraphics::blackRect32() {
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

void EdenGraphics::setSrcRect(int16 sx, int16 sy, int16 ex, int16 ey) {
	_rect_src = Common::Rect(sx, sy, ex, ey);
}

void EdenGraphics::setDestRect(int16 sx, int16 sy, int16 ex, int16 ey) {
	_rect_dst = Common::Rect(sx, sy, ex, ey);
}

void EdenGraphics::effetpix() {
	uint16 r25, r18, r31, r30;  //TODO: change to xx/yy

	uint16 ww = _game->_vm->_screenView->_pitch;
	r25 = ww * 80;
	r18 = 640 * 80;
	byte *pix = _mainView->_bufferPtr + 16 * 640;
	int x = _mainView->_normal._dstLeft;
	int y = _mainView->_normal._dstTop;
	byte *scr = _game->_vm->_screenView->_bufferPtr;
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
				_game->wait(1);
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
				_game->wait(1);
				r26 = 0;
			}
		}
	} while (r27 != 1);
	assert(_game->_vm->_screenView->_pitch == 320);
}

////// film.c
// Original name: showfilm
void EdenGraphics::showMovie(int16 num, char arg1) {
	Common::SeekableReadStream *stream = _game->loadSubStream(num - 1 + 485);
	if (!stream) {
		warning("Could not load movie %d", num);
		return;
	}

	int16 j = 0;
	color_t palette16[256];
	byte *palette = new byte[256 * 3];
	CLPalette_GetLastPalette(palette16);
	for (int16 i = 0; i < 256; i++) {
		palette[j++] = palette16[i].r >> 8;
		palette[j++] = palette16[i].g >> 8;
		palette[j++] = palette16[i].b >> 8;
	}

	Video::VideoDecoder *decoder = new Video::HNMDecoder(g_system->getScreenFormat(), false, palette);
	if (!decoder->loadStream(stream)) {
		warning("Could not load movie %d", num);
		delete decoder;
		return;
	}

	if (_game->_globals->_curVideoNum == 92) {
		decoder->setVolume(0);
	}

	decoder->start();

	_hnmView = new View(decoder->getWidth(), decoder->getHeight());
	_hnmView->setSrcZoomValues(0, 0);
	_hnmView->setDisplayZoomValues(decoder->getWidth() * 2, decoder->getHeight() * 2);
	_hnmView->centerIn(_game->_vm->_screenView);
	_hnmViewBuf = _hnmView->_bufferPtr;
	if (arg1) {
		_hnmView->_normal._height = 160;
		_hnmView->_zoom._height = 320;   //TODO: width??
		_hnmView->_normal._dstTop = _mainView->_normal._dstTop + 16;
		_hnmView->_zoom._dstTop = _mainView->_zoom._dstTop + 32;
	}

	do {
		if (decoder->needsUpdate()) {
			const Graphics::Surface *frame = decoder->decodeNextFrame();
			if (frame) {
				Graphics::copyBlit(_hnmView->_bufferPtr, (const byte *)frame->getPixels(), _hnmView->_pitch, frame->pitch, frame->w, frame->h, 1);
			}
			if (decoder->hasDirtyPalette()) {
				const byte *framePalette = decoder->getPalette();
				for (int i = 0; i < 256; i++) {
					palette16[i].r = framePalette[(i * 3) + 0] << 8;
					palette16[i].g = framePalette[(i * 3) + 1] << 8;
					palette16[i].b = framePalette[(i * 3) + 2] << 8;
				}
				CLBlitter_Send2ScreenNextCopy(palette16, 0, 256);
			}
		}
		_hnmFrameNum = decoder->getCurFrame();

		if (_game->getSpecialTextMode())
			handleHNMSubtitles();
		else
			_game->musicspy();

		CLBlitter_CopyView2Screen(_hnmView);
		assert(_game->_vm->_screenView->_pitch == 320);
		_game->_vm->pollEvents();

		if (arg1) {
			if (_game->_vm->isMouseButtonDown()) {
				if (!_game->isMouseHeld()) {
					_game->setMouseHeld();
					_videoCanceledFlag = true;
				}
			}
			else
				_game->setMouseNotHeld();
		}

		g_system->delayMillis(10);
	} while (!_game->_vm->shouldQuit() && !decoder->endOfVideo() && !_videoCanceledFlag);

	delete _hnmView;
	delete decoder;
}

bool EdenGraphics::getShowBlackBars() {
	return _showBlackBars;
}

void EdenGraphics::setShowBlackBars(bool value) {
	_showBlackBars = value;
}

void EdenGraphics::playHNM(int16 num) {
	perso_t *perso = nullptr;
	int16 oldDialogType = -1;
	_game->_globals->_curVideoNum = num;
	if (num != 2001 && num != 2012 && num != 98 && num != 171) {
		byte oldMusicType = _game->_globals->_newMusicType;
		_game->_globals->_newMusicType = MusicType::mtEvent;
		_game->musique();
		_game->musicspy();
		_game->_globals->_newMusicType = oldMusicType;
	}
	_game->_globals->_videoSubtitleIndex = 1;
	if (_game->getSpecialTextMode()) {
		perso = _game->_globals->_characterPtr;
		oldDialogType = _game->_globals->_dialogType;
		_game->preloadDialogs(num);
		_game->fademusica0(1);
		_game->stopMusic();
	}
	_showVideoSubtitle = false;
	_videoCanceledFlag = false;

	if (_needToFade) {
		fadeToBlack(4);
		clearScreen();
		_needToFade = false;
	}
	if (num == 2012 || num == 98 || num == 171)
		showMovie(num, 0);
	else
		showMovie(num, 1);

	_cursKeepPos = Common::Point(-1, -1);
	if (_game->getSpecialTextMode()) {
		_game->setMusicFade(3);
		_game->musicspy();
		_game->_globals->_characterPtr = perso;
		_game->_globals->_dialogType = oldDialogType;
		_game->setSpecialTextMode(false);
	}
	if (_videoCanceledFlag)
		_game->_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_game->_globals->_curVideoNum == 167)
		_game->_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_game->_globals->_curVideoNum == 104)
		_game->_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_game->_globals->_curVideoNum == 102)
		_game->_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_game->_globals->_curVideoNum == 77)
		_game->_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	if (_game->_globals->_curVideoNum == 149)
		_game->_globals->_varF1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
}

void EdenGraphics::initGlobals() {
	_underSubtitlesScreenRect.top = 0;
	_underSubtitlesScreenRect.left = _subtitlesXScrMargin;
	_underSubtitlesScreenRect.right = _subtitlesXScrMargin + _subtitlesXWidth - 1;
	_underSubtitlesScreenRect.bottom = 176 - 1;

	_underSubtitlesBackupRect.top = 0;
	_underSubtitlesBackupRect.left = _subtitlesXScrMargin;
	_underSubtitlesBackupRect.right = _subtitlesXScrMargin + _subtitlesXWidth - 1;
	_underSubtitlesBackupRect.bottom = 60 - 1;
}

// Original name: sauvefondbulle
void EdenGraphics::saveUnderSubtitles(int16 y) {
	_underSubtitlesScreenRect.top = y - _game->getNumTextLines() * FONT_HEIGHT;
	_underSubtitlesScreenRect.left = _game->getScrollPos() + _subtitlesXScrMargin;
	_underSubtitlesScreenRect.right = _game->getScrollPos() + _subtitlesXScrMargin + _subtitlesXWidth - 1;
	_underSubtitlesScreenRect.bottom = y;
	_underSubtitlesBackupRect.top = 0;
	_underSubtitlesBackupRect.bottom = _game->getNumTextLines() * FONT_HEIGHT;
	CLBlitter_CopyViewRect(getMainView(), _underSubtitlesView, &_underSubtitlesScreenRect, &_underSubtitlesBackupRect);
	_savedUnderSubtitles = true;
}

void EdenGraphics::setSavedUnderSubtitles(bool value) {
	_savedUnderSubtitles = value;
}

// Original name: restaurefondbulle
void EdenGraphics::restoreUnderSubtitles() {
	if (!_savedUnderSubtitles)
		return;
	CLBlitter_CopyViewRect(_underSubtitlesView, getMainView(), &_underSubtitlesBackupRect, &_underSubtitlesScreenRect);
	_savedUnderSubtitles = false;
}

// Original name: af_subtitlehnm
void EdenGraphics::displayHNMSubtitle() {
	byte *src = getSubtitlesViewBuf();
	byte *dst = getHnmViewBuf() + _subtitlesXScrMargin + (158 - _game->getNumTextLines() * FONT_HEIGHT) * 320;
	for (int16 y = 0; y < _game->getNumTextLines() * FONT_HEIGHT; y++) {
		for (int16 x = 0; x < _subtitlesXWidth; x++) {
			char c = *src++;
			if (c)
				*dst = c;
			dst++;
		}
		dst += 320 - _subtitlesXWidth;
	}
}

// Original name bullehnm
void EdenGraphics::handleHNMSubtitles() {
#define SUB_LINE(start, end) \
	(start), (end) | 0x8000

	static const uint16 kFramesVid170[] = {
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

	static const uint16 kFramesVid83[] = {
		SUB_LINE(99, 155),
		SUB_LINE(157, 256),
		0xFFFF
	};

	static const uint16 kFramesVid88[] = {
		SUB_LINE(106, 173),
		SUB_LINE(175, 244),
		SUB_LINE(246, 350),
		SUB_LINE(352, 467),
		0xFFFF
	};

	static const uint16 kFramesVid89[] = {
		SUB_LINE(126, 176),
		SUB_LINE(178, 267),
		SUB_LINE(269, 342),
		SUB_LINE(344, 398),
		SUB_LINE(400, 458),
		SUB_LINE(460, 558),
		0xFFFF
	};

	static const uint16 kFramesVid94[] = {
		SUB_LINE(101, 213),
		SUB_LINE(215, 353),
		SUB_LINE(355, 455),
		SUB_LINE(457, 518),
		SUB_LINE(520, 660),
		SUB_LINE(662, 768),
		0xFFFF
	};

#undef SUB_LINE

	const uint16 *frames = nullptr;
	perso_t *perso = nullptr;

	switch (_game->_globals->_curVideoNum) {
	case 170:
		frames = kFramesVid170;
		break;
	case 83:
		frames = kFramesVid83;
		break;
	case 88:
		frames = kFramesVid88;
		break;
	case 89:
		frames = kFramesVid89;
		break;
	case 94:
		frames = kFramesVid94;
		break;
	default:
		return;
	}

	perso = _game->personSubtitles();

	assert(perso  != nullptr);
	assert(frames != nullptr);

	const uint16 *frames_start = frames;
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
		_game->_globals->_videoSubtitleIndex = (frames - frames_start) / 2 + 1;
		_game->_globals->_characterPtr = perso;
		_game->_globals->_dialogType = DialogType::dtInspect;
		int16 num = (perso->_id << 3) | _game->_globals->_dialogType;
		_game->dialoscansvmas((Dialog *)getElem(_game->getGameDialogs(), num));
		_showVideoSubtitle = true;
	}
	if (_showVideoSubtitle)
		displayHNMSubtitle();
}

}   // namespace Cryo
