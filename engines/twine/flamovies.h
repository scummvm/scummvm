/** @file movies.h
	@brief
	This file contains movies routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef FLAMOVIES_H
#define FLAMOVIES_H

#include "main.h"

/** FLA movie directory */
#define FLA_DIR "fla/"

/** FLA movie header structure */
typedef struct FLAHeaderStruct {
	/** FLA version */
	int8 version[6];
	/** Number of frames */
	int32 numOfFrames;
	/** Frames per second */
	int8 speed;
	/** Unknown var1 */
	int8 var1;
	/** Frame width */
	int16 xsize;
	/** Frame height */
	int16 ysize;
} FLAHeaderStruct;

/** FLA movie frame structure */
typedef struct FLAFrameDataStruct {
	/** Current frame size */
	int8 videoSize;
	/** Dummy variable */
	int8 dummy;
	/** Unknown frameVar0 */
	int32 frameVar0;
} FLAFrameDataStruct;

/** FLA movie sample structure */
typedef struct FLASampleStruct {
	/** Number os samples */
	int16 sampleNum;
	/** Sample frequency */
	int16 freq;
	/** Numbers of time to repeat */
	int16 repeat;
	/** Dummy variable */
	int8 dummy;
	/** Unknown x */
	uint8 x;
	/** Unknown y */
	uint8 y;
} FLASampleStruct;

/** FLA movie file buffer */
unsigned char flaBuffer[FLASCREEN_WIDTH*FLASCREEN_HEIGHT];

/** Play FLA movies
	@param flaName FLA movie name */
void playFlaMovie(int8 *flaName);

#endif
