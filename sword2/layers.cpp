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
//high level layer initialising

//the system supports:
//		1 optional background parallax layer
//		1 not optional normal backdrop layer
//		3 normal sorted layers
//		up to 2 foreground parallax layers

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "stdafx.h"
#include "driver/driver96.h"
#include "build_display.h"
#include "console.h"
#include "debug.h"
#include "header.h"
#include "layers.h"
#include "memory.h"
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "sound.h"	// (James22july97) for Clear_fx_queue() called from FN_init_background()

//------------------------------------------------------------------------------------


screen_info	this_screen;	//this_screen describes the current back buffer and its in-game scroll positions, etc.
//------------------------------------------------------------------------------------
int32 FN_init_background(int32 *params)	//Tony11Sept96
{
//param	0 res id of normal background layer - cannot be 0
//param	1 1 yes 0 no for a new palette
//this screen defines the size of the back buffer

	_multiScreenHeader *screenLayerTable;	// James 06feb97
	_screenHeader	*screen_head;
	_layerHeader	*layer;
 	_spriteInfo		spriteInfo;
	uint32	j;
	uint8	*file;
	uint32	rv;


	//--------------------------------------
	// Write to walkthrough file (zebug0.txt)
	#ifdef _SWORD2_DEBUG
	Zdebug(0,"=====================================");
	Zdebug(0,"CHANGED TO LOCATION \"%s\"", FetchObjectName(*params));
	Zdebug(0,"=====================================");

	// Also write this to system debug file
	Zdebug("=====================================");
	Zdebug("CHANGED TO LOCATION \"%s\"", FetchObjectName(*params));
	Zdebug("=====================================");
	#endif
 	//--------------------------------------

	Clear_fx_queue();		// stops all fx & clears the queue (James22july97)


#ifdef _SWORD2_DEBUG
	Zdebug("FN_init_background(%d)", *params);

	if	(!*params)
	{
		Con_fatal_error("ERROR: FN_set_background cannot have 0 for background layer id! (%s line=%u)",__FILE__,__LINE__);
	}
#endif // _SWORD2_DEBUG


	//-------------------------------------------------------
	// if the screen is still fading down then wait for black
	do
	{
		ServiceWindows();
	}
	while(GetFadeStatus()==RDFADE_DOWN);
	//-------------------------------------------------------

	if (this_screen.mask_flag)	// if last screen was using a shading mask (see below) (James 08apr97)
	{
		rv = CloseLightMask();

		if (rv)
			ExitWithReport("Driver Error %.8x [%s line %u]", rv, __FILE__, __LINE__);
	}
	
	//--------------------------------------------------------
	// New stuff for faster screen drivers (James 06feb97)

	if (this_screen.background_layer_id)	// for drivers: close the previous screen if one is open
		CloseBackgroundLayer();

	//--------------------------------------------------------


	this_screen.background_layer_id=*params;	//set the res id
	this_screen.new_palette = *(params+1);	//yes or no - palette is taken from layer file


//ok, now read the resource and pull out all the normal sort layer info
//and set them up at the beginning of the sort list - why do it each cycle


	file = res_man.Res_open(this_screen.background_layer_id);	//file points to 1st byte in the layer file
	
	screen_head = FetchScreenHeader(file);

	this_screen.number_of_layers= screen_head->noLayers;	//set number of special sort layers
	this_screen.screen_wide = screen_head->width;
	this_screen.screen_deep = screen_head->height;

	Zdebug("res test layers=%d width=%d depth=%d", screen_head->noLayers, screen_head->width, screen_head->height);

	SetLocationMetrics(screen_head->width, screen_head->height);	//initialise the driver back buffer


	if	(screen_head->noLayers)
		for	(j=0;j<screen_head->noLayers;j++)
		{
			layer=FetchLayerHeader(file,j);	//get layer header for layer j

//			add into the sort list

			sort_list[j].sort_y = layer->y+layer->height;	//need this for sorting - but leave the rest blank, we'll take from the header at print time
			sort_list[j].layer_number=j+1;	//signifies a layer

			Zdebug("init layer %d", j);
		}



//using the screen size setup the scrolling variables

	if( ((screen_head->width) > screenWide) || (screen_head->height>screenDeep) )	// if layer is larger than physical screen
	{
		this_screen.scroll_flag = 2;	//switch on scrolling (2 means first time on screen)

//	note, if we've already set the player up then we could do the initial scroll set here

		this_screen.scroll_offset_x = 0;	//reset scroll offsets
		this_screen.scroll_offset_y = 0;

//		calc max allowed offsets (to prevent scrolling off edge) - MOVE TO NEW_SCREEN in GTM_CORE.C !!
		this_screen.max_scroll_offset_x = screen_head->width-screenWide;		// NB. min scroll offsets are both zero
		this_screen.max_scroll_offset_y = screen_head->height-(screenDeep-(RDMENU_MENUDEEP*2));	// 'screenDeep' includes the menu's, so take away 80 pixels
	}
	else	//layer fits on physical screen - scrolling not required
	{
		this_screen.scroll_flag = 0;	//switch off scrolling
		this_screen.scroll_offset_x = 0;	//reset scroll offsets
		this_screen.scroll_offset_y = 0;
	}

	ResetRenderEngine();	//no inter-cycle scrol between new screens (see setScrollTarget in build display)

	// these are the physical screen coords where the system
	// will try to maintain George's actual feet coords
	this_screen.feet_x=320;
	this_screen.feet_y=340;


	//----------------------------------------------------
	// shading mask

	screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

	if (screenLayerTable->maskOffset)
	{
		spriteInfo.x			= 0;
		spriteInfo.y			= 0;
		spriteInfo.w			= screen_head->width;
		spriteInfo.h			= screen_head->height;
		spriteInfo.scale		= 0;
		spriteInfo.scaledWidth	= 0;
		spriteInfo.scaledHeight	= 0;
		spriteInfo.type			= 0;
		spriteInfo.blend		= 0;
		spriteInfo.data			= FetchShadingMask(file);
		spriteInfo.colourTable	= 0;

		rv = OpenLightMask( &spriteInfo );
		if (rv)
			ExitWithReport("Driver Error %.8x [%s line %u]", rv, __FILE__, __LINE__);

		this_screen.mask_flag=1;	// so we know to close it later! (see above)
	}
	else
		this_screen.mask_flag=0;	// no need to close a mask later

	//----------------------------------------------------

   	res_man.Res_close(this_screen.background_layer_id);	//close the screen file

	SetUpBackgroundLayers();


	Zdebug("end init");
	return(1);
}
//------------------------------------------------------------------------------------
// called from FN_init_background & also from control panel

