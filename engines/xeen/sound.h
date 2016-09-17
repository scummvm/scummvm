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

#ifndef XEEN_SOUND_H
#define XEEN_SOUND_H

#include "audio/mixer.h"
#include "audio/audiostream.h"
//#include "common/scummsys.h"
//#include "common/system.h"
#include "xeen/files.h"
#include "xeen/music.h"

namespace Xeen {

class Sound;

class Voc: public Common::File {
private:
	static Sound *_sound;
	Audio::SoundHandle _soundHandle;
public:
	Voc() {}
	Voc(const Common::String &name);
	virtual ~Voc() { stop(); }
	static void init(XeenEngine *vm);

	/**
	 * Start playing the sound
	 */
	void play();

	/**
	 * Stop playing the sound
	 */
	void stop();
};

class Sound : public Music {
private:
	Audio::Mixer *_mixer;
public:
	Sound(XeenEngine *vm, Audio::Mixer *mixer);

	void proc2(Common::SeekableReadStream &f);

	/**
	 * Play a given sound
	 */
	void playSound(Common::SeekableReadStream *s, Audio::SoundHandle &soundHandle,
		Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType);

	/**
	 * Stop playing a sound
	 */
	void stopSound(Audio::SoundHandle &soundHandle);

	void playSample(const Common::SeekableReadStream *stream, int v2 = 1) {}

	bool playSample(int v1, int v2) { return false; }
};

} // End of namespace Xeen

#endif /* XEEN_SOUND_H */
