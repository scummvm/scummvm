/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

/* Original code by Vasyl Tsvirkunov */

#ifndef SCREEN_H
#define SCREEN_H

#ifndef UBYTE
#define UBYTE unsigned char
#endif

#ifndef INT16
#define INT16 signed short
#endif

#define GAME_SELECTION_X_OFFSET 15
#define GAME_SELECTION_Y_OFFSET 25

#define GAPI_SIMU 0x123456

void SetScreenGeometry(int w, int h);
void LimitScreenGeometry();
void RestoreScreenGeometry();
int GraphicsOn(HWND hWndMain, bool gfx_mode_switch);
void GraphicsOff();
void GraphicsSuspend();
void GraphicsResume();

void SetPalEntry(int ent, UBYTE r, UBYTE g, UBYTE b);
void Blt(UBYTE * scr_ptr);
void Blt_part(UBYTE * src_ptr, int x, int y, int width, int height, int pitch, bool check = false);
void checkToolbar();

void beginBltPart();
void endBltPart();

void Get_565(UBYTE *src, INT16 *buffer, int pitch, int x, int y, int width, int height);
void Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height);

int getColor565 (int color);

/* meaning: 0 - portrait, 1 - left hand landscape, 2 - right hand landscape */
void SetScreenMode(int mode);
int GetScreenMode();
void drawWait();
void drawVideoDevice();

void setGameSelectionPalette();
void drawBlankGameSelection();
void reducePortraitGeometry();
void drawCommentString(char *);
void drawStandardString(char *, int);
void drawHighlightedString(char *, int);
void drawWaitSelectKey();
void resetLastHighlighted();

void Translate(int* x, int* y);

typedef enum ToolbarSelected {
	ToolbarNone,
	ToolbarSaveLoad,
	ToolbarMode,
	ToolbarSkip,
	ToolbarSound
} ToolbarSelected;
	

#endif
