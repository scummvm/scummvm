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

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "common/file.h"
#include "common/system.h"

#include "engines/pegasus/MMShell/Sounds/MMSound.h"

namespace Pegasus {

MMSound::MMSound() {
	_aiffStream = 0;
	_volume = 0xFF;
}

MMSound::~MMSound() {
	DisposeSound();
}

void MMSound::DisposeSound() {
	StopSound();
	delete _aiffStream; _aiffStream = 0;
}

void MMSound::InitFromAIFFFile(const Common::String &fileName) {
	Common::File *file = new Common::File();
	if (!file->open(fileName)) {
		delete file;
		return;
	}

	_aiffStream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);
}

#if 0
// TODO!
void MMSound::AttachFader(MMSoundFader *theFader) {
	if (fTheFader)
		fTheFader->AttachSound(NULL);

	fTheFader = theFader;

	if (fTheFader)
		fTheFader->AttachSound(this);
}
#endif

void MMSound::PlaySound() {
	if (!SoundLoaded())
		return;

	StopSound();

#if 0
	// TODO!
	if (fTheFader)
		this->SetVolume(fTheFader->GetFaderValue());
#endif

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, _aiffStream, -1, _volume, 0, DisposeAfterUse::NO);
}

void MMSound::LoopSound() {
	if (!SoundLoaded())
		return;

	StopSound();

	// Create a looping stream
	Audio::AudioStream *loopStream = new Audio::LoopingAudioStream(_aiffStream, 0, DisposeAfterUse::NO);

#if 0
	// TODO!
	// Assume that if there is a fader, we're going to fade the sound in.
	if (fTheFader)
		this->SetVolume(0);
#endif

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopStream, -1, _volume, 0, DisposeAfterUse::YES);
}

void MMSound::StopSound(void) {
	g_system->getMixer()->stopHandle(_handle);
}

void MMSound::SetVolume(const uint16 volume) {
	// Clipping the volume to [0x00, 0xFF] instead of Apple's [0, 0x100]
	// We store the volume in case SetVolume is called before the sound starts

	_volume = (volume == 0x100) ? 0xFF : volume;
	g_system->getMixer()->setChannelVolume(_handle, _volume);
}

bool MMSound::IsPlaying() {
	return SoundLoaded() && g_system->getMixer()->isSoundHandleActive(_handle);
}

bool MMSound::SoundLoaded() const {
	return _aiffStream != 0;
}

} // End of namespace Pegasus
