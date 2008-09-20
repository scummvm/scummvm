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

#include "sound/audiocd.h"
#include "sound/audiostream.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"
#include "engines/engine.h"
#include "common/util.h"
#include "common/system.h"

DECLARE_SINGLETON(Audio::AudioCDManager);

namespace Audio {

AudioCDManager::AudioCDManager() {
	_cd.playing = false;
	_cd.track = 0;
	_cd.start = 0;
	_cd.duration = 0;
	_cd.numLoops = 0;
	_mixer = g_system->getMixer();
	assert(_mixer);
}

void AudioCDManager::play(int track, int numLoops, int startFrame, int duration) {
	if (numLoops != 0 || startFrame != 0) {
		_cd.track = track;
		_cd.numLoops = numLoops;
		_cd.start = startFrame;
		_cd.duration = duration;

		// Try to load the track from a compressed data file, and if found, use
		// that. If not found, attempt to start regular Audio CD playback of
		// the requested track.
		char trackName[2][16];
		sprintf(trackName[0], "track%d", track);
		sprintf(trackName[1], "track%02d", track);
		Audio::AudioStream *stream = 0;

		for (int i = 0; !stream && i < 2; ++i) {
			/*
			FIXME: Seems numLoops == 0 and numLoops == 1 both indicate a single repetition,
			while all other positive numbers indicate precisely the number of desired
			repetitions. Finally, -1 means infinitely many
			*/
			// We multiply by 40 / 3 = 1000 / 75 to convert frames to milliseconds
			stream = AudioStream::openStreamFile(trackName[i], startFrame * 40 / 3, duration * 40 / 3, (numLoops < 1) ? numLoops + 1 : numLoops);
		}

		// Stop any currently playing emulated track
		_mixer->stopHandle(_cd.handle);

		// HACK: We abuse _cd.playing to store whether we are playing a real or an emulated track.
		if (stream != 0) {
			_cd.playing = true;
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_cd.handle, stream);
		} else {
			_cd.playing = false;
			g_system->playCD(track, numLoops, startFrame, duration);
		}
	}
}

void AudioCDManager::stop() {
	if (_cd.playing) {
		// Audio CD emulation
		_mixer->stopHandle(_cd.handle);
		_cd.playing = false;
	} else {
		// Real Audio CD
		g_system->stopCD();
	}
}

bool AudioCDManager::isPlaying() const {
	if (_cd.playing) {
		// Audio CD emulation
		return _mixer->isSoundHandleActive(_cd.handle);
	} else {
		// Real Audio CD
		return g_system->pollCD();
	}
}

void AudioCDManager::updateCD() {
	if (_cd.playing) {
		// Check whether the audio track stopped playback
		if (!_mixer->isSoundHandleActive(_cd.handle)) {
			// FIXME: We do not update the numLoops parameter here (and in fact,
			// currently can't do that). Luckily, only one engine ever checks
			// this part of the AudioCD status, namely the SCUMM engine; and it
			// only checks whether the track is currently set to infinite looping
			// or not.
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

} // End of namespace Audio
