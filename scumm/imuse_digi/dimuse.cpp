/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

#include "stdafx.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

IMuseDigital::Track::Track()
	: soundId(-1), used(false), stream(NULL) {
}

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *imuseDigital = (IMuseDigital *)refCon;
	imuseDigital->callback();
}

IMuseDigital::IMuseDigital(ScummEngine *scumm)
	: _vm(scumm) {
	_mutex = g_system->createMutex();
	_pause = false;
	_sound = new ImuseDigiSndMgr(_vm);
	_volVoice = 0;
	_volSfx = 0;
	_volMusic = 0;
	resetState();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		_track[l] = new Track;
		_track[l]->used = false;
	}
	_vm->_timer->installTimerProc(timer_handler, 1000000 / 25, this);
}

IMuseDigital::~IMuseDigital() {
	Common::StackLock lock(_mutex, "IMuseDigital::~IMuseDigital()");
	stopAllSounds();
	_vm->_timer->removeTimerProc(timer_handler);
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		delete _track[l];
	}
	delete _sound;
	g_system->deleteMutex(_mutex);
}

void IMuseDigital::resetState() {
	_curMusicState = 0;
	_curMusicSeq = 0;
	_curMusicCue = 0;
	memset(_attributes, 0, sizeof(_attributes));
	_nextSeqToPlay = 0;
}

void IMuseDigital::saveOrLoad(Serializer *ser) {
}

void IMuseDigital::callback() {
	Common::StackLock lock(_mutex, "IMuseDigital::callback()");
	int l = 0;

	if (_pause || !_vm)
		return;

	for (l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		if (_track[l]->used) {
			if (_track[l]->stream2) {
				if (!_track[l]->handle.isActive() && _track[l]->started) {
					debug(5, "IMuseDigital::callback() A: stopped sound: %d", _track[l]->soundId);
					delete _track[l]->stream2;
					_track[l]->stream2 = NULL;
					_track[l]->used = false;
					continue;
				}
			} else if (_track[l]->stream) {
				if (_track[l]->toBeRemoved) {
					debug(5, "IMuseDigital::callback() B: stopped sound: %d", _track[l]->soundId);
					_track[l]->stream->finish();
					_track[l]->stream = NULL;
					_sound->closeSound(_track[l]->soundHandle);
					_track[l]->soundHandle = NULL;
					_track[l]->used = false;
					continue;
				}
			}

			if (_track[l]->volFadeUsed) {
				if (_track[l]->volFadeStep < 0) {
					if (_track[l]->vol > _track[l]->volFadeDest) {
						_track[l]->vol += _track[l]->volFadeStep;
						if (_track[l]->vol < _track[l]->volFadeDest) {
							_track[l]->vol = _track[l]->volFadeDest;
							_track[l]->volFadeUsed = false;
						}
						if (_track[l]->vol == 0) {
							_track[l]->toBeRemoved = true;
						}
					}
				} else if (_track[l]->volFadeStep > 0) {
					if (_track[l]->vol < _track[l]->volFadeDest) {
						_track[l]->vol += _track[l]->volFadeStep;
						if (_track[l]->vol > _track[l]->volFadeDest) {
							_track[l]->vol = _track[l]->volFadeDest;
							_track[l]->volFadeUsed = false;
						}
					}
				}
				debug(5, "Fade: sound(%d), Vol(%d)", _track[l]->soundId, _track[l]->vol / 1000);
			}

			int pan = (_track[l]->pan != 64) ? 2 * _track[l]->pan - 127 : 0;
			int vol = _track[l]->vol / 1000;

			if (_track[l]->volGroupId == 1)
				vol = (vol * _volVoice) / 128;
			if (_track[l]->volGroupId == 2)
				vol = (vol * _volSfx) / 128;
			if (_track[l]->volGroupId == 3)
				vol = (vol * _volMusic) / 128;

			if (_vm->_mixer->isReady()) {
				if (_track[l]->stream2) {
					if (!_track[l]->started) {
						_track[l]->started = true;
						_vm->_mixer->playInputStream(&_track[l]->handle, _track[l]->stream2, false, _track[l]->vol / 1000, _track[l]->pan, -1, false);
					} else {
						_vm->_mixer->setChannelVolume(_track[l]->handle, vol);
						_vm->_mixer->setChannelBalance(_track[l]->handle, pan);
					}
					continue;
				}
			}

			if (_track[l]->stream) {
				byte *data = NULL;
				int32 result = 0;

				if (_track[l]->curRegion == -1) {
					switchToNextRegion(l);
					if (_track[l]->toBeRemoved)
						continue;
				}

				int bits = _sound->getBits(_track[l]->soundHandle);
				int channels = _sound->getChannels(_track[l]->soundHandle);

				int32 freeSpace = _track[l]->stream->getFreeSpace() - 4;
				int32 bufferMin = (_track[l]->iteration * 3) / 4;
				if (bufferMin > freeSpace)
					continue;

				int32 mixer_size = freeSpace;

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

						mixer_size += _track[l]->mod;
						int mixer_size_12 = (mixer_size * 3) / 4;
						int length = (mixer_size_12 / 3) * 4;
						_track[l]->mod = mixer_size - length;

						int32 offset = (_track[l]->regionOffset * 3) / 4;
						int result2 = _sound->getDataFromRegion(_track[l]->soundHandle, _track[l]->curRegion, &ptr, offset, mixer_size_12);
						result = BundleCodecs::decode12BitsSample(ptr, &data, result2);

						free(ptr);
					} else if (bits == 16) {
						result = _sound->getDataFromRegion(_track[l]->soundHandle, _track[l]->curRegion, &data, _track[l]->regionOffset, mixer_size);
						if (channels == 1) {
							result &= ~1;
						}
						if (channels == 2) {
							result &= ~3;
						}
					} else if (bits == 8) {
						result = _sound->getDataFromRegion(_track[l]->soundHandle, _track[l]->curRegion, &data, _track[l]->regionOffset, mixer_size);
						if (channels == 2) {
							result &= ~1;
						}
					}

					if (result > mixer_size)
						result = mixer_size;

					if (_vm->_mixer->isReady()) {
						_vm->_mixer->setChannelVolume(_track[l]->handle, vol);
						_vm->_mixer->setChannelBalance(_track[l]->handle, pan);
						_track[l]->stream->append(data, result);
						_track[l]->regionOffset += result;
						_track[l]->trackOffset += result;
						free(data);
					}

					if (_sound->isEndOfRegion(_track[l]->soundHandle, _track[l]->curRegion)) {
						switchToNextRegion(l);
						if (_track[l]->toBeRemoved)
							break;
					}
					mixer_size -= result;
					assert(mixer_size >= 0);
				} while (mixer_size != 0);
			}
		}
	}
}

