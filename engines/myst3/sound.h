/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef SOUND_H_
#define SOUND_H_

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/str.h"

namespace Myst3 {

class Myst3Engine;

enum SoundType {
	kAny,
	kAmbient,
	kCue,
	kEffect,
	kMusic
};

enum SoundNextCommand {
	kRandom,
	kNext,
	kRandomIfOtherStarting,
	kNextIfOtherStarting
};

class SoundChannel {
public:
	SoundChannel(Myst3Engine *vm);
	virtual ~SoundChannel();

	void play(uint32 id, uint32 volume, uint16 heading, uint16 attenuation, bool loop, SoundType type);
	void setVolume3D(uint32 volume, uint16 heading, uint16 attenuation);
	void fade(uint32 targetVolume, int32 targetHeading, int32 targetAttenuation, uint32 fadeDelay);
	void fadeOut(uint32 fadeDelay);
	void update();
	void stop();
	void age(uint32 maxAge);
	uint32 playedFrames();
	uint32 adjustVolume(uint32 volume);

	uint32 _id;
	bool _playing;
	bool _stopWhenSilent;
	bool _fading;
	SoundType _type;
	uint32 _age;
	uint32 _ambientFadeOutDelay;

	uint _fadeLastTick;
	int32 _fadeDuration; // In frames (@30 fps)
	int32 _fadePosition;

	int32 _fadeSourceVolume;
	int32 _fadeTargetVolume;
	int32 _fadeSourceHeading;
	int32 _fadeTargetHeading;
	int32 _fadeSourceAttenuation;
	int32 _fadeTargetAttenuation;

	bool _hasFadeArray;
	uint32 _fadeArrayPosition;
	uint32 _fadeDurations[4];
	uint32 _fadeVolumes[4];

private:
	Myst3Engine *_vm;

	Common::String _name;

	uint32 _volume;
	int32 _heading;
	uint32 _headingAngle;

	Audio::AudioStream *_stream;
	Audio::SoundHandle _handle;
	Audio::Timestamp _length;

	Audio::RewindableAudioStream *makeAudioStream(const Common::String &name) const;
	void updateFading();
	Audio::Mixer::SoundType mixerSoundType();
};

class Sound {
public:
	Sound(Myst3Engine *vm);
	virtual ~Sound();

	SoundChannel *getChannelForSound(uint32 id, SoundType type, bool *found = nullptr);

	void playEffect(uint32 id, uint32 volume, uint16 heading = 0, uint16 attenuation = 0);
	void playEffectLooping(uint32 id, uint32 volume, uint16 heading = 0, uint16 attenuation = 0);
	void playEffectFadeInOut(uint32 id, uint32 volume, uint16 heading, uint16 attenuation,
			uint32 fadeInDuration, uint32 playDuration, uint32 fadeOutDuration);
	void stopEffect(uint32 id, uint32 fadeDuration);

	void playCue(uint32 id, uint32 volume, uint16 heading, uint16 attenuation);
	void stopCue(uint32 fadeDelay);

	void stopMusic(uint32 fadeDelay);

	bool isPlaying(uint32 id);
	int32 playedFrames(uint32 id);

	void update();
	void age();

	void fadeOutOldSounds(uint32 fadeDelay);

	void computeVolumeBalance(int32 volume, int32 heading, uint attenuation, int32 *mixerVolume, int32 *balance);

	void setupNextSound(SoundNextCommand command, int16 controlVar, int16 startSoundId, int16 soundCount,
	                    int32 soundMinDelay, int32 soundMaxDelay, int32 controlSoundId = 0, int32 controlSoundMaxPosition = 0);
	void resetSoundVars();
private:
	static const uint kNumChannels = 14;

	Myst3Engine *_vm;
	SoundChannel *_channels[kNumChannels];

	void compute3DVolumes(int32 heading, uint angle, int32 *left, int32 *right);
};

} // End of namespace Myst3

#endif // SOUND_H_
