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

//-----------------------------------------------------------------------------------------------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//#include	"src\driver96.h"

#include "stdafx.h"
#include "build_display.h"
#include "console.h"
#include "controls.h"
#include "debug.h"
#include "defs.h"
#include "header.h"
#include "interpreter.h"
#include "layers.h"
#include "logic.h"
#include "maketext.h"	// for font resource variables
#include "mem_view.h"
#include "memory.h"
#include "mouse.h"
#include "protocol.h"
#include "resman.h"
#include "router.h"
#include "save_rest.h"
#include "sound.h"		// for FN_stop_music()
#include "startup.h"
#include "sword2.h"

//-----------------------------------------------------------------------------------------------------------------------
#define	WINDOW_RES				2016
#define	MAX_STRING_LEN			64	// 20 was too low; better to be safe ;)

#define CHARACTER_OVERLAP 2		// overlap characters by 3 pixels
//-----------------------------------------------------------------------------------------------------------------------

void	Create_surface_image(_spriteInfo *sprite, uint8 **surface, uint32 res, uint32 x, uint32 y, uint32 pc);
void	Build_surfaces(void);
void	Build_chr_surfaces(void);
void	Kill_chr_surfaces(void);
void	Kill_surfaces(void);
void	Renew_surfaces(void);
void	Engine_string(uint32 x, uint32 y, uint32 res, uint8 **surface_list, uint8 *buf);
void	Kill_mini_surfaces(void);
void	Build_mini_surfaces(void);
uint32	Generic_mini_control(uint32 text_id);
uint32	Pixel_text_length(uint8	*buf, uint32 res);
void	Control_error(char* text);

//-----------------------------------------------------------------------------------------------------------------------

#define	WINDOW_X	0

#define	REST_X	84
#define	REST_Y	40

#define	SLAB_X	(REST_X+30)
#define	SLAB_Y	(REST_Y+32)

#define	REST_BUT_X	130
#define	REST_BUT_Y	377

#define	CAN_BUT_X	350
#define	CAN_BUT_Y	377

#define	UP_BUT_X	516
#define	UP_BUT_Y	85

#define	DOWN_BUT_X	516
#define	DOWN_BUT_Y	329

#define	ZUP_BUT_Y	85-20

#define	ZDOWN_BUT_Y	329+21

#define	SLAB_Y_SPACING	35

#define	QUIT_X	203
#define	QUIT_Y	104

#define OPTION_X 45
#define OPTION_Y 40
#define OPTION_W 552

#define OPT_BUT_W 53
#define OPT_BUT_H 32

#define OPT_OK_X (OPTION_X+(OPTION_W/3)-(OPT_BUT_W/2))
#define OPT_OK_Y (OPTION_Y+368-(OPT_BUT_W/2))

#define OPT_CAN_X (OPTION_X+350)
#define OPT_CAN_Y (OPT_OK_Y)

#define SLIDER_TRK_X (OPTION_X+264)
#define SLIDER_TRK_W 132
#define SLIDER_TRK_H 27
#define SLIDER_W	 38

#define	OBJ_LABEL_X (SLIDER_TRK_X-5)
#define	OBJ_LABEL_Y (OPTION_Y+60)

#define	SUBTITLE_X (OPTION_X+465)
#define	SUBTITLE_Y (OBJ_LABEL_Y)

#define	STEREO_X (SLIDER_TRK_X-5)
#define	STEREO_Y (OPTION_Y+253)

#define MUSIC_TRK_Y	(OPTION_Y+121)
#define SPEECH_TRK_Y (OPTION_Y+168)
#define FX_TRK_Y (OPTION_Y+214)
#define GRFX_TRK_Y (OPTION_Y+301)

#define MUTE_W 40
#define MUTE_H 32
#define MUTE_X (SUBTITLE_X+OPT_BUT_W/2-MUTE_W/2)

#define GRFX_ICON_X (OPTION_X+450)
#define GRFX_ICON_Y (OPTION_Y+270)

//--------------------------------------------

uint8	*panel_surface;
_spriteInfo panel_sprite;

_spriteInfo slab_sprite[8];
uint8	*slab_surface[8];

_spriteInfo	chr_sprite;

#define SIZE_OF_CHAR_SET (256-32)	// our fonts start on SPACE character (32)
uint8	*chr_surface[SIZE_OF_CHAR_SET];
uint8	*red_chr_surface[SIZE_OF_CHAR_SET];

_spriteInfo can_button_sprite[2];
uint8	*can_button_surface[2];
uint32	can_button_state=0;
uint32	touching_can_button=0;

_spriteInfo button_sprite[2];
uint8	*button_surface[2];
uint32	restore_button_state=0;
uint32	touching_restore_button=0;

_spriteInfo up_button_sprite[2];
uint8	*up_button_surface[2];
uint32	up_button_state=0;
uint32	touching_up_button=0;

_spriteInfo down_button_sprite[2];
uint8	*down_button_surface[2];
uint32	down_button_state=0;
uint32	touching_down_button=0;

_spriteInfo zup_button_sprite[2];
uint8	*zup_button_surface[2];
uint32	zup_button_state=0;
uint32	touching_zup_button=0;

_spriteInfo zdown_button_sprite[2];
uint8	*zdown_button_surface[2];
uint32	zdown_button_state=0;
uint32	touching_zdown_button=0;

_spriteInfo grfx_icon_sprite[4];
uint8 *grfx_icon_surface[4];

uint8	*charSet;
uint8	*red_charSet;

_frameHeader	*head;

uint32	base_slot=0;

uint8 subtitles;		// text selected
uint8 speechSelected;
uint8 stereoReversed = 0;

uint8 current_graphics_level;

//-----------------------------------------------------------------------------------------------------------------------
uint32	Restore_control(void)	//Tony20Mar97
{
//well, this is nice and hard wired - not the way to do it really

//	returns 0 for no restore
//			  1 for restored ok


	int	breakOut=0;
	uint32	j;
	char	key_press=0;
	uint8	description[SAVE_DESCRIPTION_LEN];
	uint8	buf[8][SAVE_DESCRIPTION_LEN];

	uint8	chr;
	int		char_no;
	_mouseEvent	*me;

	uint8	restore_text[MAX_STRING_LEN];
	uint8	cancel_text[MAX_STRING_LEN];
	uint8	*text;


	uint32	slab_text_x;
	uint32	slab_text_y;

	uint32	slot=1000;	//nothing selected
	uint32	clicked_slot;
	uint32	cur_slot_states[9];

	int	scroll_rate=0;
	//static	uint32	base_slot=0;

	int	first=0;

	uint32	rv;	// return value for RestoreGame
	uint32	res;	//result from primer game cycle


	int	names_built=0;	//0 redo, else dont



//do some driver stuff
//	for	(j=0;j<1000;j++)
//		ResetRenderEngine();




//buttons unpressed
	restore_button_state=0;
	can_button_state=0;




	Build_surfaces();
	Build_chr_surfaces();

//fetch the 'restore' text
	text = FetchTextLine( res_man.Res_open(149618690/SIZE), 149618690&0xffff );	// open text file & get the line
	strcpy((char*)&restore_text[0], (char*)text+2);
//fetch the 'cancel' text
	text = FetchTextLine( res_man.Res_open(149618689/SIZE), 149618689&0xffff );	// open text file & get the line
	strcpy((char*)&cancel_text[0], (char*)text+2);
//blimey, life's never easy is it?


//control loop
	while (1)
	{
 		//--------------------------------------------------
		// Service windows
	  	if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q during the smacker
		{
			Close_game();	//close engine systems down
			CloseAppWindow();
			exit(0);	//quit the game
		}

		while (!gotTheFocus)
		{	names_built=0;
			slot=1000;
			if (ServiceWindows() == RDERR_APPCLOSED)
				break;
		}
 		//--------------------------------------------------

		EraseBackBuffer();


//print panel
		if	(DrawSurface(&panel_sprite, panel_surface)==RDERR_SURFACELOST)
			Renew_surfaces();


//print words on panel
		Engine_string(REST_BUT_X+32, REST_BUT_Y, controls_font_id, chr_surface, restore_text);
		Engine_string(CAN_BUT_X+32, REST_BUT_Y, controls_font_id, chr_surface, cancel_text);




		slab_text_y=76;

//	print slabs
		for	(j=0;j<8;j++)
		{

			if	(slot==base_slot+j)	//red
			{	slab_sprite[((base_slot+j)&3)+4].y=SLAB_Y+(j*SLAB_Y_SPACING);
				DrawSurface(&slab_sprite[((base_slot+j)&3)+4], slab_surface[((base_slot+j)&3)+4] );
			}
			else
			{	slab_sprite[((base_slot+j)&3)].y=SLAB_Y+(j*SLAB_Y_SPACING);
				DrawSurface(&slab_sprite[((base_slot+j)&3)], slab_surface[((base_slot+j)&3)] );
			}

//			print save name on slab if a game is saved in this slot
			if	(!names_built)
			{	if (GetSaveDescription(base_slot+j, description) == SR_OK)	//if there is a savegame at this slot
				{
					cur_slot_states[j]=1;	//slot used

					if	(!description[0])
						Con_fatal_error("NULL file name passed from GetSaveDescription!");	

	//				print the name on the slab
					sprintf((char*)buf[j], "%d.  %s", base_slot+j, description );
				}
				else
				{
					sprintf((char*)buf[j], "%d.", base_slot+j);	//simply print the number
					cur_slot_states[j]=0;	//slot not used
				}
			}


			char_no=0;

			slab_text_x=SLAB_X+16;


			do
			{
				chr = buf[j][char_no];
				chr-=32;	//got true chr$

				chr_sprite.x=slab_text_x;


				chr_sprite.scale=0;
				chr_sprite.type= RDSPR_NOCOMPRESSION+RDSPR_TRANS;
				chr_sprite.blend= 0;

				if	(slot==base_slot+j)
				{
					head = (_frameHeader *)FetchFrameHeader(red_charSet, chr);

					chr_sprite.w=head->width;
					chr_sprite.h=head->height;

					chr_sprite.y=slab_text_y+2;
					DrawSurface(&chr_sprite, red_chr_surface[chr]);	//print
				}
				else
				{
					head = (_frameHeader *)FetchFrameHeader(charSet, chr);

					chr_sprite.w=head->width;
					chr_sprite.h=head->height;

					chr_sprite.y=slab_text_y;
					DrawSurface(&chr_sprite, chr_surface[chr]);	//print
				}

				slab_text_x+=head->width-CHARACTER_OVERLAP;
				char_no++;
			}
			while(buf[j][char_no]);


			slab_text_y+=SLAB_Y_SPACING;
		}



		names_built=1;	//dont GetSaveDescription each cycle

//print buttons
//print restore button
		DrawSurface(&button_sprite[restore_button_state], button_surface[restore_button_state] );

//print cancel button
		DrawSurface(&can_button_sprite[can_button_state], can_button_surface[can_button_state] );

//print up button
		DrawSurface(&up_button_sprite[up_button_state], up_button_surface[up_button_state] );

//print down button
		DrawSurface(&down_button_sprite[down_button_state], down_button_surface[down_button_state] );

//print zup button
		DrawSurface(&zup_button_sprite[zup_button_state], zup_button_surface[zup_button_state] );

//print zdown button
		DrawSurface(&zdown_button_sprite[zdown_button_state], zdown_button_surface[zdown_button_state] );


		ProcessMenu();

		if	(!first)
		{
			first++;
			SetFullPalette(CONTROL_PANEL_PALETTE);	// see Build_display.cpp (James17jun97)
		}


//mouse over buttons?
//restore
		if	((mousex>REST_BUT_X)&&(mousex<REST_BUT_X+24)&&((mousey+40)>REST_BUT_Y)&&((mousey+40)<REST_BUT_Y+24))
			touching_restore_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	restore_button_state=0;
			touching_restore_button=0;
		}

//cancel
		if	((mousex>CAN_BUT_X)&&(mousex<CAN_BUT_X+24)&&((mousey+40)>CAN_BUT_Y)&&((mousey+40)<CAN_BUT_Y+24))
			touching_can_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	can_button_state=0;
			touching_can_button=0;
		}

