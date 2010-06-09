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
 * $URL$
 * $Id$
 *
 */

#ifndef BACKENDS_AUDIOCD_ABSTRACT_H
#define BACKENDS_AUDIOCD_ABSTRACT_H

#include "common/noncopyable.h"

class AudioCDManager : Common::NonCopyable {
public:
	virtual ~AudioCDManager() {}

	struct Status {
		bool playing;
		int track;
		int start;
		int duration;
		int numLoops;
	};

	// Emulated CD functions, engines should call these functions
	virtual void play(int track, int numLoops, int startFrame, int duration, bool only_emulate = false) = 0;
	virtual void stop() = 0;
	virtual bool isPlaying() const = 0;
	virtual void update() = 0;
	virtual Status getStatus() const = 0;

	// Real CD functions. Let Subclasses implement the real code
	virtual bool openCD(int drive) { return false; }
	virtual void updateCD() {}
	virtual bool pollCD() const { return false; }
	virtual void playCD(int track, int num_loops, int start_frame, int duration) {}
	virtual void stopCD() {}
};

#endif
