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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NANCY_AUDIO_H
#define NANCY_AUDIO_H

#include "common/types.h"
#include "common/str.h"

#include "audio/mixer.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {
class SeekableAudioStream;
}

namespace Nancy {

class NancyEngine;

Audio::SeekableAudioStream *makeHISStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);

class SoundManager {
public:
    enum SoundLoopType { kLoop = 0, kOneShot = 1 };
    SoundManager(NancyEngine *engine);
    ~SoundManager() =default;

    void loadSound(Common::String &name, int16 id, uint16 numLoops = 0, uint16 volume = 60);
    void stopSound(int16 id);
    bool isSoundPlaying(int16 id);
    void stopAllSounds();

private:
    NancyEngine *_engine;
    Audio::Mixer *_mixer;

    Audio::SoundHandle handles[20];
    Common::String names[20];
};

} // End of namespace Nancy

#endif // NANCY_AUDIO_H