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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

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

} // End of namespace Scumm
