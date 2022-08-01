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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#include "hpl1/engine/impl/OpenALSoundData.h"
#include "audio/mixer.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/types.h"
#include "hpl1/debug.h"
#include "hpl1/engine/impl/OpenALSoundChannel.h"
#include "hpl1/engine/system/low_level_system.h"
#include "audio/decoders/vorbis.h"
namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cOpenALSoundData::cOpenALSoundData(tString asName, bool abStream) : iSoundData(asName, abStream), _audioStream(nullptr) {
}

//-----------------------------------------------------------------------

cOpenALSoundData::~cOpenALSoundData() {
	delete _audioStream;
	_soundFile.close();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cOpenALSoundData::CreateFromFile(const tString &filename) {
	Hpl1::logInfo(Hpl1::kDebugResourceLoading | Hpl1::kDebugAudio,
		"loading audio file %s\n", filename.c_str());
	// FIXME: string types
	if (_audioStream)
		error("trying to load a sample"); // FIXME: remove this if its not needed
	if (!_soundFile.open(filename.c_str())) {
		Hpl1::logError(Hpl1::kDebugAudio,
			"audio file %s could not be loaded", filename.c_str());
		return false;
	}
	_audioStream = Audio::makeVorbisStream(&_soundFile, DisposeAfterUse::NO);
	return static_cast<bool>(_audioStream);
}

//-----------------------------------------------------------------------

iSoundChannel *cOpenALSoundData::CreateChannel(int alPriority) {
	if (!_audioStream)
		return nullptr;

	IncUserCount();
	return hplNew(cOpenALSoundChannel, (this, mpSoundManger));
}

void cOpenALSoundData::start(Audio::SoundHandle *handle) {
	_audioStream->rewind();
	g_system->getMixer()->playStream(Audio::Mixer::SoundType::kPlainSoundType, handle, _audioStream,
		-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

} // namespace hpl
