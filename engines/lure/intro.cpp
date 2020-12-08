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
 */

#include "lure/lure.h"
#include "lure/intro.h"
#include "lure/animseq.h"
#include "lure/events.h"
#include "lure/sound.h"

namespace Lure {

struct AnimRecord {
	// The resource ID of the animation
	uint16 resourceId;
	// The index of the palette to use
	uint8 paletteIndex;
	// The time that should pass before starting the animation
	uint16 initialPause;
	// The time that should pass at the last screen of the animation
	uint16 endingPause;
	// The number of the sound that should play during the animation
	uint8 soundNumber;
	// True if the sound should fade out before the transition
	bool fadeOutSound;
	// The time that should pass on the last animation screen before the
	// next sound is played
	uint16 soundTransitionPause;
	// The number of the sound that should be played on the last screen
	// (0xFF for none)
	uint8 soundNumber2;
};

static const uint16 start_screens[] = {0x18, 0x1A, 0x1E, 0x1C, 0};
static const AnimRecord anim_screens[] = {
	// Note that the ending pause of the first anim is about 8 seconds in the
	// original interpreter vs about 14 seconds here. 8 seconds is quite short
	// to read the screen, so I kept it at 14. Conveniently, the first music
	// track is much longer than what was actually used in the game.
	{0x40, 0, 0x314, 0x2BE, 0x00, true, 0x1F4, 0x01},		// The kingdom was at peace
	{0x42, 1, 0, 0x5FA, 0x01, false, 0, 0xFF},				// Cliff overhang
	{0x44, 2, 0, 0, 0x02, false, 0, 0xFF},					// Siluette in moonlight
	{0x24, 3, 0, 0x62C + 0x24, 0xFF, false, 0x328, 0x03},	// Exposition of reaching town
	{0x46, 3, 0, 0, 0x03, false, 0, 0xFF},					// Skorl approaches
	{0, 0, 0, 0, 0xFF, false, 0, 0xFF}};

// showScreen
// Shows a screen by loading it from the given resource, and then fading it in
// with a palette in the following resource. Returns true if the introduction
// should be aborted

bool Introduction::showScreen(uint16 screenId, uint16 paletteId, uint16 delaySize, bool fadeOut) {
	Screen &screen = Screen::getReference();
	bool isEGA = LureEngine::getReference().isEGA();
	screen.screen().loadScreen(screenId);
	screen.update();
	Palette p(paletteId);

	if (LureEngine::getReference().shouldQuit()) return true;

	if (isEGA) screen.setPalette(&p);
	else screen.paletteFadeIn(&p);

	bool result = interruptableDelay(delaySize);
	if (LureEngine::getReference().shouldQuit()) return true;

	if (fadeOut && !isEGA)
		screen.paletteFadeOut();

	return result;
}

// interruptableDelay
// Delays for a given number of milliseconds. If it returns true, it indicates that
// the Escape has been pressed to abort whatever sequence is being displayed

bool Introduction::interruptableDelay(uint32 milliseconds) {
	Events &events = Events::getReference();

	if (events.interruptableDelay(milliseconds)) {
		if (events.type() == Common::EVENT_KEYDOWN)
			return events.event().kbd.keycode == 27;
		else if (LureEngine::getReference().shouldQuit())
			return true;
		else if (events.type() == Common::EVENT_LBUTTONDOWN)
			return false;
	}

	return false;
}

// show
// Main method for the introduction sequence

bool Introduction::show() {
	Screen &screen = Screen::getReference();
	bool isEGA = LureEngine::getReference().isEGA();
	screen.setPaletteEmpty();

	// Initial game company and then game screen

	for (int ctr = 0; ctr < 3; ++ctr)
		if (showScreen(start_screens[ctr], start_screens[ctr] + 1, 5000))
			return true;

	// Title screen
	if (showScreen(start_screens[3], start_screens[3] + 1, 5000, false))
		return true;

	bool result = Sound.initCustomTimbres(true);
	if (result)
		return true;

	// Fade out title screen
	if (!isEGA)
		screen.paletteFadeOut();

	PaletteCollection coll(0x32);
	Palette EgaPalette(0x1D);

	// Animated screens

	AnimationSequence *anim;
	_currentSound = 0xFF;
	const AnimRecord *curr_anim = anim_screens;
	for (; curr_anim->resourceId; ++curr_anim) {
		// Handle sound selection
		playMusic(curr_anim->soundNumber, false);

		bool fadeIn = curr_anim == anim_screens;
		anim = new AnimationSequence(curr_anim->resourceId,
			isEGA ? EgaPalette : coll.getPalette(curr_anim->paletteIndex), fadeIn,
			(curr_anim->resourceId == 0x44) ? 4 : 7);

		if (curr_anim->initialPause != 0) {
			if (interruptableDelay(curr_anim->initialPause * 1000 / 50)) {
				delete anim;
				return true;
			}
		}

		result = false;
		switch (anim->show()) {
		case ABORT_NONE:
			if (curr_anim->endingPause != 0) {
				if (curr_anim->soundTransitionPause != 0) {
					uint16 pause = curr_anim->soundTransitionPause * 1000 / 50;
					if (curr_anim->fadeOutSound) {
						pause -= 3500;
					}
					// Wait before transitioning to the next track
					result = interruptableDelay(pause);
				}

				if (!result)
					result = playMusic(curr_anim->soundNumber2, curr_anim->fadeOutSound);

				if (!result)
					// Wait remaining time before the next animation
					result = interruptableDelay((curr_anim->endingPause - curr_anim->soundTransitionPause) * 1000 / 50);
			}
			break;

		case ABORT_END_INTRO:
			result = true;
			break;

		case ABORT_NEXT_SCENE:
		default:
			break;
		}
		delete anim;

		if (result) {
			Sound.musicInterface_KillAll();
			return true;
		}
	}

	// Fade out last cutscene screen
	if (!isEGA)
		screen.paletteFadeOut();

	// Show battle pictures one frame at a time

	result = false;
	anim = new AnimationSequence(0x48, isEGA ? EgaPalette : coll.getPalette(4), false);
	do {
		result = interruptableDelay(2000);
		if (isEGA) {
			screen.empty();
		} else {
			screen.paletteFadeOut();
		}
		if (!result) result = interruptableDelay(500);
		if (result) break;
	} while (anim->step());
	delete anim;

	if (!result) {
		// Show final introduction animation
		if (!isEGA)
			showScreen(0x22, 0x21, 33300);
		else {
			Palette finalPalette(0x21);
			anim = new AnimationSequence(0x22, finalPalette, false);
			delete anim;
			interruptableDelay(34000);
		}
	}

	Sound.musicInterface_KillAll();
	return false;
}

bool Introduction::playMusic(uint8 soundNumber, bool fadeOut) {
	bool result = false;

	if (soundNumber != 0xFF && _currentSound != soundNumber) {
		// Stop the previous sound
		if (fadeOut) {
			result = Sound.fadeOut();
			if (!result)
				result = interruptableDelay(500);
		} else {
			Sound.musicInterface_KillAll();
		}

		if (!result) {
			_currentSound = soundNumber;
			Sound.musicInterface_Play(_currentSound, 0, true);
		}
	}

	return result;
}

} // End of namespace Lure
