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

//--------------------------------------------------------------------------------------
#include <stdarg.h> // for ExitWithReport, which stays in RELEASE version
#include <stdio.h>

#include "stdafx.h"
#include "driver/driver96.h"
#include "debug.h"
//--------------------------------------------------------------------------------------

#ifdef _BS2_DEBUG  // this whole file (except ExitWithReport) only included on debug versions

#include <stdlib.h>

#include "build_display.h"	// for 'fps' (frames-per-second counter)
#include "console.h"
#include "defs.h"
#include "events.h"	// for CountEvents()
#include "layers.h"
#include "logic.h"
#include "maketext.h"
#include "mem_view.h"
#include "mouse.h"
#include "protocol.h"
#include "resman.h"
#include "router.h"	// for PlotWalkGrid()
#include "speech.h"	// for 'officialTextNumber' and 'speechScriptWaiting'

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// global variables
uint8 displayDebugText		= 0;	// "INFO"		0=off; 1=on
uint8 displayWalkGrid		= 0;	// "WALKGRID"
uint8 displayMouseMarker	= 0;	// "MOUSE"
uint8 displayTime			= 0;	// "TIME"
uint8 displayPlayerMarker	= 0;	// "PLAYER"
uint8 displayTextNumbers	= 0;	// "TEXT"
uint8 renderSkip			= 0;	// Toggled on 'S' key - to render only 1 in 4 frames, to speed up game

uint8 definingRectangles	= 0;	// "RECT"
uint8 draggingRectangle		= 0;	// 0=waiting to start new rect; 1=currently dragging a rectangle
int16 rect_x1	= 0;
int16 rect_y1	= 0;
int16 rect_x2	= 0;
int16 rect_y2	= 0;
uint8 rectFlicker=0;

uint8 testingSnR = 0;				// "SAVEREST" - for system to kill all object resources (except player) in FN_add_human()

int32 startTime = 0;				// "TIMEON" & "TIMEOFF" - system start time.
int32 gameCycle = 0;				// Counter for game clocks.

int32 textNumber = 0;				// current system text line number

int32 showVar[MAX_SHOWVARS];		// "SHOWVAR"

Object_graphic playerGraphic;		// for displaying player object's current graphical info
uint32 player_graphic_no_frames=0;	// no. of frames in currently displayed anim

uint8 debug_text_blocks[MAX_DEBUG_TEXT_BLOCKS];

//--------------------------------------------------------------------------------------
// function prototypes

void Clear_debug_text_blocks( void );
void Make_debug_text_block( char *text, int16 x, int16 y );
void Plot_cross_hair( int16 x, int16 y, uint8 pen );
void DrawRect( int16 x, int16 y, int16 x2, int16 y2, uint8 pen );
//--------------------------------------------------------------------------------------
#endif	// _BS2_DEBUG

// THIS FUNCTION STAYS IN THE RELEASE VERSION
// IN FACT, CON_FATAL_ERROR IS MAPPED TO THIS AS WELL, SO WE HAVE A MORE PRESENTABLE ERROR REPORT
void ExitWithReport(const char *format,...)	// (6dec96 JEL)
{
	//	Send a printf type string to Paul's windows routine
	char buf[500];
	va_list arg_ptr;	// Variable argument pointer

	va_start(arg_ptr,format);


	vsprintf(buf, format, arg_ptr);
	Zdebug("%s",buf);		// send output to 'debug.txt' as well, just for the record

	while (GetFadeStatus())	// wait for fade to finish before calling RestoreDisplay()
		ServiceWindows();

	RestoreDisplay();
	ReportFatalError((const uint8 *)buf);	// display message box
	CloseAppWindow();
	while (ServiceWindows() != RDERR_APPCLOSED);

	exit(0);
}

#ifdef _BS2_DEBUG	// all other functions only for _BS2_DEBUG version
//--------------------------------------------------------------------------------------
void Zdebug(const char *format,...)	//Tony's special debug logging file March96
{
//	Write a printf type string to a debug file

	va_list		arg_ptr;			// Variable argument pointer
	FILE *		debug_filep=0;			// Debug file pointer
	static int	first_debug = 1;		// Flag for first time this is used

	va_start(arg_ptr,format);

	if (first_debug)					//First time round delete any previous debug file
	{
		unlink("debug.txt");
		first_debug = 0;
	}

	debug_filep = fopen("debug.txt","a+t");

	if (debug_filep != NULL)	// if it could be opened
	{
		vfprintf(debug_filep, format, arg_ptr);
		fprintf(debug_filep,"\n");

		fclose(debug_filep);
	}
}

