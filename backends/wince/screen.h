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

void SetScreenGeometry(int w, int h);
void LimitScreenGeometry();
void RestoreScreenGeometry();
int GraphicsOn(HWND hWndMain);
void GraphicsOff();
void GraphicsSuspend();
void GraphicsResume();

void SetPalEntry(int ent, UBYTE r, UBYTE g, UBYTE b);
void Blt(UBYTE * scr_ptr);

/* meaning: 0 - portrait, 1 - left hand landscape, 2 - right hand landscape */
void SetScreenMode(int mode);
int GetScreenMode();
void drawWait();

void Translate(int* x, int* y);

typedef enum ToolbarSelected {
	ToolbarNone,
	ToolbarSaveLoad,
	ToolbarMode,
	ToolbarSkip,
	ToolbarSound
} ToolbarSelected;
	

#endif
