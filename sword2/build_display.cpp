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

//------------------------------------------------------------------------------------
// BUILD_DISPLAY.CPP	like the old spr_engi but slightly more aptly named
//------------------------------------------------------------------------------------
//#include <mmsystem.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//#include <windows.h>
//#include <windowsx.h>

#include "driver/driver96.h"
#include "build_display.h"
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "events.h"
#include "function.h"
#include "header.h"
#include "interpreter.h"
#include "layers.h"
#include "logic.h"
#include "maketext.h"
#include "memory.h"
#include "mouse.h"
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "router.h"
#include "save_rest.h"
#include "scroll.h"
#include "sword2.h"

//------------------------------------------------------------------------------------

buildit	bgp0_list[MAX_bgp0_sprites];
buildit	bgp1_list[MAX_bgp1_sprites];
buildit	back_list[MAX_back_sprites];
buildit	sort_list[MAX_sort_sprites];
buildit	fore_list[MAX_fore_sprites];
buildit	fgp0_list[MAX_fgp0_sprites];
buildit	fgp1_list[MAX_fgp1_sprites];

uint16	sort_order[MAX_sort_sprites];	//holds the order of the sort list - i.e the list stays static and we sort this

uint32	cur_bgp0;
uint32	cur_bgp1;
uint32	cur_back;
uint32	cur_sort;
uint32	cur_fore;
uint32	cur_fgp0;
uint32	cur_fgp1;

#ifdef _DEBUG
uint32 largest_layer_area=0;	// should be reset to zero at start of each screen change
uint32 largest_sprite_area=0;	// - " -
char largest_layer_info[128]	= {"largest layer:  none registered"};
char largest_sprite_info[128]	= {"largest sprite: none registered"};
#endif

//------------------------------------------------------------------------------------
// last palette used - so that we can restore the correct one after a pause (which dims the screen)
// - and it's not always the main screen palette that we want, eg. during the eclipse

// This flag gets set in Start_new_palette() and SetFullPalette()

uint32 lastPaletteRes=0;

//------------------------------------------------------------------------------------
// 'frames per second' counting stuff

uint32	fps=0;
uint32	cycleTime=0;
uint32	frameCount=0;
extern uint32 mouse_status;		// So I know if the control Panel can be activated - CJR 1-5-97

//------------------------------------------------------------------------------------
// function prototypes not needed externally

void	Start_new_palette(void);	//Tony25Sept96

void	Register_frame(int32 *params, buildit *build_unit);	// (1nov96JEL)
void	Process_image(buildit	*frame);
void	Process_layer(uint32	layer_number);	//Tony24Sept96
void	Sort_the_sort_list(void);	//Tony18Sept96

void	Send_back_par0_frames(void);	//James23Jan97
void	Send_back_par1_frames(void);	//James23Jan97
void	Send_back_frames(void);			//Tony23Sept96
void	Send_sort_frames(void);
void	Send_fore_frames(void);
void	Send_fore_par0_frames(void);	//James23Jan97
void	Send_fore_par1_frames(void);	//James23Jan97


