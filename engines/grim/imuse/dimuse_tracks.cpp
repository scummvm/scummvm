/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/grim/imuse/dimuse_engine.h"

namespace Grim {

int IMuseDigital::tracksInit() {
	_trackCount = DIMUSE_MAX_TRACKS;
	_tracksPauseTimer = 0;
	_trackList = nullptr;

	if (waveOutInit(&waveOutSettings))
		return -1;

	if (_internalMixer->init(waveOutSettings.bytesPerSample,
			waveOutSettings.numChannels,
			waveOutSettings.mixBuf,
			waveOutSettings.mixBufSize,
			waveOutSettings.sizeSampleKB,
			_trackCount) ||
			dispatchInit() ||
		streamerInit()) {
		return -1;
	}

	for (int l = 0; l < _trackCount; l++) {
		_tracks[l].index = l;
		_tracks[l].prev = nullptr;
		_tracks[l].next = nullptr;
		_tracks[l].dispatchPtr = dispatchGetDispatchByTrackId(l);
		_tracks[l].dispatchPtr->trackPtr = &_tracks[l];
		_tracks[l].soundId = 0;
		_tracks[l].group = 0;
		_tracks[l].marker = 0;
		_tracks[l].priority = 0;
		_tracks[l].vol = 0;
		_tracks[l].effVol = 0;
		_tracks[l].pan = 0;
		_tracks[l].detune = 0;
		_tracks[l].transpose = 0;
		_tracks[l].pitchShift = 0;
		_tracks[l].mailbox = 0;
		_tracks[l].jumpHook = 0;
	}

	return 0;
}

void IMuseDigital::tracksPause() {
	_tracksPauseTimer = 1;
}

void IMuseDigital::tracksResume() {
	_tracksPauseTimer = 0;
}

void IMuseDigital::tracksSaveLoad(Common::Serializer &ser) {
	Common::StackLock lock(*_mutex);
	dispatchSaveLoad(ser);

	for (int l = 0; l < _trackCount; l++) {
		ser.syncAsSint32LE(_tracks[l].soundId, VER(103));
		ser.syncAsSint32LE(_tracks[l].marker, VER(103));
		ser.syncAsSint32LE(_tracks[l].group, VER(103));
		ser.syncAsSint32LE(_tracks[l].priority, VER(103));
		ser.syncAsSint32LE(_tracks[l].vol, VER(103));
		ser.syncAsSint32LE(_tracks[l].effVol, VER(103));
		ser.syncAsSint32LE(_tracks[l].pan, VER(103));
		ser.syncAsSint32LE(_tracks[l].detune, VER(103));
		ser.syncAsSint32LE(_tracks[l].transpose, VER(103));
		ser.syncAsSint32LE(_tracks[l].pitchShift, VER(103));
		ser.syncAsSint32LE(_tracks[l].mailbox, VER(103));
		ser.syncAsSint32LE(_tracks[l].jumpHook, VER(103));
	}

	if (ser.isLoading()) {
		for (int l = 0; l < _trackCount; l++) {
			_tracks[l].prev = nullptr;
			_tracks[l].next = nullptr;
			_tracks[l].dispatchPtr = dispatchGetDispatchByTrackId(l);
			_tracks[l].dispatchPtr->trackPtr = &_tracks[l];
			if (_tracks[l].soundId) {
				addTrackToList(&_trackList, &_tracks[l]);
			}
		}

		dispatchRestoreStreamZones();
	}
}

void IMuseDigital::tracksSetGroupVol() {
	IMuseDigiTrack* curTrack = _trackList;
	while (curTrack) {
		curTrack->effVol = ((curTrack->vol + 1) * _groupsHandler->getGroupVol(curTrack->group)) / 128;
		curTrack = curTrack->next;
	}
}

void IMuseDigital::tracksCallback() {
	if (_tracksPauseTimer) {
		if (++_tracksPauseTimer < 3)
			return;
		_tracksPauseTimer = 3;
	}

	// This piece of code is responsible for adaptive buffer overrun correction:
	// it checks whether a buffer underrun has occurred within our output stream
	// and then it increments the buffer count.
	//
	// This is not part of the original implementation, but it's used to yield
	// smooth audio hopefully on every device.
	if (_internalMixer->_stream->endOfData() && _checkForUnderrun) {
		debug(5, "IMuseDigital::tracksCallback(): WARNING: audio buffer underrun, adapting the buffer queue count...");

		adaptBufferCount();

		// Allow the routine to cooldown: i.e. wait until the engine manages to
		// refill the stream with the most recent maximum number of queueable buffers.
		_underrunCooldown = _maxQueuedStreams;
		_checkForUnderrun = false;
	}

	// If we leave the number of queued streams unbounded, we fill the queue with streams faster than
	// we can play them: this leads to a very noticeable audio latency and desync with the graphics.
	if ((int)_internalMixer->_stream->numQueuedStreams() < _maxQueuedStreams) {
		dispatchPredictFirstStream();

		waveOutWrite(&_outputAudioBuffer, _outputFeedSize, _outputSampleRate);

		if (_outputFeedSize != 0) {
			// Let's see if we should check for buffer underruns...
			if (!_checkForUnderrun) {
				if (_underrunCooldown == 0) {
					_checkForUnderrun = true;
				} else {
					_underrunCooldown--;
				}
			}

			_internalMixer->clearMixerBuffer();

			if (!_tracksPauseTimer) {
				IMuseDigiTrack *track = _trackList;

				while (track) {
					IMuseDigiTrack *next = track->next;
					dispatchProcessDispatches(track, _outputFeedSize, _outputSampleRate);
					track = next;
				};
			}

			_internalMixer->loop(&_outputAudioBuffer, _outputFeedSize);
		}
	}
}

void IMuseDigital::tracksLowLatencyCallback() {
	// Why do we need a low latency mode?
	//
	// For every audio callback, this engine works by collecting all the sound
	// data for every track and by mixing it up in a single output stream.
	// This is exactly how the original executables worked, so our implementation
	// provides a very faithful recreation of that experience. And it comes with
	// a compromise that e.g. The Dig and Full Throttle didn't have to front:
	//
	// in order to provide glitchless audio, an appropriate stream queue size
	// has to be enforced: a longer queue yields a lower probability of audio glitches
	// but a higher latency, and viceversa. In our case, this depends on the audio backend
	// configuration. As such: some configurations might encounter audible latency (#13462).
	//
	// We solve this issue by offering this alternate low latency mode which, instead
	// of keeping a single stream for everything, creates (and disposes) streams on the fly
	// for every different sound. This means that whenever the new sound data is ready,
	// a new stream is initialized and played immediately, without having to wait for all
	// the other sounds to be processed and mixed in the same sample pool.

	if (_tracksPauseTimer) {
		if (++_tracksPauseTimer < 3)
			return;
		_tracksPauseTimer = 3;
	}

	// The callback path is heavily inspired from the original one (see tracksCallback()),
	// but it handles each track separatedly.

	dispatchPredictFirstStream();

	IMuseDigiTrack *track = _trackList;

	while (track) {

		IMuseDigiTrack *next = track->next;
		int idx = track->index;

		bool canQueueBufs = (int)_internalMixer->getStream(idx)->numQueuedStreams() < (_maxQueuedStreams + 1);

		if (canQueueBufs) {
			waveOutLowLatencyWrite(&_outputLowLatencyAudioBuffers[idx], _outputFeedSize, _outputSampleRate, idx);

			if (_outputFeedSize != 0) {
				// Ordinary audio tracks handling...
				_internalMixer->setCurrentMixerBuffer(_outputLowLatencyAudioBuffers[idx]);
				_internalMixer->clearMixerBuffer();

				if (!_tracksPauseTimer) {
					dispatchProcessDispatches(track, _outputFeedSize, _outputSampleRate);
				}

				_internalMixer->loop(&_outputLowLatencyAudioBuffers[idx], _outputFeedSize);

				// If, after processing the track dispatch, the sound is set to zero
				// it means that it has reached the end: let's notify its stream...
				if (track->soundId == 0) {
					_internalMixer->endStream(idx);
				}
			}
		}

		track = next;
	}
}

int IMuseDigital::tracksStartSound(int soundId, int tryPriority, int group) {
	int priority = clampNumber(tryPriority, 0, 127);

	debug(5, "IMuseDigital::tracksStartSound(): sound %d with priority %d and group %d", soundId, priority, group);
	IMuseDigiTrack *allocatedTrack = tracksReserveTrack(priority);

	if (!allocatedTrack) {
		debug(5, "IMuseDigital::tracksStartSound(): ERROR: couldn't find a spare track to allocate sound %d", soundId);
		return -6;
	}

	allocatedTrack->soundId = soundId;
	allocatedTrack->marker = 0;
	allocatedTrack->group = 0;
	allocatedTrack->priority = priority;
	allocatedTrack->vol = 127;
	allocatedTrack->effVol = _groupsHandler->getGroupVol(0);
	allocatedTrack->pan = 64;
	allocatedTrack->detune = 0;
	allocatedTrack->transpose = 0;
	allocatedTrack->pitchShift = 256;
	allocatedTrack->mailbox = 0;
	allocatedTrack->jumpHook = 0;

	if (dispatchAllocateSound(allocatedTrack, group)) {
		debug(5, "IMuseDigital::tracksStartSound(): ERROR: dispatch couldn't start sound %d", soundId);
		allocatedTrack->soundId = 0;
		return -1;
	}

	_mutex->lock();
	addTrackToList(&_trackList, allocatedTrack);
	_mutex->unlock();

	return 0;
}

int IMuseDigital::tracksStopSound(int soundId) {
	if (!_trackList)
		return -1;

	IMuseDigiTrack *nextTrack = _trackList;
	IMuseDigiTrack *curTrack;
	int result = -1;

	while (nextTrack) {
		curTrack = nextTrack;
		nextTrack = curTrack->next;
		if (curTrack->soundId == soundId) {
			tracksClear(curTrack);
			result = 0;
		}
	}

	return result;
}

int IMuseDigital::tracksStopAllSounds() {
	Common::StackLock lock(*_mutex);
	IMuseDigiTrack *nextTrack = _trackList;
	IMuseDigiTrack *curTrack;

	while (nextTrack) {
		curTrack = nextTrack;
		nextTrack = curTrack->next;
		tracksClear(curTrack);
	}

	_filesHandler->closeAllSounds();

	return 0;
}

int IMuseDigital::tracksGetNextSound(int soundId) {
	int foundSoundId = 0;
	IMuseDigiTrack *track = _trackList;
	while (track) {
		if (track->soundId > soundId) {
			if (!foundSoundId || track->soundId < foundSoundId) {
				foundSoundId = track->soundId;
			}
		}
		track = track->next;
	};

	return foundSoundId;
}

int IMuseDigital::tracksQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	if (!_trackList) {
		debug(5, "IMuseDigital::tracksQueryStream(): WARNING: empty trackList, ignoring call...");
		return -1;
	}

