/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios
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

#ifndef PEGASUS_MMSHELL_SOUNDS_MMSOUND_H
#define PEGASUS_MMSHELL_SOUNDS_MMSOUND_H

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

class MMSound {
public:
	MMSound();
	~MMSound();

	// We only have one access point here because we should
	// only be opening an AIFF file from a file name. We're
	// not using the resource fork string resources.
	void InitFromAIFFFile(const Common::String &fileName);

	void DisposeSound();
	bool SoundLoaded() const;
	void PlaySound();
	void LoopSound();
	void StopSound();
	void SetVolume(const uint16 volume);
	bool IsPlaying();

	// TODO!
	//void AttachFader(MMSoundFader*);

protected:
	Audio::RewindableAudioStream *_aiffStream;
	Audio::SoundHandle _handle;
	byte _volume;

	// TODO!
	//MMSoundFader *fTheFader;
};

} // End of namespace Pegasus

#endif