//------------------------------------------------------------------------------------
//
// PC Build_display
//
//------------------------------------------------------------------------------------
void	Build_display(void)	//Tony21Sept96
{
	BOOL		end;
	uint8		pal[12]={0,0,0,0,0,0,0,0,0,255,0,0};
	uint8		*file;
	_multiScreenHeader *screenLayerTable;

#ifdef _DEBUG	// only used by console
	_spriteInfo	spriteInfo;
	uint32		rv;	// drivers error return value
#endif



	if	((!console_status)&&(this_screen.new_palette))
	{
		Start_new_palette();										// start the layer palette fading up

		#ifdef _DEBUG			// (James23jun97)
		largest_layer_area=0;	// should be reset to zero at start of each screen change
		largest_sprite_area=0;	// - " -
		#endif
	}



	if	((!console_status)&&(this_screen.background_layer_id))		// there is a valid screen to run
	{
		SetScrollTarget(this_screen.scroll_offset_x, this_screen.scroll_offset_y);	// set the scroll position

		AnimateMouse();												// increment the mouse frame

		StartRenderCycle();

		while (1)	// START OF RENDER CYCLE
		{
			//----------------------------------------------------
			// clear the back buffer, before building up the new screen
			// from the back forwards
	
			EraseBackBuffer();

			//----------------------------------------------------
			// first background parallax + related anims

			file = res_man.Res_open(this_screen.background_layer_id);		// open the screen resource
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->bg_parallax[0])
			{
				RenderParallax(FetchBackgroundParallaxLayer(file, 0), 0);
	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource before cacheing the sprites
				Send_back_par0_frames();
			}
			else
 	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource

 			//----------------------------------------------------
			// second background parallax + related anims

			file = res_man.Res_open(this_screen.background_layer_id);		// open the screen resource
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->bg_parallax[1])
			{
				RenderParallax(FetchBackgroundParallaxLayer(file, 1), 1);
	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource before cacheing the sprites
				Send_back_par1_frames();
			}
			else
 	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource

 			//----------------------------------------------------
			// normal backround layer (just the one!)

			file = res_man.Res_open(this_screen.background_layer_id);		// open the screen resource
			RenderParallax(FetchBackgroundLayer(file), 2);
			res_man.Res_close(this_screen.background_layer_id);				// release the screen resource

 			//----------------------------------------------------
			// sprites & layers

			Send_back_frames();		// background sprites
			Sort_the_sort_list();
			Send_sort_frames();		// sorted sprites & layers
			Send_fore_frames();		// foreground sprites

			//----------------------------------------------------
			// first foreground parallax + related anims

			file = res_man.Res_open(this_screen.background_layer_id);		// open the screen resource
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->fg_parallax[0])
			{
				RenderParallax(FetchForegroundParallaxLayer(file, 0), 3);
	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource before cacheing the sprites
				Send_fore_par0_frames();
			}
			else
 	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource

 			//----------------------------------------------------
			// second foreground parallax + related anims

			file = res_man.Res_open(this_screen.background_layer_id);		// open the screen resource
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->fg_parallax[1])
			{
				RenderParallax(FetchForegroundParallaxLayer(file, 1), 4);
	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource before cacheing the sprites
				Send_fore_par1_frames();
			}
			else
 	 			res_man.Res_close(this_screen.background_layer_id);			// release the screen resource

 			//----------------------------------------------------
			// walkgrid, mouse & player markers & mouse area rectangle

			Draw_debug_graphics();	// JAMES (08apr97)

 			//----------------------------------------------------
			// text blocks

			Print_text_blocs();	// speech blocks and headup debug text

 			//----------------------------------------------------
			// menu bar & icons

			ProcessMenu();

 			//----------------------------------------------------
			// ready - blit to screen

			CopyScreenBuffer();
			FlipScreens();

 			//----------------------------------------------------
			// update our fps reading

			frameCount += 1;
			if (timeGetTime() > cycleTime)
			{
				fps = frameCount;
				frameCount = 0;
				cycleTime = timeGetTime()+1000;
			}
 			//----------------------------------------------------
			// check if we've got time to render the screen again this cycle
			// (so drivers can smooth out the scrolling in between normal game cycles)

			EndRenderCycle(&end);

			if (end)	// if we haven't got time to render again this cycle, drop out of 'render cycle' while-loop
				break;

			if (ServiceWindows() == RDERR_APPCLOSED)	// if the game is being shut down, drop out
				break;

			//----------------------------------------------------
		}	// END OF RENDER CYCLE


	}
#ifdef _DEBUG
	else if (console_status)
	{
		spriteInfo.x			= 0;
		spriteInfo.y			= con_y;
		spriteInfo.w			= con_width;
		spriteInfo.h			= con_depth;
		spriteInfo.scale		= 0;
		spriteInfo.scaledWidth	= 0;
		spriteInfo.scaledHeight	= 0;
		spriteInfo.type			= RDSPR_DISPLAYALIGN+RDSPR_NOCOMPRESSION;	// no compression!
		spriteInfo.blend		= 0;
		spriteInfo.data			= console_sprite->ad;
		spriteInfo.colourTable	= 0;


		rv = DrawSprite( &spriteInfo );
		if (rv)
			ExitWithReport("Driver Error %.8x (drawing console) [%s line %u]", rv, __FILE__, __LINE__);

		CopyScreenBuffer();
		FlipScreens();
	}
	else
	{
		StartConsole();
		SetPalette(0, 3, pal, RDPAL_INSTANT);	//force the palette
		Print_to_console("no valid screen?");
	}
#endif	// _DEBUG

}

