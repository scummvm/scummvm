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

#ifndef GRIM_AIFFTRACK_H
#define GRIM_AIFFTRACK_H

#include "common/str.h"
#include "common/stream.h"
#include "engines/grim/emi/sound/track.h"

namespace Audio {
	class AudioStream;
	class SoundHandle;
}

namespace Grim {

class AIFFTrack : public SoundTrack {
public:
	AIFFTrack(Audio::Mixer::SoundType soundType, DisposeAfterUse::Flag disposeOfStream = DisposeAfterUse::YES);
	~AIFFTrack();
	bool openSound(Common::String soundName, Common::SeekableReadStream *file);
	bool isPlaying() { return true; }
	void setLooping(bool looping);
	bool play();
private:
	bool _looping;
};

}
#endif
