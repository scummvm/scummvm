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

void RebelAudio::reset() {
	if (!_vm)
		return;

	for (int i = 0; i < kMaxTracks; i++) {
		if (_trackActive[i]) {
			_vm->_mixer->stopHandle(_handles[i]);
			_trackActive[i] = false;
		}
		if (_streams[i]) {
			_streams[i]->finish();
			delete _streams[i];
			_streams[i] = nullptr;
		}
	}
}

void RebelAudio::terminate() {
	reset();
}

void RebelAudio::queueData(int trackIdx, const uint8 *data, int32 size, int volume, int pan, int sampleRate) {
	if (!_vm || trackIdx < 0 || trackIdx >= kMaxTracks || size <= 0 || !data)
		return;

	const int sourceRate = sampleRate > 0 ? sampleRate : _sampleRate;

	if (!_streams[trackIdx]) {
		debug(1, "RebelAudio: Creating audio stream for track %d at %d Hz", trackIdx, _sampleRate);
		_streams[trackIdx] = Audio::makeQueuingAudioStream(_sampleRate, false);
		_trackActive[trackIdx] = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handles[trackIdx],
			_streams[trackIdx], -1, Audio::Mixer::kMaxChannelVolume, 0,
			DisposeAfterUse::NO);
	}

	int32 queueSize = size;
	if (sourceRate != _sampleRate) {
		queueSize = ((int64)size * _sampleRate + sourceRate / 2) / sourceRate;
		if (queueSize <= 0)
			queueSize = 1;
	}

	byte *audioCopy = (byte *)malloc(queueSize);
	if (!audioCopy)
		return;

	if (sourceRate == _sampleRate) {
		memcpy(audioCopy, data, queueSize);
	} else {
		for (int32 i = 0; i < queueSize; i++) {
			int32 srcPos = ((int64)i * sourceRate) / _sampleRate;
			if (srcPos >= size)
				srcPos = size - 1;
			audioCopy[i] = data[srcPos];
		}
	}

	_streams[trackIdx]->queueBuffer(audioCopy, queueSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	const int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;
	const int scaledPan = (pan * 127) / 128;
	_vm->_mixer->setChannelVolume(_handles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_handles[trackIdx], scaledPan);
}