void SetUpBackgroundLayers(void)	// James(13jun97)
{
	_multiScreenHeader *screenLayerTable;	// James 06feb97
	_screenHeader	*screen_head;
	uint8	*file;


	if (this_screen.background_layer_id)	// if we actually have a screen to initialise (in case called from control panel)
	{
		//------------------------------
		// open resource & set pointers to headers

		file = res_man.Res_open(this_screen.background_layer_id);	//file points to 1st byte in the layer file

		screen_head = FetchScreenHeader(file);

		screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

		//------------------------------
		// first background parallax

		if (screenLayerTable->bg_parallax[0])
			InitialiseBackgroundLayer(FetchBackgroundParallaxLayer(file,0));
		else
			InitialiseBackgroundLayer(NULL);

		//------------------------------
		// second background parallax

		if (screenLayerTable->bg_parallax[1])
			InitialiseBackgroundLayer(FetchBackgroundParallaxLayer(file,1));
		else
			InitialiseBackgroundLayer(NULL);

		//------------------------------
		// normal backround layer

		InitialiseBackgroundLayer(FetchBackgroundLayer(file));

		//------------------------------
		// first foreground parallax

		if (screenLayerTable->fg_parallax[0])
			InitialiseBackgroundLayer(FetchForegroundParallaxLayer(file,0));
		else
			InitialiseBackgroundLayer(NULL);

		//------------------------------
		// second foreground parallax

		if (screenLayerTable->fg_parallax[1])
			InitialiseBackgroundLayer(FetchForegroundParallaxLayer(file,1));
		else
			InitialiseBackgroundLayer(NULL);

 		//----------------------------------------------------

		res_man.Res_close(this_screen.background_layer_id);	//close the screen file

	 	//----------------------------------------------------
	}
	else	// no current screen to initialise! (In case called from control panel)
	{
	}
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
