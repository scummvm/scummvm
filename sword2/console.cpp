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
#include "driver/driver96.h"

uint32	console_status=0;	//0 off		// LEFT IN RELEASE VERSION


#ifdef _DEBUG

//-----------------------------------------------------------------------------------------------------------------------
//its the console! <great>
//-----------------------------------------------------------------------------------------------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>		// for version string stuff

#include "build_display.h"
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "events.h"	// so we can disaply the event list in Con_display_events()
#include "header.h"
#include "layers.h"
#include "logic.h"
#include "maketext.h"	// for InitialiseFontResourceFlags()
#include "mouse.h"
#include "mem_view.h"
#include "memory.h"
#include "protocol.h"
#include "resman.h"
#include "save_rest.h"
#include "startup.h"
#include "sword2.h"
#include "time.h"

//-----------------------------------------------------------------------------------------------------------------------
// local function prototypes

uint32	Parse_user_input(void);		// Tony13Aug96
void	Clear_console_line(void);	// Tony13Aug96
void	Con_help(void);				// Tony13Aug96

void	Con_colour_block(int	x,	int width, int height, uint32 pen, uint32 paper, uint8 *sprite_data_ad);
void	Con_print(uint8	*ascii, uint32	pen, uint32 paper);
uint32	Tconsole(uint32	mode);		// Tony9Oct96

void	Con_list_savegames(void);	// James05feb97
void	Con_save_game(int total_commands, uint8 *slotString, uint8 *description);	// James05feb97
void	Con_restore_game(int total_commands, uint8 *slotString);					// James05feb97
uint8	Is_number(uint8 *ascii);	// James05feb97
void	Con_start_timer(int total_commands, uint8 *slotString);						// Paul12feb97
void	ShowVar(uint8 *varNoPtr);													// James19mar97
void	HideVar(uint8 *varNoPtr);													// James19mar97
void	Con_display_version(void);													// James27mar97

void	Var_check(uint8	*pointer);	//Tony8Jan97
void	Var_set(uint8	*pointer, uint8	*p2);	//Tony8Jan97

void Con_display_events();	// (James11july97)


//-----------------------------------------------------------------------------------------------------------------------
uint8 wantSfxDebug=0;	// sfx debug file enabled/disabled from console
//-----------------------------------------------------------------------------------------------------------------------


#define	MAX_CONSOLE_BUFFER	70
#define	MAX_CONSOLE_PARAMS	5

#define	CON_PEN	187

#define VK_TAB            0x09
#define VK_RETURN         0x0D

char	console_buffer[MAX_CONSOLE_BUFFER];

char	last_command[MAX_CONSOLE_BUFFER];	// James 03apr97
int		last_command_len=0;					// James 03apr97

uint8 grabbingSequences=0;

int	console_pos=0;	//cursor position within the typed line

int	console_mode=0;	//0 is the base command line
								//1 means only parse for yes or no commands
									//1 on




#define	TOTAL_CONSOLE_COMMANDS	47

uint8	commands[TOTAL_CONSOLE_COMMANDS][9]=	// note '9' is max command length including null-terminator
{
	"HELP",			// 0
	"MEM",			// 1
	"Q",			// 2
	"TONY",			// 3
	"YES",			// 4
	"NO",			// 5
	"RES",			// 6
	"STARTS",		// 7
	"START",		// 8
	"INFO",			// 9
	"WALKGRID",		// 10
	"MOUSE",		// 11
	"PLAYER",		// 12
	"RESLOOK",		// 13
	"CUR",			// 14
	"RUNLIST",		// 15
	"KILL",			// 16
	"NUKE",			// 17
	"S",			// 18
	"VAR",			// 19
	"RECT",			// 20
	"CLEAR",		// 21
	"DEBUGON",		// 22
	"DEBUGOFF",		// 23
	"SAVEREST",		// 24
	"SAVES",		// 25
	"SAVE",			// 26
	"RESTORE",		// 27
	"BLTFXON",		// 28
	"BLTFXOFF",		// 29
	"TIMEON",		// 30
	"TIMEOFF",		// 31
	"TEXT",			// 32
	"SHOWVAR",		// 33
	"HIDEVAR",		// 34
	"VERSION",		// 35
	"SOFT",			// 36
	"HARD",			// 37
	"ANIMTEST",		// 38
	"TEXTTEST",		// 39
	"LINETEST",		// 40
	"GRAB",			// 41
	"EVENTS",		// 42
	"SFX",			// 43
	"ENGLISH",		// 44
	"FINNISH",		// 45
	"POLISH"		// 46
};

mem	*console_sprite;
uint32	con_y;
uint32	con_depth;
uint32	con_width;
uint32	con_chr_height;

#define	CON_lines	20	//10 lines deep

