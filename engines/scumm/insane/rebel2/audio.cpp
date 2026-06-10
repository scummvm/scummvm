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

#include "common/system.h"
#include "common/util.h"

#include "scumm/file.h"
#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/rebel2/rebel.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Scumm {

// ---------------------------------------------------------------------------
// Audio Handling
// ---------------------------------------------------------------------------
// RA2 doesn't use iMUSE -- audio is handled directly through the mixer.

// initAudio -- Initialize audio system for RA2.
void InsaneRebel2::initAudio(int sampleRate) {
	_audio.init(_vm, sampleRate);
}

// terminateAudio -- Stop all tracks and release audio streams.
void InsaneRebel2::terminateAudio() {
	_audio.terminate();
}

void InsaneRebel2::resetVideoAudio() {
	_audio.reset();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (splayer)
		splayer->resetAudioTracks();
}

// queueAudioData -- Queue raw PCM data for playback on a track.
// Creates the queuing stream on first use. RA2 audio is 8-bit unsigned mono.
void InsaneRebel2::queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan) {
	_audio.queueData(trackIdx, data, size, volume, pan);
}

//
// processAudioFrame -- Per-frame audio dispatch (replaces iMUSE path)
//
// Iterates SmushPlayer audio tracks, handles FADING->PLAYING transitions,
// and feeds PCM data through queueAudioData. Called from SmushPlayer when
// iMUSE is null.
//
void InsaneRebel2::processAudioFrame(int16 feedSize) {
	_audio.processFrame(_player, feedSize);
}

// ---------------------------------------------------------------------------
// Sound Effects (SAD files)
// ---------------------------------------------------------------------------
// Standalone SAUD files from SYSTM/ loaded at init for one-shot SFX.
// Original: FUN_0042a3b0 loads into DAT_00456888[0..7].

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

// loadSfx -- Load all SAD files from SYSTM/ directory (FUN_0042a3b0).
void InsaneRebel2::loadSfx() {
	for (int i = 0; i < kRA2NumSfx; i++) {
		ScummFile *file = _vm->instantiateScummFile();
		_vm->openFile(*file, kRA2SfxFiles[i]);
		if (!file->isOpen()) {
			debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: Could not open %s", kRA2SfxFiles[i]);
			delete file;
			continue;
		}

		// SAUD file structure: SAUD header (8) + STRK sub-chunk + SDAT sub-chunk
		// We scan for the SDAT tag to find the PCM data.
		uint32 fileSize = file->size();
		if (fileSize < 38) {  // Minimum: 8 (SAUD) + 22 (STRK) + 8 (SDAT header)
			debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: %s too small (%d bytes)", kRA2SfxFiles[i], fileSize);
			file->close();
			delete file;
			continue;
		}

		// Verify SAUD tag
		uint32 tag = file->readUint32BE();
		if (tag != MKTAG('S', 'A', 'U', 'D')) {
			debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: %s not a SAUD file (tag=0x%08x)", kRA2SfxFiles[i], tag);
			file->close();
			delete file;
			continue;
		}
		file->readUint32BE();  // Skip SAUD size

		// Scan for SDAT chunk (skip STRK and any other sub-chunks)
		bool foundSdat = false;
		while (file->pos() + 8 <= (int64)fileSize) {
			uint32 chunkTag = file->readUint32BE();
			uint32 chunkSize = file->readUint32BE();

			if (chunkTag == MKTAG('S', 'D', 'A', 'T')) {
				// Found PCM data
				uint32 pcmSize = MIN(chunkSize, fileSize - (uint32)file->pos());
				_sfxData[i] = (byte *)malloc(pcmSize);
				if (_sfxData[i]) {
					file->read(_sfxData[i], pcmSize);
					_sfxSize[i] = pcmSize;
					debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: Loaded %s (%d bytes PCM)", kRA2SfxFiles[i], pcmSize);
				}
				foundSdat = true;
				break;
			} else {
				// Skip this sub-chunk
				file->seek(chunkSize, SEEK_CUR);
			}
		}

		if (!foundSdat) {
			debugC(DEBUG_INSANE, "InsaneRebel2::loadSfx: No SDAT chunk in %s", kRA2SfxFiles[i]);
		}

		file->close();
		delete file;
	}
}

// freeSfx -- Free all loaded SFX data and auxiliary buffers.
void InsaneRebel2::freeSfx() {
	for (int i = 0; i < kRA2NumSfx; i++) {
		// Stop any playing SFX on this slot
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

// playSfx -- Play a one-shot sound effect (8-bit unsigned mono, 11025 Hz).
void InsaneRebel2::playSfx(int slot, int volume, int pan) {
	if (slot < 0 || slot >= kRA2NumSfx || !_sfxData[slot] || _sfxSize[slot] == 0) {
		return;
	}
	if (_player && !_player->isChanActive(CHN_OTHER))
		return;

	// Stop any previous instance of this SFX slot
	_vm->_mixer->stopHandle(_sfxHandles[slot]);

	// Make a copy of the PCM data (makeRawStream with DisposeAfterUse::YES will free it)
	byte *pcmCopy = (byte *)malloc(_sfxSize[slot]);
	if (!pcmCopy) {
		return;
	}
	memcpy(pcmCopy, _sfxData[slot], _sfxSize[slot]);

	// Create a one-shot raw audio stream: 8-bit unsigned mono at 11025 Hz
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

// loadAuxSfx -- Load sound data into auxiliary buffer (FUN_004118df).
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

// playAuxSfx -- Play from auxiliary buffer (FUN_00411931).
// Handles both raw PCM and SAUD-wrapped data.
void InsaneRebel2::playAuxSfx(int buffer, int volume, int pan) {
	if (buffer < 0 || buffer >= kRA2NumAuxSfx || !_auxSfxData[buffer] || _auxSfxSize[buffer] == 0) {
		return;
	}
	if (_player && !_player->isChanActive(CHN_OTHER))
		return;

	_vm->_mixer->stopHandle(_auxSfxHandles[buffer]);

	// The auxiliary buffer data goes through FUN_00425fc0 (format dispatch) in the original.
	// Check if data has SAUD header; if so, extract PCM from SDAT chunk.
	// Otherwise treat as raw 8-bit unsigned PCM at 11025 Hz.
	const byte *pcmStart = _auxSfxData[buffer];
	uint32 pcmSize = _auxSfxSize[buffer];

	if (pcmSize > 8 && READ_BE_UINT32(pcmStart) == MKTAG('S', 'A', 'U', 'D')) {
		// Parse SAUD container to find SDAT chunk
		uint32 pos = 8; // Skip SAUD tag + size
		while (pos + 8 <= pcmSize) {
			uint32 chunkTag = READ_BE_UINT32(pcmStart + pos);
			uint32 chunkSize = READ_BE_UINT32(pcmStart + pos + 4);
			if (chunkTag == MKTAG('S', 'D', 'A', 'T')) {
				pcmStart = pcmStart + pos + 8;
				pcmSize = MIN(chunkSize, pcmSize - pos - 8);
				break;
			}
			pos += 8 + chunkSize;
		}
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