//up
		if	((mousex>UP_BUT_X)&&(mousex<UP_BUT_X+17)&&((mousey+40)>UP_BUT_Y)&&((mousey+40)<UP_BUT_Y+17))
			touching_up_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	up_button_state=0;
			touching_up_button=0;
		}

//down
		if	((mousex>DOWN_BUT_X)&&(mousex<DOWN_BUT_X+17)&&((mousey+40)>DOWN_BUT_Y)&&((mousey+40)<DOWN_BUT_Y+17))
			touching_down_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	down_button_state=0;
			touching_down_button=0;
		}

//up
		if	((mousex>UP_BUT_X)&&(mousex<UP_BUT_X+17)&&((mousey+40)>ZUP_BUT_Y)&&((mousey+40)<ZUP_BUT_Y+17))
			touching_zup_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	zup_button_state=0;
			touching_zup_button=0;
		}

//down
		if	((mousex>DOWN_BUT_X)&&(mousex<DOWN_BUT_X+17)&&((mousey+40)>ZDOWN_BUT_Y)&&((mousey+40)<ZDOWN_BUT_Y+17))
			touching_zdown_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	zdown_button_state=0;
			touching_zdown_button=0;
		}







//check mouse clicked on a slab
		me = MouseEvent();	//get mouse event

		if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN))	//there's a mouse event to be processed
		{
			if	((mousex>SLAB_X)&&(mousex<SLAB_X+384)&&((mousey+40)>SLAB_Y)&&((mousey+40)<SLAB_Y+SLAB_Y_SPACING*8))
			{
				clicked_slot=((mousey+40)-SLAB_Y)/SLAB_Y_SPACING;

				//Zdebug("clicked slab %d", slot);

				if	(cur_slot_states[clicked_slot])	//a selectable slot
				{
					if	(slot!= clicked_slot+base_slot)	//can select if not selected now
						slot=clicked_slot+base_slot;	//now selected

					else	slot=1000;	//else deselect
				}
			}

//			clicking on the restore button
			if	(touching_restore_button)
				restore_button_state=1;	//button now down

//			clicking on the cancel button
			if	(touching_can_button)
				can_button_state=1;	//button now down

//			clicking on the up button
			if	(touching_up_button)
			{	up_button_state=1;	//button now down
				scroll_rate=0;
			}

//			clicking on the down button
			if	(touching_down_button)
			{	down_button_state=1;	//button now down
				scroll_rate=0;
			}
//			clicking on the zup button
			if	(touching_zup_button)
			{	zup_button_state=1;	//button now down
				scroll_rate=0;
			}

//			clicking on the zdown button
			if	(touching_zdown_button)
			{	zdown_button_state=1;	//button now down
				scroll_rate=0;
			}


		}

//check for releasing the mouse button over a button
		if	((key_press==13)||((me!=NULL)&&(me->buttons&RD_LEFTBUTTONUP)))	//there's a mouse event to be processed
		{
			if	((key_press==13)||((touching_restore_button)&&(restore_button_state)))
			{
				restore_button_state=0;

				if	(slot!=1000)	//restore the game!
				{
					breakOut=1;

					rv = RestoreGame(slot);

					if (rv == SR_OK)
					{
//						DEAD=0;	//in case we were dead - well we're not anymore!

//						prime system with a game cycle
						Reset_render_lists();	// reset the graphic 'buildit' list before a new logic list (see FN_register_frame)
						Reset_mouse_list();		// reset the mouse hot-spot list (see FN_register_mouse & FN_register_frame)
	
						res = LLogic.Process_session();

						if	(res)
							Con_fatal_error("restart 1st cycle failed??");


					//	Control_error("restored OK :)");

						Kill_surfaces();
						Kill_chr_surfaces();

						return(1);
					}
					else
					{
// Save & Restore error codes

// ERROR CODE			VALUE		MEANING													REASON
// ==========			=====		=======													======
// SR_OK				0x00000000	// ok													No worries
// SR_ERR_FILEOPEN		0x00000001	// can't open file										Could create file for saving, or couldn't find file for loading
// SR_ERR_INCOMPATIBLE	0x00000002	// (RestoreGame only) incompatible savegame data		Savegame file is obsolete. (Won't happen after development stops)
// SR_ERR_READFAIL		0x00000003	// (RestoreGame only) failed on reading savegame file	Something screwed up during the fread()
// SR_ERR_WRITEFAIL		0x00000004	// (SaveGame only) failed on writing savegame file		Something screwed up during the fwrite() - could be hard-drive full..?

 
						// WE NEED A MESSAGE BOX TO INDICATE FAILED SAVE - DON'T HALT THE GAME!

						if (rv == SR_ERR_FILEOPEN)
							Control_error((char*)(FetchTextLine( res_man.Res_open(213516670/SIZE), 213516670&0xffff)+2));
//							Restore failed - could not open file

						else if (rv == SR_ERR_INCOMPATIBLE)
							Control_error((char*)(FetchTextLine( res_man.Res_open(213516671/SIZE), 213516671&0xffff)+2));
//							Restore failed - incompatible savegame data

						else	// SR_ERR_READFAIL
							Control_error((char*)(FetchTextLine( res_man.Res_open(213516673/SIZE), 213516673&0xffff)+2));
//							Restore failed
					}
				}
			}
			else	if	((touching_can_button)&&(can_button_state))	//quit the screen
				breakOut=1;

			else	if	(touching_up_button)
				up_button_state=0;

			else	if	(touching_down_button)
				down_button_state=0;

			else	if	(touching_zup_button)
				zup_button_state=0;

			else	if	(touching_zdown_button)
				zdown_button_state=0;

		}