	IMuseDigiTrack *track = _trackList;
	do {
		if (track->soundId) {
			if (soundId == track->soundId && track->dispatchPtr->streamPtr) {
				streamerQueryStream(track->dispatchPtr->streamPtr, bufSize, criticalSize, freeSpace, paused);
				return 0;
			}
		}
		track = track->next;
	} while (track);

	debug(5, "IMuseDigital::tracksQueryStream(): WARNING: couldn't find sound %d in trackList, ignoring call...", soundId);
	return -1;
}

int IMuseDigital::tracksFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused) {
	if (!_trackList)
		return -1;

	IMuseDigiTrack *track = _trackList;
	do {
		if (track->soundId != 0) {
			if (track->soundId == soundId && track->dispatchPtr->streamPtr) {
				streamerFeedStream(track->dispatchPtr->streamPtr, srcBuf, sizeToFeed, paused);
				return 0;
			}
		}
		track = track->next;
	} while (track);

	return -1;
}

void IMuseDigital::tracksClear(IMuseDigiTrack *trackPtr) {
	Common::StackLock lock(*_mutex);

	removeTrackFromList(&_trackList, trackPtr);
	dispatchRelease(trackPtr);
	_fadesHandler->clearFadeStatus(trackPtr->soundId, -1);
	_triggersHandler->clearTrigger(trackPtr->soundId, _emptyMarker, -1);

	// Unlock the sound, if it's loaded as a resource
	// TODO
	// if (trackPtr->soundId < 1000 && trackPtr->soundId) {
	// 	_vm->_res->unlock(rtSound, trackPtr->soundId);
	// }

	if (_lowLatencyMode)
		waveOutEmptyBuffer(trackPtr->index);

	trackPtr->soundId = 0;
}