//-----------------------------------------------------------------------------------------------------------------------
void	Init_console(void)	//Tony9Sept96
{
//grab the memory for the console sprite

	uint32	j;
	uint8	*ad;
	uint8   white[4] = {255,255,255,0};	// Chris 11Apr97


	con_chr_height=12;
	con_width=screenWide;	//max across

	SetPalette(CON_PEN, 1, white, RDPAL_INSTANT);	// Force a palatte for the console. Chris 11Apr97

	console_sprite = Twalloc(con_width*(CON_lines*con_chr_height), MEM_float, UID_con_sprite);

	con_depth= CON_lines*con_chr_height;
	con_y= 399-con_depth;



//clear the buffer for a nice fresh start
	ad=console_sprite->ad;
	for	(j=0;j<con_width*(CON_lines*con_chr_height);j++)
		*(ad+j)=0;


	if	(!console_sprite)
	{
		Zdebug("Init_console Talloc fail");
		ExitWithReport("Init_console Talloc fail [file=%s line=%u]",__FILE__,__LINE__);
	}

	Zdebug("console height %d, y %d", con_depth, con_y);

	//first time in message
	Con_display_version();
}
//-----------------------------------------------------------------------------------------------------------------------
void	StartConsole(void)	//Tony12Aug96
{
//start console up and restart new line
//can ne called for newline

	int	j;


	console_pos=0;	//start of new line

	for	(j=0;j<MAX_CONSOLE_BUFFER;j++)	//we need to clear the whole buffer - else the cursor overwrites the end 0
		console_buffer[j]=0;


	console_status=1;	//on
}
//-----------------------------------------------------------------------------------------------------------------------
void	EndConsole(void)	//Tony9Oct96
{

	console_status=0;	//off
}
//-----------------------------------------------------------------------------------------------------------------------
uint32	Tconsole(uint32	mode)	//Tony9Oct96
{
//call from anywhere
//returns a positive value of the token typed or 0 for windows quiting - the caller should drop back

	uint32	ret,breakOut=0;







	console_mode=mode;	//set command frame


	StartConsole();

	while (TRUE)
	{
		if (ServiceWindows() == RDERR_APPCLOSED)
		{
			break;
		}

		while (!gotTheFocus)
		{
			if (ServiceWindows() == RDERR_APPCLOSED)
			{
				breakOut = 1;
				break;
			}

		}
		if (breakOut)
		{
			break;
		}

//-----
		if	(ret = One_console())
		{	EndConsole();
			return(ret);
		}
//-----
		Build_display();	//create and flip the screen
	}



//a windows message is throwing us out of here

	EndConsole();	//switch off drawing
	return(0);
}
//-----------------------------------------------------------------------------------------------------------------------
void	Scroll_console(void)	//Tony13Aug96
{
//scroll the console sprite up 12 pixels

	uint32	*to_buffer;
	uint32	*from_buffer;
	int	x;


	x=((con_depth-con_chr_height)*640)/4;	//number of dwords

	to_buffer= (uint32	*) console_sprite->ad;	//base of console sprite
	from_buffer= to_buffer+((con_chr_height*640)/4);

	while(x--)
		*(to_buffer++)=*(from_buffer++);



	Clear_console_line();	//blank the on-coming bottom line
}
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
void	Clear_console_line(void)	//Tony13Aug96
{
//blank the bottom line

	uint32	*pbuffer;
	uint32	x;


	pbuffer= (uint32	*) console_sprite->ad;	//base of our off-screen back buffer
	pbuffer+= ((con_depth-con_chr_height)*con_width/4);	//index to console text position

	for	(x=0;x<con_chr_height*(con_width/4);x++)	//clear the bottom text line
		*(pbuffer+x)=0;
}
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
void	Print_to_console(char *format,...)	//Tony13Aug96
{
//print a NULL terminated string of ascii to the next console line
//we can assume that the user has just entered a command by pressing return - which means we're on a clean line
//so output the line and line feed

	va_list		arg_ptr;			// Variable argument pointer
	char	buf[150];

	va_start(arg_ptr,format);
	_vsnprintf( buf, 150, format, arg_ptr);

	Con_print( (uint8*)buf, 2, 0);
	Scroll_console();	//line feed

}
//-----------------------------------------------------------------------------------------------------------------------
void	Temp_print_to_console(char *format,...)	//Tony13Aug96
{
//print a NULL terminated string of ascii to the next console line
//we can assume that the user has just entered a command by pressing return - which means we're on a clean line
//so output the line and line feed

	va_list		arg_ptr;			// Variable argument pointer
	char	buf[150];

	va_start(arg_ptr,format);
	_vsnprintf( buf, 150, format, arg_ptr);

	Con_print( (uint8*)buf, 2, 0);

}
//-----------------------------------------------------------------------------------------------------------------------
uint32	One_console(void)	//Tony12Aug96
{
//its the console command line system
//do an update - check keys and flash cursor and so on
	char c;
	static	int	flash=0;	//controls the flashing cursor rate
	uint32	res;


	if (KeyWaiting())
	{
		ReadKey(&c);


		if (!c)	//escape sequences
		{
		}
		else if (c==VK_TAB)			// UP arrow
		{
			if (last_command_len)	// if anything stored in buffer
			{
				// retrieve 'last_command' buffer
				memset (console_buffer, 0, MAX_CONSOLE_BUFFER);		// first clear the entire current buffer
				memcpy (console_buffer, last_command, last_command_len);	// now copy in the last command
				console_pos = last_command_len;
			}
		}
		else if	(c==VK_RETURN)	//RETurn
		{
			console_buffer[console_pos]=' ';	//by putting a space in we'll always have a chr$ in the buffer
			Clear_console_line();
			Print_to_console(console_buffer);

//			parse the input I guess

			if	(console_pos)
			{
				// save to 'last_command' buffer, in case need to repeat same command (James03apr97)
				memcpy (last_command, console_buffer, console_pos);	// get a copy of the current command
				last_command_len = console_pos;						// get a copy of the length as well

				res = Parse_user_input();
				if	(res)
					return(res);
			}

			StartConsole();	//reset buffer
		}
		else	if	(c==8)	//delete
		{
			if	(console_pos)
			{
				console_buffer[console_pos]=0;	//delete cursor chr$
				console_pos--;
				console_buffer[console_pos]=0;
			}
		}
		else	if	((c<32)||(c>'z'))
			Zdebug("console ignoring key - %d", c);
		else
		{
			if	(console_pos<(MAX_CONSOLE_BUFFER-1))	//less one to leave room for the cursor
				console_buffer[console_pos++]=c;

			else	//end of line has been reached, so keep replacing last letter
				console_buffer[console_pos-1]=c;	//replace
		}
	}


	flash++;
	if	(flash<7)
		console_buffer[console_pos]='_';
	else
		console_buffer[console_pos]=' ';	//by putting a space in we'll always have a chr$ in the buffer

	if	(flash==14)
		flash=0;



//update the real screen - done every cycle to keep the cursor flashing
	Clear_console_line();
	Con_print( (uint8*)console_buffer, 2, 0);

	return(0);
}
//-----------------------------------------------------------------------------------------------------------------------
uint32	Parse_user_input(void)	//Tony13Aug96
{
//pressed return and now we need to analyse whats been written and call up the relevent commands

	uint8	input[MAX_CONSOLE_PARAMS][MAX_CONSOLE_BUFFER];
	int	i,j,total_commands=0;
	int	index=0;
	uint32 rv;	// for driver return value
	uint8		pal[4]={255,255,255,0};




//quick check for numbers here
	if	(!isalpha(console_buffer[0]))
	{
		Print_to_console("Eh?");	//print the standard error message and quit
		return(0);
	}



	j=0;	//current command
	do
	{
		i=0;
		do
			input[j][i++]=toupper(console_buffer[index++]);
		while( isgraph(console_buffer[index]) );

		input[j][i]=0;	//NULL terminate

		j++;
		total_commands++;

		if	(index==console_pos)
			break;

		do
			index++;
		while( console_buffer[index]==' ' );
	}
	while(j<MAX_CONSOLE_PARAMS);	//only parse first 5 params




//try to find the first word in the commands base

	for	(j=0;j<TOTAL_CONSOLE_COMMANDS;j++)
	{
		i=0;
		while((input[0][i]==commands[j][i])&&(input[0][i]))
			i++;

		if	((!input[0][i])&&(!commands[j][i]))	//got to the end of an entry - so must have matched the whole word
		{
			switch(console_mode)	//the console mode denotes the scope of the commands accepted 0 is the base mode
			{							//external console commands may only be requiring a yes/no input for example
										//a different scope would only accept yes and no and drop back out when found... see?

				case	0:	//base command line
					switch(j)
					{
						//---------------------------------
						case	0:	// HELP
							Con_help();
							return(0);
							break;
						//---------------------------------
						case	1:	// MEM
							Console_mem_display();
							return(0);
							break;
						//---------------------------------
						case	2:	// Q
							return(1);	//quit the console
							break;
						//---------------------------------
						case	3:	// TONY
							Print_to_console("What about him?");
							return(0);
							break;
						//---------------------------------
						case	6:	// RES
							res_man.Print_console_clusters();
							return(0);
							break;
						//---------------------------------
						case	7:	// STARTS
							Con_print_start_menu();
							return(0);
							break;
						//---------------------------------
						case	8:	// START
							Con_start(&input[1][0]);
							SetPalette(187, 1, pal, RDPAL_INSTANT);	//force the palette
							return(0);
							break;
						//---------------------------------
						case	9:	// INFO
							displayDebugText = 1-displayDebugText;
							if (displayDebugText)
								Print_to_console("info text on");
							else
								Print_to_console("info text off");
							return(0);
							break;
						//---------------------------------
						case	10:	// WALKGRID
							displayWalkGrid = 1-displayWalkGrid;
							if (displayWalkGrid)
								Print_to_console("walk-grid display on");
							else
								Print_to_console("walk-grid display off");
							return(0);
							break;
						//---------------------------------
						case	11:	// MOUSE
							displayMouseMarker = 1-displayMouseMarker;
							if (displayMouseMarker)
								Print_to_console("mouse marker on");
							else
								Print_to_console("mouse marker off");
							return(0);
							break;
						//---------------------------------
						case	12:	// PLAYER
							displayPlayerMarker = 1-displayPlayerMarker;
							if (displayPlayerMarker)
								Print_to_console("player feet marker on");
							else
								Print_to_console("player feet marker off");
							return(0);
							break;
						//---------------------------------
						case	13:	// RESLOOK
							res_man.Examine_res(&input[1][0]);
							return(0);
							break;
						//---------------------------------
						case	14:	// CUR
							Print_current_info();
							return(0);
							break;
						//---------------------------------
						case	15:	// RUNLIST
							LLogic.Examine_run_list();
							return(0);
							break;
						//---------------------------------
						case	16:	// KILL
							res_man.Kill_res(&input[1][0]);
							return(0);
							break;
						//---------------------------------
						case	17:	// NUKE
							Print_to_console("killing all resources except variable file & player object...");
							res_man.Kill_all_res(1);	// '1' means we want output to console
							return(0);
							break;
						//---------------------------------
						case	18:	// S (same as START)
							Con_start(&input[1][0]);
							SetPalette(187, 1, pal, RDPAL_INSTANT);	//force the palette
							return(0);
							break;
						//---------------------------------
						case	19:	// VAR
							if	(total_commands==2)
								Var_check(&input[1][0]);
							else	Var_set(&input[1][0], &input[2][0]);

							return(0);
							break;
						//---------------------------------
						case	20:	// RECT
							definingRectangles = 1-definingRectangles;	// switch on/off
							if (definingRectangles)
								Print_to_console("mouse rectangles enabled");
							else
								Print_to_console("mouse rectangles disabled");
							draggingRectangle=0;						// reset (see debug.cpp & mouse.cpp)
  							return(0);
							break;
						//---------------------------------
						case	21:	// CLEAR
							Print_to_console("killing all object resources except player...");
							res_man.Kill_all_objects(1);	// '1' means we want output to console
							return(0);
							break;
						//---------------------------------
						case	22:	// DEBUGON
							displayDebugText	= 1;
							displayWalkGrid		= 1;
							displayMouseMarker	= 1;
							displayPlayerMarker = 1;
							displayTextNumbers	= 1;

							Print_to_console("enabled all on-screen debug info");
							return(0);
							break;
						//---------------------------------
						case	23:	// DEBUGOFF
							displayDebugText	= 0;
							displayWalkGrid		= 0;
							displayMouseMarker	= 0;
							displayPlayerMarker = 0;
							displayTextNumbers	= 0;

							definingRectangles	= 0;
							draggingRectangle	= 0;

							Print_to_console("disabled all on-screen debug info");
							return(0);
							break;
						//---------------------------------
						case	24:	// SAVEREST
							testingSnR = 1-testingSnR;
							if (testingSnR)
								Print_to_console("Enabled S&R logic_script stability checking");
							else
								Print_to_console("Disabled S&R logic_script stability checking");
							return(0);
							break;
						//---------------------------------
						case	25:	// SAVES							(James05feb97)
							Print_to_console("Savegames:");
							Con_list_savegames();
							return(0);
							break;
						//---------------------------------
						case	26:	// SAVE <slotNo> <description>		(James05feb97)
							Con_save_game(total_commands, &input[1][0], &input[2][0]);
							return(0);
							break;
						//---------------------------------
						case	27:	// RESTORE <slotNo> <description>	(James05feb97)
							Con_restore_game(total_commands, &input[1][0]);
							return(1);	//quit the console
							break;
						//---------------------------------
						case	28:	// BLTFXON							(Paul12feb97)
							SetBltFx();
							Print_to_console("blit fx enabled");
							return(0);
							break;
						//---------------------------------
						case	29:	// BLTFXOFF							(Paul12feb97)
							ClearBltFx();
							Print_to_console("blit fx disabled");
							return(0);
							break;
						//---------------------------------
						case	30:	// TIMEON							(Paul12feb97)
							Con_start_timer(total_commands, &input[1][0]);
							Print_to_console("timer display on");
							return(0);
							break;
						//---------------------------------
						case	31:	// TIMEOFF							(Paul12feb97)
							displayTime = 0;
							Print_to_console("timer display off");
							return(0);
							break;
						//---------------------------------
						case	32:	// TEXT								(James25feb97)
							displayTextNumbers = 1-displayTextNumbers;
							if (displayTextNumbers)
								Print_to_console("text numbers on");
							else
								Print_to_console("text numbers off");
							return(0);
							break;
						//---------------------------------
						case	33:	// SHOWVAR <varNo>		(James19mar97)
							ShowVar(&input[1][0]);		// add variable to watch-list
							return(0);
							break;
						//---------------------------------
						case	34:	// HIDEVAR <varNo>		(James19mar97)
							HideVar(&input[1][0]);		// remove variable from watch-list
							return(0);
							break;
						//---------------------------------
						case	35:	// VERSION				(James21mar97)
							Con_display_version();
							return(0);
							break;
						//---------------------------------
						case	36:	// SOFT				(James07apr97)
							if (RenderSoft())
								Print_to_console("Software Rendering already enabled");
							else
							{
								Print_to_console("Software Rendering enabled");
								CloseBackgroundLayer();		// unlock from memory (because used in hardware rendering)
							}
							return(0);
							break;
						//---------------------------------
						case	37:	// HARD				(James07apr97)
							rv = RenderHard();

							if (rv==RDERR_NOHARDWARE)
								Print_to_console("Hardware Rendering not available");
							else if (rv==RDERR_ALREADYON)
								Print_to_console("Hardware Rendering already enabled");
							else
								Print_to_console("Hardware Rendering enabled");
							return(0);
							break;
						//---------------------------------
						case	38:	// ANIMTEST
							Con_start((uint8*)"32");	// automatically do "s 32" to run the text/speech testing start-script

							Print_to_console("Setting flag 'system_testing_anims'");
							Var_set((uint8*)"912", &input[1][0]);	// same as typing "VAR 912 <value>" at the console

							return(1);
							break;
						//---------------------------------
						case	39:	// TEXTTEST
							Con_start((uint8*)"33");	// automatically do "s 33" to run the text/speech testing start-script

							Print_to_console("Setting flag 'system_testing_text'");
							Var_set((uint8*)"1230", &input[1][0]);	// same as typing "VAR 1230 <value>" at the console

							displayTextNumbers=1;
							Print_to_console("text numbers on");

							return(1);
							break;
						//---------------------------------
						case	40:	// LINETEST
							Con_start((uint8*)"33");	// automatically do "s 33" to run the text/speech testing start-script

							Print_to_console("Setting var 1230 (system_testing_text):");
							Var_set((uint8*)"1230", &input[1][0]);	// same as typing "VAR 1230 <value>" at the console

							Print_to_console("Setting var 1264 (system_test_line_no):");
							Var_set((uint8*)"1264", &input[2][0]);	// same as typing "VAR 1264 <value>" at the console

							displayTextNumbers=1;
							Print_to_console("text numbers on");

							return(1);
							break;
						//---------------------------------
						case	41:	// GRAB								(James27jun97)
							grabbingSequences = 1-grabbingSequences;
							if (grabbingSequences)
								Print_to_console("PCX-grabbing enabled");
							else
								Print_to_console("PCX-grabbing disabled");
							return(0);
							break;
						//---------------------------------
						case	42:	// EVENTS								(James11july97)
							Con_display_events();
 							return(0);
							break;
						//---------------------------------
  						case	43:	// SFX								(James 16july97)
							wantSfxDebug = 1-wantSfxDebug;
							if (wantSfxDebug)
								Print_to_console("SFX logging activated (see zebug.txt)");
							else
								Print_to_console("SFX logging deactivated");
							return(0);
							break;
						//---------------------------------
  						case	44:	// ENGLISH	(James31july97)
							InitialiseFontResourceFlags(DEFAULT_TEXT);
							Print_to_console("Default fonts selected");
							return(0);
							break;
						//---------------------------------
  						case	45:	// FINNISH	(James31july97)
							InitialiseFontResourceFlags(FINNISH_TEXT);
							Print_to_console("Finnish fonts selected");
							return(0);
							break;
						//---------------------------------
  						case	46:	// POLISH	(James31july97)
							InitialiseFontResourceFlags(POLISH_TEXT);
							Print_to_console("Polish fonts selected");
							return(0);
							break;
						//---------------------------------
						default:	//ignores 'yes' and 'no' (for example)
							Print_to_console("??");
							return(0);
							break;
						//---------------------------------
					}
					break;


				case	1:	//checks for YES and NO and returns the 1 or 2 to the calling code
					switch(j)
					{
						case	4:	//YES
							return(1);

						case	5:	//NO
							return(2);

						default:	//ignore yes and no for example
							Print_to_console("??");
							return(0);

					}
					break;

			}
			break;
		}
	}
	Print_to_console("?");	//couldn't find a proper match
	return(0);

}
//-----------------------------------------------------------------------------------------------------------------------
void	Con_help(void)	//Tony13Aug96
{
// print out a list of commands

	int command;
	int	scrolls=0;
	char	c;





	Scroll_console();



	for (command=0; command < TOTAL_CONSOLE_COMMANDS; command++)
		{
			Print_to_console((char *)commands[command]);
			Build_display();
			scrolls++;


			if	(scrolls==18)
			{
				Temp_print_to_console("- Press ESC to stop or any other key to continue");
				Build_display();

				do
				{
			 		//--------------------------------------------------
					// Service windows
					while (!gotTheFocus)
						if (ServiceWindows() == RDERR_APPCLOSED)
							break;

				  	if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q
					{
						Close_game();	//close engine systems down
						RestoreDisplay();
						CloseAppWindow();
						exit(0);	//quit the game
					}
 					//--------------------------------------------------
				}
				while(!KeyWaiting());

				ReadKey(&c);	//kill the key we just pressed
				if	(c==27)	//ESC
					break;

				Clear_console_line();	//clear the Press Esc message ready for the new line
				scrolls=0;
			}	


		}


}
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
void	Con_print(uint8	*ascii, uint32	pen, uint32 paper)	//Em(13Apr95tw)
{
//print pixels in closen pen number - no clipping on this one
//where file is the graphic file of ascii characters

	_frameHeader *head;
	uint8 *charSet, *charPtr;
	int	chr, x=0;



	charSet = res_man.Res_open(CONSOLE_FONT_ID);			// open font file

	do
	{

//	Zdebug("#%d", *(ascii) );

		chr = (int) *(ascii);
		chr-=32;

		head = (_frameHeader *)FetchFrameHeader(charSet,chr);
		charPtr = (uint8 *)(head+1);

		Con_colour_block( x, head->width, head->height, pen, paper, charPtr);

		x+=head->width+1;						//move on the x coordinate
		ascii++;
	}
	while(*(ascii));


	res_man.Res_close(CONSOLE_FONT_ID);						// close font file

}
//-----------------------------------------------------------------------------------------------------------------------
void	Con_colour_block(int	x,	int width, int height, uint32 pen, uint32 paper, uint8 *sprite_data_ad)	//Em(26Apr96tw)
{
	int	deltaX,xx,yy;
	char	*ad;


	deltaX = con_width-width;

	ad = (char	*) console_sprite->ad;
	ad += (con_width*(con_depth-con_chr_height))+x;	//locate bottom character row

	for	(yy=0;yy<height;yy++)
	{
		for	(xx=0;xx<width;xx++)
		{	if	(pen = *(sprite_data_ad++))	//color
				*(ad++)= (uint8)CON_PEN;
			else	*(ad++)= (uint8) paper;
		}
		ad+=deltaX;
	}
}
//-----------------------------------------------------------------------------------------------------------------------