//scrolling downward
		if	( ((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(down_button_state)&&(base_slot<92))
		{	base_slot++;
			names_built=0;
		}

//scrolling upward
		if	(((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(up_button_state)&&(base_slot))
		{	base_slot--;
			names_built=0;
		}

//scrolling zdownward
		if	(((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(zdown_button_state)&&(base_slot<92))
		{	base_slot+=8;
			if	(base_slot>92)
				base_slot=92;

			names_built=0;
		}

//scrolling zupward
		if	(((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(zup_button_state)&&(base_slot))
		{	base_slot-=8;
			if	(base_slot>92)
				base_slot=0;

			names_built=0;
		}


//update scroll stuff
		scroll_rate++;




//-----
		key_press=0;
		if (KeyWaiting())
		{
			ReadKey(&key_press);	//kill the key we just pressed
			if	(key_press==27)	//ESC
				break;
		}


		if (breakOut)
		{
			break;	//quit this stuff - ap will eventually close in the mainloop
		}


	}	//while


	Kill_surfaces();
	Kill_chr_surfaces();

	return(0);
}
//-----------------------------------------------------------------------------------------------------------------------
void	Create_surface_image(_spriteInfo *sprite, uint8	**surface, uint32	res,	uint32	x,	uint32	y, uint32	pc)	//TonyMarch97
{
	uint8			*file, *colTablePtr=NULL;
	_animHeader		*anim_head;
	_frameHeader	*frame_head;
	_cdtEntry		*cdt_entry;
	//_spriteInfo		spriteInfo;
	uint32			spriteType=RDSPR_TRANS;


	file = res_man.Res_open(res);	// open anim resource file & point to base


	anim_head	= FetchAnimHeader( file );
	cdt_entry	= FetchCdtEntry( file, pc );
	frame_head	= FetchFrameHeader( file, pc );




	if (anim_head->blend)
		spriteType += RDSPR_BLEND;

	if ((cdt_entry->frameType) & FRAME_FLIPPED)	//if the frame is to be flipped (only really applicable to frames using offsets)
		spriteType += RDSPR_FLIP;

	switch (anim_head->runTimeComp)	// what compression was used?
	{
		case NONE:
			spriteType += RDSPR_NOCOMPRESSION;
			break;
		case RLE256:
			spriteType += RDSPR_RLE256;
			break;
		case RLE16:
			spriteType += RDSPR_RLE16;
			colTablePtr	= (uint8*)(anim_head+1) + anim_head->noAnimFrames*sizeof(_cdtEntry);
			// points to just after last cdt_entry, ie. start of colour table
			break;
	}

	sprite->x			= x;
	sprite->y			= y;
	sprite->w			= frame_head->width;
	sprite->h			= frame_head->height;
	sprite->scale		= 0;
//	spriteInfo.scaledWidth	= build_unit->scaled_width;
//	spriteInfo.scaledHeight	= build_unit->scaled_height;
	sprite->type			= spriteType;
	sprite->blend		= anim_head->blend;
	sprite->data			= (uint8*)(frame_head+1);	// points to just after frame header, ie. start of sprite data
//	spriteInfo.colourTable	= colTablePtr;


//	Zdebug("w %d  h %d", frame_head->width, frame_head->height);

	CreateSurface(sprite, surface);


	res_man.Res_close(res);	//release the anim resource

}
//-----------------------------------------------------------------------------------------------------------------------
void	Build_surfaces(void)	//Tony27March97
{


//setup the control window
	Create_surface_image(&panel_sprite, &panel_surface, WINDOW_RES,	WINDOW_X,	REST_Y, 0);



//setup slabs as surfaces
	Create_surface_image(&slab_sprite[0], &slab_surface[0], 2006,	SLAB_X,	0, 0);
	Create_surface_image(&slab_sprite[1], &slab_surface[1], 2007,	SLAB_X,	0, 0);
	Create_surface_image(&slab_sprite[2], &slab_surface[2], 2008,	SLAB_X,	0, 0);
	Create_surface_image(&slab_sprite[3], &slab_surface[3], 2009,	SLAB_X,	0, 0);

//now the red selected panels
	Create_surface_image(&slab_sprite[4], &slab_surface[4], 2006,	SLAB_X,	0, 1);
	Create_surface_image(&slab_sprite[5], &slab_surface[5], 2007,	SLAB_X,	0, 1);
	Create_surface_image(&slab_sprite[6], &slab_surface[6], 2008,	SLAB_X,	0, 1);
	Create_surface_image(&slab_sprite[7], &slab_surface[7], 2009,	SLAB_X,	0, 1);

//restore button
	Create_surface_image(&button_sprite[0], &button_surface[0], 2002,	REST_BUT_X, REST_BUT_Y, 0);
	Create_surface_image(&button_sprite[1], &button_surface[1], 2002,	REST_BUT_X, REST_BUT_Y, 1);

//cancel button
	Create_surface_image(&can_button_sprite[0], &can_button_surface[0], 2002,	CAN_BUT_X, CAN_BUT_Y, 0);
	Create_surface_image(&can_button_sprite[1], &can_button_surface[1], 2002,	CAN_BUT_X, CAN_BUT_Y, 1);

//up button
	Create_surface_image(&up_button_sprite[0], &up_button_surface[0], 2067,	UP_BUT_X, UP_BUT_Y, 0);
	Create_surface_image(&up_button_sprite[1], &up_button_surface[1], 2067,	UP_BUT_X, UP_BUT_Y, 1);

//down button
	Create_surface_image(&down_button_sprite[0], &down_button_surface[0], 1986,	DOWN_BUT_X, DOWN_BUT_Y, 0);
	Create_surface_image(&down_button_sprite[1], &down_button_surface[1], 1986,	DOWN_BUT_X, DOWN_BUT_Y, 1);

//zup button
	Create_surface_image(&zup_button_sprite[0], &zup_button_surface[0], 1982,	UP_BUT_X, ZUP_BUT_Y, 0);
	Create_surface_image(&zup_button_sprite[1], &zup_button_surface[1], 1982,	UP_BUT_X, ZUP_BUT_Y, 1);

//zdown button
	Create_surface_image(&zdown_button_sprite[0], &zdown_button_surface[0], 1988,	DOWN_BUT_X, ZDOWN_BUT_Y, 0);
	Create_surface_image(&zdown_button_sprite[1], &zdown_button_surface[1], 1988,	DOWN_BUT_X, ZDOWN_BUT_Y, 1);

}
//-----------------------------------------------------------------------------------------------------------------------
void	Build_chr_surfaces(void)	//tony2Apr97
{

	int	j;


//sort out the font
	charSet = res_man.Res_open(controls_font_id);	//open font file
	red_charSet = res_man.Res_open(red_font_id);	//open font file


//set up the chr$ set frame surfaces
	chr_sprite.scale=0;
	chr_sprite.type= RDSPR_NOCOMPRESSION+RDSPR_TRANS;
	chr_sprite.blend= 0;

	for	(j=0; j < SIZE_OF_CHAR_SET; j++)
	{	//normal
		head = (_frameHeader *)FetchFrameHeader(charSet,j);
		chr_sprite.data = (uint8 *)(head+1);
		chr_sprite.w=head->width;
		chr_sprite.h=head->height;
		CreateSurface(&chr_sprite, &chr_surface[j]);

//		red
		head = (_frameHeader *)FetchFrameHeader(red_charSet,j);
		chr_sprite.data = (uint8 *)(head+1);
		chr_sprite.w=head->width;
		chr_sprite.h=head->height;
		CreateSurface(&chr_sprite, &red_chr_surface[j]);

	}

	res_man.Res_close(controls_font_id);	//close font file
	res_man.Res_close(red_font_id);	//close font file

}
//-----------------------------------------------------------------------------------------------------------------------
void	Kill_surfaces(void)	//Tony27March97
{

//remove the surfaces
	DeleteSurface(panel_surface);
	DeleteSurface(slab_surface[0]);
	DeleteSurface(slab_surface[1]);
	DeleteSurface(slab_surface[2]);
	DeleteSurface(slab_surface[3]);
	DeleteSurface(slab_surface[4]);
	DeleteSurface(slab_surface[5]);
	DeleteSurface(slab_surface[6]);
	DeleteSurface(slab_surface[7]);

	DeleteSurface(button_surface[0]);
	DeleteSurface(button_surface[1]);

	DeleteSurface(can_button_surface[0]);
	DeleteSurface(can_button_surface[1]);

	DeleteSurface(up_button_surface[0]);
	DeleteSurface(up_button_surface[1]);

	DeleteSurface(down_button_surface[0]);
	DeleteSurface(down_button_surface[1]);

	DeleteSurface(zup_button_surface[0]);
	DeleteSurface(zup_button_surface[1]);

	DeleteSurface(zdown_button_surface[0]);
	DeleteSurface(zdown_button_surface[1]);
}
//-----------------------------------------------------------------------------------------------------------------------
void	Kill_chr_surfaces(void)	//Tony2Apr97
{
	int	j;



//release chr$ set surfaces
	for	(j=0; j < SIZE_OF_CHAR_SET; j++)
	{	//normal
		DeleteSurface(chr_surface[j]);
//		red
		DeleteSurface(red_chr_surface[j]);
	}
}
//-----------------------------------------------------------------------------------------------------------------------
void	Renew_surfaces(void)	//Tony27March97
{
	Kill_surfaces();
	Kill_chr_surfaces();
	Build_surfaces();
	Build_chr_surfaces();
}
//-----------------------------------------------------------------------------------------------------------------------






//-----------------------------------------------------------------------------------------------------------------------
void	Save_control(void)	//Tony1Apr97	not a joke
{
//largely the same as the restore code


	int	breakOut=0;
	uint32	j;
	char	key_press;
	uint8	description[SAVE_DESCRIPTION_LEN];
	uint8	buf[8][SAVE_DESCRIPTION_LEN];
	uint8	ed_buf[SAVE_DESCRIPTION_LEN];
	int		char_no;
	uint8	chr;
	_mouseEvent	*me;
	int	esc_release=0;

	uint32	slab_text_x;
	uint32	slab_text_y;

	uint32	clicked_slot=1000, edit_screen_slot=1000;
	uint32	cur_slot_states[9];

	int	scroll_rate=0;
//	static	uint32	base_slot=0;

	int	edit_pos=0, first_chr=0, flash=0;

	uint8	save_text[MAX_STRING_LEN];
	uint8	cancel_text[MAX_STRING_LEN];
	uint8	*text;

	int	first=0;

	uint32	rv;	// return value for SaveGame

	uint32	edit_width=0;

	int	names_built=0;



//buttons unpressed
	restore_button_state=0;
	can_button_state=0;


//do some driver stuff
//	ResetRenderEngine();



//sort out the font
	charSet = res_man.Res_open(controls_font_id);	//open font file
	red_charSet = res_man.Res_open(red_font_id);	//open font file

	chr_sprite.scale=0;
	chr_sprite.type= RDSPR_NOCOMPRESSION+RDSPR_TRANS;
	chr_sprite.blend= 0;




	Build_surfaces();
	Build_chr_surfaces();

//fetch the 'save' text
	text = FetchTextLine( res_man.Res_open(149618691/SIZE), 149618691&0xffff );	// open text file & get the line
	strcpy((char*)&save_text[0], (char*)text+2);
//fetch the 'cancel' text
	text = FetchTextLine( res_man.Res_open(149618689/SIZE), 149618689&0xffff );	// open text file & get the line
	strcpy((char*)&cancel_text[0], (char*)text+2);
//blimey, life's never easy is it?



//control loop
	while (1)
	{
 		//--------------------------------------------------
	  	if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q during the smacker
		{
			Close_game();	//close engine systems down
			CloseAppWindow();
			exit(0);	//quit the game
		}

		// Service windows
		while (!gotTheFocus)
		{	names_built=0;
			edit_screen_slot=1000;
			if (ServiceWindows() == RDERR_APPCLOSED)
				break;
		}
 		//--------------------------------------------------

		EraseBackBuffer();


//print panel
		if	(DrawSurface(&panel_sprite, panel_surface)==RDERR_SURFACELOST)
			Renew_surfaces();

//print words on panel
		Engine_string(REST_BUT_X+32, REST_BUT_Y, controls_font_id, chr_surface, save_text);
		Engine_string(CAN_BUT_X+32, REST_BUT_Y, controls_font_id, chr_surface, cancel_text);





		slab_text_y=76;

//	print slabs
		for	(j=0;j<8;j++)
		{
			if	(edit_screen_slot!=j)
			{	slab_sprite[((base_slot+j)&3)].y=SLAB_Y+(j*SLAB_Y_SPACING);
				DrawSurface(&slab_sprite[((base_slot+j)&3)], slab_surface[((base_slot+j)&3)] );

//				print save name on slab if a game is saved in this slot
				if	(!names_built)
				{
					if (GetSaveDescription(base_slot+j, description) == SR_OK)	//if there is a savegame at this slot
					{	cur_slot_states[j]=1;	//slot used

						if	(!description[0])
							Con_fatal_error("NULL file name passed from GetSaveDescription!");	


	//					print the name on the slab
						sprintf((char*)buf[j], "%d.  %s", base_slot+j, description );
					}
					else
					{	sprintf((char*)buf[j], "%d.", base_slot+j);	//simply print the number
						cur_slot_states[j]=0;	//slot not used
					}
				}

				char_no=0;

				slab_text_x=SLAB_X+16;


				do
				{
					chr = buf[j][char_no];
					chr-=32;	//got true chr$

					chr_sprite.x=slab_text_x;

					head = (_frameHeader *)FetchFrameHeader(charSet, chr);

					chr_sprite.w=head->width;
					chr_sprite.h=head->height;

					chr_sprite.y=slab_text_y;
					DrawSurface(&chr_sprite, chr_surface[chr]);	//print

					slab_text_x+=head->width-CHARACTER_OVERLAP;		// overlap characters by 3 pixels;
					char_no++;
				}
				while(buf[j][char_no]);
			}
			slab_text_y+=SLAB_Y_SPACING;
		}

		names_built=1;

//draw the typing slab and text if we are still editing
		if	(edit_screen_slot!=1000)	//we are typing a name in
		{

			flash++;
			if	(flash<7)
				ed_buf[edit_pos]='_';
			else
				ed_buf[edit_pos]=' ';	//by putting a space in we'll always have a chr$ in the buffer

			if	(flash==14)
				flash=0;


//			now draw the current edit line
//			draw a red slab
			slab_sprite[(clicked_slot&3)+4].y=SLAB_Y+(edit_screen_slot*SLAB_Y_SPACING);
			DrawSurface(&slab_sprite[(clicked_slot&3)+4], slab_surface[(clicked_slot&3)+4] );

//			draw the text line
			char_no=0;
			edit_width=0;	//total pixel width of text being typed in
			slab_text_x=SLAB_X+16;

//			print the chr$
			do
			{
				chr = ed_buf[char_no];
				chr-=32;	//got true chr$

				chr_sprite.x=slab_text_x;

				head = (_frameHeader *)FetchFrameHeader(red_charSet, chr);

				chr_sprite.w=head->width;
				chr_sprite.h=head->height;

				chr_sprite.y=SLAB_Y+(edit_screen_slot*SLAB_Y_SPACING)+5;	//why 5? when its 2 on restore????????
				DrawSurface(&chr_sprite, red_chr_surface[chr]);	//print

				slab_text_x+=head->width-CHARACTER_OVERLAP;
				edit_width+=head->width-CHARACTER_OVERLAP;

				char_no++;
			}
			while(ed_buf[char_no]);
		}




//print buttons
//print restore button
		DrawSurface(&button_sprite[restore_button_state], button_surface[restore_button_state] );

//print cancel button
		DrawSurface(&can_button_sprite[can_button_state], can_button_surface[can_button_state] );

//print up button
		DrawSurface(&up_button_sprite[up_button_state], up_button_surface[up_button_state] );

//print down button
		DrawSurface(&down_button_sprite[down_button_state], down_button_surface[down_button_state] );

//print zup button
		DrawSurface(&zup_button_sprite[zup_button_state], zup_button_surface[zup_button_state] );

//print zdown button
		DrawSurface(&zdown_button_sprite[zdown_button_state], zdown_button_surface[zdown_button_state] );


		ProcessMenu();




//mouse over buttons?
//restore
		if	((mousex>REST_BUT_X)&&(mousex<REST_BUT_X+24)&&((mousey+40)>REST_BUT_Y)&&((mousey+40)<REST_BUT_Y+24))
			touching_restore_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	restore_button_state=0;
			touching_restore_button=0;
		}

//cancel
		if	((mousex>CAN_BUT_X)&&(mousex<CAN_BUT_X+24)&&((mousey+40)>CAN_BUT_Y)&&((mousey+40)<CAN_BUT_Y+24))
			touching_can_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	can_button_state=0;
			touching_can_button=0;
		}

//up
		if	((mousex>UP_BUT_X)&&(mousex<UP_BUT_X+17)&&((mousey+40)>UP_BUT_Y)&&((mousey+40)<UP_BUT_Y+17))
			touching_up_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	up_button_state=0;
			touching_up_button=0;
		}

//down
		if	((mousex>DOWN_BUT_X)&&(mousex<DOWN_BUT_X+17)&&((mousey+40)>DOWN_BUT_Y)&&((mousey+40)<DOWN_BUT_Y+17))
			touching_down_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	down_button_state=0;
			touching_down_button=0;
		}

//up
		if	((mousex>UP_BUT_X)&&(mousex<UP_BUT_X+17)&&((mousey+40)>ZUP_BUT_Y)&&((mousey+40)<ZUP_BUT_Y+17))
			touching_zup_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	zup_button_state=0;
			touching_zup_button=0;
		}

//down
		if	((mousex>DOWN_BUT_X)&&(mousex<DOWN_BUT_X+17)&&((mousey+40)>ZDOWN_BUT_Y)&&((mousey+40)<ZDOWN_BUT_Y+17))
			touching_zdown_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	zdown_button_state=0;
			touching_zdown_button=0;
		}







//check mouse clicked on a slab
		me = MouseEvent();	//get mouse event

		if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN))	//there's a mouse event to be processed
		{
			if	((mousex>SLAB_X)&&(mousex<SLAB_X+384)&&((mousey+40)>SLAB_Y)&&((mousey+40)<SLAB_Y+SLAB_Y_SPACING*8))
			{
				edit_screen_slot=((mousey+40)-SLAB_Y)/SLAB_Y_SPACING;
				clicked_slot=edit_screen_slot+base_slot;

				Zdebug("+ %d %d", edit_screen_slot, clicked_slot);

//				now edit the line
//				take a copy of the string

				for	(j=0;j<SAVE_DESCRIPTION_LEN;j++)
					ed_buf[j]=0;	//zero the string


				sprintf((char*)ed_buf, "%d.  ", clicked_slot);	//simply print the number - assume no name in panel
				first_chr=strlen((char*)ed_buf);
				edit_pos=first_chr;


				if (GetSaveDescription(clicked_slot, description) == SR_OK)	//if there is a savegame at this slot
				{
					sprintf((char*)ed_buf, "%d.  %s", clicked_slot, description );
					edit_pos=strlen((char*)ed_buf);	//recalculate cursor pos to end of string
				}

				Zdebug("first %d [%s]", first_chr, ed_buf);

			}

//			clicking on the restore button
			if	(touching_restore_button)
				restore_button_state=1;	//button now down

//			clicking on the cancel button
			if	(touching_can_button)
				can_button_state=1;	//button now down

//			clicking on the up button
			if	(touching_up_button)
			{	up_button_state=1;	//button now down
				scroll_rate=0;
			}

//			clicking on the down button
			if	(touching_down_button)
			{	down_button_state=1;	//button now down
				scroll_rate=0;
			}
//			clicking on the zup button
			if	(touching_zup_button)
			{	zup_button_state=1;	//button now down
				scroll_rate=0;
			}

//			clicking on the zdown button
			if	(touching_zdown_button)
			{	zdown_button_state=1;	//button now down
				scroll_rate=0;
			}


		}

