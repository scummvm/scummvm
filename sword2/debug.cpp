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

#include "stdafx.h"
#include "sword2/driver/driver96.h"
#include "sword2/sword2.h"
#include "sword2/debug.h"
#include "sword2/console.h"
#include "sword2/defs.h"
#include "sword2/events.h"			// for CountEvents()
#include "sword2/layers.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/protocol.h"
#include "sword2/resman.h"
#include "sword2/router.h"			// for plotWalkGrid()
#include "sword2/speech.h"			// for 'officialTextNumber' and
						// 'speechScriptWaiting'

namespace Sword2 {

bool displayDebugText = false;		// "INFO"
bool displayWalkGrid = false;		// "WALKGRID"
bool displayMouseMarker = false;	// "MOUSE"
bool displayTime = false;		// "TIME"
bool displayPlayerMarker = false;	// "PLAYER"
bool displayTextNumbers = false;	// "TEXT"
uint8 renderSkip = 0;			// Toggled on 'S' key - to render only
					// 1 in 4 frames, to speed up game

bool definingRectangles = false;	// "RECT"
uint8 draggingRectangle = 0;		// 0 = waiting to start new rect;
					// 1 = currently dragging a rectangle
int16 rect_x1 = 0;
int16 rect_y1 = 0;
int16 rect_x2 = 0;
int16 rect_y2 = 0;
bool rectFlicker = false;

bool testingSnR = false;		// "SAVEREST" - for system to kill all
					// object resources (except player) in
					// fnAddHuman()

int32 startTime = 0;			// "TIMEON" & "TIMEOFF" - system start
					// time.
int32 gameCycle = 0;			// Counter for game clocks.

int32 textNumber = 0;			// Current system text line number

int32 showVar[MAX_SHOWVARS];		// "SHOWVAR"

Object_graphic playerGraphic;		// For displaying player object's
					// current graphical info
uint32 player_graphic_no_frames = 0;	// No. of frames in currently displayed
					// anim

uint8 debug_text_blocks[MAX_DEBUG_TEXT_BLOCKS];

void Clear_debug_text_blocks(void);
void Make_debug_text_block(char *text, int16 x, int16 y);
void Plot_cross_hair(int16 x, int16 y, uint8 pen);
void DrawRect(int16 x, int16 y, int16 x2, int16 y2, uint8 pen);

void Clear_debug_text_blocks(void) {
	uint8 blockNo = 0;

	while (blockNo < MAX_DEBUG_TEXT_BLOCKS && debug_text_blocks[blockNo] > 0) {
		// kill the system text block
		fontRenderer.killTextBloc(debug_text_blocks[blockNo]);

		// clear this element of our array of block numbers
		debug_text_blocks[blockNo] = 0;

		blockNo++;
	}
}

void Make_debug_text_block(char *text, int16 x, int16 y) {
	uint8 blockNo = 0;

	while (blockNo < MAX_DEBUG_TEXT_BLOCKS && debug_text_blocks[blockNo] > 0)
		blockNo++;

	if (blockNo == MAX_DEBUG_TEXT_BLOCKS)
		error("ERROR: debug_text_blocks[] full in Make_debug_text_block()");

	debug_text_blocks[blockNo] = fontRenderer.buildNewBloc((uint8 *) text, x, y, 640 - x, 0, RDSPR_DISPLAYALIGN, CONSOLE_FONT_ID, NO_JUSTIFICATION);
}

void Build_debug_text(void) {
	char buf[128];

	int32 showVarNo;		// for variable watching
	int32 showVarPos;
	int32 varNo;
	int32 *varTable;

	// clear the array of text block numbers for the debug text
	Clear_debug_text_blocks();

	// mouse coords
/*
	// print mouse coords beside mouse-marker, if it's being displayed
	if (displayMouseMarker) {
		sprintf(buf, "%d,%d", mousex + this_screen.scroll_offset_x, mousey + this_screen.scroll_offset_y);
		if (mousex>560)
			Make_debug_text_block(buf, mousex - 50, mousey - 15);
		else
			Make_debug_text_block(buf, mousex + 5, mousey - 15);
	}
*/

	// mouse area coords

	// defining a mouse area the easy way, by creating a box on-screen
	if (draggingRectangle || SYSTEM_TESTING_ANIMS) {
		// so we can see what's behind the lines
		rectFlicker = !rectFlicker;

		sprintf (buf, "x1=%d", rect_x1);
		Make_debug_text_block(buf, 0, 120);

		sprintf (buf, "y1=%d", rect_y1);
		Make_debug_text_block(buf, 0, 135);

		sprintf (buf, "x2=%d", rect_x2);
		Make_debug_text_block(buf, 0, 150);

		sprintf (buf, "y2=%d", rect_y2);
		Make_debug_text_block(buf, 0, 165);
	}

	// testingSnR indicator

	if (testingSnR) {		// see fnAddHuman()
		sprintf (buf, "TESTING LOGIC STABILITY!");
		Make_debug_text_block(buf, 0, 105);
	}

	// speed-up indicator

	if (renderSkip) {		// see sword.cpp
		sprintf (buf, "SKIPPING FRAMES FOR SPEED-UP!");
		Make_debug_text_block(buf, 0, 120);
	}

	// debug info at top of screen - enabled/disabled as one complete unit

	if (displayTime) {
		int32 time = SVM_timeGetTime();

		if ((time - startTime) / 1000 >= 10000)
			startTime = time;

		time -= startTime;
		sprintf(buf, "Time %.2d:%.2d:%.2d.%.3d", (time / 3600000) % 60, (time / 60000) % 60, (time / 1000) % 60, time % 1000);
		Make_debug_text_block(buf, 500, 360);
		sprintf(buf, "Game %d", gameCycle);
		Make_debug_text_block(buf, 500, 380);
	}

   	// current text number & speech-sample resource id

	if (displayTextNumbers) {
		if (textNumber) {
			if (SYSTEM_TESTING_TEXT) {
				if (SYSTEM_WANT_PREVIOUS_LINE)
					sprintf(buf, "backwards");
				else
 					sprintf(buf, "forwards");

				Make_debug_text_block(buf, 0, 340);
			}

			sprintf(buf, "res: %d", textNumber / SIZE);
			Make_debug_text_block(buf, 0, 355);

			sprintf(buf, "pos: %d", textNumber & 0xffff);
			Make_debug_text_block(buf, 0, 370);

 			sprintf(buf, "TEXT: %d", officialTextNumber);
			Make_debug_text_block(buf, 0, 385);
		}
	}

	// resource number currently being checking for animation

	if (SYSTEM_TESTING_ANIMS) {
		sprintf(buf, "trying resource %d", SYSTEM_TESTING_ANIMS);
		Make_debug_text_block(buf, 0, 90);
	}

	// general debug info

	if (displayDebugText) {
/*
		// CD in use
		sprintf (buf, "CD-%d", currentCD);
		Make_debug_text_block(buf, 0, 0);
*/

		// mouse coords & object pointed to

		if (CLICKED_ID)
			sprintf(buf, "last click at %d,%d (id %d: %s)",
				MOUSE_X, MOUSE_Y, CLICKED_ID,
				FetchObjectName(CLICKED_ID));
		else
			sprintf(buf, "last click at %d,%d (---)",
				MOUSE_X, MOUSE_Y);

 		Make_debug_text_block(buf, 0, 15);

		if (mouse_touching)
			sprintf(buf, "mouse %d,%d (id %d: %s)",
				g_display->_mouseX + this_screen.scroll_offset_x,
				g_display->_mouseY + this_screen.scroll_offset_y,
				mouse_touching,
				FetchObjectName(mouse_touching));
		else
			sprintf(buf, "mouse %d,%d (not touching)",
				g_display->_mouseX + this_screen.scroll_offset_x,
				g_display->_mouseY + this_screen.scroll_offset_y);

		Make_debug_text_block(buf, 0, 30);

 		// player coords & graphic info
		// if player objct has a graphic

		if (playerGraphic.anim_resource)
			sprintf(buf, "player %d,%d %s (%d) #%d/%d",
				this_screen.player_feet_x,
				this_screen.player_feet_y,
				FetchObjectName(playerGraphic.anim_resource),
				playerGraphic.anim_resource,
				playerGraphic.anim_pc,
				player_graphic_no_frames);
		else
			sprintf(buf, "player %d,%d --- %d",
				this_screen.player_feet_x,
				this_screen.player_feet_y,
				playerGraphic.anim_pc);

		Make_debug_text_block(buf, 0, 45);

 		// frames-per-second counter

		sprintf(buf, "fps %d", g_sword2->_fps);
		Make_debug_text_block(buf, 440, 0);

 		// location number

		sprintf(buf, "location=%d", LOCATION);
		Make_debug_text_block(buf, 440, 15);

 		// "result" variable

		sprintf(buf, "result=%d", RESULT);
		Make_debug_text_block(buf, 440, 30);

 		// no. of events in event list

		sprintf(buf, "events=%d", CountEvents());
		Make_debug_text_block(buf, 440, 45);

		// sprite list usage

		sprintf(buf, "bgp0: %d/%d", g_sword2->_curBgp0, MAX_bgp0_sprites);
		Make_debug_text_block(buf, 560, 0);

		sprintf(buf, "bgp1: %d/%d", g_sword2->_curBgp1, MAX_bgp1_sprites);
		Make_debug_text_block(buf, 560, 15);

		sprintf(buf, "back: %d/%d", g_sword2->_curBack, MAX_back_sprites);
		Make_debug_text_block(buf, 560, 30);

		sprintf(buf, "sort: %d/%d", g_sword2->_curSort, MAX_sort_sprites);
		Make_debug_text_block(buf, 560, 45);

		sprintf(buf, "fore: %d/%d", g_sword2->_curFore, MAX_fore_sprites);
		Make_debug_text_block(buf, 560, 60);

		sprintf(buf, "fgp0: %d/%d", g_sword2->_curFgp0, MAX_fgp0_sprites);
		Make_debug_text_block(buf, 560, 75);

		sprintf(buf, "fgp1: %d/%d", g_sword2->_curFgp1, MAX_fgp1_sprites);
		Make_debug_text_block(buf, 560, 90);

		// largest layer & sprite

		// NB. Strings already constructed in Build_display.cpp
		Make_debug_text_block(g_sword2->_largestLayerInfo, 0, 60);
		Make_debug_text_block(g_sword2->_largestSpriteInfo, 0, 75);

		// "waiting for person" indicator - set form fnTheyDo and
		// fnTheyDoWeWait

		if (speechScriptWaiting) {
			sprintf(buf, "script waiting for %s (%d)",
				FetchObjectName(speechScriptWaiting),
				speechScriptWaiting);
			Make_debug_text_block(buf, 0, 90);
		}

		// variable watch display

		showVarPos = 115;	// y-coord for first showVar

		// res 1 is the global variables resource
		varTable = (int32 *) (res_man.open(1) + sizeof(_standardHeader));

		for (showVarNo = 0; showVarNo < MAX_SHOWVARS; showVarNo++) {
			varNo = showVar[showVarNo];	// get variable number

			// if non-zero ie. cannot watch 'id' but not needed
			// anyway because it changes throughout the logic loop

			if (varNo) {
				sprintf(buf, "var(%d) = %d", varNo, varTable[varNo]);
				Make_debug_text_block(buf, 530, showVarPos);
				showVarPos += 15;	// next line down
			}
		}

		res_man.close(1);	// close global variables resource

		// memory indicator - this should come last, to show all the
		// sprite blocks above!

		memory.memoryString(buf);
		Make_debug_text_block(buf, 0, 0);
	}
}

void Draw_debug_graphics(void) {
	// walk-grid

	if (displayWalkGrid)
		router.plotWalkGrid(); 

	// player feet coord marker

	if (displayPlayerMarker)
		Plot_cross_hair(this_screen.player_feet_x, this_screen.player_feet_y, 215);

	// mouse marker & coords

	if (displayMouseMarker)
		Plot_cross_hair(g_display->_mouseX + this_screen.scroll_offset_x, g_display->_mouseY + this_screen.scroll_offset_y, 215);

   	// mouse area rectangle / sprite box rectangle when testing anims

	if (SYSTEM_TESTING_ANIMS) {
		// draw box around current frame
		DrawRect(rect_x1, rect_y1, rect_x2, rect_y2, 184);
	} else if (draggingRectangle) {
		// defining a mouse area the easy way, by creating a box
		// on-screen
		if (rectFlicker)
			DrawRect(rect_x1, rect_y1, rect_x2, rect_y2, 184);
	}
}

void Plot_cross_hair(int16 x, int16 y, uint8 pen) {
	g_display->plotPoint(x, y, pen);		// driver function

	g_display->drawLine(x - 2, y, x - 5, y, pen);	// driver function
	g_display->drawLine(x + 2, y, x + 5, y, pen);

	g_display->drawLine(x, y - 2, x, y - 5, pen);
	g_display->drawLine(x, y + 2, x, y + 5, pen);
}

void DrawRect(int16 x1, int16 y1, int16 x2, int16 y2, uint8 pen) {
	g_display->drawLine(x1, y1, x2, y1, pen);	// top edge
	g_display->drawLine(x1, y2, x2, y2, pen);	// bottom edge
	g_display->drawLine(x1, y1, x1, y2, pen);	// left edge
	g_display->drawLine(x2, y1, x2, y2, pen);	// right edge
}

void Print_current_info(void) {
	// prints general stuff about the screen, etc.

	if (this_screen.background_layer_id) {
		Debug_Printf("background layer id %d\n", this_screen.background_layer_id);
		Debug_Printf("%d wide, %d high\n", this_screen.screen_wide, this_screen.screen_deep);
		Debug_Printf("%d normal layers\n", this_screen.number_of_layers);

		g_logic.examineRunList();
	} else
		Debug_Printf("No screen\n");
}

} // End of namespace Sword2
