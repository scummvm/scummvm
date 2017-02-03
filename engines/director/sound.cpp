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

#include "audio/decoders/wave.h"
#include "common/file.h"
#include "audio/decoders/aiff.h"
#include "common/system.h"

#include "director/sound.h"

namespace Director {

DirectorSound::DirectorSound() {
	_sound1 = new Audio::SoundHandle();
	_sound2 = new Audio::SoundHandle();
	_scriptSound = new Audio::SoundHandle();
	_mixer = g_system->getMixer();

	_speaker = new Audio::PCSpeaker();
	_pcSpeakerHandle = new Audio::SoundHandle();
	_mixer->playStream(Audio::Mixer::kSFXSoundType,
		_pcSpeakerHandle, _speaker, -1, 50, 0, DisposeAfterUse::NO, true);
}

DirectorSound::~DirectorSound() {
	delete _sound1;
	delete _sound2;
	delete _scriptSound;
}

void DirectorSound::playWAV(Common::String filename, uint8 soundChannel) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		warning("Failed to open %s", filename.c_str());

		delete file;

		return;
	}

	Audio::RewindableAudioStream *sound = Audio::makeWAVStream(file, DisposeAfterUse::YES);

	if (soundChannel == 1)
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _sound1, sound);
	else
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _sound2, sound);
}

void DirectorSound::playAIFF(Common::String filename, uint8 soundChannel) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		warning("Failed to open %s", filename.c_str());
		delete file;
		return;
	}

	Audio::RewindableAudioStream *sound = Audio::makeAIFFStream(file, DisposeAfterUse::YES);

	if (soundChannel == 1)
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _sound1, sound);
	else
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _sound2, sound);
}

void DirectorSound::playMCI(Audio::AudioStream &stream, uint32 from, uint32 to) {
	Audio::SeekableAudioStream *seekStream = dynamic_cast<Audio::SeekableAudioStream *>(&stream);
	Audio::SubSeekableAudioStream *subSeekStream = new Audio::SubSeekableAudioStream(seekStream, Audio::Timestamp(from, seekStream->getRate()), Audio::Timestamp(to, seekStream->getRate()));

	_mixer->playStream(Audio::Mixer::kSFXSoundType, _scriptSound, subSeekStream);
}

bool DirectorSound::isChannelActive(uint8 channelID) {
	if (channelID == 1) {
		return _mixer->isSoundHandleActive(*_sound1);
	} else if (channelID == 2) {
		return _mixer->isSoundHandleActive(*_sound2);
	}

	error("Incorrect sound channel");

	return false;
}

void DirectorSound::stopSound() {
	_mixer->stopHandle(*_sound1);
	_mixer->stopHandle(*_sound2);
	_mixer->stopHandle(*_pcSpeakerHandle);
}

void DirectorSound::systemBeep() {
	_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 500, 150);
}

} // End of namespace Director
