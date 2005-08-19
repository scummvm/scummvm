/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "kyra/screen.h"
#include "kyra/kyra.h"

namespace Kyra {

Screen::Screen(KyraEngine *vm, OSystem *system)
	: _system(system), _vm(vm) {
	_curPage = 0;
	for (int pageNum = 0; pageNum < PAGE_NUM; pageNum += 2) {
		uint8 *pagePtr = (uint8 *)malloc(PAGE_SIZE);
		if (pagePtr) {
			memset(pagePtr, 0, PAGE_SIZE);
			_pagePtrs[pageNum] = _pagePtrs[pageNum + 1] = pagePtr;
		}
	}
	_palette1 = (uint8 *)malloc(768);
	if (_palette1) {
		memset(_palette1, 0, 768);
	}
}

Screen::~Screen() {
	for (int pageNum = 0; pageNum < PAGE_NUM; pageNum += 2) {
		free(_pagePtrs[pageNum]);
		_pagePtrs[pageNum] = _pagePtrs[pageNum + 1] = 0;
	}
	free(_palette1);
}

void Screen::updateScreen() {
	debug(9, "Screen::updateScreen()");
	_system->copyRectToScreen(getPagePtr(0), SCREEN_W, 0, 0, SCREEN_W, SCREEN_H);
	_system->updateScreen();
}

uint8 *Screen::getPagePtr(int pageNum) {
	debug(9, "Screen::getPagePtr(%d)", pageNum);
	assert(pageNum < PAGE_NUM);
	return _pagePtrs[pageNum];
}

void Screen::clearPage(int pageNum) {
	debug(9, "Screen::clearPage(%d)", pageNum);
	assert(pageNum < PAGE_NUM);
	memset(getPagePtr(pageNum), 0, PAGE_SIZE);
}

int Screen::setCurPage(int pageNum) {
	debug(9, "Screen::setCurPage(%d)", pageNum);
	int previousPage = _curPage;
	_curPage = pageNum;
	return previousPage;
}

void Screen::clearCurPage() {
	debug(9, "Screen::clearCurPage()");
	memset(getPagePtr(_curPage), 0, PAGE_SIZE);
}

void Screen::fadeFromBlack() {
	debug(9, "Screen::fadeFromBlack()");
	setScreenPalette(_palette1);
	warning("Screen::fadeFromBlack() UNIMPLEMENTED");
}

void Screen::fadeToBlack() {
	debug(9, "Screen::fadeToBlack()");
	warning("Screen::fadeToBlack() UNIMPLEMENTED");
}

void Screen::setScreenPalette(const uint8 *palData) {
	debug(9, "Screen::setScreenPalette(0x%X)", palData);
	uint8 screenPal[256 * 4];
	for (int i = 0; i < 256; ++i) {
		screenPal[4 * i + 0] = (palData[0] << 2) | (palData[0] & 3);
		screenPal[4 * i + 1] = (palData[1] << 2) | (palData[1] & 3);
		screenPal[4 * i + 2] = (palData[2] << 2) | (palData[2] & 3);
		screenPal[4 * i + 3] = 0;
		palData += 3;
	}
	_system->setPalette(screenPal, 0, 256);
}

void Screen::copyToPage0(int y, int h, uint8 page, uint8 *seqBuf) {
	debug(9, "Screen::copyToPage0(%d, %d, %d, 0x%X)", y, h, page, seqBuf);
	assert(y + h <= SCREEN_H);
	const uint8 *src = getPagePtr(page) + y * SCREEN_W;
	memcpy(seqBuf, src, h * SCREEN_W);
	memcpy(getPagePtr(0) + y * SCREEN_W, src, h * SCREEN_W);
}

void Screen::copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage) {
	debug(9, "Screen::copyRegion(%d, %d, %d, %d, %d, %d, %d, %d)", x1, y1, x2, y2, w, h, srcPage, dstPage);
	assert(x1 + w <= SCREEN_W && y1 + h <= SCREEN_H);
	const uint8 *src = getPagePtr(srcPage) + y1 * SCREEN_W + x1;
	assert(x2 + w <= SCREEN_W && y2 + h <= SCREEN_H);
	uint8 *dst = getPagePtr(dstPage) + y2 * SCREEN_W + x2;
	while (h--) {
		for (int i = 0; i < w; ++i) {
			if (src[i]) {
				dst[i] = src[i];
			}
		}
		src += SCREEN_W;
		dst += SCREEN_W;
	}
}

void Screen::copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src) {
	debug(9, "Screen::copyBlockToPage(%d, %d, %d, %d, %d, 0x%X)", pageNum, x, y, w, h, src);
	assert(x >= 0 && x < Screen::SCREEN_W && y >= 0 && y < Screen::SCREEN_H);
	uint8 *dst = getPagePtr(pageNum) + y * Screen::SCREEN_W + x;
	while (h--) {
		for (int i = 0; i < w; ++i) {
			dst[i] = src[i];
		}
		dst += Screen::SCREEN_W;
		src += w;
	}
}

void Screen::fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum) {
	debug(9, "Screen::fillRect(%d, %d, %d, %d, %d, %d)", x1, y1, x2, y2, color, pageNum);
	assert(x2 < SCREEN_W && y2 < SCREEN_H);
	if (pageNum == -1) {
		pageNum = _curPage;
	}
	uint8 *dst = getPagePtr(pageNum);
	for (; y1 <= y2; ++y1) {
		for (; x1 <= x2; ++x1) {
			*(dst + y1 * SCREEN_W + x1) = color;
		}
	}
}

