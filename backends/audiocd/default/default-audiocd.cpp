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

#include "backends/audiocd/default/default-audiocd.h"
#include "audio/audiostream.h"
#include "common/config-manager.h"
#include "common/system.h"

DefaultAudioCDManager::DefaultAudioCDManager() {
	_cd.playing = false;
	_cd.track = 0;
	_cd.start = 0;
	_cd.duration = 0;
	_cd.numLoops = 0;
	_cd.volume = Audio::Mixer::kMaxChannelVolume;
	_cd.balance = 0;
	_mixer = g_system->getMixer();
	_emulating = false;
	assert(_mixer);
}

DefaultAudioCDManager::~DefaultAudioCDManager() {
	// Subclasses should call close as well
	close();
}

bool DefaultAudioCDManager::open() {
	// For emulation, opening is always valid
	close();
	return true;
}

void DefaultAudioCDManager::close() {
	// Only need to stop for emulation
	stop();
}

bool DefaultAudioCDManager::play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) {
	stop();

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
		Audio::SeekableAudioStream *stream = 0;

		for (int i = 0; !stream && i < 2; ++i)
			stream = Audio::SeekableAudioStream::openStreamFile(trackName[i]);

		if (stream != 0) {
			Audio::Timestamp start = Audio::Timestamp(0, startFrame, 75);
			Audio::Timestamp end = duration ? Audio::Timestamp(0, startFrame + duration, 75) : stream->getLength();

			/*
			FIXME: Seems numLoops == 0 and numLoops == 1 both indicate a single repetition,
			while all other positive numbers indicate precisely the number of desired
			repetitions. Finally, -1 means infinitely many
			*/
			_emulating = true;
			_mixer->playStream(soundType, &_handle,
			                        Audio::makeLoopingAudioStream(stream, start, end, (numLoops < 1) ? numLoops + 1 : numLoops), -1, _cd.volume, _cd.balance);
			return true;
		}
	}

	return false;
}

void DefaultAudioCDManager::stop() {
	if (_emulating) {
		// Audio CD emulation
		_mixer->stopHandle(_handle);
		_emulating = false;
	}
}

bool DefaultAudioCDManager::isPlaying() const {
	// Audio CD emulation
	if (_emulating)
		return _mixer->isSoundHandleActive(_handle);

	// The default class only handles emulation
	return false;
}

void DefaultAudioCDManager::setVolume(byte volume) {
	_cd.volume = volume;

	// Audio CD emulation
	if (_emulating && isPlaying())
		_mixer->setChannelVolume(_handle, _cd.volume);
}

void DefaultAudioCDManager::setBalance(int8 balance) {
	_cd.balance = balance;

	// Audio CD emulation
	if (_emulating && isPlaying())
		_mixer->setChannelBalance(_handle, _cd.balance);
}

void DefaultAudioCDManager::update() {
	if (_emulating) {
		// Check whether the audio track stopped playback
		if (!_mixer->isSoundHandleActive(_handle)) {
			// FIXME: We do not update the numLoops parameter here (and in fact,
			// currently can't do that). Luckily, only one engine ever checks
			// this part of the AudioCD status, namely the SCUMM engine; and it
			// only checks whether the track is currently set to infinite looping
			// or not.
			_emulating = false;
		}
	}
}

DefaultAudioCDManager::Status DefaultAudioCDManager::getStatus() const {
	Status info = _cd;
	info.playing = isPlaying();
	return info;
}

bool DefaultAudioCDManager::openRealCD() {
	Common::String cdrom = ConfMan.get("cdrom");

	// Try to parse it as an int
	char *endPos;
	int drive = strtol(cdrom.c_str(), &endPos, 0);

	// If not an integer, treat as a drive path
	if (endPos == cdrom.c_str())
		return openCD(cdrom);

	if (drive < 0)
		return false;

	return openCD(drive);
}