//check for releasing the mouse button over a button
		if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONUP))	//there's a mouse event to be processed
		{

			if	((touching_restore_button)&&(restore_button_state))
			{
				restore_button_state=0;

				if	((edit_screen_slot!=1000)&&(edit_pos!=first_chr))	//we are editing and have a legal file name typed in
				{	//then save game - can also be saved when you press RETurn
					ed_buf[edit_pos]=0;	//remove cursor/[space]

					rv = SaveGame(clicked_slot, (uint8*)&ed_buf[first_chr]);

					if (rv == SR_OK)
					{	breakOut=1;	//finished
						if	((edit_screen_slot>6)&(base_slot<92))
							base_slot++;
					}
					else
					{
// Save & Restore error codes

// ERROR CODE			VALUE		MEANING													REASON
// ==========			=====		=======													======
// SR_OK				0x00000000	// ok													No worries
// SR_ERR_FILEOPEN		0x00000001	// can't open file										Could create file for saving, or couldn't find file for loading
// SR_ERR_INCOMPATIBLE	0x00000002	// (RestoreGame only) incompatible savegame data		Savegame file is obsolete. (Won't happen after development stops)
// SR_ERR_READFAIL		0x00000003	// (RestoreGame only) failed on reading savegame file	Something screwed up during the fread()
// SR_ERR_WRITEFAIL		0x00000004	// (SaveGame only) failed on writing savegame file		Something screwed up during the fwrite() - could be hard-drive full..?

						// WE NEED A MESSAGE BOX TO INDICATE FAILED SAVE - DON'T HALT THE GAME!

						if (rv == SR_ERR_FILEOPEN)
							Control_error((char*)(FetchTextLine( res_man.Res_open(213516674/SIZE), 213516674&0xffff)+2));
//							Save failed - could not open file

						else	// SR_ERR_WRITEFAIL
							Control_error((char*)(FetchTextLine( res_man.Res_open(213516676/SIZE), 213516676&0xffff)+2));
//							Save failed
					}
				}
			}
			else	if	((touching_can_button)&&(can_button_state))	//quit the screen
				breakOut=1;

			else	if	(touching_up_button)
				up_button_state=0;

			else	if	(touching_down_button)
				down_button_state=0;

			else	if	(touching_zup_button)
				zup_button_state=0;

			else	if	(touching_zdown_button)
				zdown_button_state=0;

		}



