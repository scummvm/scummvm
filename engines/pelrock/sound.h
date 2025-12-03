/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PELROCK_SOUND_H
#define PELROCK_SOUND_H

#include "common/file.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "audio/mixer.h"

namespace Pelrock {

class SoundManager {
public:
    SoundManager(Audio::Mixer *mixer);
    ~SoundManager();

    void playSound(const Common::String &filename, int volume = 255);
    void stopSound();
    void stopMusic();
    void setVolume(int volume);
    bool isPlaying() const;
    void playMusicTrack(int trackNumber);
    bool isMusicPlaying() const {
        return _isMusicPlaying;
    }

private:
    Audio::Mixer *_mixer;
    Audio::SoundHandle _soundHandle;
    Audio::SoundHandle _musicHandle;
    bool _isMusicPlaying = false;
    int _currentVolume;
    Common::File *_musicFile;
    byte _currentMusicTrack = 0;
    Audio::SoundHandle _sfxHandles[8];
};

} // End of namespace Pelrock

#endif // PELROCK_SOUND_H