bool RebelAudio::processAudioCodes(SmushPlayer *player, int idx, int32 &tmpFeedSize, int &mixVolume) {
	uint8 *code, *buf, subcode, value;
	int chunk;

	while (tmpFeedSize) {
		code = player->_smushDispatch[idx].headerPtr;

		switch (code[0]) {
		case SAUD_OP_INIT:
			player->_smushDispatch[idx].audioLength = 0;
			buf = player->_smushDispatch[idx].headerPtr;
			player->_smushDispatch[idx].audioRemaining = READ_BE_UINT32(buf + 2);
			player->_smushDispatch[idx].currentOffset = READ_BE_UINT32(buf + 6);
			player->_smushDispatch[idx].sampleRate = player->_smushAudioSampleRate;
			player->_smushDispatch[idx].headerPtr += player->_smushDispatch[idx].headerPtr[1] + 2;
			if (player->_smushDispatch[idx].audioRemaining < player->_smushTracks[idx].availableSize + (player->_smushTracks[idx].availableSize >= player->_smushTracks[idx].sdatSize ? 0 : 15000) - player->_smushTracks[idx].dataSize) {
				chunk = player->_smushTracks[idx].availableSize - player->_smushTracks[idx].dataSize - player->_smushDispatch[idx].audioRemaining + 15000;
				if (chunk > player->_smushDispatch[idx].currentOffset) {
					player->_smushTracks[idx].state = TRK_STATE_INACTIVE;
					player->_smushTracks[idx].groupId = GRP_MASTER;
					tmpFeedSize = 0;
					break;
				}

				player->_smushDispatch[idx].audioRemaining += chunk;
				player->_smushDispatch[idx].currentOffset -= chunk;
			}
			break;

		case SAUD_OP_UPDATE_HEADER:
		case SAUD_OP_COMPARE_GT:
		case SAUD_OP_COMPARE_LT:
		case SAUD_OP_COMPARE_EQ:
		case SAUD_OP_COMPARE_NE:
			subcode = code[4];
			switch (subcode) {
			case SAUD_VALUEID_ALL_VOLS:
				value = player->_smushTrackVols[0];
				break;
			case SAUD_VALUEID_TRK_VOL:
				value = player->_smushTracks[idx].volume;
				break;
			case SAUD_VALUEID_TRK_PAN:
				value = player->_smushTracks[idx].pan;
				break;
			default:
				value = player->_smushAudioTable[subcode];
				break;
			}

			switch (code[0]) {
			case SAUD_OP_UPDATE_HEADER:
				value = value || (subcode == 0);
				break;
			case SAUD_OP_COMPARE_GT:
				value = value > code[5];
				break;
			case SAUD_OP_COMPARE_LT:
				value = value < code[5];
				break;
			case SAUD_OP_COMPARE_EQ:
				value = value == code[5];
				break;
			case SAUD_OP_COMPARE_NE:
				value = value != code[5];
				break;
			default:
				break;
			}

			if (!value) {
				player->_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			} else {
				// Rebel Assault SAUD branch targets are signed relative displacements.
				player->_smushDispatch[idx].headerPtr = code + READ_BE_INT16(&code[2]);
			}
			break;

		case SAUD_OP_SET_PARAM:
			switch (code[2]) {
			case SAUD_VALUEID_ALL_VOLS:
				player->_smushTrackVols[0] = code[3];
				break;
			case SAUD_VALUEID_TRK_VOL:
				player->_smushTracks[idx].volume = code[3];
				mixVolume = (player->_smushTrackVols[0] * player->_smushTracks[idx].volume) / 127;

				if ((player->_smushTracks[idx].flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && player->isChanActive(CHN_SPEECH))
					mixVolume = (mixVolume * player->_gainReductionMultiplier) >> 8;
				break;
			case SAUD_VALUEID_TRK_PAN:
				player->_smushTracks[idx].pan = code[3];
				break;
			default:
				player->_smushAudioTable[code[2]] = code[3];
				break;
			}
			player->_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			break;

		case SAUD_OP_INCR_PARAM:
			switch (code[2]) {
			case SAUD_VALUEID_ALL_VOLS:
				player->_smushTrackVols[0] += code[3];
				break;
			case SAUD_VALUEID_TRK_VOL:
				player->_smushTracks[idx].volume += code[3];
				break;
			case SAUD_VALUEID_TRK_PAN:
				player->_smushTracks[idx].pan += code[3];
				break;
			default:
				player->_smushAudioTable[code[2]] += code[3];
				break;
			}
			player->_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			break;

		case SAUD_OP_SET_OFFSET:
			player->_smushDispatch[idx].audioLength = 0;
			buf = player->_smushDispatch[idx].headerPtr;
			player->_smushDispatch[idx].audioRemaining = READ_BE_UINT32(buf + 2);
			player->_smushDispatch[idx].currentOffset = READ_BE_UINT32(buf + 6);
			player->_smushDispatch[idx].sampleRate = READ_BE_UINT32(buf + 10);

			player->_smushDispatch[idx].headerPtr += player->_smushDispatch[idx].headerPtr[1] + 2;
			if (player->_smushDispatch[idx].audioRemaining < player->_smushTracks[idx].availableSize + (player->_smushTracks[idx].availableSize >= player->_smushTracks[idx].sdatSize ? 0 : 15000) - player->_smushTracks[idx].dataSize) {
				chunk = player->_smushTracks[idx].availableSize - player->_smushTracks[idx].dataSize - player->_smushDispatch[idx].audioRemaining + 15000;
				if (chunk > player->_smushDispatch[idx].currentOffset) {
					player->_smushTracks[idx].state = TRK_STATE_INACTIVE;
					player->_smushTracks[idx].groupId = GRP_MASTER;
					tmpFeedSize = 0;
					break;
				}

				player->_smushDispatch[idx].audioRemaining += chunk;
				player->_smushDispatch[idx].currentOffset -= chunk;
			}
			break;

		case SAUD_OP_SET_LENGTH:
			if (!player->_smushDispatch[idx].audioLength) {
				player->_smushDispatch[idx].audioLength = READ_BE_UINT32(&code[6]);
				player->_smushDispatch[idx].elapsedAudio = 0;
			}

			buf = player->_smushDispatch[idx].headerPtr;
			player->_smushDispatch[idx].audioRemaining = player->_smushDispatch[idx].elapsedAudio + READ_BE_UINT32(buf + 2);

			player->_smushDispatch[idx].currentOffset = READ_BE_UINT32(buf + 14);
			if (player->_smushDispatch[idx].currentOffset > player->_smushDispatch[idx].audioLength)
				player->_smushDispatch[idx].currentOffset = player->_smushDispatch[idx].audioLength;

			player->_smushDispatch[idx].sampleRate = READ_BE_UINT32(buf + 10);

			player->_smushDispatch[idx].audioLength -= player->_smushDispatch[idx].currentOffset;
			player->_smushDispatch[idx].elapsedAudio += player->_smushDispatch[idx].currentOffset;

			if (player->_smushDispatch[idx].audioLength) {
				player->_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			} else {
				player->_smushDispatch[idx].headerPtr = code + READ_BE_INT16(&code[18]);
			}

			if (player->_smushDispatch[idx].audioRemaining >= player->_smushTracks[idx].availableSize + (player->_smushTracks[idx].availableSize >= player->_smushTracks[idx].sdatSize ? 0 : 15000) - player->_smushTracks[idx].dataSize) {
				chunk = player->_smushTracks[idx].availableSize - player->_smushTracks[idx].dataSize - player->_smushDispatch[idx].audioRemaining + 15000;
				if (chunk > player->_smushDispatch[idx].currentOffset) {
					player->_smushTracks[idx].state = TRK_STATE_INACTIVE;
					player->_smushTracks[idx].groupId = GRP_MASTER;
					tmpFeedSize = 0;
				} else {
					player->_smushDispatch[idx].audioRemaining += chunk;
					player->_smushDispatch[idx].currentOffset -= chunk;
				}
			}
			break;

		default:
			player->_smushTracks[idx].state = TRK_STATE_INACTIVE;
			player->_smushTracks[idx].groupId = GRP_MASTER;
			tmpFeedSize = 0;
		}

		if (player->_smushDispatch[idx].currentOffset > 0)
			return false;
	}

	return true;
}

void RebelAudio::processFrame(SmushPlayer *player, int16 feedSize) {
	if (!player)
		return;

	if (player->_paused)
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
						int32 maxFrames = ((int64)dispatch.sampleRate * tmpFeedSize +
							player->_smushAudioSampleRate - 1) / player->_smushAudioSampleRate;
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

						track.state = TRK_STATE_PLAYING;
						queueData(i, &dispatch.dataBuf[offset], mixInFrameCount, mixVolume, track.pan, dispatch.sampleRate);

						dispatch.currentOffset -= mixInFrameCount;
						dispatch.audioRemaining += mixInFrameCount;

						if (dispatch.sampleRate > 0) {
							int32 consumedFeed = ((int64)mixInFrameCount * player->_smushAudioSampleRate +
								dispatch.sampleRate - 1) / dispatch.sampleRate;
							tmpFeedSize -= consumedFeed;
						} else {
							tmpFeedSize -= mixInFrameCount;
						}
					}
				}

				if (dispatch.currentOffset <= 0) {
					if (processAudioCodes(player, i, tmpFeedSize, mixVolume) && tmpFeedSize <= 0)
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
