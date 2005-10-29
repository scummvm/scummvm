/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/resman.h"

namespace Sword2 {

/**
 * Returns a pointer to the first palette entry, given the pointer to the start
 * of the screen file.
 */

byte *Sword2Engine::fetchPalette(byte *screenFile) {
	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());

	byte *palette = screenFile + ResHeader::size() + mscreenHeader.palette;

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

byte *Sword2Engine::fetchPaletteMatchTable(byte *screenFile) {
	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());

	return screenFile + ResHeader::size() + mscreenHeader.paletteTable;
}

/**
 * Returns a pointer to the screen header, given the pointer to the start of
 * the screen file.
 */

byte *Sword2Engine::fetchScreenHeader(byte *screenFile) {
	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());

	return screenFile + ResHeader::size() + mscreenHeader.screen;
}

/**
 * Returns a pointer to the requested layer header, given the pointer to the
 * start of the screen file. Drops out if the requested layer number exceeds
 * the number of layers on this screen.
 */

byte *Sword2Engine::fetchLayerHeader(byte *screenFile, uint16 layerNo) {
#ifdef SWORD2_DEBUG
	ScreenHeader screenHead;

	screenHead.read(fetchScreenHeader(screenFile));
	assert(layerNo < screenHead.noLayers);
#endif

	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());

	return screenFile + ResHeader::size() + mscreenHeader.layers + layerNo * LayerHeader::size();
}

/**
 * Returns a pointer to the start of the shading mask, given the pointer to the
 * start of the screen file.
 */

byte *Sword2Engine::fetchShadingMask(byte *screenFile) {
	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());

	return screenFile + ResHeader::size() + mscreenHeader.maskOffset;
}

/**
 * Returns a pointer to the anim header, given the pointer to the start of the
 * anim file.
 */

byte *Sword2Engine::fetchAnimHeader(byte *animFile) {
	return animFile + ResHeader::size();
}

/**
 * Returns a pointer to the requested frame number's cdtEntry, given the
 * pointer to the start of the anim file. Drops out if the requested frame
 * number exceeds the number of frames in this anim.
 */

byte *Sword2Engine::fetchCdtEntry(byte *animFile, uint16 frameNo) {
#ifdef SWORD2_DEBUG
	AnimHeader animHead;

	animHead.read(fetchAnimHeader(animFile));

	if (frameNo > animHead->noAnimFrames - 1)
		error("fetchCdtEntry(animFile,%d) - anim only %d frames", frameNo, animHead.noAnimFrames);
#endif

	return fetchAnimHeader(animFile) + AnimHeader::size() + frameNo * CdtEntry::size();
}

/**
 * Returns a pointer to the requested frame number's header, given the pointer
 * to the start of the anim file. Drops out if the requested frame number
 * exceeds the number of frames in this anim
 */

byte *Sword2Engine::fetchFrameHeader(byte *animFile, uint16 frameNo) {
	// required address = (address of the start of the anim header) + frameOffset
	CdtEntry cdt;

	cdt.read(fetchCdtEntry(animFile, frameNo));

	return animFile + ResHeader::size() + cdt.frameOffset;
}

/**
 * Returns a pointer to the requested parallax layer data.
 */

byte *Sword2Engine::fetchBackgroundParallaxLayer(byte *screenFile, int layer) {
	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());
	assert(mscreenHeader.bg_parallax[layer]);

	return screenFile + ResHeader::size() + mscreenHeader.bg_parallax[layer];
}

byte *Sword2Engine::fetchBackgroundLayer(byte *screenFile) {
	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());
	assert(mscreenHeader.screen);

	return screenFile + ResHeader::size() + mscreenHeader.screen + ScreenHeader::size();
}

byte *Sword2Engine::fetchForegroundParallaxLayer(byte *screenFile, int layer) {
	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());
	assert(mscreenHeader.fg_parallax[layer]);

	return screenFile + ResHeader::size() + mscreenHeader.fg_parallax[layer];
}

byte *Sword2Engine::fetchTextLine(byte *file, uint32 text_line) {
	TextHeader text_header;
	static byte errorLine[128];

	text_header.read(file + ResHeader::size());

	if (text_line >= text_header.noOfLines) {
		sprintf((char *)errorLine, "xxMissing line %d of %s (only 0..%d)", text_line, _resman->fetchName(file), text_header.noOfLines - 1);

		// first 2 chars are NULL so that actor-number comes out as '0'
		errorLine[0] = 0;
		errorLine[1] = 0;
		return errorLine;
	}

	// The "number of lines" field is followed by a lookup table

	return file + READ_LE_UINT32(file + ResHeader::size() + 4 + 4 * text_line);
}

// Used for testing text & speech (see fnISpeak in speech.cpp)

bool Sword2Engine::checkTextLine(byte *file, uint32 text_line) {
	TextHeader text_header;

	text_header.read(file + ResHeader::size());

	return text_line < text_header.noOfLines;
}

} // End of namespace Sword2
