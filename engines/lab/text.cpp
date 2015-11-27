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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"
#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/text.h"

namespace Lab {

/*****************************************************************************/
/* Closes a font and frees all memory associated with it.                    */
/*****************************************************************************/
void closeFont(struct TextFont *tf) {
	if (tf) {
		if (tf->data && tf->DataLength)
			delete[] tf->data;

		free(tf);
	}
}

/*****************************************************************************/
/* Returns the length of a text in the specified font.                       */
/*****************************************************************************/
uint16 textLength(struct TextFont *tf, const char *text, uint16 numchars) {
	uint16 length = 0;

	if (tf) {
		for (uint16 i = 0; i < numchars; i++) {
			length += tf->Widths[(uint)*text];
			text++;
		}
	}

	return length;
}

/*****************************************************************************/
/* Returns the height of a specified font.                                   */
/*****************************************************************************/
uint16 textHeight(struct TextFont *tf) {
	return (tf) ? tf->Height : 0;
}

/*****************************************************************************/
/* Draws the text to the screen.                                             */
/*****************************************************************************/
void text(struct TextFont *tf, uint16 x, uint16 y, uint16 color, const char *text, uint16 numchars) {
	byte *VGATop, *VGACur, *VGATemp, *VGATempLine, *cdata;
	uint32 RealOffset, SegmentOffset;
	int32 templeft, LeftInSegment;
	uint16 bwidth, mask, curpage, data;

	VGATop = g_lab->getVGABaseAddr();

	for (uint16 i = 0; i < numchars; i++) {
		RealOffset = (g_lab->_screenWidth * y) + x;
		curpage    = RealOffset / g_lab->_screenBytesPerPage;
		SegmentOffset = RealOffset - (curpage * g_lab->_screenBytesPerPage);
		LeftInSegment = g_lab->_screenBytesPerPage - SegmentOffset;
		VGACur = VGATop + SegmentOffset;

		if (tf->Widths[(uint)*text]) {
			cdata = tf->data + tf->Offsets[(uint)*text];
			bwidth = *cdata++;
			VGATemp = VGACur;
			VGATempLine = VGACur;

			for (uint16 rows = 0; rows < tf->Height; rows++) {
				VGATemp = VGATempLine;
				templeft = LeftInSegment;

				for (uint16 cols = 0; cols < bwidth; cols++) {
					data = *cdata++;

					if (data && (templeft >= 8)) {
						for (int j = 7; j >= 0; j--) {
							if ((1 << j) & data)
								*VGATemp = color;
							VGATemp++;
						}

						templeft -= 8;
					} else if (data) {
						mask = 0x80;
						templeft = LeftInSegment;

						for (uint16 counterb = 0; counterb < 8; counterb++) {
							if (templeft <= 0) {
								curpage++;
								VGATemp = (byte *)(VGATop - templeft);
								/* Set up VGATempLine for next line */
								VGATempLine -= g_lab->_screenBytesPerPage;
								/* Set up LeftInSegment for next line */
								LeftInSegment += g_lab->_screenBytesPerPage + templeft;
								templeft += g_lab->_screenBytesPerPage;
							}

							if (mask & data)
								*VGATemp = color;

							VGATemp++;

							mask = mask >> 1;
							templeft--;
						}
					} else {
						templeft -= 8;
						VGATemp += 8;
					}
				}

				VGATempLine += g_lab->_screenWidth;
				LeftInSegment -= g_lab->_screenWidth;

				if (LeftInSegment <= 0) {
					curpage++;
					VGATempLine -= g_lab->_screenBytesPerPage;
					LeftInSegment += g_lab->_screenBytesPerPage;
				}
			}
		}

		x += tf->Widths[(int)*text];
		text++;
	}
}

} // End of namespace Lab
