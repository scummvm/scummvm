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

namespace Sword25 {

SoundEngine::SoundEngine(Kernel *pKernel) : ResourceService(pKernel) {
	if (!_RegisterScriptBindings())
		BS_LOG_ERRORLN("Script bindings could not be registered.");
	else
		BS_LOGLN("Script bindings registered.");

	_mixer = g_system->getMixer();
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

bool SoundEngine::PlaySound(const Common::String &FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, uint Layer) {
	return true;
}

uint SoundEngine::PlaySoundEx(const Common::String &FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, uint Layer) {
	return true;
}

void SoundEngine::SetSoundVolume(uint Handle, float Volume) {
}

void SoundEngine::SetSoundPanning(uint Handle, float Pan) {
}

void SoundEngine::PauseSound(uint Handle) {
}

void SoundEngine::ResumeSound(uint Handle) {
}

void SoundEngine::StopSound(uint Handle) {
}

bool SoundEngine::IsSoundPaused(uint Handle) {
	return false;
}

bool SoundEngine::IsSoundPlaying(uint Handle) {
	return false;
}

float SoundEngine::GetSoundVolume(uint Handle) {
	return 0;
}

float SoundEngine::GetSoundPanning(uint Handle) {
	return 0;
}

float SoundEngine::GetSoundTime(uint Handle) {
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
