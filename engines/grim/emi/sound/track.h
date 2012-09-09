/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_SOUNDTRACK_H
#define GRIM_SOUNDTRACK_H

#include "audio/mixer.h"

namespace Common {
	class String;
}

namespace Audio {
	class AudioStream;
	class SoundHandle;
}

namespace Grim {

/**
 * @class Super-class for the different codecs used in EMI
 */
class SoundTrack {
protected:
	Common::String _soundName;
	Audio::AudioStream *_stream;
	Audio::SoundHandle *_handle;
	Audio::Mixer::SoundType _soundType;
	DisposeAfterUse::Flag _disposeAfterPlaying;
public:
	SoundTrack();
	virtual ~SoundTrack();
	virtual bool openSound(Common::String voiceName, Common::SeekableReadStream *file) = 0;
	virtual bool isPlaying() = 0;
	virtual bool play();
	virtual void stop();
	Audio::SoundHandle *getHandle() { return _handle; }
	Common::String getSoundName();
	void setSoundName(Common::String);
};

}

#endif