//scrolling downward
		if	( ((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(down_button_state)&&(base_slot<92))
		{	base_slot++;
			names_built=0;

			if	(edit_screen_slot!=1000)
			{	edit_screen_slot--;
				if	(base_slot>clicked_slot)
					edit_screen_slot=1000;
			}
		}

//scrolling upward
		if	(((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(up_button_state)&&(base_slot))
		{	base_slot--;
			names_built=0;

			if	(edit_screen_slot!=1000)
			{	edit_screen_slot++;
				if	((base_slot+7)<clicked_slot)
					edit_screen_slot=1000;
			}
		}

//scrolling zdownward
		if	(((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(zdown_button_state)&&(base_slot<92))
		{	base_slot+=8;
			names_built=0;

			if	(base_slot>92)
				base_slot=92;

			edit_screen_slot=1000;	//no more editing
		}

//scrolling zupward
		if	(((scroll_rate<1)||(scroll_rate>12))&&(!(scroll_rate&3))&&(zup_button_state)&&(base_slot))
		{	base_slot-=8;
			names_built=0;

			if	(base_slot>92)
				base_slot=0;
			edit_screen_slot=1000;	//no more editing
		}




//update scroll stuff
		scroll_rate++;




//-----

//deal with user input if the user is inputting
		if	((edit_screen_slot!=1000)&&(KeyWaiting()))	//we are typing a name in
		{
			ReadKey(&key_press);

			if (!key_press)	//escape sequences
			{
			}
			else	if	(key_press==27)	//ESC
			{
				edit_screen_slot=1000;	//quit this edit
				esc_release=42;	//stop the ESC key auto-repeating after this and quiting the save control
			}
			else if	(key_press==13)	//RETurn
			{
				if	(edit_pos!=first_chr)
				{	//save game
					ed_buf[edit_pos]=0;	//remove cursor/[space]

					Zdebug("%d %d %s", first_chr, edit_pos, &ed_buf[first_chr]);

					rv = SaveGame(clicked_slot, (uint8*)&ed_buf[first_chr]);

					if (rv == SR_OK)
					{	breakOut=1;	//finished
						if	((edit_screen_slot>6)&(base_slot<92))
							base_slot++;
					}
					else
					{
// Save & Restore error codes

// ERROR CODE			VALUE		MEANING													REASON
// ==========			=====		=======													======
// SR_OK				0x00000000	// ok													No worries
// SR_ERR_FILEOPEN		0x00000001	// can't open file										Could create file for saving, or couldn't find file for loading
// SR_ERR_INCOMPATIBLE	0x00000002	// (RestoreGame only) incompatible savegame data		Savegame file is obsolete. (Won't happen after development stops)
// SR_ERR_READFAIL		0x00000003	// (RestoreGame only) failed on reading savegame file	Something screwed up during the fread()
// SR_ERR_WRITEFAIL		0x00000004	// (SaveGame only) failed on writing savegame file		Something screwed up during the fwrite() - could be hard-drive full..?


						// WE NEED A MESSAGE BOX TO INDICATE FAILED SAVE - DON'T HALT THE GAME!

						if (rv == SR_ERR_FILEOPEN)
							Control_error((char*)(FetchTextLine( res_man.Res_open(213516674/SIZE), 213516674&0xffff)+2));
//							Save failed - could not open file

						else	// SR_ERR_WRITEFAIL
							Control_error((char*)(FetchTextLine( res_man.Res_open(213516676/SIZE), 213516676&0xffff)+2));
//							Save failed
					}
				}
				else	edit_screen_slot=1000;	//dont save an empty slot and cancel editing
			}

			else	if	(key_press==8)	//delete
			{
				if	(edit_pos!=first_chr)
				{
					ed_buf[edit_pos]=0;	//delete cursor chr$
					edit_pos--;
					ed_buf[edit_pos]=0;
				}
			}
			else	if	((key_press<32)||(key_press>'z'))
				Zdebug("save ignoring key - %d", key_press);
			else
			{
//				if	(edit_pos<(20))	//less one to leave room for the cursor


				if	((edit_width<350)&&(edit_pos<SAVE_DESCRIPTION_LEN-2))
					ed_buf[edit_pos++]=key_press;
				else	//end of line has been reached, so keep replacing last letter
					ed_buf[edit_pos-1]=key_press;	//replace
			}
		}
		else	if (KeyWaiting())
		{
			ReadKey(&key_press);	//kill the key we just pressed
			if	((key_press==27)&&(!esc_release))	//ESC
				breakOut=1;

			else	if	(key_press!=27)
				esc_release=0;
		}
		else	if (!KeyWaiting())
			esc_release=0;











		if (breakOut)
		{
			break;	//quit this stuff - ap will eventually close in the mainloop
		}

		if	(!first)
		{
			first++;
			SetFullPalette(CONTROL_PANEL_PALETTE);	// see Build_display.cpp (James17jun97)
		}
	}	//while


	Kill_surfaces();
	Kill_chr_surfaces();

}
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
void	Quit_control(void)	//Tony2Apr97
{
	uint32	res;


	res=Generic_mini_control(149618692);	//quit text


	if	(!res)
		return;	//just return to game
	EraseBackBuffer();

	Close_game();	//close engine systems down
	CloseAppWindow();

	exit(0);

}
//-----------------------------------------------------------------------------------------------------------------------
void	Restart_control(void)	//Tony4Apr97
{
	uint32	res;
	uint32	temp_demo_flag;


	res=Generic_mini_control(149618693);	//restart text


	if	(!res)
		return;	//just return to game


	Kill_music();	// Stop music instantly! (James22aug97)


	DEAD=0;	//in case we were dead - well we're not anymore!

//clean up surface memory
	Kill_mini_surfaces();
	EraseBackBuffer();
	//ProcessMenu();	//draw menu

//restart the game
//clear all memory and reset the globals

	temp_demo_flag = DEMO;

	res_man.Remove_all_res();	//remove all resources from memory, including player object & global variables
	SetGlobalInterpreterVariables((int32*)(res_man.Res_open(1)+sizeof(_standardHeader)));	//reopen global variables resource & send address to interpreter - it won't be moving
	res_man.Res_close(1);

	DEMO = temp_demo_flag;

	FreeAllRouteMem();	// free all the route memory blocks from previous game

	g_sword2->Start_game();	// call the same function that first started us up


//prime system with a game cycle
	Reset_render_lists();	// reset the graphic 'buildit' list before a new logic list (see FN_register_frame)
	Reset_mouse_list();		// reset the mouse hot-spot list (see FN_register_mouse & FN_register_frame)

	CloseMenuImmediately();


 	//---------------------------------------------------------------
	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET! (James29may97)
	// - this is taken from FN_init_background
	this_screen.scroll_flag = 2;	// switch on scrolling (2 means first time on screen)
	//---------------------------------------------------------------

	res = LLogic.Process_session();

	if	(res)
		Con_fatal_error("restart 1st cycle failed??");

 	this_screen.new_palette=99;	// (JEL08oct97) so palette not restored immediately after control panel - we want to fade up instead!

}
//-----------------------------------------------------------------------------------------------------------------------
uint32	Generic_mini_control(uint32	text_id)	//Tony2Apr97
{

//	returns 1 for OK pressed
// returns 0 for CANCEL pressed

	int	breakOut=0;
	char	c;

	uint8	quit_text[MAX_STRING_LEN];
	uint8	ok_text[MAX_STRING_LEN];
	uint8	cancel_text[MAX_STRING_LEN];
	uint8	*text;

	_mouseEvent	*me;

	int	first=0;

	int	text_len;

	#define	OK_BUT_X	(QUIT_X+40)
	#define	OK_BUT_Y	(QUIT_Y+110)
	#define	CAN_BU_Y	(QUIT_Y+172)


//do some driver stuff
//	ResetRenderEngine();


//fetch the 'quit' text
	text = FetchTextLine( res_man.Res_open(text_id/SIZE), text_id&0xffff );	//quit or restart
	strcpy((char*)&quit_text[0], (char*)text+2);
	text_len = Pixel_text_length(&quit_text[0], controls_font_id);


//fetch the 'ok' text
	text = FetchTextLine( res_man.Res_open(149618688/SIZE), 149618688&0xffff );	//ok
	strcpy((char*)&ok_text[0], (char*)text+2);

//fetch the 'cancel' text
	text = FetchTextLine( res_man.Res_open(149618689/SIZE), 149618689&0xffff );	//cancel
	strcpy((char*)&cancel_text[0], (char*)text+2);
//blimey, life's never easy is it?




//buttons unpressed
	restore_button_state=0;
	can_button_state=0;


//build surfaces
	Build_mini_surfaces();



//control loop
	while (1)
	{
		//--------------------------------------------------
		// Service windows
		if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q
		{
			Close_game();	//close engine systems down
			CloseAppWindow();
			exit(0);	//quit the game
		}

		while (!gotTheFocus)
			if (ServiceWindows() == RDERR_APPCLOSED)
				break;

 		//--------------------------------------------------


		EraseBackBuffer();


//print panel
		if	(DrawSurface(&panel_sprite, panel_surface)==RDERR_SURFACELOST)
		{
				Kill_mini_surfaces();
				Build_mini_surfaces();
		}

//print words on panel quit_x+81
		Engine_string(310-(text_len/2), QUIT_Y+30, controls_font_id, chr_surface, quit_text);	// quit
		Engine_string(QUIT_X+67, QUIT_Y+110, controls_font_id, chr_surface, ok_text);		// ok
		Engine_string(QUIT_X+67, QUIT_Y+172, controls_font_id, chr_surface, cancel_text);	// cancel

//print buttons
//print ok button
		DrawSurface(&button_sprite[restore_button_state], button_surface[restore_button_state] );
//print cancel button
		DrawSurface(&can_button_sprite[can_button_state], can_button_surface[can_button_state] );

//keep menu up too
		ProcessMenu();

//user can ESC quit
		if (KeyWaiting())
		{
			ReadKey(&c);	//kill the key we just pressed
			if	(c==27)	//ESC
				break;
		}


//mouse over ok button?
		if	((mousex>OK_BUT_X)&&(mousex<OK_BUT_X+24)&&((mousey+40)>OK_BUT_Y)&&((mousey+40)<OK_BUT_Y+24))
			touching_restore_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	restore_button_state=0;
			touching_restore_button=0;
		}
//mouse over cancel button?
		if	((mousex>OK_BUT_X)&&(mousex<OK_BUT_X+24)&&((mousey+40)>CAN_BU_Y)&&((mousey+40)<CAN_BU_Y+24))
			touching_can_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	can_button_state=0;
			touching_can_button=0;
		}


//pressing on a button
		me = MouseEvent();	//get mouse event

		if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN))	//there's a mouse event to be processed
		{
			if	(touching_restore_button)
				restore_button_state=1;

			if	(touching_can_button)
				can_button_state=1;

		}
		else	if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONUP))
		{
			if	((touching_restore_button)&&(restore_button_state))	//quit the game
			{
				return(1);
			}

			if	((touching_can_button)&&(can_button_state))
			{	can_button_state=0;
				breakOut=1;
			}

		}


		if	(breakOut)
			break;

		if	(!first)
		{
			first++;
			SetFullPalette(CONTROL_PANEL_PALETTE);	// see Build_display.cpp (James17jun97)
		}
	}


	Kill_mini_surfaces();

	return(0);
}
//-----------------------------------------------------------------------------------------------------------------------
void	Build_mini_surfaces(void)	//tony3Apr97
{
	Create_surface_image(&panel_sprite, &panel_surface, 1996, QUIT_X, QUIT_Y, 0);
//ok button
	Create_surface_image(&button_sprite[0], &button_surface[0], 2002,	OK_BUT_X, OK_BUT_Y, 0);
	Create_surface_image(&button_sprite[1], &button_surface[1], 2002,	OK_BUT_X, OK_BUT_Y, 1);
//cancel button
	Create_surface_image(&can_button_sprite[0], &can_button_surface[0], 2002,	OK_BUT_X, CAN_BU_Y, 0);
	Create_surface_image(&can_button_sprite[1], &can_button_surface[1], 2002,	OK_BUT_X, CAN_BU_Y, 1);

	Build_chr_surfaces();
}
//-----------------------------------------------------------------------------------------------------------------------
void	Kill_mini_surfaces(void)	//tony3Apr97
{
	DeleteSurface(panel_surface);
//ok button
	DeleteSurface(button_surface[0]);
	DeleteSurface(button_surface[1]);
//cancel button
	DeleteSurface(can_button_surface[0]);
	DeleteSurface(can_button_surface[1]);

	Kill_chr_surfaces();
}
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
void Engine_string(uint32 x, uint32 y, uint32 res, uint8 **surface_list, uint8 *buf)	//tony2Apr97
{
//takes fonts as sprites and prints transparently to screen
//requires the chr$ surfaces have been setup

	int	char_no=0;
	int	chr;
	uint8	*chars;


	chars = res_man.Res_open(res);	//open font file

	chr_sprite.scale=0;
	chr_sprite.type= RDSPR_NOCOMPRESSION+RDSPR_TRANS;
	chr_sprite.blend= 0;


	chr_sprite.x=x;
	chr_sprite.y=y;

	do
	{
		chr = buf[char_no];
		chr-=32;	//got true chr$

		head = (_frameHeader *)FetchFrameHeader(chars, chr);

		chr_sprite.w=head->width;
		chr_sprite.h=head->height;

		DrawSurface(&chr_sprite, surface_list[chr]);	//print

		chr_sprite.x+=head->width-CHARACTER_OVERLAP;
		char_no++;
	}
	while(buf[char_no]);

	res_man.Res_close(res);	//close font file
}
//-----------------------------------------------------------------------------------------------------------------------
uint32	Pixel_text_length(uint8	*buf, uint32 res)	//tony4Apr97
{

	int	char_no=0;
	int	chr;
	uint8	*chars;
	uint32	width=0;


	chars = res_man.Res_open(res);	//open font file

	do
	{
		chr = buf[char_no];
		chr-=32;	//got true chr$

		head = (_frameHeader *)FetchFrameHeader(chars, chr);

		width+=head->width-CHARACTER_OVERLAP;

		char_no++;
	}
	while(buf[char_no]);

	res_man.Res_close(res);	//close font file

	return(width);
}
//-----------------------------------------------------------------------------------------------------------------------
void	Control_error(char* text)	//Tony13May97
{
//print a message on screen, wait for key, return

	_mouseEvent	*me;
	char	c;

	DisplayMsg( (uint8*)text, 0 );	// 2nd param is duration


	while (1)
	{
 		//--------------------------------------------------
		// Service windows

	  	if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q during the smacker
		{
			Close_game();	//close engine systems down
			CloseAppWindow();
			exit(0);	//quit the game
		}
 
		while (!gotTheFocus)
			if (ServiceWindows() == RDERR_APPCLOSED)
				break;
		//--------------------------------------------------

		if (KeyWaiting())
		{
			ReadKey(&c);	//kill the key we just pressed
			if	(c==27)	//ESC
				break;
		}

		me = MouseEvent();	//get mouse event

		if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN))	//there's a mouse event to be processed
			break;



	}


	RemoveMsg();	// Removes the message.

}

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
int32	ReadOptionSettings(void)	//pete10Jun97
{
	// settings file is 9 bytes long, bytes 1 to 3 = music, speech and fx volumes
	// bytes 4 to 6 = music, speech and fx mute states, byte 7 = grfx level
	// byte 8 = subtitle state and byte 9 = object label state.

	uint8 buff[10];
	FILE *fp;
	
	if ((fp = fopen("Settings.dat","rb"))== NULL)
		return (1);

	if (fread(buff,1,10,fp) != 10)
	{
		fclose(fp);
		return (2);
	}

	fclose(fp);

	g_sword2->_sound->SetMusicVolume(buff[0]);
	g_sword2->_sound->SetSpeechVolume(buff[1]);
	g_sword2->_sound->SetFxVolume(buff[2]);
	g_sword2->_sound->MuteMusic(buff[3]);
	g_sword2->_sound->MuteSpeech(buff[4]);
	g_sword2->_sound->MuteFx(buff[5]);


	UpdateGraphicsLevel(GetRenderType(), buff[6]);	// (James13jun97)

	speechSelected      = !buff[4];
	subtitles			= buff[7];
	pointerTextSelected = buff[8];

	if (buff[9] != stereoReversed)
		g_sword2->_sound->ReverseStereo();

	stereoReversed = buff[9];

	return (0);
}
//-----------------------------------------------------------------------------------------------------------------------
int32	WriteOptionSettings(void)	//pete10Jun97
{
	uint8 buff[10];
	FILE *fp;

	buff[0] = g_sword2->_sound->GetMusicVolume();
	buff[1] = g_sword2->_sound->GetSpeechVolume();
	buff[2] = g_sword2->_sound->GetFxVolume();
	buff[3] = g_sword2->_sound->IsMusicMute();
	buff[4] = g_sword2->_sound->IsSpeechMute();
	buff[5] = g_sword2->_sound->IsFxMute();
	buff[6] = GetRenderType();
	buff[7] = subtitles;
	buff[8] = pointerTextSelected;
	buff[9] = stereoReversed;
	
	if ((fp = fopen("Settings.dat","wb"))== NULL)
		return (1);

	if (fwrite(buff,1,10,fp) != 10)
	{
		fclose(fp);
		return (2);
	}

	fclose(fp);

	return (0);
}


