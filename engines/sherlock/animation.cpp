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

#include "sherlock/animation.h"
#include "sherlock/sherlock.h"
#include "common/algorithm.h"

namespace Sherlock {

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

	// Add on the VDX extension
	Common::String vdxName = filename + ".vdx";

	// Load the animation
	Common::SeekableReadStream *stream;
	if (!_vm->_titleOverride.empty())
		stream = _vm->_res->load(vdxName, _vm->_titleOverride);
	else if (_vm->_useEpilogue2)
		stream = _vm->_res->load(vdxName, "epilog2.lib");
	else
		stream = _vm->_res->load(vdxName, "epilogue.lib");

	// Load initial image
	Common::String vdaName = filename + ".vda";
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
			// At this point, either the sprites for the frame has been complete, or there weren't any sprites
			// at all to draw for the frame
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
					Common::String::format("%s%01d", filename.c_str(), soundNumber) :
					Common::String::format("%s%02d", filename.c_str(), soundNumber);

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
 * Load the prologue name array 
 */
void Animation::setPrologueNames(const char *const *names, int count) {
	for (int idx = 0; idx < count; ++idx, names++) {
		_prologueNames.push_back(*names);
	}
}

/**
 * Load the prologue frame array
 */
void Animation::setPrologueFrames(const int *frames, int count, int maxFrames) {
	_prologueFrames.resize(count);

	for (int idx = 0; idx < count; ++idx, frames + maxFrames) {
		_prologueFrames[idx].resize(maxFrames);
		Common::copy(frames, frames + maxFrames, &_prologueFrames[idx][0]);
	}
}

/**
 * Load the title name array
 */
void Animation::setTitleNames(const char *const *names, int count) {
	for (int idx = 0; idx < count; ++idx, names++) {
		_titleNames.push_back(*names);
	}
}

/**
 * Load the title frame array
 */
void Animation::setTitleFrames(const int *frames, int count, int maxFrames) {
	_titleFrames.resize(count);

	for (int idx = 0; idx < count; ++idx, frames + maxFrames) {
		_titleFrames[idx].resize(maxFrames);
		Common::copy(frames, frames + maxFrames, &_titleFrames[idx][0]);
	}
}

/**
 * Checks for whether an animation is being played that has associated sound
 */
const int *Animation::checkForSoundFrames(const Common::String &filename) {
	const int *frames = &NO_FRAMES;

	if (_vm->_soundOverride.empty()) {
		for (Common::Array<const char *>::size_type idx = 0; idx < _prologueNames.size(); ++idx) {
			if (filename.equalsIgnoreCase(_prologueNames[idx])) {
				frames = &_prologueFrames[idx][0];
				break;
			}
		}
	} else {
		for (Common::Array<const char *>::size_type idx = 0; idx < _titleNames.size(); ++idx) {
			if (filename.equalsIgnoreCase(_titleNames[idx])) {
				frames = &_titleFrames[idx][0];
				break;
			}
		}
	}

	return frames;
}

} // End of namespace Sherlock
