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

#include <stdio.h>
//#include <windows.h>

#include "stdafx.h"
//#include "src\driver96.h"
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "header.h"
#include "logic.h"
#include "memory.h"
#include "protocol.h"
#include "resman.h"

//-----------------------------------------------------------------------------------------------------------------------
// returns a pointer to the first palette entry, given the pointer to the start of the screen file
// assumes it has been passed a pointer to a valid screen file
uint8 *FetchPalette(uint8 *screenFile)	// Chris 04Oct96
{
	uint8 *palette;

	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	palette = (uint8 *)mscreenHeader + mscreenHeader->palette;

	palette[0] = 0;	// always set colour 0 to black
	palette[1] = 0;	// because most background screen palettes have a bright colour 0
	palette[2] = 0;	// although it should come out as black in the game!
	palette[3] = 0;
   
	return palette;
}
//-----------------------------------------------------------------------------------------------------------------------
// returns a pointer to the start of the palette match table, given the pointer to the start of the screen file
// assumes it has been passed a pointer to a valid screen file

uint8 *FetchPaletteMatchTable(uint8 *screenFile)	// James 09dec96
{
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	return (uint8 *) mscreenHeader + mscreenHeader->paletteTable;
}

//-----------------------------------------------------------------------------------------------------------------------
// returns a pointer to the screen header, given the pointer to the start of the screen file
// assumes it has been passed a pointer to a valid screen file
_screenHeader *FetchScreenHeader(uint8 *screenFile)	//Chris 04Oct96
{
	// Get the table
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	return (_screenHeader*) ((uint8 *) mscreenHeader + mscreenHeader->screen);

}
//-----------------------------------------------------------------------------------------------------------------------
// returns a pointer to the requested layer header, given the pointer to the start of the screen file
// drops out if the requested layer number exceeds the number of layers on this screen
// assumes it has been passed a pointer to a valid screen file
_layerHeader *FetchLayerHeader(uint8 *screenFile, uint16 layerNo)	//Chris 04Oct96
{
	_screenHeader *screenHead;


	screenHead = FetchScreenHeader(screenFile);

#ifdef _SWORD2_DEBUG
	if (layerNo > (screenHead->noLayers-1))	// layer number too large!
		Con_fatal_error("FetchLayerHeader(%d) invalid layer number! (%s line %u)",layerNo,__FILE__,__LINE__);
#endif

	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	return (_layerHeader *) ((uint8 *) mscreenHeader + mscreenHeader->layers + (layerNo * sizeof(_layerHeader)));
}

//---------------------------------------------------------------
// returns a pointer to the start of the shading mask, given the pointer to the start of the screen file
// assumes it has been passed a pointer to a valid screen file

uint8 *FetchShadingMask(uint8 *screenFile)	// James 08apr97
{
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	return (uint8 *) mscreenHeader + mscreenHeader->maskOffset;
}

//-----------------------------------------------------------------------------------------------------------------------
// returns a pointer to the anim header, given the pointer to the start of the anim file
// assumes it has been passed a pointer to a valid anim file

_animHeader *FetchAnimHeader(uint8 *animFile)	// (25sep96JEL)
{
	return (_animHeader *) (animFile + sizeof(_standardHeader));
}

//---------------------------------------------------------------
// returns a pointer to the requested frame number's cdtEntry, given the pointer to the start of the anim file
// drops out if the requested frame number exceeds the number of frames in this anim
// assumes it has been passed a pointer to a valid anim file

_cdtEntry *FetchCdtEntry(uint8 *animFile, uint16 frameNo)	// Chris 09Oct96
{
	_animHeader *animHead;

	animHead = FetchAnimHeader(animFile);

#ifdef _SWORD2_DEBUG
	if (frameNo > (animHead->noAnimFrames-1))	// frame number too large!
		Con_fatal_error("FetchCdtEntry(animFile,%d) - anim only %d frames (%s line %u)",frameNo,animHead->noAnimFrames,__FILE__,__LINE__);
#endif

	return (_cdtEntry *) ( (uint8 *)animHead + sizeof(_animHeader) + frameNo * sizeof(_cdtEntry) );
}
//---------------------------------------------------------------
// returns a pointer to the requested frame number's header, given the pointer to the start of the anim file
// drops out if the requested frame number exceeds the number of frames in this anim
// assumes it has been passed a pointer to a valid anim file