//-----------------------------------------------------------------------------------------------------------------------
void Con_fatal_error(char *format,...)	//Tony17Oct96
{
//use this to alert the user of a major problem from which we cannot allow the game to continue
//while in console mode the user will still be ble to use the console commands - which may be useful
//this message is also written to the Zdebug file in case the console itself blows up

	va_list		arg_ptr;	//variable argument pointer
	char	buf[150];
	uint8 white[4]	= {255,255,255,0};	// (James 05mar97)


	SetPalette(CON_PEN, 1, white, RDPAL_INSTANT);	// set text colour in case screen is faded down! (James 05mar97)

	va_start(arg_ptr,format);
	_vsnprintf( buf, 150, format, arg_ptr);

	this_screen.background_layer_id=0;	//in case error in display loop

	Zdebug("CON_FATAL_ERROR:");
	Zdebug(buf);	//write to file first in-case the screen is screwed up and we never see the console

	Print_to_console(buf);
	Print_to_console("fatal error, sword2 must terminate :-(  (%d)", ID);
	Tconsole(0);	//mode 0 so all commands are available but quit will terminate the game


	Close_game();	//should down game services - free's mallocs, etc.

	RestoreDisplay();	//reset the Windows stuff
	CloseAppWindow();	//

	exit(0);
}
//-----------------------------------------------------------------------------------------------------------------------
void	Var_check(uint8	*pointer)	//Tony8Jan97
{
	int	var;


	sscanf((char*)pointer,"%d",&var);

	Print_to_console("%d", *(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4*var) );

}
//-----------------------------------------------------------------------------------------------------------------------
void	Var_set(uint8	*pointer, uint8	*p2)	//Tony8Jan97
{
	int	var;
	int	val;



	sscanf((char*)pointer,"%d",&var);
	sscanf((char*)p2,"%d",&val);


	Print_to_console("was %d", *(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4*var) );

	*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* var)=val;

	Print_to_console("now %d", val);

}

