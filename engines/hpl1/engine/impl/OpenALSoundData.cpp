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
#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#include "common/memstream.h"
#include "hpl1/debug.h"
#include "hpl1/engine/impl/OpenALSoundChannel.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

enum DataFormat {
	kWav,
	kOgg,
	kNone,
};

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cOpenALSoundData::cOpenALSoundData(tString asName, bool abStream, cLowLevelSoundOpenAL *lowLevelSound)
	: iSoundData(asName, abStream), _lowLevelSound(lowLevelSound) {
}

//-----------------------------------------------------------------------

cOpenALSoundData::~cOpenALSoundData() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

static uint audioDataFormat(const tString &filename) {
	if (filename.hasSuffix("wav"))
		return kWav;
	else if (filename.hasSuffix("ogg"))
		return kOgg;
	return kNone;
}

bool cOpenALSoundData::CreateFromFile(const tString &filename) {
	if (_audioData) {
		Hpl1::logWarning(Hpl1::kDebugAudio, "overriding previous sound data with new audio at '%s'\n", filename.c_str());
	}
	Common::File file;
	if (!file.open(filename)) {
		Hpl1::logWarning(Hpl1::kDebugFilePath | Hpl1::kDebugResourceLoading | Hpl1::kDebugAudio, "Audio file '%s' could not be opened\n", filename.c_str());
		return false;
	}
	if (file.err() || file.size() < 0) {
		Hpl1::logError(Hpl1::kDebugResourceLoading | Hpl1::kDebugAudio, "error reading file '%s'\n", filename.c_str());
		return false;
	}
	_format = audioDataFormat(filename);
	_audioDataSize = file.size();
	_audioData.reset(Common::SharedPtr<byte>((byte *)malloc(_audioDataSize), free));
	file.read(_audioData.get(), _audioDataSize);
	return true;
}

//-----------------------------------------------------------------------

static Audio::SeekableAudioStream *createAudioStream(Common::MemoryReadStream *data, uint format) {
	switch (format) {
#ifdef USE_VORBIS
	case kOgg:
		return Audio::makeVorbisStream(data, DisposeAfterUse::YES);
#endif
	case kWav:
		return Audio::makeWAVStream(data, DisposeAfterUse::YES);
	}
	return nullptr;
}

iSoundChannel *cOpenALSoundData::CreateChannel(int priority) {
	IncUserCount();
	if (!_audioData)
		return nullptr;
	auto *dataStream = new Common::MemoryReadStream(_audioData, _audioDataSize);
	auto *audioStream = createAudioStream(dataStream, _format);
	return new cOpenALSoundChannel(this, audioStream, mpSoundManger, _lowLevelSound, priority);
}

} // namespace hpl