void IMuseDigital::switchToNextRegion(int track) {
	debug(5, "switchToNextRegion(track:%d)", track);

	if (track >= MAX_DIGITAL_TRACKS) {
		_track[track]->toBeRemoved = true;
		debug(5, "exit (fadetrack can't go next region) switchToNextRegion(track:%d)", track);
		return;
	}

	int num_regions = _sound->getNumRegions(_track[track]->soundHandle);

	if (++_track[track]->curRegion == num_regions) {
		_track[track]->toBeRemoved = true;
		debug(5, "exit (end of regions) switchToNextRegion(track:%d)", track);
		return;
	}

	int jumpId = _sound->getJumpIdByRegionAndHookId(_track[track]->soundHandle, _track[track]->curRegion, _track[track]->curHookId);
	if (jumpId == -1)
		jumpId = _sound->getJumpIdByRegionAndHookId(_track[track]->soundHandle, _track[track]->curRegion, 0);
	if (jumpId != -1) {
		int region = _sound->getRegionIdByJumpId(_track[track]->soundHandle, jumpId);
		assert(region != -1);
		int sampleHookId = _sound->getJumpHookId(_track[track]->soundHandle, jumpId);
		assert(sampleHookId != -1);
		int fadeDelay = (60 * _sound->getJumpFade(_track[track]->soundHandle, jumpId)) / 1000;
		if (sampleHookId != 0) {
			if (_track[track]->curHookId == sampleHookId) {
				if (fadeDelay != 0) {
					int fadeTrack = cloneToFadeOutTrack(track, fadeDelay, false);
					_track[fadeTrack]->dataOffset = _sound->getRegionOffset(_track[fadeTrack]->soundHandle, _track[fadeTrack]->curRegion);
					_track[fadeTrack]->regionOffset = 0;
					debug(5, "switchToNextRegion-sound(%d) select %d region, curHookId: %d", _track[fadeTrack]->soundId, _track[fadeTrack]->curRegion, _track[fadeTrack]->curHookId);
					_track[fadeTrack]->curHookId = 0;
				}
				_track[track]->curRegion = region;
				debug(5, "switchToNextRegion-sound(%d) jump to %d region, curHookId: %d", _track[track]->soundId, _track[track]->curRegion, _track[track]->curHookId);
				_track[track]->curHookId = 0;
			}
		} else {
			if (fadeDelay != 0) {
				int fadeTrack = cloneToFadeOutTrack(track, fadeDelay, false);
				_track[fadeTrack]->dataOffset = _sound->getRegionOffset(_track[fadeTrack]->soundHandle, _track[fadeTrack]->curRegion);
				_track[fadeTrack]->regionOffset = 0;
				debug(5, "switchToNextRegion-sound(%d) select %d region, curHookId: %d", _track[fadeTrack]->soundId, _track[fadeTrack]->curRegion, _track[fadeTrack]->curHookId);
			}
			_track[track]->curRegion = region;
			debug(5, "switchToNextRegion-sound(%d) jump to %d region, curHookId: %d", _track[track]->soundId, _track[track]->curRegion, _track[track]->curHookId);
		}
	}

	debug(5, "switchToNextRegion-sound(%d) select %d region, curHookId: %d", _track[track]->soundId, _track[track]->curRegion, _track[track]->curHookId);
	_track[track]->dataOffset = _sound->getRegionOffset(_track[track]->soundHandle, _track[track]->curRegion);
	_track[track]->regionOffset = 0;
}

} // End of namespace Scumm
