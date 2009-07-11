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
 * $URL$
 * $Id$
 *
 */

#ifndef CRUISE_GFXMODULE_H
#define CRUISE_GFXMODULE_H

namespace Cruise {

struct gfxModuleDataStruct {
	int useTandy;
	int useEGA;
	int useVGA;

	uint8 *pPage00;
	uint8 *pPage10;
};

struct palEntry {
	uint8 R;
	uint8 G;
	uint8 B;
	uint8 A;
};

extern gfxModuleDataStruct gfxModuleData;

void gfxModuleData_gfxClearFrameBuffer(uint8 *ptr);
void gfxModuleData_setDirtyColors(int min, int max);
void gfxModuleData_setPalColor(int idx, int r, int g, int b);
void gfxModuleData_field_90(void);
void gfxModuleData_gfxWaitVSync(void);
void gfxModuleData_flip(void);
void gfxCopyRect(const uint8 *sourceBuffer, int width, int height, byte *dest, int x, int y, int colour);
void gfxModuleData_gfxCopyScreen(const uint8 *sourcePtr, uint8 *destPtr);
void convertGfxFromMode4(const uint8 *sourcePtr, int width, int height, uint8 *destPtr);
void convertGfxFromMode5(const uint8 *sourcePtr, int width, int height, uint8 *destPtr);
void gfxModuleData_Init(void);
void gfxModuleData_flipScreen(void);
//void gfxModuleData_setPal(uint8 * ptr);
void gfxModuleData_convertOldPalColor(uint16 oldColor, uint8 *pOutput);
void gfxModuleData_setPalEntries(const byte *ptr, int start, int num);
void gfxModuleData_setPal256(const byte *ptr);
void flip(void);
void drawSolidBox(int32 x1, int32 y1, int32 x2, int32 y2, uint8 colour);
void resetBitmap(uint8 *dataPtr, int32 dataSize);

} // End of namespace Cruise

#endif
