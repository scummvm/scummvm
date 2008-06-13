/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/sys.h"
#include "common/debug.h"
#include "common/timer.h"
#include "common/mutex.h"

#include "engine/engine.h"
#include "engine/savegame.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include "engine/imuse/imuse.h"
#include "engine/imuse/imuse_sndmgr.h"

Imuse *g_imuse = NULL;

extern uint16 imuseDestTable[];
extern ImuseTable grimStateMusicTable[];
extern ImuseTable grimSeqMusicTable[];
extern ImuseTable grimDemoStateMusicTable[];
extern ImuseTable grimDemoSeqMusicTable[];

void Imuse::timerHandler(void *refCon) {
	Imuse *imuse = (Imuse *)refCon;
	imuse->callback();
}

Imuse::Imuse(int fps) {
	_pause = false;
	_sound = new ImuseSndMgr();
	assert(_sound);
	_callbackFps = fps;
	resetState();
	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		_track[l] = new Track;
		assert(_track[l]);
		_track[l]->trackId = l;
		_track[l]->used = false;
		strcpy(_track[l]->soundName, "");
	}
	vimaInit(imuseDestTable);
	_stateMusicTable = grimStateMusicTable;
	_seqMusicTable = grimSeqMusicTable;
	g_timer->installTimerProc(timerHandler, 1000000 / _callbackFps, this);
}

Imuse::~Imuse() {
	g_timer->removeTimerProc(timerHandler);
	stopAllSounds();
	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		delete _track[l];
	}
	delete _sound;
}

void Imuse::resetState() {
	_curMusicState = 0;
	_curMusicSeq = 0;
	memset(_attributes, 0, sizeof(_attributes));
}

void Imuse::restoreState(SaveGame *savedState) {
	Common::StackLock lock(_mutex);
	printf("Imuse::restoreState() started.\n");

	savedState->beginSection('IMUS');
	savedState->read(&_curMusicState, sizeof(int32));
	savedState->read(&_curMusicSeq, sizeof(int32));
	savedState->read(_attributes, sizeof(int32) * 185);

	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		memset(track, 0, sizeof(Track));
		track->trackId = l;
		savedState->read(&track->pan, sizeof(int32));
		savedState->read(&track->panFadeDest, sizeof(int32));
		savedState->read(&track->panFadeDelay, sizeof(int32));
		savedState->read(&track->panFadeUsed, sizeof(bool));
		savedState->read(&track->vol, sizeof(int32));
		savedState->read(&track->volFadeDest, sizeof(int32));
		savedState->read(&track->volFadeDelay, sizeof(int32));
		savedState->read(&track->volFadeUsed, sizeof(bool));
		savedState->read(track->soundName, 32);
		savedState->read(&track->used, sizeof(bool));
		savedState->read(&track->toBeRemoved, sizeof(bool));
		savedState->read(&track->priority, sizeof(int32));
		savedState->read(&track->regionOffset, sizeof(int32));
		savedState->read(&track->dataOffset, sizeof(int32));
		savedState->read(&track->curRegion, sizeof(int32));
		savedState->read(&track->curHookId, sizeof(int32));
		savedState->read(&track->volGroupId, sizeof(int32));
		savedState->read(&track->feedSize, sizeof(int32));
		savedState->read(&track->mixerFlags, sizeof(int32));

		if (!track->used)
			continue;

		if (track->toBeRemoved || track->curRegion == -1) {
			track->used = false;
			continue;
		}

		track->soundDesc = _sound->openSound(track->soundName, track->volGroupId);
		if (!track->soundDesc) {
			warning("Imuse::restoreState: Can't open sound so will not be resumed");
			track->used = false;
			continue;
		}

		int channels = _sound->getChannels(track->soundDesc);
		int freq = _sound->getFreq(track->soundDesc);
		track->mixerFlags = kFlag16Bits;
		if (channels == 2)
			track->mixerFlags |= kFlagStereo | kFlagReverseStereo;

		track->stream = Audio::makeAppendableAudioStream(freq,  makeMixerFlags(track->mixerFlags));
		g_mixer->playInputStream(track->getType(), &track->handle, track->stream, -1, track->getVol(), track->getPan());
		g_mixer->pauseHandle(track->handle, true);
	}
	savedState->endSection();
	g_mixer->pauseAll(false);

	printf("Imuse::restoreState() finished.\n");
}

