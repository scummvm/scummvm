/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	D_DEBUG
#define D_DEBUG

// this whole file only included on debug versions
#ifdef _SWORD2_DEBUG

#include "bs2/driver.h"
#include "bs2/object.h"

namespace Sword2 {

#define MAX_DEBUG_TEXT_BLOCKS	50

extern uint8 displayDebugText;	// 0=off; 1=on
extern uint8 displayWalkGrid;
extern uint8 displayMouseMarker;
extern uint8 displayPlayerMarker;
extern uint8 displayTime;
extern uint8 displayTextNumbers;
extern uint8 definingRectangles;
extern uint8 draggingRectangle;
extern uint8 displayTime;
extern int32 startTime;
extern int32 gameCycle;
extern uint8 renderSkip;

extern int16 rect_x1;
extern int16 rect_y1;
extern int16 rect_x2;
extern int16 rect_y2;

extern	uint8 testingSnR;

extern	int32 textNumber;

extern Object_graphic playerGraphic;
extern uint32 player_graphic_no_frames;

#define MAX_SHOWVARS 15

extern int32 showVar[MAX_SHOWVARS];

void Build_debug_text(void);
void Draw_debug_graphics(void);

void Print_current_info(void);

} // End of namespace Sword2

#else	// ie. not _SWORD2_DEBUG

namespace Sword2 {

void Build_debug_text(void);
void Draw_debug_graphics(void);

} // End of namespace Sword2

#endif

#endif
