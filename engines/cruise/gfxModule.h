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
	int field_1;
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

void gfxModuleData_gfxClearFrameBuffer(uint8 * ptr);
void gfxModuleData_setDirtyColors(int min, int max);
void gfxModuleData_setPalColor(int idx, int r, int g, int b);
void gfxModuleData_field_90(void);
void gfxModuleData_gfxWaitVSync(void);
void gfxModuleData_flip(void);
void gfxModuleData_field_64(char *sourceBuffer, int width, int height, char *dest, int x, int y, int color);
void gfxModuleData_gfxCopyScreen(char *sourcePtr, char *destPtr);
void convertGfxFromMode4(uint8 *sourcePtr, int width, int height, uint8 *destPtr);
void convertGfxFromMode5(uint8 *sourcePtr, int width, int height, uint8 *destPtr);
void gfxModuleData_flipScreen(void);
//void gfxModuleData_setPal(uint8 * ptr);
void gfxModuleData_convertOldPalColor(uint16 oldColor, uint8* pOutput);
void gfxModuleData_setPal256(uint8 * ptr);
void flip(void);
void drawSolidBox(int32 x1, int32 y1, int32 x2, int32 y2, uint8 colour);

} // End of namespace Cruise

#endif
