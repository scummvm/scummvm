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

#ifndef NANCY_SOUND_H
#define NANCY_SOUND_H

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

class SoundManager {
public:
    // Combines four different structs in one
    struct SoundDescription {
        enum Type { kNormal, kMenu, kDIGI, kScene };

        Common::String name;
        uint16 channelID;
        uint16 numLoops;
        uint16 volume;

        void read(Common::SeekableReadStream &stream, Type type);
    };

    SoundManager(NancyEngine *engine);
    ~SoundManager();

    // Load a sound into a channel without starting it
    uint16 loadSound(const SoundDescription &description);

    void playSound(uint16 channelID);
    void pauseSound(uint16 channelID, bool pause);
    bool isSoundPlaying(uint16 channelID);

    // Stop playing a sound and unload it from the channel
    void stopSound(uint16 channelID);
    void stopAllSounds();

    static Audio::SeekableAudioStream *makeHISStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);

protected:
    struct Channel {
        Common::String name;
        Audio::Mixer::SoundType type;
        uint16 numLoops = 0;
        uint volume = 0;
        Audio::SeekableAudioStream *stream = nullptr;
        Audio::SoundHandle handle;
    };

    void initSoundChannels();
    NancyEngine *_engine;
    Audio::Mixer *_mixer;

    Channel _channels[32];
};

} // End of namespace Nancy

#endif // NANCY_SOUND_H
