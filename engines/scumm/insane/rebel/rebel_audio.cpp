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

#include "common/util.h"

#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/rebel/rebel_audio.h"

namespace Scumm {

RebelAudio::RebelAudio() : _vm(nullptr), _sampleRate(11025) {
	for (int i = 0; i < kMaxTracks; i++) {
		_streams[i] = nullptr;
		_fadeStreams[i] = nullptr;
		_trackActive[i] = false;
		_fadeTrackActive[i] = false;
		_streamTypes[i] = Audio::Mixer::kSFXSoundType;
		_fadeStreamTypes[i] = Audio::Mixer::kSFXSoundType;
	}
}

void RebelAudio::init(ScummEngine_v7 *vm, int sampleRate) {
	_vm = vm;
	_sampleRate = sampleRate;
	for (int i = 0; i < kMaxTracks; i++) {
		_streams[i] = nullptr;
		_fadeStreams[i] = nullptr;
		_trackActive[i] = false;
		_fadeTrackActive[i] = false;
		_streamTypes[i] = Audio::Mixer::kSFXSoundType;
		_fadeStreamTypes[i] = Audio::Mixer::kSFXSoundType;
	}
}

void RebelAudio::reset() {
	if (!_vm)
		return;

	for (int i = 0; i < kMaxTracks; i++) {
		stopStream(i);
		stopFadeStream(i);
	}
}

void RebelAudio::terminate() {
	reset();
}

void RebelAudio::queueData(int trackIdx, const uint8 *data, int32 size, int volume, int pan, int sampleRate) {
	queueData(trackIdx, data, size, volume, pan, sampleRate, Audio::Mixer::kSFXSoundType);
}

void RebelAudio::stopStream(int trackIdx) {
	if (!_vm || trackIdx < 0 || trackIdx >= kMaxTracks)
		return;

	if (_trackActive[trackIdx]) {
		_vm->_mixer->stopHandle(_handles[trackIdx]);
		_trackActive[trackIdx] = false;
	}

	if (_streams[trackIdx]) {
		_streams[trackIdx]->finish();
		delete _streams[trackIdx];
		_streams[trackIdx] = nullptr;
	}
}

void RebelAudio::stopFadeStream(int trackIdx) {
	if (!_vm || trackIdx < 0 || trackIdx >= kMaxTracks)
		return;

	if (_fadeTrackActive[trackIdx]) {
		_vm->_mixer->stopHandle(_fadeHandles[trackIdx]);
		_fadeTrackActive[trackIdx] = false;
	}

	if (_fadeStreams[trackIdx]) {
		_fadeStreams[trackIdx]->finish();
		delete _fadeStreams[trackIdx];
		_fadeStreams[trackIdx] = nullptr;
	}
}

Audio::Mixer::SoundType RebelAudio::getSoundType(int flags) const {
	switch (flags & TRK_TYPE_MASK) {
	case IS_SPEECH:
		return Audio::Mixer::kSpeechSoundType;
	case IS_BKG_MUSIC:
		return Audio::Mixer::kMusicSoundType;
	case IS_SFX:
	default:
		return Audio::Mixer::kSFXSoundType;
	}
}

int RebelAudio::getTrackPan(SmushPlayer *player, int idx) const {
	if (!player || idx < 0 || idx >= player->_smushNumTracks)
		return 0;

	return CLIP<int>((int8)player->_smushTracks[idx].pan, -127, 127);
}

int RebelAudio::computeMixVolume(SmushPlayer *player, int idx) const {
	if (!player || idx < 0 || idx >= player->_smushNumTracks)
		return 0;

	const SmushPlayer::SmushAudioTrack &track = player->_smushTracks[idx];
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

	int mixVolume = (baseVolume * player->_smushTrackVols[0]) / 127;
	if ((track.flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && player->isChanActive(CHN_SPEECH))
		mixVolume = (mixVolume * player->_gainReductionMultiplier) >> 8;

	return CLIP<int>(mixVolume, 0, 127);
}

void RebelAudio::updateTrackMixerState(SmushPlayer *player, int idx) {
	if (!_vm || !player || idx < 0 || idx >= player->_smushNumTracks || idx >= kMaxTracks)
		return;

	if (_trackActive[idx]) {
		const int scaledVolume = (computeMixVolume(player, idx) * Audio::Mixer::kMaxChannelVolume) / 127;
		const int scaledPan = (getTrackPan(player, idx) * 127) / 128;
		_vm->_mixer->setChannelVolume(_handles[idx], scaledVolume);
		_vm->_mixer->setChannelBalance(_handles[idx], scaledPan);
	}
}

void RebelAudio::queueFadeData(int trackIdx, const uint8 *data, int32 size, int volume, int pan, int sampleRate, Audio::Mixer::SoundType soundType) {
	if (!_vm || trackIdx < 0 || trackIdx >= kMaxTracks || size <= 0 || !data)
		return;

	const int sourceRate = sampleRate > 0 ? sampleRate : _sampleRate;

	if (_fadeStreams[trackIdx] && _fadeStreamTypes[trackIdx] != soundType)
		stopFadeStream(trackIdx);

	if (!_fadeStreams[trackIdx]) {
		debugC(DEBUG_INSANE, "RebelAudio: Creating fade audio stream for track %d at %d Hz", trackIdx, _sampleRate);
		_fadeStreams[trackIdx] = Audio::makeQueuingAudioStream(_sampleRate, false);
		_fadeStreamTypes[trackIdx] = soundType;
		_fadeTrackActive[trackIdx] = true;
		_vm->_mixer->playStream(soundType, &_fadeHandles[trackIdx],
			_fadeStreams[trackIdx], -1, Audio::Mixer::kMaxChannelVolume, 0,
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

	_fadeStreams[trackIdx]->queueBuffer(audioCopy, queueSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	const int scaledVolume = (CLIP<int>(volume, 0, 127) * Audio::Mixer::kMaxChannelVolume) / 127;
	const int scaledPan = (CLIP<int>(pan, -127, 127) * 127) / 128;
	_vm->_mixer->setChannelVolume(_fadeHandles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_fadeHandles[trackIdx], scaledPan);
}

void RebelAudio::queueData(int trackIdx, const uint8 *data, int32 size, int volume, int pan, int sampleRate, Audio::Mixer::SoundType soundType) {
	if (!_vm || trackIdx < 0 || trackIdx >= kMaxTracks || size <= 0 || !data)
		return;

	const int sourceRate = sampleRate > 0 ? sampleRate : _sampleRate;

	if (_streams[trackIdx] && _streamTypes[trackIdx] != soundType)
		stopStream(trackIdx);

	if (!_streams[trackIdx]) {
		debugC(DEBUG_INSANE, "RebelAudio: Creating audio stream for track %d at %d Hz", trackIdx, _sampleRate);
		_streams[trackIdx] = Audio::makeQueuingAudioStream(_sampleRate, false);
		_streamTypes[trackIdx] = soundType;
		_trackActive[trackIdx] = true;
		_vm->_mixer->playStream(soundType, &_handles[trackIdx],
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

	const int scaledVolume = (CLIP<int>(volume, 0, 127) * Audio::Mixer::kMaxChannelVolume) / 127;
	const int scaledPan = (CLIP<int>(pan, -127, 127) * 127) / 128;
	_vm->_mixer->setChannelVolume(_handles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_handles[trackIdx], scaledPan);
}

void RebelAudio::setGroupVolume(SmushPlayer *player, int groupId, int volume) {
	if (!player)
		return;

	volume = CLIP<int>(volume, 0, 127);
	switch (groupId) {
	case GRP_MASTER:
		player->_smushTrackVols[0] = volume;
		for (int i = 0; i < player->_smushNumTracks; i++)
			updateTrackMixerState(player, i);
		return;
	case GRP_SFX:
		player->_smushTrackVols[1] = volume;
		for (int i = 0; i < player->_smushNumTracks; i++)
			updateTrackMixerState(player, i);
		return;
	case GRP_BKGMUS:
		player->_smushTrackVols[3] = volume;
		for (int i = 0; i < player->_smushNumTracks; i++)
			updateTrackMixerState(player, i);
		return;
	case GRP_SPEECH:
		player->_smushTrackVols[2] = volume;
		for (int i = 0; i < player->_smushNumTracks; i++)
			updateTrackMixerState(player, i);
		return;
	default:
		break;
	}

	for (int i = 0; i < player->_smushNumTracks; i++) {
		if (player->_smushTracks[i].groupId == groupId) {
			player->_smushTracks[i].volume = volume;
			updateTrackMixerState(player, i);
		}
	}
}

void RebelAudio::setGroupPan(SmushPlayer *player, int groupId, int pan) {
	if (!player)
		return;

	pan = CLIP<int>(pan, -127, 127);
	for (int i = 0; i < player->_smushNumTracks; i++) {
		if (player->_smushTracks[i].groupId == groupId) {
			player->_smushTracks[i].pan = (byte)(int8)pan;
			updateTrackMixerState(player, i);
		}
	}
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
				mixVolume = computeMixVolume(player, idx);
				break;
			case SAUD_VALUEID_TRK_VOL:
				player->_smushTracks[idx].volume = code[3];
				mixVolume = computeMixVolume(player, idx);
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
				mixVolume = computeMixVolume(player, idx);
				break;
			case SAUD_VALUEID_TRK_VOL:
				player->_smushTracks[idx].volume += code[3];
				mixVolume = computeMixVolume(player, idx);
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

	const int kMaxMixChunkSize = 0x100;
	bool speechIsPlaying = false;

	if (player->_smushTracksNeedInit) {
		player->_smushTracksNeedInit = false;
		for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
			player->_smushDispatch[i].fadeRemaining = 0;
			player->_smushDispatch[i].fadeVolume = 0;
			player->_smushDispatch[i].fadeSampleRate = 0;
			player->_smushDispatch[i].volumeStep = 0;
			player->_smushDispatch[i].elapsedAudio = 0;
			player->_smushDispatch[i].audioLength = 0;
		}
	}

	for (int i = 0; i < player->_smushNumTracks; i++) {
		SmushPlayer::SmushAudioTrack &track = player->_smushTracks[i];
		SmushPlayer::SmushAudioDispatch &dispatch = player->_smushDispatch[i];

		if (!track.blockPtr)
			continue;

		const bool isPlayableTrack =
			((track.flags & TRK_TYPE_MASK) == IS_SPEECH && player->isChanActive(CHN_SPEECH)) ||
			((track.flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && player->isChanActive(CHN_BKGMUS)) ||
			((track.flags & TRK_TYPE_MASK) == IS_SFX && player->isChanActive(CHN_OTHER));

		const Audio::Mixer::SoundType soundType = getSoundType(track.flags);
		int mixVolume = computeMixVolume(player, i);

		if (track.state == TRK_STATE_FADING && dispatch.state == TRK_STATE_PLAYING &&
				dispatch.dataBuf && dispatch.dataSize > 0 && track.fadeBuf) {
			const int32 fadeStartOffset = dispatch.audioRemaining % dispatch.dataSize;
			dispatch.fadeRemaining = SMUSH_FADE_SIZE;
			dispatch.fadeVolume = track.volume;
			dispatch.fadeSampleRate = dispatch.sampleRate;

			memset(track.fadeBuf, 127, dispatch.fadeRemaining);
			int32 copySize = dispatch.dataSize - fadeStartOffset;
			if (copySize > dispatch.fadeRemaining)
				copySize = dispatch.fadeRemaining;
			if (copySize > 0)
				memcpy(track.fadeBuf, &dispatch.dataBuf[fadeStartOffset], copySize);

			dispatch.volumeStep = 0;
			stopFadeStream(i);
			stopStream(i);
		} else if (track.state == TRK_STATE_PLAYING && dispatch.dataBuf && dispatch.dataSize > 0 && track.fadeBuf) {
			if (dispatch.audioRemaining < track.availableSize - track.dataSize + 15000 && track.availableSize < track.sdatSize) {
				dispatch.volumeStep = 0;
				int32 mixInFrameCount = track.availableSize - dispatch.audioRemaining - 15000;

				if (mixInFrameCount > dispatch.currentOffset)
					mixInFrameCount = dispatch.currentOffset;

				if (dispatch.audioRemaining + mixInFrameCount > track.sdatSize - dispatch.dataSize)
					mixInFrameCount = track.sdatSize - dispatch.dataSize - dispatch.audioRemaining;

				if (mixInFrameCount > 0) {
					const int32 fadeStartOffset = dispatch.audioRemaining % dispatch.dataSize;
					dispatch.fadeRemaining = SMUSH_FADE_SIZE;
					dispatch.fadeSampleRate = dispatch.sampleRate;

					memset(track.fadeBuf, 127, dispatch.fadeRemaining);
					int32 copySize = dispatch.dataSize - fadeStartOffset;
					if (copySize > dispatch.fadeRemaining)
						copySize = dispatch.fadeRemaining;
					if (copySize > 0)
						memcpy(track.fadeBuf, &dispatch.dataBuf[fadeStartOffset], copySize);

					dispatch.audioRemaining += mixInFrameCount;
					dispatch.currentOffset -= mixInFrameCount;
					stopFadeStream(i);
				}
			}
		}

		if (dispatch.fadeRemaining && dispatch.fadeSampleRate > 0 && player->_smushAudioSampleRate > 0 && track.fadeBuf) {
			int32 maxFadeChunkSize = ((int64)dispatch.fadeSampleRate * feedSize + player->_smushAudioSampleRate - 1) / player->_smushAudioSampleRate;
			if (maxFadeChunkSize <= 0)
				maxFadeChunkSize = 1;

			int32 fadeRemaining = MIN<int32>(dispatch.fadeRemaining, maxFadeChunkSize);
			while (fadeRemaining > 0) {
				int32 fadeInFrameCount = MIN<int32>(fadeRemaining, kMaxMixChunkSize);
				const int fadeOffset = SMUSH_FADE_SIZE - dispatch.fadeRemaining;

				if (isPlayableTrack) {
					int fadeBaseVolume;
					switch (track.flags & TRK_TYPE_MASK) {
					case IS_SFX:
						fadeBaseVolume = (player->_smushTrackVols[1] * dispatch.fadeVolume) >> 7;
						break;
					case IS_BKG_MUSIC:
						fadeBaseVolume = (player->_smushTrackVols[3] * dispatch.fadeVolume) >> 7;
						break;
					case IS_SPEECH:
						fadeBaseVolume = (player->_smushTrackVols[2] * dispatch.fadeVolume) >> 7;
						break;
					default:
						fadeBaseVolume = dispatch.fadeVolume;
						break;
					}
					const int fadeVolume = CLIP<int>(dispatch.fadeRemaining * fadeBaseVolume * player->_smushTrackVols[0] / (SMUSH_FADE_SIZE * 127), 0, 127);
					queueFadeData(i, &track.fadeBuf[fadeOffset], fadeInFrameCount, fadeVolume, getTrackPan(player, i), dispatch.fadeSampleRate, soundType);
				}

				fadeRemaining -= fadeInFrameCount;
				dispatch.fadeRemaining -= fadeInFrameCount;
			}
		}

		if (track.state == TRK_STATE_FADING) {
			if (dispatch.state != TRK_STATE_PLAYING)
				dispatch.volumeStep = 16;

			stopStream(i);
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

				if (mixInFrameCount > 0) {
					if (!dispatch.dataBuf || dispatch.dataSize <= 0)
						break;

					if (dispatch.audioRemaining < 0)
						dispatch.audioRemaining = 0;

					int32 offset = dispatch.audioRemaining % dispatch.dataSize;
					int32 maxFrames = mixInFrameCount;

					if (dispatch.sampleRate > 0 && player->_smushAudioSampleRate > 0) {
						maxFrames = ((int64)dispatch.sampleRate * tmpFeedSize +
							player->_smushAudioSampleRate - 1) / player->_smushAudioSampleRate;
						if (maxFrames <= 0)
							maxFrames = 1;
						if (mixInFrameCount > maxFrames)
							mixInFrameCount = maxFrames;
					}

					if (offset + mixInFrameCount > dispatch.dataSize)
						mixInFrameCount = dispatch.dataSize - offset;

					if (dispatch.audioRemaining + mixInFrameCount <= track.availableSize) {
						if (dispatch.volumeStep < 16)
							dispatch.volumeStep++;

						if (mixInFrameCount > kMaxMixChunkSize)
							mixInFrameCount = kMaxMixChunkSize;

						track.state = TRK_STATE_PLAYING;

						if (!speechIsPlaying)
							speechIsPlaying = (track.flags & TRK_TYPE_MASK) == IS_SPEECH;
					} else {
						if (dispatch.volumeStep)
							dispatch.volumeStep--;

						track.state = TRK_STATE_ENDING;

						if (mixInFrameCount > kMaxMixChunkSize)
							mixInFrameCount = kMaxMixChunkSize;

						dispatch.audioRemaining -= mixInFrameCount;
						dispatch.currentOffset += mixInFrameCount;
						offset = dispatch.audioRemaining % dispatch.dataSize;
					}

					if (mixInFrameCount > 0) {
						if (!dispatch.dataBuf || offset < 0 || offset + mixInFrameCount > dispatch.dataSize)
							break;

						int32 consumedFeed;
						if (dispatch.sampleRate > 0 && player->_smushAudioSampleRate > 0) {
							if (mixInFrameCount == maxFrames) {
								consumedFeed = tmpFeedSize;
							} else {
								consumedFeed = ((int64)mixInFrameCount * player->_smushAudioSampleRate +
									dispatch.sampleRate - 1) / dispatch.sampleRate;
								if (consumedFeed <= 0)
									consumedFeed = 1;
							}
						} else {
							consumedFeed = mixInFrameCount;
						}

						if (isPlayableTrack)
							queueData(i, &dispatch.dataBuf[offset], mixInFrameCount,
								(mixVolume * dispatch.volumeStep) >> 4, getTrackPan(player, i),
								dispatch.sampleRate, soundType);

						dispatch.currentOffset -= mixInFrameCount;
						dispatch.audioRemaining += mixInFrameCount;
						tmpFeedSize -= consumedFeed;
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

	if (speechIsPlaying) {
		if (player->_gainReductionMultiplier > player->_gainReductionLowerBound) {
			player->_gainReductionMultiplier -= (feedSize * 2 * player->_gainReductionFactor) >> 13;
			if (player->_gainReductionMultiplier < player->_gainReductionLowerBound)
				player->_gainReductionMultiplier = player->_gainReductionLowerBound;
		}
	} else if (player->_gainReductionMultiplier < 256) {
		player->_gainReductionMultiplier += (feedSize * 2 * player->_gainReductionFactor) >> 15;
		if (player->_gainReductionMultiplier > 256)
			player->_gainReductionMultiplier = 256;
	}
}

} // End of namespace Scumm
