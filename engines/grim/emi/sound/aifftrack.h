/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
	AIFFTrack(Audio::Mixer::SoundType soundType);
	~AIFFTrack();
	bool openSound(const Common::String &filename, const Common::String &soundName, const Audio::Timestamp *start = nullptr) override;
	bool isPlaying() override;
	bool isStreamOpen() { return _stream != NULL; }
	void setLooping(bool looping) override;
	bool isLooping() const override { return _looping; }
	bool play() override;
	Audio::Timestamp getPos() override;
private:
	bool _looping;
};

}
#endif
