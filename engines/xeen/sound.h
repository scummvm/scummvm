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

class Sound : public Music {
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
public:
	bool _soundOn;
public:
	Sound(XeenEngine *vm, Audio::Mixer *mixer);
	virtual ~Sound();

	/**
	 * Play a given sound
	 */
	void playSound(Common::SeekableReadStream &s, int unused = 0);

	/**
	 * Play a given sound
	 */
	void playSound(const Common::String &name, int unused = 0);

	/**
	 * Play a given sound
	 */
	void playSound(const Common::String &name, int ccNum, int unused);

	/**
	 * Stop playing a sound
	 * @remarks		In the original, passing 1 to playSound stopped the sound
	 */
	void stopSound();

	/**
	 * Returns true if a sound is currently playing
	 * @remarks		In the original, passing 0 to playSound returned play status
	 */
	bool isPlaying() const;

	/**
	 * Stops all playing music, FX, and sound samples
	 */
	void stopAllAudio();

	/**
	 * Sets whether sound effects is on
	 */
	void setEffectsOn(bool isOn);

	/**
	 * Called to reload sound settings
	 */
	virtual void updateSoundSettings();

};

} // End of namespace Xeen

#endif /* XEEN_SOUND_H */
