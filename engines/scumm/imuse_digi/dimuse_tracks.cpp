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

#include "scumm/imuse_digi/dimuse_engine.h"

namespace Scumm {

int IMuseDigital::tracksInit() {
	_trackCount = 6;
	_tracksPauseTimer = 0;
	_trackList = nullptr;
	_tracksPrefSampleRate = DIMUSE_SAMPLERATE;

	if (waveOutInit(DIMUSE_SAMPLERATE, &waveOutSettings))
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
		_tracks[l].prev = nullptr;
		_tracks[l].next = nullptr;
		_tracks[l].dispatchPtr = dispatchGetDispatchByTrackId(l);
		_tracks[l].dispatchPtr->trackPtr = &_tracks[l];
		_tracks[l].soundId = 0;
		_tracks[l].syncSize_0 = 0;
		_tracks[l].syncSize_1 = 0;
		_tracks[l].syncSize_2 = 0;
		_tracks[l].syncSize_3 = 0;
		_tracks[l].syncPtr_0 = nullptr;
		_tracks[l].syncPtr_1 = nullptr;
		_tracks[l].syncPtr_2 = nullptr;
		_tracks[l].syncPtr_3 = nullptr;
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
	Common::StackLock lock(_mutex);
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

		if (_vm->_game.id == GID_CMI) {
			ser.syncAsSint32LE(_tracks[l].syncSize_0, VER(103));
			ser.syncAsSint32LE(_tracks[l].syncSize_1, VER(103));
			ser.syncAsSint32LE(_tracks[l].syncSize_2, VER(103));
			ser.syncAsSint32LE(_tracks[l].syncSize_3, VER(103));

			if (_tracks[l].syncSize_0) {
				if (ser.isLoading())
					_tracks[l].syncPtr_0 = (byte *)malloc(_tracks[l].syncSize_0);
				ser.syncArray(_tracks[l].syncPtr_0, _tracks[l].syncSize_0, Common::Serializer::Byte, VER(103));
			}

			if (_tracks[l].syncSize_1) {
				if (ser.isLoading())
					_tracks[l].syncPtr_1 = (byte *)malloc(_tracks[l].syncSize_1);
				ser.syncArray(_tracks[l].syncPtr_1, _tracks[l].syncSize_1, Common::Serializer::Byte, VER(103));
			}

			if (_tracks[l].syncSize_2) {
				if (ser.isLoading())
					_tracks[l].syncPtr_2 = (byte *)malloc(_tracks[l].syncSize_2);
				ser.syncArray(_tracks[l].syncPtr_2, _tracks[l].syncSize_2, Common::Serializer::Byte, VER(103));
			}

			if (_tracks[l].syncSize_3) {
				if (ser.isLoading())
					_tracks[l].syncPtr_3 = (byte *)malloc(_tracks[l].syncSize_3);
				ser.syncArray(_tracks[l].syncPtr_3, _tracks[l].syncSize_3, Common::Serializer::Byte, VER(103));
			}
		}
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

	Common::StackLock lock(_mutex);

	// If we leave the number of queued streams unbounded, we fill the queue with streams faster than
	// we can play them: this leads to a very noticeable audio latency and desync with the graphics.
	if ((int)_internalMixer->_stream->numQueuedStreams() < _maxQueuedStreams) {
		if (!_isEarlyDiMUSE)
			dispatchPredictFirstStream();

		waveOutWrite(&_outputAudioBuffer, _outputFeedSize, _outputSampleRate);

		if (_outputFeedSize != 0) {
			_internalMixer->clearMixerBuffer();
			if (_isEarlyDiMUSE && _splayer && _splayer->isAudioCallbackEnabled()) {
				_splayer->processDispatches(_outputFeedSize);
			}

			if (!_tracksPauseTimer) {
				IMuseDigiTrack *track = _trackList;

				while (track) {
					IMuseDigiTrack *next = track->next;
					if (_isEarlyDiMUSE) {
						dispatchProcessDispatches(track, _outputFeedSize);
					} else {
						dispatchProcessDispatches(track, _outputFeedSize, _outputSampleRate);
					}
					track = next;
				};
			}

			_internalMixer->loop(&_outputAudioBuffer, _outputFeedSize);

			// The Dig tries to write a second time
			if (!_isEarlyDiMUSE && _vm->_game.id == GID_DIG) {
				waveOutWrite(&_outputAudioBuffer, _outputFeedSize, _outputSampleRate);
			}
		}
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
	allocatedTrack->syncSize_0 = 0;
	allocatedTrack->syncPtr_0 = nullptr;
	allocatedTrack->syncSize_1 = 0;
	allocatedTrack->syncPtr_1 = nullptr;
	allocatedTrack->syncSize_2 = 0;
	allocatedTrack->syncPtr_2 = nullptr;
	allocatedTrack->syncSize_3 = 0;
	allocatedTrack->syncPtr_3 = nullptr;

	if (dispatchAllocateSound(allocatedTrack, group)) {
		debug(5, "IMuseDigital::tracksStartSound(): ERROR: dispatch couldn't start sound %d", soundId);
		allocatedTrack->soundId = 0;
		return -1;
	}

	Common::StackLock lock(_mutex);
	addTrackToList(&_trackList, allocatedTrack);
	Common::StackLock unlock(_mutex);

	return 0;
}

int IMuseDigital::tracksStopSound(int soundId) {
	if (!_trackList)
		return -1;

	IMuseDigiTrack *nextTrack = _trackList;
	IMuseDigiTrack *curTrack;

	while (nextTrack) {
		curTrack = nextTrack;
		nextTrack = curTrack->next;
		if (curTrack->soundId == soundId) {
			tracksClear(curTrack);
		}
	}

	return 0;
}

int IMuseDigital::tracksStopAllSounds() {
	Common::StackLock lock(_mutex);
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

void IMuseDigital::tracksQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	if (!_trackList)
		debug(5, "IMuseDigital::tracksQueryStream(): WARNING: empty trackList, ignoring call...");

	IMuseDigiTrack *track = _trackList;
	do {
		if (track->soundId) {
			if (soundId == track->soundId && track->dispatchPtr->streamPtr) {
				streamerQueryStream(track->dispatchPtr->streamPtr, bufSize, criticalSize, freeSpace, paused);
				return;
			}
		}
		track = track->next;
	} while (track);

	debug(5, "IMuseDigital::tracksQueryStream(): WARNING: couldn't find sound %d in trackList, ignoring call...", soundId);
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
	if (_vm->_game.id == GID_CMI) {
		if (trackPtr->syncPtr_0) {
			trackPtr->syncSize_0 = 0;
			free(trackPtr->syncPtr_0);
			trackPtr->syncPtr_0 = nullptr;
		}

		if (trackPtr->syncPtr_1) {
			trackPtr->syncSize_1 = 0;
			free(trackPtr->syncPtr_1);
			trackPtr->syncPtr_1 = nullptr;
		}

		if (trackPtr->syncPtr_2) {
			trackPtr->syncSize_2 = 0;
			free(trackPtr->syncPtr_2);
			trackPtr->syncPtr_2 = nullptr;
		}

		if (trackPtr->syncPtr_3) {
			trackPtr->syncSize_3 = 0;
			free(trackPtr->syncPtr_3);
			trackPtr->syncPtr_3 = nullptr;
		}
	}

	removeTrackFromList(&_trackList, trackPtr);
	dispatchRelease(trackPtr);
	_fadesHandler->clearFadeStatus(trackPtr->soundId, -1);
	_triggersHandler->clearTrigger(trackPtr->soundId, _emptyMarker, -1);

	// Unlock the sound, if it's loaded as a resource
	if (trackPtr->soundId < 1000 && trackPtr->soundId) {
		_vm->_res->unlock(rtSound, trackPtr->soundId);
	}

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
				if (_vm->_game.id == GID_DIG || _vm->_game.id == GID_FT) {
					if (value < -12 || value > 12)
						return -5;

					if (value == 0) {
						track->transpose = 0;
					} else {
						track->transpose = clampTuning(track->detune + value, -12, 12);
					}

					track->pitchShift = track->detune + (track->transpose * 256);
				} else if (_vm->_game.id == GID_CMI) {
					if (value < 0 || value > 4095)
						return -5;

					track->pitchShift = value;
				}

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
		if (track)
			l++;
		if (track->soundId == soundId) {
			switch (opcode) {
			case DIMUSE_P_BOGUS_ID:
				return -1;
			case DIMUSE_P_SND_TRACK_NUM:
				return l;
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

	return 0;
}

int IMuseDigital::tracksLipSync(int soundId, int syncId, int msPos, int32 &width, int32 &height) {
	int32 h, w;

	byte *syncPtr = nullptr;
	int32 syncSize = 0;

	IMuseDigiTrack *curTrack;
	uint16 msPosDiv;
	uint16 *tmpPtr;
	int32 loopIndex;
	int16 val;

	h = 0;
	w = 0;
	curTrack = _trackList;

	if (msPos >= 0) {
		msPosDiv = msPos >> 4;
		if (((msPos >> 4) & 0xFFFF0000) != 0) {
			return -5;
		} else {
			if (_trackList) {
				do {
					if (curTrack->soundId == soundId)
						break;
					curTrack = curTrack->next;
				} while (curTrack);
			}

			if (curTrack) {
				if (syncId >= 0 && syncId < 4) {
					if (syncId == 0) {
						syncPtr = curTrack->syncPtr_0;
						syncSize = curTrack->syncSize_0;
					} else if (syncId == 1) {
						syncPtr = curTrack->syncPtr_1;
						syncSize = curTrack->syncSize_1;
					} else if (syncId == 2) {
						syncPtr = curTrack->syncPtr_2;
						syncSize = curTrack->syncSize_2;
					} else if (syncId == 3) {
						syncPtr = curTrack->syncPtr_3;
						syncSize = curTrack->syncSize_3;
					}

					if (syncSize && syncPtr) {
						tmpPtr = (uint16 *)(syncPtr + 2);
						loopIndex = (syncSize >> 2) - 1;
						if (syncSize >> 2) {
							do {
								if (*tmpPtr >= msPosDiv)
									break;
								tmpPtr += 2;
							} while (loopIndex--);
						}

						if (loopIndex < 0 || *tmpPtr > msPosDiv)
							tmpPtr -= 2;

						val = *(tmpPtr - 1);
						w = (val >> 8) & 0x7F;
						h = val & 0x7F;
					}
				}
			} else {
				return -4;
			}
		}
	}

	width = w;
	height = h;

	return 0;
}

int IMuseDigital::tracksSetHook(int soundId, int hookId) {
	if (_isEarlyDiMUSE)
		return -2;

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
	if (_isEarlyDiMUSE)
		return -2;

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
		reservedTrack = &_tracks[i];
		if (!reservedTrack->soundId) {
			return reservedTrack;
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
	}

	return reservedTrack;
}

void IMuseDigital::tracksDeinit() {
	tracksStopAllSounds();
}

} // End of namespace Scumm
