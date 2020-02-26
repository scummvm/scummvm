/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_AUDIO_MIXER_H
#define BLADERUNNER_AUDIO_MIXER_H

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/mutex.h"

namespace BladeRunner {

class BladeRunnerEngine;

class AudioMixer {
#if BLADERUNNER_ORIGINAL_BUGS
	static const int kChannels         = 9;
	static const int kUsableChannels   = 8;
	static const int kMusicChannel     = 8;
#else
	static const int kChannels         = 15;
	static const int kUsableChannels   = 14;
	static const int kMusicChannel     = 14;
#endif // BLADERUNNER_ORIGINAL_BUGS
	static const int kUpdatesPerSecond = 40;

	struct Channel {
		bool                isPresent;
		int                 priority;
		bool                loop;
		Audio::SoundHandle  handle;
		Audio::AudioStream *stream;
		float               volume;
		float               volumeDelta;
		float               volumeTarget;
		float               pan;
		float               panDelta;
		float               panTarget;
		void              (*endCallback)(int channel, void *data);
		void               *callbackData;
		uint32              timeStarted;
		uint32              trackDurationMs;
		bool                sentToMixer;
	};

	BladeRunnerEngine *_vm;

	Channel       _channels[kChannels];
	Common::Mutex _mutex;

public:
	AudioMixer(BladeRunnerEngine *vm);
	~AudioMixer();

	int play(Audio::Mixer::SoundType type, Audio::RewindableAudioStream *stream, int priority, bool loop, int volume, int pan, void(*endCallback)(int, void *), void *callbackData, uint32 trackDurationMs);
	int playMusic(Audio::RewindableAudioStream *stream, int volume, void(*endCallback)(int, void *), void *callbackData, uint32 trackDurationMs);
	void stop(int channel, uint32 delay);

	void adjustVolume(int channel, int newVolume, uint32 time);
	void adjustPan(int channel, int newPan, uint32 time);

	void resume(int channel, uint32 delay);
	void pause(int channel, uint32 delay);

private:
	int playInChannel(int channel, Audio::Mixer::SoundType type, Audio::RewindableAudioStream *stream, int priority, bool loop, int volume, int pan, void(*endCallback)(int, void *), void *callbackData, uint32 trackDurationMs);

	bool isActive(int channel) const;
	void tick();
	static void timerCallback(void *refCon);
};

} // End of namespace BladeRunner

#endif