//------------------------------------------------------------------------------------
//
// Fades down and displays a message on the screen for time seconds
//
void DisplayMsg( uint8 *text, int time )	// Chris 15May97
{
	mem *text_spr;
	_frameHeader *frame;
	_spriteInfo  spriteInfo;
	bool		 done = false;
	_palEntry	 pal[256];
	_palEntry	 oldPal[256];
	int16		 oldY;
	int16		 oldX;
	uint32		rv;	// drivers error return value

	warning("DisplayMsg: %s", (char *) text);
	

	if (GetFadeStatus() != RDFADE_BLACK)
	{
		FadeDown((float) 0.75);

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
		while(GetFadeStatus()==RDFADE_DOWN);
	}

	Set_mouse(NULL);
	Set_luggage(0);	//tw28Aug

	EraseBackBuffer();				// for hardware rendering
	EraseSoftwareScreenBuffer();	// for software rendering

	text_spr = MakeTextSprite( text, 640, 187, speech_font_id );

	frame = (_frameHeader*) text_spr->ad;

	spriteInfo.x			= screenWide/2 - frame->width/2;
	if (!time)
		spriteInfo.y		= screenDeep/2 - frame->height/2 - RDMENU_MENUDEEP;
	else
		spriteInfo.y		= 400 - frame->height;
	spriteInfo.w			= frame->width;
	spriteInfo.h			= frame->height;
	spriteInfo.scale		= 0;
	spriteInfo.scaledWidth	= 0;
	spriteInfo.scaledHeight	= 0;
	spriteInfo.type			= RDSPR_DISPLAYALIGN+RDSPR_NOCOMPRESSION+RDSPR_TRANS;
	spriteInfo.blend		= 0;
	spriteInfo.data			= text_spr->ad + sizeof(_frameHeader);
	spriteInfo.colourTable	= 0;
	oldX					= spriteInfo.x;
	oldY					= spriteInfo.y;


	rv = DrawSprite( &spriteInfo );
	if (rv)
		ExitWithReport("Driver Error %.8x (in DisplayMsg) [%s line %u]", rv, __FILE__, __LINE__);



	spriteInfo.x = oldX;
	spriteInfo.y = oldY;

	memcpy((char *) oldPal, (char *) palCopy, 256*sizeof(_palEntry));

	memset(pal, 0, 256*sizeof(_palEntry));
	pal[187].red   = 255;
	pal[187].green = 255;
	pal[187].blue  = 255;
	SetPalette(0, 256, (uint8 *) pal, RDPAL_FADE);

	CopyScreenBuffer();
	FlipScreens();

	FadeUp((float)0.75);

	Free_mem(text_spr);

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
	while(GetFadeStatus()==RDFADE_UP);

	DWORD targetTime = timeGetTime() + (time*1000);

	while(timeGetTime() < targetTime)
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

		EraseBackBuffer();				// for hardware rendering
		EraseSoftwareScreenBuffer();	// for software rendering

		rv = DrawSprite( &spriteInfo );		// Keep the message there even when the user task swaps.
		if (rv)
			ExitWithReport("Driver Error %.8x (in DisplayMsg) [%s line %u]", rv, __FILE__, __LINE__);

		spriteInfo.y = oldY;			// Drivers change the y co-ordinate, don't know why...
		spriteInfo.x = oldX;
		CopyScreenBuffer();
		FlipScreens();
	}

	SetPalette(0, 256, (uint8 *) oldPal, RDPAL_FADE);
}

