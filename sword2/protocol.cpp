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
#include "bs2/sword2.h"
#include "bs2/console.h"
#include "bs2/defs.h"
#include "bs2/header.h"
#include "bs2/memory.h"
#include "bs2/protocol.h"
#include "bs2/resman.h"

namespace Sword2 {

// Returns a pointer to the first palette entry, given the pointer to the
// start of the screen file.

uint8 *FetchPalette(uint8 *screenFile) {
	uint8 *palette;

	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	palette = (uint8 *) mscreenHeader + mscreenHeader->palette;

	// Always set colour 0 to black, because while most background screen
	// palettes have a bright colour 0 it should come out as black in the
	// game.

	palette[0] = 0;
	palette[1] = 0;
	palette[2] = 0;
	palette[3] = 0;
   
	return palette;
}

// Returns a pointer to the start of the palette match table, given the
// pointer to the start of the screen file.

uint8 *FetchPaletteMatchTable(uint8 *screenFile) {
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	return (uint8 *) mscreenHeader + mscreenHeader->paletteTable;
}

// Returns a pointer to the screen header, given the pointer to the start of
// the screen file.

_screenHeader *FetchScreenHeader(uint8 *screenFile) {
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));
	_screenHeader *screenHeader = (_screenHeader*) ((uint8 *) mscreenHeader + mscreenHeader->screen);

	return screenHeader;
}

// Returns a pointer to the requested layer header, given the pointer to the
// start of the screen file. Drops out if the requested layer number exceeds
// the number of layers on this screen.

_layerHeader *FetchLayerHeader(uint8 *screenFile, uint16 layerNo) {	// Chris 04Oct96
#ifdef _SWORD2_DEBUG
	_screenHeader *screenHead = FetchScreenHeader(screenFile);

	if (layerNo > screenHead->noLayers - 1)
		error("FetchLayerHeader(%d) invalid layer number!", layerNo);
#endif

	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	_layerHeader *layerHeader = (_layerHeader *) ((uint8 *) mscreenHeader + mscreenHeader->layers + (layerNo * sizeof(_layerHeader)));

	return layerHeader;
}

// Returns a pointer to the start of the shading mask, given the pointer to
// the start of the screen file.

uint8 *FetchShadingMask(uint8 *screenFile) {
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

	return (uint8 *) mscreenHeader + mscreenHeader->maskOffset;
}

// Returns a pointer to the anim header, given the pointer to the start of
// the anim file.

_animHeader *FetchAnimHeader(uint8 *animFile) {
	return (_animHeader *) (animFile + sizeof(_standardHeader));
}

// Returns a pointer to the requested frame number's cdtEntry, given the
// pointer to the start of the anim file. Drops out if the requested frame
// number exceeds the number of frames in this anim.

_cdtEntry *FetchCdtEntry(uint8 *animFile, uint16 frameNo) {
	_animHeader *animHead = FetchAnimHeader(animFile);

#ifdef _SWORD2_DEBUG
	if (frameNo > animHead->noAnimFrames - 1)
		error("FetchCdtEntry(animFile,%d) - anim only %d frames", frameNo, animHead->noAnimFrames);
#endif

	return (_cdtEntry *) ((uint8 *) animHead + sizeof(_animHeader) + frameNo * sizeof(_cdtEntry));
}

// Returns a pointer to the requested frame number's header, given the
// pointer to the start of the anim file. Drops out if the requested frame
// number exceeds the number of frames in this anim

_frameHeader *FetchFrameHeader(uint8 *animFile, uint16 frameNo)	{
	// required address = (address of the start of the anim header) + frameOffset
	return (_frameHeader *) (animFile + sizeof(_standardHeader) + FetchCdtEntry(animFile, frameNo)->frameOffset);
}

// Returns a pointer to the requested parallax layer data.

_parallax *FetchBackgroundParallaxLayer(uint8 *screenFile, int layer) {
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->bg_parallax[layer] == 0)
		error("FetchBackgroundParallaxLayer(%d) - No parallax layer exists", layer);
#endif

	return (_parallax *) ((uint8 *) mscreenHeader + mscreenHeader->bg_parallax[layer]);
}

_parallax *FetchBackgroundLayer(uint8 *screenFile) {
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->screen == 0)
		error("FetchBackgroundLayer (%d) - No background layer exists");
#endif

	return (_parallax *) ((uint8 *) mscreenHeader + mscreenHeader->screen + sizeof(_screenHeader));
}

_parallax *FetchForegroundParallaxLayer(uint8 *screenFile, int layer) {
	_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) (screenFile + sizeof(_standardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->fg_parallax[layer] == 0)
		error("FetchForegroundParallaxLayer(%d) - No parallax layer exists", layer);
#endif

	return (_parallax *) ((uint8 *) mscreenHeader + mscreenHeader->fg_parallax[layer]);
}

uint8 errorLine[128];

uint8 *FetchTextLine(uint8 *file, uint32 text_line) {
	_standardHeader *fileHeader;
	uint32 *point;

	_textHeader *text_header = (_textHeader *) (file + sizeof(_standardHeader));

	if (text_line >= text_header->noOfLines) {
		fileHeader = (_standardHeader*)file;
		sprintf((char *) errorLine, "xxMissing line %d of %s (only 0..%d)", text_line, fileHeader->name, text_header->noOfLines - 1);


		// first 2 chars are NULL so that actor-number comes out as '0'
		errorLine[0] = 0;
		errorLine[1] = 0;
		return errorLine;
	}

	//point to the lookup table
	point = (uint32 *) text_header + 1;

	return (uint8 *) (file + READ_LE_UINT32(point + text_line));
}


// Used for testing text & speech (see fnISpeak in speech.cpp)

uint8 CheckTextLine(uint8 *file, uint32	text_line) {
	_textHeader *text_header = (_textHeader *) (file + sizeof(_standardHeader));

	// out of range => invalid
	if (text_line >= text_header->noOfLines)
		return 0;

	// valid
	return 1;
}

uint8 *FetchObjectName(int32 resourceId) {
	_standardHeader *header;
	
	header = (_standardHeader *) res_man.open(resourceId);
	res_man.close(resourceId);

	// note this pointer is no longer valid, but it should be ok until
	// another resource is opened!

	// FIXME: I don't like the sound of this at all. Though thanks to the
	// BS2 memory manager, at least it will still point to malloced
	// memory.

	return header->name;
}

} // End of namespace Sword2
