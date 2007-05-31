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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/font.h $
 * $Id:font.h 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#ifndef CRUISE_FONT_H
#define CRUISE_FONT_H

namespace Cruise {

void loadFNT(const void *fileName);
void loadSystemFont(void);

//////////////////////////////////////////////////
void flipShort(int16 * var);
void flipShort(uint16 * var);
void flipLong(int32 * var);	// TODO: move away
void flipLong(uint32 * var);	// TODO: move away
void flipGen(void *var, int32 length);

int32 getLineHeight(int16 charCount, uint8 * fontPtr, uint8 * fontPrt_Desc);	// fontProc1
int32 getTextLineCount(int32 rightBorder_X, int32 wordSpacingWidth, uint8 * ptr, const uint8 *textString);	// fontProc2

void renderWord(uint8 * fontPtr_Data, uint8 * outBufferPtr,
    int32 drawPosPixel_X, int32 heightOff, int32 height, int32 param4,
    int32 stringRenderBufferSize, int32 width, int32 charWidth);
gfxEntryStruct *renderText(int inRightBorder_X, const uint8 *string);
void drawString(int32 x, int32 y, uint8 * string, uint8 * buffer, uint8 color,
    int32 inRightBorder_X);

} // End of namespace Cruise

#endif
