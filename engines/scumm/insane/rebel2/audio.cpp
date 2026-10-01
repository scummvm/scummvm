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

#include "common/memstream.h"
#include "common/system.h"
#include "common/util.h"

#include "scumm/file.h"
#include "scumm/scumm_v7.h"

#include "scumm/smush/saud.h"
#include "scumm/smush/smush_player.h"

#include "scumm/insane/rebel2/rebel.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Scumm {

void InsaneRebel2::initAudio(int sampleRate) {
	_audio.init(_vm, sampleRate);
}

void InsaneRebel2::terminateAudio() {
	_audio.terminate();
}

void InsaneRebel2::resetVideoAudio() {
	_audio.reset();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (splayer)
		splayer->resetAudioTracks();
}

void InsaneRebel2::queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan) {
	_audio.queueData(trackIdx, data, size, volume, pan);
}

void InsaneRebel2::processAudioFrame(int16 feedSize) {
	_audio.processFrame(_player, feedSize);
}

const char *const kRA2SfxFiles[InsaneRebel2::kRA2NumSfx] = {
	"SYSTM/BLAST.SAD",    // 0 - Player laser fire
	"SYSTM/CRASH.SAD",    // 1 - Corridor/wall collision
	"SYSTM/EXPLODE.SAD",  // 2 - Enemy explosion
	"SYSTM/ALERT.SAD",    // 3 - Alert/warning
	"SYSTM/LOCKON.SAD",   // 4 - Target lock-on
	"SYSTM/BONUS.SAD",    // 5 - Bonus pickup
	"SYSTM/HBLAST.SAD",   // 6 - Heavy blast (player weapon)
	"SYSTM/TBLAST.SAD"    // 7 - TIE blast
};

void InsaneRebel2::loadSfx() {
	for (int i = 0; i < kRA2NumSfx; i++) {
		ScummFile *file = _vm->instantiateScummFile();
		_vm->openFile(*file, kRA2SfxFiles[i]);
		if (!file->isOpen()) {
			debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: Could not open %s", kRA2SfxFiles[i]);
			delete file;
			continue;
		}

		SaudHeader header;
		if (!readSaudHeader(*file, header, true)) {
			debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: Invalid SAUD file %s", kRA2SfxFiles[i]);
			file->close();
			delete file;
			continue;
		}

		byte *pcm = (byte *)malloc(header.dataSize);
		if (pcm) {
			if (file->read(pcm, header.dataSize) == header.dataSize && !file->err()) {
				_sfxData[i] = pcm;
				_sfxSize[i] = header.dataSize;
				debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: Loaded %s (%u bytes PCM)", kRA2SfxFiles[i], header.dataSize);
			} else {
				debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: Could not read PCM from %s", kRA2SfxFiles[i]);
				free(pcm);
			}
		}

		file->close();
		delete file;
	}
}

void InsaneRebel2::freeSfx() {
	for (int i = 0; i < kRA2NumSfx; i++) {
		_vm->_mixer->stopHandle(_sfxHandles[i]);
		free(_sfxData[i]);
		_sfxData[i] = nullptr;
		_sfxSize[i] = 0;
	}
	for (int i = 0; i < kRA2NumAuxSfx; i++) {
		_vm->_mixer->stopHandle(_auxSfxHandles[i]);
		free(_auxSfxData[i]);
		_auxSfxData[i] = nullptr;
		_auxSfxSize[i] = 0;
	}
}

void InsaneRebel2::playSfx(int slot, int volume, int pan) {
	if (slot < 0 || slot >= kRA2NumSfx || !_sfxData[slot] || _sfxSize[slot] == 0) {
		return;
	}
	if (_player && !_player->isChanActive(CHN_OTHER))
		return;

	_vm->_mixer->stopHandle(_sfxHandles[slot]);

	byte *pcmCopy = (byte *)malloc(_sfxSize[slot]);
	if (!pcmCopy) {
		return;
	}
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

	debugC(DEBUG_INSANE, "InsaneRebel2::playSfx: slot=%d vol=%d pan=%d size=%d", slot, mixVolume, clampedPan, _sfxSize[slot]);
}

void InsaneRebel2::loadAuxSfx(int buffer, const byte *data, uint32 size) {
	if (buffer < 0 || buffer >= kRA2NumAuxSfx || !data || size == 0) {
		return;
	}
	if ((int)size > kRA2AuxBufSize) {
		debugC(DEBUG_INSANE, "InsaneRebel2::loadAuxSfx: buffer %d size %d exceeds max %d, truncating",
			buffer, size, kRA2AuxBufSize);
		size = kRA2AuxBufSize;
	}

	memcpy(_auxSfxData[buffer], data, size);
	_auxSfxSize[buffer] = size;

	debugC(DEBUG_INSANE, "InsaneRebel2::loadAuxSfx: buffer=%d size=%d", buffer, size);
}

void InsaneRebel2::playAuxSfx(int buffer, int volume, int pan) {
	if (buffer < 0 || buffer >= kRA2NumAuxSfx || !_auxSfxData[buffer] || _auxSfxSize[buffer] == 0) {
		return;
	}
	if (_player && !_player->isChanActive(CHN_OTHER))
		return;

	_vm->_mixer->stopHandle(_auxSfxHandles[buffer]);

	const byte *pcmStart = _auxSfxData[buffer];
	uint32 pcmSize = _auxSfxSize[buffer];

	if (pcmSize >= 4 && READ_BE_UINT32(pcmStart) == MKTAG('S', 'A', 'U', 'D')) {
		Common::MemoryReadStream soundStream(pcmStart, pcmSize);
		SaudHeader header;
		if (!readSaudHeader(soundStream, header, true)) {
			debugC(DEBUG_INSANE, "InsaneRebel2::playAuxSfx: Invalid SAUD in buffer %d", buffer);
			return;
		}
		pcmStart += soundStream.pos();
		pcmSize = header.dataSize;
	}

	byte *pcmCopy = (byte *)malloc(pcmSize);
	if (!pcmCopy) {
		return;
	}
	memcpy(pcmCopy, pcmStart, pcmSize);

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(
		pcmCopy, pcmSize, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);

	int mixVolume = CLIP(volume, 0, 127);
	if (_player) {
		const int baseVolume = (_player->_smushTrackVols[1] * mixVolume) >> 7;
		mixVolume = (baseVolume * _player->_smushTrackVols[0]) / 127;
	}
	const int scaledVolume = (mixVolume * Audio::Mixer::kMaxChannelVolume) / 127;
	const int clampedPan = CLIP(pan, -127, 127);

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_auxSfxHandles[buffer],
		stream, -1, scaledVolume, clampedPan);

	debugC(DEBUG_INSANE, "InsaneRebel2::playAuxSfx: buffer=%d vol=%d pan=%d pcmSize=%d", buffer, mixVolume, clampedPan, pcmSize);
}

} // End of namespace Scumm
