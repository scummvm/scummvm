/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "sound/audiocd.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"
#include "base/engine.h"
#include "common/file.h"
#include "common/util.h"

struct TrackFormat {
	/** Decodername */
	const char* decoderName;
	/** 
	 * Pointer to a function which tries to open the specified track - the only argument
	 * is the number of the track to be played.
	 * Returns either the DigitalTrackInfo object representing the requested track or null
	 * in case of an error
	 */
	DigitalTrackInfo* (*openTrackFunction)(int);
};

static const TrackFormat TRACK_FORMATS[] = {
	/* decoderName,		openTrackFunction */ 
#ifdef USE_FLAC
	{ "Flac",			getFlacTrack },
#endif // #ifdef USE_FLAC
#ifdef USE_VORBIS
	{ "Ogg Vorbis",		getVorbisTrack },
#endif // #ifdef USE_VORBIS
#ifdef USE_MAD
	{ "Mpeg Layer 3",	getMP3Track },
#endif // #ifdef USE_MAD

	{ NULL, NULL } // Terminator
};


DECLARE_SINGLETON(AudioCDManager);

AudioCDManager::AudioCDManager() {
	memset(&_cd, 0, sizeof(_cd));
	memset(_cached_tracks, 0, sizeof(_cached_tracks));
	memset(_track_info, 0, sizeof(_track_info));
	_current_cache = 0;
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
			g_engine->_mixer->stopHandle(_cd.handle);
			_cd.playing = true;
			_track_info[index]->play(g_engine->_mixer, &_cd.handle, _cd.start, _cd.duration);
		} else {
			g_system->playCD(track, numLoops, startFrame, duration);
		}
	}
}

void AudioCDManager::stop() {
	if (_cd.playing) {
		g_engine->_mixer->stopHandle(_cd.handle);
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
		// If the sound handle is 0, then playback stopped.
		if (!_cd.handle.isActive()) {
			// If playback just stopped, check if the current track is supposed
			// to be repeated, and if that's the case, play it again. Else, stop
			// the CD explicitly.
			if (_cd.numLoops == -1 || --_cd.numLoops > 0) {
				int index = getCachedTrack(_cd.track);
				assert(index >= 0);
				_track_info[index]->play(g_engine->_mixer, &_cd.handle, _cd.start, _cd.duration);
			} else {
				g_engine->_mixer->stopHandle(_cd.handle);
				_cd.playing = false;
			}
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
	int i;
	int current_index;

	// See if we find the track in the cache
	for (i = 0; i < CACHE_TRACKS; i++)
		if (_cached_tracks[i] == track) {
			if (_track_info[i])
				return i;
			else
				return -1;
		}
	current_index = _current_cache++;
	_current_cache %= CACHE_TRACKS;

	// Not found, see if it exists

	// First, delete the previous track info object
	delete _track_info[current_index];
	_track_info[current_index] = NULL;

	_cached_tracks[current_index] = track;

	for (i = 0; i < ARRAYSIZE(TRACK_FORMATS)-1 && _track_info[current_index] == NULL; ++i)
		_track_info[current_index] = TRACK_FORMATS[i].openTrackFunction(track);

	if (_track_info[current_index] != NULL)
		return current_index;

	debug(2, "Track %d not available in compressed format", track);
	return -1;
}