void Screen::setAnimBlockPtr(uint8 *p, int size) {
	debug(9, "Screen::setAnimBlockPtr(0x%X, %d)", p, size);
	_animBlockPtr = p;
	_animBlockSize = size;
}

void Screen::setTextColorMap(const uint8 *cmap) {
	debug(9, "Screen::setTextColorMap(0x%X)", cmap);
	setTextColor(cmap, 0, 11);
}

void Screen::setTextColor(const uint8 *cmap, int a, int b) {
	debug(9, "Screen::setTextColor(0x%X, %d, %d)", cmap, a, b);
	for (int i = a; i <= b; ++i) {
		_textColorsMap[i] = *cmap++;
	}
}

int Screen::getCharWidth(uint8 c) const {
	debug(9, "Screen::getCharWidth(%c)", c);
	warning("Screen::getCharWidth() UNIMPLEMENTED");
	return 0;
}

int Screen::getTextWidth(const char *str) const {
	debug(9, "Screen::getTextWidth('%s')", str);
	warning("Screen::getTextWidth() UNIMPLEMENTED");
	return 0;
}

void Screen::printText(const char *str, int x, int y, uint8 color1, uint8 color2) {
	debug(9, "Screen::printText('%s', %d, %d, 0x%X, 0x%X)", str, x, y, color1, color2);
	warning("Screen::printText() UNIMPLEMENTED");
}

void Screen::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, int *flagsTable) {
	debug(9, "Screen::drawShape(%d, %d, %d, %d, %d)", pageNum, x, y, sd, flags);
	warning("Screen::drawShape() UNIMPLEMENTED");
}

void Screen::decodeFrame3(const uint8 *src, uint8 *dst, uint32 size) {
	debug(9, "Screen::decodeFrame3(0x%X, 0x%X, %d)", src, dst, size);
	const uint8 *dstEnd = dst + size;
	while (dst < dstEnd) {
		int8 code = *src++;
		if (code == 0) {
			uint16 sz = READ_BE_UINT16(src);
			src += 2;
			memset(dst, *src++, sz);
			dst += sz;
		} else if (code < 0) {
			memset(dst, *src++, -code);
			dst -= code;
		} else {
			memcpy(dst, src, code);
			dst += code;
			src += code;
		}
	}
}

void Screen::decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize) {
	debug(9, "Screen::decodeFrame4(0x%X, 0x%X, %d)", src, dst, dstSize);
	uint8 *dstOrig = dst;
	uint8 *dstEnd = dst + dstSize;
	while (1) {
		int count = dstEnd - dst;
		if (count == 0) {
			break;
		}
		uint8 code = *src++;
		if (!(code & 0x80)) {
			int len = MIN(count, (code >> 4) + 3);
			int offs = ((code & 0xF) << 8) | *src++;
			const uint8 *dstOffs = dst - offs;
			while (len--) {
				*dst++ = *dstOffs++;
			}
		} else if (code & 0x40) {
			int len = (code & 0x3F) + 3;
			if (code == 0xFE) {
				len = READ_LE_UINT16(src); src += 2;
				if (len > count) {
					len = count;
				}
				memset(dst, *src++, len); dst += len;
			} else {
				if (code == 0xFF) {
					len = READ_LE_UINT16(src); src += 2;
				}
				int offs = READ_LE_UINT16(src); src += 2;
				if (len > count) {
					len = count;
				}
				const uint8 *dstOffs = dstOrig + offs;
				while (len--) {
					*dst++ = *dstOffs++;
				}
			}				
		} else if (code != 0x80) {
			int len = MIN(count, code & 0x3F);
			while (len--) {
				*dst++ = *src++;
			}
		} else {
			break;
		}
	}
}

void Screen::decodeFrameDelta(uint8 *dst, const uint8 *src) {
	debug(9, "Screen::decodeFrameDelta(0x%X, 0x%X)", dst, src);
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				*dst++ ^= code;
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							*dst++ ^= code;
						}
					} else {
						while (subcode--) {
							*dst++ ^= *src++;
						}
					}
				} else {
					dst += subcode;
				}
			}
		} else {
			while (code--) {
				*dst++ ^= *src++;
			}
		}
	}
}

void Screen::decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch) {
	debug(9, "Screen::decodeFrameDeltaPage(0x%X, 0x%X, %d)", dst, src, pitch);
	int count = 0;
	uint8 *dstNext = dst;
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				*dst++ ^= code;
				if (++count == pitch) {
					count = 0;
					dstNext += SCREEN_W;
					dst = dstNext;
				}
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;
				
				count += code;
				while (count >= pitch) {
					count -= pitch;
					dstNext += SCREEN_W;
					dst = dstNext + count;
				}
				
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							*dst++ ^= code;
							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					} else {
						while (subcode--) {
							*dst++ ^= *src++;
							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					}
				} else {
					dst += subcode;
					
					count += subcode;
					while (count >= pitch) {
						count -= pitch;
						dstNext += SCREEN_W;
						dst = dstNext + count;
					}
					
				}
			}
		} else {
			while (code--) {
				*dst++ ^= *src++;
				if (++count == pitch) {
					count = 0;
					dstNext += 320;
					dst = dstNext;
				}
			}
		}
	}
}

} // End of namespace Kyra
