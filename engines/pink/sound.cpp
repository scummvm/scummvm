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

#include <audio/audiostream.h>
#include <audio/decoders/wave.h>
#include <audio/decoders/adpcm.h>
#include <common/substream.h>
#include "sound.h"

namespace Pink {

Sound::Sound(Audio::Mixer *mixer, Common::SafeSeekableSubReadStream *stream)
    : _mixer(mixer), _fileStream(stream)
{

}

Sound::~Sound() {
    stop();
    delete _fileStream;
}

bool Sound::isPlaying() {
    return _mixer->isSoundHandleActive(_handle);
}

void Sound::pause() {
    _mixer->pauseHandle(_handle, true);
}

void Sound::resume() {
    _mixer->pauseHandle(_handle, false);
}

void Sound::stop() {
    _mixer->stopHandle(_handle);
}

void Sound::play(Audio::Mixer::SoundType type, int volume, bool isLoop) {
    // Vox files in pink have wave format.
    // RIFF (little-endian) data, WAVE audio, Microsoft PCM, 8 bit, mono 22050 Hz
    _mixer->stopHandle(_handle);

    _fileStream->seek(0);
    Audio::AudioStream *audioStream ;
    Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(_fileStream, DisposeAfterUse::NO);
    if (isLoop) {
        audioStream = Audio::makeLoopingAudioStream(wavStream, 0, 0, 0);
    }
    else audioStream = wavStream;

    _mixer->playStream(type, &_handle , audioStream, -1 , Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
}

void Sound::setBalance(int8 balance) {
    _mixer->setChannelBalance(_handle, balance);
}

uint32 Sound::getCurrentSample() {
    return _mixer->getSoundElapsedTime(_handle) * 22050 / 1000;
}

} // End of namespace Pink