//--------------------------------------------------------------------------------------
void Zdebug(uint32 stream, const char *format,...)	//Tony's special debug logging file March96
{
//	Write a printf type string to a debug file

	va_list		arg_ptr;			// Variable argument pointer
	FILE *		debug_filep=0;			// Debug file pointer
	static int	first = 1;		// Flag for first time this is used
	int	j;
	static	int	first_debugs[100];



	if	(first==1)	//first time run then reset the states
	{	for	(j=0;j<100;j++)
			first_debugs[j]=0;

		first=0;
	}




	char	name[20];


	sprintf(name, "debug%d.txt", stream);

	va_start(arg_ptr,format);

	if (!first_debugs[stream])					//First time round delete any previous debug file
	{
		unlink(name);
		first_debugs[stream] = 1;
	}

	debug_filep = fopen(name,"a+t");

	if (debug_filep != NULL)	// if it could be opened
	{
		vfprintf(debug_filep, format, arg_ptr);
		fprintf(debug_filep,"\n");

		fclose(debug_filep);
	}
}
//--------------------------------------------------------------------------------------
void Clear_debug_text_blocks( void )	// JAMES
{
	uint8 blockNo=0;


	while ((blockNo < MAX_DEBUG_TEXT_BLOCKS) && (debug_text_blocks[blockNo] > 0))
	{
		Kill_text_bloc(debug_text_blocks[blockNo]);	// kill the system text block
		debug_text_blocks[blockNo] = 0;				// clear this element of our array of block numbers
		blockNo++;
	}
}
//--------------------------------------------------------------------------------------
void Make_debug_text_block( char *text, int16 x, int16 y)	// JAMES
{
	uint8 blockNo=0;


	while ((blockNo < MAX_DEBUG_TEXT_BLOCKS) && (debug_text_blocks[blockNo] > 0))
		blockNo++;

	if (blockNo == MAX_DEBUG_TEXT_BLOCKS)
		Con_fatal_error("ERROR: debug_text_blocks[] full in Make_debug_text_block() at line %d in file \"%s\"",__LINE__,__FILE__);

	debug_text_blocks[blockNo] = Build_new_block( (uint8 *)text, x, y, 640-x, 0, RDSPR_DISPLAYALIGN, CONSOLE_FONT_ID, NO_JUSTIFICATION);
}

