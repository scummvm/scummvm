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

#include "common/config-manager.h"
#include "common/system.h"

#include "scumm/file.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

const char *const kRA1SfxFiles[8] = {
	"SYS/LASRSHOT.SAD",
	"SYS/EXPLODE.SAD",
	"SYS/BOOM.SAD",
	"SYS/KLAXON.SAD",
	"SYS/LOCKON.SAD",
	"SYS/ALERT.SAD",
	"SYS/BONUS.SAD",
	"SYS/BLAST.SAD"
};

// ---------------------------------------------------------------------------
// Audio
// ---------------------------------------------------------------------------

void InsaneRebel1::initAudio(int sampleRate) {
	_audio.init(_vm, sampleRate);
}

void InsaneRebel1::terminateAudio() {
	_audio.terminate();

	for (int i = 0; i < kNumSfx; i++) {
		_vm->_mixer->stopHandle(_sfxHandles[i]);
	}
}

void InsaneRebel1::queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan) {
	_audio.queueData(trackIdx, data, size, volume, pan);
}

void InsaneRebel1::processAudioFrame(int16 feedSize) {
	_audio.processFrame(_player, feedSize);
}

void InsaneRebel1::applyAudioOptions() {
	const int musicVolume = ConfMan.getInt("music_volume");
	const int sfxVolume = ConfMan.getInt("sfx_volume");
	const int speechVolume = ConfMan.getInt("speech_volume");

	_optVolume = CLIP<int>(musicVolume / 2, 0, 127);

	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, musicVolume);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, sfxVolume);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, speechVolume);

	_vm->_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, !_optMusicEnabled);
	_vm->_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, !_optSfxEnabled);
	_vm->_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, !_optSfxEnabled);

	if (_player) {
		_player->setChanFlag(CHN_BKGMUS, _optMusicEnabled ? 1 : 0);
		_player->setChanFlag(CHN_OTHER, _optSfxEnabled ? 1 : 0);
		_player->setChanFlag(CHN_SPEECH, _optSfxEnabled ? 1 : 0);
	}
}

void InsaneRebel1::loadSfx() {
	for (int i = 0; i < kNumSfx; i++) {
		if (_sfxData[i] || _sfxSize[i] != 0)
			continue;

		ScummFile *file = _vm->instantiateScummFile();
		_vm->openFile(*file, kRA1SfxFiles[i]);
		if (!file->isOpen()) {
			debugC(DEBUG_INSANE, "InsaneRebel1::loadSfx: could not open %s", kRA1SfxFiles[i]);
			delete file;
			continue;
		}

		const uint32 fileSize = file->size();
		if (fileSize < 16) {
			debugC(DEBUG_INSANE, "InsaneRebel1::loadSfx: %s too small (%u bytes)", kRA1SfxFiles[i], fileSize);
			file->close();
			delete file;
			continue;
		}

		const uint32 tag = file->readUint32BE();
		if (tag != MKTAG('S', 'A', 'U', 'D')) {
			debugC(DEBUG_INSANE, "InsaneRebel1::loadSfx: %s not a SAUD file (tag=0x%08x)", kRA1SfxFiles[i], tag);
			file->close();
			delete file;
			continue;
		}
		file->readUint32BE();

		bool foundSdat = false;
		while (file->pos() + 8 <= (int64)fileSize) {
			const uint32 chunkTag = file->readUint32BE();
			const uint32 chunkSize = file->readUint32BE();
			if (chunkTag == MKTAG('S', 'D', 'A', 'T')) {
				const uint32 pcmSize = MIN(chunkSize, fileSize - (uint32)file->pos());
				byte *pcm = (byte *)malloc(pcmSize);
				if (pcm) {
					file->read(pcm, pcmSize);
					_sfxData[i] = pcm;
					_sfxSize[i] = pcmSize;
					debugC(DEBUG_INSANE, "InsaneRebel1::loadSfx: loaded %s (%u bytes PCM)", kRA1SfxFiles[i], pcmSize);
				}
				foundSdat = true;
				break;
			}
			file->seek(chunkSize, SEEK_CUR);
		}

		if (!foundSdat)
			debugC(DEBUG_INSANE, "InsaneRebel1::loadSfx: no SDAT chunk in %s", kRA1SfxFiles[i]);

		file->close();
		delete file;
	}
}

void InsaneRebel1::freeSfx() {
	for (int i = 0; i < kNumSfx; i++) {
		_vm->_mixer->stopHandle(_sfxHandles[i]);
		free(_sfxData[i]);
		_sfxData[i] = nullptr;
		_sfxSize[i] = 0;
	}
}

void InsaneRebel1::stopSfx(int slot) {
	if (slot < 0 || slot >= kNumSfx)
		return;

	_vm->_mixer->stopHandle(_sfxHandles[slot]);
}

void InsaneRebel1::playSfx(int slot, int volume, int pan) {
	if (slot < 0 || slot >= kNumSfx || !_sfxData[slot] || _sfxSize[slot] == 0)
		return;
	if (_player && !_player->isChanActive(CHN_OTHER))
		return;

	_vm->_mixer->stopHandle(_sfxHandles[slot]);

	byte *pcmCopy = (byte *)malloc(_sfxSize[slot]);
	if (!pcmCopy)
		return;
	memcpy(pcmCopy, _sfxData[slot], _sfxSize[slot]);

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(
		pcmCopy, _sfxSize[slot], 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	int mixVolume = CLIP(volume, 0, 127);
	if (_player) {
		const int baseVolume = (_player->_smushTrackVols[1] * mixVolume) >> 7;
		mixVolume = (baseVolume * _player->_smushTrackVols[0]) / 127;
	}
	const int scaledVolume = (mixVolume * Audio::Mixer::kMaxChannelVolume) / 127;
	const int clampedPan = CLIP(pan, -127, 127);

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[slot],
		stream, -1, scaledVolume, clampedPan);
}

} // End of namespace Scumm
