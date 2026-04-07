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
	_audioSampleRate = sampleRate;
	for (int i = 0; i < kMaxAudioTracks; i++) {
		_audioStreams[i] = nullptr;
		_audioTrackActive[i] = false;
	}
}

void InsaneRebel1::terminateAudio() {
	for (int i = 0; i < kMaxAudioTracks; i++) {
		if (_audioTrackActive[i]) {
			_vm->_mixer->stopHandle(_audioHandles[i]);
			_audioTrackActive[i] = false;
		}
		if (_audioStreams[i]) {
			_audioStreams[i]->finish();
			_audioStreams[i] = nullptr;
		}
	}

	for (int i = 0; i < kNumSfx; i++) {
		_vm->_mixer->stopHandle(_sfxHandles[i]);
	}
}

void InsaneRebel1::queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan) {
	if (trackIdx < 0 || trackIdx >= kMaxAudioTracks || size <= 0 || !data)
		return;

	if (!_audioStreams[trackIdx]) {
		debug(1, "InsaneRebel1: Creating audio stream for track %d at %d Hz", trackIdx, _audioSampleRate);
		_audioStreams[trackIdx] = Audio::makeQueuingAudioStream(_audioSampleRate, false);
		_audioTrackActive[trackIdx] = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioHandles[trackIdx],
								_audioStreams[trackIdx], -1, Audio::Mixer::kMaxChannelVolume, 0,
								DisposeAfterUse::NO);
	}

	byte *audioCopy = (byte *)malloc(size);
	if (!audioCopy)
		return;
	memcpy(audioCopy, data, size);

	_audioStreams[trackIdx]->queueBuffer(audioCopy, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;
	int scaledPan = (pan * 127) / 128;
	_vm->_mixer->setChannelVolume(_audioHandles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_audioHandles[trackIdx], scaledPan);
}

