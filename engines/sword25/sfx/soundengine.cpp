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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#define BS_LOG_PREFIX "SOUNDENGINE"

#include "sword25/sfx/soundengine.h"
#include "sword25/package/packagemanager.h"

#include "sound/decoders/vorbis.h"

namespace Sword25 {

SoundEngine::SoundEngine(Kernel *pKernel) : ResourceService(pKernel) {
	if (!_RegisterScriptBindings())
		BS_LOG_ERRORLN("Script bindings could not be registered.");
	else
		BS_LOGLN("Script bindings registered.");

	_mixer = g_system->getMixer();

	for (int i = 0; i < SOUND_HANDLES; i++)
		_handles[i].type = kFreeHandle;
}

Service *SoundEngine_CreateObject(Kernel *pKernel) {
	return new SoundEngine(pKernel);
}

bool SoundEngine::Init(uint SampleRate, uint Channels) {
	return true;
}

void SoundEngine::Update() {
}

void SoundEngine::SetVolume(float Volume, SOUND_TYPES Type) {
}

float SoundEngine::GetVolume(SOUND_TYPES Type) {
	return 0;
}

void SoundEngine::PauseAll() {
}

void SoundEngine::ResumeAll() {
}

void SoundEngine::PauseLayer(uint Layer) {
}

void SoundEngine::ResumeLayer(uint Layer) {
}

SndHandle *SoundEngine::getHandle(uint *id) {
	for (uint i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type != kFreeHandle && !_mixer->isSoundHandleActive(_handles[i].handle)) {
			debug(5, "Handle %d has finished playing", i);
			_handles[i].type = kFreeHandle;
		}
	}

	for (uint i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kFreeHandle) {
			debug(5, "Allocated handle %d", i);
			if (id)
				*id = i;
			return &_handles[i];
		}
	}

	error("Sound::getHandle(): Too many sound handles");

	return NULL;
}

Audio::Mixer::SoundType getType(SoundEngine::SOUND_TYPES type) {
	switch (type) {
	case SoundEngine::MUSIC:
		return Audio::Mixer::kMusicSoundType;
	case SoundEngine::SPEECH:
		return Audio::Mixer::kSpeechSoundType;
	case SoundEngine::SFX:
		return Audio::Mixer::kSFXSoundType;
	default:
		error("Unknown SOUND_TYPE");
	}

	return Audio::Mixer::kPlainSoundType;
}

bool SoundEngine::PlaySound(const Common::String &fileName, SOUND_TYPES type, float volume, float pan, bool loop, int loopStart, int loopEnd, uint layer) {
	//PlaySoundEx(fileName, type, volume, pan, loop, loopStart, loopEnd, layer);

	return true;
}

uint SoundEngine::PlaySoundEx(const Common::String &fileName, SOUND_TYPES type, float volume, float pan, bool loop, int loopStart, int loopEnd, uint layer) {
	return true;

	Common::SeekableReadStream *in = Kernel::GetInstance()->GetPackage()->GetStream(fileName);
	Audio::SeekableAudioStream *stream = Audio::makeVorbisStream(in, DisposeAfterUse::YES);
	uint id;
	SndHandle *handle = getHandle(&id);

	_mixer->playStream(getType(type), &(handle->handle), stream, -1, (byte)(volume * 255), (int8)(pan * 127));

	return id;
}

void SoundEngine::SetSoundVolume(uint handle, float volume) {
	assert(handle < SOUND_HANDLES);

	_mixer->setChannelVolume(_handles[handle].handle, (byte)(volume * 255));
}

void SoundEngine::SetSoundPanning(uint handle, float pan) {
	assert(handle < SOUND_HANDLES);

	_mixer->setChannelBalance(_handles[handle].handle, (int8)(pan * 127));
}

void SoundEngine::PauseSound(uint handle) {
	assert(handle < SOUND_HANDLES);

	_mixer->pauseHandle(_handles[handle].handle, true);
}

void SoundEngine::ResumeSound(uint handle) {
	assert(handle < SOUND_HANDLES);

	_mixer->pauseHandle(_handles[handle].handle, false);
}

void SoundEngine::StopSound(uint handle) {
	assert(handle < SOUND_HANDLES);

	_mixer->stopHandle(_handles[handle].handle);
}

bool SoundEngine::IsSoundPaused(uint handle) {
	return false;
}

bool SoundEngine::IsSoundPlaying(uint handle) {
	return false;
}

float SoundEngine::GetSoundVolume(uint handle) {
	return 0;
}

float SoundEngine::GetSoundPanning(uint handle) {
	return 0;
}

float SoundEngine::GetSoundTime(uint handle) {
	return 0;
}

Resource *SoundEngine::LoadResource(const Common::String &FileName) {
	return 0;
}

bool SoundEngine::CanLoadResource(const Common::String &fileName) {
	Common::String fname = fileName;

	fname.toLowercase();

	return fname.hasSuffix(".ogg");
}


bool SoundEngine::persist(OutputPersistenceBlock &writer) {
	warning("STUB: SoundEngine::persist()");

	return true;
}

bool SoundEngine::unpersist(InputPersistenceBlock &reader) {
	warning("STUB: SoundEngine::unpersist()");

	return true;
}


} // End of namespace Sword25
