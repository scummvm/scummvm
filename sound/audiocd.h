/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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

#ifndef SOUND_AUDIOCD_H
#define SOUND_AUDIOCD_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/singleton.h"
#include "sound/mixer.h"

class DigitalTrackInfo {
public:
	virtual bool error() = 0;
	virtual void play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration) = 0;
	virtual ~DigitalTrackInfo() { }
};


class AudioCDManager : public Common::Singleton<AudioCDManager> {
public:
	struct Status {
		bool playing;
		int track;
		int start;
		int duration;
		int numLoops;
	};

	void play(int track, int numLoops, int startFrame, int duration);
	void stop();
	bool isPlaying() const;

	void updateCD();

	Status getStatus() const;

private:
	friend class Common::Singleton<SingletonBaseType>;
	AudioCDManager();

	int getCachedTrack(int track);

private:
	/* used for emulated CD music */
	struct ExtStatus : Status {
		Audio::SoundHandle handle;
	};
	ExtStatus _cd;

	enum {
#if defined(__PSP__)
		CACHE_TRACKS = 4	//the PSP can't have more than 8 files open simultaneously
					//so don't use more than 4 filehandles for CD tracks
#else
		CACHE_TRACKS = 10
#endif
	};
	int _cachedTracks[CACHE_TRACKS];
	DigitalTrackInfo *_trackInfo[CACHE_TRACKS];
	int _currentCache;

};

/** Shortcut for accessing the audio CD manager. */
#define AudioCD		AudioCDManager::instance()

#endif
