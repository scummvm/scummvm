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
 * $URL$
 * $Id$
 */


#include "common/system.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/saveload.h"
#include "scumm/intern.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

IMuseDigital::Track::Track()
	: soundId(-1), used(false), stream(NULL), streamSou(NULL) {
}

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *imuseDigital = (IMuseDigital *)refCon;
	imuseDigital->callback();
}

IMuseDigital::IMuseDigital(ScummEngine_v7 *scumm, Audio::Mixer *mixer, int fps)
	: _vm(scumm), _mixer(mixer) {
	assert(_vm);
	assert(mixer);

	_pause = false;
	_sound = new ImuseDigiSndMgr(_vm);
	assert(_sound);
	_callbackFps = fps;
	resetState();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		_track[l] = new Track;
		assert(_track[l]);
		_track[l]->trackId = l;
		_track[l]->used = false;
	}
	_vm->_timer->installTimerProc(timer_handler, 1000000 / _callbackFps, this);

	_audioNames = NULL;
	_numAudioNames = 0;
}

IMuseDigital::~IMuseDigital() {
	_vm->_timer->removeTimerProc(timer_handler);
	stopAllSounds();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		delete _track[l];
	}
	delete _sound;
	free(_audioNames);
}

int32 IMuseDigital::makeMixerFlags(int32 flags) {
	int32 mixerFlags = 0;
	if (flags & kFlagUnsigned)
		mixerFlags |= Audio::Mixer::FLAG_UNSIGNED;
	if (flags & kFlag16Bits)
		mixerFlags |= Audio::Mixer::FLAG_16BITS;
	if (flags & kFlagLittleEndian)
		mixerFlags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
	if (flags & kFlagStereo)
		mixerFlags |= Audio::Mixer::FLAG_STEREO;
	if (flags & kFlagReverseStereo)
		mixerFlags |= Audio::Mixer::FLAG_REVERSE_STEREO;
	return mixerFlags;
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
		MKLINE(Track, souStreamUsed, sleByte, VER(31)),
		MKLINE(Track, mixerStreamRunning, sleByte, VER(31)),
		MKLINE(Track, soundPriority, sleInt32, VER(31)),
		MKLINE(Track, regionOffset, sleInt32, VER(31)),
		MK_OBSOLETE(Track, trackOffset, sleInt32, VER(31), VER(31)),
		MKLINE(Track, dataOffset, sleInt32, VER(31)),
		MKLINE(Track, curRegion, sleInt32, VER(31)),
		MKLINE(Track, curHookId, sleInt32, VER(31)),
		MKLINE(Track, volGroupId, sleInt32, VER(31)),
		MKLINE(Track, soundType, sleInt32, VER(31)),
		MKLINE(Track, feedSize, sleInt32, VER(31)),
		MKLINE(Track, dataMod12Bit, sleInt32, VER(31)),
		MKLINE(Track, mixerFlags, sleInt32, VER(31)),
		MK_OBSOLETE(Track, mixerVol, sleInt32, VER(31), VER(42)),
		MK_OBSOLETE(Track, mixerPan, sleInt32, VER(31), VER(42)),
		MKLINE(Track, sndDataExtComp, sleByte, VER(45)),
		MKEND()
	};

	ser->saveLoadEntries(this, mainEntries);

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (!ser->isSaving()) {
			track->sndDataExtComp = false;
		}
		ser->saveLoadEntries(track, trackEntries);
		if (!ser->isSaving()) {
			if (!track->used)
				continue;
			track->readyToRemove = false;
			if ((track->toBeRemoved) || (track->souStreamUsed) || (track->curRegion == -1)) {
				track->streamSou= NULL;
				track->stream = NULL;
				track->used = false;
				continue;
			}

			track->soundDesc = _sound->openSound(track->soundId,
									track->soundName, track->soundType,
									track->volGroupId, -1);
			if (!track->soundDesc) {
				warning("IMuseDigital::saveOrLoad: Can't open sound so will not be resumed, propably on diffrent CD");
				track->streamSou = NULL;
				track->stream = NULL;
				track->used = false;
				continue;
			}

			if (track->sndDataExtComp) {
				track->regionOffset = 0;
			}
			track->sndDataExtComp = _sound->isSndDataExtComp(track->soundDesc);
			if (track->sndDataExtComp) {
				track->regionOffset = 0;
			}
			track->dataOffset = _sound->getRegionOffset(track->soundDesc, track->curRegion);
			int bits = _sound->getBits(track->soundDesc);
			int channels = _sound->getChannels(track->soundDesc);
			int freq = _sound->getFreq(track->soundDesc);
			track->feedSize = freq * channels;
			track->mixerFlags = 0;
			if (channels == 2)
				track->mixerFlags = kFlagStereo | kFlagReverseStereo;

			if ((bits == 12) || (bits == 16)) {
				track->mixerFlags |= kFlag16Bits;
				track->feedSize *= 2;
			} else if (bits == 8) {
				track->mixerFlags |= kFlagUnsigned;
			} else
				error("IMuseDigital::saveOrLoad(): Can't handle %d bit samples", bits);

#ifdef SCUMM_LITTLE_ENDIAN
			if (track->sndDataExtComp)
				track->mixerFlags |= kFlagLittleEndian;
#endif

			track->streamSou = NULL;
			track->stream = Audio::makeAppendableAudioStream(freq, makeMixerFlags(track->mixerFlags));

			const int pan = (track->pan != 64) ? 2 * track->pan - 127 : 0;
			const int vol = track->vol / 1000;
			Audio::Mixer::SoundType type = Audio::Mixer::kPlainSoundType;

			if (track->volGroupId == 1)
				type = Audio::Mixer::kSpeechSoundType;
			if (track->volGroupId == 2)
				type = Audio::Mixer::kSFXSoundType;
			if (track->volGroupId == 3)
				type = Audio::Mixer::kMusicSoundType;

			_mixer->playInputStream(type, &track->mixChanHandle, track->stream, -1, vol, pan, false);
			_mixer->pauseHandle(track->mixChanHandle, true);
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

			if (_pause)
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
				byte *tmpSndBufferPtr = NULL;
				int32 curFeedSize = 0;

				if (track->curRegion == -1) {
					switchToNextRegion(track);
					if (track->toBeRemoved)
						continue;
				}

				int bits = _sound->getBits(track->soundDesc);
				int channels = _sound->getChannels(track->soundDesc);

				int32 feedSize = track->feedSize / _callbackFps;

				if (track->stream->endOfData()) {
					feedSize *= 2;
				}

				if ((bits == 12) || (bits == 16)) {
					if (channels == 1)
						feedSize &= ~1;
					if (channels == 2)
						feedSize &= ~3;
				} else {
					if (channels == 2)
						feedSize &= ~1;
				}

				if (feedSize == 0)
					continue;

				do {
					if (bits == 12) {
						byte *tmpPtr = NULL;

						feedSize += track->dataMod12Bit;
						int tmpFeedSize12Bits = (feedSize * 3) / 4;
						int tmpLength12Bits = (tmpFeedSize12Bits / 3) * 4;
						track->dataMod12Bit = feedSize - tmpLength12Bits;

						int32 tmpOffset = (track->regionOffset * 3) / 4;
						int tmpFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpPtr, tmpOffset, tmpFeedSize12Bits);
						curFeedSize = BundleCodecs::decode12BitsSample(tmpPtr, &tmpSndBufferPtr, tmpFeedSize);

						free(tmpPtr);
					} else if (bits == 16) {
						curFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpSndBufferPtr, track->regionOffset, feedSize);
						if (channels == 1) {
							curFeedSize &= ~1;
						}
						if (channels == 2) {
							curFeedSize &= ~3;
						}
					} else if (bits == 8) {
						curFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpSndBufferPtr, track->regionOffset, feedSize);
						if (channels == 2) {
							curFeedSize &= ~1;
						}
					}

					if (curFeedSize > feedSize)
						curFeedSize = feedSize;

					if (_mixer->isReady()) {
						_mixer->setChannelVolume(track->mixChanHandle, vol);
						_mixer->setChannelBalance(track->mixChanHandle, pan);
						track->stream->queueBuffer(tmpSndBufferPtr, curFeedSize);
						track->regionOffset += curFeedSize;
					} else
						delete[] tmpSndBufferPtr;

					if (_sound->isEndOfRegion(track->soundDesc, track->curRegion)) {
						switchToNextRegion(track);
						if (track->toBeRemoved)
							break;
					}
					feedSize -= curFeedSize;
					assert(feedSize >= 0);
				} while (feedSize != 0);
			} else if (track->streamSou) {
				if (_mixer->isReady()) {
					if (!track->mixerStreamRunning) {
						track->mixerStreamRunning = true;
						_mixer->playInputStream(type, &track->mixChanHandle, track->streamSou, -1, vol, pan, false);
					} else {
						_mixer->setChannelVolume(track->mixChanHandle, vol);
						_mixer->setChannelBalance(track->mixChanHandle, pan);
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

	int num_regions = _sound->getNumRegions(track->soundDesc);

	if (++track->curRegion == num_regions) {
		track->toBeRemoved = true;
		debug(5, "exit (end of regions) switchToNextRegion(track:%d)", track->trackId);
		return;
	}

	ImuseDigiSndMgr::SoundDesc *soundDesc = track->soundDesc;
	int jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, track->curHookId);
	if (jumpId == -1)
		jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, 0);
	if (jumpId != -1) {
		int region = _sound->getRegionIdByJumpId(soundDesc, jumpId);
		assert(region != -1);
		int sampleHookId = _sound->getJumpHookId(soundDesc, jumpId);
		assert(sampleHookId != -1);
		int fadeDelay = (60 * _sound->getJumpFade(soundDesc, jumpId)) / 1000;
		if (sampleHookId != 0) {
			if (track->curHookId == sampleHookId) {
				if (fadeDelay != 0) {
					Track *fadeTrack = cloneToFadeOutTrack(track, fadeDelay);
					if (fadeTrack) {
						fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundDesc, fadeTrack->curRegion);
						fadeTrack->regionOffset = 0;
						debug(5, "switchToNextRegion-sound(%d) select region %d, curHookId: %d", fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
						fadeTrack->curHookId = 0;
					}
				}
				track->curRegion = region;
				debug(5, "switchToNextRegion-sound(%d) jump to region %d, curHookId: %d", track->soundId, track->curRegion, track->curHookId);
				track->curHookId = 0;
			}
		} else {
			if (fadeDelay != 0) {
				Track *fadeTrack = cloneToFadeOutTrack(track, fadeDelay);
				if (fadeTrack) {
					fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundDesc, fadeTrack->curRegion);
					fadeTrack->regionOffset = 0;
					debug(5, "switchToNextRegion-sound(%d) select region %d, curHookId: %d", fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
				}
			}
			track->curRegion = region;
			debug(5, "switchToNextRegion-sound(%d) jump to region %d, curHookId: %d", track->soundId, track->curRegion, track->curHookId);
		}
	}

	debug(5, "switchToNextRegion-sound(%d) select region %d, curHookId: %d", track->soundId, track->curRegion, track->curHookId);
	track->dataOffset = _sound->getRegionOffset(soundDesc, track->curRegion);
	track->regionOffset = 0;
}

} // End of namespace Scumm
