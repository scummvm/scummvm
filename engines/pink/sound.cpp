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

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "common/substream.h"

#include "pink/sound.h"

namespace Pink {

void Sound::play(Common::SafeSeekableSubReadStream *stream, Audio::Mixer::SoundType type, byte volume, int8 balance, bool isLoop) {
	// Vox files in pink have wave format.
	// RIFF (little-endian) data, WAVE audio, Microsoft PCM, 8 bit, mono 22050 Hz

	volume = ((int)volume * 255) / 100;
	Audio::Mixer *mixer = g_system->getMixer();
	mixer->stopHandle(_handle);

	Audio::AudioStream *audioStream;
	Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (isLoop)
		audioStream = Audio::makeLoopingAudioStream(wavStream, 0, 0, 0);
	else
		audioStream = wavStream;

	mixer->playStream(type, &_handle , audioStream, -1, volume, balance, DisposeAfterUse::YES);
}

} // End of namespace Pink
