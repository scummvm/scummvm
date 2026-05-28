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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/rebel/rebel_audio.h"

namespace Scumm {

RebelAudio::RebelAudio() : _vm(nullptr), _sampleRate(11025) {
	for (int i = 0; i < kMaxTracks; i++) {
		_streams[i] = nullptr;
		_trackActive[i] = false;
	}
}

void RebelAudio::init(ScummEngine_v7 *vm, int sampleRate) {
	_vm = vm;
	_sampleRate = sampleRate;
	for (int i = 0; i < kMaxTracks; i++) {
		_streams[i] = nullptr;
		_trackActive[i] = false;
	}
}

void RebelAudio::terminate() {
	if (!_vm)
		return;

	for (int i = 0; i < kMaxTracks; i++) {
		if (_trackActive[i]) {
			_vm->_mixer->stopHandle(_handles[i]);
			_trackActive[i] = false;
		}
		if (_streams[i]) {
			_streams[i]->finish();
			_streams[i] = nullptr;
		}
	}
}

void RebelAudio::queueData(int trackIdx, const uint8 *data, int32 size, int volume, int pan) {
	if (!_vm || trackIdx < 0 || trackIdx >= kMaxTracks || size <= 0 || !data)
		return;

	if (!_streams[trackIdx]) {
		debug(1, "RebelAudio: Creating audio stream for track %d at %d Hz", trackIdx, _sampleRate);
		_streams[trackIdx] = Audio::makeQueuingAudioStream(_sampleRate, false);
		_trackActive[trackIdx] = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handles[trackIdx],
			_streams[trackIdx], -1, Audio::Mixer::kMaxChannelVolume, 0,
			DisposeAfterUse::NO);
	}

	byte *audioCopy = (byte *)malloc(size);
	if (!audioCopy)
		return;
	memcpy(audioCopy, data, size);

	_streams[trackIdx]->queueBuffer(audioCopy, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	const int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;
	const int scaledPan = (pan * 127) / 128;
	_vm->_mixer->setChannelVolume(_handles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_handles[trackIdx], scaledPan);
}

void RebelAudio::processFrame(SmushPlayer *player, int16 feedSize) {
	if (!player)
		return;

	if (player->_smushTracksNeedInit) {
		player->_smushTracksNeedInit = false;
		for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
			player->_smushDispatch[i].fadeRemaining = 0;
			player->_smushDispatch[i].fadeVolume = 0;
			player->_smushDispatch[i].fadeSampleRate = 0;
			player->_smushDispatch[i].elapsedAudio = 0;
			player->_smushDispatch[i].audioLength = 0;
		}
	}

	for (int i = 0; i < player->_smushNumTracks; i++) {
		SmushPlayer::SmushAudioTrack &track = player->_smushTracks[i];
		SmushPlayer::SmushAudioDispatch &dispatch = player->_smushDispatch[i];

		if (track.state == TRK_STATE_INACTIVE || !track.blockPtr)
			continue;

		bool isPlayableTrack =
			((track.flags & TRK_TYPE_MASK) == IS_SPEECH && player->isChanActive(CHN_SPEECH)) ||
			((track.flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && player->isChanActive(CHN_BKGMUS)) ||
			((track.flags & TRK_TYPE_MASK) == IS_SFX && player->isChanActive(CHN_OTHER));

		if (!isPlayableTrack)
			continue;

		int baseVolume;
		switch (track.flags & TRK_TYPE_MASK) {
		case IS_SFX:
			baseVolume = (player->_smushTrackVols[1] * track.volume) >> 7;
			break;
		case IS_BKG_MUSIC:
			baseVolume = (player->_smushTrackVols[3] * track.volume) >> 7;
			break;
		case IS_SPEECH:
			baseVolume = (player->_smushTrackVols[2] * track.volume) >> 7;
			break;
		default:
			baseVolume = track.volume;
			break;
		}
		int mixVolume = baseVolume * player->_smushTrackVols[0] / 127;

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

					if (dispatch.sampleRate > 0 && player->_smushAudioSampleRate > 0) {
						int32 maxFrames = dispatch.sampleRate * tmpFeedSize / player->_smushAudioSampleRate;
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

						queueData(i, &dispatch.dataBuf[offset], mixInFrameCount, mixVolume, track.pan);

						dispatch.currentOffset -= mixInFrameCount;
						dispatch.audioRemaining += mixInFrameCount;

						if (dispatch.sampleRate > 0) {
							int32 consumedFeed = mixInFrameCount * player->_smushAudioSampleRate / dispatch.sampleRate;
							tmpFeedSize -= consumedFeed;
						} else {
							tmpFeedSize -= mixInFrameCount;
						}
					}
				}

				if (dispatch.currentOffset <= 0) {
					if (!player->processAudioCodes(i, tmpFeedSize, mixVolume))
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

} // End of namespace Scumm
