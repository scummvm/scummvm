/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef _MAINDRAW_H_
#define _MAINDRAW_H_

namespace Cruise {

extern int currentTransparent;
extern int16 polyBuffer3[404];
extern int16 polyBuffer2[512];
extern int m_color;

int upscaleValue(int value, int scale);

void pixel(int x, int y, char color);
void mainDraw(int16 param);
void flipScreen(void);
void buildPolyModel(int X, int Y, int scale, char* ptr2, char* destBuffer, char* dataPtr);
void getPolyData(int fileIndex, int X, int Y, int *newScale, int *newY, int *newX, char **newDataPtr, int scale, char* dataPtr);
void mainDrawSub4(int objX1, int var_6, objectStruct* currentObjPtr, char* data1, int objY2, int objX2, char* output, char* data2);
char* drawPolyMode2(char* si, int cx);

} // End of namespace Cruise

#endif

