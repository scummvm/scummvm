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

namespace Lure {

struct AnimRecord {
	uint16 resourceId;
	uint8 paletteIndex;
	bool initialPause;
	bool endingPause;
};

static const uint16 start_screens[] = {0x18, 0x1A, 0x1E, 0x1C, 0};
static const AnimRecord anim_screens[] = {{0x40, 0, true, true}, {0x42, 1, false, true}, 
	{0x44, 2, false, false}, {0x24, 3, false, true}, {0x46, 3, false, false}, 
	{0, 0, false, false}};

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

	AnimationSequence *anim;
	bool result;

	// Animated screens

	PaletteCollection coll(0x32);
	const AnimRecord *curr_anim = anim_screens;
	for (; curr_anim->resourceId; ++curr_anim)
	{
		bool fadeIn = curr_anim == anim_screens;
		anim = new AnimationSequence(_screen, _system, curr_anim->resourceId, 
			coll.getPalette(curr_anim->paletteIndex), fadeIn);
		if (curr_anim->initialPause) 
			if (events.interruptableDelay(12000)) return true;

		result = false;
		switch (anim->show()) {
			case ABORT_NONE:
				if (curr_anim->endingPause) {
					result = events.interruptableDelay(12000);
				}
				break;

			case ABORT_END_INTRO:
				result = true;
				break;

			case ABORT_NEXT_SCENE:
				break;
		}
		delete anim;

		if (result) return true;
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
	if (result) return true;

	// Show final introduction screen

	showScreen(0x22, 0x21, 10000);

	return false;
}

} // end of namespace Lure