//--------------------------------------------------------------------------------------
//
//
// PC Build_debug_info
//
//
//--------------------------------------------------------------------------------------
void Build_debug_text( void )	// JAMES
{
	char buf[128];

	int32 showVarNo;	// for variable watching
	int32 showVarPos;
	int32 varNo;
	int32 *varTable;


	Clear_debug_text_blocks();	// clear the array of text block numbers for the debug text

	//-------------------------------------------------------------------
	// mouse coords
/*
	if (displayMouseMarker)	// print mouse coords beside mouse-marker, if it's being displayed
	{
		sprintf (buf, "%d,%d", mousex+this_screen.scroll_offset_x, mousey+this_screen.scroll_offset_y);
		if (mousex>560)
			Make_debug_text_block (buf, mousex-50, mousey-15);
		else
			Make_debug_text_block (buf, mousex+5, mousey-15);
	}
*/
	//-------------------------------------------------------------------
	// mouse area coords

	if (draggingRectangle || SYSTEM_TESTING_ANIMS)	// defining a mouse area the easy way, by creating a box on-screen
	{
		rectFlicker = 1-rectFlicker;	// so we can see what's behind the lines

		sprintf (buf, "x1=%d", rect_x1);
		Make_debug_text_block (buf, 0, 120);

		sprintf (buf, "y1=%d", rect_y1);
		Make_debug_text_block (buf, 0, 135);

		sprintf (buf, "x2=%d", rect_x2);
		Make_debug_text_block (buf, 0, 150);

		sprintf (buf, "y2=%d", rect_y2);
		Make_debug_text_block (buf, 0, 165);
	}
	//-------------------------------------------------------------------
	// testingSnR indicator

	if (testingSnR)		// see FN_add_human()
	{
		sprintf (buf, "TESTING LOGIC STABILITY!");
		Make_debug_text_block (buf, 0, 105);
	}
	//---------------------------------------------
	// speed-up indicator

	if (renderSkip)		// see sword.cpp
	{
		sprintf (buf, "SKIPPING FRAMES FOR SPEED-UP!");
		Make_debug_text_block (buf, 0, 120);
	}
	//---------------------------------------------
	// debug info at top of screen - enabled/disabled as one complete unit

	if (displayTime)
	{
		int32 time = timeGetTime();

		if ((time - startTime) / 1000 >= 10000)
			startTime = time;

		time -= startTime;
		sprintf(buf, "Time %.2d:%.2d:%.2d.%.3d",(time / 3600000) % 60,(time / 60000) % 60, (time / 1000) % 60,time%1000);
		Make_debug_text_block(buf, 500, 360);
		sprintf(buf, "Game %d", gameCycle);
		Make_debug_text_block(buf, 500, 380);
	}
	//---------------------------------------------
   	// current text number & speech-sample resource id

	if (displayTextNumbers)
	{
		if (textNumber)
		{
			if (SYSTEM_TESTING_TEXT)
			{
				if (SYSTEM_WANT_PREVIOUS_LINE)
					sprintf (buf, "backwards");
				else
 					sprintf (buf, "forwards");

				Make_debug_text_block (buf, 0, 340);
			}

			sprintf (buf, "res: %d", textNumber/SIZE);
			Make_debug_text_block (buf, 0, 355);

			sprintf (buf, "pos: %d", textNumber&0xffff);
			Make_debug_text_block (buf, 0, 370);

 			sprintf (buf, "TEXT: %d", officialTextNumber);
			Make_debug_text_block (buf, 0, 385);

		}
	}
	//---------------------------------------------
	// resource number currently being checking for animation

	if (SYSTEM_TESTING_ANIMS)
	{
		sprintf (buf, "trying resource %d", SYSTEM_TESTING_ANIMS);
		Make_debug_text_block (buf, 0, 90);
	}
	//---------------------------------------------

	// general debug info

	if (displayDebugText)
	{
		//---------------------------------------------
/*
		// CD in use
		sprintf (buf, "CD-%d", currentCD);
		Make_debug_text_block (buf, 0, 0);
*/
		//---------------------------------------------
		// mouse coords & object pointed to

		if (CLICKED_ID)
			sprintf (buf, "last click at %d,%d (id %d: %s)", MOUSE_X, MOUSE_Y, CLICKED_ID, FetchObjectName(CLICKED_ID));
		else
			sprintf (buf, "last click at %d,%d (---)", MOUSE_X, MOUSE_Y);

 		Make_debug_text_block (buf, 0, 15);

		if (mouse_touching)
			sprintf (buf, "mouse %d,%d (id %d: %s)", mousex+this_screen.scroll_offset_x, mousey+this_screen.scroll_offset_y, mouse_touching, FetchObjectName(mouse_touching));
		else
			sprintf (buf, "mouse %d,%d (not touching)", mousex+this_screen.scroll_offset_x, mousey+this_screen.scroll_offset_y);

		Make_debug_text_block (buf, 0, 30);

		//---------------------------------------------
 		// player coords & graphic info

		if (playerGraphic.anim_resource)	// if player objct has a graphic
			sprintf (buf, "player %d,%d %s (%d) #%d/%d", this_screen.player_feet_x, this_screen.player_feet_y, FetchObjectName(playerGraphic.anim_resource), playerGraphic.anim_resource, playerGraphic.anim_pc, player_graphic_no_frames);
		else
			sprintf (buf, "player %d,%d --- %d", this_screen.player_feet_x, this_screen.player_feet_y, playerGraphic.anim_pc);

		Make_debug_text_block (buf, 0, 45);

		//---------------------------------------------
 		// frames-per-second counter

		sprintf (buf, "fps %d", fps);
		Make_debug_text_block (buf, 440, 0);

		//---------------------------------------------
 		// location number

		sprintf (buf, "location=%d", LOCATION);
		Make_debug_text_block (buf, 440, 15);

		//---------------------------------------------
 		// "result" variable

		sprintf (buf, "result=%d", RESULT);
		Make_debug_text_block (buf, 440, 30);

		//---------------------------------------------
 		// no. of events in event list

		sprintf (buf, "events=%d", CountEvents());
		Make_debug_text_block (buf, 440, 45);

		//---------------------------------------------
		// sprite list usage

		sprintf (buf, "bgp0: %d/%d",cur_bgp0,MAX_bgp0_sprites);
		Make_debug_text_block (buf, 560, 0);

		sprintf (buf, "bgp1: %d/%d",cur_bgp1,MAX_bgp1_sprites);
		Make_debug_text_block (buf, 560, 15);

		sprintf (buf, "back: %d/%d",cur_back,MAX_back_sprites);
		Make_debug_text_block (buf, 560, 30);

		sprintf (buf, "sort: %d/%d",cur_sort,MAX_sort_sprites);
		Make_debug_text_block (buf, 560, 45);

		sprintf (buf, "fore: %d/%d",cur_fore,MAX_fore_sprites);
		Make_debug_text_block (buf, 560, 60);

		sprintf (buf, "fgp0: %d/%d",cur_fgp0,MAX_fgp0_sprites);
		Make_debug_text_block (buf, 560, 75);

		sprintf (buf, "fgp1: %d/%d",cur_fgp1,MAX_fgp1_sprites);
		Make_debug_text_block (buf, 560, 90);

		//---------------------------------------------
		// largest layer & sprite

		// NB. Strings already constructed in Build_display.cpp
		Make_debug_text_block (largest_layer_info, 0, 60);
		Make_debug_text_block (largest_sprite_info, 0, 75);

		//---------------------------------------------
		// "waiting for person" indicator - set form FN_they_do & FN_they_do_we_wait

		if (speechScriptWaiting)
		{
			sprintf (buf, "script waiting for %s (%d)", FetchObjectName(speechScriptWaiting), speechScriptWaiting);
			Make_debug_text_block (buf, 0, 90);
		}
 		//---------------------------------------------
		// variable watch display

		showVarPos = 115;	// y-coord for first showVar

		varTable = (int32*)(res_man.Res_open(1) + sizeof(_standardHeader));	// res 1 is the global variables resource

		for (showVarNo=0; showVarNo < MAX_SHOWVARS; showVarNo++)
		{
			varNo = showVar[showVarNo];	// get variable number

			if (varNo)	// if non-zero ie. cannot watch 'id' but not needed anyway because it changes throughout the logic loop
			{
				sprintf (buf, "var(%d) = %d", varNo, varTable[varNo]);
				Make_debug_text_block (buf, 530, showVarPos);
				showVarPos += 15;	// next line down
			}
		}

		res_man.Res_close(1);	// close global variables resource

		//---------------------------------------------
		// memory indicator - this should come last, to show all the sprite blocks above!

		Create_mem_string (buf);
		Make_debug_text_block (buf, 0, 0);

		//---------------------------------------------
	}
	
	//-------------------------------------------------------------------
	

}

