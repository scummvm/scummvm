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

//Dynamically linked GAPI
typedef int (*tGXOpenInput)(void);
typedef struct GXKeyList (*tGXGetDefaultKeys)(int);
typedef int (*tGXCloseDisplay)(void);
typedef int (*tGXCloseInput)(void);
typedef int (*tGXSuspend)(void);
typedef int (*tGXResume)(void);
typedef struct GXDisplayProperties (*tGXGetDisplayProperties)(void);
typedef int (*tGXOpenDisplay)(HWND, unsigned long);
typedef int (*tGXEndDraw)(void);
typedef void* (*tGXBeginDraw)(void);

extern tGXOpenInput dynamicGXOpenInput;
extern tGXGetDefaultKeys dynamicGXGetDefaultKeys;
extern tGXCloseDisplay dynamicGXCloseDisplay;
extern tGXCloseInput dynamicGXCloseInput;
extern tGXSuspend dynamicGXSuspend;
extern tGXResume dynamicGXResume;
extern tGXGetDisplayProperties dynamicGXGetDisplayProperties;
extern tGXOpenDisplay dynamicGXOpenDisplay;
extern tGXEndDraw dynamicGXEndDraw;
extern tGXBeginDraw dynamicGXBeginDraw;