//-----------------------------------------------------------------------------------------------------------------------
void	Build_option_surfaces(void)	//pete6Jun97
{
	Create_surface_image(&panel_sprite, &panel_surface, 3405, 0, OPTION_Y, 0);
// object label button
	Create_surface_image(&up_button_sprite[0], &up_button_surface[0], 3687, OBJ_LABEL_X,OBJ_LABEL_Y, 0);
	Create_surface_image(&up_button_sprite[1], &up_button_surface[1], 3687, OBJ_LABEL_X,OBJ_LABEL_Y, 1);
//subtitle button
	Create_surface_image(&down_button_sprite[0], &down_button_surface[0], 3687, SUBTITLE_X, SUBTITLE_Y, 0);
	Create_surface_image(&down_button_sprite[1], &down_button_surface[1], 3687, SUBTITLE_X, SUBTITLE_Y, 1);
//ok button
	Create_surface_image(&button_sprite[0], &button_surface[0],	901, OPT_OK_X, OPT_OK_Y, 0);
	Create_surface_image(&button_sprite[1],	&button_surface[1],	901, OPT_OK_X, OPT_OK_Y, 1);
//cancel button
	Create_surface_image(&can_button_sprite[0], &can_button_surface[0], 901, OPT_CAN_X, OPT_CAN_Y, 0);
	Create_surface_image(&can_button_sprite[1], &can_button_surface[1], 901, OPT_CAN_X, OPT_CAN_Y, 1);
//sliders
	Create_surface_image(&slab_sprite[0], &slab_surface[0], 3406, SLIDER_TRK_X,MUSIC_TRK_Y,0);	// music slider
	Create_surface_image(&slab_sprite[1], &slab_surface[1], 3406, SLIDER_TRK_X,SPEECH_TRK_Y,0); // speech slider
	Create_surface_image(&slab_sprite[2], &slab_surface[2], 3406, SLIDER_TRK_X,FX_TRK_Y,0);		// fx slider
	Create_surface_image(&slab_sprite[3], &slab_surface[3], 3406, SLIDER_TRK_X,GRFX_TRK_Y,0);	// graphics slider
//mute buttons
	Create_surface_image(&zup_button_sprite[0], &zup_button_surface[0], 3315, MUTE_X, MUSIC_TRK_Y-4, 0);	// music mute
	Create_surface_image(&zup_button_sprite[1], &zup_button_surface[1], 3315, MUTE_X, MUSIC_TRK_Y-4, 1);
	Create_surface_image(&zdown_button_sprite[0], &zdown_button_surface[0],3315, MUTE_X, SPEECH_TRK_Y-3, 0);// speech mute
	Create_surface_image(&zdown_button_sprite[1], &zdown_button_surface[1],3315, MUTE_X, SPEECH_TRK_Y-3, 1);
	Create_surface_image(&slab_sprite[4], &slab_surface[4], 3315, MUTE_X,FX_TRK_Y-4,0);						// fx mute
	Create_surface_image(&slab_sprite[5], &slab_surface[5], 3315, MUTE_X,FX_TRK_Y-4,1);
//graphics level icon
	Create_surface_image(&grfx_icon_sprite[0], &grfx_icon_surface[0], 256, GRFX_ICON_X, GRFX_ICON_Y, 0);	// lowest grapihics level icon
	Create_surface_image(&grfx_icon_sprite[1], &grfx_icon_surface[1], 256, GRFX_ICON_X, GRFX_ICON_Y, 1);	// medium low grapihics level icon
	Create_surface_image(&grfx_icon_sprite[2], &grfx_icon_surface[2], 256, GRFX_ICON_X, GRFX_ICON_Y, 2);	// mewdium high grapihics level icon
	Create_surface_image(&grfx_icon_sprite[3], &grfx_icon_surface[3], 256, GRFX_ICON_X, GRFX_ICON_Y, 3);	// highest grapihics level icon
//reverse stereo button
	Create_surface_image(&slab_sprite[6], &slab_surface[6], 3687, STEREO_X,STEREO_Y,0);
	Create_surface_image(&slab_sprite[7], &slab_surface[7], 3687, STEREO_X,STEREO_Y,1);

	Build_chr_surfaces();
}
//-----------------------------------------------------------------------------------------------------------------------
void	Kill_option_surfaces(void)	//pete6Jun97
{
	DeleteSurface(panel_surface);
//object label button
	DeleteSurface(up_button_surface[0]);
	DeleteSurface(up_button_surface[1]);
//subtitle button
	DeleteSurface(down_button_surface[0]);
	DeleteSurface(down_button_surface[1]);
//ok button
	DeleteSurface(button_surface[0]);
	DeleteSurface(button_surface[1]);
//cancel button
	DeleteSurface(can_button_surface[0]);
	DeleteSurface(can_button_surface[1]);
//sliders
	DeleteSurface(slab_surface[0]);
	DeleteSurface(slab_surface[1]);
	DeleteSurface(slab_surface[2]);
	DeleteSurface(slab_surface[3]);
//mute buttons
	DeleteSurface(zup_button_surface[0]);
	DeleteSurface(zup_button_surface[1]);
	DeleteSurface(zdown_button_surface[0]);
	DeleteSurface(zdown_button_surface[1]);
	DeleteSurface(slab_surface[4]);
	DeleteSurface(slab_surface[5]);
//graphics level icon
	DeleteSurface(grfx_icon_surface[0]);
	DeleteSurface(grfx_icon_surface[1]);
	DeleteSurface(grfx_icon_surface[2]);
	DeleteSurface(grfx_icon_surface[3]);
//reverse stereo icon
	DeleteSurface(slab_surface[6]);
	DeleteSurface(slab_surface[7]);

	Kill_chr_surfaces();
}
//-----------------------------------------------------------------------------------------------------------------------
int		Mouse_touching_button(int32 x, int32 y, int32 w, int32 h)	//pete9Jun97
{
	if	((mousex>x)&&(mousex<x+w)&&((mousey+40)>y)&&((mousey+40)<y+h))
		return (1);
	else
		return (0);
}

