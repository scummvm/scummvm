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

#include <cstring>

#include "common/scummsys.h"

#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/define.h"
#include "trecision/graphics.h"
#include "trecision/trecision.h"

namespace Trecision {

void SDText::set(SDText org) {
	set(org.x, org.y, org.dx, org.dy, org._subtitleRect.left, org._subtitleRect.top, org._subtitleRect.right, org._subtitleRect.bottom, org.tcol, org.scol, org.text);
}

void SDText::set(uint16 px, uint16 py, uint16 pdx, uint16 pdy, uint16 pl0, uint16 pl1, uint16 pl2, uint16 pl3, uint16 ptcol, uint16 pscol, const char *psign) {
	x = px;
	y = py;
	dx = pdx;
	dy = pdy;
	_subtitleRect.left = pl0;
	_subtitleRect.top = pl1;
	_subtitleRect.right = pl2;
	_subtitleRect.bottom = pl3;
	tcol = ptcol;
	scol = pscol;
	text = psign;
}

void SDText::clear() {
	set(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr);
}
/*-------------------------------------------------------------
   checkDText - Computes and returns the dy of the given DText
--------------------------------------------------------------*/
uint16 SDText::checkDText() {
	if (text == nullptr)
		return 0;

	uint8 curLine = 0;
	if (g_vm->textLength(text, 0) <= dx) {
		strcpy((char *)DTextLines[curLine], text);
		return CARHEI;
	}

	uint16 a = 0;
	uint16 tmpDy = 0;
	uint16 lastSpace = 0;
	uint16 curInit = 0;

	while (a < strlen(text)) {
		a++;
		if (text[a] == ' ') {
			if (g_vm->textLength(text + curInit, a - curInit) <= dx)
				lastSpace = a;
			else if (g_vm->textLength(text + curInit, lastSpace - curInit) <= dx) {
				uint16 b;
				for (b = curInit; b < lastSpace; b++)
					DTextLines[curLine][b - curInit] = text[b];

				DTextLines[curLine][b - curInit] = '\0';
				curLine++;

				curInit = lastSpace + 1;

				tmpDy += CARHEI;
				a = curInit;
			} else
				return 0;
		} else if (text[a] == '\0') {
			if (g_vm->textLength(text + curInit, a - curInit) <= dx) {
				uint16 b;
				for (b = curInit; b < a; b++)
					DTextLines[curLine][b - curInit] = text[b];
				DTextLines[curLine][b - curInit] = '\0';

				tmpDy += CARHEI;

				return tmpDy;
			}

			if (g_vm->textLength(text + curInit, lastSpace - curInit) <= dx) {
				uint16 b;
				for (b = curInit; b < lastSpace; b++)
					DTextLines[curLine][b - curInit] = text[b];

				DTextLines[curLine][b - curInit] = '\0';
				curLine++;

				curInit = lastSpace + 1;
				tmpDy += CARHEI;

				if (curInit < strlen(text)) {
					for (b = curInit; b < strlen(text); b++)
						DTextLines[curLine][b - curInit] = text[b];

					DTextLines[curLine][b - curInit] = '\0';

					tmpDy += CARHEI;
				}
				return tmpDy;
			}
			return 0;
		}
	}
	return 0;
}

void SDText::DText(uint16 *frameBuffer) {
	uint16 tmpTCol = tcol;
	uint16 tmpSCol = scol;
	g_vm->_graphicsMgr->updatePixelFormat(&tmpTCol, 1);
	if (scol != MASKCOL)
		g_vm->_graphicsMgr->updatePixelFormat(&tmpSCol, 1);

	if (text == nullptr)
		return;

	uint16 *buffer = (frameBuffer == nullptr) ? g_vm->_screenBuffer : frameBuffer;
	uint16 curDy = checkDText();

	for (uint16 b = 0; b < (curDy / CARHEI); b++) {
		char *curText = (char *)DTextLines[b];
		uint16 inc = (dx - g_vm->textLength(curText, 0)) / 2;
		uint16 len = strlen(curText);

		if (len >= MAXCHARS) {
			strcpy(curText, g_vm->_sysText[kMessageError]);
			len = strlen(curText);
		}

		for (uint16 c = 0; c < len; c++) {
			byte curChar = curText[c]; /* legge prima parte del font */

			const uint16 charOffset = g_vm->_font[curChar * 3] + (uint16)(g_vm->_font[curChar * 3 + 1] << 8);
			uint16 fontDataOffset = 768;
			const uint16 charWidth = g_vm->_font[curChar * 3 + 2];

			if (c == len - 1 && g_vm->_blinkLastDTextChar != MASKCOL)
				tmpTCol = g_vm->_blinkLastDTextChar;

			for (uint16 a = b * CARHEI; a < (b + 1) * CARHEI; a++) {
				uint16 curPos = 0;
				uint16 CurColor = tmpSCol;

				while (curPos <= charWidth - 1) {
					if (a >= _subtitleRect.top && a < _subtitleRect.bottom) {
						if (CurColor != MASKCOL && (g_vm->_font[charOffset + fontDataOffset])) {
							const uint16 charLeft = inc + curPos;
							const uint16 charRight = charLeft + g_vm->_font[charOffset + fontDataOffset];
							uint16 *dst1 = buffer + x + charLeft + (y + a) * MAXX;
							uint16 *dst2 = buffer + x + _subtitleRect.left + (y + a) * MAXX;
							uint16 *dst = nullptr;
							uint16 size = 0;

							if (charLeft >= _subtitleRect.left && charRight < _subtitleRect.right) {
								dst = dst1;
								size = charRight - charLeft;
							} else if (charLeft < _subtitleRect.left && charRight < _subtitleRect.right && charRight > _subtitleRect.left) {
								dst = dst2;
								size = charRight - _subtitleRect.left;
							} else if (charLeft >= _subtitleRect.left && charRight >= _subtitleRect.right && _subtitleRect.right > charLeft) {
								dst = dst1;
								size = _subtitleRect.right - charLeft;
							} else if (charLeft < _subtitleRect.left && charRight >= _subtitleRect.right && _subtitleRect.right > charLeft) {
								dst = dst2;
								size = _subtitleRect.right - _subtitleRect.left;
							}

							if (dst && size > 0) {
								uint16 *d = dst;
								for (uint32 i = 0; i < size; i++)
									*d++ = CurColor;
							}
						}
					}

					curPos += g_vm->_font[charOffset + fontDataOffset];
					fontDataOffset++;

					if (CurColor == tmpSCol)
						CurColor = 0;
					else if (CurColor == 0)
						CurColor = tmpTCol;
					else if (CurColor == tmpTCol)
						CurColor = tmpSCol;
				}
			}
			inc += charWidth;
		}
	}
}

} // End of namespace Trecision
