/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

IMuseDigital::Track::Track()
	: soundId(-1), used(false), stream(NULL), stream2(NULL) {
}

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *imuseDigital = (IMuseDigital *)refCon;
	imuseDigital->callback();
}

IMuseDigital::IMuseDigital(ScummEngine *scumm, int fps)
	: _vm(scumm) {
	_pause = false;
	_sound = new ImuseDigiSndMgr(_vm);
	_callbackFps = fps;
	resetState();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		_track[l] = new Track;
		_track[l]->trackId = l;
		_track[l]->used = false;
	}
	_vm->_timer->installTimerProc(timer_handler, 1000000 / _callbackFps, this);

	_audioNames = NULL;
	_numAudioNames = 0;
}

IMuseDigital::~IMuseDigital() {
	stopAllSounds();
	_vm->_timer->removeTimerProc(timer_handler);
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		delete _track[l];
	}
	delete _sound;
	free(_audioNames);
}

void IMuseDigital::resetState() {
	_curMusicState = 0;
	_curMusicSeq = 0;
	_curMusicCue = 0;
	memset(_attributes, 0, sizeof(_attributes));
	_nextSeqToPlay = 0;
}

void IMuseDigital::saveOrLoad(Serializer *ser) {
	Common::StackLock lock(_mutex, "IMuseDigital::saveOrLoad()");

	const SaveLoadEntry mainEntries[] = {
		MK_OBSOLETE(IMuseDigital, _volVoice, sleInt32, VER(31), VER(42)),
		MK_OBSOLETE(IMuseDigital, _volSfx, sleInt32, VER(31), VER(42)),
		MK_OBSOLETE(IMuseDigital, _volMusic, sleInt32, VER(31), VER(42)),
		MKLINE(IMuseDigital, _curMusicState, sleInt32, VER(31)),
		MKLINE(IMuseDigital, _curMusicSeq, sleInt32, VER(31)),
		MKLINE(IMuseDigital, _curMusicCue, sleInt32, VER(31)),
		MKLINE(IMuseDigital, _nextSeqToPlay, sleInt32, VER(31)),
		MKARRAY(IMuseDigital, _attributes[0], sleInt32, 188, VER(31)),
		MKEND()
	};

	const SaveLoadEntry trackEntries[] = {
		MKLINE(Track, pan, sleInt8, VER(31)),
		MKLINE(Track, vol, sleInt32, VER(31)),
		MKLINE(Track, volFadeDest, sleInt32, VER(31)),
		MKLINE(Track, volFadeStep, sleInt32, VER(31)),
		MKLINE(Track, volFadeDelay, sleInt32, VER(31)),
		MKLINE(Track, volFadeUsed, sleByte, VER(31)),
		MKLINE(Track, soundId, sleInt32, VER(31)),
		MKARRAY(Track, soundName[0], sleByte, 15, VER(31)),
		MKLINE(Track, used, sleByte, VER(31)),
		MKLINE(Track, toBeRemoved, sleByte, VER(31)),
		MKLINE(Track, souStream, sleByte, VER(31)),
		MKLINE(Track, started, sleByte, VER(31)),
		MKLINE(Track, priority, sleInt32, VER(31)),
		MKLINE(Track, regionOffset, sleInt32, VER(31)),
		MK_OBSOLETE(Track, trackOffset, sleInt32, VER(31), VER(31)),
		MKLINE(Track, dataOffset, sleInt32, VER(31)),
		MKLINE(Track, curRegion, sleInt32, VER(31)),
		MKLINE(Track, curHookId, sleInt32, VER(31)),
		MKLINE(Track, volGroupId, sleInt32, VER(31)),
		MKLINE(Track, soundType, sleInt32, VER(31)),
		MKLINE(Track, iteration, sleInt32, VER(31)),
		MKLINE(Track, mod, sleInt32, VER(31)),
		MKLINE(Track, mixerFlags, sleInt32, VER(31)),
		MK_OBSOLETE(Track, mixerVol, sleInt32, VER(31), VER(42)),
		MK_OBSOLETE(Track, mixerPan, sleInt32, VER(31), VER(42)),
		MKLINE(Track, compressed, sleByte, VER(45)),
		MKEND()
	};

	ser->_ref_me = this;
	ser->_save_ref = NULL;
	ser->_load_ref = NULL;

	ser->saveLoadEntries(this, mainEntries);

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (!ser->isSaving()) {
			track->compressed = false;
		}
		ser->saveLoadEntries(track, trackEntries);
		if (!ser->isSaving()) {
			if (!track->used)
				continue;
			track->readyToRemove = false;
			if ((track->toBeRemoved) || (track->souStream)) {
				track->stream2 = NULL;
				track->stream = NULL;
				track->used = false;
				continue;
			}

			track->soundHandle = _sound->openSound(track->soundId,
									track->soundName, track->soundType,
									track->volGroupId, -1);
			if (!track->soundHandle) {
				warning("IMuseDigital::saveOrLoad: Can't open sound so will not be resumed, propably on diffrent CD");
				track->stream2 = NULL;
				track->stream = NULL;
				track->used = false;
				continue;
			}

			if (track->compressed) {
				track->regionOffset = 0;
			}
			track->compressed = _sound->isCompressed(track->soundHandle);
			if (track->compressed) {
				track->regionOffset = 0;
			}
			track->dataOffset = _sound->getRegionOffset(track->soundHandle, track->curRegion);
			int bits = _sound->getBits(track->soundHandle);
			int channels = _sound->getChannels(track->soundHandle);
			int freq = _sound->getFreq(track->soundHandle);
			track->iteration = freq * channels;
			track->mixerFlags = 0;
			if (channels == 2)
				track->mixerFlags = Audio::Mixer::FLAG_STEREO | Audio::Mixer::FLAG_REVERSE_STEREO;

			if ((bits == 12) || (bits == 16)) {
				track->mixerFlags |= Audio::Mixer::FLAG_16BITS;
				track->iteration *= 2;
			} else if (bits == 8) {
				track->mixerFlags |= Audio::Mixer::FLAG_UNSIGNED;
			} else
				error("IMuseDigital::saveOrLoad(): Can't handle %d bit samples", bits);

#ifdef SCUMM_LITTLE_ENDIAN
			if (track->compressed)
				track->mixerFlags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif

			int32 streamBufferSize = track->iteration;
			track->stream2 = NULL;
			track->stream = makeAppendableAudioStream(freq, track->mixerFlags, streamBufferSize);

			const int pan = (track->pan != 64) ? 2 * track->pan - 127 : 0;
			const int vol = track->vol / 1000;
			Audio::Mixer::SoundType type = Audio::Mixer::kPlainSoundType;

			if (track->volGroupId == 1)
				type = Audio::Mixer::kSpeechSoundType;
			if (track->volGroupId == 2)
				type = Audio::Mixer::kSFXSoundType;
			if (track->volGroupId == 3)
				type = Audio::Mixer::kMusicSoundType;

			_vm->_mixer->playInputStream(type, &track->handle, track->stream, -1, vol, pan, false);
		}
	}
}