//--------------------------------------------------------------------------------------
void Draw_debug_graphics( void )	// JAMES (08apr97)
{
 	//-------------------------------
	// walk-grid

	if (displayWalkGrid)
		PlotWalkGrid(); 

 	//-------------------------------
	// player feet coord marker

	if (displayPlayerMarker)
		Plot_cross_hair (this_screen.player_feet_x, this_screen.player_feet_y, 215);

	//-------------------------------------------------------------------
	// mouse marker & coords

	if (displayMouseMarker)
		Plot_cross_hair (mousex+this_screen.scroll_offset_x, mousey+this_screen.scroll_offset_y, 215);

	//-------------------------------------------------------------------
   	// mouse area rectangle / sprite box rectangle when testing anims

	if (SYSTEM_TESTING_ANIMS)
	{
		DrawRect(rect_x1, rect_y1, rect_x2, rect_y2, 184);	// draw box around current frame
	}
	else if (draggingRectangle)		// defining a mouse area the easy way, by creating a box on-screen
	{
		if (rectFlicker)
			DrawRect(rect_x1, rect_y1, rect_x2, rect_y2, 184);
	}
	//-------------------------------------------------------------------
}
//--------------------------------------------------------------------------------------
void Plot_cross_hair( int16 x, int16 y, uint8 pen )
{
	PlotPoint(x,y,pen);			// driver function

	DrawLine(x-2,y,x-5,y,pen);	// driver function
	DrawLine(x+2,y,x+5,y,pen);

	DrawLine(x,y-2,x,y-5,pen);
	DrawLine(x,y+2,x,y+5,pen);
}
//--------------------------------------------------------------------------------------
void DrawRect( int16 x1, int16 y1, int16 x2, int16 y2, uint8 pen )
{
	DrawLine(x1,y1,x2,y1,pen);	// top edge
	DrawLine(x1,y2,x2,y2,pen);	// bottom edge
	DrawLine(x1,y1,x1,y2,pen);	// left edge
	DrawLine(x2,y1,x2,y2,pen);	// right edge
}
//--------------------------------------------------------------------------------------
void Print_current_info(void)	//Tony30Oct96
{
//prints general stuff about the screen, etc.


	if	(this_screen.background_layer_id)
	{	Print_to_console(" background layer id %d", this_screen.background_layer_id);
		Print_to_console(" %d wide, %d high", this_screen.screen_wide, this_screen.screen_deep);
		Print_to_console(" %d normal layers", this_screen.number_of_layers);

		LLogic.Examine_run_list();

	}
	else
		Print_to_console(" no screen");


	Scroll_console();
}
//--------------------------------------------------------------------------------------
#else // not debug

void Draw_debug_graphics(void) {};

#endif	// _BS2_DEBUG
//--------------------------------------------------------------------------------------