_frameHeader *FetchFrameHeader(uint8 *animFile, uint16 frameNo)	// James 31oct96
{
	// required address = (address of the start of the anim header) + frameOffset
	return (_frameHeader *) (animFile + sizeof(_standardHeader) + (FetchCdtEntry(animFile,frameNo)->frameOffset) );
}
//---------------------------------------------------------------
// Returns a pointer to the requested parallax layer data.
// Assumes it has been passed a pointer to a valid screen file.
_parallax *FetchBackgroundParallaxLayer(uint8 *screenFile, int layer) // Chris 04Oct96
{
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->bg_parallax[layer] == 0)
		Con_fatal_error("FetchBackgroundParallaxLayer(%d) - No parallax layer exists (%s line %u)",layer,__FILE__,__LINE__);
#endif

	return (_parallax *) ((uint8 *) mscreenHeader + mscreenHeader->bg_parallax[layer]);
}
//---------------------------------------------------------------
_parallax *FetchBackgroundLayer(uint8 *screenFile) // Chris 04Oct96
{
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->screen == 0)
		Con_fatal_error("FetchBackgroundLayer (%d) - No background layer exists (%s line %u)",__FILE__,__LINE__);
#endif

	return (_parallax *) ((uint8 *) mscreenHeader + mscreenHeader->screen + sizeof(_screenHeader));
}
//---------------------------------------------------------------
_parallax *FetchForegroundParallaxLayer(uint8 *screenFile, int layer) // Chris 04Oct96
{
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->fg_parallax[layer] == 0)
		Con_fatal_error("FetchForegroundParallaxLayer(%d) - No parallax layer exists (%s line %u)",layer,__FILE__,__LINE__);
#endif

	return (_parallax *) ((uint8 *) mscreenHeader + mscreenHeader->fg_parallax[layer]);
}
//---------------------------------------------------------------
uint8 errorLine[128];
//---------------------------------------------------------------
uint8 *FetchTextLine(uint8 *file, uint32	text_line)	//Tony24Oct96
{
	// Get the table
	_standardHeader *fileHeader;
	uint32	*point;


	_textHeader *text_header = (_textHeader *) (file + sizeof(_standardHeader));


	if	(text_line>=text_header->noOfLines)		// (James08aug97)
	{
		fileHeader = (_standardHeader*)file;
		sprintf ((char*)errorLine, "xxMissing line %d of %s (only 0..%d)", text_line, fileHeader->name, text_header->noOfLines-1);
		errorLine[0]=0;	// first 2 chars are NULL so that actor-number comes out as '0'
		errorLine[1]=0;
		return(errorLine);

//		GOT RID OF CON_FATAL_ERROR HERE BECAUSE WE DON'T WANT IT TO CRASH OUT ANY MORE!
//		Con_fatal_error("FetchTextLine cannot get %d, only 0..%d avail (%s line %u)", text_line, text_header->noOfLines-1,__FILE__,__LINE__);
	}


	point=(uint32*) text_header+1;	//point to the lookup table

	return( (uint8*) (file + READ_LE_UINT32(point+text_line)) );
}
//---------------------------------------------------------------
// Used for testing text & speech (see FN_I_speak in speech.cpp)
uint8 CheckTextLine(uint8 *file, uint32	text_line)	// (James26jun97)
{
	_textHeader *text_header = (_textHeader *) (file + sizeof(_standardHeader));

	if (text_line>=text_header->noOfLines)
		return(0);	// out of range => invalid
	else
		return(1);	// valid
}
//---------------------------------------------------------------
uint8 *FetchObjectName(int32 resourceId)	// James15jan97
{
	_standardHeader *header;
	
	header = (_standardHeader*) res_man.Res_open(resourceId);

	res_man.Res_close(resourceId);

	return (header->name);	// note this pointer is no longer valid, but it should be ok until another resource is opened!
}
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
