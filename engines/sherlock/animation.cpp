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
	_useEpilogue2 = false;
}

void Animation::playPrologue(const Common::String &filename, int minDelay, int fade, 
		bool setPalette, int speed) {
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
	if (!_titleOverride.empty())
		stream = _vm->_res->load(vdxName, _titleOverride);
	else if (_useEpilogue2)
		stream = _vm->_res->load(vdxName, "epilog2.lib");
	else
		stream = _vm->_res->load(vdxName, "epilogoue.lib");
	int resoucreIndex = _vm->_res->resouceIndex();

	// Load initial image
	//Common::String vdaName = baseName + ".vda";
	// TODO


	delete stream;
}

/**
 * Checks for whether an animation is being played that has associated sound
 */
const int *Animation::checkForSoundFrames(const Common::String &filename) {
	const int *frames = &NO_FRAMES;

	if (!_soundOverride.empty()) {
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
