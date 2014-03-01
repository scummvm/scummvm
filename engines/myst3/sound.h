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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "common/str.h"

namespace Myst3 {

class Myst3Engine;

enum SoundType {
	kUnk0,
	kAmbient,
	kCue,
	kEffect,
	kMusic
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

	uint32 _id;
	bool _playing;
	SoundType _type;
	uint32 _age;
	uint32 _ambientFadeOutDelay;

private:
	Myst3Engine *_vm;

	Common::String _name;
	uint32 _volume;
	Audio::AudioStream *_stream;
	Audio::SoundHandle _handle;

	Audio::RewindableAudioStream *makeAudioStream(const Common::String &name) const;
};

class Sound {
public:
	Sound(Myst3Engine *vm);
	virtual ~Sound();

	SoundChannel *getChannelForSound(uint32 id, SoundType type, bool *found = nullptr);

	void playEffect(uint32 id, uint32 volume, uint16 heading = 0, uint16 attenuation = 0);

	void playCue(uint32 id, uint32 volume, uint16 heading, uint16 attenuation);
	void stopCue(uint32 fadeDelay);

	void update();
	void age();

	void fadeOutOldSounds(uint32 fadeDelay);

private:
	static const uint kNumChannels = 14;

	Myst3Engine *_vm;
	SoundChannel *_channels[kNumChannels];

};

} /* namespace Myst3 */
#endif /* SOUND_H_ */
