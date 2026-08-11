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

#ifndef SCUMM_INSANE_REBEL_AUDIO_H
#define SCUMM_INSANE_REBEL_AUDIO_H

#include "audio/mixer.h"
#include "common/scummsys.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Scumm {

class ScummEngine_v7;
class SmushPlayer;

class RebelAudio {
public:
	RebelAudio();

	void init(ScummEngine_v7 *vm, int sampleRate);
	void reset();
	void terminate();
	int sampleRate() const { return _sampleRate; }

	void queueData(int trackIdx, const uint8 *data, int32 size, int volume, int pan, int sampleRate = 0);
	void processFrame(SmushPlayer *player, int16 feedSize);
	void setGroupVolume(SmushPlayer *player, int groupId, int volume);
	void setGroupPan(SmushPlayer *player, int groupId, int pan);

private:
	static const int kMaxTracks = 4;

	bool processAudioCodes(SmushPlayer *player, int idx, int32 &tmpFeedSize, int &mixVolume);
	int computeMixVolume(SmushPlayer *player, int idx) const;
	Audio::Mixer::SoundType getSoundType(int flags) const;
	int getTrackPan(SmushPlayer *player, int idx) const;
	void updateTrackMixerState(SmushPlayer *player, int idx);
	void stopStream(int trackIdx);
	void stopFadeStream(int trackIdx);
	void queueData(int trackIdx, const uint8 *data, int32 size, int volume, int pan, int sampleRate, Audio::Mixer::SoundType soundType);
	void queueFadeData(int trackIdx, const uint8 *data, int32 size, int volume, int pan, int sampleRate, Audio::Mixer::SoundType soundType);

	ScummEngine_v7 *_vm;
	Audio::QueuingAudioStream *_streams[kMaxTracks];
	Audio::QueuingAudioStream *_fadeStreams[kMaxTracks];
	Audio::SoundHandle _handles[kMaxTracks];
	Audio::SoundHandle _fadeHandles[kMaxTracks];
	bool _trackActive[kMaxTracks];
	bool _fadeTrackActive[kMaxTracks];
	Audio::Mixer::SoundType _streamTypes[kMaxTracks];
	Audio::Mixer::SoundType _fadeStreamTypes[kMaxTracks];
	int _sampleRate;
};

} // End of namespace Scumm

#endif
