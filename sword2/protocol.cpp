/* Copyright (C) 1994-2004 Revolution Software Ltd
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

#include "common/stdafx.h"
#include "sword2/sword2.h"

namespace Sword2 {

/**
 * Returns a pointer to the first palette entry, given the pointer to the start
 * of the screen file.
 */

uint8 *Sword2Engine::fetchPalette(uint8 *screenFile) {
	uint8 *palette;

	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));

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

/**
 * Returns a pointer to the start of the palette match table, given the pointer
 * to the start of the screen file.
 */

uint8 *Sword2Engine::fetchPaletteMatchTable(uint8 *screenFile) {
	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));

	return (uint8 *) mscreenHeader + mscreenHeader->paletteTable;
}

/**
 * Returns a pointer to the screen header, given the pointer to the start of
 * the screen file.
 */

ScreenHeader *Sword2Engine::fetchScreenHeader(uint8 *screenFile) {
	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));
	ScreenHeader *screenHeader = (ScreenHeader *) ((uint8 *) mscreenHeader + mscreenHeader->screen);

	return screenHeader;
}

/**
 * Returns a pointer to the requested layer header, given the pointer to the
 * start of the screen file. Drops out if the requested layer number exceeds
 * the number of layers on this screen.
 */

LayerHeader *Sword2Engine::fetchLayerHeader(uint8 *screenFile, uint16 layerNo) {
#ifdef _SWORD2_DEBUG
	ScreenHeader *screenHead = fetchScreenHeader(screenFile);

	if (layerNo > screenHead->noLayers - 1)
		error("fetchLayerHeader(%d) invalid layer number!", layerNo);
#endif

	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));

	LayerHeader *layerHeader = (LayerHeader *) ((uint8 *) mscreenHeader + mscreenHeader->layers + (layerNo * sizeof(LayerHeader)));

	return layerHeader;
}

/**
 * Returns a pointer to the start of the shading mask, given the pointer to the
 * start of the screen file.
 */

uint8 *Sword2Engine::fetchShadingMask(uint8 *screenFile) {
	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));

	return (uint8 *) mscreenHeader + mscreenHeader->maskOffset;
}

/**
 * Returns a pointer to the anim header, given the pointer to the start of the
 * anim file.
 */

AnimHeader *Sword2Engine::fetchAnimHeader(uint8 *animFile) {
	return (AnimHeader *) (animFile + sizeof(StandardHeader));
}

/**
 * Returns a pointer to the requested frame number's cdtEntry, given the
 * pointer to the start of the anim file. Drops out if the requested frame
 * number exceeds the number of frames in this anim.
 */

CdtEntry *Sword2Engine::fetchCdtEntry(uint8 *animFile, uint16 frameNo) {
	AnimHeader *animHead = fetchAnimHeader(animFile);

#ifdef _SWORD2_DEBUG
	if (frameNo > animHead->noAnimFrames - 1)
		error("fetchCdtEntry(animFile,%d) - anim only %d frames", frameNo, animHead->noAnimFrames);
#endif

	return (CdtEntry *) ((uint8 *) animHead + sizeof(AnimHeader) + frameNo * sizeof(CdtEntry));
}

/**
 * Returns a pointer to the requested frame number's header, given the pointer
 * to the start of the anim file. Drops out if the requested frame number
 * exceeds the number of frames in this anim
 */

FrameHeader *Sword2Engine::fetchFrameHeader(uint8 *animFile, uint16 frameNo)	{
	// required address = (address of the start of the anim header) + frameOffset
	return (FrameHeader *) (animFile + sizeof(StandardHeader) + fetchCdtEntry(animFile, frameNo)->frameOffset);
}

/**
 * Returns a pointer to the requested parallax layer data.
 */

Parallax *Sword2Engine::fetchBackgroundParallaxLayer(uint8 *screenFile, int layer) {
	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->bg_parallax[layer] == 0)
		error("fetchBackgroundParallaxLayer(%d) - No parallax layer exists", layer);
#endif

	return (Parallax *) ((uint8 *) mscreenHeader + mscreenHeader->bg_parallax[layer]);
}

Parallax *Sword2Engine::fetchBackgroundLayer(uint8 *screenFile) {
	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->screen == 0)
		error("fetchBackgroundLayer (%d) - No background layer exists");
#endif

	return (Parallax *) ((uint8 *) mscreenHeader + mscreenHeader->screen + sizeof(ScreenHeader));
}

Parallax *Sword2Engine::fetchForegroundParallaxLayer(uint8 *screenFile, int layer) {
	MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) (screenFile + sizeof(StandardHeader));

#ifdef _SWORD2_DEBUG
	if (mscreenHeader->fg_parallax[layer] == 0)
		error("fetchForegroundParallaxLayer(%d) - No parallax layer exists", layer);
#endif

	return (Parallax *) ((uint8 *) mscreenHeader + mscreenHeader->fg_parallax[layer]);
}

uint8 errorLine[128];

uint8 *Sword2Engine::fetchTextLine(uint8 *file, uint32 text_line) {
	StandardHeader *fileHeader;
	uint32 *point;

	TextHeader *text_header = (TextHeader *) (file + sizeof(StandardHeader));

	if (text_line >= text_header->noOfLines) {
		fileHeader = (StandardHeader *) file;
		sprintf((char *) errorLine, "xxMissing line %d of %s (only 0..%d)", text_line, fileHeader->name, text_header->noOfLines - 1);


		// first 2 chars are NULL so that actor-number comes out as '0'
		errorLine[0] = 0;
		errorLine[1] = 0;
		return errorLine;
	}

	// point to the lookup table
	point = (uint32 *) text_header + 1;

	return (uint8 *) (file + READ_LE_UINT32(point + text_line));
}


// Used for testing text & speech (see fnISpeak in speech.cpp)

bool Sword2Engine::checkTextLine(uint8 *file, uint32 text_line) {
	TextHeader *text_header = (TextHeader *) (file + sizeof(StandardHeader));

	return text_line < text_header->noOfLines;
}

uint8 *Sword2Engine::fetchObjectName(int32 resourceId) {
	StandardHeader *header;
	
	header = (StandardHeader *) _resman->openResource(resourceId);
	_resman->closeResource(resourceId);

	// note this pointer is no longer valid, but it should be ok until
	// another resource is opened!

	// FIXME: I don't like the sound of this at all. Though thanks to the
	// BS2 memory manager, at least it will still point to malloced
	// memory.

	return header->name;
}

} // End of namespace Sword2
