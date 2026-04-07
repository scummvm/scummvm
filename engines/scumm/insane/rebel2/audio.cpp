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
	_audioSampleRate = sampleRate;
	for (int i = 0; i < kRA2MaxAudioTracks; i++) {
		_audioStreams[i] = nullptr;
		_audioTrackActive[i] = false;
	}
}

// terminateAudio -- Stop all tracks and release audio streams.
void InsaneRebel2::terminateAudio() {
	for (int i = 0; i < kRA2MaxAudioTracks; i++) {
		if (_audioTrackActive[i]) {
			_vm->_mixer->stopHandle(_audioHandles[i]);
			_audioTrackActive[i] = false;
		}
		if (_audioStreams[i]) {
			_audioStreams[i]->finish();
			_audioStreams[i] = nullptr;
		}
	}
}

// queueAudioData -- Queue raw PCM data for playback on a track.
// Creates the queuing stream on first use. RA2 audio is 8-bit unsigned mono.
void InsaneRebel2::queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan) {
	if (trackIdx < 0 || trackIdx >= kRA2MaxAudioTracks || size <= 0 || !data) {
		debug(5, "InsaneRebel2::queueAudioData: Invalid params trackIdx=%d size=%d data=%p", trackIdx, size, (void*)data);
		return;
	}

	debug(5, "InsaneRebel2::queueAudioData: trackIdx=%d size=%d volume=%d pan=%d", trackIdx, size, volume, pan);

	// Create audio stream if not already active
	if (!_audioStreams[trackIdx]) {
		// RA2 audio is 8-bit unsigned mono at the track's sample rate
		debug("InsaneRebel2: Creating audio stream for track %d at %d Hz", trackIdx, _audioSampleRate);
		_audioStreams[trackIdx] = Audio::makeQueuingAudioStream(_audioSampleRate, false);
		_audioTrackActive[trackIdx] = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioHandles[trackIdx],
								_audioStreams[trackIdx], -1, Audio::Mixer::kMaxChannelVolume, 0,
								DisposeAfterUse::NO);
	}

	debug(6, "InsaneRebel2: Queueing %d bytes to track %d (vol=%d)", size, trackIdx, volume);

	// Copy the audio data since queueBuffer may need to own it
	byte *audioCopy = (byte *)malloc(size);
	if (!audioCopy) {
		return;
	}
	memcpy(audioCopy, data, size);

	// Queue the audio data - RA2 SMUSH audio is 8-bit unsigned mono
	_audioStreams[trackIdx]->queueBuffer(audioCopy, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	// Apply volume and pan to the channel
	int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;
	int scaledPan = (pan * 127) / 128;  // Convert -128..127 to -127..127
	_vm->_mixer->setChannelVolume(_audioHandles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_audioHandles[trackIdx], scaledPan);
}