//------------------------------------------------------------------------------------
//
// Fades message down and removes it, fading up again afterwards
//
void RemoveMsg( void )		// Chris 15May97
{
	FadeDown((float)0.75);

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
	while(GetFadeStatus()==RDFADE_DOWN);

	EraseBackBuffer();				// for hardware rendering
	EraseSoftwareScreenBuffer();	// for software rendering
	CopyScreenBuffer();
	FlipScreens();

//	FadeUp((float)0.75);	
//	removed by JEL (08oct97) to prevent "eye" smacker corruption when restarting game from CD2
//	and also to prevent palette flicker when restoring game to a different CD
//	- since the "insert CD" message uses this routine to clean up!
							
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void	Send_back_par0_frames(void)	//James23Jan97
{
	uint32 j;

	for (j=0; j<cur_bgp0; j++)			// could be none at all - theoretically at least
	{
		Process_image(&bgp0_list[j]);	// frame attached to 1st background parallax
	}
}
//------------------------------------------------------------------------------------
void	Send_back_par1_frames(void)	//James23Jan97
{
	uint32 j;

	for (j=0; j<cur_bgp1; j++)			// could be none at all - theoretically at least
	{
		Process_image(&bgp1_list[j]);	// frame attached to 1nd background parallax
	}
}
//------------------------------------------------------------------------------------
void	Send_back_frames(void)	//Tony23Sept96
{
	uint32 j;

	for (j=0; j<cur_back; j++)			// could be none at all - theoretically at least
	{
		Process_image(&back_list[j]);
	}
}
//------------------------------------------------------------------------------------
void	Send_sort_frames(void)	//Tony23Sept96
{
	//send the sort frames for printing - layers, shrinkers & normal flat sprites
	uint32 j;

	for (j=0; j<cur_sort; j++)			// could be none at all - theoretically at least
	{
		if (sort_list[sort_order[j]].layer_number)	//its a layer  - minus 1 for true layer number
			Process_layer(sort_list[sort_order[j]].layer_number-1);	//we need to know from the buildit because the layers will have been sorted in random order
		else
			Process_image(&sort_list[sort_order[j]]);	// sprite
	}
}
//------------------------------------------------------------------------------------
void	Send_fore_frames(void)	//Tony23Sept96
{
	uint32 j;

	for (j=0; j<cur_fore; j++)			// could be none at all - theoretically at least
	{
		Process_image(&fore_list[j]);
	}
}
//------------------------------------------------------------------------------------
void	Send_fore_par0_frames(void)	//James23Jan97
{
	uint32 j;

	for (j=0; j<cur_fgp0; j++)			// could be none at all - theoretically at least
	{
		Process_image(&fgp0_list[j]);	// frame attached to 1st foreground parallax
	}
}
//------------------------------------------------------------------------------------
void	Send_fore_par1_frames(void)	//James23Jan97
{
	uint32 j;

	for (j=0; j<cur_fgp1; j++)			// could be none at all - theoretically at least
	{
		Process_image(&fgp1_list[j]);	// frame attached to 2nd foreground parallax
	}
}
//------------------------------------------------------------------------------------
void	Process_layer(uint32	layer_number)	//Tony24Sept96
{
	uint8			*file;
	_layerHeader	*layer_head;
 	_spriteInfo		spriteInfo;
	uint32 rv;

	#ifdef _DEBUG
	uint32 current_layer_area=0;
	#endif


	file = res_man.Res_open(this_screen.background_layer_id);	// file points to 1st byte in the layer file

	layer_head = FetchLayerHeader(file,layer_number);			// point to layer header


	spriteInfo.x			= layer_head->x;
	spriteInfo.y			= layer_head->y;
	spriteInfo.w			= layer_head->width;
	spriteInfo.scale		= 0;
	spriteInfo.scaledWidth	= 0;
	spriteInfo.scaledHeight	= 0;
	spriteInfo.h			= layer_head->height;
	spriteInfo.type			= RDSPR_TRANS + RDSPR_RLE256FAST;
	spriteInfo.blend		= 0;
	spriteInfo.data			= file+sizeof(_standardHeader)+layer_head->offset;
	spriteInfo.colourTable	= 0;


	//------------------------------------------
	// check for largest layer for debug info
	#ifdef _DEBUG
	current_layer_area = layer_head->width * layer_head->height;

	if (current_layer_area > largest_layer_area)
	{
		largest_layer_area = current_layer_area;
		sprintf (largest_layer_info, "largest layer:  %s layer(%d) is %dx%d", FetchObjectName(this_screen.background_layer_id), layer_number, layer_head->width, layer_head->height);
	}
	#endif
	//------------------------------------------

	rv = DrawSprite( &spriteInfo );
	if (rv)
		ExitWithReport("Driver Error %.8x in Process_layer(%d) [%s line %u]", rv, layer_number, __FILE__, __LINE__);


	res_man.Res_close(this_screen.background_layer_id);

}
//------------------------------------------------------------------------------------
void	Process_image(buildit *build_unit)	// (5nov96 JEL)
{
	uint8			*file, *colTablePtr=NULL;
	_animHeader		*anim_head;
	_frameHeader	*frame_head;
	_cdtEntry		*cdt_entry;
	_spriteInfo		spriteInfo;
	uint32			spriteType;
	uint32			rv;

	#ifdef _DEBUG
	uint32 current_sprite_area=0;
	#endif

	file = res_man.Res_open(build_unit->anim_resource);	// open anim resource file & point to base

	anim_head	= FetchAnimHeader( file );
	cdt_entry	= FetchCdtEntry( file, build_unit->anim_pc );
	frame_head	= FetchFrameHeader( file, build_unit->anim_pc );


	spriteType = RDSPR_TRANS;		// so that 0-colour is transparent

	if (anim_head->blend)
		spriteType += RDSPR_BLEND;

	if ((cdt_entry->frameType) & FRAME_FLIPPED)	// if the frame is to be flipped (only really applicable to frames using offsets)
		spriteType += RDSPR_FLIP;

	if ((cdt_entry->frameType) & FRAME_256_FAST)
	{
		if ((build_unit->scale)||(anim_head->blend)||(build_unit->shadingFlag))	// scaling, shading & blending don't work with RLE256FAST
			spriteType += RDSPR_RLE256;		// but the same compression can be decompressed using the RLE256 routines!
		else
			spriteType += RDSPR_RLE256FAST;
	}
	else
	{
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
	}

	if (build_unit->shadingFlag==1)		// if we want this frame to be affected by the shading mask
		spriteType += RDSPR_SHADOW;		// add the status bit

	spriteInfo.x			= build_unit->x;
	spriteInfo.y			= build_unit->y;
	spriteInfo.w			= frame_head->width;
	spriteInfo.h			= frame_head->height;
	spriteInfo.scale		= build_unit->scale;
	spriteInfo.scaledWidth	= build_unit->scaled_width;
	spriteInfo.scaledHeight	= build_unit->scaled_height;
	spriteInfo.type			= spriteType;
	spriteInfo.blend		= anim_head->blend;
	spriteInfo.data			= (uint8*)(frame_head+1);	// points to just after frame header, ie. start of sprite data
	spriteInfo.colourTable	= colTablePtr;


	//------------------------------------------
	// check for largest layer for debug info
	#ifdef _DEBUG
	current_sprite_area = frame_head->width * frame_head->height;

	if (current_sprite_area > largest_sprite_area)
	{
		largest_sprite_area = current_sprite_area;
		sprintf (largest_sprite_info, "largest sprite: %s frame(%d) is %dx%d", FetchObjectName(build_unit->anim_resource), build_unit->anim_pc, frame_head->width, frame_head->height);
	}
	#endif
	//------------------------------------------


	//-----------------------------------------------------------
	#ifdef _DEBUG
	if (SYSTEM_TESTING_ANIMS)	// see anims.cpp
	{
		if ((spriteInfo.x + spriteInfo.scaledWidth) >= 639)		// bring the anim into the visible screen
			spriteInfo.x = 639-spriteInfo.scaledWidth;			// but leave extra pixel at edge for box

		if ((spriteInfo.y + spriteInfo.scaledHeight) >= 399)
			spriteInfo.y = 399-spriteInfo.scaledHeight;

		if (spriteInfo.x < 1)
			spriteInfo.x = 1;

		if (spriteInfo.y < 1)
			spriteInfo.y = 1;

		rect_x1 = spriteInfo.x - 1;	// create box to surround sprite - just outside sprite box
		rect_y1 = spriteInfo.y - 1;
		rect_x2 = spriteInfo.x + spriteInfo.scaledWidth;
		rect_y2 = spriteInfo.y + spriteInfo.scaledHeight;
	}
	#endif
	//-----------------------------------------------------------

	//--------------------------------------------------
//	#ifdef _DEBUG
//	if (frame_head->width <= 1)
//	{
//		Zdebug(8,"WARNING: 1-pixel-wide frame found in %s (%d)", FetchObjectName(build_unit->anim_resource), build_unit->anim_resource);
//	}
//	#endif
 	//--------------------------------------------------

	rv = DrawSprite( &spriteInfo );
	if (rv)
		ExitWithReport("Driver Error %.8x with sprite %s (%d) in Process_image [%s line %u]", rv, FetchObjectName(build_unit->anim_resource), build_unit->anim_resource, __FILE__, __LINE__);


	res_man.Res_close(build_unit->anim_resource);	// release the anim resource

}
//------------------------------------------------------------------------------------
void	Reset_render_lists(void)	//Tony18Sept96
{
//reset the sort lists - do this before a logic loop
//takes into account the fact that the start of the list is pre-built with the special sortable layers

	uint32	j;

	cur_bgp0=0;
	cur_bgp1=0;
	cur_back=0;
	cur_sort=this_screen.number_of_layers;	//beginning of sort list is setup with the special sort layers
	cur_fore=0;
	cur_fgp0=0;
	cur_fgp1=0;


	if	(cur_sort)	//there are some layers - so rebuild the sort order positioning
		for	(j=0;j<cur_sort;j++)
			sort_order[j]=j;	//rebuild the order list
}
//------------------------------------------------------------------------------------
void	Sort_the_sort_list(void)	//Tony18Sept96
{
//sort the list

	uint16	i,j,swap;


//sort the list

	if	(cur_sort>1)	//cannot bubble sort 0 or 1 items!
		for	(i=0; i<cur_sort-1; i++)
			for	(j=0; j<cur_sort-1; j++)
				if	(sort_list[sort_order[j]].sort_y > sort_list[sort_order[j+1]].sort_y)		//this > next then swap
				{	swap=sort_order[j];
					sort_order[j]=sort_order[j+1];
					sort_order[j+1]=swap;
				}
}
//------------------------------------------------------------------------------------
void Register_frame(int32 *params, buildit *build_unit)	// (1nov96JEL)
{
	// params:	0 pointer to mouse structure or NULL for no write to mouse list (non-zero means write sprite-shape to mouse list)
	//			1 pointer to graphic structure
	//			2 pointer to mega structure

	Object_mega		*ob_mega;
	Object_graphic	*ob_graph;
	Object_mouse	*ob_mouse;
	uint8			*file;
	_frameHeader	*frame_head;
	_animHeader		*anim_head;
	_cdtEntry		*cdt_entry;
	int				scale=0;


	//-------------------------------------------	
	// open animation file & set up the necessary pointers

	ob_graph	= (Object_graphic *) params[1];

	#ifdef _DEBUG
	if (ob_graph->anim_resource == 0)
		Con_fatal_error("ERROR: %s(%d) has no anim resource in Register_frame [line=%d file=%s]", FetchObjectName(ID), ID, __LINE__, __FILE__);
	#endif

	file		= res_man.Res_open(ob_graph->anim_resource);

	anim_head	= FetchAnimHeader( file );
	cdt_entry	= FetchCdtEntry( file, ob_graph->anim_pc );
	frame_head	= FetchFrameHeader( file, ob_graph->anim_pc );


	#ifdef _DEBUG
	if (ID == CUR_PLAYER_ID)	// update player graphic details for on-screen debug info
	{
		playerGraphic.type			= ob_graph->type;
		playerGraphic.anim_resource	= ob_graph->anim_resource;
		playerGraphic.anim_pc		= ob_graph->anim_pc+1;	// counting 1st frame as 'frame 1'
		player_graphic_no_frames	= anim_head->noAnimFrames;
	}
	#endif

	//-------------------------------------------	
	// fill in the buildit structure for this frame

 	build_unit->anim_resource	= ob_graph->anim_resource;	//retrieve the resource
	build_unit->anim_pc			= ob_graph->anim_pc;	//retrieve the frame
	build_unit->layer_number	= 0;	//not a layer

	if (ob_graph->type & SHADED_SPRITE)
		build_unit->shadingFlag = 1;	// affected by shading mask
	else
		build_unit->shadingFlag = 0;	// not shaded

	//-------------------------------------------	
	// check if this frame has offsets ie. this is a scalable mega frame
	if ((cdt_entry->frameType) & FRAME_OFFSET)
	{
		ob_mega = (Object_mega *) params[2];	// param 2 is pointer to mega structure

		// calc scale at which to print the sprite, based on feet y-coord & scaling constants (NB. 'scale' is actually 256*true_scale, to maintain accuracy)
		scale = (ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b)/256;	// Ay+B gives 256*scale ie. 256*256*true_scale for even better accuracy, ie. scale = (Ay+B)/256
		// calc final render coordinates (top-left of sprite), based on feet coords & scaled offsets
		build_unit->x = ob_mega->feet_x + (cdt_entry->x * scale)/256;	// add scaled offsets to feet coords
		build_unit->y = ob_mega->feet_y + (cdt_entry->y * scale)/256;

		// work out new width and height
		build_unit->scaled_width	= ((scale * frame_head->width) / 256);	// always divide by 256 after everything else, to maintain accurary
		build_unit->scaled_height	= ((scale * frame_head->height) / 256);
	}
	else	// it's a non-scaling anim
	{
		// get render coords for sprite, from cdt
		build_unit->x				= cdt_entry->x;	//retrieve the x
		build_unit->y				= cdt_entry->y;	//retrieve the y

 		// get width and height
		build_unit->scaled_width	= frame_head->width;
		build_unit->scaled_height	= frame_head->height;
	}
	//-------------------------------------------	

	build_unit->scale = scale;	// either 0 or required scale, depending on whether 'scale' computed

	// calc the bottom y-coord for sorting purposes
	build_unit->sort_y	= build_unit->y + build_unit->scaled_height - 1;
	//-------------------------------------------	

	if (params[0])	// passed a mouse structure, so add to the mouse_list
	{
		ob_mouse = (Object_mouse *) params[0];

		if (ob_mouse->pointer)	// only if 'pointer' isn't NULL (James13feb97)
		{
			#ifdef _DEBUG
			if	(cur_mouse==TOTAL_mouse_list)
				Con_fatal_error("ERROR: mouse_list full [%s line %u]",__FILE__,__LINE__);
			#endif

			mouse_list[cur_mouse].x1			= build_unit->x;
			mouse_list[cur_mouse].y1			= build_unit->y;
			mouse_list[cur_mouse].x2			= build_unit->x + build_unit->scaled_width;
			mouse_list[cur_mouse].y2			= build_unit->y + build_unit->scaled_height;

 			mouse_list[cur_mouse].priority		= ob_mouse->priority;
			mouse_list[cur_mouse].pointer		= ob_mouse->pointer;

			//-----------------------------------------------
			// (James17jun97)
			// check if pointer text field is set due to previous object using this slot (ie. not correct for this one)
			if ((mouse_list[cur_mouse].pointer_text) && (mouse_list[cur_mouse].id != (int32)ID))	// if 'pointer_text' field is set, but the 'id' field isn't same is current id
				mouse_list[cur_mouse].pointer_text=0;												// then we don't want this "left over" pointer text
			//-----------------------------------------------


			mouse_list[cur_mouse].id			= ID;

			mouse_list[cur_mouse].anim_resource	= 0;	// not using sprite as detection mask
			mouse_list[cur_mouse].anim_pc		= 0;

			cur_mouse++;
		}
	}
 	//-------------------------------------------	

	res_man.Res_close(ob_graph->anim_resource);	// close animation file
}
//------------------------------------------------------------------------------------

int32 FN_register_frame(int32 *params)	// (27nov96 JEL)
{
	//this call would be made from an objects service script 0

	// params:	0 pointer to mouse structure or NULL for no write to mouse list (non-zero means write sprite-shape to mouse list)
	//			1 pointer to graphic structure
	//			2 pointer to mega structure or NULL if not a mega

	Object_graphic	*ob_graph = (Object_graphic *) params[1];


	switch (ob_graph->type & 0x0000ffff)	// check low word for sprite type
	{
		//---------------
		case BGP0_SPRITE:
		{
			#ifdef _DEBUG
			if (cur_bgp0==MAX_bgp0_sprites)
				Con_fatal_error("ERROR: bgp0_list full in FN_register_frame [line=%d file=%s]",__LINE__,__FILE__);
			#endif

			Register_frame(params, &bgp0_list[cur_bgp0]);
			cur_bgp0++;
		}
		break;
		//---------------
		case BGP1_SPRITE:
		{
			#ifdef _DEBUG
			if (cur_bgp1==MAX_bgp1_sprites)
				Con_fatal_error("ERROR: bgp1_list full in FN_register_frame [line=%d file=%s]",__LINE__,__FILE__);
			#endif

			Register_frame(params, &bgp1_list[cur_bgp1]);
			cur_bgp1++;
		}
		break;
		//---------------
		case BACK_SPRITE:
		{
			#ifdef _DEBUG
			if (cur_back==MAX_back_sprites)
				Con_fatal_error("ERROR: back_list full in FN_register_frame [line=%d file=%s]",__LINE__,__FILE__);
			#endif

			Register_frame(params, &back_list[cur_back]);
			cur_back++;
		}
		break;
		//---------------
		case SORT_SPRITE:
		{
			#ifdef _DEBUG
			if (cur_sort==MAX_sort_sprites)
				Con_fatal_error("ERROR: sort_list full in FN_register_frame [line=%d file=%s]",__LINE__,__FILE__);
			#endif

			sort_order[cur_sort]=cur_sort;

			Register_frame(params, &sort_list[cur_sort]);
			cur_sort++;
		}
		break;
		//---------------
		case FORE_SPRITE:
		{
			#ifdef _DEBUG
			if (cur_fore==MAX_fore_sprites)
				Con_fatal_error("ERROR: fore_list full in FN_register_frame [line=%d file=%s]",__LINE__,__FILE__);
			#endif

			Register_frame(params, &fore_list[cur_fore]);
			cur_fore++;
		}
		break;
		//---------------
		case FGP0_SPRITE:
		{
			#ifdef _DEBUG
			if (cur_fgp0==MAX_fgp0_sprites)
				Con_fatal_error("ERROR: fgp0_list full in FN_register_frame [line=%d file=%s]",__LINE__,__FILE__);
			#endif

			Register_frame(params, &fgp0_list[cur_fgp0]);
			cur_fgp0++;
		}
		break;
		//---------------
		case FGP1_SPRITE:
		{
			#ifdef _DEBUG
			if (cur_fgp1==MAX_fgp1_sprites)
				Con_fatal_error("ERROR: fgp1_list full in FN_register_frame [line=%d file=%s]",__LINE__,__FILE__);
			#endif

			Register_frame(params, &fgp1_list[cur_fgp1]);
			cur_fgp1++;
		}
		break;
		//---------------
		// NO_SPRITE no registering!
	}


	return(IR_CONT);
}
//------------------------------------------------------------------------------------
void Start_new_palette(void)	//Tony25Sept96
{
	//start layer palette fading up
	uint8 black[4]={0,0,0,0};

	uint8 *screenFile;

	//if the screen is still fading down then wait for black - could happen when everythings cached into a large memory model
	do
	{
		ServiceWindows();
	}
	while(GetFadeStatus()==RDFADE_DOWN);


	screenFile = res_man.Res_open(this_screen.background_layer_id);	// open the screen file

	UpdatePaletteMatchTable((uint8 *) FetchPaletteMatchTable(screenFile));

	SetPalette(0, 256, FetchPalette(screenFile), RDPAL_FADE);
	lastPaletteRes=0;	// indicating that it's a screen palette

  	res_man.Res_close(this_screen.background_layer_id);	// close screen file

	//FadeUp((float)1.75);	// start fade up
	FadeUp((float)0.75);	// start fade up

 	this_screen.new_palette=0;	// reset
}
//------------------------------------------------------------------------------------
int32 FN_update_player_stats(int32 *params)	//Tony28Nov96
{
//engine needs to know certain info about the player


	Object_mega	*ob_mega = (Object_mega *) params[0];




	this_screen.player_feet_x = ob_mega->feet_x;
	this_screen.player_feet_y = ob_mega->feet_y;

	PLAYER_FEET_X=ob_mega->feet_x;	//for the script
	PLAYER_FEET_Y=ob_mega->feet_y;
	PLAYER_CUR_DIR=ob_mega->current_dir;

	SCROLL_OFFSET_X=this_screen.scroll_offset_x;

	//Zdebug(42,"%d %d", ob_mega->feet_x, ob_mega->feet_y);

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
int32 FN_fade_down(int32 *params)	//Tony5Dec96
{

	if	(GetFadeStatus()==RDFADE_NONE)	//NONE means up! can only be called when screen is fully faded up - multiple calls wont have strange effects
	{
		FadeDown((float)0.75);

		return(IR_CONT);
	}

	return(IR_CONT);
}

int32 FN_fade_up(int32 *params)		//Chris 15May97
{
	do
	{
		ServiceWindows();
	}
	while(GetFadeStatus()==RDFADE_DOWN);

	if (GetFadeStatus()==RDFADE_BLACK)
	{
		FadeUp((float)0.75);

		return(IR_CONT);
	}

	return(IR_CONT);
}
//------------------------------------------------------------------------------------
//	typedef struct
//	{
//		uint8	red;
//		uint8	green;
//		uint8	blue;
//		uint8	alpha;
//	} _palEntry;

//------------------------------------------------------------
//	typedef struct
//	{
//		uint8	firstEntry;	// first colour number in this palette (0..255)
//		uint8	noEntries;	// number of Entries-1 (0..255) to be taken as (1..256)
//	} _paletteHeader;

//------------------------------------------------------------
int32 FN_set_palette(int32 *params)		// James05jun97
{
	SetFullPalette(params[0]);

	return(IR_CONT);
}
//------------------------------------------------------------
void SetFullPalette(int32 palRes)		// James17jun97
{
	// params 0 resource number of palette file
	//			or 0 if it's to be the palette from the current screen

	uint8 black[4]={0,0,0,0};
	uint8 *file;
	_standardHeader *head;


	//----------------------------------
	// fudge for hut interior
	// - unpausing should restore last palette as normal (could be screen palette or 'dark_palette_13')
	// - but restoring the screen palette after 'dark_plaette_13' should now work properly too!
	if (LOCATION==13)	// hut interior
	{
		if (palRes==0xffffffff)	// unpausing
			palRes = lastPaletteRes;	// restore whatever palette was last set (screen palette or 'dark_palette_13')
	}
	else
	{
		// (James 03sep97)
		// check if we're just restoring the current screen palette
		// because we might actually need to use a separate palette file anyway
		// eg. for pausing & unpausing during the eclipse

 		if (palRes==0xffffffff)	// unpausing (fudged for location 13)
			palRes=0;			// we really meant '0'

		if ((palRes==0) && (lastPaletteRes))
			palRes = lastPaletteRes;
	}
	//----------------------------------


	if (palRes)	// non-zero: set palette to this separate palette file
	{
		head	= (_standardHeader*)res_man.Res_open(palRes);	// open the palette file

		#ifdef _DEBUG
		if (head->fileType != PALETTE_FILE)
 			Con_fatal_error("FN_set_palette() called with invalid resource! (%s line %u)",__FILE__,__LINE__);
		#endif

		file = (uint8*)(head+1);

		file[0] = 0;	// always set colour 0 to black
		file[1] = 0;	// because most background screen palettes have a bright colour 0
		file[2] = 0;	// although it should come out as black in the game!
		file[3] = 0;

//		UpdatePaletteMatchTable(file+(256*4));	// not yet in separate palette files

		SetPalette(0, 256, file, RDPAL_INSTANT);

		if (palRes != CONTROL_PANEL_PALETTE)	// (James 03sep97)
			lastPaletteRes=palRes;	// indicating that it's a separate palette resource


	  	res_man.Res_close(palRes);			// close palette file
	}
	else			// 0: set palette to current screen palette
	{
		if (this_screen.background_layer_id)
		{
			file = res_man.Res_open(this_screen.background_layer_id);	// open the screen file

			UpdatePaletteMatchTable((uint8 *) FetchPaletteMatchTable(file));

			SetPalette(0, 256, FetchPalette(file), RDPAL_INSTANT);
			lastPaletteRes=0;	// indicating that it's a screen palette

	  		res_man.Res_close(this_screen.background_layer_id);	// close screen file
		}
		else
			Con_fatal_error("FN_set_palette(0) called, but no current screen available! (%s line %u)",__FILE__,__LINE__);
	}
}
//------------------------------------------------------------

int32 FN_restore_game(int32 *params)
{

	return (IR_CONT);
}

//------------------------------------------------------------

int32 FN_change_shadows(int *params)
{
	uint32 rv;

	if (this_screen.mask_flag)	// if last screen was using a shading mask (see below) (James 08apr97)
	{
		rv = CloseLightMask();

		if (rv)
			ExitWithReport("Driver Error %.8x [%s line %u]", rv, __FILE__, __LINE__);

		this_screen.mask_flag = 0;
	}

	return (IR_CONT);
}


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

