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

#include "lure/intro.h"
#include "lure/animseq.h"
#include "lure/events.h"
#include "lure/sound.h"

namespace Lure {

struct AnimRecord {
	uint16 resourceId;
	uint8 paletteIndex;
	uint16 initialPause;
	uint16 endingPause;
	uint8 soundNumber;
};

static const uint16 start_screens[] = {0x18, 0x1A, 0x1E, 0x1C, 0};
static const AnimRecord anim_screens[] = {
	{0x40, 0, 0x35A, 0xC8, 0},			// The kingdom was at peace
	{0x42, 1, 0, 0x5FA, 1},				// Cliff overhang
	{0x44, 2, 0, 0, 2},					// Siluette in moonlight
	{0x24, 3, 0, 0x328 + 0x24, 0xff},	// Exposition of reaching town
	{0x46, 3, 0, 0, 3},					// Skorl approaches
	{0, 0, 0, 0, 0xff}};

// showScreen
// Shows a screen by loading it from the given resource, and then fading it in
// with a palette in the following resource. Returns true if the introduction
// should be aborted

bool Introduction::showScreen(uint16 screenId, uint16 paletteId, uint16 delaySize) {
	Events &events = Events::getReference();
	_screen.screen().loadScreen(screenId);
	_screen.update();
	Palette p(paletteId);
	_screen.paletteFadeIn(&p);
	
	bool result = events.interruptableDelay(delaySize);
	if (events.quitFlag) return true;

	_screen.paletteFadeOut();
	return result;
}

// show
// Main method for the introduction sequence

bool Introduction::show() {
	Events &events = Events::getReference();
	_screen.setPaletteEmpty();

	// Initial game company and then game screen

	for (int ctr = 0; start_screens[ctr]; ++ctr)
		if (showScreen(start_screens[ctr], start_screens[ctr] + 1, 5000)) 
			return true;	

	// Animated screens

	AnimationSequence *anim;
	bool result;
	uint8 currentSound = 0xff;
	PaletteCollection coll(0x32);
	const AnimRecord *curr_anim = anim_screens;
	for (; curr_anim->resourceId; ++curr_anim) {
		// Handle sound selection
		if (curr_anim->soundNumber != 0xff) {
			if (currentSound != 0xff) 
				// Fade out the previous sound
				Sound.fadeOut();
		
			currentSound = curr_anim->soundNumber;
			Sound.musicInterface_Play(currentSound, 0);
			// DEBUG TEST
//			g_system->delayMillis(1000);
//			Sound.musicInterface_Play(1, 1);
		}

		bool fadeIn = curr_anim == anim_screens;
		anim = new AnimationSequence(_screen, _system, curr_anim->resourceId, 
			coll.getPalette(curr_anim->paletteIndex), fadeIn);
		if (curr_anim->initialPause != 0)  
			if (events.interruptableDelay(curr_anim->initialPause * 1000 / 50)) return true;

		result = false;
		switch (anim->show()) {
			case ABORT_NONE:
				if (curr_anim->endingPause != 0) {
					result = events.interruptableDelay(curr_anim->endingPause * 1000 / 50);
				}
				break;

			case ABORT_END_INTRO:
				result = true;
				break;

			case ABORT_NEXT_SCENE:
				break;
		}
		delete anim;

		if (result) {
			Sound.musicInterface_KillAll();	
			return true;
		}
	}

	// Show battle pictures one frame at a time

	result = false;
	anim = new AnimationSequence(_screen, _system, 0x48, coll.getPalette(4), false);
	do {
		result = events.interruptableDelay(2000);
		_screen.paletteFadeOut();
		if (!result) result = events.interruptableDelay(500);
		if (result) break;
	} while (anim->step());
	delete anim;
	
	if (!result) 
		// Show final introduction screen
		showScreen(0x22, 0x21, 10000);

	Sound.musicInterface_KillAll();
	return false;
}

} // end of namespace Lure