int IMuseDigital::tracksSetParam(int soundId, int opcode, int value) {
	if (!_trackList)
		return -4;

	IMuseDigiTrack *track = _trackList;
	while (track) {
		if (track->soundId == soundId) {
			switch (opcode) {
			case DIMUSE_P_GROUP:
				if (value >= 16)
					return -5;
				track->group = value;
				track->effVol = ((track->vol + 1) * _groupsHandler->getGroupVol(value)) / 128;
				return 0;
			case DIMUSE_P_PRIORITY:
				if (value > 127)
					return -5;
				track->priority = value;
				return 0;
			case DIMUSE_P_VOLUME:
				if (value > 127)
					return -5;
				track->vol = value;
				track->effVol = ((value + 1) * _groupsHandler->getGroupVol(track->group)) / 128;
				return 0;
			case DIMUSE_P_PAN:
				if (value > 127)
					return -5;
				track->pan = value;
				return 0;
			case DIMUSE_P_DETUNE:
				if (value < -9216 || value > 9216)
					return -5;
				track->detune = value;
				track->pitchShift = value + track->transpose * 256;
				return 0;
			case DIMUSE_P_TRANSPOSE:
				if (value < 0 || value > 4095)
					return -5;

				track->pitchShift = value;
				return 0;
			case DIMUSE_P_MAILBOX:
				track->mailbox = value;
				return 0;
			default:
				debug(5, "IMuseDigital::tracksSetParam(): unknown opcode %d", opcode);
				return -5;
			}
		}
		track = track->next;
	}

	return -4;
}

