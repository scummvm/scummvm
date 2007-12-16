/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
#include "lure/decode.h"
#include "lure/events.h"
#include "lure/palette.h"
#include "lure/sound.h"
#include "common/endian.h"

namespace Lure {

// delay
// Delays for a given number of milliseconds. If it returns true, it indicates that
// Escape has been pressed, and the introduction should be aborted.

AnimAbortType AnimationSequence::delay(uint32 milliseconds) {
	Events &events = Events::getReference();
	uint32 delayCtr = g_system->getMillis() + milliseconds;

	while (g_system->getMillis() < delayCtr) {
		while (events.pollEvent()) {
			if ((events.type() == Common::EVENT_KEYDOWN) && (events.event().kbd.ascii != 0)) {
				if (events.event().kbd.keycode == Common::KEYCODE_ESCAPE) return ABORT_END_INTRO;
				else return ABORT_NEXT_SCENE;
			} else if (events.type() == Common::EVENT_LBUTTONDOWN)
				return ABORT_NEXT_SCENE;
			else if (events.type() == Common::EVENT_QUIT) 
				return ABORT_END_INTRO;
		}

		uint32 delayAmount = delayCtr - g_system->getMillis();
		if (delayAmount > 10) delayAmount = 10;
		g_system->delayMillis(delayAmount);
	}
	return ABORT_NONE;
}

// decodeFrame
// Decodes a single frame of the animation sequence

void AnimationSequence::decodeFrame(byte *&pPixels, byte *&pLines) {
	Screen &screen = Screen::getReference();
	byte *screenData = screen.screen_raw();   
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
		memcpy(screenData, pPixels, len);
		screenData += len;
		screenPos += len;
		pPixels += len;

		// Get the offset inc amount
		len = (uint16) *pLines++;
		if (len == 0) {
			len = READ_LE_UINT16(pLines);
			pLines += 2;
		}

		screenData += len;
		screenPos += len;
	}

	// Make the decoded frame visible
	screen.update();
}

AnimationSequence::AnimationSequence(uint16 screenId, Palette &palette,  bool fadeIn, int frameDelay, 
					 const AnimSoundSequence *soundList): _screenId(screenId), _palette(palette), 
					 _frameDelay(frameDelay), _soundList(soundList) {
	Screen &screen = Screen::getReference();
	PictureDecoder decoder;
	Disk &d = Disk::getReference();
	MemoryBlock *data = d.getEntry(_screenId);
	_decodedData = decoder.decode(data, MAX_ANIM_DECODER_BUFFER_SIZE);
	delete data;

	_lineRefs = d.getEntry(_screenId + 1);

	// Reset the palette and set the initial starting screen
	screen.setPaletteEmpty(RES_PALETTE_ENTRIES);
	screen.screen().data().copyFrom(_decodedData, 0, 0, FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH);
	screen.update();

	// Set the palette
	if (fadeIn)	screen.paletteFadeIn(&_palette);
	else screen.setPalette(&_palette, 0, _palette.numEntries());

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
	const AnimSoundSequence *soundFrame = _soundList;
	int frameCtr = 0;

	// Loop through displaying the animations
	while ((_pPixels < _pPixelsEnd) && (_pLines < _pLinesEnd)) {
		if ((soundFrame != NULL) && (frameCtr == 0))
			Sound.musicInterface_Play(
				Sound.isRoland() ? soundFrame->rolandSoundId : soundFrame->adlibSoundId, 
				soundFrame->channelNum);

		decodeFrame(_pPixels, _pLines);

		result = delay(_frameDelay * 1000 / 50);
		if (result != ABORT_NONE) return result;

		if ((soundFrame != NULL) && (++frameCtr == soundFrame->numFrames)) {
			frameCtr = 0;
			++soundFrame;
			if (soundFrame->numFrames == 0) soundFrame = NULL;
		}
	}

	return ABORT_NONE;
}

bool AnimationSequence::step() {
	Screen &screen = Screen::getReference();
	if ((_pPixels >= _pPixelsEnd) || (_pLines >= _pLinesEnd)) return false;
	decodeFrame(_pPixels, _pLines);
	screen.setPalette(&_palette);
	return true;
}

} // end of namespace Lure
