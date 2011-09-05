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

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/quicktime.h"
#include "common/file.h"
#include "common/system.h"

#include "pegasus/sound.h"

namespace Pegasus {

Sound::Sound() {
	_stream = 0;
	_volume = 0xFF;
}

Sound::~Sound() {
	disposeSound();
}

void Sound::disposeSound() {
	stopSound();
	delete _stream; _stream = 0;
}

void Sound::initFromAIFFFile(const Common::String &fileName) {
	disposeSound();

	Common::File *file = new Common::File();
	if (!file->open(fileName)) {
		delete file;
		return;
	}

	_stream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);
}

void Sound::initFromQuickTime(const Common::String &fileName) {
	disposeSound();

	_stream = Audio::makeQuickTimeStream(fileName);
}

#if 0
// TODO!
void Sound::attachFader(SoundFader *fader) {
	if (_fader)
		_fader->attachSound(NULL);

	_fader = fader;

	if (_fader)
		_fader->attachSound(this);
}
#endif

void Sound::playSound() {
	if (!isSoundLoaded())
		return;

	stopSound();

#if 0
	// TODO!
	if (_fader)
		setVolume(_fader->getFaderValue());
#endif

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, _stream, -1, _volume, 0, DisposeAfterUse::NO);
}

void Sound::loopSound() {
	if (!isSoundLoaded())
		return;

	stopSound();

	// Create a looping stream
	Audio::AudioStream *loopStream = new Audio::LoopingAudioStream(_stream, 0, DisposeAfterUse::NO);

#if 0
	// TODO!
	// Assume that if there is a fader, we're going to fade the sound in.
	if (_fader)
		setVolume(0);
#endif

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopStream, -1, _volume, 0, DisposeAfterUse::YES);
}

void Sound::playSoundSegment(uint32 start, uint32 end) {
	if (!isSoundLoaded())
		return;

	stopSound();

	Audio::AudioStream *subStream = new Audio::SubSeekableAudioStream(_stream, Audio::Timestamp(0, start, 600), Audio::Timestamp(0, end, 600), DisposeAfterUse::NO);

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, subStream, -1, _volume, 0, DisposeAfterUse::YES);
}

void Sound::loopSoundSegment(uint32 start, uint32 end) {
	if (!isSoundLoaded())
		return;

	stopSound();

	Audio::AudioStream *subLoopStream = new Audio::SubLoopingAudioStream(_stream, 0, Audio::Timestamp(0, start, 600), Audio::Timestamp(0, end, 600), DisposeAfterUse::NO);

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, subLoopStream, -1, _volume, 0, DisposeAfterUse::YES);
}

void Sound::stopSound() {
	g_system->getMixer()->stopHandle(_handle);
}

void Sound::setVolume(const uint16 volume) {
	// Clipping the volume to [0x00, 0xFF] instead of Apple's [0, 0x100]
	// We store the volume in case SetVolume is called before the sound starts

	_volume = (volume == 0x100) ? 0xFF : volume;
	g_system->getMixer()->setChannelVolume(_handle, _volume);
}

bool Sound::isPlaying() {
	return isSoundLoaded() && g_system->getMixer()->isSoundHandleActive(_handle);
}

bool Sound::isSoundLoaded() const {
	return _stream != 0;
}

} // End of namespace Pegasus
