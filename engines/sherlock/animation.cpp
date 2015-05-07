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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/animation.h"
#include "sherlock/sherlock.h"
#include "common/algorithm.h"

namespace Sherlock {

// The following are a list of filenames played in the prologue that have
// special effects associated with them at specific frames

#define FRAMES_END 32000
#define PROLOGUE_NAMES_COUNT 6
#define TITLE_NAMES_COUNT 7
static const char *const PROLOGUE_NAMES[6] = {
	"subway1", "subway2", "finale2", "suicid", "coff3", "coff4"
};
static const int PROLOGUE_FRAMES[6][9] = {
	{ 4, 26, 54, 72, 92, 134, FRAMES_END },
	{ 2, 80, 95, 117, 166, FRAMES_END },
	{ 1, FRAMES_END },
	{ 42, FRAMES_END },
	{ FRAMES_END },
	{ FRAMES_END }
};

// Title animations file list
static const char *const TITLE_NAMES[7] = {
	"27pro1", "14note", "coff1", "coff2", "coff3", "coff4", "14kick"
};

static const int TITLE_FRAMES[7][9] = {
	{ 29, 131, FRAMES_END },
	{ 55, 80, 95, 117, 166, FRAMES_END },
	{ 15, FRAMES_END },
	{ 4, 37, 92, FRAMES_END },
	{ 2, 43, FRAMES_END },
	{ 2, FRAMES_END },
	{ 10, 50, FRAMES_END }
};

static const int NO_FRAMES = FRAMES_END;

Animation::Animation(SherlockEngine *vm): _vm(vm) {
}

/**
 * Play a full-screen animation
 */
bool Animation::play(const Common::String &filename, int minDelay, int fade,
		bool setPalette, int speed) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	int soundNumber = 0;
	sound._playingEpilogue = true;

	// Check for any any sound frames for the given animation
	const int *soundFrames = checkForSoundFrames(filename);

	// Strip any extension off of the passed filename and add .vdx suffix
	Common::String baseName = filename;
	const char *p = strchr(baseName.c_str(), '.');
	if (p)
		baseName = Common::String(filename.c_str(), MIN(p - 1, baseName.c_str() + 7));

	Common::String vdxName = baseName + ".vdx";

	// Load the animation
	Common::SeekableReadStream *stream;
	if (!_vm->_titleOverride.empty())
		stream = _vm->_res->load(vdxName, _vm->_titleOverride);
	else if (_vm->_useEpilogue2)
		stream = _vm->_res->load(vdxName, "epilog2.lib");
	else
		stream = _vm->_res->load(vdxName, "epilogue.lib");

	// Load initial image
	Common::String vdaName = baseName + ".vda";
	ImageFile images(vdaName, true, true);

	events.wait(minDelay);
	if (fade != 0 && fade != 255)
		screen.fadeToBlack();

	if (setPalette) {
		if (fade != 255)
			screen.setPalette(images._palette);
	}

	int frameNumber = 0;
	Common::Point pt;
	bool skipped = false;
	while (!_vm->shouldQuit()) {
		// Get the next sprite to display
		int imageFrame = stream->readSint16LE();

		if (imageFrame == -2) {
			// End of animation reached
			break;
		} else if (imageFrame != -1) {
			// Read position from either animation stream or the sprite frame itself
			if (imageFrame < 0) {
				imageFrame += 32768;
				pt.x = stream->readUint16LE();
				pt.y = stream->readUint16LE();
			} else {
				pt = images[imageFrame]._offset;
			}

			// Draw the sprite. Note that we explicitly use the raw frame below, rather than the ImageFrame,
			// since we don't want the offsets in the image file to be used, just the explicit position we specify
			screen.transBlitFrom(images[imageFrame]._frame, pt);
		} else {
			// No sprite to show for this animation frame
			if (fade == 255) {
				// Gradual fade in
				if (screen.equalizePalette(images._palette) == 0)
					fade = 0;
			}

			// Check if we've reached a frame with sound
			if (frameNumber++ == *soundFrames) {
				++soundNumber;
				++soundFrames;
				Common::String fname = _vm->_soundOverride.empty() ?
					Common::String::format("%s%01d", baseName.c_str(), soundNumber) :
					Common::String::format("%s%02d", baseName.c_str(), soundNumber);

				if (sound._voices)
					sound.playSound(fname);
			}

			events.wait(speed);
		}

		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();
			if (keyState.keycode == Common::KEYCODE_ESCAPE ||
				keyState.keycode == Common::KEYCODE_SPACE) {
				skipped = true;
				break;
			}
		} else if (events._pressed) {
			skipped = true;
			break;
		}
	}

	events.clearEvents();
	sound.stopSound();
	delete stream;
	sound._playingEpilogue = false;

	return !skipped && !_vm->shouldQuit();
}

/**
 * Checks for whether an animation is being played that has associated sound
 */
const int *Animation::checkForSoundFrames(const Common::String &filename) {
	const int *frames = &NO_FRAMES;

	if (_vm->_soundOverride.empty()) {
		for (int idx = 0; idx < PROLOGUE_NAMES_COUNT; ++idx) {
			if (!scumm_stricmp(filename.c_str(), PROLOGUE_NAMES[idx])) {
				frames = &PROLOGUE_FRAMES[idx][0];
				break;
			}
		}
	} else {
		for (int idx = 0; idx < TITLE_NAMES_COUNT; ++idx) {
			if (!scumm_stricmp(filename.c_str(), TITLE_NAMES[idx])) {
				frames = &TITLE_FRAMES[idx][0];
				break;
			}
		}
	}

	return frames;
}

} // End of namespace Sherlock