//-----------------------------------------------------------------------------------------------------------------------
void	ShowVar(uint8 *varNoPtr)		// James19mar97
{
	int32 showVarNo=0;
	int32 varNo;
	

	sscanf((char*)varNoPtr,"%d",&varNo);	// 'varNo' is what we want to add

	// search for a spare slot in the watch-list, but also watch out for this variable already being in the list
	while ((showVarNo < MAX_SHOWVARS) && (showVar[showVarNo] != 0) && (showVar[showVarNo] != varNo))
		showVarNo++;

	if (showVarNo < MAX_SHOWVARS)	// if we've found a spare slot or the variable's already there
	{
		if (showVar[showVarNo]==0)	// empty slot
		{
			showVar[showVarNo] = varNo;	// add it to the list at this slot
			Print_to_console("var(%d) added to the watch-list", varNo);
		}
		else
			Print_to_console("var(%d) already in the watch-list!", varNo);
	}
	else
		Print_to_console("Sorry - no more allowed - hide one or extend the system watch-list");

}
//-----------------------------------------------------------------------------------------------------------------------
void	HideVar(uint8 *varNoPtr)		// James19mar97
{
	int32 showVarNo=0;
	int32 varNo;
	

	sscanf((char*)varNoPtr,"%d",&varNo);	// 'varNo' is what we want to remove

	while ((showVarNo < MAX_SHOWVARS) && (showVar[showVarNo] != varNo))	// search for 'varNo' in the watch-list
		showVarNo++;

	if (showVarNo < MAX_SHOWVARS)	// if we've found 'varNo' in the list
	{
		showVar[showVarNo] = 0;		// clear this slot
		Print_to_console("var(%d) removed from watch-list", varNo);
	}
	else
		Print_to_console("Sorry - can't find var(%d) in the list", varNo);

}
//-----------------------------------------------------------------------------------------------------------------------
void	Con_list_savegames(void)	// (James05feb97)	Tony1Apr97
{
	uint8	description[SAVE_DESCRIPTION_LEN];
	int	j, scrolls=0;
	char	c;

	for	(j=0;j<100;j++)
		if (GetSaveDescription(j, description) == SR_OK)	//if there is a save game print the name
		{	Print_to_console("%d: \"%s\"", j, description);

			scrolls++;
			Build_display();

			if	(scrolls==18)
			{
				Temp_print_to_console("- Press ESC to stop or any other key to continue");
				Build_display();

				do
				{
			 		//--------------------------------------------------
					// Service windows
					while (!gotTheFocus)
						if (ServiceWindows() == RDERR_APPCLOSED)
							break;

				  	if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q
					{
						Close_game();	//close engine systems down
						RestoreDisplay();
						CloseAppWindow();
						exit(0);	//quit the game
					}
 					//--------------------------------------------------
				}
				while(!KeyWaiting());

				ReadKey(&c);	//kill the key we just pressed
				if	(c==27)	//ESC
					break;

				Clear_console_line();	//clear the Press Esc message ready for the new line
				scrolls=0;
			}
		}

}
//-----------------------------------------------------------------------------------------------------------------------
#define SR_OK				0x00000000	// ok													No worries
#define SR_ERR_FILEOPEN		0x00000001	// can't open file										Could create file for saving, or couldn't find file for loading
#define SR_ERR_INCOMPATIBLE	0x00000002	// (RestoreGame only) incompatible savegame data		Savegame file is obsolete. (Won't happen after development stops)
#define	SR_ERR_READFAIL		0x00000003	// (RestoreGame only) failed on reading savegame file	Something screwed up during the fread()
#define SR_ERR_WRITEFAIL	0x00000004	// (SaveGame only) failed on writing savegame file		Something screwed up during the fwrite() - could be hard-drive full..?
//-----------------------------------------------------------------------------------------------------------------------
void	Con_save_game(int total_commands, uint8 *slotString, uint8 *description)	// James05feb97
{
	uint16	slotNo;
	uint32	rv;

	if ((mouse_status) || (mouse_mode_locked))	// if mouse if off, or system menu is locked off
	{
		Print_to_console("WARNING: Cannot save game while control menu unavailable!");
		return;
	}

	if (total_commands >= 3)	// SAVE <slot> <description>
	{
		if (Is_number(slotString))
		{
			slotNo = atoi((char*)slotString);

			rv = SaveGame(slotNo,description);

			if (rv == SR_OK)
				Print_to_console("Saved game \"%s\" to file \"savegame.%.3d\"", description, slotNo);

			else if (rv == SR_ERR_FILEOPEN)
				Print_to_console("ERROR: Cannot open file \"savegame.%.3d\"", slotNo);

			else	// SR_ERR_WRITEFAIL
				Print_to_console("ERROR: Write error on file \"savegame.%.3d\"", slotNo);
		}
	}
	else
		Print_to_console("Syntax Error: type SAVE (slot_number) (description)");
}
//-----------------------------------------------------------------------------------------------------------------------
void	Con_restore_game(int total_commands, uint8 *slotString)		// James05feb97
{
	uint16	slotNo;
	uint8	description[SAVE_DESCRIPTION_LEN];
	uint32	rv;


	if ((mouse_status) || (mouse_mode_locked))	// if mouse if off, or system menu is locked off
	{
		Print_to_console("WARNING: Cannot restore game while control menu unavailable!");
		return;
	}

	if (total_commands >= 2)	// RESTORE <slot>
	{
		if (Is_number(slotString))
		{
			slotNo = atoi((char*)slotString);
			rv = RestoreGame(slotNo);

			if (rv == SR_OK)
			{
				GetSaveDescription(slotNo, description);
				Print_to_console("Restored game \"%s\" from file \"savegame.%.3d\"", description, slotNo);
			}
			else if (rv == SR_ERR_FILEOPEN)
				Print_to_console("ERROR: Cannot open file \"savegame.%.3d\"", slotNo);

			else if (rv == SR_ERR_INCOMPATIBLE)
				Print_to_console("ERROR: \"savegame.%.3d\" is no longer compatible with current player/variable resources", slotNo);

			else	// SR_ERR_READFAIL
				Print_to_console("ERROR: Read error on file \"savegame.%.3d\"", slotNo);
		}
	}
	else
		Print_to_console("Syntax Error: type RESTORE (slot_number)");
}
//-----------------------------------------------------------------------------------------------------------------------
void	Con_start_timer(int total_commands, uint8 *slotString)		// Paul12feb97
{

	if (total_commands >= 2)	// RESTORE <slot>
	{
		if (Is_number(slotString))
		{
			startTime = timeGetTime() - (atoi((char*)slotString) * 1000);
		}
	}
	else
	{
		if (startTime = 0)
			startTime = timeGetTime();
	}
	displayTime = 1;

}
//-----------------------------------------------------------------------------------------------------------------------
uint8	Is_number(uint8 *ascii)			// James05feb97
{
	while (*ascii)	// until we reach the null terminator
	{
		if ((*ascii >= '0') && (*ascii <= '9'))
			ascii++;
		else
			return(0);
	}

	return(1);
}
//-----------------------------------------------------------------------------------------------------------------------
void Con_display_version(void)				// James27mar97
{
	struct tm *time;
	time_t t;
	char dateStamp[255];
	char version[6];

	strcpy(version,(char*)version_string+HEAD_LEN);
	*(((unsigned char *)&t))	= *(version_string+14);
	*(((unsigned char *)&t)+1)	= *(version_string+15);
	*(((unsigned char *)&t)+2)	= *(version_string+16);
	*(((unsigned char *)&t)+3)	= *(version_string+17);

	time = localtime( &t );
	sprintf(dateStamp,"%s", asctime( time ) );
	dateStamp[24]=0;	// fudge over the newline character!

	Print_to_console("\"Broken Sword II\" (c) Revolution Software 1997.");
	Print_to_console("v%s created on %s for %s", version, dateStamp, unencoded_name+HEAD_LEN);
	Scroll_console();

	// THE FOLLOWING LINES ARE TO BE COMMENTED OUT OF THE FINAL VERSION
//	Print_to_console("This program has a personalised fingerprint encrypted into the code.");
//	Print_to_console("If this CD was not sent directly to you by Virgin Interactive or Revolution Software");
//	Print_to_console("then please contact James Long at Revolution on (+44) 1904 639698.");
//	Scroll_console();
}
//-----------------------------------------------------------------------------------------------------------------------
//	typedef	struct
//	{
//		uint32	id;
//		uint32	interact_id;
//	}	_event_unit;
//-----------------------------------------------------------------------------------------------------------------------
void Con_display_events()	// (James11july97)
{
	uint32	j;
	uint32	target;
	uint32	script;


	Print_to_console("EVENT LIST:");

	for	(j=0; j<MAX_events; j++)
	{
		if (event_list[j].id)
		{
			target = event_list[j].id;
			script = event_list[j].interact_id;

			Print_to_console("slot %d: id = %s (%d)", j, FetchObjectName(target), target);
			Print_to_console("         script = %s (%d) pos %d", FetchObjectName(script/65536), script/65536, script%65536);
		}
	}
}
//------------------------------------------------------------------------------------

#else // not debug

void	Print_to_console(char *format,...) {};
void	Temp_print_to_console(char *format,...) {};
void	Clear_console_line(void) {};
void	Scroll_console(void) {};
void	Init_console(void) {};
void	StartConsole(void) {};

#endif	// _DEBUG