//-----------------------------------------------------------------------------------------------------------------------
void	Option_control(void)	//Pete6Jun97
{
#define WORD_BUTTON_GAP 10

// some things left by the last tennant
	char		 c;
	_mouseEvent	*me;
	int			 first = 0;

// text strings and lengths
	uint8	title_text[MAX_STRING_LEN];
	uint8   subtitle_text[MAX_STRING_LEN];
	uint8	object_text[MAX_STRING_LEN];
	uint8	ok_text[MAX_STRING_LEN];
	uint8	cancel_text[MAX_STRING_LEN];
	uint8	music_text[MAX_STRING_LEN];
	uint8	speech_text[MAX_STRING_LEN];
	uint8	fx_text[MAX_STRING_LEN];
	uint8	graphics_text[MAX_STRING_LEN];
	uint8	stereo_text[MAX_STRING_LEN];
	uint8  *text;
	int		title_len, subtitle_len, ok_len, cancel_len, left_align, test_len;

// slider values
	uint8   musicVolume  = g_sword2->_sound->GetMusicVolume();
	uint8	speechVolume = g_sword2->_sound->GetSpeechVolume();
	uint8	fxVolume	 = g_sword2->_sound->GetFxVolume();
	uint8	grfxLevel    = GetRenderType();

// safe slider values for restoring on cancel
	//uint8   safe_musicVolume  = musicVolume;
	//uint8   safe_speechVolume = speechVolume;
	//uint8   safe_fxVolume     = fxVolume;
	uint8   safe_grfxLevel    = grfxLevel;

// button state variables
	uint8	dreverse_stereo_state = 0, dmusic_mute_state = 0, dspeech_mute_state = 0, dfx_mute_state = 0, dobject_state = 0, dsubtitle_state = 0;
	uint8	touching_reverse_stereo = 0, touching_music_mute = 0, touching_fx_mute = 0, touching_speech_mute = 0, touching_object = 0, touching_subtitle = 0;
	uint8   lb_down = 0;

// Slider targets
	uint8	music_target  = musicVolume;
	uint8	fx_target     = fxVolume;
	uint8	speech_target = speechVolume;
	uint8   grfx_target   = grfxLevel;

// Slider movement types (click in track or drag button)
	uint8   music_tracking = 0, fx_tracking = 0, speech_tracking = 0, grfx_tracking = 0;

//do some driver stuff
//	ResetRenderEngine();

// FETCH THE TEXT
//fetch the 'options' text
	text = FetchTextLine( res_man.Res_open(149618698/SIZE), 149618698&0xffff );	//options (title)
	strcpy((char*)title_text, (char*)text+2);
	title_len = Pixel_text_length(title_text, controls_font_id);

//fetch the 'subtitles' text
	text = FetchTextLine( res_man.Res_open(149618699/SIZE), 149618699&0xffff );	//subtitles
	strcpy((char*)subtitle_text, (char*)text+2);
	subtitle_len = Pixel_text_length(subtitle_text, controls_font_id) + WORD_BUTTON_GAP;

//fetch the 'object labels' text
	text = FetchTextLine( res_man.Res_open(149618700/SIZE), 149618700&0xffff );	//object
	strcpy((char*)object_text, (char*)text+2);
	left_align = Pixel_text_length(object_text, controls_font_id) + WORD_BUTTON_GAP;

//fetch the 'ok' text
	text = FetchTextLine( res_man.Res_open(149618688/SIZE), 149618688&0xffff );	//ok
	strcpy((char*)ok_text, (char*)text+2);
	ok_len = Pixel_text_length(ok_text, controls_font_id) + WORD_BUTTON_GAP;

//fetch the 'cancel' text
	text = FetchTextLine( res_man.Res_open(149618689/SIZE), 149618689&0xffff );	//cancel
	strcpy((char*)cancel_text, (char*)text+2);
	cancel_len = Pixel_text_length(cancel_text, controls_font_id) + WORD_BUTTON_GAP;

//fetch the 'music volume' text
	text = FetchTextLine( res_man.Res_open(149618702/SIZE), 149618702&0xffff );	//music volume
	strcpy((char*)music_text, (char*)text+2);
	test_len = Pixel_text_length(music_text, controls_font_id) + WORD_BUTTON_GAP;
	if (test_len>left_align)
		left_align = test_len;

//fetch the 'speech volume' text
	text = FetchTextLine( res_man.Res_open(149618703/SIZE), 149618703&0xffff );	//speech volume
	strcpy((char*)speech_text, (char*)text+2);
	test_len = Pixel_text_length(speech_text, controls_font_id) + WORD_BUTTON_GAP;
	if (test_len>left_align)
		left_align = test_len;

//fetch the 'fx volume' text
	text = FetchTextLine( res_man.Res_open(149618704/SIZE), 149618704&0xffff );	//fx volume
	strcpy((char*)fx_text, (char*)text+2);
	test_len = Pixel_text_length(fx_text, controls_font_id) + WORD_BUTTON_GAP;
	if (test_len>left_align)
		left_align = test_len;

//fetch the 'grapihics quality' text
	text = FetchTextLine( res_man.Res_open(149618705/SIZE), 149618705&0xffff );	//graphics quality
	strcpy((char*)graphics_text, (char*)text+2);
	test_len = Pixel_text_length(graphics_text, controls_font_id) + WORD_BUTTON_GAP;
	if (test_len>left_align)
		left_align = test_len;

//fetch the 'grapihics quality' text
	text = FetchTextLine( res_man.Res_open(149618709/SIZE), 149618709&0xffff );	//graphics quality
	strcpy((char*)stereo_text, (char*)text+2);
	test_len = Pixel_text_length(stereo_text, controls_font_id) + WORD_BUTTON_GAP;
	if (test_len>left_align)
		left_align = test_len;


//blimey, life's never easy is it?
//not once you've got out of bed !


//set the button states
	restore_button_state = 0;
	can_button_state	 = 0;
	touching_object	     = 0;
	touching_subtitle    = 0;
	uint8 object_state	 = pointerTextSelected;
	uint8 subtitle_state = subtitles;
	uint8 stereo_state   = stereoReversed;

	uint8 music_mute_state   = g_sword2->_sound->IsMusicMute();
	uint8 speech_mute_state  = g_sword2->_sound->IsSpeechMute();
	uint8 fx_mute_state	     = g_sword2->_sound->IsFxMute();


//build the button surfaces surfaces
	Build_option_surfaces();

//position the sliders
	slab_sprite[0].x = SLIDER_TRK_X + (SLIDER_TRK_W * musicVolume) / 16;
	slab_sprite[1].x = SLIDER_TRK_X + (SLIDER_TRK_W * speechVolume) / 14;
	slab_sprite[2].x = SLIDER_TRK_X + (SLIDER_TRK_W * fxVolume) / 14;
	slab_sprite[3].x = SLIDER_TRK_X + (SLIDER_TRK_W * grfxLevel) / 3;

//control loop
	while (1)
	{
// Update any moving sliders
		// music
		if (slab_sprite[0].x<SLIDER_TRK_X + (SLIDER_TRK_W * music_target) / 16)
		{
			if ((SLIDER_TRK_X + (SLIDER_TRK_W * music_target) / 16)-slab_sprite[0].x<2)
				slab_sprite[0].x++;
			else
				slab_sprite[0].x +=2;
			musicVolume = (int)((float)((slab_sprite[0].x-SLIDER_TRK_X)*16)/(float)SLIDER_TRK_W+0.5);
		}
		else if (slab_sprite[0].x>SLIDER_TRK_X + (SLIDER_TRK_W * music_target) / 16)
		{
			if (slab_sprite[0].x-(SLIDER_TRK_X + (SLIDER_TRK_W * music_target) / 16)<2)
				slab_sprite[0].x--;
			else
				slab_sprite[0].x -=2;
			musicVolume = (int)((float)((slab_sprite[0].x-SLIDER_TRK_X)*16)/(float)SLIDER_TRK_W+0.5);

			if (!musicVolume)
				music_mute_state = 1;
			else
				music_mute_state = 0;
		}

		// speech
		if (slab_sprite[1].x<SLIDER_TRK_X + (SLIDER_TRK_W * speech_target) / 14)
		{
			if ((SLIDER_TRK_X + (SLIDER_TRK_W * speech_target) / 14)-slab_sprite[1].x<2)
				slab_sprite[1].x++;
			else
				slab_sprite[1].x +=2;
			speechVolume = (int)((float)((slab_sprite[1].x-SLIDER_TRK_X)*14)/(float)SLIDER_TRK_W+0.5);
		}
		else if (slab_sprite[1].x>SLIDER_TRK_X + (SLIDER_TRK_W * speech_target) / 14)
		{
			if (slab_sprite[1].x-(SLIDER_TRK_X + (SLIDER_TRK_W * speech_target) / 14)<2)
				slab_sprite[1].x--;
			else
				slab_sprite[1].x -=2;
			speechVolume = (int)((float)((slab_sprite[1].x-SLIDER_TRK_X)*14)/(float)SLIDER_TRK_W+0.5);

			if (!speechVolume)
				speech_mute_state = 1;
			else
				speech_mute_state = 0;
		}

		// fx
		if (slab_sprite[2].x<SLIDER_TRK_X + (SLIDER_TRK_W * fx_target) / 14)
		{
			if ((SLIDER_TRK_X + (SLIDER_TRK_W * fx_target) / 14)-slab_sprite[2].x<2)
				slab_sprite[2].x++;
			else
				slab_sprite[2].x +=2;
			fxVolume = (int)((float)((slab_sprite[2].x-SLIDER_TRK_X)*14)/(float)SLIDER_TRK_W+0.5);
		}
		else if (slab_sprite[2].x>SLIDER_TRK_X + (SLIDER_TRK_W * fx_target) / 14)
		{
			if (slab_sprite[2].x-(SLIDER_TRK_X + (SLIDER_TRK_W * fx_target) / 14)<2)
				slab_sprite[2].x--;
			else
				slab_sprite[2].x -=2;
			fxVolume = (int)((float)((slab_sprite[2].x-SLIDER_TRK_X)*14)/(float)SLIDER_TRK_W+0.5);

			if (!fxVolume)
				fx_mute_state = 1;
			else
				fx_mute_state = 0;
		}

		// grfx
		if (slab_sprite[3].x<SLIDER_TRK_X + (SLIDER_TRK_W * grfx_target) / 3)
		{
			if ((SLIDER_TRK_X + (SLIDER_TRK_W * grfx_target) / 3)-slab_sprite[3].x<2)
				slab_sprite[3].x++;
			else
				slab_sprite[3].x +=2;
			grfxLevel = (int)((float)((slab_sprite[3].x-SLIDER_TRK_X)*3)/(float)SLIDER_TRK_W+0.5);
		}
		else if (slab_sprite[3].x>SLIDER_TRK_X + (SLIDER_TRK_W * grfx_target) / 3)
		{
			if (slab_sprite[3].x-(SLIDER_TRK_X + (SLIDER_TRK_W * grfx_target) / 3)<2)
				slab_sprite[3].x--;
			else
				slab_sprite[3].x -=2;
			grfxLevel = (int)((float)((slab_sprite[3].x-SLIDER_TRK_X)*3)/(float)SLIDER_TRK_W+0.5);
		}

		
		if (music_tracking)	// music tracking
		{
			slab_sprite[0].x = mousex - SLIDER_W/2;
			if (slab_sprite[0].x < SLIDER_TRK_X)
				slab_sprite[0].x = SLIDER_TRK_X;
			else if (slab_sprite[0].x > SLIDER_TRK_X+SLIDER_TRK_W)
				slab_sprite[0].x = SLIDER_TRK_X+SLIDER_TRK_W;
			music_target = musicVolume = (int)((float)((slab_sprite[0].x-SLIDER_TRK_X)*16)/(float)SLIDER_TRK_W+0.5);

			if (!musicVolume)
				music_mute_state = 1;
			else
				music_mute_state = 0;

		}
		else if (speech_tracking) // speech tracking
		{
			slab_sprite[1].x = mousex - SLIDER_W/2;
			if (slab_sprite[1].x < SLIDER_TRK_X)
				slab_sprite[1].x = SLIDER_TRK_X;
			else if (slab_sprite[1].x > SLIDER_TRK_X+SLIDER_TRK_W)
				slab_sprite[1].x = SLIDER_TRK_X+SLIDER_TRK_W;
			speech_target = speechVolume = (int)((float)((slab_sprite[1].x-SLIDER_TRK_X)*14)/(float)SLIDER_TRK_W+0.5);

			if (!speechVolume)
				speech_mute_state = 1;
			else
				speech_mute_state = 0;

		}
		else if (fx_tracking) // fx tracking
		{
			slab_sprite[2].x = mousex - SLIDER_W/2;
			if (slab_sprite[2].x < SLIDER_TRK_X)
				slab_sprite[2].x = SLIDER_TRK_X;
			else if (slab_sprite[2].x > SLIDER_TRK_X+SLIDER_TRK_W)
				slab_sprite[2].x = SLIDER_TRK_X+SLIDER_TRK_W;
			fx_target = fxVolume = (int)((float)((slab_sprite[2].x-SLIDER_TRK_X)*14)/(float)SLIDER_TRK_W+0.5);

			if (!fxVolume)
				fx_mute_state = 1;
			else
				fx_mute_state = 0;

		}
		else if (grfx_tracking) // grfx tracking
		{
			slab_sprite[3].x = mousex - SLIDER_W/2;
			if (slab_sprite[3].x < SLIDER_TRK_X)
				slab_sprite[3].x = SLIDER_TRK_X;
			else if (slab_sprite[3].x > SLIDER_TRK_X+SLIDER_TRK_W)
				slab_sprite[3].x = SLIDER_TRK_X+SLIDER_TRK_W;
			grfx_target = grfxLevel = (int)((float)((slab_sprite[3].x-SLIDER_TRK_X)*3)/(float)SLIDER_TRK_W+0.5);
		}

		if (!music_mute_state)
			g_sword2->_sound->SetMusicVolume(musicVolume);
		else
			g_sword2->_sound->SetMusicVolume(0);

		if (!fx_mute_state)
			g_sword2->_sound->SetFxVolume(fxVolume);
		else
			g_sword2->_sound->SetFxVolume(0);

		if (!speech_mute_state)
			g_sword2->_sound->SetSpeechVolume(speechVolume);
		else
			g_sword2->_sound->SetSpeechVolume(0);

		//--------------------------------------------------
 		// Service windows

	  	if (ServiceWindows() == RDERR_APPCLOSED)	// if we pressed Ctrl-Q during the smacker
		{
			Close_game();	//close engine systems down
			CloseAppWindow();
			exit(0);	//quit the game
		}

		while (!gotTheFocus)
			if (ServiceWindows() == RDERR_APPCLOSED)
				break;
 		//--------------------------------------------------

		EraseBackBuffer();


//print panel
		while (DrawSurface(&panel_sprite, panel_surface)==RDERR_SURFACELOST)
		{
				Kill_option_surfaces();
				Build_option_surfaces();
		};

//print words on panel option panel
		Engine_string(OPTION_W/2-(title_len/2)+OPTION_X,OPTION_Y+15,	controls_font_id, chr_surface, title_text);		//options
		Engine_string(SUBTITLE_X-subtitle_len,			SUBTITLE_Y+3,	controls_font_id, chr_surface, subtitle_text);	//subtitles
		Engine_string(SLIDER_TRK_X-left_align,			OBJ_LABEL_Y+3,	controls_font_id, chr_surface, object_text);	//object labels
		Engine_string(OPT_OK_X-ok_len,					OPT_OK_Y,		controls_font_id, chr_surface, ok_text);		//ok
		Engine_string(OPT_CAN_X-cancel_len,				OPT_CAN_Y,		controls_font_id, chr_surface, cancel_text);	//cancel
		Engine_string(SLIDER_TRK_X-left_align,			MUSIC_TRK_Y,	controls_font_id, chr_surface, music_text);		//music volume
		Engine_string(SLIDER_TRK_X-left_align,			SPEECH_TRK_Y,	controls_font_id, chr_surface, speech_text);	//speech volume
		Engine_string(SLIDER_TRK_X-left_align,			FX_TRK_Y,		controls_font_id, chr_surface, fx_text);		//fx volume
		Engine_string(SLIDER_TRK_X-left_align,			GRFX_TRK_Y,		controls_font_id, chr_surface, graphics_text);	//graphics quality
		Engine_string(SLIDER_TRK_X-left_align,			STEREO_Y+3,		controls_font_id, chr_surface, stereo_text);	//reverse stereo

//print buttons
		DrawSurface(&down_button_sprite[subtitle_state], down_button_surface[subtitle_state] );		//print subtitles button

		DrawSurface(&up_button_sprite[object_state], up_button_surface[object_state] );				//print object labels button

		DrawSurface(&button_sprite[restore_button_state], button_surface[restore_button_state] );	//print ok button

		DrawSurface(&can_button_sprite[can_button_state], can_button_surface[can_button_state] );	//print cancel button

		DrawSurface(&slab_sprite[0], slab_surface[0]);												//print sliders
		DrawSurface(&slab_sprite[1], slab_surface[1]);
		DrawSurface(&slab_sprite[2], slab_surface[2]);
		DrawSurface(&slab_sprite[3], slab_surface[3]);

		DrawSurface(&zup_button_sprite[music_mute_state], zup_button_surface[music_mute_state] );	//print mute buttons
		DrawSurface(&zdown_button_sprite[speech_mute_state], zdown_button_surface[speech_mute_state] );
		DrawSurface(&slab_sprite[fx_mute_state+4], slab_surface[fx_mute_state+4] );

		DrawSurface(&grfx_icon_sprite[grfxLevel], grfx_icon_surface[grfxLevel] );	//print the graphics level icon

		DrawSurface(&slab_sprite[6+stereo_state], slab_surface[6+stereo_state]);	// print reverse stereo button

//keep menu up too
		ProcessMenu();

//user can ESC quit
		if (KeyWaiting())
		{
			ReadKey(&c);	//kill the key we just pressed
			if	(c==27)	//ESC
			{
				ReadOptionSettings();	// Reset options to previous settings.
				break;
			}
		}

// check what if anything the mouse is touching
//mouse over ok button?
		if (Mouse_touching_button(OPT_OK_X,OPT_OK_Y,OPT_BUT_W,OPT_BUT_H))
			touching_restore_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	restore_button_state=0;
			touching_restore_button=0;
		}

//mouse over cancel button?
		if (Mouse_touching_button(OPT_CAN_X,OPT_CAN_Y,OPT_BUT_W,OPT_BUT_H))
			touching_can_button=1;	//mouse over button
		else	//not over so release even if pressed previously
		{	can_button_state=0;
			touching_can_button=0;
		}

//mouse over object label button?
		if (Mouse_touching_button(OBJ_LABEL_X,OBJ_LABEL_Y,OPT_BUT_W,OPT_BUT_H))
		{
			if (!lb_down)
				touching_object=1;	//mouse over button
		}
		else	//not over so release even if pressed previously
		{
			if (touching_object && lb_down && !dobject_state)
				object_state=!object_state;
			touching_object=0;
		}

//mouse over subtitles button?
		if (Mouse_touching_button(SUBTITLE_X,SUBTITLE_Y,OPT_BUT_W,OPT_BUT_H))
		{
			if (!lb_down)
				touching_subtitle=1;	//mouse over button
		}
		else	//not over so release even if pressed previously
		{
			if (touching_subtitle && lb_down && !dsubtitle_state)
				subtitle_state=!subtitle_state;
			touching_subtitle=0;
		}

//mouse over reverse stereo button?
		if (Mouse_touching_button(STEREO_X,STEREO_Y,OPT_BUT_W,OPT_BUT_H))
		{
			if (!lb_down)
				touching_reverse_stereo=1;	//mouse over button
		}
		else	//not over so release even if pressed previously
		{
			if (touching_reverse_stereo && lb_down && !dreverse_stereo_state)
				stereo_state=!stereo_state;
			touching_reverse_stereo=0;
		}

//mouse over music mute button?
		if (Mouse_touching_button(MUTE_X,MUSIC_TRK_Y-4,MUTE_W,MUTE_H))
		{
			if (!lb_down)
				touching_music_mute=1;	//mouse over button
		}
		else	//not over so release even if pressed previously
		{
			if (touching_music_mute && lb_down && !dmusic_mute_state)
				music_mute_state=!music_mute_state;
			touching_music_mute=0;
		}

//mouse over fx mute button?
		if (Mouse_touching_button(MUTE_X,FX_TRK_Y-4,MUTE_W,MUTE_H))
		{
			if (!lb_down)
				touching_fx_mute=1;	//mouse over button
		}
		else	//not over so release even if pressed previously
		{
			if (touching_fx_mute && lb_down && !dfx_mute_state)
				fx_mute_state=!fx_mute_state;
			touching_fx_mute=0;
		}

//mouse over speech mute button?
		if (Mouse_touching_button(MUTE_X,SPEECH_TRK_Y-4,MUTE_W,MUTE_H))
		{
			if (!lb_down)
				touching_speech_mute=1;	//mouse over button
		}
		else	//not over so release even if pressed previously
		{
			if (touching_speech_mute && lb_down && !dspeech_mute_state)
				speech_mute_state=!speech_mute_state;
			touching_speech_mute=0;
		}


//pressing on a button
		me = MouseEvent();	//get mouse event

		if	(me!=NULL)
		{
			if	(me->buttons&RD_LEFTBUTTONUP)
			{
				lb_down = 0;
				if	(touching_restore_button && restore_button_state)	// ok to settings
				{
					UpdateGraphicsLevel(safe_grfxLevel, grfxLevel);	// (James13jun97)

					g_sword2->_sound->MuteMusic(music_mute_state);					// Ensure all the levels are recorded correctly (Pete21Aug97)
					g_sword2->_sound->MuteSpeech(speech_mute_state);
					g_sword2->_sound->MuteFx(fx_mute_state);
					g_sword2->_sound->SetMusicVolume(music_target);
					g_sword2->_sound->SetSpeechVolume(speech_target);
					g_sword2->_sound->SetFxVolume(fx_target);

					subtitles = subtitle_state;    // Save object label and subtitle settings
					pointerTextSelected = object_state;
					speechSelected      = !speech_mute_state;

					if (stereo_state != stereoReversed)
						g_sword2->_sound->ReverseStereo();

					stereoReversed      = stereo_state;
					WriteOptionSettings();
					break;
				}

				if	(touching_can_button && can_button_state) // cancel, so restore old settings
				{
					ReadOptionSettings();
					break;
				}

				if (touching_object && dobject_state)
					dobject_state = object_state=0;  // if the button was in now let it out

				if (touching_subtitle && dsubtitle_state)
					subtitle_state = dsubtitle_state = 0;  // if the button was in now let it out

				if (touching_reverse_stereo && dreverse_stereo_state)
					dreverse_stereo_state = stereo_state = 0;  // if the button was in now let it out

				if (touching_music_mute && dmusic_mute_state) {
					music_mute_state = dmusic_mute_state = 0;  // if the button was in now let it out
					g_sword2->_sound->MuteMusic(0);
				}

				if (touching_fx_mute && dfx_mute_state) {
					fx_mute_state = dfx_mute_state = 0;  // if the button was in now let it out
					g_sword2->_sound->MuteFx(0);
				}

				if (touching_speech_mute && dspeech_mute_state) {
					speech_mute_state = dspeech_mute_state = 0;  // if the button was in now let it out
					g_sword2->_sound->MuteSpeech(0);
				}

				// Stop tracking any sliders
				music_tracking = fx_tracking = speech_tracking = grfx_tracking = 0;
			}
	
			else if (me->buttons&RD_LEFTBUTTONDOWN)	//there's a mouse event to be processed
			{
				lb_down = 1;
				if	(touching_restore_button)
					restore_button_state=1;
	
				if	(touching_can_button)
					can_button_state=1;

				if (touching_object)
				{
					if (object_state)  // push in the button if it's out
						dobject_state = 1;
					else
						object_state=!object_state;
				}

				if (touching_subtitle)
				{
					if (subtitle_state)
						dsubtitle_state = 1;
					else
						subtitle_state=!subtitle_state;
				}

				if (touching_reverse_stereo)
				{
					if (stereo_state)  // push in the button if it's out
						dreverse_stereo_state = 1;
					else
						stereo_state = !stereo_state;
				}


				if (touching_music_mute)
				{
					if (music_mute_state)
						dmusic_mute_state = 1;
					else
					{
						music_mute_state = 1;
						g_sword2->_sound->MuteMusic(1);
					}
				}

				if (touching_fx_mute)
				{
					if (fx_mute_state)
						dfx_mute_state = 1;
					else
					{
						fx_mute_state=1;
						g_sword2->_sound->MuteFx(1);
					}
				}

				if (touching_speech_mute)
				{
					if (speech_mute_state)
						dspeech_mute_state = 1;
					else
					{
						speech_mute_state=1;
						g_sword2->_sound->MuteSpeech(1);
					}
				}

				if (Mouse_touching_button(SLIDER_TRK_X,MUSIC_TRK_Y,SLIDER_TRK_W+SLIDER_W,SLIDER_TRK_H))
				{
					if (music_mute_state)
					{
						music_mute_state = 0;
						g_sword2->_sound->MuteMusic(0);
					}

					if (mousex>(slab_sprite[0].x+SLIDER_W))
					{
						if (music_target<musicVolume)
							music_target = musicVolume;
						music_target++;
						if (music_target>15)
							music_target = 15;
					}
					else if (mousex<slab_sprite[0].x)
					{
						if (music_target>musicVolume)
							music_target = musicVolume;
						music_target--;
						if (music_target>15)
							music_target = 0;
					} else
						music_tracking = 1;
				}

				if (Mouse_touching_button(SLIDER_TRK_X,SPEECH_TRK_Y,SLIDER_TRK_W+SLIDER_W,SLIDER_TRK_H))
				{
					if (speech_mute_state)
					{
						speech_mute_state = 0;
						g_sword2->_sound->MuteSpeech(0);
					}

					if (mousex>(slab_sprite[1].x+SLIDER_W))
					{
						if (speech_target<speechVolume)
							speech_target = speechVolume;
						speech_target++;
						if (speech_target>14)
							speech_target = 14;
					}
					else if (mousex<slab_sprite[1].x)
					{
						if (speech_target>speechVolume)
							speech_target = speechVolume;
						speech_target--;
						if (speech_target>14)
							speech_target = 0;
					} else
						speech_tracking = 1;
				}

				if (Mouse_touching_button(SLIDER_TRK_X,FX_TRK_Y,SLIDER_TRK_W+SLIDER_W,SLIDER_TRK_H))
				{
					if (fx_mute_state)
					{
						fx_mute_state = 0;
						g_sword2->_sound->MuteFx(0);
					}

					if (mousex>(slab_sprite[2].x+SLIDER_W))
					{
						if (fx_target<fxVolume)
							fx_target = fxVolume;
						fx_target++;
						if (fx_target>14)
							fx_target = 14;
					}
					else if (mousex<slab_sprite[2].x)
					{
						if (fx_target>fxVolume)
							fx_target = fxVolume;
						fx_target--;
						if (fx_target>14)
							fx_target = 0;
					}
					 else
						 fx_tracking = 1;

					 fx_mute_state = 0;
				}

				if (Mouse_touching_button(SLIDER_TRK_X,GRFX_TRK_Y,SLIDER_TRK_W+SLIDER_W,SLIDER_TRK_H))
				{
					if (mousex>(slab_sprite[3].x+SLIDER_W))
					{
						if (grfx_target<grfxLevel)
							grfx_target = grfxLevel;
						grfx_target++;
						if (grfx_target>3)
							grfx_target = 3;
					}
					else if (mousex<slab_sprite[3].x)
					{
						if (grfx_target>grfxLevel)
							grfx_target = grfxLevel;
						grfx_target--;
						if (grfx_target>3)
							grfx_target = 0;
					}
					 else
						 grfx_tracking = 1;
				}
			}
		}


		if	(!first)
		{
			first++;
			SetFullPalette(CONTROL_PANEL_PALETTE);	// see Build_display.cpp (James17jun97)
		}
	}


	Kill_option_surfaces();

	return;	//just return to game
}

