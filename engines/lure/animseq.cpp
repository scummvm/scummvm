/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/animseq.h"
#include "lure/palette.h"
#include "lure/decode.h"
#include "lure/events.h"
#include "common/endian.h"

namespace Lure {

// delay
// Delays for a given number of milliseconds. If it returns true, it indicates that
// Escape has been pressed, and the introduction should be aborted.

AnimAbortType AnimationSequence::delay(uint32 milliseconds) {
	uint32 delayCtr = _system.getMillis() + milliseconds;
	Events &events = Events::getReference();

	while (_system.getMillis() < delayCtr) {
		while (events.pollEvent()) {
			if (events.type() == OSystem::EVENT_KEYDOWN) {
				if (events.event().kbd.keycode == 27) return ABORT_END_INTRO;
				else return ABORT_NEXT_SCENE;
			} else if (events.type() == OSystem::EVENT_LBUTTONDOWN)
				return ABORT_NEXT_SCENE;
			else if (events.type() == OSystem::EVENT_QUIT) 
				return ABORT_END_INTRO;
		}

		uint32 delayAmount = delayCtr - _system.getMillis();
		if (delayAmount > 10) delayAmount = 10;
		_system.delayMillis(delayAmount);
	}
	return ABORT_NONE;
}

// decodeFrame
// Decodes a single frame of the animation sequence

void AnimationSequence::decodeFrame(byte *&pPixels, byte *&pLines) {
	byte *screen = _screen.screen_raw();   
	uint16 screenPos = 0;
	uint16 len;

	while (screenPos < SCREEN_SIZE) {
		// Get line length
		len = (uint16) *pLines++;
		if (len == 0) {
			len = READ_LE_UINT16(pLines);
			pLines += 2;
		}
	
		// Move the splice over
		memcpy(screen, pPixels, len);
		screen += len;
		screenPos += len;
		pPixels += len;

		// Get the offset inc amount
		len = (uint16) *pLines++;
		if (len == 0) {
			len = READ_LE_UINT16(pLines);
			pLines += 2;
		}

		screen += len;
		screenPos += len;
	}

	// Make the decoded frame visible
	_screen.update();
}

AnimationSequence::AnimationSequence(Screen &screen, OSystem &system, uint16 screenId, Palette &palette, 
					 bool fadeIn): _screen(screen), _system(system), _screenId(screenId), _palette(palette) {
	PictureDecoder decoder;
	Disk &d = Disk::getReference();
	MemoryBlock *data = d.getEntry(_screenId);
	_decodedData = decoder.decode(data, MAX_ANIM_DECODER_BUFFER_SIZE);
	delete data;

	_lineRefs = d.getEntry(_screenId + 1);

	// Show the screen that preceeds the start of the animation data
	_screen.setPaletteEmpty();
	_screen.screen().data().copyFrom(_decodedData, 0, 0, FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH);
	_screen.update();

	// Set the palette
	if (fadeIn)	_screen.paletteFadeIn(&_palette);
	else _screen.setPalette(&_palette);

	// Set up frame poitners
	_pPixels = _decodedData->data() + SCREEN_SIZE;
	_pLines = _lineRefs->data();
	_pPixelsEnd = _decodedData->data() + _decodedData->size() - 1;
	_pLinesEnd = _lineRefs->data() + _lineRefs->size() - 1;
}

AnimationSequence::~AnimationSequence() {
	delete _lineRefs;
	delete _decodedData;
}

// show
// Main method for displaying the animation

AnimAbortType AnimationSequence::show() {
	AnimAbortType result;

	// Loop through displaying the animations
	while ((_pPixels < _pPixelsEnd) && (_pLines < _pLinesEnd)) {
		decodeFrame(_pPixels, _pLines);

		result = delay(130);
		if (result != ABORT_NONE) return result;
	}

	return ABORT_NONE;
}

bool AnimationSequence::step() {
	if ((_pPixels >= _pPixelsEnd) || (_pLines >= _pLinesEnd)) return false;
	decodeFrame(_pPixels, _pLines);
	_screen.setPalette(&_palette);
	return true;
}

} // end of namespace Lure
