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

#include "common/stdafx.h"

#include "sound/audiocd.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"
#include "engines/engine.h"
#include "common/file.h"
#include "common/util.h"
#include "common/system.h"

DECLARE_SINGLETON(Audio::AudioCDManager);

namespace Audio {

struct TrackFormat {
	/** Decodername */
	const char* decoderName;
	/**
	 * Pointer to a function which tries to open the specified track - the only argument
	 * is the number of the track to be played.
	 * Returns either a DigitalTrackInfo object representing the requested track or null
	 * in case of an error
	 */
	DigitalTrackInfo* (*openTrackFunction)(int);
};

static const TrackFormat s_trackFormats[] = {
	/* decoderName,		openTrackFunction */
#ifdef USE_VORBIS
	{ "Ogg Vorbis",		getVorbisTrack },
#endif
#ifdef USE_MAD
	{ "MPEG Layer 3",	getMP3Track },
#endif
#ifdef USE_FLAC
	{ "Flac",			getFlacTrack },
#endif

	{ NULL, NULL } // Terminator
};


AudioCDManager::AudioCDManager() {
	memset(_cachedTracks, 0, sizeof(_cachedTracks));
	memset(_trackInfo, 0, sizeof(_trackInfo));
	_cd.playing = false;
	_cd.track = 0;
	_cd.start = 0;
	_cd.duration = 0;
	_cd.numLoops = 0;
	_currentCacheIdx = 0;
	_mixer = g_system->getMixer();
	assert(_mixer);
}

void AudioCDManager::play(int track, int numLoops, int startFrame, int duration) {
	if (numLoops != 0 || startFrame != 0) {
		// Try to load the track from a .mp3/.ogg file, and if found, use
		// that. If not found, attempt to do regular Audio CD playback of
		// the requested track.
		int index = getCachedTrack(track);

		_cd.track = track;
		_cd.numLoops = numLoops;
		_cd.start = startFrame;
		_cd.duration = duration;

		if (index >= 0) {
			_mixer->stopHandle(_cd.handle);
			_cd.playing = true;
			/*
			FIXME: Seems numLoops == 0 and numLoops == 1 both indicate a single repetition,
			while all other positive numbers indicate precisely the number of desired
			repetitions. Finally, -1 means infinitely many
			*/
			numLoops = (numLoops < 1) ? numLoops + 1 : numLoops;
			_trackInfo[index]->play(_mixer, &_cd.handle, numLoops, _cd.start, _cd.duration);
		} else {
			g_system->playCD(track, numLoops, startFrame, duration);
			_cd.playing = false;
		}
	}
}

void AudioCDManager::stop() {
	if (_cd.playing) {
		_mixer->stopHandle(_cd.handle);
		_cd.playing = false;
	} else {
		g_system->stopCD();
	}
}

bool AudioCDManager::isPlaying() const {
	return _cd.playing || g_system->pollCD();
}

void AudioCDManager::updateCD() {
	if (_cd.playing) {
		// Check whether the audio track stopped playback
		if (!_mixer->isSoundHandleActive(_cd.handle)) {
			// FIXME: We do not update the numLoops parameter here (and in fact,
			// currently can't do that). Luckily, only one engine ever checks
			// this part of the AudioCD status, namely the SCUMM engine; and it
			// only checks
			_cd.playing = false;
		}
	} else {
		g_system->updateCD();
	}
}

AudioCDManager::Status AudioCDManager::getStatus() const {
	// TODO: This could be improved for "real" CD playback.
	// But to do that, we would have to extend the OSystem interface.
	Status info = _cd;
	info.playing = isPlaying();
	return info;
}

int AudioCDManager::getCachedTrack(int track) {
	// See if we find the track in the cache
	for (int i = 0; i < CACHE_TRACKS; i++)
		if (_cachedTracks[i] == track) {
			return _trackInfo[i] ? i : -1;
		}

	// The track is not already in the cache. Try and see if
	// we can load it.
	DigitalTrackInfo *newTrack = 0;
	for (const TrackFormat *format = s_trackFormats;
	     format->openTrackFunction != NULL && newTrack == NULL;
	     ++format) {
		newTrack = format->openTrackFunction(track);
	}

	int currentIndex = -1;

	if (newTrack != NULL) {
		// We successfully loaded a digital track. Store it into _trackInfo.

		currentIndex = _currentCacheIdx++;
		_currentCacheIdx %= CACHE_TRACKS;
	
		// First, delete the previous track info object
		delete _trackInfo[currentIndex];

		// Then, store the new track info object
		_trackInfo[currentIndex] = newTrack;
		_cachedTracks[currentIndex] = track;
	} else {
		debug(2, "Track %d not available in compressed format", track);
	}

	return currentIndex;
}

} // End of namespace Audio