//
// processAudioFrame -- Per-frame audio dispatch (replaces iMUSE path)
//
// Iterates SmushPlayer audio tracks, handles FADING->PLAYING transitions,
// and feeds PCM data through queueAudioData. Called from SmushPlayer when
// iMUSE is null.
//
void InsaneRebel2::processAudioFrame(int16 feedSize) {
	if (!_player) {
		return;
	}

	// Initialize dispatch data if needed (normally done in processDispatches for iMUSE games)
	if (_player->_smushTracksNeedInit) {
		_player->_smushTracksNeedInit = false;
		for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
			_player->_smushDispatch[i].fadeRemaining = 0;
			_player->_smushDispatch[i].fadeVolume = 0;
			_player->_smushDispatch[i].fadeSampleRate = 0;
			_player->_smushDispatch[i].elapsedAudio = 0;
			_player->_smushDispatch[i].audioLength = 0;
		}
	}

	// Access SmushPlayer's audio track data (InsaneRebel2 is a friend class)
	// Only iterate over actually allocated tracks (not SMUSH_MAX_TRACKS)
	for (int i = 0; i < _player->_smushNumTracks; i++) {
		SmushPlayer::SmushAudioTrack &track = _player->_smushTracks[i];
		SmushPlayer::SmushAudioDispatch &dispatch = _player->_smushDispatch[i];

		if (track.state == TRK_STATE_INACTIVE) {
			continue;
		}

		// Skip tracks that don't have valid buffer pointers yet
		// Note: dispatch.dataBuf is set when transitioning from FADING to PLAYING,
		// so tracks in FADING state won't have it set yet - that's OK, they'll be
		// transitioned below and then processed
		if (!track.blockPtr) {
			debug(5, "InsaneRebel2: Skipping track %d - blockPtr=%p state=%d",
				  i, (void*)track.blockPtr, track.state);
			continue;
		}

		// Check if this track type should be played
		bool isPlayableTrack = ((track.flags & TRK_TYPE_MASK) == IS_SPEECH && _player->isChanActive(CHN_SPEECH)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && _player->isChanActive(CHN_BKGMUS)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_SFX && _player->isChanActive(CHN_OTHER));

		if (!isPlayableTrack) {
			continue;
		}

		// Calculate base volume for this track type
		int baseVolume;
		switch (track.flags & TRK_TYPE_MASK) {
		case IS_SFX:
			baseVolume = (_player->_smushTrackVols[1] * track.volume) >> 7;
			break;
		case IS_BKG_MUSIC:
			baseVolume = (_player->_smushTrackVols[3] * track.volume) >> 7;
			break;
		case IS_SPEECH:
			baseVolume = (_player->_smushTrackVols[2] * track.volume) >> 7;
			break;
		default:
			baseVolume = track.volume;
			break;
		}
		int mixVolume = baseVolume * _player->_smushTrackVols[0] / 127;

		// Handle track state transitions: FADING -> PLAYING
		if (track.state == TRK_STATE_FADING) {
			dispatch.headerPtr = track.dataBuf;
			dispatch.dataBuf = track.subChunkPtr;
			dispatch.dataSize = track.dataSize;
			dispatch.currentOffset = 0;
			dispatch.audioLength = 0;
			track.state = TRK_STATE_PLAYING;
		}

		// Process audio for this track
		if (track.state != TRK_STATE_INACTIVE) {
			int32 tmpFeedSize = feedSize;

			while (tmpFeedSize > 0) {
				int32 mixInFrameCount = dispatch.currentOffset;

				// Use dispatch.dataBuf and dispatch.dataSize which are set consistently
				// when the track transitions from FADING to PLAYING, and audioRemaining
				// is calculated relative to these values by processAudioCodes
				if (mixInFrameCount > 0 && dispatch.dataBuf && dispatch.dataSize > 0) {
					// Ensure audioRemaining is non-negative for proper circular buffer access
					if (dispatch.audioRemaining < 0) {
						debug(5, "InsaneRebel2: Resetting negative audioRemaining=%d for track %d", dispatch.audioRemaining, i);
						dispatch.audioRemaining = 0;
					}
					int32 offset = dispatch.audioRemaining % dispatch.dataSize;

					// Limit to feed size proportional to sample rate
					if (dispatch.sampleRate > 0 && _player->_smushAudioSampleRate > 0) {
						int32 maxFrames = dispatch.sampleRate * tmpFeedSize / _player->_smushAudioSampleRate;
						if (mixInFrameCount > maxFrames) {
							mixInFrameCount = maxFrames;
						}
					}

					// Don't read past the buffer
					if (offset + mixInFrameCount > dispatch.dataSize) {
						mixInFrameCount = dispatch.dataSize - offset;
					}

					// Make sure we don't exceed available data
					if (dispatch.audioRemaining + mixInFrameCount > track.availableSize) {
						mixInFrameCount = track.availableSize - dispatch.audioRemaining;
						if (mixInFrameCount <= 0) {
							// Track is ending - no more data
							track.state = TRK_STATE_ENDING;
							break;
						}
					}

					if (mixInFrameCount > 0) {
						// Safety check: verify the pointer and offset are within bounds
						if (!dispatch.dataBuf || offset < 0 || offset + mixInFrameCount > dispatch.dataSize) {
							debug(1, "InsaneRebel2: Invalid audio buffer access track=%d dataBuf=%p offset=%d mixInFrameCount=%d dataSize=%d",
								  i, (void*)dispatch.dataBuf, offset, mixInFrameCount, dispatch.dataSize);
							break;
						}

						// Queue audio data directly to our audio streams
						queueAudioData(i, &dispatch.dataBuf[offset], mixInFrameCount, mixVolume, track.pan);

						// Update dispatch state
						dispatch.currentOffset -= mixInFrameCount;
						dispatch.audioRemaining += mixInFrameCount;

						// Calculate how much feed time was consumed
						if (dispatch.sampleRate > 0) {
							int32 consumedFeed = mixInFrameCount * _player->_smushAudioSampleRate / dispatch.sampleRate;
							tmpFeedSize -= consumedFeed;
						} else {
							tmpFeedSize -= mixInFrameCount;
						}
					}
				}

				// If currentOffset is depleted, process audio codes to get more
				if (dispatch.currentOffset <= 0) {
					// processAudioCodes returns true if there's more audio, false if done
					if (!_player->processAudioCodes(i, tmpFeedSize, mixVolume)) {
						break;
					}
					// If still no offset after processing codes, we're done
					if (dispatch.currentOffset <= 0) {
						break;
					}
				} else if (tmpFeedSize <= 0) {
					break;
				}
			}
		}

		track.audioRemaining = dispatch.audioRemaining;
		dispatch.state = track.state;
	}
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
			debug("InsaneRebel2::loadSfx: Could not open %s", kRA2SfxFiles[i]);
			delete file;
			continue;
		}

		// SAUD file structure: SAUD header (8) + STRK sub-chunk + SDAT sub-chunk
		// We scan for the SDAT tag to find the PCM data.
		uint32 fileSize = file->size();
		if (fileSize < 38) {  // Minimum: 8 (SAUD) + 22 (STRK) + 8 (SDAT header)
			debug("InsaneRebel2::loadSfx: %s too small (%d bytes)", kRA2SfxFiles[i], fileSize);
			file->close();
			delete file;
			continue;
		}

		// Verify SAUD tag
		uint32 tag = file->readUint32BE();
		if (tag != MKTAG('S', 'A', 'U', 'D')) {
			debug("InsaneRebel2::loadSfx: %s not a SAUD file (tag=0x%08x)", kRA2SfxFiles[i], tag);
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
					debug("InsaneRebel2::loadSfx: Loaded %s (%d bytes PCM)", kRA2SfxFiles[i], pcmSize);
				}
				foundSdat = true;
				break;
			} else {
				// Skip this sub-chunk
				file->seek(chunkSize, SEEK_CUR);
			}
		}

		if (!foundSdat) {
			debug("InsaneRebel2::loadSfx: No SDAT chunk in %s", kRA2SfxFiles[i]);
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

	// Scale volume from 0-127 to ScummVM's 0-255 range
	int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[slot],
		stream, -1, scaledVolume, pan);

	debug(5, "InsaneRebel2::playSfx: slot=%d vol=%d pan=%d size=%d", slot, volume, pan, _sfxSize[slot]);
}