void IMuseDigital::callback() {
	Common::StackLock lock(_mutex, "IMuseDigital::callback()");

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->readyToRemove) {
			if (track->toBeRemoved) {
				track->readyToRemove = true;
				continue;
			}

			if (_pause || !_vm)
				return;

			if (track->volFadeUsed) {
				if (track->volFadeStep < 0) {
					if (track->vol > track->volFadeDest) {
						track->vol += track->volFadeStep;
						if (track->vol < track->volFadeDest) {
							track->vol = track->volFadeDest;
							track->volFadeUsed = false;
						}
						if (track->vol == 0) {
							track->toBeRemoved = true;
						}
					}
				} else if (track->volFadeStep > 0) {
					if (track->vol < track->volFadeDest) {
						track->vol += track->volFadeStep;
						if (track->vol > track->volFadeDest) {
							track->vol = track->volFadeDest;
							track->volFadeUsed = false;
						}
					}
				}
				debug(5, "Fade: sound(%d), Vol(%d)", track->soundId, track->vol / 1000);
			}

			const int pan = (track->pan != 64) ? 2 * track->pan - 127 : 0;
			const int vol = track->vol / 1000;
			Audio::Mixer::SoundType type = Audio::Mixer::kPlainSoundType;

			if (track->volGroupId == 1)
				type = Audio::Mixer::kSpeechSoundType;
			if (track->volGroupId == 2)
				type = Audio::Mixer::kSFXSoundType;
			if (track->volGroupId == 3)
				type = Audio::Mixer::kMusicSoundType;

			if (track->stream) {
				byte *data = NULL;
				int32 result = 0;

				if (track->curRegion == -1) {
					switchToNextRegion(track);
					if (track->toBeRemoved)
						continue;
				}

				int bits = _sound->getBits(track->soundHandle);
				int channels = _sound->getChannels(track->soundHandle);

				int32 mixer_size = track->iteration / _callbackFps;

				if (track->stream->endOfData()) {
					mixer_size *= 2;
				}

				if ((bits == 12) || (bits == 16)) {
					if (channels == 1)
						mixer_size &= ~1;
					if (channels == 2)
						mixer_size &= ~3;
				} else {
					if (channels == 2)
						mixer_size &= ~1;
				}

				if (mixer_size == 0)
					continue;

				do {
					if (bits == 12) {
						byte *ptr = NULL;

						mixer_size += track->mod;
						int mixer_size_12 = (mixer_size * 3) / 4;
						int length = (mixer_size_12 / 3) * 4;
						track->mod = mixer_size - length;

						int32 offset = (track->regionOffset * 3) / 4;
						int result2 = _sound->getDataFromRegion(track->soundHandle, track->curRegion, &ptr, offset, mixer_size_12);
						result = BundleCodecs::decode12BitsSample(ptr, &data, result2);

						free(ptr);
					} else if (bits == 16) {
						result = _sound->getDataFromRegion(track->soundHandle, track->curRegion, &data, track->regionOffset, mixer_size);
						if (channels == 1) {
							result &= ~1;
						}
						if (channels == 2) {
							result &= ~3;
						}
					} else if (bits == 8) {
						result = _sound->getDataFromRegion(track->soundHandle, track->curRegion, &data, track->regionOffset, mixer_size);
						if (channels == 2) {
							result &= ~1;
						}
					}

					if (result > mixer_size)
						result = mixer_size;

					if (_vm->_mixer->isReady()) {
						_vm->_mixer->setChannelVolume(track->handle, vol);
						_vm->_mixer->setChannelBalance(track->handle, pan);
						track->stream->append(data, result);
						track->regionOffset += result;
					}
					free(data);

					if (_sound->isEndOfRegion(track->soundHandle, track->curRegion)) {
						switchToNextRegion(track);
						if (track->toBeRemoved)
							break;
					}
					mixer_size -= result;
					assert(mixer_size >= 0);
				} while (mixer_size != 0);
			} else if (track->stream2) {
				if (_vm->_mixer->isReady()) {
					if (!track->started) {
						track->started = true;
						_vm->_mixer->playInputStream(type, &track->handle, track->stream2, -1, vol, pan, false);
					} else {
						_vm->_mixer->setChannelVolume(track->handle, vol);
						_vm->_mixer->setChannelBalance(track->handle, pan);
					}
				}
			}
		}
	}
}

