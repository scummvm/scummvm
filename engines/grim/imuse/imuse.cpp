/* Residual - A 3D game interpreter
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

#include "common/timer.h"

#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"

#include "engines/grim/imuse/imuse.h"

namespace Grim {

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
		memset(_track[l], 0, sizeof(Track));
		_track[l]->trackId = l;
	}
	vimaInit(imuseDestTable);
	if (g_grim->getGameFlags() & GF_DEMO) {
		_stateMusicTable = grimDemoStateMusicTable;
		_seqMusicTable = grimDemoSeqMusicTable;
	} else {
		_stateMusicTable = grimStateMusicTable;
		_seqMusicTable = grimSeqMusicTable;
	}
	g_system->getTimerManager()->installTimerProc(timerHandler, 1000000 / _callbackFps, this);
}

Imuse::~Imuse() {
	g_system->getTimerManager()->removeTimerProc(timerHandler);
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
	_curMusicState = savedState->readLESint32();
	_curMusicSeq = savedState->readLESint32();
	for (int r = 0; r < 185; r++) {
		_attributes[r] = savedState->readLESint32();
	}

	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		memset(track, 0, sizeof(Track));
		track->trackId = l;
		track->pan = savedState->readLESint32();
		track->panFadeDest = savedState->readLESint32();
		track->panFadeDelay = savedState->readLESint32();
		track->panFadeUsed = savedState->readLEBool();
		track->vol = savedState->readLESint32();
		track->volFadeDest = savedState->readLESint32();
		track->volFadeDelay = savedState->readLESint32();
		track->volFadeUsed = savedState->readLEBool();
		savedState->read(track->soundName, 32);
		track->used = savedState->readLEBool();
		track->toBeRemoved = savedState->readLEBool();
		track->priority = savedState->readLESint32();
		track->regionOffset = savedState->readLESint32();
		track->dataOffset = savedState->readLESint32();
		track->curRegion = savedState->readLESint32();
		track->curHookId = savedState->readLESint32();
		track->volGroupId = savedState->readLESint32();
		track->feedSize = savedState->readLESint32();
		track->mixerFlags = savedState->readLESint32();

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
		g_system->getMixer()->playInputStream(track->getType(), &track->handle, track->stream, -1, track->getVol(), track->getPan());
		g_system->getMixer()->pauseHandle(track->handle, true);
	}
	savedState->endSection();
	g_system->getMixer()->pauseAll(false);

	printf("Imuse::restoreState() finished.\n");
}

void Imuse::saveState(SaveGame *savedState) {
	Common::StackLock lock(_mutex);
	printf("Imuse::saveState() started.\n");

	savedState->beginSection('IMUS');
	savedState->writeLESint32(_curMusicState);
	savedState->writeLESint32(_curMusicSeq);
	for (int r = 0; r < 185; r++) {
		savedState->writeLESint32(_attributes[r]);
	}

	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		savedState->writeLESint32(track->pan);
		savedState->writeLESint32(track->panFadeDest);
		savedState->writeLESint32(track->panFadeDelay);
		savedState->writeLEBool(track->panFadeUsed);
		savedState->writeLESint32(track->vol);
		savedState->writeLESint32(track->volFadeDest);
		savedState->writeLESint32(track->volFadeDelay);
		savedState->writeLEBool(track->volFadeUsed);
		savedState->write(track->soundName, 32);
		savedState->writeLEBool(track->used);
		savedState->writeLEBool(track->toBeRemoved);
		savedState->writeLESint32(track->priority);
		savedState->writeLESint32(track->regionOffset);
		savedState->writeLESint32(track->dataOffset);
		savedState->writeLESint32(track->curRegion);
		savedState->writeLESint32(track->curHookId);
		savedState->writeLESint32(track->volGroupId);
		savedState->writeLESint32(track->feedSize);
		savedState->writeLESint32(track->mixerFlags);
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
				if (!g_system->getMixer()->isSoundHandleActive(track->handle))
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

				if (g_system->getMixer()->isReady()) {
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
			if (g_system->getMixer()->isReady()) {
				g_system->getMixer()->setChannelVolume(track->handle, track->getVol());
				g_system->getMixer()->setChannelBalance(track->handle, track->getPan());
			}
		}
	}
}

void Imuse::switchToNextRegion(Track *track) {
	assert(track);

	if (track->trackId >= MAX_IMUSE_TRACKS) {
		if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
			printf("Imuse::switchToNextRegion(): fadeTrack end: soundName:%s\n", track->soundName);
		flushTrack(track);
		return;
	}

	int numRegions = _sound->getNumRegions(track->soundDesc);

	if (++track->curRegion == numRegions) {
		if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
			printf("Imuse::switchToNextRegion(): end of tracks: soundName:%s\n", track->soundName);
		flushTrack(track);
		return;
	}

	ImuseSndMgr::SoundDesc *soundDesc = track->soundDesc;
	int jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, track->curHookId);
	if (jumpId == -1)
		jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, 0);
	if (jumpId != -1) {
		if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
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

	if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
		printf("Imuse::switchToNextRegion(): REGION %d: soundName:%s\n", (int)track->curRegion, track->soundName);
	track->dataOffset = _sound->getRegionOffset(soundDesc, track->curRegion);
	track->regionOffset = 0;
}

} // end of namespace Grim
