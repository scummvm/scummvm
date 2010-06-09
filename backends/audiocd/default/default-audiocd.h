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

#ifndef BACKENDS_AUDIOCD_DEFAULT_H
#define BACKENDS_AUDIOCD_DEFAULT_H

#include "common/noncopyable.h"
#include "sound/mixer.h"

class DefaultAudioCDManager : Common::NonCopyable {
public:
	DefaultAudioCDManager();
	virtual ~DefaultAudioCDManager() {}

	struct Status {
		bool playing;
		int track;
		int start;
		int duration;
		int numLoops;
	};

	// Emulated CD functions, engines should call this functions
	void play(int track, int numLoops, int startFrame, int duration, bool only_emulate = false);
	void stop();
	bool isPlaying() const;
	void update();
	virtual Status getStatus() const; // Subclasses should override for better status results

protected:

	// Real CD functions. Let Subclasses implement the real code
	virtual bool openCD(int drive) { return false; }
	virtual void updateCD() {}
	virtual bool pollCD() const { return false; }
	virtual void playCD(int track, int num_loops, int start_frame, int duration) {}
	virtual void stopCD() {}

	Audio::SoundHandle _handle;
	bool _emulating;

	Status _cd;
	Audio::Mixer *_mixer;
};

#endif