void IMuseDigital::switchToNextRegion(Track *track) {
	assert(track);
	debug(5, "switchToNextRegion(track:%d)", track->trackId);

	if (track->trackId >= MAX_DIGITAL_TRACKS) {
		track->toBeRemoved = true;
		debug(5, "exit (fadetrack can't go next region) switchToNextRegion(trackId:%d)", track->trackId);
		return;
	}

	int num_regions = _sound->getNumRegions(track->soundHandle);

	if (++track->curRegion == num_regions) {
		track->toBeRemoved = true;
		debug(5, "exit (end of regions) switchToNextRegion(track:%d)", track->trackId);
		return;
	}

	ImuseDigiSndMgr::soundStruct *soundHandle = track->soundHandle;
	int jumpId = _sound->getJumpIdByRegionAndHookId(soundHandle, track->curRegion, track->curHookId);
	if (jumpId == -1)
		jumpId = _sound->getJumpIdByRegionAndHookId(soundHandle, track->curRegion, 0);
	if (jumpId != -1) {
		int region = _sound->getRegionIdByJumpId(soundHandle, jumpId);
		assert(region != -1);
		int sampleHookId = _sound->getJumpHookId(soundHandle, jumpId);
		assert(sampleHookId != -1);
		int fadeDelay = (60 * _sound->getJumpFade(soundHandle, jumpId)) / 1000;
		if (sampleHookId != 0) {
			if (track->curHookId == sampleHookId) {
				if (fadeDelay != 0) {
					Track *fadeTrack = cloneToFadeOutTrack(track, fadeDelay);
					fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundHandle, fadeTrack->curRegion);
					fadeTrack->regionOffset = 0;
					debug(5, "switchToNextRegion-sound(%d) select region %d, curHookId: %d", fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
					fadeTrack->curHookId = 0;
				}
				track->curRegion = region;
				debug(5, "switchToNextRegion-sound(%d) jump to region %d, curHookId: %d", track->soundId, track->curRegion, track->curHookId);
				track->curHookId = 0;
			}
		} else {
			if (fadeDelay != 0) {
				Track *fadeTrack = cloneToFadeOutTrack(track, fadeDelay);
				fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundHandle, fadeTrack->curRegion);
				fadeTrack->regionOffset = 0;
				debug(5, "switchToNextRegion-sound(%d) select region %d, curHookId: %d", fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
			}
			track->curRegion = region;
			debug(5, "switchToNextRegion-sound(%d) jump to region %d, curHookId: %d", track->soundId, track->curRegion, track->curHookId);
		}
	}

	debug(5, "switchToNextRegion-sound(%d) select region %d, curHookId: %d", track->soundId, track->curRegion, track->curHookId);
	track->dataOffset = _sound->getRegionOffset(soundHandle, track->curRegion);
	track->regionOffset = 0;
}

} // End of namespace Scumm