int IMuseDigital::tracksGetParam(int soundId, int opcode) {
	if (!_trackList) {
		if (opcode != DIMUSE_P_SND_TRACK_NUM)
			return -4;
		else
			return 0;
	}

	IMuseDigiTrack *track = _trackList;
	int l = 0;
	do {
		if (track->soundId == soundId) {
			switch (opcode) {
			case DIMUSE_P_BOGUS_ID:
				return -1;
			case DIMUSE_P_SND_TRACK_NUM:
				l++;
				break;
			case DIMUSE_P_TRIGS_SNDS:
				return -1;
			case DIMUSE_P_MARKER:
				return track->marker;
			case DIMUSE_P_GROUP:
				return track->group;
			case DIMUSE_P_PRIORITY:
				return track->priority;
			case DIMUSE_P_VOLUME:
				return track->vol;
			case DIMUSE_P_PAN:
				return track->pan;
			case DIMUSE_P_DETUNE:
				return track->detune;
			case DIMUSE_P_TRANSPOSE:
				return track->transpose;
			case DIMUSE_P_MAILBOX:
				return track->mailbox;
			case DIMUSE_P_SND_HAS_STREAM:
				return (track->dispatchPtr->streamPtr != 0);
			case DIMUSE_P_STREAM_BUFID:
				return track->dispatchPtr->streamBufID;
			case DIMUSE_P_SND_POS_IN_MS: // getCurSoundPositionInMs
				if (track->dispatchPtr->wordSize == 0)
					return 0;
				if (track->dispatchPtr->sampleRate == 0)
					return 0;
				if (track->dispatchPtr->channelCount == 0)
					return 0;
				return (track->dispatchPtr->currentOffset * 5) / (((track->dispatchPtr->wordSize / 8) * track->dispatchPtr->sampleRate * track->dispatchPtr->channelCount) / 200);
			default:
				return -5;
			}
		}

		track = track->next;
	} while (track);

	if (opcode != DIMUSE_P_SND_TRACK_NUM)
		return -4;

	return l;
}

int IMuseDigital::tracksSetHook(int soundId, int hookId) {
	if (hookId > 128)
		return -5;
	if (!_trackList)
		return -4;

	IMuseDigiTrack *track = _trackList;
	while (track->soundId != soundId) {
		track = track->next;
		if (!track)
			return -4;
	}

	track->jumpHook = hookId;

	return 0;
}

int IMuseDigital::tracksGetHook(int soundId) {
	if (!_trackList)
		return -4;

	IMuseDigiTrack *track = _trackList;
	while (track->soundId != soundId) {
		track = track->next;
		if (!track)
			return -4;
	}

	return track->jumpHook;
}

IMuseDigiTrack *IMuseDigital::tracksReserveTrack(int priority) {
	IMuseDigiTrack *curTrack;
	IMuseDigiTrack *reservedTrack = nullptr;
	int minPriorityFound;

	// Pick the track from the pool of free tracks
	for (int i = 0; i < _trackCount; i++) {
		if (!_tracks[i].soundId) {
			return &_tracks[i];
		}
	}

	// If no free track is found, steal the lower priority one
	curTrack = _trackList;
	for (minPriorityFound = 127; curTrack; curTrack = curTrack->next) {
		if (curTrack->priority <= minPriorityFound) {
			minPriorityFound = curTrack->priority;
			reservedTrack = curTrack;
		}
	}

	if (reservedTrack && priority >= minPriorityFound) {
		tracksClear(reservedTrack);
		return reservedTrack;
	}

	return nullptr;
}

void IMuseDigital::tracksDeinit() {
	tracksStopAllSounds();
}

} // End of namespace Grim