void InsaneRebel1::processAudioFrame(int16 feedSize) {
	if (!_player)
		return;

	SmushPlayer *sp = _player;

	if (sp->_smushTracksNeedInit) {
		sp->_smushTracksNeedInit = false;
		for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
			sp->_smushDispatch[i].fadeRemaining = 0;
			sp->_smushDispatch[i].fadeVolume = 0;
			sp->_smushDispatch[i].fadeSampleRate = 0;
			sp->_smushDispatch[i].elapsedAudio = 0;
			sp->_smushDispatch[i].audioLength = 0;
		}
	}

	for (int i = 0; i < sp->_smushNumTracks; i++) {
		SmushPlayer::SmushAudioTrack &track = sp->_smushTracks[i];
		SmushPlayer::SmushAudioDispatch &dispatch = sp->_smushDispatch[i];

		if (track.state == TRK_STATE_INACTIVE || !track.blockPtr)
			continue;

		bool isPlayableTrack = ((track.flags & TRK_TYPE_MASK) == IS_SPEECH && sp->isChanActive(CHN_SPEECH)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && sp->isChanActive(CHN_BKGMUS)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_SFX && sp->isChanActive(CHN_OTHER));

		if (!isPlayableTrack)
			continue;

		int baseVolume;
		switch (track.flags & TRK_TYPE_MASK) {
		case IS_SFX:
			baseVolume = (sp->_smushTrackVols[1] * track.volume) >> 7;
			break;
		case IS_BKG_MUSIC:
			baseVolume = (sp->_smushTrackVols[3] * track.volume) >> 7;
			break;
		case IS_SPEECH:
			baseVolume = (sp->_smushTrackVols[2] * track.volume) >> 7;
			break;
		default:
			baseVolume = track.volume;
			break;
		}
		int mixVolume = baseVolume * sp->_smushTrackVols[0] / 127;

		// Handle FADING -> PLAYING transition
		if (track.state == TRK_STATE_FADING) {
			dispatch.headerPtr = track.dataBuf;
			dispatch.dataBuf = track.subChunkPtr;
			dispatch.dataSize = track.dataSize;
			dispatch.currentOffset = 0;
			dispatch.audioLength = 0;
			track.state = TRK_STATE_PLAYING;
		}

		if (track.state != TRK_STATE_INACTIVE) {
			int32 tmpFeedSize = feedSize;

			while (tmpFeedSize > 0) {
				int32 mixInFrameCount = dispatch.currentOffset;

				if (mixInFrameCount > 0 && dispatch.dataBuf && dispatch.dataSize > 0) {
					if (dispatch.audioRemaining < 0)
						dispatch.audioRemaining = 0;

					int32 offset = dispatch.audioRemaining % dispatch.dataSize;

					if (dispatch.sampleRate > 0 && sp->_smushAudioSampleRate > 0) {
						int32 maxFrames = dispatch.sampleRate * tmpFeedSize / sp->_smushAudioSampleRate;
						if (mixInFrameCount > maxFrames)
							mixInFrameCount = maxFrames;
					}

					if (offset + mixInFrameCount > dispatch.dataSize)
						mixInFrameCount = dispatch.dataSize - offset;

					if (dispatch.audioRemaining + mixInFrameCount > track.availableSize) {
						mixInFrameCount = track.availableSize - dispatch.audioRemaining;
						if (mixInFrameCount <= 0) {
							track.state = TRK_STATE_ENDING;
							break;
						}
					}

					if (mixInFrameCount > 0) {
						if (!dispatch.dataBuf || offset < 0 || offset + mixInFrameCount > dispatch.dataSize)
							break;

						queueAudioData(i, &dispatch.dataBuf[offset], mixInFrameCount, mixVolume, track.pan);

						dispatch.currentOffset -= mixInFrameCount;
						dispatch.audioRemaining += mixInFrameCount;

						if (dispatch.sampleRate > 0) {
							int32 consumedFeed = mixInFrameCount * sp->_smushAudioSampleRate / dispatch.sampleRate;
							tmpFeedSize -= consumedFeed;
						} else {
							tmpFeedSize -= mixInFrameCount;
						}
					}
				}

				if (dispatch.currentOffset <= 0) {
					if (!sp->processAudioCodes(i, tmpFeedSize, mixVolume))
						break;
					if (dispatch.currentOffset <= 0)
						break;
				} else if (tmpFeedSize <= 0) {
					break;
				}
			}
		}

		track.audioRemaining = dispatch.audioRemaining;
		dispatch.state = track.state;
	}
}

void InsaneRebel1::loadSfx() {
	for (int i = 0; i < kNumSfx; i++) {
		if (_sfxData[i] || _sfxSize[i] != 0)
			continue;

		ScummFile *file = _vm->instantiateScummFile();
		_vm->openFile(*file, kRA1SfxFiles[i]);
		if (!file->isOpen()) {
			debug("InsaneRebel1::loadSfx: could not open %s", kRA1SfxFiles[i]);
			delete file;
			continue;
		}

		const uint32 fileSize = file->size();
		if (fileSize < 16) {
			debug("InsaneRebel1::loadSfx: %s too small (%u bytes)", kRA1SfxFiles[i], fileSize);
			file->close();
			delete file;
			continue;
		}

		const uint32 tag = file->readUint32BE();
		if (tag != MKTAG('S', 'A', 'U', 'D')) {
			debug("InsaneRebel1::loadSfx: %s not a SAUD file (tag=0x%08x)", kRA1SfxFiles[i], tag);
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
					debug("InsaneRebel1::loadSfx: loaded %s (%u bytes PCM)", kRA1SfxFiles[i], pcmSize);
				}
				foundSdat = true;
				break;
			}
			file->seek(chunkSize, SEEK_CUR);
		}

		if (!foundSdat)
			debug("InsaneRebel1::loadSfx: no SDAT chunk in %s", kRA1SfxFiles[i]);

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