//-----------------------------------------------------------------------------------------------------------------------
void UpdateGraphicsLevel(uint8 oldLevel, uint8 newLevel)	// (James13jun97)
{

	switch (oldLevel)			// Set the graphics level
	{
		//-------------------------------
		case 0:		// lowest setting: h/w only; no graphics fx
			switch(newLevel)
			{
				case 0:
					break;

				case 1:
					ClearBltFx();
					ClearShadowFx();
					CloseBackgroundLayer();
					break;

				case 2:
					ClearBltFx();
					CloseBackgroundLayer();
					break;

				case 3:	// same as case 2 until case 2 has edge-blending inactivated
					CloseBackgroundLayer();
					break;
			}
			break;
		//-------------------------------
		case 1:		// medium-low setting: s/w transparency-blending
			switch(newLevel)
			{
				case 1:
					break;

				case 0:
					SetUpBackgroundLayers();	// InitialiseBackgroundLayer for each layer! (see layers.cpp)
					break;

				case 2:
					SetShadowFx();
					break;

				case 3:	// same as case 2 until case 2 has edge-blending inactivated
					SetBltFx();
					break;
			}
			break;
		//-------------------------------
		case 2:		// medium-high setting: s/w transparency-blending + shading
			switch(newLevel)
			{
				case 2:
					break;

				case 3:	// same as case 2 until case 2 has edge-blending inactivated
					SetBltFx();
					break;

				case 1:
					ClearShadowFx();
					break;

				case 0:
					SetUpBackgroundLayers();	// InitialiseBackgroundLayer for each layer! (see layers.cpp)
					break;
			}
			break;
		//-------------------------------
		case 3:		// highest setting: s/w transparency-blending + shading + edge-blending (& improved stretching)
			switch(newLevel)
			{
				case 2:
					ClearBltFx();
					break;

				case 3:	// same as case 2 until case 2 has edge-blending inactivated
					break;

				case 1:
					ClearBltFx();
					ClearShadowFx();
					break;

				case 0:
					SetUpBackgroundLayers();	// InitialiseBackgroundLayer for each layer! (see layers.cpp)
					break;
			}
			break;
		//-------------------------------
	}

	// update our global variable - which needs to be checked when dimming the palette
	// in PauseGame() in sword2.cpp (since palette-matching cannot be done with dimmed palette
	// so we turn down one notch while dimmed, if at top level)
	current_graphics_level = newLevel;
}
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