// loadAuxSfx -- Load sound data into auxiliary buffer (FUN_004118df).
void InsaneRebel2::loadAuxSfx(int buffer, const byte *data, uint32 size) {
	if (buffer < 0 || buffer >= kRA2NumAuxSfx || !data || size == 0) {
		return;
	}
	if ((int)size > kRA2AuxBufSize) {
		debug("InsaneRebel2::loadAuxSfx: buffer %d size %d exceeds max %d, truncating",
			buffer, size, kRA2AuxBufSize);
		size = kRA2AuxBufSize;
	}

	memcpy(_auxSfxData[buffer], data, size);
	_auxSfxSize[buffer] = size;

	debug(5, "InsaneRebel2::loadAuxSfx: buffer=%d size=%d", buffer, size);
}

// playAuxSfx -- Play from auxiliary buffer (FUN_00411931).
// Handles both raw PCM and SAUD-wrapped data.
void InsaneRebel2::playAuxSfx(int buffer, int volume, int pan) {
	if (buffer < 0 || buffer >= kRA2NumAuxSfx || !_auxSfxData[buffer] || _auxSfxSize[buffer] == 0) {
		return;
	}

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

	int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_auxSfxHandles[buffer],
		stream, -1, scaledVolume, pan);

	debug(5, "InsaneRebel2::playAuxSfx: buffer=%d vol=%d pan=%d pcmSize=%d", buffer, volume, pan, pcmSize);
}

} // End of namespace Scumm