void Imuse::saveState(SaveGame *savedState) {
	Common::StackLock lock(_mutex);
	printf("Imuse::saveState() started.\n");

	savedState->beginSection('IMUS');
	savedState->write(&_curMusicState, sizeof(int32));
	savedState->write(&_curMusicSeq, sizeof(int32));
	savedState->write(_attributes, sizeof(int32) * 185);

	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		savedState->write(&track->pan, sizeof(int32));
		savedState->write(&track->panFadeDest, sizeof(int32));
		savedState->write(&track->panFadeDelay, sizeof(int32));
		savedState->write(&track->panFadeUsed, sizeof(bool));
		savedState->write(&track->vol, sizeof(int32));
		savedState->write(&track->volFadeDest, sizeof(int32));
		savedState->write(&track->volFadeDelay, sizeof(int32));
		savedState->write(&track->volFadeUsed, sizeof(bool));
		savedState->write(track->soundName, 32);
		savedState->write(&track->used, sizeof(bool));
		savedState->write(&track->toBeRemoved, sizeof(bool));
		savedState->write(&track->priority, sizeof(int32));
		savedState->write(&track->regionOffset, sizeof(int32));
		savedState->write(&track->dataOffset, sizeof(int32));
		savedState->write(&track->curRegion, sizeof(int32));
		savedState->write(&track->curHookId, sizeof(int32));
		savedState->write(&track->volGroupId, sizeof(int32));
		savedState->write(&track->feedSize, sizeof(int32));
		savedState->write(&track->mixerFlags, sizeof(int32));
	}
	savedState->endSection();
	printf("Imuse::saveState() finished.\n");
}

int32 Imuse::makeMixerFlags(int32 flags) {
	int32 mixerFlags = 0;
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

void Imuse::callback() {
	Common::StackLock lock(_mutex);

	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used) {
			// Ignore tracks which are about to finish. Also, if it did finish in the meantime,
			// mark it as unused.
			if (!track->stream) {
				if (!g_mixer->isSoundHandleActive(track->handle))
					memset(track, 0, sizeof(Track));
				continue;
			}

			if (_pause)
				return;

			if (track->volFadeUsed) {
				if (track->volFadeStep < 0) {
					if (track->vol > track->volFadeDest) {
						track->vol += track->volFadeStep;
						//warning("fade: %d", track->vol);
						if (track->vol < track->volFadeDest) {
							track->vol = track->volFadeDest;
							track->volFadeUsed = false;
						}
						if (track->vol == 0) {
							// Fade out complete -> remove this track
							flushTrack(track);
							continue;
						}
					}
				} else if (track->volFadeStep > 0) {
					if (track->vol < track->volFadeDest) {
						track->vol += track->volFadeStep;
						//warning("fade: %d", track->vol);
						if (track->vol > track->volFadeDest) {
							track->vol = track->volFadeDest;
							track->volFadeUsed = false;
						}
					}
				}
			}

			if (track->panFadeUsed) {
				if (track->panFadeStep < 0) {
					if (track->pan > track->panFadeDest) {
						track->pan += track->panFadeStep;
						if (track->pan < track->panFadeDest) {
							track->pan = track->panFadeDest;
							track->panFadeUsed = false;
						}
					}
				} else if (track->panFadeStep > 0) {
					if (track->pan < track->panFadeDest) {
						track->pan += track->panFadeStep;
						if (track->pan > track->panFadeDest) {
							track->pan = track->panFadeDest;
							track->panFadeUsed = false;
						}
					}
				}
			}

			assert(track->stream);
			byte *data = NULL;
			int32 result = 0;

			if (track->curRegion == -1) {
				switchToNextRegion(track);
				if (!track->stream)	// Seems we reached the end of the stream
					continue;
			}

			int channels = _sound->getChannels(track->soundDesc);
			int32 mixer_size = track->feedSize / _callbackFps;

			if (track->stream->endOfData()) {
				mixer_size *= 2;
			}

			if (channels == 1)
				mixer_size &= ~1;
			if (channels == 2)
				mixer_size &= ~3;

			if (mixer_size == 0)
				continue;

			do {
				result = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &data, track->regionOffset, mixer_size);
				if (channels == 1) {
					result &= ~1;
				}
				if (channels == 2) {
					result &= ~3;
				}

				if (result > mixer_size)
					result = mixer_size;

				if (g_mixer->isReady()) {
					track->stream->queueBuffer(data, result);
					track->regionOffset += result;
				} else
					delete[] data;

				if (_sound->isEndOfRegion(track->soundDesc, track->curRegion)) {
					switchToNextRegion(track);
					if (!track->stream)
						break;
				}
				mixer_size -= result;
				assert(mixer_size >= 0);
			} while (mixer_size);
			if (g_mixer->isReady()) {
				g_mixer->setChannelVolume(track->handle, track->getVol());
				g_mixer->setChannelBalance(track->handle, track->getPan());
			}
		}
	}
}

