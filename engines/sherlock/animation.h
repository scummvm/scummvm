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

#ifndef SHERLOCK_ANIMATION_H
#define SHERLOCK_ANIMATION_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/array.h"

namespace Sherlock {

#define FRAMES_END 32000

class SherlockEngine;

class Animation {
private:
	SherlockEngine *_vm;

	Common::Array<const char *> _prologueNames;
	Common::Array<Common::Array<int> > _prologueFrames;
	Common::Array<const char *> _titleNames;
	Common::Array<Common::Array<int> > _titleFrames;

	/**
	 * Checks for whether an animation is being played that has associated sound
	 */
	const int *checkForSoundFrames(const Common::String &filename, bool intro);
public:
	Common::String _soundLibraryFilename;
	Common::String _gfxLibraryFilename;

public:
	Animation(SherlockEngine *vm);

	/**
	 * Load the prologue name array
	 */
	void setPrologueNames(const char *const *names, int count);

	/**
	 * Load the prologue frame array
	 */
	void setPrologueFrames(const int *frames, int count, int maxFrames);

	/**
	 * Load the title name array
	 */
	void setTitleNames(const char *const *names, int count);

	/**
	 * Load the title frame array
	 */
	void setTitleFrames(const int *frames, int count, int maxFrames);

	/**
	 * Play a full-screen animation
	 */
	bool play(const Common::String &filename, bool intro, int minDelay, int fade, bool setPalette, int speed);

	bool play3DO(const Common::String &filename, bool intro, int minDelay, bool fadeFromGrey, int speed);
};

} // End of namespace Sherlock

#endif
