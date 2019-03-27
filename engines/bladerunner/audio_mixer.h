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
	static const int kChannels         = 15; // original was 9;
	static const int kUsableChannels   = 14; // original was 8;
	static const int kMusicChannel     = 14; // original was 8;
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
	};

	BladeRunnerEngine *_vm;

	Channel       _channels[kChannels];
	Common::Mutex _mutex;

public:
	AudioMixer(BladeRunnerEngine *vm);
	~AudioMixer();

	int play(Audio::Mixer::SoundType type, Audio::RewindableAudioStream *stream, int priority, bool loop, int volume, int pan, void(*endCallback)(int, void *), void *callbackData);
	int playMusic(Audio::RewindableAudioStream *stream, int volume, void(*endCallback)(int, void *), void *callbackData);
	void stop(int channel, int delay);

	void adjustVolume(int channel, int newVolume, int time);
	void adjustPan(int channel, int newPan, int time);

	void resume(int channel, int delay);
	void pause(int channel, int delay);

private:
	int playInChannel(int channel, Audio::Mixer::SoundType type, Audio::RewindableAudioStream *stream, int priority, bool loop, int volume, int pan, void(*endCallback)(int, void *), void *callbackData);

	bool isActive(int channel) const;
	void tick();
	static void timerCallback(void *refCon);
};

} // End of namespace BladeRunner

#endif