void Imuse::switchToNextRegion(Track *track) {
	assert(track);

	if (track->trackId >= MAX_IMUSE_TRACKS) {
		if (debugLevel == DEBUG_IMUSE || debugLevel == DEBUG_ALL)
			printf("Imuse::switchToNextRegion(): fadeTrack end: soundName:%s\n", track->soundName);
		flushTrack(track);
		return;
	}

	int numRegions = _sound->getNumRegions(track->soundDesc);

	if (++track->curRegion == numRegions) {
		if (debugLevel == DEBUG_IMUSE || debugLevel == DEBUG_ALL)
			printf("Imuse::switchToNextRegion(): end of tracks: soundName:%s\n", track->soundName);
		flushTrack(track);
		return;
	}

	ImuseSndMgr::SoundDesc *soundDesc = track->soundDesc;
	int jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, track->curHookId);
	if (jumpId == -1)
		jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, 0);
	if (jumpId != -1) {
		if (debugLevel == DEBUG_IMUSE || debugLevel == DEBUG_ALL)
			printf("Imuse::switchToNextRegion(): JUMP: soundName:%s\n", track->soundName);
		int region = _sound->getRegionIdByJumpId(soundDesc, jumpId);
		assert(region != -1);
		int sampleHookId = _sound->getJumpHookId(soundDesc, jumpId);
		assert(sampleHookId != -1);
		int fadeDelay = (60 * _sound->getJumpFade(soundDesc, jumpId)) / 1000;
		if (fadeDelay) {
			Track *fadeTrack = cloneToFadeOutTrack(track, fadeDelay);
			if (fadeTrack) {
				fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundDesc, fadeTrack->curRegion);
				fadeTrack->regionOffset = 0;
				fadeTrack->curHookId = 0;
			}
		}
		track->curRegion = region;
		if (track->curHookId == sampleHookId)
			track->curHookId = 0;
		else
			if (track->curHookId == 0x80)
				track->curHookId = 0;
	}

	if (debugLevel == DEBUG_IMUSE || debugLevel == DEBUG_ALL)
		printf("Imuse::switchToNextRegion(): REGION %d: soundName:%s\n", track->curRegion, track->soundName);
	track->dataOffset = _sound->getRegionOffset(soundDesc, track->curRegion);
	track->regionOffset = 0;
}
