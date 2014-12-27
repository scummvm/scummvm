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

#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/text.h"
#include "lab/vga.h"

namespace Lab {

/*****************************************************************************/
/* Opens up a font from disk, but uses buffer memory to store it in.         */
/*****************************************************************************/
bool openFontMem(const char *TextFontPath, struct TextFont *tf, byte *fontbuffer) {
	byte **file = NULL;
	char header[5];
	int32 filesize, headersize = 4L + 2L + 256 * 3 + 4L;

	filesize = sizeOfFile(TextFontPath);
	file = g_music->newOpen(TextFontPath);

	if ((file != NULL) && (filesize > headersize)) {
		header[4] = 0;
		readBlock(&header, 4L, file);

		if (strcmp(header, "VGAF") == 0) {
			tf->DataLength = filesize - headersize;
			readBlock(&(tf->Height), 2L, file);
			swapUShortPtr(&(tf->Height), 1);

			readBlock(tf->Widths, 256L, file);
			readBlock(tf->Offsets, 256L * 2L, file);
			swapUShortPtr(tf->Offsets, 256);

			skip(file, 4L);
			tf->data = fontbuffer;
			readBlock(tf->data, tf->DataLength, file);
			return true;
		}
	}

	return false;
}


/*****************************************************************************/
/* Opens up a font from disk.                                                */
/*****************************************************************************/
bool openFont(const char *TextFontPath, struct TextFont **tf) {
	byte **file = NULL;
	char header[5];
	int32 filesize, headersize = 4L + 2L + 256 * 3 + 4L;

	if ((*tf = (TextFont *)calloc(sizeof(struct TextFont), 1))) {
		filesize = sizeOfFile(TextFontPath);
		file = g_music->newOpen(TextFontPath);

		if ((file != NULL) && (filesize > headersize)) {
			header[4] = 0;
			readBlock(&header, 4L, file);

			if (strcmp(header, "VGAF") == 0) {
				(*tf)->DataLength = filesize - headersize;
				readBlock(&((*tf)->Height), 2L, file);
				swapUShortPtr(&((*tf)->Height), 1);

				readBlock((*tf)->Widths, 256L, file);
				readBlock((*tf)->Offsets, 256L * 2L, file);
				swapUShortPtr((*tf)->Offsets, 256);

				skip(file, 4L);

				if (((*tf)->data = (byte *)calloc((*tf)->DataLength, 1))) {
					readBlock((*tf)->data, (*tf)->DataLength, file);
					return true;
				}
			}
		}

		free(*tf);
	}

	*tf = NULL;
	return false;
}


/*****************************************************************************/
/* Closes a font and frees all memory associated with it.                    */
/*****************************************************************************/
void closeFont(struct TextFont *tf) {
	if (tf) {
		if (tf->data && tf->DataLength)
			free(tf->data);

		free(tf);
	}
}



/*****************************************************************************/
/* Returns the length of a text in the specified font.                       */
/*****************************************************************************/
uint16 textLength(struct TextFont *tf, const char *text, uint16 numchars) {
	uint16 counter, length = 0;

	if (tf)
		for (counter = 0; counter < numchars; counter++) {
			length += tf->Widths[(uint)*text];
			text++;
		}

	return length;
}



/*****************************************************************************/
/* Returns the height of a specified font.                                   */
/*****************************************************************************/
uint16 textHeight(struct TextFont *tf) {
	if (tf)
		return tf->Height;
	else
		return 0;
}



extern uint32 VGAScreenWidth, VGABytesPerPage;


/*****************************************************************************/
/* Draws the text to the screen.                                             */
/*****************************************************************************/
void text(struct TextFont *tf, uint16 x, uint16 y, uint16 color, const char *text, uint16 numchars) {
	byte *VGATop, *VGACur, *VGATemp, *VGATempLine, *cdata;
	uint32 RealOffset, SegmentOffset;
	int32 templeft, LeftInSegment;
	uint16 counter, counterb, bwidth, mask, curpage, rows, cols, data;

	VGATop = getVGABaseAddr();

	for (counter = 0; counter < numchars; counter++) {
		RealOffset = (VGAScreenWidth * y) + x;
		curpage    = RealOffset / VGABytesPerPage;
		SegmentOffset = RealOffset - (curpage * VGABytesPerPage);
		LeftInSegment = VGABytesPerPage - SegmentOffset;
		VGACur = VGATop + SegmentOffset;
		setPage(curpage);

		if (tf->Widths[(uint)*text]) {
			cdata = tf->data + tf->Offsets[(uint)*text];
			bwidth = *cdata;
			cdata++;
			VGATemp = VGACur;
			VGATempLine = VGACur;

			for (rows = 0; rows < tf->Height; rows++) {
				VGATemp = VGATempLine;
				templeft = LeftInSegment;

				for (cols = 0; cols < bwidth; cols++) {
					data = *cdata;
					cdata++;

					if (data && (templeft >= 8)) {
						if (0x80 & data)
							*VGATemp = color;

						VGATemp++;

						if (0x40 & data)
							*VGATemp = color;

						VGATemp++;

						if (0x20 & data)
							*VGATemp = color;

						VGATemp++;

						if (0x10 & data)
							*VGATemp = color;

						VGATemp++;

						if (0x08 & data)
							*VGATemp = color;

						VGATemp++;

						if (0x04 & data)
							*VGATemp = color;

						VGATemp++;

						if (0x02 & data)
							*VGATemp = color;

						VGATemp++;

						if (0x01 & data)
							*VGATemp = color;

						VGATemp++;

						templeft -= 8;
					} else if (data) {
						mask = 0x80;
						templeft = LeftInSegment;

						for (counterb = 0; counterb < 8; counterb++) {
							if (templeft <= 0) {
								curpage++;
								setPage(curpage);
								VGATemp = (byte *)(VGATop - templeft);
								/* Set up VGATempLine for next line */
								VGATempLine -= VGABytesPerPage;
								/* Set up LeftInSegment for next line */
								LeftInSegment += VGABytesPerPage + templeft;
								templeft += VGABytesPerPage;
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

				VGATempLine += VGAScreenWidth;
				LeftInSegment -= VGAScreenWidth;

				if (LeftInSegment <= 0) {
					curpage++;
					setPage(curpage);
					VGATempLine -= VGABytesPerPage;
					LeftInSegment += VGABytesPerPage;
				}
			}
		}

		x += tf->Widths[(int)*text];
		text++;
	}

	ungetVGABaseAddr();
}

} // End of namespace Lab
