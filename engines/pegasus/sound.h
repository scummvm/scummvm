/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_SOUND_H
#define PEGASUS_SOUND_H

#include "audio/mixer.h"
#include "common/str.h"

namespace Audio {
	class AudioStream;
	class RewindableAudioStream;
}

namespace Pegasus {

// TODO!
//class MMSoundFader;

//	Things you might want to do with sound:
//		Start it
//		Stop it
//		Loop it
//		Pause it
//		Set the volume
//		Set the pitch (rate)
//		Pan the sound
//		Change these settings dynamically over time

class Sound {
public:
	Sound();
	~Sound();

	// We only have one access point here because we should
	// only be opening an AIFF file from a file name. We're
	// not using the resource fork string resources.
	void initFromAIFFFile(const Common::String &fileName);

	void disposeSound();
	bool isSoundLoaded() const;
	void playSound();
	void loopSound();
	void stopSound();
	void setVolume(const uint16 volume);
	bool isPlaying();

	// TODO!
	//void attachFader(SoundFader *fader);

protected:
	Audio::RewindableAudioStream *_aiffStream;
	Audio::SoundHandle _handle;
	byte _volume;

	// TODO!
	//SoundFader *_fader;
};

} // End of namespace Pegasus

#endif
