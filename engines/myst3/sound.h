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
	kUnk1,
	kUnk2,
	kEffect,
	kMusic
};

class SoundChannel {
public:
	SoundChannel(Myst3Engine *vm);
	virtual ~SoundChannel();

	void play(uint32 id, uint32 volume, uint16 heading, uint16 attenuation, uint unk1, uint unk2, uint unk3, SoundType type);
	void update();

	uint32 _id;
	bool _playing;
	SoundType _type;

private:
	Myst3Engine *_vm;

	Common::String _name;
	uint32 _volume;
	Audio::RewindableAudioStream *_stream;
	Audio::SoundHandle _handle;

	Audio::RewindableAudioStream *makeAudioStream(const Common::String &name) const;
};

class Sound {
public:
	Sound(Myst3Engine *vm);
	virtual ~Sound();

	void playEffect(uint32 id, uint32 volume, uint16 heading = 0, uint16 attenuation = 0);
	void update();

private:
	static const uint kNumChannels = 13;

	Myst3Engine *_vm;
	SoundChannel *_channels[kNumChannels];

	SoundChannel *getChannelForSound(uint32 id, SoundType priority);
};

} /* namespace Myst3 */
#endif /* SOUND_H_ */
